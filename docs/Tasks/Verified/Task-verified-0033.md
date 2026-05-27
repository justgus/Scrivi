## T-0033: Adapter — `applyRepair` Method + Full `RepairIssue` Serialization in Scan/Open Results

**Status:** ✅ Implemented - Verified
**Component:** ScriviCoreAdapter (C++) / Apple platform wrapper
**Priority:** High
**Epic:** EP-004: Repair and Recovery
**Date Requested:** 2026-05-26
**Date Implemented:** 2026-05-27
**Date Verified:** 2026-05-27
**Sprint Assigned:** SP-009

**Rationale:**
For `applyRepair` to be callable from Swift, two things are needed: (1) the adapter must expose `applyRepair`, and (2) the `scanForExternalChanges` and `openProject` adapter methods must serialize the full `RepairIssue` list (including `issueID` and `suggestedActions`) so the caller has the `issueID` to pass back. Currently both adapters drop the issue list and emit only `issueCount`.

**Current Behavior:**
`ScriviCoreAdapter` has no `applyRepair` method. `scanForExternalChanges` and `openProject` JSON results contain `issueCount: int` only.

**Desired Behavior:**
`ScriviCoreAdapter` exposes `applyRepair` with JSON envelope output. `scanForExternalChanges` and `openProject` JSON results include a `repairIssues` array. `swift test` passes all 9 interop tests.

**Requirements:**
1. `ScriviCoreAdapter.hpp` declares `applyRepair` ✅
2. `ScriviCoreAdapter.cpp` implements it, using `RepairIssueJson` for serialization ✅
3. `scanForExternalChanges` adapter method updated: `repairIssues` array in JSON (replaces `issueCount`) ✅
4. `openProject` adapter method updated: `repairIssues` array in JSON when mode is `repairRequired` ✅
5. `swift build` passes ✅
6. `swift test` — all 9 interop tests pass ✅
7. `ScanResult` Swift struct updated: `repairIssues: [RepairIssueResult]` replaces `issueCount: Int` ✅

**Components Affected:**
- Modified: `platforms/apple/Sources/ScriviCoreAdapter/ScriviCoreAdapter.hpp`
- Modified: `platforms/apple/Sources/ScriviCoreAdapter/ScriviCoreAdapter.cpp`
- Modified: `ScriviCore/src/schemas/RepairIssueJson.hpp` — added `appendRepairIssuesToDoc`
- Modified: `ScriviCore/src/schemas/RepairIssueJson.cpp` — added `appendRepairIssuesToDoc`
- Modified: `platforms/apple/Sources/Scrivi/ScriviEngine.swift`
- Modified: `platforms/apple/Tests/ScriviInteropTests/ScriviInteropTests.swift`

**Implementation Details:**
- Added `appendRepairIssuesToDoc(doc, key, issues)` to `RepairIssueJson` — avoids re-parsing; shares enum-to-string helpers.
- `openProject` adapter returns ok envelope for `repairRequired` mode; only `cannotOpen` returns error envelope.
- `activeScene` in `OpenProjectResult` changed to `Optional` in Swift.
- `RepairIssueResult.suggestedActions` decoded with `decodeIfPresent` (issues from `ProjectValidator` have no suggestedActions).
- Test 9 added: creates project, deletes content file, scans, calls `applyRepair(actionKind: "createEmptyContentFile")`, asserts `resolved==true` and file exists on disk.

**Test Steps:**
1. `ctest --test-dir build --output-on-failure` — 112/112 passed ✅
2. `swift build` — zero errors ✅
3. `swift test` — 9/9 passed ✅
