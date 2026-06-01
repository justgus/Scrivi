# T-0059: `OpenProjectResult` — Add Scene List

**Status:** ✅ Implemented - Verified
**Component:** ScriviCore / ScriviCoreAdapter / ScriviEngine
**Priority:** Critical
**Date Requested:** 2026-06-01
**Date Implemented:** 2026-06-01
**Date Verified:** 2026-06-01
**Sprint Assigned:** SP-019

**Rationale:**
The UI needs a full ordered scene list when opening a project so it can populate a navigation sidebar without calling `openScene` for each entry.

**Implementation:**

- `std::vector<SceneSummary> scenes` field added to `OpenProjectResult` in `Results.hpp`
- `ProjectOpener::open` populates `scenes` from `ManuscriptOrderResolver` output (all scenes in manuscript order)
- Adapter (`openProject`) serializes the scenes as a JSON array under key `"scenes"`; each entry contains `sceneID`, `chapterID`, `title`, `slug`, `metadataPath`, `contentPath`
- Swift `OpenProjectResult` gains `scenes: [SceneInfo]`; `SceneInfo` is a new `Decodable` struct mirroring `SceneSummary`

**Files changed:**
- `ScriviCore/include/scrivi/Results.hpp`
- `ScriviCore/src/project_package/ProjectOpener.cpp`
- `platforms/apple/Sources/ScriviCoreAdapter/ScriviCoreAdapter.cpp`
- `platforms/apple/Sources/Scrivi/ScriviEngine.swift`

**Test result:** 165/165 ctest, 19/19 swift test

---

*Verified: 2026-06-01 (user approved)*
