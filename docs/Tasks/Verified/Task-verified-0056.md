# T-0056: Windows CMake Build — MSVC Green + Gap Document

**Status:** ✅ Verified
**Component:** ScriviCore (cross-platform build)
**Priority:** Critical
**Date Requested:** 2026-05-31
**Date Implemented:** 2026-05-31
**Date Verified:** 2026-06-01
**Sprint Assigned:** SP-018

## Result

**MSVC 19.44 (VS 2022, Windows 11, dumbledor-2): 160/160 CTests pass. Zero errors.**

- Visual Studio 17 2022 generator, x64
- All three platforms green at close: macOS 160/160, Ubuntu 160/160, Windows 160/160

## Gaps Found and Fixed

| # | Gap | Fix |
|---|-----|-----|
| 1 | `PathUtils::join` / `makeAbsolute` used `.string()` — produces `\` on Windows | Changed to `.generic_string()` in `PathUtils.cpp` |
| 2 | `LocalFileSystem::listDirectory` used `.string()` — produces `\` paths | Changed to `.generic_string()` |
| 3 | Test fixture `std::ofstream` opened without `std::ios::binary` — text mode translates `\n` to `\r\n` on write | Added `std::ios::binary` to all bare ofstream opens in test files |
| 4 | Test names with UTF-8 em-dash `—` mangled by Windows console codepage in CTest name matching | Replaced all ` — ` with ` - ` in test names across 16 files |
| 5 | `SHGetKnownFolderPath` requires `Shell32.lib` | Added via CMake generator expression (T-0057) |
| 6 | `PRIx64` / `<cinttypes>` | Fixed in SP-017 |
| 7 | `<sys/wait.h>` | Fixed in SP-017 |

## Files Changed

- `ScriviCore/src/util/PathUtils.cpp` — `.generic_string()` for `join` and `makeAbsolute`
- `ScriviCore/src/platform/LocalFileSystem.cpp` — `.generic_string()` for `listDirectory`
- `ScriviCore/tests/CMakeLists.txt` — `/utf-8` compile flag + UTF-8 manifest for MSVC
- `ScriviCore/tests/utf8.manifest` — new file: active code page UTF-8 manifest
- All 16 test `.cpp` files — em-dash `—` replaced with ` - ` in test names
- Several test `.cpp` files — `std::ios::binary` added to bare ofstream opens

---

*Verified: 2026-06-01 (user approved)*
