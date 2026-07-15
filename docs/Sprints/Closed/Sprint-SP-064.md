# SP-064: [Linux] Cursor/focus behavior + quit-reopen restore + EP-022 verify & close

**Status:** ✅ Closed
**Epic:** EP-022 `[Linux]` — Writing Surface & Scene Navigator (**fourth of 4 — closed EP-022**)
**Codebase:** `[Linux]` (`platforms/linux/`) only — **no ScriviCore source change** (`scrivi.h` untouched,
AC7). Reused the `restored{anchor,focus,scroll}` `scrivi_open_project` already returns and the
`selectionAnchor/focus/scroll` `scrivi_save_scene` already takes — no new endpoints.
**Activated:** 2026-07-15
**Closed:** 2026-07-15 (user-approved)

**Goal:** The EP-011-equivalent **cursor/focus polish** and **quit→reopen surface restore**, then the full
EP-022 verification pass and Epic close. On open the editor takes focus and the caret lands sensibly;
crossing a scene boundary places the caret correctly (no jump-to-start); the scene-boundary separator stays
non-editable/non-deletable. Closing + reopening the project (recents) restores the **last active scene +
cursor + scroll** end-to-end via the `restored{…}` payload `open_project` returns and the
`selectionAnchor/focus/scroll` args `save_scene` persists. Delivered **AC4 (cursor/focus half) + AC5 + AC6 +
AC7** and **closed the Epic**.

## Delivered Tasks

| ID | Title | Status |
| -- | ----- | ------ |
| T-0246 | **Cursor placement + focus** — `EditorShell::load()` calls `viewport_->setFocus()` and a new `showEvent` override re-focuses the viewport when the editor page becomes visible (the `QStackedWidget` swaps to it *after* load). New `SceneDocument::nearestEditablePosition(pos)` (snap to the closer of preceding body end / following body start; tie → previous); `ManuscriptEditor::normalizeCaret` (connected to its own `cursorPositionChanged`, re-entrancy-guarded) snaps a plain caret out of a heading/separator gap. Non-editable/non-deletable separator already enforced by the T-0238 `keyPressEvent`/`insertFromMimeData` guard (confirmed, no change). | ✅ Verified |
| T-0247 | **Quit→reopen surface restore** — **Save half** (`EditorShell::saveScene`): for the segment the caret is *actually in*, persist scene-local `anchor/focus` (`caret − seg.bodyStart`, clamped ≥0) + the whole-document scroll fraction; every other scene stays body-only `0/0/0.0` so a background flush can't clobber a live cursor with a stale one. **Restore half** (`EditorShell::load`): read `restored{anchor,focus,scroll}`, map local→global (`bodyStart + offset`, `qBound` into the body), set the caret (a selection when anchor≠focus), `centerCursor()`, then apply the saved scroll fraction — replacing the old snap-to-scene-start. Navigator clicks still go to the scene start (restore fires only here, once). | ✅ Verified |
| T-0248 | **EP-022 verification + close prep** — full write→save→switch→scroll→create→quit→reopen loop VNC-verified with real `.md` on disk (reachable from macOS via the shared `/projects` mount); AC1–AC7 checklist walked; `scene_save_smoke` extended to round-trip a non-zero caret (anchor 7 / focus 12) + scroll 0.375 through `restored{}`; seven headless smokes + Xvfb launch green in Docker; `scrivi.h` confirmed untouched (last modified SP-055); EP-022 completion summary drafted. | ✅ Verified |

## Exit criteria — met

Editor takes focus on open with a sensible caret; caret correct across boundary crossings; boundary markers
non-editable/non-deletable; closing + reopening (recents) restores the last active scene with its cursor +
scroll; the full write→save→switch→scroll→quit→reopen loop VNC-verified with real `.md` on disk; **all
EP-022 ACs (AC1–AC7) user-verified 2026-07-15**; CI green (seven Linux smokes + launch; ScriviCore `ctest`
unchanged; macOS/EP-020/021 untouched; `scrivi.h` unchanged). EP-022 completion summary drafted; **Epic
closed with user approval 2026-07-15**.

## Files touched

- `platforms/linux/src/EditorShell.{hpp,cpp}` — `showEvent` + load focus (T-0246); save-half surface-state
  capture + restore-half caret/scroll placement (T-0247).
- `platforms/linux/src/ManuscriptEditor.{hpp,cpp}` — `normalizeCaret` + connect (T-0246).
- `platforms/linux/src/SceneDocument.{hpp,cpp}` — `nearestEditablePosition` (T-0246).
- `platforms/linux/tests/editor_map_smoke.cpp` — caret-snap assertions (T-0246).
- `platforms/linux/tests/scene_save_smoke.cpp` — surface-state round-trip through `restored{}` (T-0247/T-0248).

## Deferred (logged, not blocking)

- **I-0062** — new chapter's heading label reads "Chapter" until reload (→ EP-023 structure editing).
- **T-0249** — Page Forward/Backward + jump to manuscript start/end; touch-friendly bindings TBD, unscheduled.
