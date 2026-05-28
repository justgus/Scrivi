# SP-011: Objects Layer — Remaining Types and Generalized CRUD

**Status:** ✅ Closed
**Epic:** EP-005: Full Project Package — Objects, Assets, and Comments
**Goal:** Extend the object layer from character-only to all five object types (location, item, rule, timeline). Define the `ObjectKind` discriminant, generalize the CRUD facade to route by type, and cover all types with integration tests.
**Start Date:** 2026-05-28
**End Date:** 2026-05-28
**Capacity:** Single session

### Assigned Tasks

| ID     | Title | Priority | Status |
| ------ | ----- | -------- | ------ |
| T-0037 | Remaining Object Types — Location, Item, Rule, Timeline Schemas | High | ✅ Implemented - Verified |
| T-0038 | Generalize Object CRUD Facade to All Object Types | High | ✅ Implemented - Verified |
| T-0039 | Integration Tests for All Object Types CRUD | High | ✅ Implemented - Verified |

### Assigned Issues

| ID | Title | Severity | Status |
| -- | ----- | -------- | ------ |
| —  | None  | —        | —      |

### Retrospective

**Completed:**
- T-0037: Four new object type structs + `ObjectKind` enum + `WorldObject` variant + schema serialize/parse for all five types
- T-0038: Generalized CRUD facade routing by kind; `ObjectStore` dispatches to correct subdirectory and schema
- T-0039: Full CRUD integration tests for location, item, rule, and timeline objects; 132 tests passing (up from 121)

**Returned to Backlog:**
- None

**What went well:**
- Shared `serializeFields`/`parseFields` helper kept ObjectJson.cpp DRY; all five types reuse identical logic
- Template `runCrudCycle<T>` in the test file avoids copy-paste across four type tests

**What to improve:**
- `ObjectCrudTests.cpp` was missing from `tests/CMakeLists.txt` — catch this at file creation time, not build time

**Carry-forward notes:**
- SP-012 (Assets and Comments) depends on `WorldObject` variant being stable; no further changes to the variant are expected

---

*Closed: 2026-05-28*
