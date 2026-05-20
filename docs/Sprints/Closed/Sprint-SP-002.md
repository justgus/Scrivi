# Sprint SP-002 — Closed

## SP-002: ScriviCore Services — Project Lifecycle and Repair

**Status:** ✅ Closed
**Epic:** EP-002: ScriviCore Services
**Goal:** Implement the full C++ service layer for the MVP project lifecycle: create, open, save, scan for external changes, and Git snapshots. At the end of this Sprint, all five service operations are implemented and pass integration tests against real temporary directories.
**Start Date:** 2026-05-20
**End Date:** TBD
**Actual Close Date:** 2026-05-20
**Capacity:** TBD

### Assigned Tasks

| ID     | Title | Priority | Status |
| ------ | ----- | -------- | ------ |
| T-0006 | Project Creation | High | ✅ Verified |
| T-0007 | Open/Resume | High | ✅ Verified |
| T-0008 | Save Scene | High | ✅ Verified |
| T-0009 | External Change Scan | High | ✅ Verified |
| T-0010 | Git Snapshots | High | ✅ Verified |

### Assigned Issues

| ID     | Title | Severity | Status |
| ------ | ----- | -------- | ------ |
| I-0001 | git commit fails on CI runners with no global git identity configured | High | ✅ Verified |

### Sprint Notes

- T-0006 (Project Creation) was a prerequisite for T-0007, T-0008, and T-0009 — completed first.
- T-0007, T-0008, and T-0009 were worked in parallel once T-0006 was in place.
- T-0010 (Git Snapshots) depended on T-0006 for the project creation path.
- Success measured by integration tests passing against real temp directories for all five operations.
- T-0011 (Swift Interop) intentionally deferred to SP-003 to de-risk the toolchain boundary work.

### Retrospective

**Completed:**
- T-0006: Project Creation — `ProjectCreator` writes all 7 required files; Git path covered; 6/6 tests passing (Tests 53–58)
- T-0007: Open/Resume — `ProjectOpener`, `ProjectValidator`, `ManuscriptOrderResolver`, `SceneReader`, `WorkspaceStateService` (read); workspace state restored after reopen; 7/7 tests passing (Tests 59–65)
- T-0008: Save Scene — `SceneWriter` with atomic write and idempotent hash check; cursor/scroll restored after save+reopen; 5/5 tests passing (Tests 66–70)
- T-0009: External Change Scan — `ExternalChangeScanner`, `RepairClassifier`; detects all four damage categories; 6/6 tests passing (Tests 71–76)
- T-0010: Git Snapshots — `SystemGitProvider`, `SnapshotService`, `Process` utility; mock and real-git paths covered; 6/6 tests passing (Tests 77–82)
- I-0001: CI fix — `git commit` on machines with no global git identity; resolved by passing `-c user.name` and `-c user.email` per-invocation

**Returned to Backlog:**
- None — all 5 tasks and 1 issue completed and verified

**What went well:**
- All five service operations implemented and verified in a single sprint
- Integration tests run against real temp directories — no over-reliance on mocks
- Real-git tests skip gracefully via `SKIP()` when git is unavailable
- Atomic write prevents partial writes on crash (T-0003 foundation paid off)
- Idempotent save (hash check) correctly skips content write while still updating workspace state

**What to improve:**
- Xcode project file (pbxproj) must be updated in the same step as adding source files — this was discovered mid-sprint and added to CLAUDE.md as a hard rule
- CI environment assumptions (git identity) surfaced a gap; committer identity should always be self-contained in the commit call

**Carry-forward notes:**
- EP-002 still open; T-0011 (Swift Interop Prototype) moves to SP-003
- All 82 tests passing; full integration test suite covers the MVP project lifecycle
- `WorkspaceStateService` write path (T-0008) is shared with the read path (T-0007) — single implementation in `src/workspace/`

---

*Closed: 2026-05-20*
