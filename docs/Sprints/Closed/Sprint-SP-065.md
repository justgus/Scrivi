# SP-065: [Linux] Delete scene/chapter — context menu, confirmation, removal splice + delete-of-active

**Status:** ✅ Closed (user-approved 2026-07-15)
**Epic:** EP-023 `[Linux]` — Manuscript Structure Editing (first of 4 sprints)
**Codebase:** `[Linux]` (`platforms/linux/`) only — **no ScriviCore source change.** `scrivi.h` unchanged
(AC8). Endpoint signatures confirmed against source at activation (2026-07-15):

- `scrivi_delete_scene(projectRootPath, sceneID)` (`scrivi.h:191`) — removes the scene from disk + the
  manuscript/chapter index; returns its `{"ok":…}` envelope.
- `scrivi_delete_chapter(projectRootPath, chapterID)` (`scrivi.h:195`) — removes the chapter **and all its
  scenes**; returns its `{"ok":…}` envelope.

**Activated:** 2026-07-15
**Closed:** 2026-07-15 (user-approved)

**Goal:** Add **delete** to the Linux scene navigator — the first EP-023 structure op. Right-clicking a
scene or chapter row in the `QTreeView` navigator opens a context menu with **Delete**; a confirmation
dialog runs first (chapter delete warns "this chapter **and all its scenes**"). Confirmed deletes call
`scrivi_delete_scene`/`scrivi_delete_chapter`, then **surgically splice** the removed scene(s) out of the
continuous `QTextDocument`, the `SceneDocument` offset map, and the navigator model — mirroring SP-062's
insert splice in reverse (no full-document rebuild). Deleting the **currently active scene** loads the
nearest remaining scene (next, else previous) and transfers keyboard focus to the editor. Delivered **EP-023
AC1 + AC2**.

## Delivered Tasks

| ID | Title | Status |
| -- | ----- | ------ |
| T-0250 | **`delete_scene`/`delete_chapter` bridge wrappers** — `ScriviBridge::deleteScene`/`deleteChapter` (`ScriviBridge.cpp:195/211`) following the `createScene` pattern: RAII `ScriviString`, `parseEnvelope`, `errorOccurred` on `{"error":…}`. No identity args (delete takes none). | ✅ Verified |
| T-0251 | **Navigator context menu + delete confirmation dialogs** — `EditorShell::onNavigatorContextMenu` (custom context-menu policy) → **Delete Scene** / **Delete Chapter** for scene + chapter rows (chapter rows carry `kChapterIDRole`); `QMessageBox::question` confirmations (scene names its title; chapter warns "and all N scene(s)" via `tr` plural); Cancel is a no-op; last-scene / last-chapter guards refuse with an info dialog. | ✅ Verified |
| T-0252 | **Removal splice + delete-of-active-scene navigation** — `SceneDocument::removeScene`/`removeChapter` surgically delete the scene body(ies) + one adjoining boundary from the live `QTextDocument`, shift later `bodyStart`s, and `reflowBoundaryAt` promotes a follower to chapter-first (heading) when a chapter's first scene is deleted. `EditorShell::deleteSceneByID`/`deleteChapterByID`/`afterStructuralRemoval` flush survivors + drop the doomed scene(s) from dirty, call the bridge, splice under the `loading_` guard, rebuild the navigator, and preserve-or-re-anchor the active scene (nearest = next-else-previous + caret-to-start + `setFocus`). | ✅ Verified |
| T-0253 | **Verify AC1/AC2 + delete headless smoke** — new `scene_delete_smoke` (`.cpp`+`.sh`, CMake target `scrivi_linux_scene_delete_smoke`, CI step added): 4-scene/2-chapter fixture; Cases A–E cover mid-chapter delete, first-scene delete (heading reflow), chapter-first promotion, whole-chapter delete, and bridge-delete persistence on reopen; a `checkMapWellFormed` invariant guards the offset map. Docker build green + all 8 headless smokes pass (Ubuntu 24.04/Qt 6.4); VNC walk-through passed. `scrivi.h` untouched. | ✅ Verified |

## Exit criteria — met

Right-click on a scene/chapter navigator row → **Delete** with a confirmation dialog (chapter warns "+ all
its scenes"); confirmed delete calls the bridge, removes the scene(s) from disk, and splices them out of the
live document + map + navigator with no corruption and stable caret/scroll for untouched scenes. Deleting the
**active** scene promotes the nearest remaining scene and transfers editor focus + caret. Last-scene and
last-chapter deletes are refused with a notice (a project keeps ≥1 scene). New `scene_delete_smoke` green in
CI alongside the existing seven; `scrivi.h` unchanged; macOS/EP-020–022 flows unaffected. **AC1 + AC2
user-verified over Docker+VNC 2026-07-15.**

## Files touched

- `platforms/linux/src/ScriviBridge.{hpp,cpp}` — `deleteScene`/`deleteChapter` wrappers (T-0250).
- `platforms/linux/src/SceneDocument.{hpp,cpp}` — `removeScene`/`removeChapter` + `reflowBoundaryAt` +
  `leadingBoundaryFor` helper (T-0252).
- `platforms/linux/src/EditorShell.{hpp,cpp}` — `onNavigatorContextMenu`, `deleteSceneByID`,
  `deleteChapterByID`, `afterStructuralRemoval`, `kChapterIDRole` on chapter rows (T-0251/T-0252).
- `platforms/linux/tests/scene_delete_smoke.{cpp,sh}` — new headless smoke (T-0253).
- `platforms/linux/CMakeLists.txt` — `scrivi_linux_scene_delete_smoke` target (T-0253).
- `.github/workflows/scrivi-linux-ci.yml` — Scene-delete smoke CI step (T-0253).

## Verification

- **AC1 + AC2 user-verified over VNC 2026-07-15.** Right-click → context menu → confirm → scene/chapter
  removed from navigator + viewport; chapter delete warns "and all N scene(s)"; delete-of-active promotes
  the nearest remaining scene with the editor keeping keyboard focus; last-scene/last-chapter guards refuse
  with a notice; deletions persist across quit→reopen (real `.md` / index removal).
- CI: ScriviCore `ctest` unaffected; Linux build + **eight** headless smokes green (adds `scene_delete` to
  editor-map, scene-load, scene-save, scene-create, quit, lifecycle, persistence); macOS/EP-020–022
  untouched (`scrivi.h` unchanged).

## Issues

_(none — no defects surfaced during SP-065. I-0062 remains open, targeted for SP-066.)_

## Retrospective

**Completed:**
- T-0250 — `ScriviBridge::deleteScene`/`deleteChapter` wrappers.
- T-0251 — navigator context menu (Delete Scene / Delete Chapter) + confirmations + last-scene/last-chapter guards.
- T-0252 — `SceneDocument::removeScene`/`removeChapter` + `reflowBoundaryAt` splice; delete-of-active → nearest + focus.
- T-0253 — `scene_delete_smoke` (Cases A–E) + CI step; Docker build + 8 smokes green; VNC walk-through.

**Returned to Backlog:** none.

**What went well:**
- Zero ScriviCore work — the eight structure endpoints already existed, so SP-065 was pure `platforms/linux/`.
- Reusing SP-062's splice model (`insertSceneAfter`) in reverse made `removeScene`/`removeChapter` a clean
  mirror; the `reflowBoundaryAt` helper localized the one genuinely new case (chapter-first promotion).
- Real end-to-end verification available on the macOS host after all: the Docker build compiled the app +
  the new smoke, and all 8 headless smokes ran green before VNC — proving the logic independently of the GUI.

**What to improve:**
- The Task index totals had lagged behind EP-022 (T-0234–T-0249 weren't folded into aggregate counts until
  SP-065 planning). Reconciled at SP-065; keep index stats current at each Epic close.

**Carry-forward notes:**
- SP-066 (rename) closes **I-0062** (live chapter-heading label) — the `reflowBoundaryAt` helper added here
  already rewrites a chapter heading in place, so the rename path can reuse it to update the live label
  without a reload.
- Reorder (SP-067/068) will add `moveScene`/`moveChapter` peers to the same `SceneDocument` splice family.

---

*Closed: 2026-07-15 (user-approved). First of EP-023's 4 sprints. Delivered AC1 + AC2. Tasks T-0250–T-0253
✅ Verified and archived to `Tasks/Verified/Task-verified-0250-0253.md`.*
