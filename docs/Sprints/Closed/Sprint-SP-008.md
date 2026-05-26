# SP-008: Repair and Recovery — Schema and Facade

**Status:** ✅ Closed
**Epic:** EP-004: Repair and Recovery
**Goal:** Implement the serialization foundation and facade entry point for the repair lifecycle. Deliver `RepairIssueJson` schema module (T-0028) and `ScriviCore::applyRepair()` with full request/result types and a dispatch table (T-0029).
**Start Date:** 2026-05-26
**End Date:** 2026-05-26

### Assigned Tasks

| ID     | Title | Priority | Status |
| ------ | ----- | -------- | ------ |
| T-0028 | `RepairIssueJson` Schema Module | High | ✅ Verified |
| T-0029 | `applyRepair` Facade Method — Request, Result, and Dispatch | High | ✅ Verified |

### Assigned Issues

| ID | Title | Severity | Status |
| -- | ----- | -------- | ------ |
| —  | None  | —        | —      |

### Sprint Notes

- Execution order: T-0028 (schema module) → T-0029 (facade + dispatch table).
- T-0028 must land before T-0029 because `ApplyRepairResult` references `RepairIssue` in its `warnings` field.
- T-0029 uses stub handler functions (returning `internalError / "not yet implemented"`) for all action kinds; T-0030 fills them in during SP-009.
- SP-009 depends on this sprint.

### Retrospective

Both tasks implemented in one session. Key notes:
- **T-0028**: `RepairIssueJson.hpp/.cpp` added to `src/schemas/`. Enum ↔ string converters for all three enums (`RepairSeverity`, `RepairCategory`, `RepairActionKind`). Strong ID types (`ProjectID`, `ChapterID`, `SceneID`) require `.value` for string access — initial draft used direct assignment and caught a compile error. Fixed. 6 new schema tests (tests #56–#61) all pass.
- **T-0029**: `ApplyRepairRequest` and `ApplyRepairResult` added to `Requests.hpp` / `Results.hpp`. `RepairDispatcher` service created in `src/repair/`. Dispatcher re-scans project on every call (stateless), validates `issueID` and `actionKind`, then routes to named stub handlers. `ScriviCore::applyRepair()` wired into facade and `ScriviCore.cpp`. `CMakeLists.txt` and `project.pbxproj` updated in same step.
- Build clean, 102/102 tests pass.

---

*Closed: 2026-05-26*
