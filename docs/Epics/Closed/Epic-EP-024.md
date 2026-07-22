# EP-024 (Closed)

## EP-024: [Linux] Scene Inspector Panel

**Status:** ✅ Closed (Human-approved 2026-07-22)
**Goal:** The scene inspector side panel on the Linux app — a hideable, tabbed panel docked to the right of
the manuscript surface, mirroring Apple EP-014. First deliverable is the **UI skeleton**: the panel + tab
structure + a stub "Scene Entities" tab, toggled from a View-menu item; real entity/object data is deferred
to a later sprint/Epic (exactly as Apple's EP-014 shipped a skeleton first).
**Date Created:** 2026-07-22
**Target Close Date:** 2026-07-22 (estimated — one sprint, like Apple EP-014/SP-037)
**Actual Close Date:** 2026-07-22

> **Codebase:** `[Linux]` — Qt/QML app only. No `scrivi_*` endpoint, no `scrivi.h` change, no Apple sources.
> The panel holds no writer data yet, so there is no backend surface this Epic.

### Background

The macOS app's Scene Inspector (EP-014, closed 2026-06-10) is a right-side hideable tabbed panel with a
single stub "Scene Entities" tab (headline, "No entities yet.", disabled "Add Entity"), 240 min / 280 default
width, toggled from a View menu at ⌘⌥I, session-scoped visibility. This Epic brought the same panel to Linux.
The Apple Epic delivered the full skeleton in a single sprint; EP-024 mirrored that scope.

### Acceptance Criteria — all met

- [x] AC1 — A Scene Inspector panel is present on the **right** of the Linux editor, between the manuscript
  surface and the window edge (the third pane of `EditorShell`'s splitter). ✅ **Verified (2026-07-22).**
- [x] AC2 — The panel is independently **hideable** via a **View ▸ Show Inspector** menu item (Ctrl+Alt+I);
  its shown/hidden state persists **within the session** and the menu check-state reflects it. ✅ **Verified.**
- [x] AC3 — The panel contains a tab bar with at least one tab, **"Scene Entities"** (stub content — a title,
  a "No entities yet." empty state, and a **disabled** "Add Entity" button, all clearly placeholder). ✅ **Verified.**
- [x] AC4 — The tab bar is architecturally capable of hosting additional tabs in future sprints without a
  structural refactor (a `QTabWidget` — new tabs are a single `addTab`). ✅ **Verified.**
- [x] AC5 — When hidden, the manuscript surface expands to fill the reclaimed width (no dead space); the
  panel has a configurable minimum + default width. ✅ **Verified.** Widths set to **120px min / 200px default**
  (user preference 2026-07-22 — narrower than Apple's 240/280).
- [x] AC6 — Menu item + panel are **absent/inert on the landing page** (editor-only), like the other
  editor-only menu actions. No regression: container build + all existing Linux smokes green. ✅ **Verified.**

> **Apple ACs N/A on Linux:** the iPhone-exclusion / iPad-parity ACs from EP-014 do not apply — the Linux app
> is desktop-only.

### Sprints

| Sprint | Title | Status | Dates |
| ------ | ----- | ------ | ----- |
| SP-078 | `[Linux]` Scene Inspector Panel — dockable panel + Scene Entities stub tab + View-menu toggle | ✅ Closed | 2026-07-22 – 2026-07-22 |

### Tasks

| ID     | Title | Sprint | Status |
| ------ | ----- | ------ | ------ |
| T-0318 | `[Linux]` `SceneInspector` widget + Scene Entities stub tab (`QTabWidget`; title + empty state + disabled Add Entity; 120/200 width) | SP-078 | ✅ Verified (2026-07-22) |
| T-0319 | `[Linux]` Dock the panel as `EditorShell`'s third splitter pane + `setInspectorVisible`/`isInspectorVisible` (session-scoped, default shown) | SP-078 | ✅ Verified (2026-07-22) |
| T-0320 | `[Linux]` View ▸ Show Inspector menu toggle (Ctrl+Alt+I, checkable, editor-only, check-state synced) + verify | SP-078 | ✅ Verified (2026-07-22) |

### Scope Notes

- **UI skeleton only.** No `scrivi_*`, no `SceneDocument` coupling — the tab holds no project data. Real entity
  cards + the add-entity flow are a future sprint / later Epic (matching how Apple deferred them past EP-014).
- **Default visibility = shown** (Apple parity; user decision 2026-07-22).
- **Panel widths = 120px min / 200px default** (user decision 2026-07-22 — narrower than Apple's 240/280); the
  window default was also enlarged to 1020×760 (+200 each) the same day.
- **Menu home:** the SP-077 `QMenuBar` (`ScriviWindow::buildMenuBar`) — the View item rides the existing
  `editorOnlyActions_` enable/disable + `updateMenuState()` machinery.

### Completion Summary

EP-024 delivered the Linux Scene Inspector panel in a single sprint (SP-078), mirroring Apple EP-014's
single-sprint scope. A new `SceneInspector` (`QTabWidget`, one stub "Scene Entities" tab) docks as the third
pane of `EditorShell`'s horizontal splitter, right of the writing surface; **View ▸ Show Inspector** (Ctrl+Alt+I,
checkable, editor-only) on the SP-077 menu bar toggles it, with the check-state synced per page swap. Visibility
is session-scoped (no on-disk persistence); the panel defaults **shown**. Widths are **120px min / 200px default**
(user preference — narrower than Apple's 240/280); the window default was enlarged to 1020×760. All 6 ACs
Verified live over VNC 2026-07-22; container build green (184/184, 0 warnings), all Linux regression smokes +
the Xvfb app-launch smoke PASS. Qt/C++ UI only — no `scrivi_*`, no `scrivi.h`, no Apple change, no new headless
smoke (the panel is pure UI). Real "Scene Entities" data (entity cards + add-entity flow) is deferred to a
future sprint/Epic, exactly as Apple deferred it past EP-014.

### Retrospective

- **What went well:** Crisp scope from the Apple EP-014 reference (UI skeleton, no backend) and the existing
  SP-061 `QSplitter` layout + SP-077 `QMenuBar` machinery meant zero new infrastructure — a third pane + a
  checkable action riding `editorOnlyActions_`/`updateMenuState`. Clean first-pass build (0 warnings); one
  sprint, exactly like Apple EP-014/SP-037.
- **What surfaced:** Only the user's post-verification sizing preferences (narrower panel, larger window),
  applied as a one-line-each tweak and re-verified green. Live GUI over VNC was the correct and only
  verification surface for a pure-UI panel.
- **Outcome:** All ACs Verified in a single sprint; EP-024 closed same-day.

---

*Closed 2026-07-22 (Human-approved). EP-024 delivered the Linux Scene Inspector panel — a hideable right-side
`QTabWidget` docked as `EditorShell`'s third splitter pane, a stub "Scene Entities" tab, and a View ▸ Show
Inspector toggle (Ctrl+Alt+I) on the SP-077 menu bar; session-scoped, default shown, 120/200 width. 1 sprint
(SP-078), 3 tasks, AC1–AC6 all Verified. Qt/C++ UI only; no `scrivi_*`/`scrivi.h`/Apple change. Next in line:
EP-025–EP-026 `[Linux]` (Draft). Archived here.*
