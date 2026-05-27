## T-0032: Integration Tests for `applyRepair`

**Status:** ✅ Implemented - Verified
**Component:** ScriviCore (C++ backend) — integration tests
**Priority:** High
**Epic:** EP-004: Repair and Recovery
**Date Requested:** 2026-05-26
**Date Implemented:** 2026-05-27
**Date Verified:** 2026-05-27
**Sprint Assigned:** SP-009

**Rationale:**
Each repair action kind needs an integration test that creates a project, induces the failure condition, scans to get the `RepairIssue`, then calls `applyRepair` and verifies the outcome on disk.

**Current Behavior:**
No `applyRepair` integration tests exist.

**Desired Behavior:**
`integration/ApplyRepairTests.cpp` contains ≥1 test per action kind (8 handler tests + 2 rename detection tests = 10 tests minimum). All pass via `ctest`.

**Requirements:**
1. New file `ScriviCore/tests/integration/ApplyRepairTests.cpp`
2. One `TEST_CASE` per action kind, each creating a fresh `TempDir`, inducing the condition, scanning, applying, and verifying
3. `CMakeLists.txt` and `project.pbxproj` updated
4. All existing tests continue to pass

**Components Affected:**
- New: `ScriviCore/tests/integration/ApplyRepairTests.cpp`
- Modified: `ScriviCore/tests/CMakeLists.txt`
- Modified: `ScriviCore.xcodeproj/project.pbxproj`

**Implementation Details:**
- `RepairFixture` helper struct with `scanForIssue(category)`, `checkClean()`, and `makeRepairReq()` helpers.
- 10 tests total (#103–#112): 8 handler tests + 2 rename detection tests.
- Discovered and fixed during test writing: `issueID` never set (always ""); `handleRelinkToFile` reading wrong path; `missingContent` missing `relinkToFile`/`removeFromProject` in suggestedActions.
- `CMakeLists.txt` and `project.pbxproj` (ID `B009`) updated in same step.
- Build clean, 112/112 tests pass.

**Test Steps:**
1. `cmake --build build --parallel` — zero errors ✅
2. `ctest --test-dir build --output-on-failure` — 112/112 pass ✅
