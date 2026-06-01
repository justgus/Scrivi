# Active Sprint

## SP-020: clang-tidy Housekeeping — Full Warning Sweep

**Status:** 🟡 Active
**Epic:** None
**Goal:** Eliminate all 234 clang-tidy warnings across ScriviCore's 42 source files. Warnings are grouped into four tasks by category. At completion, `scripts/verify-scrivi-core-tidy.sh` must pass with zero warnings, and all 165 ctests must remain green.
**Start Date:** 2026-06-01
**End Date:** —
**Capacity:** —

### Assigned Tasks

| ID     | Title | Priority | Status |
| ------ | ----- | -------- | ------ |
| T-0063 | clang-tidy — `readability-braces-around-statements` (142 warnings) | Medium | 🟣 Implemented — Not Verified |
| T-0064 | clang-tidy — `modernize-use-designated-initializers` + `readability-qualified-auto` (51 warnings) | Medium | 🟣 Implemented — Not Verified |
| T-0065 | clang-tidy — `[[nodiscard]]`, `const`, and `static` member functions (21 warnings) | Medium | 🟣 Implemented — Not Verified |
| T-0066 | clang-tidy — Remaining checks (20 warnings) | Low | 🟣 Implemented — Not Verified |

### Assigned Issues

| ID | Title | Severity | Status |
| -- | ----- | -------- | ------ |
| —  | None  | —        | —      |

### Sprint Notes

- Execute tasks in order: T-0063 first (largest, most mechanical), T-0066 last (manual).
- After each task: `cmake --build build/ninja --parallel && ctest --test-dir build/ninja --output-on-failure`.
- Final gate: `bash scripts/verify-scrivi-core-tidy.sh` must exit 0.
- Note: SP-020 is not under EP-008 but is a housekeeping sprint run concurrently with EP-008 close.

---

*Last Updated: 2026-06-01 (T-0063–T-0066 all implemented — not verified; tidy gate passes)*
