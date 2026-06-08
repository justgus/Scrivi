## T-0114: All-scenes-in-memory viewport strategy — replace load/release cycle

**Status:** ✅ Implemented - Verified
**Component:** `ViewportSceneLoader`, `ManuscriptTextView`
**Priority:** High
**Date Requested:** 2026-06-08
**Sprint Assigned:** SP-033

**Rationale:**
The current dynamic load/release cycle is the root cause of the scroll jump defect (EP-011 Known Defect 1). Text is memory-cheap; loading all scenes at once eliminates the need for position restoration entirely.

**Current Behavior:**
`ViewportSceneLoader` maintains a sliding window of loaded scenes. When the author scrolls past a scene boundary, the departing scene is saved and released and a new scene is promoted to current, triggering `rebuildStorage` and resetting scroll position.

**Desired Behavior:**
- All scenes in the manuscript are loaded into `NSTextStorage` at once on project open.
- The dynamic load/release cycle is removed.
- `rebuildStorage` is only called on structural changes (scene create, delete, reorder) — not on scroll.
- For very large manuscripts (threshold TBD in spec T-0113), scenes far from the viewport may be evicted, but this is a future optimization; the default is all-in-memory.
- Scroll position is never disturbed by scene boundary crossing.

**Requirements:**
1. `ViewportSceneLoader` loads all scenes into `NSTextStorage` on `openProject`
2. Scene separator objects are inserted between scenes (see T-0113 for implementation approach)
3. `rebuildStorage` is not triggered by scroll events
4. Structural operations (create, delete, reorder) still trigger a targeted storage update
5. Scene Navigator continues to highlight the correct scene based on cursor position

**Design Approach:**
Remove the scene-promotion logic from the scroll delegate. On open, iterate all scenes and append to `NSTextStorage` with separators between them. The scene start position map (T-0115) is built at this point.

**Components Affected:**
- `ViewportSceneLoader.swift`: remove load/release, add full-load path
- `ManuscriptTextView.swift` / `Coordinator`: update `rebuildStorage` call sites
- Separator injection: new method on `ViewportSceneLoader` or `ManuscriptTextView`

**Dependencies:**
- T-0113 (spec) must be complete first — separator implementation approach must be chosen
- T-0115 (scene start map) is built as a side effect of this task

**Test Steps:**
1. Open a project with 3+ scenes
2. Scroll past multiple scene boundaries — confirm no scroll jump
3. Type in Scene 1, scroll to Scene 3, type — confirm both scenes save correctly
4. Delete a scene — confirm `NSTextStorage` updates correctly without full reload

**Notes:**
The memory eviction fallback for very large manuscripts is explicitly deferred. Do not implement it in this task.
