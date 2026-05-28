## T-0039: Integration Tests for All Object Types CRUD

**Status:** ✅ Implemented - Verified
**Component:** ScriviCore — tests/integration
**Priority:** High
**Date Requested:** 2026-05-28
**Date Implemented:** 2026-05-28
**Date Verified:** 2026-05-28
**Sprint Assigned:** SP-011

**Rationale:**
EP-005 requires integration tests for all object types. T-0036 covered characters only.

**Implementation Details:**
- `ObjectCrudTests.cpp`: Updated existing character tests to use `objectKind` field and `std::get<CharacterObject>()` on `OpenObjectResult.object`. Added `runCrudCycle<T>` template that exercises create/open/save/delete for any object type. New test cases: `LocationObject` full CRUD, `ItemObject` full CRUD, `RuleObject` full CRUD, `TimelineObject` full CRUD, cross-kind slug independence (same slug in `characters/` and `locations/` does not conflict).
- `tests/CMakeLists.txt`: Added `integration/ObjectCrudTests.cpp` to `ScriviCoreTests` target (was previously missing).

**Test Results:**
- 132/132 tests passing (was 121 before SP-011)
- New integration tests: 128–132
