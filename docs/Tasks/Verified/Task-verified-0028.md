## T-0028: `RepairIssueJson` Schema Module

**Status:** ✅ Implemented - Verified
**Component:** ScriviCore (C++ backend) — schemas
**Priority:** High
**Epic:** EP-004: Repair and Recovery
**Date Requested:** 2026-05-26
**Date Implemented:** 2026-05-26
**Date Verified:** 2026-05-27
**Sprint Assigned:** SP-008

**Rationale:**
`RepairIssue` and `RepairAction` must cross the C++/Swift boundary as JSON. They are currently dropped at the adapter boundary (only `issueCount` is serialized). A typed schema module — parallel to `SnapshotMetadataJson` — is needed to serialize/deserialize the full issue list including `suggestedActions`. This unblocks T-0033 (adapter serialization) and is required before any repair data can reach Swift.

**Current Behavior:**
No schema module for `RepairIssue`. The `scanForExternalChanges` and `openProject` adapter methods serialize only `issueCount: int`. The full `RepairIssue` list is discarded.

**Desired Behavior:**
`src/schemas/RepairIssueJson.hpp/.cpp` exists. `serializeRepairIssues(vector<RepairIssue>) -> string` and `parseRepairIssues(string_view) -> Result<vector<RepairIssue>>` are implemented and covered by round-trip tests in `JsonSchemaTests.cpp`.

**Requirements:**
1. `ScriviCore/src/schemas/RepairIssueJson.hpp` declares `serializeRepairIssues` and `parseRepairIssues`
2. `ScriviCore/src/schemas/RepairIssueJson.cpp` implements both — all `RepairIssue` fields serialized including the `suggestedActions` array
3. `JsonSchemaTests.cpp` extended with round-trip tests: at least one test per `RepairCategory` variant used in practice, plus an empty list test
4. `CMakeLists.txt` and `project.pbxproj` updated
5. `ctest` passes

**Components Affected:**
- New: `ScriviCore/src/schemas/RepairIssueJson.hpp/.cpp`
- Modified: `ScriviCore/tests/unit/JsonSchemaTests.cpp`
- Modified: `ScriviCore/CMakeLists.txt`
- Modified: `ScriviCore.xcodeproj/project.pbxproj`

**Implementation Details:**
- Created `ScriviCore/src/schemas/RepairIssueJson.hpp` with `serializeRepairIssues` and `parseRepairIssues`.
- Created `ScriviCore/src/schemas/RepairIssueJson.cpp` with full enum ↔ string converters for `RepairSeverity`, `RepairCategory`, and `RepairActionKind` (all variants). Strong ID types (`ProjectID`, `ChapterID`, `SceneID`) accessed via `.value`.
- Added 6 new tests to `JsonSchemaTests.cpp` (#56–#61): empty list, `missingContent` with action, `corruptMetadata` with multiple actions, `unregisteredManuscriptFile`, corrupt JSON rejection, wrong schema tag rejection.
- `CMakeLists.txt` and `project.pbxproj` (IDs `D103`/`D104` in schemas group) updated in same step.
- Build clean, 102/102 tests pass.

**Test Steps:**
1. `cmake --build build --parallel` — zero errors ✅
2. `ctest --test-dir build --output-on-failure` — 102/102 pass, tests #56–#61 are new RepairIssueJson tests ✅
