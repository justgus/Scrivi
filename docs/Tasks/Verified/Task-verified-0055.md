# T-0055: Ubuntu CMake Build — GCC/Clang Green + Gap Document

**Status:** ✅ Verified
**Component:** ScriviCore (cross-platform build)
**Priority:** Critical
**Date Requested:** 2026-05-30
**Date Implemented:** 2026-05-31
**Date Verified:** 2026-05-31
**Sprint Assigned:** SP-017

## Result

**GCC 13.4.0 (Ubuntu/Debian via `gcc:13` Docker image): 159/159 CTests pass. Zero warnings. Zero errors.**

- CMake 3.25.1 (apt); Ninja build system
- `cxx_std_23` feature flag: accepted cleanly by GCC 13 (`-std=c++23`)
- FetchContent for nlohmann/json v3.11.3 and Catch2 v3: resolved and built without issue
- macOS CTest baseline unchanged: 159/159 still passing after fixes

## Fixes Applied

Two portability issues identified by pre-flight analysis and fixed before the build:

### 1. `%llx` / `unsigned long long` in `snprintf` — `SystemUUIDProvider.cpp`, `IdentityService.cpp`

On GCC, `uint64_t` is `unsigned long` (not `unsigned long long`) on LP64 platforms. Using `%llx` with a `uint64_t` value produces a format-mismatch warning (and would be an error under `-Werror`).

**Fix:** Replaced with `PRIx64` / `PRIx32` / `PRIx16` from `<cinttypes>` and cast operands to the matching `uint32_t` / `uint16_t` / `uint64_t` types. Added `#include <cinttypes>` and `#include <cstdint>` to both files.

- `ScriviCore/src/platform/SystemUUIDProvider.cpp`
- `ScriviCore/src/identity/IdentityService.cpp`

### 2. `WIFEXITED` / `WEXITSTATUS` without `<sys/wait.h>` — `Process.cpp`

These macros are defined in `<sys/wait.h>` on Linux. The file only included `<unistd.h>` on the non-Windows path; on some toolchains the transitive include is absent.

**Fix:** Added `#include <sys/wait.h>` inside the `#ifndef _WIN32` guard.

- `ScriviCore/src/util/Process.cpp`

## Gap Document — Findings for SP-018 (Windows) and Future Reference

### No C++23 stdlib gaps found
No `<format>`, `<expected>`, `<ranges>`, or `<print>` usage exists anywhere in ScriviCore. The codebase uses only well-supported C++17/20 stdlib features. GCC 13 C++23 gap risk: **zero**.

### `std::filesystem` — clean
All `std::filesystem` usage (`rename`, `remove`, `create_directories`, `directory_iterator`, `path::string()`) is standard and portable. GCC 13 includes `std::filesystem` in the main library with no `-lstdc++fs` flag needed (that was only required on GCC 8).

### `AtomicWrite` — clean on Linux
`fs::rename(tmp, target)` maps to POSIX `rename()` which is atomic on Linux ext4/xfs. No changes needed.

### `FetchContent` — clean
Both dependencies (nlohmann/json SHA256-pinned, Catch2) resolve and build without issue from the Docker network. No proxy or offline concerns encountered.

### `popen` / `pclose` — clean on Linux
The `#ifdef _WIN32` alias for `_popen`/`_pclose` works correctly. On Linux, `popen` is POSIX standard.

### `AppSupportLayout::platformDefault()` — not yet implemented
`AppSupportLayout` currently has no `platformDefault()` method — it only bootstraps a caller-supplied root. This is intentional: `platformDefault()` is T-0057 / SP-018. The existing `bootstrapAppSupport()` compiles and links cleanly on Linux.

### Items to watch on Windows (SP-018)
- `%llx` fix using `PRIx64` is also correct for MSVC — `uint64_t` is `unsigned __int64` there and `%I64x` is the MSVC-ism; `PRIx64` resolves correctly on all three compilers.
- `fs::rename` on Windows: `std::filesystem::rename` calls `MoveFileExW(MOVEFILE_REPLACE_EXISTING)` in MSVC's STL implementation — atomic replace on NTFS. No change needed.
- `popen`/`_popen` alias already present in `Process.cpp`.
- `WIFEXITED`/`WEXITSTATUS` are `#ifndef _WIN32` guarded — correct.
- `SHGetKnownFolderPath` for `AppSupportLayout::platformDefault()` requires linking `shell32.lib` — note for T-0057 CMake changes.

---

*Verified: 2026-05-31 (user approved)*
