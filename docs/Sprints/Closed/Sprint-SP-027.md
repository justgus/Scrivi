---
id: SP-027
title: "C++ Core: deleteScene and deleteChapter"
epic: EP-010
status: ✅ Closed
start_date: 2026-06-04
end_date: 2026-06-04
---

## Goal

Implement `deleteScene` and `deleteChapter` facade methods in ScriviCore. Each method removes the target from disk and updates the relevant index JSON atomically. Covered by Catch2 unit and integration tests.

## Assigned Tasks

| ID | Title | Priority | Status |
| -- | ----- | -------- | ------ |
| T-0097 | Implement `deleteScene` C++ facade method — remove scene file + sidecar, update chapter index | High | ✅ Verified |
| T-0098 | Implement `deleteChapter` C++ facade method — remove chapter directory, update manuscript index | High | ✅ Verified |

## Assigned Issues

*None*

## Sprint Notes

- `deleteScene`: removes scene `.md` and `.meta.json`, removes the scene entry from the parent chapter's `chapter.meta.json` atomically
- `deleteChapter`: removes the entire chapter directory (all scenes + sidecars + `chapter.meta.json`), removes the chapter entry from `manuscript.meta.json` atomically
- Both exposed via `scrivi.h` C API (`scrivi_delete_scene`, `scrivi_delete_chapter`)
- All ctests green: 178/178

### Retrospective

**Completed:**
- T-0097: `deleteScene` — `SceneDeleter.hpp/.cpp`, wired through `ScriviCore`, C API, 4 integration tests
- T-0098: `deleteChapter` — `ChapterDeleter.hpp/.cpp`, wired through `ScriviCore`, C API, 3 integration tests

**Returned to Backlog:** None

**What went well:**
- Atomic index-first ordering (write new index, then delete files) was clean to implement
- Test fixture `TwoSceneProject` made multi-scene tests concise

**What to improve:**
- `ManuscriptOrderResolver` constructor requires a `CoreServices&` reference — had to store services on the test fixture explicitly; worth noting for future test authors

**Carry-forward notes:**
- SP-030 will wire Swift-side; `scrivi_delete_scene` and `scrivi_delete_chapter` are ready in the C API

---

*Last Updated: 2026-06-04 (SP-027 closed; T-0097 and T-0098 verified)*
