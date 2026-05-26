# SP-004: Skeleton v0.2 Migration — Structural Alignment

**Status:** 🟡 Active
**Epic:** EP-004: Skeleton v0.2 Migration
**Goal:** Restructure the repository to match the approved Skeleton v0.2 layout exactly: introduce `src/domain/`, relocate `AppSupportLayout`, consolidate the two extra unit test files into the canonical six, and reach a first confirmed green build + green test suite.
**Start Date:** 2026-05-26
**End Date:** 2026-05-26
**Capacity:** TBD

### Assigned Tasks

| ID     | Title | Priority | Status |
| ------ | ----- | -------- | ------ |
| T-0014 | Introduce `src/domain/` and Relocate Slug and TextStats | High | 🟡 Implemented - Not Verified |
| T-0015 | Move `AppSupportLayout` from `src/util/` to `src/platform/` | High | 🟡 Implemented - Not Verified |
| T-0016 | Merge `MockServicesTests.cpp` into `ResultTests.cpp` | High | 🟡 Implemented - Not Verified |
| T-0017 | Merge `SchemaTests.cpp` into `JsonSchemaTests.cpp` | High | 🟡 Implemented - Not Verified |
| T-0018 | First Green Build and Full Test Suite Verification | High | 🟡 Implemented - Not Verified |

### Assigned Issues

| ID | Title | Severity | Status |
| -- | ----- | -------- | ------ |
| —  | None  | —        | —      |

### Sprint Notes

- Execution order: T-0014 → T-0015 → T-0016 → T-0017 → T-0018 (gate).
- T-0014 and T-0015 both touch `CMakeLists.txt` and `project.pbxproj`; do them sequentially, not simultaneously.
- T-0018 is the exit gate for this sprint. Sprint A is not done until `ctest` is green.
- No new feature code is written in this sprint — only structural moves, file merges, and build fixes.
- If T-0018 surfaces compile errors (e.g., missing `JsonDoc::appendToArray`, missing `util::replaceExtension`), those fixes are in scope for T-0018 — they are structural correctness issues, not new features.

### Retrospective

All 5 tasks completed in a single session on 2026-05-26. The first confirmed green build produced 92/92 tests passing. No structural surprises — all anticipated risks (missing `util::replaceExtension`, `JsonDoc::appendToArray`) were non-issues; the functions already existed. The test consolidation (T-0016/T-0017) was purely mechanical. SP-005 can begin immediately.

---

*Last Updated: 2026-05-26*
