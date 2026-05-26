# Active Epics

---

## EP-004: Repair and Recovery

**Status:** 🟡 Active
**Goal:** Implement the full repair apply side: a stateless `applyRepair` facade method that accepts an `issueID + actionKind + context`, dispatches to the appropriate repair handler, and returns a result. Also serialize the full `RepairIssue` list (with `suggestedActions`) into the JSON envelopes for `scanForExternalChanges` and `openProject`, making the repair lifecycle fully callable from Swift. Detection already works (EP-002). This Epic delivers: **Detect → Stage → Apply → Confirm**.
**Date Created:** 2026-05-26
**Start Date:** 2026-05-26
**Target Close Date:** TBD
**Actual Close Date:** —

### Acceptance Criteria

- [ ] `RepairIssueJson` schema module serializes/deserializes `RepairIssue` and `RepairAction` lists
- [ ] `scanForExternalChanges` JSON result includes full `repairIssues` array (not just `issueCount`)
- [ ] `openProject` JSON result includes full `repairIssues` array when mode is `repairRequired`
- [ ] `ApplyRepairRequest` / `ApplyRepairResult` types defined in `Requests.hpp` / `Results.hpp`
- [ ] `ScriviCore::applyRepair()` facade method dispatches to the correct handler based on `actionKind`
- [ ] Repair handlers implemented for all 8 manuscript-level actions: `relinkToFile`, `createEmptyContentFile`, `markMissing`, `removeFromProject`, `moveToInbox`, `reloadExternalVersion`, `regenerateMetadata` (scene), `regenerateMetadata` (chapter)
- [ ] Automatic rename detection: `possibleRename` and `possiblePairedRename` classifiers added to `RepairClassifier`; auto-apply when ID match is unambiguous and single candidate exists
- [ ] `ScriviCoreAdapter` exposes `applyRepair` with JSON envelope output
- [ ] Integration tests cover each action kind (1 test per action minimum)
- [ ] `ctest --test-dir build --output-on-failure` passes all tests

### Sprints

| Sprint | Title | Status | Dates |
| ------ | ----- | ------ | ----- |
| SP-008 | Repair and Recovery — Schema and Facade | 🔵 Planning | TBD |
| SP-009 | Repair and Recovery — Handlers and Tests | 🔵 Planning | TBD |

### Tasks

| ID     | Title | Status |
| ------ | ----- | ------ |
| T-0028 | `RepairIssueJson` Schema Module | 🔵 Backlog |
| T-0029 | `applyRepair` Facade Method — Request, Result, and Dispatch | 🔵 Backlog |
| T-0030 | Repair Handlers — Manuscript File Operations | 🔵 Backlog |
| T-0031 | Automatic Rename Detection in `RepairClassifier` | 🔵 Backlog |
| T-0032 | Integration Tests for `applyRepair` | 🔵 Backlog |
| T-0033 | Adapter: `applyRepair` Method + Full `RepairIssue` Serialization in Scan/Open Results | 🔵 Backlog |

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

*To be filled in when EP-004 reaches 🟠 Complete.*

---

*Last Updated: 2026-05-26 (EP-004 activated)*
