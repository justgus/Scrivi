# Unverified Tasks

Tasks listed here are implemented and awaiting user verification before being archived.

---

## T-0059: `OpenProjectResult` — Add Scene List

**Status:** 🟠 Implemented — Not Verified
**Sprint:** SP-019
**Epic:** EP-008

**Implementation:**

- `std::vector<SceneSummary> scenes` field added to `OpenProjectResult` in `Results.hpp`
- `ProjectOpener::open` populates `scenes` from the resolved `ManuscriptOrderResolver` output (all scenes in manuscript order)
- No new types required — `SceneSummary` already existed in `Types.hpp`
- Adapter (`openProject`) serializes the scenes as a JSON array under key `"scenes"`; each entry contains `sceneID`, `chapterID`, `title`, `slug`, `metadataPath`, `contentPath`
- Swift `OpenProjectResult` gains `scenes: [SceneInfo]`; `SceneInfo` is a new `Decodable` struct mirroring `SceneSummary`

**Files changed:**
- `ScriviCore/include/scrivi/Results.hpp` — `scenes` field added to `OpenProjectResult`
- `ScriviCore/src/project_package/ProjectOpener.cpp` — scenes vector populated from resolver output
- `platforms/apple/Sources/ScriviCoreAdapter/ScriviCoreAdapter.cpp` — scenes array serialized in `openProject`
- `platforms/apple/Sources/Scrivi/ScriviEngine.swift` — `SceneInfo` type added; `OpenProjectResult.scenes` added

**CTest result:** 165/165 (was 160/160; 5 new tests added)
**swift test result:** 19/19 (was 17/17; 2 new tests added)

---

## T-0060: `openScene` Facade Method — Switch Active Scene

**Status:** 🟠 Implemented — Not Verified
**Sprint:** SP-019
**Epic:** EP-008

**Implementation:**

- `OpenSceneRequest` added to `Requests.hpp` (`projectRootPath`, `appSupportRoot`, `projectID`, `sceneID`)
- `OpenSceneResult` added to `Results.hpp` (`scene: SceneSummary`, `markdown`, `restoredSelection`, `restoredScroll`)
- `ScriviCore::openScene` added to `ScriviCore.hpp` / `ScriviCore.cpp`
- Implementation: resolves manuscript order, finds scene by `sceneID`, reads content, updates workspace state `lastWritingSurface`, returns result
- Returns `ErrorCode::invalidArgument` if `sceneID` not found in manuscript

**Files changed:**
- `ScriviCore/include/scrivi/Requests.hpp` — `OpenSceneRequest` added
- `ScriviCore/include/scrivi/Results.hpp` — `OpenSceneResult` added
- `ScriviCore/include/scrivi/ScriviCore.hpp` — `openScene` declaration added
- `ScriviCore/src/public_api/ScriviCore.cpp` — `openScene` implementation added

---

## T-0061: Adapter + Swift Engine — Expose `openScene` and Scene List

**Status:** 🟠 Implemented — Not Verified
**Sprint:** SP-019
**Epic:** EP-008

**Implementation:**

- `openScene(projectRootPath, appSupportRoot, projectID, sceneID)` added to `ScriviCoreAdapter.hpp` and `ScriviCoreAdapter.cpp`
- Adapter serializes `OpenSceneResult` as `{"scene":{...}, "markdown":"..."}`
- `openScene(...)` added to `ScriviEngine.swift`
- `OpenSceneResult` Swift struct added (decodes `scene: SceneInfo` and `markdown: String`)

**Files changed:**
- `platforms/apple/Sources/ScriviCoreAdapter/ScriviCoreAdapter.hpp` — `openScene` declaration
- `platforms/apple/Sources/ScriviCoreAdapter/ScriviCoreAdapter.cpp` — `openScene` implementation
- `platforms/apple/Sources/Scrivi/ScriviEngine.swift` — `openScene` method + `OpenSceneResult` type

---

## T-0062: Integration Tests — Multi-Scene `openProject` and `openScene`

**Status:** 🟠 Implemented — Not Verified
**Sprint:** SP-019
**Epic:** EP-008

**Implementation:**

New test file `ScriviCore/tests/integration/MultiSceneTests.cpp` with 5 tests:

**T-0059 tests:**
- `openProject - scenes list contains all scenes in manuscript order` — creates project, appends second scene, verifies both appear in order
- `openProject - scenes list has one entry for single-scene project`

**T-0060 tests:**
- `openScene - loads correct content for requested scene`
- `openScene - updates workspace state to newly opened scene` — verifies re-opening project restores the scene opened via `openScene`
- `openScene - returns error for unknown sceneID`

New Swift interop tests in `ScriviInteropTests.swift`:
- `openProject returns scenes array with one entry for a freshly created project`
- `openScene returns correct scene content and openProject restores it as active scene`

`MultiSceneTests.cpp` registered in `ScriviCore/tests/CMakeLists.txt` and `ScriviCore.xcodeproj/project.pbxproj`.

**CTest result:** 165/165 passed
**swift test result:** 19/19 passed

---

*Last Updated: 2026-06-01 (T-0059, T-0060, T-0061, T-0062 implemented)*
