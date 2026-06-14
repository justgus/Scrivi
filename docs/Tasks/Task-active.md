# Active Tasks

## SP-042 — Historical Events, Imported Timelines, Export, and Clustering

| ID | Title | Status |
| -- | ----- | ------ |
| T-0161 | Historical event CRUD — author in project, drag on timeline, `#C8A97A` dot | ✅ Verified |
| T-0162 | Imported timeline row — render events as grey row below project row | ✅ Verified |
| T-0163 | Epoch offset dialog — import flow with window intersection preview | ✅ Verified |
| T-0164 | Multiple imported timeline rows — distinct grey shades, hide/show toggle | ✅ Verified |
| T-0165 | Timeline export — produce `.scrivi-timeline.json` | ✅ Verified |
| T-0166 | Co-located dot clustering — hexagonal ring layout, count badge | 🟡 Implemented - Not Verified |
| T-0169 | Hover tooltips for historical event and imported event dots | 🔴 Open |
| T-0170 | Scene/Chapter split and merge — Cmd-Enter splits at cursor, Cmd-Backspace merges | 🔴 Open |

---

---

## T-0170: Scene/Chapter split and merge — Cmd-Enter splits at cursor, Cmd-Backspace merges

**Status:** 🔴 Open
**Sprint:** SP-042
**Epic:** EP-016

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
- `Scrivi/Views/SceneNavigatorView.swift` — navigator reload after split/merge

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

*Last Updated: 2026-06-14 (T-0161–T-0165 verified; T-0170 opened)*
