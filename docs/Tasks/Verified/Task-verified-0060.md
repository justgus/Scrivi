# T-0060: `openScene` Facade Method — Switch Active Scene

**Status:** ✅ Implemented - Verified
**Component:** ScriviCore public API
**Priority:** Critical
**Date Requested:** 2026-06-01
**Date Implemented:** 2026-06-01
**Date Verified:** 2026-06-01
**Sprint Assigned:** SP-019

**Rationale:**
The UI must be able to switch the active scene without reopening the project. `openScene` provides this by reading scene content and updating `workspace-state.json`.

**Implementation:**

- `OpenSceneRequest` added to `Requests.hpp` (`projectRootPath`, `appSupportRoot`, `projectID`, `sceneID`)
- `OpenSceneResult` added to `Results.hpp` (`scene: SceneSummary`, `markdown`, `restoredSelection`, `restoredScroll`)
- `ScriviCore::openScene` added to `ScriviCore.hpp` / `ScriviCore.cpp`
- Resolves manuscript order, finds scene by `sceneID`, reads content, updates workspace state `lastWritingSurface`, returns result
- Returns `ErrorCode::invalidArgument` if `sceneID` not found in manuscript

**Files changed:**
- `ScriviCore/include/scrivi/Requests.hpp`
- `ScriviCore/include/scrivi/Results.hpp`
- `ScriviCore/include/scrivi/ScriviCore.hpp`
- `ScriviCore/src/public_api/ScriviCore.cpp`

**Test result:** 165/165 ctest, 19/19 swift test

---

*Verified: 2026-06-01 (user approved)*
