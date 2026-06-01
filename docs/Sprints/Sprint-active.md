# Active Sprint

## SP-018: Cross-Platform Build — Windows (MSVC) + SecureStore Trade Study

**Status:** 🟡 Active
**Epic:** EP-008: Multi-Scene Navigation and Cross-Platform Build
**Goal:** Get ScriviCore building and all CTests passing on Windows (MSVC 19.38+ / VS 2022). Implement `AppSupportLayout::platformDefault()` for Linux (`$XDG_DATA_HOME`) and Windows (`%APPDATA%`). Produce a written trade study selecting the `SecureStore` implementation strategy for Linux and Windows.
**Start Date:** 2026-05-31
**End Date:** —
**Capacity:** —

### Assigned Tasks

| ID     | Title | Priority | Status |
| ------ | ----- | -------- | ------ |
| T-0056 | Windows CMake Build — MSVC Green + Gap Document | Critical | 🟠 Unverified |
| T-0057 | `AppSupportLayout` — Linux and Windows Platform Paths | High | 🟠 Unverified |
| T-0058 | SecureStore Trade Study — Linux and Windows | High | 🟠 Unverified |

### Assigned Issues

| ID | Title | Severity | Status |
| -- | ----- | -------- | ------ |
| —  | None  | —        | —      |

### Sprint Notes

Carry-forward from SP-017 gap document:
- `PRIx64` fix already applied — correct for MSVC as well; no further change needed
- `fs::rename` on Windows uses `MoveFileExW(MOVEFILE_REPLACE_EXISTING)` via MSVC STL — verify but expect no code change
- `popen`/`_popen` alias already present in `Process.cpp`
- `WIFEXITED`/`WEXITSTATUS` correctly guarded `#ifndef _WIN32`
- `SHGetKnownFolderPath` for `AppSupportLayout::platformDefault()` requires linking `shell32.lib` — add to CMake in T-0057
- Windows build via Docker using `mcr.microsoft.com/windows/servercore` is not viable on macOS ARM; use GitHub Actions `windows-latest` runner for T-0056
- SecureStore trade study must produce a concrete recommendation per platform, not just a list of options

---

*Last Updated: 2026-05-31 (SP-018 activated)*
