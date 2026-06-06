# Sprint SP-031: Drag Reorder: Scene and Chapter (SwiftUI)

**Status:** ✅ Closed
**Epic:** EP-010: Manuscript Structure Editing — Delete, Reorder, and Title
**Goal:** Wire `reorderScene` and `reorderChapter` through `ScriviEngine.swift`, then implement drag-and-drop reordering in the Scene Navigator. Scenes drag within chapter and across chapter boundaries. Chapter rows drag as containers carrying all their scenes. Drop targets show a clear insertion-line highlight.
**Start Date:** 2026-06-04
**End Date:** 2026-06-05
**Capacity:** —

### Assigned Tasks

| ID | Title | Priority | Status |
| -- | ----- | -------- | ------ |
| T-0106 | Wire `reorderScene` and `reorderChapter` through `scrivi.h` C API and `ScriviEngine.swift` | High | ✅ Verified |
| T-0107 | SwiftUI drag-and-drop for scene rows — reorder within chapter and move across chapter boundary | High | ✅ Verified |
| T-0108 | SwiftUI drag-and-drop for chapter rows — reorder chapter as container carrying all scenes; insertion-line highlight at chapter boundaries | High | ✅ Verified |

### Assigned Issues

| ID | Title | Priority | Status |
| -- | ----- | -------- | ------ |
| I-0008 | Scene Navigation: Scroll to End, Place Cursor, Transfer Focus | High | ✅ Verified |
| I-0009 | Manuscript Text Unreadable in Dark Mode (Black Text on Dark Background) | High | ✅ Verified |

### Sprint Notes

All three Tasks and both Issues were implemented and verified during this Sprint. SP-031 closed clean with no items returned to backlog.

### Retrospective

**Completed (Implemented):**
- T-0106 — `reorderScene`/`reorderChapter` wired through `scrivi.h` C API and `ScriviEngine.swift`
- T-0107 — SwiftUI drag-and-drop for scene rows (within-chapter and cross-chapter)
- T-0108 — SwiftUI drag-and-drop for chapter rows with insertion-line highlight

**Returned to Backlog:**
- None — all assigned items resolved and verified.

**What went well:**
- All three Tasks were implemented in a single Sprint, covering the full SwiftUI drag-reorder surface.
- C API wiring and SwiftUI implementation proceeded without blocking issues.

**What to improve:**
- Nothing notable — Sprint closed clean.

**Carry-forward notes:**
- SP-032 is next: Chapter Title Toggle, Navigator Fallback, and EP-010 Close.
