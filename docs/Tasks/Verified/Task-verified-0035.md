## T-0035: `createObject` / `openObject` / `saveObject` / `deleteObject` Facade — Characters

**Status:** ✅ Implemented - Verified
**Component:** ScriviCore (C++ backend) — public API
**Priority:** High
**Epic:** EP-005: Full Project Package — Objects, Assets, and Comments
**Date Requested:** 2026-05-27
**Date Implemented:** 2026-05-27
**Date Verified:** 2026-05-27
**Sprint Assigned:** SP-010

**Rationale:**
With the character schema defined (T-0034), the facade needs four CRUD methods so callers can create, open, save, and delete character objects. These are the first object-layer facade methods.

**Current Behavior:**
`ScriviCore` has no object CRUD methods. Character objects cannot be created or read through the facade.

**Desired Behavior:**
`ScriviCore` exposes `createObject`, `openObject`, `saveObject`, `deleteObject` — all scoped to character objects.

**Requirements:**
1. `CreateObjectRequest` / `CreateObjectResult` in `Requests.hpp` / `Results.hpp`
2. `OpenObjectRequest` / `OpenObjectResult` in `Requests.hpp` / `Results.hpp`
3. `SaveObjectRequest` / `SaveObjectResult` in `Requests.hpp` / `Results.hpp`
4. `DeleteObjectRequest` / `DeleteObjectResult` in `Requests.hpp` / `Results.hpp`
5. All four methods on `ScriviCore` facade
6. `ObjectStore` service class in `src/objects/`
7. `cmake --build build` passes with zero errors

**Components Affected:**
- Modified: `ScriviCore/include/scrivi/Requests.hpp`
- Modified: `ScriviCore/include/scrivi/Results.hpp`
- Modified: `ScriviCore/include/scrivi/ScriviCore.hpp`
- Modified: `ScriviCore/src/public_api/ScriviCore.cpp`
- New: `ScriviCore/src/objects/ObjectStore.hpp`
- New: `ScriviCore/src/objects/ObjectStore.cpp`
- Modified: `ScriviCore/CMakeLists.txt`
- Modified: `ScriviCore.xcodeproj/project.pbxproj`

**Implementation Details:**
- `ObjectStore` in `src/objects/`: `charsDir()` returns `projectRoot/objects/characters`; `findByID()` scans directory and matches on `objectID.value`; `create()` derives slug if empty, checks uniqueness; `save()` writes `.bak` before overwriting; `remove()` calls `fs.removeFile`.
- `ErrorCode::ioError` for not-found; `ErrorCode::invalidArgument` for duplicate slug.
- `UUIDProvider::newObjectID()` added to interface; `SystemUUIDProvider` uses `character_` prefix; `DeterministicUUIDProvider` uses `obj-N` sequence.
- `project.pbxproj` updated with E004/E005 under new `objects` group.
- 121/121 tests pass.

**Test Steps:**
1. `cmake --build build --parallel` — zero errors ✅
2. `ctest --test-dir build --output-on-failure` — 121/121 pass ✅
