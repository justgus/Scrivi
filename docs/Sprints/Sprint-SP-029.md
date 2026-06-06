---
id: SP-029
title: "C++ Core: reorderScene and reorderChapter"
epic: EP-010
status: 🟡 Active
start_date: 2026-06-04
end_date: —
---

## Goal

Implement `reorderScene` (within-chapter and cross-chapter move) and `reorderChapter` facade methods in ScriviCore. All affected index JSON files are updated atomically. This is the most complex C++ sprint of EP-010. Covered by Catch2 unit and integration tests.

## Assigned Tasks

| ID | Title | Priority | Status |
| -- | ----- | -------- | ------ |
| T-0101 | Implement `reorderScene(sceneID, sourceChapterID, targetChapterID, afterSceneID?)` C++ facade method — reorder within chapter or move cross-chapter, update all affected indices | High | 🟠 Implemented - Not Verified |
| T-0102 | Implement `reorderChapter(chapterID, afterChapterID?)` C++ facade method — reorder chapter in manuscript index, carrying all its scenes | High | 🟠 Implemented - Not Verified |

## Assigned Issues

*None*

## Sprint Notes

- `reorderScene(sceneID, sourceChapterID, targetChapterID, afterSceneID?)`: caller supplies both source and target chapter — no index walk needed. If same chapter, reorder within that `chapter.meta.json`; if different, remove from source chapter index and insert into target chapter index after `afterSceneID` (or at start if empty). Scene files do not move on disk.
- `reorderChapter(chapterID, afterChapterID?)`: reorders the chapter entry in `manuscript.meta.json`; chapter directory and its scenes do not move on disk — only index order changes. Empty `afterChapterID` = insert at beginning.
- Cross-chapter scene move writes source chapter index first, then target — scene is never in two indices simultaneously
- Both exposed via `scrivi.h` C API for Swift wiring in SP-031
- All existing ctests must remain green

### Retrospective

*(To be completed at close)*

---

*Last Updated: 2026-06-04 (SP-029 activated; T-0101 and T-0102 implemented, 195/195 ctests passing)*
