# SP-005: Skeleton v0.2 Migration — Milestone 10 and 11 Verification

**Status:** 🟡 Active
**Epic:** EP-004: Skeleton v0.2 Migration
**Goal:** Verify Milestones 10 (External Change Scan) and 11 (Git Snapshots) are functionally correct. Add the missing `SnapshotMetadataJson` schema module. All integration tests for both milestones pass green.
**Start Date:** 2026-05-26
**End Date:** 2026-05-26
**Capacity:** TBD

### Assigned Tasks

| ID     | Title | Priority | Status |
| ------ | ----- | -------- | ------ |
| T-0019 | Add `SnapshotMetadataJson` Schema | High | 🟡 Implemented - Not Verified |
| T-0020 | Add `SnapshotMetadataJson` Test Coverage | Medium | 🟡 Implemented - Not Verified |
| T-0021 | Milestone 10 Verification — External Change Scan | High | 🟡 Implemented - Not Verified |
| T-0022 | Milestone 11 Verification — Git Snapshots | High | 🟡 Implemented - Not Verified |

### Assigned Issues

| ID | Title | Severity | Status |
| -- | ----- | -------- | ------ |
| —  | None  | —        | —      |

### Sprint Notes

- Depends on SP-004 (first green build). Do not start this sprint until SP-004 is verified closed.
- Execution order: T-0019 first (schema module), then T-0020 (schema tests), then T-0021 and T-0022 (integration verification).
- T-0021 and T-0022 can be worked in parallel once T-0019 is done.
- Key risks: `util::replaceExtension` existence (T-0021), `JsonDoc::appendToArray`/`arraySize`/`arrayItem` existence (T-0022). If either is missing, the fix goes into that task — not a new task.
- Real-git tests in `GitSnapshotTests.cpp` must skip gracefully if `git` is not on PATH. This is already coded with `if (!SystemGitProvider::available()) { SKIP(...);}`.
- Sprint ends when `ctest --output-on-failure` is fully green, including all integration tests.

### Retrospective

All 4 tasks completed on 2026-05-26. T-0021 and T-0022 were already green from the SP-004 build — no fixes required. T-0019 added `SnapshotMetadataJson.hpp/.cpp` following the exact schema module pattern; `SnapshotService` was refactored to use it. T-0020 added 3 round-trip/error tests. Final run: 95/95 tests passed.

---

*Last Updated: 2026-05-26*
