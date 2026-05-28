## T-0038: Generalize Object CRUD Facade to All Object Types

**Status:** ✅ Implemented - Verified
**Component:** ScriviCore — Requests.hpp, Results.hpp, ObjectStore, ScriviCore facade
**Priority:** High
**Date Requested:** 2026-05-28
**Date Implemented:** 2026-05-28
**Date Verified:** 2026-05-28
**Sprint Assigned:** SP-011

**Rationale:**
The CRUD facade was hardcoded to `CharacterObject`. EP-005 requires it to work for all five object types via an `ObjectKind` discriminant.

**Implementation Details:**
- `Requests.hpp`: Added `objectKind` field (default `ObjectKind::character`) to `CreateObjectRequest`, `OpenObjectRequest`, `DeleteObjectRequest`. Changed `SaveObjectRequest.object` from `CharacterObject` to `WorldObject`.
- `Results.hpp`: Changed `OpenObjectResult.object` from `CharacterObject` to `WorldObject`.
- `ObjectStore.hpp`: Replaced `charsDir`/`findByID(id)` with `kindDir(projectRoot, kind)` and `findByID(projectRoot, kind, id)`.
- `ObjectStore.cpp`: All four methods dispatch by `objectKind` to `objects/<kind>/` subdirectory and the corresponding schema serialize/parse function. `save` uses `std::visit` with a generic lambda to mutate modification timestamps on the concrete alternative.

**Test Results:**
- 132/132 tests passing
- All pre-existing character CRUD integration tests (122–127) pass unchanged
