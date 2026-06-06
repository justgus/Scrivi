---
id: SP-027
title: "C++ Core: deleteScene and deleteChapter"
epic: EP-010
status: 🟡 Active
start_date: 2026-06-04
end_date: —
---

## Goal

Implement `deleteScene` and `deleteChapter` facade methods in ScriviCore. Each method removes the target from disk and updates the relevant index JSON atomically. Covered by Catch2 unit and integration tests.

## Assigned Tasks

| ID | Title | Priority | Status |
| -- | ----- | -------- | ------ |
| T-0097 | Implement `deleteScene` C++ facade method — remove scene file + sidecar, update chapter index | High | 🟠 Implemented - Not Verified |
| T-0098 | Implement `deleteChapter` C++ facade method — remove chapter directory, update manuscript index | High | 🟠 Implemented - Not Verified |

## Assigned Issues

*None*

## Sprint Notes

- `deleteScene`: removes `scene.txt` and `scene.meta.json`, removes the scene entry from the parent chapter's `chapter.index.json`
- `deleteChapter`: removes the entire chapter directory (all scenes + sidecars + `chapter.index.json`), removes the chapter entry from `manuscript.index.json`
- Both operations must be atomic (write new index, then delete, not the reverse)
- Both must be exposed via `scrivi.h` C API for later Swift wiring in SP-030
- All existing ctests must remain green

### Retrospective

*(To be completed at close)*

---

*Last Updated: 2026-06-04 (SP-027 activated; T-0097 and T-0098 implemented, 178/178 ctests passing)*
