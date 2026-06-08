# EP-011: Writing Surface Behavior — Scroll, Cursor, and Focus

**Status:** ✅ Closed
**Goal:** Redesign the interaction model of `ManuscriptTextView` and `ViewportSceneLoader` so that scroll position, cursor placement, and keyboard focus behave correctly and predictably for an author working through a multi-scene manuscript.
**Date Created:** 2026-06-06
**Start Date:** 2026-06-08
**Actual Close Date:** 2026-06-08

### Known Defects — Resolved

1. **Scroll jump on scene promotion** → Eliminated by all-in-memory model (T-0114). The load/release cycle no longer exists; scroll position is never disturbed by scene boundary crossing.
2. **Cursor placement after navigate/delete** → Fixed by scene start position map (T-0115, T-0116). Cursor placed using storage offset map, not ad-hoc boundary scanning.
3. **Focus transfer after delete** → Fixed by `takeFocus()` on `ViewportSceneLoader` (T-0117). `focusManuscriptView` binding removed entirely.

### Scope Decisions (2026-06-08)

All five pre-sprint questions answered and implemented. See `docs/Scrivi_WritingSurface_Behavior_Spec_v0_1.md` for the authoritative behavioral contract.

### Tasks

| ID | Title | Sprint | Status |
| -- | ----- | ------ | ------ |
| T-0113 | EP-011 Behavior Spec | SP-033 | ✅ Verified |
| T-0114 | All-scenes-in-memory viewport strategy | SP-033 | ✅ Verified |
| T-0115 | Global cursor position tracking + scene start position map | SP-033 | ✅ Verified |
| T-0116 | Correct cursor placement after navigate and delete | SP-033 | ✅ Verified |
| T-0117 | Reliable first-responder transfer | SP-033 | ✅ Verified |
| T-0118 | Scroll bar fidelity | SP-034 | 🔵 Backlog (deferred) |
| T-0119 | EP-011 verification | SP-033 | ✅ Verified |

### Issues Resolved in This Epic

| ID | Title | Status |
| -- | ----- | ------ |
| I-0010 | Cursor jumps to scene start on boundary crossing | ✅ Verified |
| I-0011 | Chapter title headings editable; text leaks into scene content | ✅ Verified |
| I-0012 | First chapter title never shown | ✅ Verified |
| I-0013 | Project settings do not persist | ✅ Verified |
| I-0014 | Navigation panel shows internal project ID | ✅ Verified |
| I-0015 | No way to edit project title or subtitle | ✅ Verified |
| I-0016 | Navigator selection on load | ⚪ Superseded by I-0018 (backlog) |
| I-0017 | Window maximized state not restored | 🔴 Backlog |
| I-0018 | Navigator shows no selection on load | 🔴 Backlog |

### Sprints

| Sprint | Title | Status |
| ------ | ----- | ------ |
| SP-033 | Writing Surface — All-in-Memory Viewport, Separator, Cursor, and Focus | ✅ Closed |
| SP-034 | Writing Surface — Scroll Bar Fidelity and EP-011 Close | ⚪ Cancelled — T-0118 returned to backlog |

### Deferred Work

- **T-0118** (scroll bar fidelity) — returned to Task backlog. With all-in-memory loading, `NSScrollView` may already provide accurate scroll bar behavior; evaluate before implementing custom logic.
- **I-0017** (window maximized state) — backlog.
- **I-0018** (Navigator no selection on load) — backlog.

---

*Closed: 2026-06-08*
