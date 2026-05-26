# Task-verified-0017

## T-0017: Merge `SchemaTests.cpp` into `JsonSchemaTests.cpp`

**Status:** ✅ Implemented - Verified
**Component:** ScriviCore (C++ backend) — test suite
**Priority:** High
**Epic:** EP-007: Skeleton v0.2 Migration
**Date Requested:** 2026-05-22
**Date Implemented:** 2026-05-26
**Date Verified:** 2026-05-26
**Sprint Assigned:** SP-004

**Implementation Details:**
- Appended all schema round-trip tests from `SchemaTests.cpp` into `JsonSchemaTests.cpp`
- Added schema includes and `using namespace scrivi::schemas` to `JsonSchemaTests.cpp`
- Deleted `tests/unit/SchemaTests.cpp`
- Updated `tests/CMakeLists.txt` and `project.pbxproj`
- Unit test directory now contains exactly the six canonical files per skeleton v0.2 §12.1

---

*Last Updated: 2026-05-26*
