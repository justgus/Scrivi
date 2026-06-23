# T-0173: Bidirectional Timeline ↔ Scene Navigator selection and highlight

**Status:** ✅ Implemented - Verified
**Component:** `EditorView.swift`, `TimelineStripView.swift`
**Priority:** Medium
**Epic:** EP-016
**Sprint:** SP-043
**Date Requested:** 2026-06-18
**Date Implemented:** 2026-06-18
**Date Verified:** 2026-06-18

**Description:**
Unified manuscript navigation between the Timeline panel and the Scene Navigator. Selecting a
scene in either place selects it in the other and in the manuscript, with a clear highlight.

1. **Timeline → Navigator + manuscript:** Clicking a scene dot selects that scene (scrolls the
   manuscript into view as the Navigator does) and highlights the clicked dot.
2. **Navigator → Timeline:** Selecting a scene in the Navigator highlights the matching dot in
   the Timeline and pans it into view if off-screen.

**Design Approach:**
`loader.viewportSceneID` (on `@Observable ViewportSceneLoader`) is the single source of truth
for the selected scene. The Navigator already highlights from it, and setting
`navigateToSceneID` in `ManuscriptTextView` already scrolls the manuscript and calls
`loader.setViewportScene`. T-0173 makes the Timeline both a driver and observer of that state —
no new selection model.

**Implementation:**
`Scrivi/Views/EditorView.swift`
- Both `TimelineStripView` call sites pass `loader:` and
  `onSelectScene: { sceneID in navigateToSceneID = sceneID }`.

`Scrivi/Views/TimelineStripView.swift`
- New `loader: ViewportSceneLoader?` and `onSelectScene: ((String) -> Void)?` properties.
- `SceneDotView` gained `isSelected` (orange fill + yellow ring + slight scale-up) and
  `onSelect`; `.onTapGesture { onSelect() }` (coexists with the `minimumDistance: 4` drag);
  the disabled "View Scene" menu item became an enabled "Go to Scene".
- Call site: `isSelected: loader?.viewportSceneID == dot.sceneID`,
  `onSelect: { onSelectScene?(dot.sceneID) }`.
- New `revealScene(_:)` pans `scrollOffsetFraction` to centre an off-screen selected dot;
  `.onChange(of: loader?.viewportSceneID)` calls it.

**Files Affected:**
- `Scrivi/Views/EditorView.swift`
- `Scrivi/Views/TimelineStripView.swift`

**Acceptance Criteria (all verified):**
- [x] Clicking a Timeline dot selects the scene (manuscript scrolls, Navigator highlights).
- [x] The clicked Timeline dot is visibly highlighted.
- [x] Selecting a scene in the Navigator highlights the matching Timeline dot.
- [x] If the selected dot is off-screen (zoomed in), the Timeline pans to reveal it.
- [x] Single click is used; drag-to-reposition is unaffected.
- [x] "Go to Scene" context-menu item performs the same selection.

**Verification note:** Single click did not conflict with drag-to-reposition on the user's
hardware. Verified working as designed.
