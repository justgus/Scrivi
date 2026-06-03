## T-0089: Scroll-up past divider — promote previous scene, save departing scene

**Status:** ✅ Implemented - Verified
**Component:** `Scrivi/Views/ManuscriptTextView.swift`, `Scrivi/Views/ViewportSceneLoader.swift`
**Priority:** High
**Date Requested:** 2026-06-03
**Date Implemented:** 2026-06-03
**Date Verified:** 2026-06-03
**Sprint Assigned:** SP-025

**Rationale:**
Scrolling up past a scene divider should promote the scene above to current and save the departing scene below, symmetrically with scroll-down behaviour.

**Desired Behavior:**
Scrolling up past a divider makes the scene above the divider the current scene. The departing scene (below) is saved immediately. The Navigator highlight updates to the new current scene.

**Implementation Details:**
Handled by the same `scrollDidChange` mechanism as T-0088. The segment at the top of the visible rect determines the current scene regardless of scroll direction — scrolling up moves the visible top into a higher segment, which `segmentIndex(for:)` resolves correctly. `scrollPromoteTo` handles the save and `currentIndex` update in both directions.

Also fixed in this task: `fillForward` and `fillBackward` were updated to always load at least one buffer scene in each direction regardless of `viewportCharBudget`, so scenes adjacent to a long scene (one that exceeds the budget alone) are always available for scrolling into.

**Files Affected:**
- `Scrivi/Views/ManuscriptTextView.swift` — same scroll handler as T-0088
- `Scrivi/Views/ViewportSceneLoader.swift` — `fillForward` and `fillBackward` minimum-one-buffer fix

**Verification:**
- ✅ Scrolling up past a divider promotes the previous scene to current
- ✅ Departing scene is saved
- ✅ Navigator highlight updates to the new current scene
- ✅ Long scenes (exceeding viewport budget) correctly load adjacent scenes as buffer

---

*Closed: 2026-06-03*
