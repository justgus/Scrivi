---
id: SP-028
title: "C++ Core: renameScene and renameChapter"
epic: EP-010
status: 🟡 Active
start_date: 2026-06-04
end_date: —
---

## Goal

Implement `renameScene` and `renameChapter` facade methods in ScriviCore. Each method writes the `title` field to the target's sidecar JSON atomically. A blank or whitespace-only title is saved as an empty string. Covered by Catch2 unit and integration tests.

## Assigned Tasks

| ID | Title | Priority | Status |
| -- | ----- | -------- | ------ |
| T-0099 | Implement `renameScene(sceneID, newTitle)` C++ facade method — write `title` to scene sidecar JSON | High | 🟠 Implemented - Not Verified |
| T-0100 | Implement `renameChapter(chapterID, newTitle)` C++ facade method — write `title` to chapter sidecar JSON | High | 🟠 Implemented - Not Verified |

## Assigned Issues

*None*

## Sprint Notes

- `renameScene`: writes `title` field to `scene.meta.json` for the target scene; blank/whitespace title saved as `""`
- `renameChapter`: writes `title` field to `chapter.meta.json` (or equivalent sidecar) for the target chapter; blank/whitespace title saved as `""`
- Both must use `AtomicWrite` — write to temp file, rename into place
- Both must be exposed via `scrivi.h` C API for later Swift wiring in SP-030
- All existing ctests must remain green

### Retrospective

*(To be completed at close)*

---

*Last Updated: 2026-06-04 (SP-028 activated; T-0099 and T-0100 implemented, 186/186 ctests passing)*
