---
id: SP-029
title: "C++ Core: reorderScene and reorderChapter"
epic: EP-010
status: ✅ Closed
start_date: 2026-06-04
end_date: 2026-06-04
---

## Goal

Implement `reorderScene` (within-chapter and cross-chapter move) and `reorderChapter` facade methods in ScriviCore. Scene and chapter files do not move on disk — only index JSON is updated atomically. Covered by Catch2 integration tests.

## Assigned Tasks

| ID | Title | Priority | Status |
| -- | ----- | -------- | ------ |
| T-0101 | Implement `reorderScene(sceneID, sourceChapterID, targetChapterID, afterSceneID?)` C++ facade method | High | ✅ Verified |
| T-0102 | Implement `reorderChapter(chapterID, afterChapterID?)` C++ facade method | High | ✅ Verified |

## Assigned Issues

*None*

## Sprint Notes

- `reorderScene`: caller supplies sourceChapterID and targetChapterID — no index walk needed. Same-chapter reorder writes one `chapter.meta.json`; cross-chapter writes source first then target (scene never in two indices simultaneously). Scene files do not move on disk.
- `reorderChapter`: reorders chapter entry in `manuscript.meta.json` only. Chapter directories and scenes do not move on disk.
- Both exposed via `scrivi.h` C API (`scrivi_reorder_scene`, `scrivi_reorder_chapter`)
- All ctests green: 195/195

### Retrospective

**Completed:**
- T-0101: `reorderScene` — `SceneReorderer.hpp/.cpp`, wired through `ScriviCore`, C API, 7 integration tests (within-chapter, cross-chapter, error cases)
- T-0102: `reorderChapter` — `ChapterReorderer.hpp/.cpp`, wired through `ScriviCore`, C API, 4 integration tests

**Returned to Backlog:** None

**What went well:**
- Caller-supplied sourceChapterID eliminated the manuscript walk, keeping the implementation minimal
- `FourSceneProject` fixture (2 chapters × 2 scenes) gave good coverage of cross-chapter cases with readable assertions

**What to improve:** Nothing notable.

**Carry-forward notes:**
- SP-031 will wire Swift-side; `scrivi_reorder_scene` and `scrivi_reorder_chapter` are ready in the C API

---

*Last Updated: 2026-06-04 (SP-029 closed; T-0101 and T-0102 verified)*
