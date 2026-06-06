---
id: SP-030
title: "Adapter + Swift Engine: Delete and Rename"
epic: EP-010
status: ✅ Closed
start_date: 2026-06-04
end_date: 2026-06-04
---

## Goal

Wire the four C++ delete/rename methods through `ScriviEngine.swift`, then build the SwiftUI context menu and rename sheet for scene and chapter rows in the Navigator. Delete shows a confirmation dialog. Rename opens a focused edit sheet pre-populated with the current title.

## Assigned Tasks

| ID | Title | Priority | Status |
| -- | ----- | -------- | ------ |
| T-0103 | Wire `deleteScene`, `deleteChapter`, `renameScene`, `renameChapter` through `ScriviEngine.swift` | High | ✅ Verified |
| T-0104 | SwiftUI context menu on scene and chapter Navigator rows — right-click (macOS) / long-press (iOS) with Rename and Delete items | High | ✅ Verified |
| T-0105 | Rename sheet (focused edit field, pre-populated title, save/cancel) and delete confirmation dialog (warns chapter deletes all scenes) | High | ✅ Verified |

## Assigned Issues

| I# | Title | Status |
| -- | ----- | ------ |
| I-0003 | Rename does not update Navigator title in real time | ✅ Verified |
| I-0004 | Renamed scene and chapter titles not displayed in Navigator after restart | ✅ Verified |
| I-0005 | Delete confirmation dialog excessively wide when scene has a long live title | ✅ Verified |
| I-0006 | Scene custom title overridden by prose first-line text in Navigator | ✅ Verified |
| I-0007 | Rename fails with "metadataPath must not be empty" on a newly created scene | ✅ Verified |

## Sprint Notes

- Delete confirmation for chapter explicitly warns that all scenes will be deleted ✅
- Rename sheet saves on confirm; blank title saves as `""` — Navigator falls back to live text ✅
- Swipe-left delete action on Navigator rows implemented alongside context menu ✅
- After a successful delete, Navigator refreshes; if the deleted scene was open, nearest remaining scene is loaded ✅
- `ScriviEngine` methods throw `ScriviError` on failure; UI handles with an alert ✅
- Depends on SP-027 (delete C++) and SP-028 (rename C++) — both closed ✅

### Retrospective

**Completed:**
- T-0103: `deleteScene`, `deleteChapter`, `renameScene`, `renameChapter` wired through `ScriviEngine.swift` with matching Swift result types
- T-0104: Context menu (right-click macOS / long-press iOS) on scene rows and chapter headers; swipe-left delete on scene rows
- T-0105: `RenameSheet` (focused `TextField`, pre-populated, Save/Cancel toolbar); delete confirmation dialogs with chapter scene-count warning; error alert for backend failures

**Issues found and fixed during sprint:**
- I-0003: In-session rename didn't update Navigator — fixed by mutating `allScenes` via `updateSceneTitle` / `updateChapterTitle`
- I-0004: Stored titles absent from `ResolvedScene` — added `chapterTitle` and `chapterMetadataPath` throughout the C++ → Swift stack
- I-0005: Long live title made delete dialog absurdly wide — added `truncated(_:limit:30)` helper
- I-0006: Default `"Scene N"` title stored in sidecar by `SceneCreator` blocked live-text display — fixed by writing `""` at creation time
- I-0007: Empty `metadataPath` on new in-session scenes — fixed `SceneInfo` Decodable init and `insertScene` field population

**What went well:**
- C API was fully ready from SP-027/SP-028; Swift wiring was straightforward
- Issues caught promptly during smoke testing and fixed within the sprint

**What to improve:**
- `SceneCreator` writing a default title was a design smell caught late; the correct invariant (empty title = no custom title) should be documented in the sprint spec upfront

**Carry-forward notes:**
- SP-031 will wire `scrivi_reorder_scene` and `scrivi_reorder_chapter` (C API ready from SP-029)
- Navigator title fallback chain is now: custom title (non-empty) → live first-line prose → "Scene N" / "Chapter N"

---

*Last Updated: 2026-06-04 (SP-030 closed; T-0103/T-0104/T-0105 and I-0003–I-0007 verified)*
