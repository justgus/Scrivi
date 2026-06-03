## T-0085: Live title updates — ~300ms debounce on keystroke for currently active scene segment

**Status:** ✅ Implemented - Verified
**Component:** `Scrivi/Views/ManuscriptTextView.swift`, `Scrivi/Views/ViewportSceneLoader.swift`
**Priority:** High
**Date Requested:** 2026-06-01
**Date Implemented:** 2026-06-01
**Date Verified:** 2026-06-03
**Sprint Assigned:** SP-024

**Rationale:**
The Navigator title for the active scene should reflect what the author is typing without requiring a save or scene switch.

**Desired Behavior:**
As the author types the first line of a scene, the corresponding Navigator row updates within ~300ms of the last keystroke.

**Implementation Details:**
Added `titleTask` to `ManuscriptTextView.Coordinator`. In `textDidChange`, after extracting the current segment's text via `recomputeBoundaries`, a 300ms debounced `Task` fires `loader.updateLiveTitle(firstLine, forSceneID:)`. `ViewportSceneLoader` exposes `liveTitles: [String: String]` (via `@Observable`) and `updateLiveTitle(_:forSceneID:)`. SwiftUI observes the `@Observable` mutation and re-renders `SceneNavigatorView`.

**Files Affected:**
- `Scrivi/Views/ManuscriptTextView.swift` — `titleTask` debounce in `textDidChange`
- `Scrivi/Views/ViewportSceneLoader.swift` — `liveTitles`, `updateLiveTitle`

**Verification:**
- ✅ Navigator title updates within ~300ms of typing the first line
- ✅ Update is debounced (not per-keystroke)

---

*Closed: 2026-06-03*
