# SP-061: [Linux] Shell flip to Widgets host + Scene navigator + read-only continuous viewport

**Status:** ✅ Closed
**Epic:** EP-022 `[Linux]` — Writing Surface & Scene Navigator (first of 4 sprints)
**Codebase:** `[Linux]` (`platforms/linux/`) only — **no ScriviCore source change.** The scene-load
endpoints already existed and were verified against source: `scrivi_open_project` returns the ordered
`scenes[]` list (each `{sceneID, chapterID, title, chapterTitle, slug, metadataPath, contentPath,
chapterMetadataPath}`) + `activeScene{sceneID, metadataPath, contentPath, markdown}` + `restored{anchor,
focus, scroll}`; `scrivi_open_scene(projectRootPath, appSupportRoot, projectID, sceneID)` returns
`{scene{sceneID,chapterID,title,slug,metadataPath,contentPath}, markdown}` (`scrivi.h:43` /
`scrivi_c_api.cpp:430`). `scrivi.h` stayed untouched — AC7's additive-only clause held trivially.
**Activated:** 2026-07-14
**Closed:** 2026-07-14 (user-approved)

**Goal:** Turn EP-021's placeholder project window into the **editor shell**, on the architecture EP-022
committed to. Three moves: (1) **flip the app to a Qt Widgets host** — `QApplication` + `QMainWindow`
re-hosting the EP-020/021 QML (Landing / create / open / close / recents) via **`QQuickWidget`**; (2) a
**scene navigator** listing scenes in manuscript order (chapter grouping + live titles, display + select
only); (3) a **read-only continuous viewport** — every scene body in **one `QPlainTextEdit` /
`QTextDocument`** with scene-boundary markers, initial active scene applied. Delivered **EP-022 AC1 + AC2**.

**Why the shell flip was here (and first):** EP-022 locked **`QPlainTextEdit`** (the `NSTextView` analogue)
for parity with the Apple writing surface — in-editor ⌘↩ createScene / ⌘⇧↩ createChapter, custom undo/redo
(`document()->setUndoRedoEnabled(false)`), cut/copy/paste hooks. On the pinned **Qt 6.4** a `QWidget`
can't embed cleanly in a QML `ApplicationWindow` (`WidgetsInQuick` is Qt 6.7+; `QGraphicsProxyWidget` is
too fragile for a live editor), so QML must live **inside** Widgets, not the reverse. Refactoring the
verified EP-021 bootstrap (`QQmlApplicationEngine`-loads-`Landing.qml`) into a `QMainWindow` shell was the
sprint's first and riskiest piece — front-loaded and re-verified before the navigator/viewport went on top.

## Delivered Tasks

| ID | Title | Status |
| -- | ----- | ------ |
| T-0234 | **Shell flip** — `main.cpp` → `QApplication` + `QMainWindow` (`ScriviWindow`) host; landing QML re-hosted via `QQuickWidget` in a `QStackedWidget`; `ShellController` (`shell` context prop) drives the landing→editor swap; context props / `QFileDialog` picker / Quit preserved; full EP-021 loop re-verified over VNC | ✅ Verified |
| T-0235 | **Editor shell + read-only continuous viewport** — `EditorShell` hosts one read-only `QPlainTextEdit` (undo disabled); `SceneDocument` assembles ordered `scenes[]` + `activeScene` body from `openProject` + a `scrivi_open_scene` loop into one `QTextDocument`, keeping the per-scene body-offset map (`sceneStartMap` seed); replaces `ProjectWindow.qml` | ✅ Verified |
| T-0236 | **Scene navigator** — native `QTreeView` (chapter parent rows → scene child rows), manuscript order, live titles; click/keyboard selects a scene and scrolls the viewport to its offset; display + selection only | ✅ Verified |
| T-0237 | **Verify (AC1/AC2) + headless smoke** — `scene_load_smoke` builds a 3-scene fixture with distinct bodies and asserts the editor's assembly (open_project + open_scene loop + `SceneDocument`) loads every body at its recorded offset **and** that a real `QPlainTextEdit` renders the text; wired into CI; VNC click-through | ✅ Verified |

## Exit criteria — met

App launches as a Widgets `QMainWindow` and the **entire EP-021 flow still works** through the
`QQuickWidget`-hosted landing (create, open all 3 modes, close→landing, recents, identity, `QFileDialog`
picker, Quit) — re-verified over VNC (AC7 guard). Opening a `ready` project lands in the editor shell:
navigator lists every scene in order; the read-only continuous viewport shows **all** bodies in one
`QPlainTextEdit`/`QTextDocument` with scene-boundary markers; the initial active scene is applied; clicking
a navigator scene scrolls the viewport. No editing yet (read-only — that's SP-062). CI green: ScriviCore
`ctest` green, the new scene-load smoke green, macOS + EP-020/021 untouched (`scrivi.h` unchanged).
**AC1 + AC2 user-verified over VNC 2026-07-14.**

## Decisions log (during-sprint)

- **Navigator host (T-0236):** **native Widgets `QTreeView`** (chapter parent rows → scene child rows) —
  simpler in the new Widgets shell; EP-023 structure editing wants native drag/drop anyway. No second
  `QQuickWidget`.
- **Scene-boundary marker representation (T-0235):** a **side-kept offset map** (`sceneStartMap`: sceneID →
  char start) plus a chapter-heading line + a blank separator line rendered into the single `QTextDocument`.
  The offset map is the authority (not the text markers), so it survives the viewport becoming
  editable/splittable in SP-062/SP-063 — the marker text is presentation, the map is truth.
- **`ProjectWindow.qml` fate:** superseded by the `QMainWindow` editor shell — **removed** from the qml
  module (dead QML deleted).
- **`QPlainTextDocumentLayout` requirement (bug found in VNC click-through):** the first cut built a bare
  `QTextDocument` and handed it to `QPlainTextEdit::setDocument`, which **rejects** any document lacking a
  `QPlainTextDocumentLayout` (console: *"Document set does not support QPlainTextDocumentLayout"*), leaving
  the viewport blank while the navigator still populated. Fix: `SceneDocument` installs a
  `QPlainTextDocumentLayout` in its constructor before the document is attached (`QTextDocument::clear()`
  preserves it). The scene-load smoke was **hardened** to `setDocument` into a real `QPlainTextEdit` and
  assert the widget renders non-empty text — so this regresses loudly in CI, not silently in the GUI.

## Verification

- **AC1 + AC2 user-verified over VNC 2026-07-14.** Navigator lists all scenes in order; the read-only
  continuous viewport renders every scene body; click-to-scroll works; initial active scene applied. The
  full EP-021 landing→create→open→close→recents loop still works through the `QQuickWidget`-hosted landing.
- CI: ScriviCore `ctest` green; Linux build + persistence + lifecycle + new **scene-load** smoke green;
  macOS + EP-020/021 untouched (`scrivi.h` unchanged).

## Files (SP-061)

- **New:** `src/ScriviWindow.{hpp,cpp}` (host + `ShellController`), `src/EditorShell.{hpp,cpp}`
  (navigator + viewport), `src/SceneDocument.{hpp,cpp}` (continuous document + offset map),
  `tests/scene_load_smoke.{cpp,sh}`.
- **Changed:** `src/main.cpp` (shell flip), `src/ScriviBridge.{hpp,cpp}` (`openScene`),
  `qml/Landing.qml` (root `Item`; `shell.openEditor` handoff), `CMakeLists.txt` (new sources + `QuickWidgets`
  + scene-load smoke target), `.github/workflows/scrivi-linux-ci.yml` (scene-load smoke step),
  `platforms/linux/README.md` (file tree).
- **Deleted:** `qml/ProjectWindow.qml`.

---

*Closed 2026-07-14 (user-approved). Delivered EP-022 `[Linux]` AC1 + AC2. Next: SP-062 (editable viewport
+ per-scene auto-save + in-editor ⌘↩/⌘⇧↩ scene/chapter creation — AC3).*
