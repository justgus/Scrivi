## T-0037: Remaining Object Types — Location, Item, Rule, Timeline Schemas

**Status:** ✅ Implemented - Verified
**Component:** ScriviCore — ObjectTypes.hpp, ObjectJson
**Priority:** High
**Date Requested:** 2026-05-28
**Date Implemented:** 2026-05-28
**Date Verified:** 2026-05-28
**Sprint Assigned:** SP-011

**Rationale:**
EP-005 requires all five object type schemas. Only `CharacterObject` was implemented.

**Implementation Details:**
- `ObjectTypes.hpp`: Added `ObjectKind` enum (`character`, `location`, `item`, `rule`, `timeline`), `WorldObjectFields` base struct holding all shared fields, four new typed structs (`LocationObject`, `ItemObject`, `RuleObject`, `TimelineObject`) each inheriting from `WorldObjectFields`, `WorldObject` variant (`std::variant<CharacterObject, LocationObject, ItemObject, RuleObject, TimelineObject>`), `objectKindSubdir()` inline helper, `worldObjectFields()` const accessor.
- `ObjectJson.hpp`: Added `serializeLocation`/`parseLocation`, `serializeItem`/`parseItem`, `serializeRule`/`parseRule`, `serializeTimeline`/`parseTimeline`, `serializeWorldObject`/`parseWorldObject`.
- `ObjectJson.cpp`: Refactored to shared `serializeFields`/`parseFields` helpers; each concrete function calls the helper with its schema tag constant. `serializeWorldObject` dispatches via `std::get_if`; `parseWorldObject` switches on `ObjectKind`.
- `JsonSchemaTests.cpp`: Round-trip tests for all four new types, cross-type schema-rejection test, `serializeWorldObject`/`parseWorldObject` variant round-trip for all five kinds.

**Test Results:**
- 132/132 tests passing (was 121 before SP-011)
- New schema tests: 65–70
