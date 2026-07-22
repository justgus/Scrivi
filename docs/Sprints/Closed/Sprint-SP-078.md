# SP-078 (Closed)

## SP-078: [Linux] Scene Inspector Panel — dockable panel + Scene Entities stub tab + View-menu toggle

**Status:** ✅ Closed (Human-approved 2026-07-22)
**Epic:** EP-024 `[Linux]` Scene Inspector Panel (opened AND closed the Epic; mirrors Apple EP-014)
**Goal:** Add a hideable **Scene Inspector** panel to the right of the manuscript surface in the Linux editor,
housing a tabbed container whose first tab is a **stub "Scene Entities"** tab (placeholder empty-state, no
entity data yet). Toggle its visibility from a new **View ▸ Show Inspector** menu item (Ctrl+Alt+I),
session-scoped, default **shown** (Apple parity). A **UI-skeleton sprint** — the full panel/tab structure in
place, wired to no data — the shape Apple EP-014 shipped in a single sprint (SP-037). **Closed EP-024.**
**Start Date:** 2026-07-22
**End Date:** 2026-07-22
**Capacity:** ~4–6 hours

### Assigned Tasks

| ID     | Title | Priority | Status |
| ------ | ----- | -------- | ------ |
| T-0318 | `[Linux]` **Inspector panel widget + Scene Entities stub tab** — new `SceneInspector` `QWidget` (`SceneInspector.cpp/.hpp`): a `QTabWidget` with one **"Scene Entities"** tab whose body is the stub — a bold title, a muted **"No entities yet."** empty state, and a **disabled "Add Entity"** button. **120px min / 200px default width** (user pref — narrower than Apple's 240/280). Extensible (new tabs = one `addTab`). No `scrivi_*`. Registered in `platforms/linux/CMakeLists.txt`. | High | ✅ Verified (2026-07-22) |
| T-0319 | `[Linux]` **Dock the panel in `EditorShell` + hide/show plumbing** — added `SceneInspector` as the **third pane** of the existing `QSplitter(Horizontal)` (`navigator_ \| viewport_ \| inspector_`). Public `EditorShell::setInspectorVisible(bool)` / `isInspectorVisible()` — hiding collapses the pane (viewport reclaims the width, no dead space). Session-scoped (no on-disk persistence — Apple parity). Default **shown** (user decision). | High | ✅ Verified (2026-07-22) |
| T-0320 | `[Linux]` **View ▸ Show Inspector menu toggle (Ctrl+Alt+I) + verify** — added a **View** menu to the SP-077 `QMenuBar` with a **checkable "Show Inspector"** action (Ctrl+Alt+I) calling `EditorShell::setInspectorVisible`; editor-only (`editorOnlyActions_`, disabled on landing), check-state synced in `updateMenuState()`. Container build + smokes green; live VNC walkthrough confirmed. Closes EP-024 ACs. | High | ✅ Verified (2026-07-22) |

### Assigned Issues

_(none.)_

### Sprint Notes

- **Reference — Apple EP-014 (`Epics/Closed/Epic-EP-014.md`, `Scrivi/Views/SceneInspectorView.swift`).** The
  Apple inspector is a right-side, hideable, tabbed panel: one "Scene Entities" tab (headline, "No entities
  yet.", disabled "Add Entity"); 240 min / 280 default width; toggled from a View menu at ⌘⌥I; visibility
  session-scoped. This sprint is the Linux mirror. Apple's iPhone/iPad idiom ACs are N/A (Linux desktop only).
- **Docking point.** `EditorShell` lays out `navigator_ | viewport_` in a `QSplitter(Qt::Horizontal)`. The
  inspector is a **third `addWidget`** on that same splitter — the cleanest match for Apple's "between the
  manuscript surface and the window edge." `setCollapsible(2, false)` so a drag can't vanish it; the menu is
  the hide path.
- **Menu home.** SP-077's `QMenuBar` (`ScriviWindow::buildMenuBar`) with `editorOnlyActions_` +
  `updateMenuState()`. **View ▸ Show Inspector** is a checkable `QAction` at **Ctrl+Alt+I** (Apple's ⌘⌥I
  analogue; not macOS→VNC-intercepted — same reasoning as SP-077's Ctrl+W). `QSignalBlocker` when syncing the
  check-state so it doesn't re-drive the toggle.
- **No backend.** No `scrivi_*`, no `SceneDocument` change, no `scrivi.h` touch — pure Qt UI. Real "Scene
  Entities" data is a future EP-024-successor sprint / later Epic (as Apple deferred it past EP-014).
- **pbxproj:** N/A — Linux/Qt only. `SceneInspector.cpp/.hpp` registered in `platforms/linux/CMakeLists.txt`,
  not `Scrivi.xcodeproj/project.pbxproj`.

### Implementation summary (2026-07-22)

- **T-0318 — `SceneInspector`.** `platforms/linux/src/SceneInspector.cpp/.hpp`: a `QTabWidget` (North tab bar,
  document mode — mirrors Apple's segmented Picker above the body) with one **"Scene Entities"** tab. The tab
  body is the stub — a bold title label, a muted (`setEnabled(false)`) **"No entities yet."** centered empty
  state, and a **disabled** "Add Entity" `QPushButton`. `setMinimumWidth(120)` + a 200 preferred width.
  `buildSceneEntitiesTab()` is its own method so a future data-backed version swaps in cleanly. Registered in
  the Linux CMake app target.
- **T-0319 — dock + visibility.** `EditorShell` builds `inspector_` and adds it as the third `QSplitter` pane
  (`navigator_ | viewport_ | inspector_`); stretch factors `0,1,0` so the viewport takes the slack;
  `setCollapsible(2, false)`; `setSizes({240, 580, 200})`. New public `setInspectorVisible(bool)`
  (→ `inspector_->setVisible`) / `isInspectorVisible()`. Session-scoped; defaults **shown**. `splitter_`
  promoted to a member (was a local).
- **T-0320 — View menu toggle.** A **View** menu in `ScriviWindow::buildMenuBar()` (after Edit): a checkable
  **"Show Inspector"** `QAction` at **Ctrl+Alt+I**, `toggled` → `editor_->setInspectorVisible`. Stored as
  `showInspectorAction_`; appended to `editorOnlyActions_` (disabled on landing). `updateMenuState()` syncs its
  check-state to `editor_->isInspectorVisible()` on every page swap, under a `QSignalBlocker`.

### Post-verification tweaks (2026-07-22, user)

After the VNC sign-off the user asked for three sizing adjustments (build re-verified green, 0 warnings):

- **Inspector default width 280 → 200**, **min 240 → 120** (`SceneInspector.cpp` `kDefaultWidth`/`kMinWidth`;
  the splitter's initial third-pane size in `EditorShell.cpp` also 280 → 200). Narrower panel, wider writing
  surface — a departure from Apple's 240/280 by preference.
- **Window default size 820×560 → 1020×760** (`ScriviWindow.cpp` `resize`) — +200 on each dimension.

### Verification (2026-07-22)

- ✅ **Build green** — Qt 6.4 container build (CI toolchain): **184/184** targets; the new `SceneInspector.cpp`
  + edited `EditorShell.cpp`/`ScriviWindow.cpp` compiled with **0 errors, 0 warnings**; `scrivi_linux` linked.
  Re-verified green after the post-verify sizing tweaks.
- ✅ **Regression smokes green** — `scene_merge_smoke`, `scene_create_smoke`, `scene_reorder_smoke`,
  `chapter_reorder_smoke`, `editor_map_smoke` all PASS (this sprint touches no `SceneDocument`/`ScriviBridge`
  logic, so these confirm no collateral breakage).
- ✅ **App-launch smoke** — the app boots under Xvfb and stays up, proving the inspector wiring loads without
  crashing.
- ✅ **Live VNC walkthrough (user-verified 2026-07-22):** panel renders on the right with the tab + stub
  content; **View ▸ Show Inspector** (menu + Ctrl+Alt+I) hides/shows it; the viewport reflows into the
  reclaimed width on hide; "Add Entity" disabled; the menu check-state tracks visibility; the View item inert
  on the landing page. **T-0318/T-0319/T-0320 all Verified; EP-024 AC1–AC6 all met.**
- **No new headless smoke** — the panel is pure UI with no headless-testable logic; verified live over VNC.

### Retrospective

- **What went well:** The Apple EP-014 reference made the scope crisp — a UI skeleton, no backend — and the
  SP-061 `QSplitter` layout + SP-077 `QMenuBar` machinery meant the panel and its toggle slotted in with no new
  infrastructure (a third `addWidget` + a checkable action riding `editorOnlyActions_`/`updateMenuState`). Built
  clean on the first container pass, 0 warnings; delivered in a single sprint exactly like Apple EP-014/SP-037.
- **What surfaced:** Nothing broke; the only follow-ups were the user's sizing preferences (narrower panel,
  bigger window), applied post-verification as a one-line-each tweak and re-verified green. Live GUI was the
  right (only) verification surface for a pure-UI panel — no headless smoke was warranted.
- **Outcome:** All EP-024 ACs Verified in one sprint. **This closed EP-024** (→ `Closed/Epic-EP-024.md`).

---

*Closed 2026-07-22 (Human-approved). SP-078 delivered the Linux Scene Inspector panel (a hideable right-side
`QTabWidget` docked as `EditorShell`'s third splitter pane + a View ▸ Show Inspector toggle at Ctrl+Alt+I on
the SP-077 menu bar; stub "Scene Entities" tab, session-scoped, default shown, 120/200 width). Verified live
over VNC; window enlarged to 1020×760 per user preference. Qt/C++ UI only — no `scrivi_*`/`scrivi.h`/Apple
change. **This closed EP-024** in a single sprint, mirroring Apple EP-014/SP-037.*
