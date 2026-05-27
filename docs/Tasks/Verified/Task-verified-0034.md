## T-0034: Character Object Schema (`ObjectJson`)

**Status:** ✅ Implemented - Verified
**Component:** ScriviCore (C++ backend) — schemas
**Priority:** High
**Epic:** EP-005: Full Project Package — Objects, Assets, and Comments
**Date Requested:** 2026-05-27
**Date Implemented:** 2026-05-27
**Date Verified:** 2026-05-27
**Sprint Assigned:** SP-010

**Rationale:**
The `objects/characters/` directory in the `.scrivi` package is the first object type to implement. A typed schema module is needed to read/write character JSON files, following the same pattern as `ProjectJson`, `SceneMetaJson`, and `RepairIssueJson`.

**Current Behavior:**
No object schema exists. The `objects/` directory is created on disk but never read or written by `ScriviCore`.

**Desired Behavior:**
`src/schemas/ObjectJson.hpp/.cpp` exists with `CharacterObject` struct and `serializeCharacter` / `parseCharacter` functions. A character object is stored at `objects/characters/<slug>.json`. `JsonSchemaTests.cpp` covers round-trip serialization.

**Requirements:**
1. `ObjectID` strong ID type added to `IDs.hpp` (prefix `character_`)
2. `CharacterObject` struct defined in `include/scrivi/ObjectTypes.hpp`
3. `src/schemas/ObjectJson.hpp` declares `serializeCharacter` / `parseCharacter`
4. `src/schemas/ObjectJson.cpp` implements both
5. `JsonSchemaTests.cpp` extended with ≥2 round-trip tests
6. `CMakeLists.txt` and `project.pbxproj` updated in the same step

**Components Affected:**
- New: `ScriviCore/include/scrivi/ObjectTypes.hpp`
- New: `ScriviCore/src/schemas/ObjectJson.hpp`
- New: `ScriviCore/src/schemas/ObjectJson.cpp`
- Modified: `ScriviCore/include/scrivi/IDs.hpp` (ObjectID)
- Modified: `ScriviCore/tests/unit/JsonSchemaTests.cpp`
- Modified: `ScriviCore/CMakeLists.txt`
- Modified: `ScriviCore.xcodeproj/project.pbxproj`

**Implementation Details:**
- `ObjectID` strong ID type added to `IDs.hpp`.
- `CharacterObject` in `ObjectTypes.hpp`: `objectID`, `slug`, `displayName`, `status`, `createdAt/By`, `modifiedAt/By`, `notes`, `tags: vector<string>`, `attributes: map<string,string>`.
- Tags serialized as `[{v: "..."}]`, attributes as `[{k: "...", v: "..."}]` due to `JsonDoc` API constraints.
- Schema tag: `scrivi.object.character.v1`.
- 3 unit tests (#62–#64): minimal round-trip, tags+attributes round-trip, wrong-schema rejection.
- `project.pbxproj` updated with E001/E002/E003 in `include/scrivi` and `schemas` groups.
- 115/115 tests pass.

**Test Steps:**
1. `cmake --build build --parallel` — zero errors ✅
2. `ctest --test-dir build --output-on-failure` — 115/115 pass ✅
