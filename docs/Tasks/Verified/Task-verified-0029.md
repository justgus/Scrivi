## T-0029: `applyRepair` Facade Method — Request, Result, and Dispatch

**Status:** ✅ Implemented - Verified
**Component:** ScriviCore (C++ backend) — public API
**Priority:** High
**Epic:** EP-004: Repair and Recovery
**Date Requested:** 2026-05-26
**Date Implemented:** 2026-05-26
**Date Verified:** 2026-05-27
**Sprint Assigned:** SP-008

**Rationale:**
The facade has no method for applying a repair action. This task adds `ApplyRepairRequest`, `ApplyRepairResult`, and `ScriviCore::applyRepair()`. The method is stateless: it re-reads project state, validates the issue still applies, and dispatches to the correct handler (implemented in T-0030).

**Current Behavior:**
`ScriviCore` has no `applyRepair` method. Callers can detect issues but cannot act on them through the facade.

**Desired Behavior:**
`ScriviCore::applyRepair(const ApplyRepairRequest&) -> Result<ApplyRepairResult>` exists and routes to handlers. Stubs may be used for unimplemented action kinds during this task; T-0030 fills them in.

**Requirements:**
1. `ApplyRepairRequest` added to `Requests.hpp`: `issueID`, `projectRootPath`, `appSupportRoot`, `actionKind: RepairActionKind`, `targetPath`, `author: AuthorshipRef`
2. `ApplyRepairResult` added to `Results.hpp`: `issueID`, `actionApplied`, `resolved`, `detail`, `warnings: vector<RepairIssue>`
3. `ScriviCore::applyRepair()` added to `ScriviCore.hpp` and `ScriviCore.cpp`
4. Dispatch table routes each `RepairActionKind` to its handler (T-0030 stubs acceptable)
5. Returns `ErrorCode::invalidArgument` for unknown `issueID` or unsupported `actionKind`
6. Build passes

**Components Affected:**
- Modified: `ScriviCore/include/scrivi/Requests.hpp`
- Modified: `ScriviCore/include/scrivi/Results.hpp`
- Modified: `ScriviCore/include/scrivi/ScriviCore.hpp`
- Modified: `ScriviCore/src/public_api/ScriviCore.cpp`

**Implementation Details:**
- `ApplyRepairRequest` added to `Requests.hpp` (includes `RepairIssue.hpp` for `RepairActionKind`): `issueID`, `projectRootPath`, `appSupportRoot`, `actionKind`, `targetPath`, `author`.
- `ApplyRepairResult` added to `Results.hpp`: `issueID`, `actionApplied`, `resolved`, `detail`, `warnings`.
- New `RepairDispatcher` service created at `ScriviCore/src/repair/RepairDispatcher.hpp/.cpp`. Stateless: re-scans project on each call via `ExternalChangeScanner`, validates `issueID` exists in current scan result, verifies `actionKind` is in `suggestedActions`, then dispatches to named stub functions for each of the 16 action kinds.
- `ScriviCore::applyRepair()` added to `ScriviCore.hpp` and wired in `ScriviCore.cpp`.
- `CMakeLists.txt` updated; `project.pbxproj` updated with IDs `A009`/`A010` in repair group.
- Build clean, 102/102 tests pass.

**Test Steps:**
1. `cmake --build build --parallel` — zero errors ✅
2. Dispatch table compiles with stubs for all action kinds ✅
