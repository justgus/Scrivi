# Verified Tasks — T-0250–T-0253 (SP-065, EP-023 [Linux] Manuscript Structure Editing — delete)

_Archived 2026-07-15 on SP-065 close. All four tasks ✅ Verified over Docker+VNC. Original active-task detail
preserved below verbatim._

**Sprint SP-065** — `[Linux]` Delete scene/chapter (first of 4 EP-023 sprints): the **delete** structure op —
navigator context menu, confirmation dialogs, the `SceneDocument` removal splice, and delete-of-active-scene
navigation. Epic: EP-023 `[Linux]`. **No ScriviCore work** — `scrivi_delete_scene`/`scrivi_delete_chapter`
already exist (`scrivi.h:191/195`); pure `platforms/linux/` bridge + `QTreeView`/viewport UI. Delivered
**EP-023 AC1 + AC2**.

| ID | Title | Status |
| -- | ----- | ------ |
| T-0250 | `delete_scene`/`delete_chapter` `ScriviBridge` wrappers — `ScriviBridge::deleteScene`/`deleteChapter` (`ScriviBridge.cpp:195/211`) following the `createScene` pattern: RAII `ScriviString`, `parseEnvelope`, `errorOccurred` on `{"error":…}`. No identity args (delete takes none). | ✅ Verified |
| T-0251 | Navigator context menu + delete confirmation dialogs — `EditorShell::onNavigatorContextMenu` (custom context-menu policy) → **Delete Scene** / **Delete Chapter** for scene + chapter rows (chapter rows carry `kChapterIDRole`); `QMessageBox::question` confirmations (scene names its title; chapter warns "and all N scene(s)" via `tr` plural); Cancel is a no-op; last-scene / last-chapter guards refuse with an info dialog. | ✅ Verified |
| T-0252 | Removal splice + delete-of-active-scene navigation — `SceneDocument::removeScene`/`removeChapter` surgically delete the scene body(ies) + one adjoining boundary from the live `QTextDocument`, shift later `bodyStart`s, and `reflowBoundaryAt` promotes a follower to chapter-first (heading) when a chapter's first scene is deleted. `EditorShell::deleteSceneByID`/`deleteChapterByID`/`afterStructuralRemoval` flush survivors + drop the doomed scene(s) from dirty, call the bridge, splice under the `loading_` guard, rebuild the navigator, preserve-or-re-anchor the active scene (nearest = next-else-previous + caret-to-start + `setFocus`). | ✅ Verified |
| T-0253 | Verify AC1/AC2 + delete headless smoke — new `scene_delete_smoke` (`.cpp`+`.sh`, CMake target `scrivi_linux_scene_delete_smoke`, CI step): 4-scene/2-chapter fixture; Cases A–E cover mid-chapter delete, first-scene delete (heading reflow), chapter-first promotion, whole-chapter delete, and bridge-delete persistence on reopen; a `checkMapWellFormed` invariant guards the offset map. Docker build + all 8 headless smokes green; VNC walk-through passed. `scrivi.h` untouched. | ✅ Verified |

## Verification

**AC1 + AC2 user-verified over Docker+VNC 2026-07-15.** Right-click → context menu → confirm →
scene/chapter removed from navigator + viewport; chapter delete warns "and all N scene(s)"; delete-of-active
promotes the nearest remaining scene with the editor keeping keyboard focus; last-scene/last-chapter guards
refuse with a notice; deletions persist across quit→reopen (real `.md` / index removal). Docker build + eight
headless smokes green (adds `scene_delete`); ScriviCore `ctest` + macOS/EP-020–022 unaffected; `scrivi.h`
unchanged.

---

*Archived 2026-07-15 on SP-065 close (user-approved). See `Sprints/Closed/Sprint-SP-065.md`.*
