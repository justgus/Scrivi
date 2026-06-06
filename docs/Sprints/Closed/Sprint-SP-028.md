---
id: SP-028
title: "C++ Core: renameScene and renameChapter"
epic: EP-010
status: ✅ Closed
start_date: 2026-06-04
end_date: 2026-06-04
---

## Goal

Implement `renameScene` and `renameChapter` facade methods in ScriviCore. Each method writes the `title` field to the target's sidecar JSON atomically. A blank or whitespace-only title is saved as an empty string. Covered by Catch2 unit and integration tests.

## Assigned Tasks

| ID | Title | Priority | Status |
| -- | ----- | -------- | ------ |
| T-0099 | Implement `renameScene(metadataPath, newTitle)` C++ facade method — write `title` to scene sidecar JSON | High | ✅ Verified |
| T-0100 | Implement `renameChapter(metadataPath, newTitle)` C++ facade method — write `title` to chapter sidecar JSON | High | ✅ Verified |

## Assigned Issues

*None*

## Sprint Notes

- `renameScene`: takes `metadataPath` (not sceneID) — reads `scene.meta.json` directly, updates `title`, rewrites atomically
- `renameChapter`: takes `metadataPath` (not chapterID) — reads `chapter.meta.json` directly, updates `title`, rewrites atomically
- Blank/whitespace-only title normalised to `""` before write
- Manuscript-walking helpers `findSceneMetadataPath` / `findChapterMetadataPath` preserved in implementation for future use
- Both exposed via `scrivi.h` C API (`scrivi_rename_scene`, `scrivi_rename_chapter`)
- All ctests green: 184/184

### Retrospective

**Completed:**
- T-0099: `renameScene` — `SceneRenamer.hpp/.cpp`, wired through `ScriviCore`, C API, 3 integration tests
- T-0100: `renameChapter` — `ChapterRenamer.hpp/.cpp`, wired through `ScriviCore`, C API, 3 integration tests

**Returned to Backlog:** None

**What went well:**
- Refactor from ID-based to path-based request was clean — caller already holds the path from `openProject`/`createScene`
- Walking helpers preserved for future use without bloating the hot path

**What to improve:**
- Initial implementation walked the manuscript unnecessarily; caught and corrected before close

**Carry-forward notes:**
- SP-030 will wire Swift-side; `scrivi_rename_scene` and `scrivi_rename_chapter` are ready in the C API
- `findSceneMetadataPath` / `findChapterMetadataPath` available if SP-029 or later needs ID→path resolution

---

*Last Updated: 2026-06-04 (SP-028 closed; T-0099 and T-0100 verified)*
