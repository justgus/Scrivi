# T-0061: Adapter + Swift Engine — Expose `openScene` and Scene List

**Status:** ✅ Implemented - Verified
**Component:** ScriviCoreAdapter / ScriviEngine.swift
**Priority:** Critical
**Date Requested:** 2026-06-01
**Date Implemented:** 2026-06-01
**Date Verified:** 2026-06-01
**Sprint Assigned:** SP-019

**Rationale:**
The JSON-over-string boundary must expose `openScene` and the updated `openProject` scene list so Swift can use both without any C++ struct exposure.

**Implementation:**

- `openScene(projectRootPath, appSupportRoot, projectID, sceneID)` added to `ScriviCoreAdapter.hpp` and `.cpp`
- Adapter serializes `OpenSceneResult` as `{"scene":{...}, "markdown":"..."}`
- `openScene(...)` added to `ScriviEngine.swift`
- `OpenSceneResult` Swift struct added (decodes `scene: SceneInfo` and `markdown: String`)

**Files changed:**
- `platforms/apple/Sources/ScriviCoreAdapter/ScriviCoreAdapter.hpp`
- `platforms/apple/Sources/ScriviCoreAdapter/ScriviCoreAdapter.cpp`
- `platforms/apple/Sources/Scrivi/ScriviEngine.swift`

**Test result:** 165/165 ctest, 19/19 swift test

---

*Verified: 2026-06-01 (user approved)*
