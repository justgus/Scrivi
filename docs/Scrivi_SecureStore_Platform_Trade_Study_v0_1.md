# Scrivi SecureStore Platform Trade Study v0.1

**Status:** Draft  
**Task:** T-0058  
**Date:** 2026-05-31  
**Author:** Scrivi Engineering

---

## Purpose

`SecureStore` is the interface through which ScriviCore persists sensitive credentials — currently the per-device `deviceSecret` used to derive project encryption keys and sign identity claims. On Apple platforms this is implemented via `KeychainSecureStore` (T-0049), which wraps the macOS/iOS Security framework Keychain.

This document trades the available options for **Linux** and **Windows** and produces a concrete recommendation for each platform.

---

## Interface Contract (recap)

```cpp
class SecureStore {
public:
    virtual ~SecureStore() = default;

    // Stores or overwrites the secret for key.
    virtual Result<void> store(const std::string& key,
                               const std::string& value) = 0;

    // Retrieves the secret for key. Returns failure if not found.
    virtual Result<std::string> retrieve(const std::string& key) = 0;

    // Removes the secret for key. Succeeds even if key is absent.
    virtual Result<void> remove(const std::string& key) = 0;
};
```

Requirements:

- Secrets must survive process restart.
- Secrets must not appear in plain text in app support directories or config files that a naive backup might pick up.
- The implementation must be **statically linkable** into ScriviCore without requiring a daemon running in the same session (or must gracefully degrade to the encrypted-file fallback if the daemon is unavailable).
- No GUI prompt may block a headless build or CLI invocation.

---

## Linux Options

### Option L-1: libsecret (GNOME Keyring / SecretService D-Bus API)

**How it works:** libsecret is the reference C library for the [SecretService D-Bus specification](https://specifications.freedesktop.org/secret-service/), which GNOME Keyring and KWallet both implement. Secrets are stored in the session keyring daemon and unlocked on user login.

**Pros:**
- OS-managed secret storage; secrets are not accessible to other users on a multi-user system.
- No encryption code to maintain in Scrivi.
- Standard API; works on any distribution that ships a compliant keyring daemon.
- libsecret is available as a system package on all major distros (`libsecret-1-dev` / `libsecret-devel`).

**Cons:**
- **Runtime daemon dependency**: requires a running GNOME Keyring or KWallet daemon. Not available in headless server environments, Docker containers, WSL without a session bus, or minimal DEs.
- **D-Bus session bus dependency**: no D-Bus session means libsecret calls block indefinitely or fail with a cryptic error.
- Requires a dynamic library on the system (`-lsecret-1`); not trivially statically linkable due to GLib dependency chain.
- CMake `find_package(PkgConfig)` + `pkg_check_modules(LIBSECRET …)` integration is feasible but adds build complexity.
- First-unlock prompt may appear if the keyring is locked (e.g., after screen lock, or on first run).
- **Non-interactive unlock is not possible** without also configuring PAM integration; headless builds and CLI tools cannot rely on it.

**Assessment:** Appropriate for a fully-installed desktop GUI Scrivi app running in a GNOME or KDE session. **Not appropriate as the sole implementation** because the ctest suite and any headless use must also work without a keyring daemon.

---

### Option L-2: Encrypted-File Store (`EncryptedFileSecureStore`)

**How it works:** A portable C++ class that stores secrets as AES-256-GCM encrypted entries in a dedicated file at `$XDG_DATA_HOME/Scrivi/secure/secrets.enc`. The per-file encryption key is derived from:

1. The machine's `/etc/machine-id` (stable across reboots, unique per install, not a user secret),
2. The user's UID and username,
3. A random salt stored in plain text alongside the encrypted file.

The derived key is never persisted; it is re-derived on each access from the combination above. An attacker with the encrypted file but without root access to the machine cannot trivially extract the key.

**Pros:**
- **Zero runtime dependencies** beyond OpenSSL or mbedTLS (already available on all target Linux distros, or bundled via FetchContent).
- Works in Docker, WSL, CI, headless servers — all environments where libsecret is unavailable.
- Statically linkable.
- No D-Bus, no daemon, no GUI prompt.
- Fully controllable fallback behavior.

**Cons:**
- Encryption is implemented in Scrivi; any implementation flaw is our bug.
- If `/etc/machine-id` changes (e.g., after a re-image or cloning a VM), the derived key changes and stored secrets become inaccessible. Mitigation: store the salt alongside the cipher text and surface a clear error for re-enrollment.
- Security model is weaker than the keyring: root can read `/etc/machine-id` and the encrypted file, so a root-level attacker can reconstruct the key. This is acceptable for the `deviceSecret` use case (it protects against casual file-system snooping, not a privileged system compromise).

**Assessment:** Appropriate for all Linux environments. Solves the headless requirement cleanly.

---

### Option L-3: Hybrid (libsecret primary, encrypted-file fallback)

Use libsecret when the SecretService D-Bus interface is available; fall back to `EncryptedFileSecureStore` when it is not.

**Pros:**
- Best security on a real desktop; graceful degradation everywhere else.

**Cons:**
- Requires implementing both. libsecret adds build complexity and a conditional CMake path. The hybrid dispatch logic must handle all failure modes of D-Bus initialization (timeout, no session bus, daemon locked).
- Maintenance surface doubles; two SecureStore implementations must be kept in sync.
- Not needed for the current MVP — the `deviceSecret` threat model does not require OS keyring strength.

**Assessment:** Desirable long-term for the shipped desktop app, but premature for the current implementation phase.

---

### Linux Recommendation: **Option L-2 — `EncryptedFileSecureStore`**

**Rationale:**

The immediate goal (EP-008) is a green ctest build on Ubuntu. The device secret's threat model is protecting against casual filesystem snooping, not privileged attackers. `EncryptedFileSecureStore` meets that bar, has zero runtime dependencies, and works in every environment the ctest suite runs in.

**Defer Option L-1/L-3 (libsecret hybrid) to the Linux desktop app Epic**, when there is a real GUI session and the cost of the D-Bus dependency chain is justified by the threat model of a published app.

**Crypto recommendation:** AES-256-GCM via **OpenSSL 3.x** (`libcrypto`), which is universally available on target distros and has a clean EVP API. Key derivation via HKDF-SHA256 over the machine-id + UID + salt inputs.

**Key derivation inputs:**

| Input | Source | Notes |
|-------|--------|-------|
| Machine ID | `/etc/machine-id` | 32-char hex string, stable per install |
| UID | `getuid()` → decimal string | Isolates per-user stores on multi-user systems |
| Username | `getpwuid(getuid())->pw_name` | Belt-and-suspenders with UID |
| Random salt | Stored in plain text in `secure/secrets.enc.salt` (16 bytes, hex) | Generated once on first store; prevents precomputation |

---

## Windows Options

### Option W-1: DPAPI (`CryptProtectData` / `CryptUnprotectData`)

**How it works:** The Windows Data Protection API (DPAPI) encrypts arbitrary blobs using a key derived from the current user's Windows credentials. The encrypted blob can be stored as a file. Decryption succeeds only in the same user context.

**Pros:**
- Built into Windows since XP; zero external dependencies.
- OS manages the key; ties protection to Windows account credentials.
- Simple API: `CryptProtectData` / `CryptUnprotectData` from `<dpapi.h>` + `Crypt32.lib`.
- Works headlessly (no GUI prompt for encryption/decryption of LOCAL_MACHINE scope — for CURRENT_USER scope, it requires the user's profile to be loaded, which it always is in an interactive session).
- Widely used: this is how Chrome, Firefox, and most Windows apps protect local secrets.

**Cons:**
- DPAPI master key is tied to the Windows user account; if the account password is reset by an admin without knowing the old password, DPAPI-encrypted data is permanently lost (no recovery path via Scrivi — user would need to re-enroll).
- Not portable to Wine/Proton (limited DPAPI support).
- `Crypt32.lib` must be linked; already implied by the `SHGetKnownFolderPath` (`Shell32`) addition in T-0057 — add `Crypt32` alongside it.

**Assessment:** The correct Windows choice. This is the platform-idiomatic, zero-dependency, OS-managed secret protection mechanism. Every major Windows application uses it for local secret storage.

---

### Option W-2: Windows Credential Manager (CredMan)

**How it works:** `CredWriteW` / `CredReadW` APIs store named credentials in the Windows Credential Manager (visible in Control Panel → Credential Manager).

**Pros:**
- Credentials are visible and manageable by the user in the OS UI.
- Protected by DPAPI internally.

**Cons:**
- Maximum blob size is limited (~2.5 KB per credential, 512 bytes for the password field on some versions).
- Credentials are visible in Credential Manager — potentially surprising to users and exposes internal key names.
- More complex API than raw DPAPI for the use case of a single opaque blob.

**Assessment:** Adds UI-visible artefacts and size constraints for no benefit over raw DPAPI in Scrivi's use case. Not recommended.

---

### Option W-3: Encrypted-File Store (same as L-2, Windows derivation)

On Windows the machine ID equivalent is the `MachineGuid` registry value (`HKLM\SOFTWARE\Microsoft\Cryptography\MachineGuid`). The same AES-256-GCM approach as Linux.

**Pros:**
- Code reuse with the Linux implementation.

**Cons:**
- Reinvents DPAPI. DPAPI is the correct Windows primitive; using a custom encrypted file is strictly weaker (DPAPI additionally ties decryption to user credentials, not just machine identity).

**Assessment:** Unnecessary when DPAPI is available.

---

### Windows Recommendation: **Option W-1 — `DPAPISecureStore`**

**Rationale:**

DPAPI is the idiomatic Windows mechanism for protecting local secrets. It has zero external dependencies, is available on all target Windows versions (Windows 11 as per Scrivi's target), and provides stronger protection than a custom encrypted-file approach because decryption is gated on the user's Windows credentials.

**Implementation notes:**

- Storage: encrypted blob written to `%APPDATA%\Scrivi\secure\<key-name>.dpapi`. One file per key (same as Keychain item granularity on Apple).
- `dwFlags` for `CryptProtectData`: `CRYPTPROTECT_LOCAL_MACHINE` flag intentionally **not** set — use `CURRENT_USER` scope so the secret is not accessible to other users on a shared machine.
- `Crypt32.lib` already required; add to the `$<$<PLATFORM_ID:Windows>:…>` generator expression in `CMakeLists.txt` alongside `Shell32`.
- On encrypt: `CryptProtectData` → write `DATA_BLOB.cbData` + `DATA_BLOB.pbData` to file.
- On decrypt: read file → populate `DATA_BLOB` → `CryptUnprotectData` → extract string.

---

## Implementation Plan

### Phase 1 (EP-008 / SP-018 scope — T-0058)

This document is the deliverable for T-0058. No code is written in this task.

### Phase 2 (next cross-platform Epic)

| Platform | Class | File | CMake |
|----------|-------|------|-------|
| Linux | `EncryptedFileSecureStore` | `src/platform/EncryptedFileSecureStore.{hpp,cpp}` | Link `libcrypto` (OpenSSL 3) |
| Windows | `DPAPISecureStore` | `src/platform/DPAPISecureStore.{hpp,cpp}` | Link `Crypt32` |

Both classes live in `src/platform/` alongside `KeychainSecureStore`. The `ScriviCoreAdapter` selects the correct implementation via `#ifdef` at compile time, exactly as `KeychainSecureStore` is selected on Apple platforms today.

The `MockSecureStore` (already in `tests/mocks/`) remains the test double for all unit and integration tests on all platforms.

### Phase 3 (deferred — Linux desktop app Epic)

Evaluate libsecret hybrid (`EncryptedFileSecureStore` fallback when D-Bus unavailable) for the shipped GNOME/KDE desktop app.

---

## Decision Summary

| Platform | Recommendation | Rationale |
|----------|---------------|-----------|
| Linux (headless/CI/daemon-less) | `EncryptedFileSecureStore` (AES-256-GCM, OpenSSL) | Zero deps; works everywhere; matches current threat model |
| Linux (desktop GUI, future) | Hybrid: libsecret + `EncryptedFileSecureStore` fallback | OS keyring when available; deferred to desktop app Epic |
| Windows | `DPAPISecureStore` | Platform-idiomatic; zero deps; user-credential-gated decryption |

---

*Last Updated: 2026-05-31 (T-0058 — initial draft)*
