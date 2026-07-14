#pragma once

#include "scrivi/Services.hpp"
#include "scrivi/Types.hpp"

#include <string>

namespace scrivi::platform {

// EncryptedFileSecureStore — the persistent SecureStore for headless / desktop
// Linux (and any POSIX host), per the SecureStore Platform Trade Study (T-0058,
// Option L-2). Replaces the in-memory PrototypeSecureStore so the local identity
// survives process restart (SP-059 / AC4).
//
// Each secret is written as an AES-256-GCM-encrypted file at
//   <secureDir>/<sanitized-key>.enc
// The per-store key is derived (HKDF-SHA256) from stable machine + user inputs:
//   • /etc/machine-id            (stable per install; not a user secret)
//   • getuid() + username        (isolates per-user stores on multi-user hosts)
//   • a random 16-byte salt      (stored in <secureDir>/store.salt, plaintext)
// The derived key is never persisted; it is re-derived on each access. This
// protects against casual filesystem snooping (the deviceSecret threat model),
// not a privileged/root attacker — as the trade study documents.
//
// Zero runtime daemon/D-Bus dependency (works in Docker, CI, WSL); depends only
// on OpenSSL 3 libcrypto, which is universally available on the target distros.
class EncryptedFileSecureStore final : public SecureStore {
public:
    // secureDir is the directory holding the encrypted secret files and salt
    // (production: <appSupportRoot>/secure). Created on first write. Tests pass a
    // temp dir. The directory is NOT required to exist at construction.
    explicit EncryptedFileSecureStore(std::string secureDir);

    Result<bool>        containsSecret(std::string_view key) override;
    Result<void>        putSecret(std::string_view key, const SecretBytes& value) override;
    Result<SecretBytes> getSecret(std::string_view key) override;

private:
    // Absolute path of the .enc file for a key (key is sanitized to a safe
    // filename — the identity key "scrivi.identity.v1" maps to a fixed name).
    std::string fileFor(std::string_view key) const;

    // Derives the 32-byte AES key from machine-id + uid + username + salt. The
    // salt is loaded (or generated + persisted) from <secureDir>/store.salt.
    Result<std::string> deriveKey() const;

    std::string secureDir_;
};

} // namespace scrivi::platform
