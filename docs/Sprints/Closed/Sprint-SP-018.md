# SP-018: Cross-Platform Build — Windows (MSVC) + SecureStore Trade Study

**Status:** ✅ Closed
**Epic:** EP-008: Multi-Scene Navigation and Cross-Platform Build
**Goal:** Get ScriviCore building and all CTests passing on Windows (MSVC 19.38+ / VS 2022). Implement `AppSupportLayout::platformDefault()` for Linux (`$XDG_DATA_HOME`) and Windows (`%APPDATA%`). Produce a written trade study selecting the `SecureStore` implementation strategy for Linux and Windows.
**Start Date:** 2026-05-31
**End Date:** 2026-06-01
**Capacity:** —

### Assigned Tasks

| ID     | Title | Priority | Status |
| ------ | ----- | -------- | ------ |
| T-0056 | Windows CMake Build — MSVC Green + Gap Document | Critical | ✅ Verified |
| T-0057 | `AppSupportLayout` — Linux and Windows Platform Paths | High | ✅ Verified |
| T-0058 | SecureStore Trade Study — Linux and Windows | High | ✅ Verified |

### Assigned Issues

| ID | Title | Severity | Status |
| -- | ----- | -------- | ------ |
| —  | None  | —        | —      |

### Sprint Notes

Carry-forward from SP-017 gap document:
- `PRIx64` fix already applied — correct for MSVC as well; no further change needed
- `fs::rename` on Windows uses `MoveFileExW(MOVEFILE_REPLACE_EXISTING)` via MSVC STL — verified; no code change needed
- `popen`/`_popen` alias already present in `Process.cpp`
- `WIFEXITED`/`WEXITSTATUS` correctly guarded `#ifndef _WIN32`
- `SHGetKnownFolderPath` for `AppSupportLayout::platformDefault()` requires linking `shell32.lib` — added to CMake in T-0057

Windows build via Docker using `mcr.microsoft.com/windows/servercore` is not viable on macOS ARM; build and test executed natively on dumbledor-2 (Windows 11) via SSH.

### Retrospective

**Completed:**
- T-0056: Windows MSVC build green; 160/160 CTests pass; gap document written
- T-0057: `platformDefault()` implemented for all three platforms; 3 new tests pass
- T-0058: SecureStore trade study produced; DPAPI (Windows) and EncryptedFile (Linux) recommended

**Returned to Backlog:**
- None

**What went well:**
- Pre-flight gap analysis from SP-017 correctly predicted all MSVC issues
- UTF-8 manifest approach cleanly solved the em-dash test name issue on Windows
- `.generic_string()` fix for path separators was simple and correct on all platforms
- All three platforms green simultaneously at sprint close: macOS 160/160, Ubuntu 160/160, Windows 160/160

**What to improve:**
- Nothing significant for this sprint

**Carry-forward notes for SP-019 (Multi-Scene C++ Core):**
- `SecureStore` implementation (DPAPI/EncryptedFile) deferred — `MockSecureStore` continues for tests
- `build-windows.ps1` and `test-ubuntu.sh` scripts added to `scripts/`; `devops/docker/linux/Dockerfile` added
- Windows repo on dumbledor-2 has three extra unit test files modified (`PathUtilsTests.cpp`, `SlugTests.cpp`, `TextStatsTests.cpp`) — merge pending commit from macOS

---

*Closed: 2026-06-01 (T-0056, T-0057, T-0058 verified; user approved Sprint close)*
