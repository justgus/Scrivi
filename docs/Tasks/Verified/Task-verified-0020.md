# Task-verified-0020

## T-0020: Add `SnapshotMetadataJson` Test Coverage

**Status:** ✅ Implemented - Verified
**Component:** ScriviCore (C++ backend) — test suite
**Priority:** Medium
**Epic:** EP-007: Skeleton v0.2 Migration
**Date Requested:** 2026-05-22
**Date Implemented:** 2026-05-26
**Date Verified:** 2026-05-26
**Sprint Assigned:** SP-005

**Implementation Details:**
- Added 3 tests to `tests/unit/JsonSchemaTests.cpp`:
  - `SnapshotMetadataJson round-trips with one entry` (test 53)
  - `SnapshotMetadataJson round-trips empty snapshots array` (test 54)
  - `SnapshotMetadataJson rejects corrupt JSON` (test 55)
- All 3 pass in `ctest`

---

*Last Updated: 2026-05-26*
