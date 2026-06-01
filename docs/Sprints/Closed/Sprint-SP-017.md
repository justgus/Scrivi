# SP-017: Cross-Platform Build — Ubuntu (GCC/Clang)

**Status:** ✅ Closed
**Epic:** EP-008: Multi-Scene Navigation and Cross-Platform Build
**Goal:** Get ScriviCore building and all CTests passing on Ubuntu using GCC 13+ (or Clang 17+) via CMake. Document every compiler gap, stdlib difference, or FetchContent issue encountered. No new features — a clean green build is the only success criterion.
**Start Date:** 2026-05-30
**End Date:** 2026-05-31
**Capacity:** —

### Assigned Tasks

| ID     | Title | Priority | Status |
| ------ | ----- | -------- | ------ |
| T-0055 | Ubuntu CMake Build — GCC/Clang Green + Gap Document | Critical | ✅ Verified |

### Assigned Issues

| ID | Title | Severity | Status |
| -- | ----- | -------- | ------ |
| —  | None  | —        | —      |

### Sprint Notes

Build executed via `gcc:13` Docker image (GCC 13.4.0, CMake 3.25.1, Ninja) against the macOS source tree mounted read-only. Two pre-flight portability fixes applied before the run; build came up clean first attempt with zero warnings.

### Retrospective

**Completed:**
- T-0055: Ubuntu GCC 13 build green; 159/159 CTests pass; gap document written

**Returned to Backlog:**
- None

**What went well:**
- Pre-flight code analysis before running Docker identified both issues precisely — no iterative debug cycle needed
- No C++23 stdlib gaps anywhere in the codebase; the portability surface was smaller than expected
- `std::filesystem`, `FetchContent`, and `AtomicWrite` all portable as-is

**What to improve:**
- Nothing significant for this sprint

**Carry-forward notes for SP-018 (Windows/MSVC):**
- `PRIx64` fix is also correct for MSVC — no further change needed there
- `fs::rename` on Windows uses `MoveFileExW(MOVEFILE_REPLACE_EXISTING)` via MSVC STL — no code change needed
- `popen`/`_popen` alias already present
- `SHGetKnownFolderPath` for `AppSupportLayout::platformDefault()` (T-0057) requires linking `shell32.lib` — add to CMake when implementing

---

*Closed: 2026-05-31 (T-0055 verified; user approved Sprint close)*
