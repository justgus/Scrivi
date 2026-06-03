## T-0086: Click-to-navigate — load selected scene, scroll editor to segment start

**Status:** ✅ Implemented - Verified
**Component:** `Scrivi/Views/SceneNavigatorView.swift`, `Scrivi/Views/ViewportSceneLoader.swift`, `Scrivi/Views/ManuscriptTextView.swift`, `Scrivi/Views/EditorView.swift`
**Priority:** High
**Date Requested:** 2026-06-01
**Date Implemented:** 2026-06-01
**Date Verified:** 2026-06-03
**Sprint Assigned:** SP-024

**Rationale:**
Authors must be able to jump directly to any scene by tapping its Navigator row, with the editor scrolling to that scene's content.

**Desired Behavior:**
Tapping a Navigator row saves the current scene, reloads the viewport centred on the target scene, scrolls the `NSTextView` to that segment's start, and updates the Navigator selection highlight.

**Implementation Details:**
`SceneNavigatorView` row `.onTapGesture` calls `navigate(to:sceneID)`, which calls `loader.navigateTo(sceneID:engine:ref:)` (saves current scene, clears segments, reloads viewport from target). `EditorView` stores `navigateToSceneID: String?` state; `ManuscriptTextView.updateNSView` scrolls to that range and clears the binding.

**Files Affected:**
- `Scrivi/Views/SceneNavigatorView.swift` — tap gesture + `navigate(to:)`
- `Scrivi/Views/ViewportSceneLoader.swift` — `navigateTo(sceneID:engine:ref:)`
- `Scrivi/Views/ManuscriptTextView.swift` — `navigateToSceneID` binding, scroll-to-scene in `updateNSView`
- `Scrivi/Views/EditorView.swift` — `navigateToSceneID` state

**Verification:**
- ✅ Tapping a Navigator row loads that scene and scrolls the editor to it
- ✅ Navigator selection highlight follows the tap

---

*Closed: 2026-06-03*
