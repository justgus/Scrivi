# Epic EP-007 — Closed

## EP-007: Skeleton v0.2 Migration

**Status:** ✅ Closed
**Goal:** Migrate the ScriviCore repository to exact conformance with the approved Skeleton v0.2 layout, complete the first verified green build, verify all Milestone 10 and 11 integration tests, and extend the Apple platform wrapper to cover all seven ScriviCore facade operations.
**Date Created:** 2026-05-22
**Start Date:** 2026-05-22
**Target Close Date:** TBD
**Actual Close Date:** 2026-05-26

### Acceptance Criteria

- [x] `src/domain/` exists with `Slug`, `TextStats`, and `IDs` per skeleton v0.2 §6
- [x] `AppSupportLayout` resides in `src/platform/` per skeleton v0.2 §6
- [x] Unit test directory contains exactly the six canonical files from skeleton v0.2 §12.1
- [x] `cmake --build build --parallel && ctest --test-dir build --output-on-failure` exits 0 — all tests green
- [x] `SnapshotMetadataJson.hpp/.cpp` schema module exists and is covered by `JsonSchemaTests`
- [x] All 6 `ExternalChangeTests` integration tests pass
- [x] All `GitSnapshotTests` mock-based tests pass; real-git tests pass or skip gracefully
- [x] `ScriviError.swift` is a separate file per skeleton v0.2 §7
- [x] `ScriviCoreAdapter` exposes all 7 facade methods
- [x] `ScriviEngine.swift` exposes all 7 facade methods with Swift result types
- [x] `swift test` from `platforms/apple/` passes all 8 interop tests

### Sprints

| Sprint | Title | Status | Dates |
| ------ | ----- | ------ | ----- |
| SP-004 | Skeleton v0.2 Migration — Structural Alignment | ✅ Closed | 2026-05-26 – 2026-05-26 |
| SP-005 | Skeleton v0.2 Migration — Milestone 10 and 11 Verification | ✅ Closed | 2026-05-26 – 2026-05-26 |
| SP-006 | Skeleton v0.2 Migration — Adapter and Swift Completion | ✅ Closed | 2026-05-26 – 2026-05-26 |

### Tasks

| ID     | Title | Status |
| ------ | ----- | ------ |
| T-0014 | Introduce `src/domain/` and Relocate Slug and TextStats | ✅ Verified |
| T-0015 | Move `AppSupportLayout` from `src/util/` to `src/platform/` | ✅ Verified |
| T-0016 | Merge `MockServicesTests.cpp` into `ResultTests.cpp` | ✅ Verified |
| T-0017 | Merge `SchemaTests.cpp` into `JsonSchemaTests.cpp` | ✅ Verified |
| T-0018 | First Green Build and Full Test Suite Verification | ✅ Verified |
| T-0019 | Add `SnapshotMetadataJson` Schema | ✅ Verified |
| T-0020 | Add `SnapshotMetadataJson` Test Coverage | ✅ Verified |
| T-0021 | Milestone 10 Verification — External Change Scan | ✅ Verified |
| T-0022 | Milestone 11 Verification — Git Snapshots | ✅ Verified |
| T-0023 | Extract `ScriviError.swift` as a Separate File | ✅ Verified |
| T-0024 | Add Adapter Methods for `scanForExternalChanges`, `enableGitSnapshots`, `createSnapshot` | ✅ Verified |
| T-0025 | Add Swift Engine Methods for `scanForExternalChanges`, `enableGitSnapshots`, `createSnapshot` | ✅ Verified |
| T-0026 | Add Swift Interop Tests for New Adapter Methods | ✅ Verified |

### Issues

| ID | Title | Status |
| -- | ----- | ------ |
| —  | None  | —      |

### Scope Notes

This Epic was a structural and verification milestone, not a feature milestone. No new user-facing behavior was added. The work was: (1) align the repository structure to the approved skeleton, (2) reach the first confirmed green build, (3) verify all previously-written but unrun integration tests, (4) complete the Apple platform wrapper surface.

EP-007 was a prerequisite for closing EP-002 and EP-003, because the Swift interop verification (T-0011's actual acceptance criteria) depended on the adapter being complete.

### Completion Summary

EP-007 delivered full conformance to the Skeleton v0.2 layout across three sprints:

- **SP-004** relocated `src/domain/` (Slug, TextStats, IDs), moved `AppSupportLayout` to `src/platform/`, merged redundant unit test files, and achieved the first confirmed green build (95/95 tests).
- **SP-005** added the `SnapshotMetadataJson` schema module, verified all 6 ExternalChange integration tests and all GitSnapshot tests (mock + real-git skip-guarded).
- **SP-006** completed the Apple platform wrapper: extracted `ScriviError.swift`, added the three missing adapter methods (`scanForExternalChanges`, `enableGitSnapshots`, `createSnapshot`), wired `SystemGitProvider` into the adapter `Impl`, added corresponding Swift engine methods and result types, and added 3 new interop tests. Also fixed a pre-existing `openProject` bug where blocking repair issues returned `ok:true` with an incomplete result struct.

Final state: all 11 acceptance criteria met. 13 tasks verified. 3 sprints closed. All 8 Swift interop tests pass. `swift test` clean.

---

*Closed: 2026-05-26*
