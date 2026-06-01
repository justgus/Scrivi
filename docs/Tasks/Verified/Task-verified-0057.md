# T-0057: `AppSupportLayout` — Linux and Windows Platform Paths

**Status:** ✅ Verified
**Component:** ScriviCore / platform
**Priority:** High
**Date Requested:** 2026-05-31
**Date Implemented:** 2026-05-31
**Date Verified:** 2026-06-01
**Sprint Assigned:** SP-018

## Result

`AppSupportLayout::platformDefault()` implemented and tested on all three platforms.

## Implementation

- `AppSupportLayout::platformDefault()` added to `AppSupportLayout.hpp` / `.cpp`
- Three platform branches:
  - **Apple:** `~/Library/Application Support/Scrivi` (via `getenv("HOME")` / `getpwuid`)
  - **Linux:** `$XDG_DATA_HOME/Scrivi` if set and non-empty, else `~/.local/share/Scrivi`
  - **Windows:** `SHGetKnownFolderPath(FOLDERID_RoamingAppData)` + `\Scrivi`
- Returns `Result<AbsolutePath>`; all failure paths use `Error{ErrorCode::ioError, …}`
- `Shell32` linked via `$<$<PLATFORM_ID:Windows>:Shell32>` generator expression
- Three new tests added to `AppSupportLayoutTests.cpp` (tagged `[T-0057]`):
  - `platformDefault - returns a non-empty path ending in 'Scrivi'` (all platforms)
  - `platformDefault - Linux respects XDG_DATA_HOME when set` (`#ifdef __linux__`)
  - `platformDefault - Linux falls back to ~/.local/share when XDG_DATA_HOME unset` (`#ifdef __linux__`)
- CTest result: 160/160 passed (was 159/159)

## Files Changed

- `ScriviCore/src/platform/AppSupportLayout.hpp` — declaration added
- `ScriviCore/src/platform/AppSupportLayout.cpp` — implementation added
- `ScriviCore/CMakeLists.txt` — `Shell32` generator expression added
- `ScriviCore/tests/integration/AppSupportLayoutTests.cpp` — three new tests

---

*Verified: 2026-06-01 (user approved)*
