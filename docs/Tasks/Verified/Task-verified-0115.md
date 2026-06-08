## T-0115: Global cursor position tracking + scene start position map

**Status:** ✅ Implemented - Verified
**Date Implemented:** 2026-06-08
**Component:** `ViewportSceneLoader`, `ManuscriptTextView.swift`
**Priority:** High
**Date Requested:** 2026-06-08
**Sprint Assigned:** SP-033

**Rationale:**
Correct cursor placement after delete and navigate requires knowing where each scene begins in the full `NSTextStorage`, and where the cursor currently is in manuscript coordinates. Absorbs T-0112.

**Current Behavior:**
No global cursor position is tracked. `sceneBoundaries` are computed locally in `ManuscriptTextView.Coordinator` for the current buffer only and are not available to the rest of the app. After a scene is deleted, cursor placement is undefined.

**Desired Behavior:**
- A scene start position map is maintained: `sceneStartMap[sceneID]` = character offset of the first character of that scene in the full `NSTextStorage`.
- The map is rebuilt whenever `NSTextStorage` is structurally modified (scene load, create, delete, reorder).
- A global manuscript cursor position is published: absolute character offset from position 0 of the manuscript.
- A current scene ID is derived from the cursor position and the start map, and published to the SwiftUI environment.
- Cursor position is device-local (not persisted, not shared with other devices).
- For all scenes except the one containing the cursor, scene-level cursor position is null.

**Requirements:**
1. `sceneStartMap: [SceneID: Int]` is maintained on `ViewportSceneLoader` or a companion observable
2. Map is rebuilt on every `rebuildStorage` call
3. `textViewDidChangeSelection` delegate method publishes manuscript cursor position
4. Published `currentSceneID` is derived from cursor position + start map and consumed by `SceneNavigatorView`
5. Scene Navigator selection highlight updates correctly as cursor moves across scene boundaries

**Design Approach:**
Build `sceneStartMap` during the full-load pass in T-0114. Update it on structural operations. Derive `currentSceneID` from a binary search on the map. Publish via `@Observable` or `@Published` property on `ViewportSceneLoader`.

**Components Affected:**
- `ViewportSceneLoader.swift`: add `sceneStartMap`, `manuscriptCursorPosition`, `currentSceneID`
- `ManuscriptTextView.swift` / `Coordinator`: wire `textViewDidChangeSelection`
- `SceneNavigatorView.swift`: consume `currentSceneID` for highlight

**Dependencies:**
- T-0114 must be complete first (full-load populates the map)
- T-0096 (scroll bar fidelity) is a natural consumer of this map — deferred to SP-034

**Test Steps:**
1. Open a project; confirm `sceneStartMap` is populated for all scenes
2. Click in Scene 2 — confirm Scene 2 is highlighted in Navigator
3. Arrow key past a scene boundary — confirm Navigator highlight updates
4. Delete a scene — confirm map is rebuilt and Navigator highlight is correct

**Notes:**
Separator characters have zero width in stored text; they must not contribute to scene start offsets.
