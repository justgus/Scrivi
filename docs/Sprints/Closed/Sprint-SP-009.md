# Sprint SP-009: Repair and Recovery — Handlers, Tests, and Adapter

**Status:** ✅ Closed
**Epic:** EP-004: Repair and Recovery
**Goal:** Implement all 8 manuscript repair handlers (T-0030), automatic rename detection in `RepairClassifier` (T-0031), a full integration test suite for `applyRepair` (T-0032), and the adapter method + full `RepairIssue` serialization in scan/open results (T-0033). After this sprint, EP-004 acceptance criteria are fully met and the repair lifecycle is callable end-to-end from Swift.
**Start Date:** 2026-05-27
**End Date:** 2026-05-27

### Assigned Tasks

| ID     | Title | Priority | Status |
| ------ | ----- | -------- | ------ |
| T-0030 | Repair Handlers — Manuscript File Operations | High | ✅ Implemented - Verified |
| T-0031 | Automatic Rename Detection in `RepairClassifier` | Medium | ✅ Implemented - Verified |
| T-0032 | Integration Tests for `applyRepair` | High | ✅ Implemented - Verified |
| T-0033 | Adapter — `applyRepair` Method + Full `RepairIssue` Serialization in Scan/Open Results | High | ✅ Implemented - Verified |

### Assigned Issues

| ID | Title | Severity | Status |
| -- | ----- | -------- | ------ |
| —  | None  | —        | —      |

### Sprint Notes

- Depends on SP-008 complete ✅ (T-0028/T-0029 verified 2026-05-27).
- Execution order: T-0030 → T-0031 → T-0032 → T-0033.
- T-0031 parallelized with T-0030 (different files: `RepairClassifier` vs `RepairHandlers`).
- T-0032 was the acceptance gate — all action kinds covered by ≥1 integration test.
- T-0033 completed the EP-004 adapter surface, making the full repair lifecycle callable from Swift.

### Retrospective

**Completed:**
- T-0030: 7 handler free functions (`relinkToFile`, `createEmptyContentFile`, `markMissing`, `removeFromProject`, `moveToInbox`, `reloadExternalVersion`, `regenerateMetadata` scene + chapter). `findChapterForScene` private helper walks manuscript dir. `.bak` copies written before overwrites.
- T-0031: 4 rename classifiers added to `RepairClassifier` (`possibleRename`, `possibleMetadataRename`, `possiblePairedRename`, `possibleChapterFolderRename`). Stable `issueID` via SHA-256 hash added to all 8 classifiers. `ExternalChangeScanner` rewritten to auto-apply unambiguous metadata renames; stage `possibleMetadataRename` for multiple candidates; detect chapter folder renames.
- T-0032: 10 integration tests in `ApplyRepairTests.cpp` (#103–#112), one per action kind plus 2 rename detection tests. 112/112 C++ tests passing.
- T-0033: `applyRepair` added to `ScriviCoreAdapter`. `scanForExternalChanges` upgraded from `issueCount: int` to `repairIssues: [...]` array. `openProject` now returns ok envelope for `repairRequired` mode with full issues list. New Swift types: `RepairIssueResult`, `RepairActionResult`, `ApplyRepairResult`. New Swift interop test (Test 9): full end-to-end `applyRepair` round-trip. 9/9 Swift tests passing.

**Returned to Backlog:**
- None.

**What went well:**
- Deterministic `issueID` via SHA-256 was the right call — it made the dispatcher re-scan validation work cleanly across all handler tests without any test plumbing.
- `findChapterForScene` as a shared private helper kept all four chapter-mutating handlers (`relinkToFile`, `markMissing`, `removeFromProject`, `regenerateMetadata`) lean.
- Splitting `appendRepairIssuesToDoc` into `RepairIssueJson` kept the adapter clean and avoided re-parsing the standalone JSON string.

**What to improve:**
- `issue.path` semantics differ by category (`missingContent` → content path; `missingMetadata` → meta path), which caused a build error in `handleRelinkToFile` that required a fix mid-sprint. Consider documenting this in `RepairIssue.hpp` as a field-level comment.
- `openProject` returning `repairRequired` as ok (not error) changed test 5 semantics — a previously passing test needed rewriting. The new behavior is more correct, but the change was not anticipated upfront.

**Carry-forward notes:**
- EP-004 all 10 acceptance criteria met. EP-004 ready to close when user approves.
- Remaining unimplemented `applyRepair` action kinds (`importAsNewScene`, `attachToExistingScene`, `restoreFromSnapshot`, `ignore`, `deleteAfterConfirmation`, `openReadOnly`, `cancelOpen`, `keepCurrentVersion`, `saveCurrentVersionAsCopy`) return `internalError / "not yet implemented"`. These are out of scope for EP-004.
- EP-005 (Full Project Package — Objects, Assets, and Comments) is next.

---

*Closed: 2026-05-27*
