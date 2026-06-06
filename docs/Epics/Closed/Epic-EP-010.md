# EP-010: Manuscript Structure Editing — Delete, Reorder, and Title

**Status:** ✅ Closed
**Goal:** Give the writer full control over manuscript structure from the Scene Navigator. Scenes and chapters can be deleted (with confirmation), reordered by drag (scenes within and across chapters; chapters as containers carrying their scenes), and renamed via a context menu that opens a focused edit sheet. Scene and chapter titles are read from and written to the existing `title` field in their sidecar JSON. A global toggle controls whether chapter titles render as headings in the writing surface.
**Date Created:** 2026-06-03
**Start Date:** 2026-06-04
**Target Close Date:** TBD
**Actual Close Date:** 2026-06-06

### Acceptance Criteria

- [x] Swipe-left (trackpad two-finger swipe / mouse swipe on macOS; swipe gesture on iOS) on a scene row in the Navigator reveals a **Delete** action; tapping it presents a confirmation dialog before the scene is removed
- [x] Swipe-left on a chapter header row reveals a **Delete** action; tapping it presents a confirmation dialog that warns the chapter and **all its scenes** will be deleted
- [x] Right-click (macOS) / long-press (iOS) on any scene or chapter row opens a context menu with **Rename** and **Delete** items
- [x] Tapping **Rename** opens a focused edit sheet pre-populated with the current title; saving writes the new `title` field to the scene or chapter sidecar JSON via a new backend facade method
- [x] A blank or whitespace-only title is treated as "no custom title" — the Navigator falls back to first line of text (scene) or "Chapter N" (chapter); the sidecar `title` field is saved as an empty string
- [x] Scene rows in the Navigator are draggable; dragging a scene within its chapter reorders it; dragging a scene across a chapter boundary moves it to the target chapter at the indicated position
- [x] Chapter header rows in the Navigator are draggable; dragging a chapter moves it and **all its scenes** as a unit to the new position in manuscript order
- [x] Drop targets show a clear insertion-line highlight that makes the landing position unambiguous — including positions at the boundary between chapters (first position of next chapter vs. last position of previous chapter)
- [x] All reorder and cross-chapter move operations update the manuscript index and chapter index JSON on disk atomically
- [x] A **global** "Show chapter titles in manuscript" toggle (in Project Settings) controls whether chapter titles are injected as headings in the writing surface; default **off**
- [x] When the chapter title toggle is **on**, chapter titles render as non-editable headings (visually distinct from author prose) at the divider between the last scene of the previous chapter and the first scene of the new chapter
- [x] Deleting the currently open scene loads the nearest remaining scene (next, or previous if none); keyboard focus transfers to the text view automatically
- [x] All ctests remain green throughout
- [ ] macOS smoke test passed — *deferred: scroll surface defect discovered during SP-032; full smoke test deferred to EP-011*

*(13/14 acceptance criteria verified; AC 14 deferred to EP-011)*

### Scope Notes

**In scope (delivered):**
- Scene delete (single scene, with confirmation)
- Chapter delete (chapter + all its scenes, with confirmation)
- Scene rename (title field in sidecar JSON)
- Chapter rename (title field in sidecar JSON)
- Scene reorder within chapter (drag)
- Scene move across chapter boundary (drag)
- Chapter reorder (drag, carries all scenes)
- Unambiguous drop-target highlight at chapter boundaries
- Global chapter-title-in-manuscript toggle (Project Settings sheet, default off)
- Navigator title fallback chain: custom title → first line of text → "Scene N" / "Chapter N"
- Delete-of-open-scene: navigate to nearest remaining scene + transfer keyboard focus

**Deferred out of EP-010 → EP-011:**
- macOS smoke test (blocked by scroll surface defect)
- Full writing surface behavior redesign (scroll, cursor, focus)

**Still in backlog:**
- iOS/iPadOS drag reorder UI
- "Move to Chapter…" picker
- Scene status flags, synopsis field, word count targets
- Undo/redo for structural operations
- Scroll bar manuscript fidelity (T-0096)

### Backend Facade Methods Delivered

All six planned C++ facade methods implemented and integration-tested:
- `renameScene(metadataPath, newTitle)`
- `renameChapter(metadataPath, newTitle)`
- `deleteScene(sceneID)`
- `deleteChapter(chapterID)`
- `reorderScene(sceneID, sourceChapterID, targetChapterID, afterSceneID?)`
- `reorderChapter(chapterID, afterChapterID?)`

### Sprints

| Sprint | Title | Status | Dates |
| ------ | ----- | ------ | ----- |
| SP-027 | C++ Core: deleteScene and deleteChapter | ✅ Closed | 2026-06-04 → 2026-06-04 |
| SP-028 | C++ Core: renameScene and renameChapter | ✅ Closed | 2026-06-04 → 2026-06-04 |
| SP-029 | C++ Core: reorderScene and reorderChapter | ✅ Closed | 2026-06-04 → 2026-06-04 |
| SP-030 | Adapter + Swift Engine: Delete and Rename | ✅ Closed | 2026-06-04 → 2026-06-04 |
| SP-031 | Drag Reorder: Scene and Chapter (SwiftUI) | ✅ Closed | 2026-06-04 → 2026-06-05 |
| SP-032 | Chapter Title Toggle, Navigator Fallback, and EP-010 Close | ✅ Closed | 2026-06-06 → 2026-06-06 |

### Tasks

| ID | Title | Sprint | Status |
| -- | ----- | ------ | ------ |
| T-0097 | Implement `deleteScene` C++ facade method | SP-027 | ✅ Verified |
| T-0098 | Implement `deleteChapter` C++ facade method | SP-027 | ✅ Verified |
| T-0099 | Implement `renameScene` C++ facade method | SP-028 | ✅ Verified |
| T-0100 | Implement `renameChapter` C++ facade method | SP-028 | ✅ Verified |
| T-0101 | Implement `reorderScene(sceneID, sourceChapterID, targetChapterID, afterSceneID?)` C++ facade method | SP-029 | ✅ Verified |
| T-0102 | Implement `reorderChapter(chapterID, afterChapterID?)` C++ facade method | SP-029 | ✅ Verified |
| T-0103 | Wire delete/rename through C API and ScriviEngine.swift | SP-030 | ✅ Verified |
| T-0104 | SwiftUI context menu on Navigator rows (Rename + Delete) | SP-030 | ✅ Verified |
| T-0105 | Rename sheet and delete confirmation dialog | SP-030 | ✅ Verified |
| T-0106 | Wire reorderScene/reorderChapter through C API and ScriviEngine.swift | SP-031 | ✅ Verified |
| T-0107 | SwiftUI drag-and-drop for scene rows (within chapter + cross-chapter) | SP-031 | ✅ Verified |
| T-0108 | SwiftUI drag-and-drop for chapter rows + insertion-line highlight | SP-031 | ✅ Verified |
| T-0109 | Global chapter title toggle + headings in writing surface | SP-032 | ✅ Verified |
| T-0110 | Navigator title fallback chain + delete-of-open-scene edge case + cursor placement | SP-032 | ✅ Verified |
| T-0111 | EP-010 acceptance criteria verification + macOS smoke test | SP-032 | 🚫 Retired (OBE) |

### Completion Summary

EP-010 delivered full structural editing for Scrivi's manuscript from the Scene Navigator. Six sprints across three weeks built and verified:

- **C++ backend:** Six facade methods (delete, rename, reorder for scenes and chapters) with full integration test coverage
- **Delete:** Swipe-left and context-menu delete with confirmation dialogs for scenes and chapters; delete-of-open-scene navigates to the nearest remaining scene and transfers keyboard focus to the text view
- **Rename:** Context-menu rename opens a focused edit sheet; blank title treated as "no custom title" with fallback chain in the Navigator (custom → first prose line → "Scene N" / "Chapter N")
- **Reorder:** Full drag-and-drop in the Scene Navigator — scenes within chapter, scenes across chapters, chapters as containers; unambiguous insertion-line highlight at chapter boundaries
- **Chapter title toggle:** Project Settings sheet with a "Show chapter titles in manuscript" toggle; headings injected at chapter boundaries in the writing surface when enabled

**Defect discovered at close:** Scroll-driven scene promotion jumps the viewport position when the author scrolls up past a scene boundary. This is a fundamental design issue in `ManuscriptTextView`/`ViewportSceneLoader` predating EP-010 and will be addressed in EP-011.

**Final state:** 13/14 ACs verified · ctests green · macOS full smoke test deferred to EP-011

---

*Closed: 2026-06-06*
