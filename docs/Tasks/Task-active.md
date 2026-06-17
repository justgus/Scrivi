# Active Tasks

## SP-042 — Historical Events, Imported Timelines, Export, and Clustering

| ID | Title | Status |
| -- | ----- | ------ |
| T-0161 | Historical event CRUD — author in project, drag on timeline, `#C8A97A` dot | ✅ Verified |
| T-0162 | Imported timeline row — render events as grey row below project row | ✅ Verified |
| T-0163 | Epoch offset dialog — import flow with window intersection preview | ✅ Verified |
| T-0164 | Multiple imported timeline rows — distinct grey shades, hide/show toggle | ✅ Verified |
| T-0165 | Timeline export — produce `.scrivi-timeline.json` | ✅ Verified |
| T-0166 | Co-located dot clustering — hexagonal ring layout, count badge | ✅ Verified |
| T-0169 | Hover tooltips for historical event and imported event dots | ✅ Verified |
| T-0170 | Scene/Chapter split and merge — Cmd-Enter splits at cursor, Cmd-Backspace merges | ✅ Verified |

---

## SP-043 — EP-016 Verification and Polish

| ID | Title | Status |
| -- | ----- | ------ |
| T-0167 | EP-016 verification | 🔴 Open |
| T-0171 | Cluster count badge when ring height exceeds panel height | 🟡 Implemented - Not Verified |
| T-0172 | Zoom in resolves clusters into individual dots | 🟡 Implemented - Not Verified |

---

## T-0170: Scene/Chapter split and merge — Cmd-Enter splits at cursor, Cmd-Backspace merges

**Status:** 🟡 Implemented - Not Verified
**Sprint:** SP-042
**Epic:** EP-016
**Date Implemented:** 2026-06-14

**Description:**
Extend Scrivi's manuscript editing with split and merge operations that respect the cursor position, providing natural authoring flow for restructuring content.

### Split Operations

**Cmd-Enter — Split Scene at cursor (or append if at end):**
- If the cursor is at the **end** of a scene (current behavior): create a new empty scene in the same chapter immediately after the current scene.
- If the cursor is **in the middle** of a scene: split the scene at the cursor position. Text before the cursor stays in the current scene. Text from the cursor onward becomes a new scene in the same chapter, inserted immediately after.
- If the cursor is at the **beginning** of a scene: equivalent to inserting a new empty scene before the current scene (current scene shifts down).

**Shift-Cmd-Enter — Split at cursor creating a new Chapter:**
- If the cursor is at the **end** of the last scene in a chapter: create a new chapter with one empty scene after the current chapter.
- If the cursor is **in the middle** of a scene: split the scene at the cursor. Text before cursor stays in current scene (which stays in current chapter). Text from cursor onward becomes the first scene of a new chapter inserted after the current chapter. All subsequent scenes that were in the current chapter move to the new chapter.
- If the cursor is at the **beginning** of a scene: the current scene and all following scenes in the chapter move to a new chapter; the current chapter retains only its preceding scenes.

### Merge Operations

**Cmd-Backspace — Merge scene with previous scene:**
- Only fires if cursor is at the **very beginning** of a scene (character offset 0 within that scene's text).
- Does nothing if the cursor is at the beginning of the **first scene in a chapter** (no previous scene in the chapter to merge with — merging across chapter boundaries requires Shift-Cmd-Backspace).
- Merges the current scene's text onto the end of the previous scene. The current scene is deleted. Cursor is placed at the join point.

**Shift-Cmd-Backspace — Merge chapter with previous chapter:**
- Only fires if cursor is at the **very beginning of the first scene of a chapter**.
- Does nothing if cursor is in Chapter 1 (no previous chapter).
- All scenes from the current chapter are appended to the previous chapter in order. The current chapter is deleted. Cursor is placed at the start of the first moved scene.

### Constraints
- A chapter must always have at least one scene; split/merge operations must maintain this invariant.
- All split/merge operations must write updated metadata to disk via ScriviEngine immediately.
- The Scene Navigator must reflect changes immediately after each operation.
- The Timeline must reflect changes immediately after each operation (related: I-0038).

**Files Affected:**
- `Scrivi/Views/ManuscriptTextView.swift` — Coordinator key binding handlers
- `Scrivi/Views/ViewportSceneLoader.swift` — split/merge logic and scene boundary tracking
- `Scrivi/Views/TimelineStripView.swift` — `TimelineViewModel.reloadSceneDots`

**Implementation Details:**

### ManuscriptNSTextView.keyDown
Added Cmd-Backspace → `handleMergeScene()` and Shift-Cmd-Backspace → `handleMergeChapter()` intercepts alongside the existing Cmd-Enter / Shift-Cmd-Enter intercepts.

### Coordinator — Cmd-Enter (`handleCreateScene`)
Now cursor-aware. Computes `splitOffsetInSeg` from the cursor's position within the current segment boundary.
- At end: original empty-scene-append behaviour (no text split).
- In middle or at beginning: saves `headText` to current scene via `saveScene`, saves `tailText` to the new scene via `saveScene`, then calls `loader.splitScene(result, at:headText:tailText:)`.
- Always calls `timelineModel.reloadSceneDots` after success.

### Coordinator — Shift-Cmd-Enter (`handleCreateChapter`)
Same cursor-awareness as above for the scene-level split, plus `loader.splitChapter(result, movingFrom:oldChapterID:)` to re-assign any subsequent scenes in the old chapter to the new chapter. Always calls `timelineModel.reloadSceneDots`.

### Coordinator — Cmd-Backspace (`handleMergeScene`)
Guard: only fires if `loc == segRange.location` (cursor at very start of segment) and `segments[segIdx].chapterID == segments[segIdx-1].chapterID` (not first scene in chapter). Joins predecessor text + current text, saves via `saveScene`, deletes current scene via `deleteScene`, calls `loader.mergeSceneIntoPredecessor`. Cursor placed at join point.

### Coordinator — Shift-Cmd-Backspace (`handleMergeChapter`)
Guard: cursor at very start of a segment that is the first scene of its chapter, and not chapter 1. Calls `loader.mergeChapterIntoPredecessor`, then `deleteChapter` on the engine. Cursor placed at the start of the first moved scene.

### ViewportSceneLoader — new methods
- `splitScene(_:at:headText:tailText:)` — inserts new segment and allScenes entry; updates head segment text.
- `mergeSceneIntoPredecessor(at:joinText:)` — removes the current segment and its allScenes entry; updates predecessor text.
- `splitChapter(_:movingFrom:oldChapterID:)` — re-assigns segments at movingFrom+1 onward that still have oldChapterID to the new chapter.
- `mergeChapterIntoPredecessor(at:predecessorChapterID:predecessorChapterMetadataPath:predecessorChapterTitle:)` — re-assigns all segments in the current chapter to the predecessor chapter.

### TimelineViewModel — `reloadSceneDots`
New method that rebuilds only `dots` from an updated scene list, preserving existing band assignments from the current `dots` array. Does not touch historical events, imported timelines, or story structure.

**Test Steps:**
1. **Cmd-Enter at end of scene:** place cursor at end of last word in a scene → press Cmd-Enter → new empty scene appears in navigator and as a new dot on timeline.
2. **Cmd-Enter in middle of scene:** place cursor mid-sentence → press Cmd-Enter → text before cursor stays in current scene, text after cursor moves to a new scene; both visible in navigator and timeline.
3. **Cmd-Enter at beginning of scene:** place cursor at position 0 of a scene → press Cmd-Enter → empty new scene inserted before (cursor scene becomes second); verify navigator and timeline.
4. **Shift-Cmd-Enter at end of chapter's last scene:** cursor at end → press Shift-Cmd-Enter → new chapter created with empty first scene; verify navigator shows new chapter header.
5. **Shift-Cmd-Enter in middle of scene:** cursor mid-sentence → press Shift-Cmd-Enter → head stays in current chapter, tail + all following scenes move to new chapter.
6. **Cmd-Backspace at beginning of non-first scene in chapter:** cursor at position 0 of second scene → press Cmd-Backspace → scenes merge; merged scene in navigator, one fewer dot on timeline.
7. **Cmd-Backspace at beginning of first scene in chapter:** cursor at position 0 of first scene in a chapter → press Cmd-Backspace → nothing happens (no merge across chapter boundary).
8. **Shift-Cmd-Backspace at beginning of first scene of a chapter (not Ch.1):** cursor at position 0 → press Shift-Cmd-Backspace → all scenes of that chapter move to previous chapter; chapter header disappears in navigator.
9. **Shift-Cmd-Backspace in Chapter 1:** cursor at position 0 of first scene → press Shift-Cmd-Backspace → nothing happens.
10. **Timeline sync:** after each operation above, confirm dot count on timeline matches scene count in navigator.

**Acceptance Criteria:**
- [ ] Cmd-Enter at end of scene creates a new scene after current scene in same chapter
- [ ] Cmd-Enter in middle of scene splits text at cursor into two scenes in same chapter
- [ ] Cmd-Enter at beginning of scene inserts an empty scene before current scene
- [ ] Shift-Cmd-Enter at end of last scene in chapter creates new chapter with empty first scene
- [ ] Shift-Cmd-Enter in middle of scene splits text; subsequent scenes move to new chapter
- [ ] Cmd-Backspace at beginning of a scene (not first in chapter) merges with previous scene
- [ ] Cmd-Backspace at beginning of first scene in chapter does nothing
- [ ] Shift-Cmd-Backspace at beginning of first scene in chapter merges chapter with previous
- [ ] Shift-Cmd-Backspace at beginning of first scene in Chapter 1 does nothing
- [ ] Scene Navigator updates immediately after all operations
- [ ] Timeline updates immediately after all operations

---

## T-0171: Cluster count badge when ring height exceeds panel height

**Status:** 🟡 Implemented - Not Verified
**Sprint:** SP-043
**Epic:** EP-016
**Date Implemented:** 2026-06-16

**Description:**
When a dot cluster's ring stack is taller than the available space above the timeline line (from the line up to the label row or the top of the content area), a count badge appears on the cluster centre dot showing the total number of events in the cluster.

**Design Reference:** FR-035

**Implementation:**
Count badge was present in `TimelineStripView.swift` at line ~755. The badge renders as a white number on an accent-coloured capsule, positioned above the tallest ring, and is `allowsHitTesting(false)` so it does not interfere with dot interaction. Identical logic is applied to clusters on imported timeline rows.

The condition is:
```
tallest = ringCount * (dotDiameter + 4)
if tallest > lineY - (activeBands.isEmpty ? 0 : labelRowHeight)  →  show badge
```

**Files Affected:**
- `Scrivi/Views/TimelineStripView.swift` — lines ~755–768 (main row), ~815–826 (imported rows)

**Test Steps:**
1. Shrink the timeline panel to its minimum height (drag top edge down).
2. Create 8+ scenes in the project (enough for ring 2) with the same or very close story-time offset so they cluster.
3. Verify a count badge (white number on coloured capsule) appears above the cluster centre dot.
4. Expand the panel height — verify the badge disappears once there is room for all rings.
5. Zoom in until the cluster dissolves — verify badge disappears.

**Acceptance Criteria:**
- [ ] Count badge appears on cluster centre when ring stack height > available panel space above line.
- [ ] Badge shows the total count (not ring count).
- [ ] Badge is non-interactive (does not block dot hover/drag).
- [ ] Badge disappears when panel is tall enough to show all rings.
- [ ] Badge disappears when zoom level resolves the cluster into individual dots.

---

## T-0172: Zoom in resolves clusters into individual dots

**Status:** 🟡 Implemented - Not Verified
**Sprint:** SP-043
**Epic:** EP-016
**Date Implemented:** 2026-06-16

**Description:**
The timeline supports pinch-to-zoom (trackpad) and horizontal pan, so that as the writer zooms in, co-located dots spread apart and clusters resolve into individual dots that can be independently interacted with.

**Design Reference:** FR-009, FR-032, NFR-007

**Implementation:**

Added to `TimelineStripView`:
- `@State zoomFactor: CGFloat = 1.0` — base zoom level (1 = full span; higher = smaller visible window)
- `@GestureState magnifyGestureScale: CGFloat = 1.0` — live in-progress pinch scale
- `@State scrollOffsetFraction: CGFloat = 0.0` — horizontal pan position (0 = left, 1 = right)
- `effectiveZoom` computed var combining both
- `visibleSpanMs()` and `visibleMinMs()` helpers used by all coordinate math (`dotX`, `eventX`, `offsetMs(fromPanelX:)`)
- `MagnifyGesture` with `.updating($magnifyGestureScale)` for live pinch preview; `zoomFactor` committed on `.onEnded`
- `TimelineScrollCaptureView` (NSViewRepresentable + local NSEvent monitor) for trackpad scroll pan and vertical/⌘ scroll-to-zoom
- Zoom is clamped to [1, 50]; scroll fraction is clamped to [0, 1]
- Visible window centre is preserved during zoom so content doesn't jump

**Clustering auto-resolves:** `buildClusters` uses `itemX()` which picks up the zoomed coordinate math. As zoom increases, pixel distance between dots increases and previously co-located dots exceed the one-diameter threshold, breaking the cluster naturally.

**Files Affected:**
- `Scrivi/Views/TimelineStripView.swift` — `TimelineScrollCaptureView`, `_TimelineScrollNSView`, zoom state vars, zoom helpers, `MagnifyGesture`, scroll background

**Test Steps:**
1. Create a project with 3+ scenes at the same or adjacent story-time positions so they cluster.
2. Pinch outward on the trackpad over the timeline panel — verify dots spread apart and the cluster breaks into individual dots.
3. Pinch inward — verify dots return to a cluster.
4. Scroll horizontally on the trackpad (two-finger swipe left/right) while zoomed in — verify the view pans.
5. Scroll vertically with ⌘ held — verify zoom changes.
6. Verify dot hover tooltips, drag-to-reposition, and context menus all still work correctly on individual dots after zooming in.
7. Verify the timeline returns to full-span view after pinching back to zoom 1.0.

**Acceptance Criteria:**
- [ ] Pinch outward on trackpad zooms in; clusters resolve into individual dots at sufficient zoom.
- [ ] Pinch inward zooms out; dots re-cluster when within one diameter.
- [ ] Horizontal trackpad scroll pans the visible window when zoomed in.
- [ ] Dot interactions (hover, drag, context menu) work correctly at any zoom level.
- [ ] Zoom does not alter underlying `offsetMs` values — it is purely a display transform.
- [ ] All dots return to clustered view when zoom is returned to 1.0.

---

*Last Updated: 2026-06-16 (T-0171 and T-0172 implemented for SP-043)*
