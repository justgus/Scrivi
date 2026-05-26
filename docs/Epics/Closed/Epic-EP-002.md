# Epic EP-002 — Closed

## EP-002: ScriviCore Services

**Status:** ✅ Closed
**Goal:** All MVP service operations implemented and callable from Swift. Delivers the full MVP loop: create local identity/persona, create project, open project, resume last writing surface, save scene, update metadata, restore workspace state, detect external changes, optionally initialize and create Git-backed snapshots.
**Date Created:** 2026-05-19
**Start Date:** 2026-05-20
**Target Close Date:** TBD
**Actual Close Date:** 2026-05-26

### Acceptance Criteria

- [x] Full MVP loop from Section 3 works end-to-end in integration tests
- [x] Swift can call `createProject()`, `openProject()`, `saveScene()` through C++ core
- [x] No backend behavior reimplemented in Swift
- [x] Normal project works without Git (Git is opt-in)
- [x] Cursor and scroll position restored after reopen

### Sprints

| Sprint | Title | Status | Dates |
| ------ | ----- | ------ | ----- |
| SP-002 | ScriviCore Services — Project Lifecycle and Repair | ✅ Closed | 2026-05-20 – 2026-05-20 |
| SP-003 | ScriviCore Swift Interop | ✅ Closed | 2026-05-20 – 2026-05-21 |
| SP-007 | EP-002 Completion — Full MVP Loop Test | ✅ Closed | 2026-05-26 – 2026-05-26 |

### Tasks

| ID     | Title | Status |
| ------ | ----- | ------ |
| T-0006 | Project Creation | ✅ Verified |
| T-0007 | Open/Resume | ✅ Verified |
| T-0008 | Save Scene | ✅ Verified |
| T-0009 | External Change Scan | ✅ Verified |
| T-0010 | Git Snapshots | ✅ Verified |
| T-0011 | Swift Interop Prototype | ✅ Verified |
| T-0027 | Full MVP Loop Integration Test | ✅ Verified |

### Issues

| ID     | Title | Status |
| ------ | ----- | ------ |
| I-0001 | git commit fails on CI runners with no global git identity configured | ✅ Verified |

### Scope Notes

EP-002 began after EP-001 closed. Scope drawn from Section 16, Milestones 6–11 of `Scrivi_Cpp24_Backend_Core_Plan_v0_1.md`.

### Completion Summary

EP-002 delivered the full ScriviCore service layer across three sprints:

- **SP-002** implemented all six core facade operations: `createProject`, `openProject`, `saveScene`, `scanForExternalChanges`, `enableGitSnapshots`, `createSnapshot`. All operations backed by real file I/O through `LocalFileSystem`, with integration tests for each milestone.
- **SP-003** implemented `ensureLocalIdentity` (via EP-003 prerequisites T-0012/T-0013) and proved the Swift/C++ interop boundary works end-to-end through the `ScriviCoreAdapter` and `ScriviEngine` prototype (T-0011).
- **SP-007** added `MvpLoopTests.cpp` — a single integration test (test #96) chaining all 13 Section 3 steps in sequence, making criterion 1 unambiguous. 96/96 tests pass.

Final state: all 5 acceptance criteria met. All 7 tasks verified. 3 sprints closed.

---

*Closed: 2026-05-26*
