## T-0088: Scroll-down past divider — promote next scene, save departing scene

**Status:** ✅ Implemented - Verified
**Component:** `Scrivi/Views/ManuscriptTextView.swift`, `Scrivi/Views/ViewportSceneLoader.swift`
**Priority:** High
**Date Requested:** 2026-06-03
**Date Implemented:** 2026-06-03
**Date Verified:** 2026-06-03
**Sprint Assigned:** SP-025

**Rationale:**
Scrolling past a scene divider should silently promote the scene entering the viewport to "current", saving the scene that scrolled out — without any explicit author action.

**Desired Behavior:**
Scrolling down past a divider makes the scene below the divider the current scene. The departing scene (above) is saved immediately. The Navigator highlight updates to the new current scene.

**Implementation Details:**
`NSScrollView.contentView` posts `NSView.boundsDidChangeNotification`. The Coordinator registers for this in `makeNSView` and handles it with `@objc scrollDidChange`. After a 100ms debounce, the handler calls `recomputeBoundaries`, then uses `NSLayoutManager.glyphIndex(for:in:)` + `characterIndexForGlyph` to find the character at the visible top, resolves it to a segment index via `segmentIndex(for:)`, and calls `loader.scrollPromoteTo(index:engine:ref:)` if the index changed. `scrollPromoteTo` saves the departing segment and updates `currentIndex`, which `SceneNavigatorView` observes via `@Observable` to update its highlight. Both scroll-down and scroll-up are handled by the same code path.

**Files Affected:**
- `Scrivi/Views/ManuscriptTextView.swift` — `makeNSView` scroll observation, `scrollDidChange` handler, `scrollTask`
- `Scrivi/Views/ViewportSceneLoader.swift` — `scrollPromoteTo(index:engine:ref:)`

**Verification:**
- ✅ Scrolling down past a divider promotes the next scene to current
- ✅ Departing scene is saved
- ✅ Navigator highlight updates to the new current scene

---

*Closed: 2026-06-03*
