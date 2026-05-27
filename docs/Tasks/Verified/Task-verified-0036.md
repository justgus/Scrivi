## T-0036: Integration Tests for Character Object CRUD

**Status:** ✅ Implemented - Verified
**Component:** ScriviCore (C++ backend) — integration tests
**Priority:** High
**Epic:** EP-005: Full Project Package — Objects, Assets, and Comments
**Date Requested:** 2026-05-27
**Date Implemented:** 2026-05-27
**Date Verified:** 2026-05-27
**Sprint Assigned:** SP-010

**Rationale:**
The character CRUD facade (T-0035) needs integration tests that exercise the full create → open → save → delete lifecycle against a real temp directory, proving the on-disk format is correct.

**Current Behavior:**
No object layer integration tests exist.

**Desired Behavior:**
`tests/integration/ObjectCrudTests.cpp` contains ≥5 test cases tagged `[integration][EP-005][T-0036]` covering the full CRUD lifecycle.

**Requirements:**
1. `tests/integration/ObjectCrudTests.cpp` created with ≥5 test cases
2. `tests/CMakeLists.txt` updated to include `integration/ObjectCrudTests.cpp`
3. `ScriviCore.xcodeproj/project.pbxproj` updated
4. `ctest` passes all tests

**Components Affected:**
- New: `ScriviCore/tests/integration/ObjectCrudTests.cpp`
- Modified: `ScriviCore/tests/CMakeLists.txt`
- Modified: `ScriviCore.xcodeproj/project.pbxproj`

**Implementation Details:**
- `ObjectFixture` struct: creates real temp dirs, bootstraps a project via `createProject`, provides `makeCreateReq` helper.
- 6 test cases:
  1. `createObject` succeeds; file at `objects/characters/ada-thornwood.json`; slug derived from displayName
  2. Explicit slug override produces correct filename
  3. `openObject` returns correct displayName, slug, status "active", objectID
  4. `saveObject` updates displayName/notes/tags; re-open confirms all three fields
  5. `deleteObject` removes file; subsequent `openObject` returns failure
  6. Duplicate slug → `ErrorCode::invalidArgument`
- `project.pbxproj` updated with E006 in integration group.
- 121/121 tests pass.

**Test Steps:**
1. `cmake --build build --parallel` — zero errors ✅
2. `ctest --test-dir build --output-on-failure` — 121/121 pass (115 prior + 6 new) ✅
