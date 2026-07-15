# SP-063: [Linux] Scroll-driven scene switching + active-scene tracking

**Status:** ✅ Closed
**Epic:** EP-022 `[Linux]` — Writing Surface & Scene Navigator (third of 4 sprints)
**Codebase:** `[Linux]` (`platforms/linux/`) only — **no ScriviCore source change** (`scrivi.h` untouched,
AC7). Reused the SP-062 save path (`ScriviBridge::saveScene`) and the `SceneDocument` offset map — no new
endpoints.
**Activated:** 2026-07-15
**Closed:** 2026-07-15 (user-approved)

**Goal:** Make the **visible scene** the "active" scene and keep **navigator selection ↔ viewport scroll**
in sync — the behavior Apple gave its own sprint (SP-025), kept separate here so it didn't destabilize the
SP-062 write loop. Scrolling the continuous `QTextDocument` past a scene boundary promotes the neighbour to
active (saving the departing scene, reusing SP-062's save path); the navigator highlight follows the
scroll. Delivered the scroll-driven portion of **EP-022 AC1/AC4**.

## Delivered Tasks

| ID | Title | Status |
| -- | ----- | ------ |
| T-0243 | **Scroll → active-scene promotion** — `EditorShell::onScrolled` (on the viewport `verticalScrollBar()::valueChanged`) computes `visibleSceneIndex()` (viewport vertical **midpoint** → `cursorForPosition` → `SceneDocument::sceneIndexForCaret`) and calls the single-writer `promoteActiveScene(newSeg)`, which saves the departing dirty scene (reusing `saveDirtyScenes`) and updates `activeSegment_` **only on a real boundary crossing**. `onCursorMoved` routes through the same writer, so caret and scroll never double-drive or double-save. | ✅ Verified |
| T-0244 | **Navigator ↔ scroll sync** — the navigator highlight **follows** the scroll-driven active scene (`promoteActiveScene` → highlight-only `selectNavigatorScene`, no re-scroll/no caret move); a navigator **click takes the caret to the start of the clicked scene** (via `moveCaretToSegment`, which scrolls it into view + promotes active + highlights). No scroll↔selection feedback loop (`programmaticViewportChange_` guard around programmatic viewport moves). | ✅ Verified |
| T-0245 | **Verify (AC1/AC4 scroll) + headless coverage** — `editor_map_smoke` extended (`sceneIndexForScene`, `sceneIndexForCaret` incl. the gap-belongs-to-trailing-scene rule); VNC walk-through confirmed. | ✅ Verified |

## Exit criteria — met

Scrolling past a boundary changes the active scene **and saves the departing one**; the navigator highlight
tracks scroll position; a navigator click takes the caret to the clicked scene's start and brings it into
view; **no scroll↔selection feedback loop**. CI green (ScriviCore `ctest` unchanged; the seven Linux smokes
still green; macOS/EP-020/021 untouched; `scrivi.h` unchanged). **AC1/AC4 (scroll portion) user-verified
over VNC 2026-07-15.**

## Decisions log (during-sprint)

- **Visible-region → active-scene mapping: vertical midpoint.** `visibleSceneIndex()` maps the viewport's
  vertical midpoint to a document offset → scene (Apple's midpoint rule). Cheap per scroll tick.
- **Active-scene authority: single writer.** Caret and scroll both route through `promoteActiveScene`, which
  acts only on an actual scene change — no double-drive of `activeSegment_`, no double-save.
- **Feedback-loop guard: `programmaticViewportChange_`.** Set around programmatic viewport moves so
  `onScrolled` doesn't re-promote off the shell's own scroll; navigator-follow is highlight-only.
- **No per-tick save.** `promoteActiveScene` no-ops unless the midpoint scene changed → a departing-scene
  save fires once per boundary crossing, not per scroll event. No explicit debounce needed.
- **Navigator click MOVES the caret (reversed mid-sprint, per user).** The sprint plan initially specified a
  *caret-free* navigator scroll (cursor placement was slated for SP-064). During VNC verification the user
  clarified the requirement: a navigator click should **place the caret at the start of the clicked scene**
  (they're about to review/edit from the top). So the caret-free `scrollToScene` (and its scroll-math) was
  **deleted**; navigator click + initial load both use `moveCaretToSegment` (caret → scene `bodyStart`,
  scroll into view, promote, highlight). Caret goes to scene **start**, not end (reviewed as clearer).

## Verification

- **AC1/AC4 (scroll portion) user-verified over VNC 2026-07-15.** Scroll-driven active-scene tracking and
  the navigator highlight follow the scroll correctly; departing scenes are saved on boundary crossings; a
  navigator click lands the caret at the clicked scene's start and scrolls it into view (stable on repeated
  clicks); no scroll↔selection jitter/feedback loop.
- CI: ScriviCore `ctest` unaffected; Linux build + seven headless smokes green; macOS/EP-020/021 untouched
  (`scrivi.h` unchanged).

## Issues / follow-ups

- **Two mid-sprint click-scroll bugs** (relative-nudge scroll, then an absolute-map that landed everything
  at the top) were superseded by the caret-based revert above — not tracked as standalone Issues (never
  shipped/committed; found and fixed within the sprint).
- **T-0249 raised (unscheduled).** Manuscript navigation gestures — **Page Forward / Page Backward + jump to
  absolute manuscript start / end**. Gestures/keystrokes undecided (laptops & tablets lack extended
  keyboards — no PageUp/Down/Home/End assumed). Logged to `Task-backlog.md`; likely an EP-022 follow-up or
  EP-026 (menus/parity). Out of SP-063 scope.

## Files (SP-063)

- **Changed:** `src/EditorShell.{hpp,cpp}` (scroll hook + `visibleSceneIndex` + single-writer
  `promoteActiveScene` + navigator highlight follow + caret-based navigator click; deleted the caret-free
  `scrollToScene`), `src/SceneDocument.{hpp,cpp}` (`sceneIndexForScene`), `tests/editor_map_smoke.cpp`
  (extended assertions). No new files; no CMake/CI change.

---

*Closed 2026-07-15 (user-approved). Delivered EP-022 `[Linux]` AC1/AC4 scroll portion. Next: SP-064
(cursor/focus + quit-reopen restore + full EP-022 verify & Epic close — AC4/AC5/AC6/AC7).*
