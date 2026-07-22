# SP-077 (Closed)

## SP-077: [Linux] Native menu bar (File / Edit / Scene / Chapter / Project)

**Status:** ✅ Closed (Human-approved 2026-07-22)
**Epic:** — (Linux app UX; unblocked EP-028 AC5/AC6 Linux verification)
**Goal:** Give the Linux app a proper native **menu bar** on the `ScriviWindow` (`QMainWindow`), so every
core operation has a **mouse-driven trigger** in addition to its keyboard shortcut. This is standard desktop
UX on native Linux, and it is the **only way to exercise `Ctrl-Shift-Backspace` (chapter-merge) over VNC** —
the macOS→VNC input path swallows that combination (proven: only `Key_Control`+`Key_Shift` arrive, the
`Key_Backspace` never does; scene-merge's `Ctrl-Backspace` works because no Shift is held).
**Start Date:** 2026-07-22
**End Date:** 2026-07-22
**Capacity:** ~4–6 hours

### Menu structure

| Menu | Items | Routing |
| ---- | ----- | ------- |
| **File** | New Project…, Open Project…, Close Project, Quit | New/Open → landing page + `ShellController` signal (T-0314/T-0315); Close → landing (flush-safe); Quit → `qApp->quit()` (flush-on-quit wired) |
| **Edit** | Cut, Copy, Paste | `viewport_->cut()/copy()/paste()` via `EditorShell` passthroughs |
| **Scene** | Split Scene (`Ctrl+Return`), Merge Scene (`Ctrl+Backspace`) | `EditorShell::splitScene` / `mergeScene` (public triggers → private slots) |
| **Chapter** | Split Chapter (`Ctrl+Shift+Return`), Merge Chapter (`Ctrl+Shift+Backspace`) | `EditorShell::splitChapter` / `mergeChapter` |
| **Project** | Project Settings… | Stub `QDialog` ("coming soon") |

### Assigned Tasks

| ID     | Title | Priority | Status |
| ------ | ----- | -------- | ------ |
| T-0310 | `[Linux]` **Menu bar scaffold + File/Edit** — build the `QMenuBar` on `ScriviWindow` (File: New/Open/Close/Quit; Edit: Cut/Copy/Paste). Wire File to landing/quit and Edit to the focused `QPlainTextEdit` slots. Enable/disable Close+Edit per active page. | High | ✅ Verified (2026-07-22) |
| T-0311 | `[Linux]` **Scene/Chapter menus** — add Scene ▸ Split/Merge and Chapter ▸ Split/Merge; add public `EditorShell` trigger methods (`splitScene`/`mergeScene`/`splitChapter`/`mergeChapter`) calling the existing private slots; wire the actions; enable only when the editor is active. **This is what made chapter-merge testable over VNC.** | High | ✅ Verified (2026-07-22) |
| T-0312 | `[Linux]` **Project ▸ Settings… stub dialog** — a `QDialog` with a title + "coming soon" placeholder (no settings backend). Menu item enabled when the editor is active. | Medium | ✅ Verified (2026-07-22) |
| T-0313 | `[Linux]` **Verify chapter-merge live over VNC via Chapter ▸ Merge** (closes EP-028 AC5/AC6 Linux) + strip the TEMP key/guard diagnostics in `ManuscriptEditor::keyPressEvent` and `EditorShell::onMergeChapterRequested`. | High | ✅ Verified (2026-07-22) — diagnostics stripped (build+smokes green); chapter-merge Verified live via Chapter ▸ Merge. AC5/AC6 met. |
| T-0314 | `[Linux]` **File ▸ New Project opens the New Project panel** — the menu action now `showLanding()` + emits `ShellController::newProjectRequested`; a `Connections { target: shell }` block in `Landing.qml` pops to the landing root and pushes `newProjectDialog`. Wired `ScriviWindow::setShellController` in `main.cpp`. | High | ✅ Verified (2026-07-22) |
| T-0315 | `[Linux]` **File ▸ Open Project shows the folder picker** — the menu action now emits `ShellController::openProjectRequested`; the `Landing.qml` `Connections` block runs `bridge.chooseFolder(defaultProjectsFolder)` + `window.openPath(picked)`. | High | ✅ Verified (2026-07-22) |
| T-0316 | `[Linux]` **Flush-safe editor-leaving paths + remove the raw "‹ Close" button.** File ▸ Close/New/Open left the editor via `showLanding()` with **no flush** — a live data-loss path. Now each calls `flushEditor()` (== `saveDirtyScenes()`, no-op when nothing open) FIRST. Removed the one-off `‹ Close` toolbar button (kept the `closeRequested` signal for a future button bar). Title bar shows the project title only. | High | ✅ Verified (2026-07-22) — data-safety passed; editor screen looks good |
| T-0317 | `[Linux]` **Ctrl+W closes the project** — bind `QKeySequence::Close` (Ctrl+W on Linux) to the Close Project menu action; standard first-order "close the document" gesture back to the landing page. Flush-safe; editor-only. Not macOS-intercepted (macOS uses Cmd+W). | Medium | ✅ Verified (2026-07-22) |

### Assigned Issues

_(none.)_

### Implementation summary (2026-07-22)

- **Menu bar (T-0310/T-0311/T-0312).** Native `QMenuBar` built in `ScriviWindow::buildMenuBar()`: **File**
  (New/Open → landing + `ShellController` signal; Close → `showLanding` flush-safe; Quit → `qApp->quit()`),
  **Edit** (Cut/Copy/Paste → `EditorShell::cut/copy/pasteSelection` → `viewport_->cut()/copy()/paste()`),
  **Scene** (Split/Merge), **Chapter** (Split/Merge), **Project** (Settings… → `QDialog` "coming soon").
  New public `EditorShell` triggers (`splitScene`/`mergeScene`/`splitChapter`/`mergeChapter` + cut/copy/paste)
  forward to the existing private slots; the Scene/Chapter triggers `viewport_->setFocus()` first so the
  caret-resolving handlers see the real writing-surface caret (a menu click moves focus). Editor-only actions
  collected in `editorOnlyActions_`, enabled/disabled by `updateMenuState()` on every `showEditor`/`showLanding`.
  Actions carry shortcut hints; on native Linux the key still works, and the editor's `keyPressEvent` accepts
  the merge/split keys so there is no double-trigger.
- **File ▸ New / Open routing (T-0314/T-0315).** Rather than reimplement the create dialog + folder picker in
  native widgets, the menu actions route to the **landing page**, which already hosts that UI + the
  `ScriviBridge`. New Project emits `ShellController::newProjectRequested` → a `Landing.qml`
  `Connections { target: shell }` block pops to the landing root and pushes `newProjectDialog`; Open Project
  emits `openProjectRequested` → the same block runs `chooseFolder` + `openPath`. `ScriviWindow::setShellController`
  wired in `main.cpp`.
- **Data-safety (T-0316).** File ▸ Close/New/Open now call `flushEditor()` (== `saveDirtyScenes()`, no-op when
  nothing open) BEFORE leaving the editor via `showLanding()` — closing the live data-loss path. Removed the
  clunky one-off `‹ Close` toolbar button (`closeRequested` signal kept for a future button bar).
- **Ctrl+W (T-0317).** `QKeySequence::Close` (Ctrl+W on Linux) bound to Close Project — standard close-document
  gesture, flush-safe, editor-only, not macOS-intercepted.
- **Diagnostics stripped (T-0313).** Removed the TEMP key/guard logging from `ManuscriptEditor::keyPressEvent`
  and `EditorShell::onMergeChapterRequested` (added in the SP-076 #4 investigation) once chapter-merge was
  confirmed live via the menu.
- **pbxproj:** N/A — Linux/Qt only; no Apple sources. New files (dialog `.cpp`/`.hpp`, if any) go in the Linux
  CMake, not `Scrivi.xcodeproj/project.pbxproj`.

### Verification (2026-07-22)

- ✅ **Container build green** — Qt 6 container build compiled the app + all smokes, exit 0.
- ✅ **Regression smokes green** — `scene_merge_smoke`, `scene_create_smoke`, `scene_reorder_smoke` PASS.
- ✅ **Live VNC walkthrough (user-verified):** every menu action fires; **Chapter ▸ Merge performs the chapter
  merge** (surviving quit→reopen with no scene loss — the AC5/AC6 Linux verification); Edit cut/copy/paste work;
  Scene ▸ Split/Merge work; File ▸ New opens the New Project panel; File ▸ Open shows the folder picker;
  File ▸ Close/New/Open flush before leaving; Ctrl+W closes to landing; menus disable correctly on the landing
  page.

### Retrospective

- **What went well:** The menu bar was the right unblock for chapter-merge over VNC — a clean, decoupled layer
  (`QMenuBar` + public `EditorShell` triggers) that neither touched backend logic nor duplicated the landing's
  bridge plumbing. Routing New/Open back to the landing page kept the window thin.
- **What surfaced:** File ▸ New/Open initially just closed to landing (VNC bug class, fixed in T-0314/T-0315),
  and the editor-leaving paths had a real **data-loss hole** (no flush on Close/New/Open) that T-0316 closed —
  a good catch surfaced only by exercising the menu live. The `‹ Close` toolbar button was retired in favor of
  the menu as the expected UI.
- **Outcome:** The Linux app now has a proper native menu bar; chapter-merge is verifiable over VNC, closing
  EP-028 AC5/AC6 (Linux). All tasks Verified.

---

*Closed 2026-07-22 (Human-approved). SP-077 delivered the Linux native menu bar (File/Edit/Scene/Chapter/Project)
+ flush-safe editor-leaving paths + Ctrl+W. It unblocked EP-028 AC5/AC6 Linux verification (chapter-merge live
via Chapter ▸ Merge) and closed the last EP-028 gap. No Apple sources changed; `scrivi.h` untouched.*
