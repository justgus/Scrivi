# EP-004: Repair and Recovery

**Status:** ✅ Closed
**Goal:** Implement the full repair apply side: a stateless `applyRepair` facade method that accepts an `issueID + actionKind + context`, dispatches to the appropriate repair handler, and returns a result. Also serialize the full `RepairIssue` list (with `suggestedActions`) into the JSON envelopes for `scanForExternalChanges` and `openProject`, making the repair lifecycle fully callable from Swift. Detection already works (EP-002). This Epic delivers: **Detect → Stage → Apply → Confirm**.
**Date Created:** 2026-05-26
**Start Date:** 2026-05-26
**Target Close Date:** TBD
**Actual Close Date:** 2026-05-27

### Acceptance Criteria

- [x] `RepairIssueJson` schema module serializes/deserializes `RepairIssue` and `RepairAction` lists
- [x] `scanForExternalChanges` JSON result includes full `repairIssues` array (not just `issueCount`)
- [x] `openProject` JSON result includes full `repairIssues` array when mode is `repairRequired`
- [x] `ApplyRepairRequest` / `ApplyRepairResult` types defined in `Requests.hpp` / `Results.hpp`
- [x] `ScriviCore::applyRepair()` facade method dispatches to the correct handler based on `actionKind`
- [x] Repair handlers implemented for all 8 manuscript-level actions: `relinkToFile`, `createEmptyContentFile`, `markMissing`, `removeFromProject`, `moveToInbox`, `reloadExternalVersion`, `regenerateMetadata` (scene), `regenerateMetadata` (chapter)
- [x] Automatic rename detection: `possibleRename` and `possiblePairedRename` classifiers added to `RepairClassifier`; auto-apply when ID match is unambiguous and single candidate exists
- [x] `ScriviCoreAdapter` exposes `applyRepair` with JSON envelope output
- [x] Integration tests cover each action kind (1 test per action minimum)
- [x] `ctest --test-dir build --output-on-failure` passes all tests

### Sprints

| Sprint | Title | Status | Dates |
| ------ | ----- | ------ | ----- |
| SP-008 | Repair and Recovery — Schema and Facade | ✅ Closed | 2026-05-26 – 2026-05-26 |
| SP-009 | Repair and Recovery — Handlers, Tests, and Adapter | ✅ Closed | 2026-05-27 – 2026-05-27 |

### Tasks

| ID     | Title | Status |
| ------ | ----- | ------ |
| T-0028 | `RepairIssueJson` Schema Module | ✅ Verified |
| T-0029 | `applyRepair` Facade Method — Request, Result, and Dispatch | ✅ Verified |
| T-0030 | Repair Handlers — Manuscript File Operations | ✅ Verified |
| T-0031 | Automatic Rename Detection in `RepairClassifier` | ✅ Verified |
| T-0032 | Integration Tests for `applyRepair` | ✅ Verified |
| T-0033 | Adapter: `applyRepair` Method + Full `RepairIssue` Serialization in Scan/Open Results | ✅ Verified |

### Issues

| ID | Title | Status |
| -- | ----- | ------ |
| —  | None  | —      |

### Scope Notes

Drawn from `Scrivi_External_Change_Repair_Matrix_v0_2.md` (20 conditions) and `Scrivi_Backend_Behavior_Spec_v0_2.md` §11. Detection is complete (T-0009/EP-002). This Epic implements the apply side only.

**Out of scope:**
- Asset repair (EP-005 precondition — assets not yet implemented)
- Merge conflict resolution UI
- Comment anchor / object relationship repair
- Encryption-aware repair
- Cloud sync conflict behavior

**Architecture decision:** `applyRepair` is stateless. The caller passes `issueID + projectRootPath + appSupportRoot + actionKind + optional targetPath + author`. `ScriviCore` re-reads project state to validate and execute. No in-memory staged issue list.

### Completion Summary

All 10 acceptance criteria met. Delivered across SP-008 and SP-009:

- Full `RepairIssue` serialization schema (`RepairIssueJson`) with round-trip tests (T-0028)
- Stateless `applyRepair` facade with `RepairDispatcher` routing all 8 manuscript action kinds (T-0029)
- 7 repair handler free functions covering the full manuscript repair matrix; `findChapterForScene` private helper; `.bak` copies before overwrites (T-0030)
- Automatic rename detection in `ExternalChangeScanner` (auto-apply single candidate; stage ambiguous); 4 rename classifiers in `RepairClassifier`; stable SHA-256 `issueID` across all classifiers (T-0031)
- 10 C++ integration tests (#103–#112) covering every implemented action kind (T-0032)
- `ScriviCoreAdapter.applyRepair` + full `repairIssues` array in scan/open JSON envelopes; Swift types `RepairIssueResult`, `RepairActionResult`, `ApplyRepairResult`; 9/9 Swift interop tests pass (T-0033)
- Final test counts: **112/112 C++** · **9/9 Swift**

---

*Closed: 2026-05-27*
