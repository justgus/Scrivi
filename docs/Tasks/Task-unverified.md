# Unverified Tasks

Tasks listed here are implemented and awaiting user verification before being archived.

---

## T-0057: `AppSupportLayout` — Linux and Windows Platform Paths

**Status:** 🟠 Implemented — Not Verified
**Sprint:** SP-018
**Epic:** EP-008

**Implementation:**

- `AppSupportLayout::platformDefault()` added to `AppSupportLayout.hpp` / `.cpp`
- Three platform branches:
  - **Apple:** `~/Library/Application Support/Scrivi` (via `getenv("HOME")` / `getpwuid`)
  - **Linux:** `$XDG_DATA_HOME/Scrivi` if set and non-empty, else `~/.local/share/Scrivi`
  - **Windows:** `SHGetKnownFolderPath(FOLDERID_RoamingAppData) + \Scrivi` via `SHGetKnownFolderPath`
- Returns `Result<AbsolutePath>`; all failure paths use `Error{ErrorCode::ioError, …}`
- `Shell32` linked via `$<$<PLATFORM_ID:Windows>:Shell32>` generator expression in `CMakeLists.txt`
- `Error.hpp` included in `.cpp` for `ErrorCode`
- Three new tests added to `AppSupportLayoutTests.cpp` (tagged `[T-0057]`):
  - `platformDefault — returns a non-empty path ending in 'Scrivi'` (all platforms)
  - `platformDefault — Linux respects XDG_DATA_HOME when set` (`#ifdef __linux__`)
  - `platformDefault — Linux falls back to ~/.local/share when XDG_DATA_HOME unset` (`#ifdef __linux__`)
- **CTest result:** 160/160 passed (was 159/159)

**Files changed:**
- `ScriviCore/src/platform/AppSupportLayout.hpp` — declaration added
- `ScriviCore/src/platform/AppSupportLayout.cpp` — implementation added
- `ScriviCore/CMakeLists.txt` — `Shell32` generator expression added
- `ScriviCore/tests/integration/AppSupportLayoutTests.cpp` — three new tests + `platform/AppSupportLayout.hpp` include

---

## T-0058: SecureStore Trade Study — Linux and Windows

**Status:** 🟠 Implemented — Not Verified
**Sprint:** SP-018
**Epic:** EP-008

**Implementation:**

Trade study document produced at `docs/Scrivi_SecureStore_Platform_Trade_Study_v0_1.md`.

**Conclusions:**

| Platform | Recommendation | Notes |
|----------|---------------|-------|
| Linux (current) | `EncryptedFileSecureStore` (AES-256-GCM via OpenSSL 3) | Zero deps; works headless, CI, Docker |
| Linux (future desktop app) | libsecret hybrid + encrypted-file fallback | Deferred to desktop app Epic |
| Windows | `DPAPISecureStore` (`CryptProtectData` / `CryptUnprotectData`) | Platform-idiomatic; zero deps; user-credential-gated |

Implementation of these classes is deferred to the next cross-platform Epic (EP-009 or later). The `MockSecureStore` continues to serve all test suites.

**Files changed:**
- `docs/Scrivi_SecureStore_Platform_Trade_Study_v0_1.md` — created

---

## T-0056: Windows CMake Build — MSVC Green + Gap Document

**Status:** 🟠 Implemented — Not Verified
**Sprint:** SP-018
**Epic:** EP-008

**Implementation:**

MSVC 19.44 (VS 2022) build verified green on dumbledor-2 (Windows 11). 160/160 CTests pass.

**Gaps found and fixed:**

| # | Gap | Fix |
|---|-----|-----|
| 1 | `PathUtils::join` / `makeAbsolute` used `.string()` — produces `\` on Windows | Changed to `.generic_string()` in `PathUtils.cpp` |
| 2 | `LocalFileSystem::listDirectory` used `.string()` — produces `\` paths | Changed to `.generic_string()` |
| 3 | Test fixture `std::ofstream` opened without `std::ios::binary` — text mode translates `\n` to `\r\n` on write | Added `std::ios::binary` to all bare ofstream opens in test files |
| 4 | Test names with UTF-8 em-dash `—` mangled by Windows console codepage in CTest name matching | Replaced all ` — ` with ` - ` in test names across 16 files |
| 5 | `SHGetKnownFolderPath` requires `Shell32.lib` | Added via CMake generator expression (T-0057) |
| 6 | `PRIx64` / `<cinttypes>` | Fixed in SP-017 |
| 7 | `<sys/wait.h>` | Fixed in SP-017 |

No MSVC C++23 feature parity issues. `std::format`, `std::expected`, `<ranges>` not used. `fs::rename` uses `MoveFileExW(MOVEFILE_REPLACE_EXISTING)` via MSVC STL — no code change needed.

**All three platforms green:**
- macOS ctest: 160/160
- Ubuntu ctest: 160/160 (SP-017)
- Windows MSVC ctest: 160/160

**Files changed:**
- `ScriviCore/src/util/PathUtils.cpp` — `.generic_string()` for `join` and `makeAbsolute`
- `ScriviCore/src/platform/LocalFileSystem.cpp` — `.generic_string()` for `listDirectory`
- `ScriviCore/tests/CMakeLists.txt` — `/utf-8` compile flag + UTF-8 manifest for MSVC
- `ScriviCore/tests/utf8.manifest` — new file: active code page UTF-8 manifest
- `ScriviCore/tests/integration/OpenProjectTests.cpp` — `std::ios::binary` on ofstream
- `ScriviCore/tests/integration/ExternalChangeTests.cpp` — `std::ios::binary` on ofstream
- `ScriviCore/tests/integration/MvpLoopTests.cpp` — `std::ios::binary` on ofstream
- `ScriviCore/tests/integration/ApplyRepairTests.cpp` — `std::ios::binary` on ofstream/ifstream
- All 16 test `.cpp` files — em-dash `—` replaced with ` - ` in test names

---

*Last Updated: 2026-05-31 (T-0056, T-0057, T-0058 implemented)*
