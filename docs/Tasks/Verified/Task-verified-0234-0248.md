# T-0234 – T-0248 — `[Linux]` EP-022 Manuscript Editor (SP-061 – SP-064)

**Status:** ✅ **All Verified** (2026-07-14 / 2026-07-15, VNC-verified with real `.md` on disk)
**Epic:** EP-022 — `[Linux]` Manuscript Editor · ✅ **Complete — Closed 2026-07-15**
→ [`Epic-EP-022.md`](../../Epics/Closed/Epic-EP-022.md)
**Codebase:** `[Linux]` Qt/QML only — no ScriviCore change (`scrivi.h` confirmed untouched, last
modified SP-055).

**Records of truth:** [`Sprint-SP-061.md`](../../Sprints/Closed/Sprint-SP-061.md) ·
[`Sprint-SP-062.md`](../../Sprints/Closed/Sprint-SP-062.md) ·
[`Sprint-SP-063.md`](../../Sprints/Closed/Sprint-SP-063.md) ·
[`Sprint-SP-064.md`](../../Sprints/Closed/Sprint-SP-064.md)

---

## SP-061 — Shell flip, read-only viewport, navigator (AC1, AC2) — ✅ Verified 2026-07-14

**T-0234 — Shell flip.** `main.cpp` → `QApplication` + `QMainWindow` (`ScriviWindow`) host; the
landing QML re-hosted via `QQuickWidget` in a `QStackedWidget`; `ShellController` as the `shell`
context property; `QFileDialog` picker + Quit preserved; the EP-021 loop re-verified over VNC.

> The flip was forced by a platform constraint, not a preference: `QPlainTextEdit` (needed for Apple
> parity) cannot be embedded in QML on Qt 6.4, so the app re-hosts QML inside a Widgets shell rather
> than the reverse.

**T-0235 — Editor shell + read-only continuous viewport.** `EditorShell` hosts one read-only
`QPlainTextEdit` (document-level undo disabled); `SceneDocument` assembles ordered `scenes[]` +
`activeScene` bodies via `openProject` + a `scrivi_open_scene` loop, with scene-boundary markers and
per-scene start offsets seeding `sceneStartMap`.

**T-0236 — Scene navigator.** Native `QTreeView` (chapter parent rows → scene child rows) in
manuscript order with live titles; click/keyboard selects a scene and scrolls the viewport to its
offset. Display and select only — structure editing was EP-023.

**T-0237 — Verify AC1/AC2 + headless smoke.** `scene_load_smoke` builds a 3-scene fixture with
distinct bodies and asserts the editor's assembly; wired to CI. VNC click-through.

## SP-062 — Editable viewport, auto-save, in-editor creation (AC3) — ✅ Verified 2026-07-14

**T-0238 — Editable viewport + dirty tracking.** New `ManuscriptEditor` (`QPlainTextEdit` subclass)
makes the viewport editable with a `keyPressEvent` / `insertFromMimeData` **boundary guard** keeping
separators non-editable and non-deletable; edits map to the owning scene through the offset map;
per-scene dirty flags.

**T-0239 — Per-scene auto-save.** `scrivi_save_scene` (new `ScriviBridge::saveScene`) on a **~1.5 s
idle debounce** (`QTimer`), **on scene-switch** (`cursorPositionChanged` → owning-segment change), and
on close/app-quit (the VNC foreground-quit path). Real `.md` on disk.

**T-0240 — Ctrl+Return create scene.** (Apple's ⌘↩.) `ManuscriptEditor` emits
`createSceneRequested`; the shell saves, calls `ScriviBridge::createScene` in the caret scene's
chapter after it, inserts the boundary, moves the caret into the new segment, and updates the offset
map + navigator.

**T-0241 — Ctrl+Shift+Return create chapter.** (Apple's ⌘⇧↩.) `createChapterRequested`; the shell
saves, calls `ScriviBridge::createChapter`, then `insertSceneAfter(lastIdx, …, newChapter=true)`,
landing the caret in the new chapter's first scene.

**T-0242 — Verify AC3 + headless smokes.** New `scene_save_smoke`, `scene_create_smoke` (scene +
chapter), `editor_map_smoke`, `quit_smoke`, all wired into CI; full VNC AC3 walk-through
(edit → `save_scene` → reopen asserts new bytes).

## SP-063 — Scroll-driven scene switching (AC1/AC4) — ✅ Verified 2026-07-15

**T-0243 — Scroll → active-scene promotion.** `EditorShell::onScrolled` (on the viewport
`verticalScrollBar()::valueChanged`) computes `visibleSceneIndex()` from the viewport vertical
**midpoint**; crossing a boundary promotes the newly visible scene and saves the departing one.

**T-0244 — Navigator ↔ scroll sync.** The navigator highlight **follows** the scroll-driven active
scene (`promoteActiveScene` → highlight-only `selectNavigatorScene`, no re-scroll, no caret move), so
there is no feedback loop; a navigator click still takes the caret to the clicked scene's start.

**T-0245 — Verify AC1/AC4 (scroll).** `editor_map_smoke` extended (`sceneIndexForScene`,
`sceneIndexForCaret` including the **gap-belongs-to-trailing-scene** rule); VNC walk-through.

## SP-064 — Cursor/focus, restore fidelity, Epic close (AC4–AC7) — ✅ Verified 2026-07-15

**T-0246 — Cursor placement + focus.** `EditorShell::load()` calls `viewport_->setFocus()`, and a new
`showEvent` override re-focuses the viewport when the editor page becomes visible (the
`QStackedWidget` swaps to it *after* load). New `SceneDocument::nearestEditablePosition(pos)` snaps to
the closer of preceding body end / following body start (tie → previous);
`ManuscriptEditor::normalizeCaret` (connected to its own `cursorPositionChanged`, re-entrancy-guarded)
snaps a plain caret out of a heading/separator gap. The non-editable/non-deletable separator was
already enforced by the T-0238 guard — confirmed, no change.

**T-0247 — Quit→reopen surface restore.** **Save half** (`EditorShell::saveScene`): for the segment
the caret is *actually in*, persist scene-local `anchor`/`focus` (`caret − seg.bodyStart`, clamped
≥ 0) plus the whole-document scroll fraction; every other scene stays body-only `0/0/0.0` so a
background flush cannot clobber a live cursor with a stale one. **Restore half**
(`EditorShell::load`): read `restored{anchor,focus,scroll}`, map local→global
(`bodyStart + offset`, `qBound` into the body), set the caret (a selection when anchor ≠ focus),
`centerCursor()`, then apply the saved scroll fraction — replacing the old snap-to-scene-start.
Navigator clicks still go to the scene start (restore fires only on load, once).

**T-0248 — EP-022 verification + close prep.** Full write→save→switch→scroll→create→quit→reopen loop
VNC-verified with real `.md` on disk (reachable from macOS via the shared `/projects` mount); AC1–AC7
checklist walked; `scene_save_smoke` extended to round-trip a non-zero caret (anchor 7 / focus 12) +
scroll 0.375 through `restored{}`; seven headless smokes + Xvfb launch green in Docker; `scrivi.h`
confirmed untouched; EP-022 completion summary drafted for user close.

---

## Files touched (SP-064 representative)

- `platforms/linux/src/EditorShell.{hpp,cpp}` — `showEvent` + load focus (T-0246); save-half surface-state
  capture + restore-half caret/scroll placement (T-0247).
- `platforms/linux/src/ManuscriptEditor.{hpp,cpp}` — `normalizeCaret` + connect (T-0246).
- `platforms/linux/src/SceneDocument.{hpp,cpp}` — `nearestEditablePosition` (T-0246).
- `platforms/linux/tests/editor_map_smoke.cpp` — caret-snap assertions (T-0246).
- `platforms/linux/tests/scene_save_smoke.cpp` — surface-state round-trip through `restored{}` (T-0247/T-0248).

---

⚠️ *Archived 2026-08-18 during the tracking-documentation audit. **These fifteen rows were the worst
staleness found:** `Task-backlog.md` still carried **T-0247 and T-0248 as 🔵 Backlog** and **T-0246 as
🟡 Implemented — Not Verified** — i.e. it showed unstarted work on an Epic that closed 2026-07-15,
a month earlier. All three were Verified in `Sprint-SP-064.md` at the time. The Sprint and Epic
archives were correct throughout; only the backlog rows rotted.*
