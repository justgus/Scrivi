---
id: SP-031
title: "Drag Reorder: Scene and Chapter (SwiftUI)"
epic: EP-010
status: 🟡 Active
start_date: 2026-06-04
end_date: —
---

## Goal

Wire `reorderScene` and `reorderChapter` through the C API and `ScriviEngine`, then implement drag-and-drop reordering in the Scene Navigator. Scenes drag within a chapter, across chapter boundaries, and chapters drag as containers carrying all their scenes. Drop targets show an unambiguous insertion-line highlight including at chapter boundaries.

## Assigned Tasks

| ID | Title | Priority | Status |
| -- | ----- | -------- | ------ |
| T-0106 | Wire `reorderScene` and `reorderChapter` through `scrivi.h` C API and `ScriviEngine.swift` | High | 🔵 Backlog |
| T-0107 | SwiftUI drag-and-drop for scene rows — reorder within chapter and move across chapter boundary | High | 🔵 Backlog |
| T-0108 | SwiftUI drag-and-drop for chapter rows — reorder chapter as container carrying all scenes; insertion-line highlight at chapter boundaries | High | 🔵 Backlog |

## Assigned Issues

*None*

## Sprint Notes

- Drop target insertion-line must be visible at the boundary between chapters (last position of chapter N vs. first position of chapter N+1) — this is the most ambiguous case and must be explicitly handled
- macOS is primary; iOS drag reorder is deferred out of EP-010
- Navigator must refresh after any reorder; currently open scene remains open (selection follows the moved scene)
- Depends on SP-029 (reorder C++) being complete

### Retrospective

*(To be completed at close)*

---

*Last Updated: 2026-06-04 (SP-031 activated)*
