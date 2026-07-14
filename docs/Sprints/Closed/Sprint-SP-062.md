# SP-062: [Linux] Editable continuous viewport + per-scene auto-save + in-editor Scene/Chapter creation

**Status:** ✅ Closed
**Epic:** EP-022 `[Linux]` — Writing Surface & Scene Navigator (second of 4 sprints)
**Codebase:** `[Linux]` (`platforms/linux/`) only — **no ScriviCore source change.** `scrivi.h` unchanged
(AC7). Endpoint signatures confirmed against source at activation (2026-07-14):

- `scrivi_save_scene(projectID, projectRootPath, appSupportRoot, sceneID, sceneMetadataPath,
  sceneContentPath, markdown, long long selectionAnchor, long long selectionFocus, double scroll,
  identityID, personaID, displayName)` (`scrivi.h:49`).
- `scrivi_create_scene(projectRootPath, appSupportRoot, projectID, chapterID, afterSceneID, identityID,
  personaID, displayName)` → `{sceneID, chapterID, metadataPath, contentPath}` — takes an `afterSceneID`
  (insert position) + a `chapterID`.
- `scrivi_create_chapter(projectRootPath, appSupportRoot, projectID, identityID, personaID, displayName)`
  → `{chapterID, chapterMetadataPath, firstSceneID, firstSceneMetadataPath, firstSceneContentPath}` — a
  chapter is created **with its first scene**; ScriviCore **appends** it to the end of the manuscript.
**Activated:** 2026-07-14
**Closed:** 2026-07-14 (user-approved)

**Goal:** Make the read-only viewport from SP-061 **editable** and close the **write→save→reopen loop** —
the heart of EP-022. Typing edits the active scene's segment in the single `QTextDocument`; edits persist
per scene via `scrivi_save_scene` on a **debounce (~1.5s idle) + on scene-switch + on close/app-quit**
cadence (Apple `ProjectSession.saveAllDirty` parity). The in-editor Scene/Chapter creation keystrokes
(the reason `QPlainTextEdit` was chosen) land here: **Ctrl+Return** (⌘↩) creates a scene; **Ctrl+Shift+Return**
(⌘⇧↩) creates a chapter. Delivered **EP-022 AC3**.

## Delivered Tasks

| ID | Title | Status |
| -- | ----- | ------ |
| T-0238 | **Editable viewport + dirty tracking** — new `ManuscriptEditor` (`QPlainTextEdit` subclass) makes the viewport editable with a `keyPressEvent`/`insertFromMimeData` **boundary guard** (edits allowed only inside a scene body via `SceneDocument::isEditableRange`); `SceneDocument::applyContentsChange` keeps the offset map in sync on each edit (no rebuild); `EditorShell::onContentsChange` marks the touched scene in `dirtyScenes_` | ✅ Verified |
| T-0239 | **Per-scene auto-save** — `scrivi_save_scene` (new `ScriviBridge::saveScene`) on a **~1.5s idle debounce** (`QTimer`), **on scene-switch** (`cursorPositionChanged` → owning-segment change), and **on close/quit** (Close flush + `QCoreApplication::aboutToQuit` for the landing Quit / foreground-process path + `ScriviWindow::closeEvent` for window-X); drains `dirtyScenes_`, writes per-scene `.md` | ✅ Verified |
| T-0240 | **Ctrl+Return create scene** — `ManuscriptEditor` emits `createSceneRequested`; the shell saves, `ScriviBridge::createScene` (in the caret scene's chapter, after it), then `SceneDocument::insertSceneAfter` surgically splices the empty scene into the live document + map (later offsets shift), rebuilds the navigator, drops the caret in | ✅ Verified |
| T-0241 | **Ctrl+Shift+Return create chapter** — `createChapterRequested`; the shell saves, `ScriviBridge::createChapter`, then `insertSceneAfter(lastIdx, …, newChapter=true)` (ScriviCore appends chapters, so the splice goes at the manuscript end with a chapter-heading boundary), caret into `firstSceneID`, navigator refreshed | ✅ Verified |
| T-0242 | **Verify AC3 + headless smokes** — new `scene_save_smoke`, `scene_create_smoke` (scene + chapter), `editor_map_smoke`, `quit_smoke`, all wired into CI; full VNC AC3 walk-through | ✅ Verified |

## Exit criteria — met

Editable viewport with boundary integrity; per-scene auto-save (debounce + scene-switch + close/quit,
incl. the container foreground-quit path) writing real `.md` with **no data loss** on switch or quit;
`Ctrl+Return` / `Ctrl+Shift+Return` create scene/chapter in-editor with caret landing + navigator +
offset-map update; scene-boundary markers stay non-editable/non-deletable. CI: **seven** headless smokes
green (editor-map, scene-save, scene-create, quit + unregressed scene-load / lifecycle / persistence);
`scrivi.h` unchanged (AC7); macOS + EP-020/021 untouched. **AC3 user-verified over VNC 2026-07-14.**

## Decisions log (during-sprint)

- **Undo:** document-level undo stays **disabled** (`setUndoRedoEnabled(false)`); ⌘Z reserved for EP-026.
- **Boundary integrity:** a **`keyPressEvent` guard** in `ManuscriptEditor` (not read-only text-block
  state) — a modifying keystroke/paste is allowed only if the touched range sits inside one scene body;
  rejected edits never fire `contentsChange`, so the offset map can't be corrupted.
- **Edit→scene routing:** `SceneDocument::applyContentsChange` shifts the owning scene's `bodyLength` +
  later `bodyStart`s by the char delta on each edit — no document rebuild.
- **⌘ vs. Ctrl on Linux:** resolved to **`Ctrl+Return` / `Ctrl+Shift+Return`** (⌘ has no Linux equivalent;
  Qt maps `Qt::ControlModifier` to Ctrl). Documented as the parity keystrokes.
- **New-chapter splice position:** ScriviCore **appends** chapters (`ChapterCreator` `chapters.push_back`),
  so the in-editor create splices at the manuscript **end** (`insertSceneAfter(lastIdx, …)`), not after the
  caret — verified against source and by `scene_create_smoke`.

## Verification

- **AC3 user-verified over VNC 2026-07-14.** Insert text works; edits route to the correct (caret) scene;
  **Add Scene** (`Ctrl+Return`) + **Add Chapter** (`Ctrl+Shift+Return`) gestures work well; auto-save +
  scene-switch + **quit-path save** all fire (edits + new scenes/chapters survive quit→reopen).
- CI: ScriviCore `ctest` unaffected; Linux build + **seven** headless smokes green; macOS/EP-020/021
  untouched (`scrivi.h` unchanged).

## Issues

- **I-0061 — Landing Quit button did nothing (Medium; regression from SP-061's shell flip).** A
  `QQuickWidget`'s engine (unlike `QQmlApplicationEngine`) doesn't auto-wire `QQmlEngine::quit()` to
  `QApplication::quit()`, so the landing Quit button emitted into the void ("no receivers connected").
  **Fixed** in `main.cpp` (explicit `quit`/`exit(int)` connection), which also restored the T-0239
  quit-path save chain. New headless `quit_smoke` guards it. **✅ Resolved - Verified (2026-07-14).**
- **I-0062 — New chapter heading reads "Chapter" (not "Chapter N") until reload (Low; deferred).**
  `scrivi_create_chapter` returns no display title, so the live heading uses the fallback; `open_project`
  supplies the real ordinal title on reload (self-correcting; data on disk correct). Chapter
  naming/structure is **EP-023** scope. Options A/B/C in `docs/Issues/Issue-active.md`. Did **not** block
  close.

## Files (SP-062)

- **New:** `src/ManuscriptEditor.{hpp,cpp}` (editable surface + boundary guard + Ctrl+Return/Shift signals),
  `tests/editor_map_smoke.{cpp,sh}`, `tests/scene_save_smoke.{cpp,sh}`, `tests/scene_create_smoke.{cpp,sh}`,
  `tests/quit_smoke.{cpp,sh}`.
- **Changed:** `src/SceneDocument.{hpp,cpp}` (edit queries, `applyContentsChange`, `bodyText`,
  `insertSceneAfter`), `src/EditorShell.{hpp,cpp}` (editable wiring, auto-save cadence, create handlers,
  navigator/caret helpers), `src/ScriviBridge.{hpp,cpp}` (`saveScene`, `createScene`, `createChapter`),
  `src/ScriviWindow.{hpp,cpp}` (`flushEditor` + `closeEvent`), `src/main.cpp` (`QQmlEngine::quit` connection
  + `aboutToQuit` flush), `CMakeLists.txt` (four new smoke targets), `.github/workflows/scrivi-linux-ci.yml`
  (four new smoke steps).

---

*Closed 2026-07-14 (user-approved). Delivered EP-022 `[Linux]` AC3. Next: SP-063 (scroll-driven scene
switching + navigator↔scroll sync — the AC1/AC4 scroll portion).*
