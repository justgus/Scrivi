---
id: SP-032
title: "Chapter Title Toggle, Navigator Fallback, and EP-010 Close"
epic: EP-010
status: 🔵 Planning
start_date: —
end_date: —
---

## Goal

Implement the global "Show chapter titles in manuscript" toggle and the Navigator title fallback chain. Handle edge cases for delete (currently open scene/chapter). Verify all EP-010 acceptance criteria, confirm ctests green and macOS smoke test passed, then mark EP-010 complete.

## Assigned Tasks

| ID | Title | Priority | Status |
| -- | ----- | -------- | ------ |
| T-0109 | Global "Show chapter titles in manuscript" toggle (Preferences or toolbar); chapter titles render as non-editable headings in writing surface when on; default off | High | 🔵 Backlog |
| T-0110 | Navigator title fallback chain: custom title → first line of text → "Scene N" / "Chapter N"; handle edge case where delete of currently open scene or chapter loads nearest remaining | High | 🔵 Backlog |
| T-0111 | EP-010 acceptance criteria verification — all 14 criteria, ctests green, macOS smoke test | High | 🔵 Backlog |

## Assigned Issues

*None*

## Sprint Notes

- Chapter title headings in the writing surface must be visually distinct from author prose (non-editable, styled differently)
- "Show chapter titles" toggle default is **off**
- Fallback chain applies in the Navigator only; the writing surface is unaffected by fallback logic
- Edge case: deleting the currently open scene → load nearest scene (next preferred, previous if none); deleting all scenes of currently open chapter → load nearest scene of adjacent chapter
- T-0111 is the EP-010 close gate — do not mark EP-010 complete until all 14 acceptance criteria are checked and smoke test is signed off
- Depends on SP-030 and SP-031 being complete

### Retrospective

*(To be completed at close)*

---

*Last Updated: 2026-06-04 (SP-032 created, 🔵 Planning)*
