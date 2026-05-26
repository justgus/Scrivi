# Task-verified-0027

## T-0027: Full MVP Loop Integration Test

**Status:** ✅ Implemented - Verified
**Component:** ScriviCore (C++ backend) — integration tests
**Priority:** High
**Epic:** EP-002: ScriviCore Services
**Date Requested:** 2026-05-26
**Date Implemented:** 2026-05-26
**Date Verified:** 2026-05-26
**Sprint Assigned:** SP-007

**Rationale:**
EP-002 acceptance criterion 1 requires the full MVP loop from Section 3 to work end-to-end in integration tests. All 13 steps were individually covered across prior tasks, but no single test chained them in sequence. This task adds that test, making the criterion unambiguous.

**Implementation Details:**
- Created `ScriviCore/tests/integration/MvpLoopTests.cpp` with one `TEST_CASE` tagged `[integration][EP-002][T-0027]`.
- Chains all 13 Section 3 steps: identity → createProject → openProject → saveScene → reopen + restore → scanForExternalChanges → missing-file detection (repairRequired mode) → enableGitSnapshots → createSnapshot.
- Uses `MockSecureStore`, `DeterministicUUIDProvider`, `MockGitProvider`, `FixedClock`, `LocalFileSystem`.
- `CMakeLists.txt` and `project.pbxproj` updated in same step.
- 96/96 tests pass (test #96 is the new MVP loop test).

**Components Affected:**
- New: `ScriviCore/tests/integration/MvpLoopTests.cpp`
- Modified: `ScriviCore/tests/CMakeLists.txt`
- Modified: `ScriviCore.xcodeproj/project.pbxproj`

**Test Steps:**
1. `cmake --build build --parallel` — zero errors ✅
2. `ctest --test-dir build --output-on-failure` — 96/96 pass ✅
