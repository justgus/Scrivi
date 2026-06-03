## T-0084: Navigator title derivation — first line of author text or "Scene X"; chapter section headers

**Status:** ✅ Implemented - Verified
**Component:** `Scrivi/Views/SceneNavigatorView.swift`, `Scrivi/Views/ViewportSceneLoader.swift`
**Priority:** High
**Date Requested:** 2026-06-01
**Date Implemented:** 2026-06-01
**Date Verified:** 2026-06-03
**Sprint Assigned:** SP-024

**Rationale:**
Navigator rows must show meaningful titles derived from the author's own text, not system-generated slugs or placeholder IDs.

**Desired Behavior:**
Each Navigator row shows the first non-empty line of the scene's content, trimmed. If the scene has no text, shows "Scene X" (1-based ordinal across the full manuscript). Chapter headers labelled "Chapter N".

**Implementation Details:**
Title resolution in `SceneNavigatorView.chapterGroups`: checks `loader.liveTitles[sceneID]` first (trimmed non-empty first line), falling back to "Scene X" (X = 1-based ordinal in full `allScenes` array). Chapter headers derived from unique `chapterID` transitions in `allScenes`, labelled "Chapter N". On scene load, `loadScene` in `ViewportSceneLoader` seeds `liveTitles` from the loaded content so titles appear immediately on launch without requiring the author to type.

**Files Affected:**
- `Scrivi/Views/SceneNavigatorView.swift` — `chapterGroups` computed property
- `Scrivi/Views/ViewportSceneLoader.swift` — `liveTitles` seeding in `loadScene`

**Verification:**
- ✅ Scenes with text show first line as title
- ✅ Scenes without text show "Scene X"
- ✅ Titles appear correctly immediately on launch (no typing required)
- ✅ Chapter headers labelled "Chapter N"

---

*Closed: 2026-06-03*
