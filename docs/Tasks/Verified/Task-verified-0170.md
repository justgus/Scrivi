# T-0170: Scene/Chapter split and merge — Cmd-Enter splits at cursor, Cmd-Backspace merges

**Status:** ✅ Implemented - Verified
**Component:** `ManuscriptTextView.swift`, `ViewportSceneLoader.swift`, `TimelineStripView.swift`
**Epic:** EP-016
**Sprint:** SP-042
**Date Implemented:** 2026-06-14
**Date Verified:** 2026-06-23

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
- [x] Cmd-Enter at end of scene creates a new scene after current scene in same chapter
- [x] Cmd-Enter in middle of scene splits text at cursor into two scenes in same chapter
- [x] Cmd-Enter at beginning of scene inserts an empty scene before current scene
- [x] Shift-Cmd-Enter at end of last scene in chapter creates new chapter with empty first scene
- [x] Shift-Cmd-Enter in middle of scene splits text; subsequent scenes move to new chapter
- [x] Cmd-Backspace at beginning of a scene (not first in chapter) merges with previous scene
- [x] Cmd-Backspace at beginning of first scene in chapter does nothing
- [x] Shift-Cmd-Backspace at beginning of first scene in chapter merges chapter with previous
- [x] Shift-Cmd-Backspace at beginning of first scene in Chapter 1 does nothing
- [x] Scene Navigator updates immediately after all operations
- [x] Timeline updates immediately after all operations

*Verified by user 2026-06-23 (per prior indication).*
