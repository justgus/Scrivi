# Active Tasks

**Epic:** EP-028 `[Cross]` (Scene & Chapter Merging) — **Sprint:** SP-074 `[ScriviCore]` (`Sprints/Sprint-active.md`).

| ID | Title | Status |
| -- | ----- | ------ |
| T-0298 | **Reproduce the chapter-merge data-loss bug (I-0083)** — new `ScriviCore/tests/integration/MergeSceneTests.cpp`: build a 2-chapter project, replay the app's compose-merge (in-memory reassign + `deleteChapter`), reopen via `ManuscriptOrderResolver::resolve`, assert scene 2 is LOST (`size==1`, ch2 folder gone, scene-2 body deleted on disk). Also assert same-chapter scene-merge (`saveScene`+`deleteScene`) survives reopen with both bodies. Registered in `tests/CMakeLists.txt` (CMake-only; ScriviCore integration tests are not in pbxproj). ctest **308/308 macOS**. | 🟢 Implemented, Not Verified |
| T-0299 | **`scrivi_merge_scene`** — `SceneMerger` impl (`src/manuscript/SceneMerger.{hpp,cpp}`), `MergeSceneRequest`/`MergeSceneResult`, facade `ScriviCore::mergeScene`, C dispatcher in `scrivi_c_api.cpp` (mirror `scrivi_reorder_scene`), `scrivi.h` decl, CMake source entry. Same-chapter join into the previous scene; survivor keeps its own order-key files, merged body appended, merged files removed, chapter cache rebuilt. First-scene/empty/unknown sceneID → `invalidArgument`. 4 facade tests; `_scrivi_merge_scene` exported in `libScriviCore.a`. ctest **312/312 macOS**. **No pbxproj change** — the Xcode app links the prebuilt `libScriviCore.a`, it does not compile ScriviCore `.cpp` files. | 🟢 Implemented, Not Verified |
| T-0300 | **`scrivi_merge_chapter`** — atomic whole-chapter merge: `ChapterMerger` relocates every scene file into the predecessor folder (mint `keyAfter` the survivor's last scene, rename `.meta.json`+`.md` like `SceneReorderer`, rewrite sidecar slug/contentPath), rebuild survivor cache, remove the emptied chapter via `ChapterDeleter` (folder + `manuscript.meta.json` entry), `rebuildIndexIfInconsistent`. `MergeChapterRequest`/`MergeChapterResult`, facade, C ABI `scrivi_merge_chapter`, `scrivi.h`, CMake. First-chapter/empty/unknown → `invalidArgument`. **Fixes I-0083** (proven: 4-scene 2-chapter merge → all 4 survive in order after reopen). 4 tests; `_scrivi_merge_chapter` exported. ctest **316/316 macOS**. | 🟢 Implemented, Not Verified |
| T-0301 | **Integration coverage + `ctest` green** — 11 tests in `MergeSceneTests.cpp`: same-chapter scene merge (+ empty-scene body elision), cross-chapter whole-chapter merge (4-scene/2-chapter, order + bodies preserved), manuscript-start / first-in-chapter no-op, empty/unknown ID errors, reopen round-trip via `ManuscriptOrderResolver`; the I-0083 loss test guards the old `deleteChapter`-composed path. Registered in `tests/CMakeLists.txt`. **ctest green: macOS 317/317, Linux (Ubuntu 24.04/GCC 13) 324/324.** Also fixed a stale devops file: `devops/docker/linux/Dockerfile` was missing `libssl-dev` (OpenSSL 3, required since SP-059) so the local Linux harness couldn't configure — added it to match GitHub CI. | 🟢 Implemented, Not Verified |

**Next available T-0302.**

---

**SP-073 ✅ closed 2026-07-19** (EP-023 `[Linux]` — chapter drag-reorder AC5 + full EP-023 verify; **this
closed EP-023**). T-0294–T-0297 all ✅ Verified & archived to `Verified/Task-verified-0294-0297.md`; fixed
& Verified en route: I-0080 (`[ScriviCore]` open-time migration undid chapter reorders), I-0081 (stale
scene paths after drag), I-0082 (undraggable chapter rows). Sprint record:
`Sprints/Closed/Sprint-SP-073.md`. **Next available T-0298.**

---

**SP-072 ✅ closed 2026-07-18** (EP-027 `[Apple]` — **P5 Apple-verify migration fixes**). Fixed I-0078 (macOS
`createChapter` wrapper drift), I-0076 (legacy scene `content.path` bared on read + stale sidecars normalised on
migration), I-0077 (id-mismatched chapter path-folder fallback). 2 regression tests (RED-without-fix). ctest
**304/304 macOS + 311/311 Linux**; real legacy project opens clean. Full task table in `Closed/Sprint-SP-072.md`
(T-0290–T-0293). **Next available T-0294.**

---

**SP-070** (EP-027 `[ScriviCore]` — **P6 filesystem-authoritative scene identity & ordering**) is
🟢 Implemented, Not Verified (2026-07-17). Trade study **§8** (Human-approved 2026-07-17). `SceneRef` filename-only
(identity derived by sceneID sidecar scan); order-key scene filenames; cross-chapter reorder **relocates the files**;
chapter-folder rename touches **zero** scene fields; scene migration + orphan-repair on open (**before** validation)
dissolves the **C6** "Missing scene.meta.json". `scrivi.h` untouched; ctest **298/298 macOS** (Linux-container parity
pending). Full task table in `Sprints/Sprint-active.md` (T-0271–T-0277). **Next available T-0278** (P4 Linux verify,
now unblocked).

---

**SP-069** (EP-027 `[ScriviCore]` — **P1 rename primitive + P2 order-key/disk-authority + P3 migration**) is
🟢 Implemented, Not Verified (2026-07-16). `scrivi.h` untouched; ctest green **290/290 macOS + Linux**. **I-0072
root cause fixed + regression + open-time self-heal for existing damage + lazy migration of legacy `chapter-NNN`
projects.** Full task table in `Sprints/Sprint-active.md` (T-0264 P1; T-0265–T-0269 P2; T-0270 P3). **Deferred:**
drop `chapterID` from `ChapterRef` schema (self-healing cache — churn without gain; trade study §7.6).

**Epic:** EP-027 `[ScriviCore]` — **Sprint:** SP-069 (`Sprints/Sprint-active.md`). (P6/SP-070 consumed
T-0271–T-0277; next available **T-0278**.)

---

_Prior:_ **SP-067 ✅ closed 2026-07-16** — delivered **AC4 scene drag-reorder** (T-0260, verified;
I-0067/I-0068 fixed via `Qt::CopyAction`) + **I-0063 renumber** (T-0262, verified). Its chapter-split task
**T-0261 (I-0064)** was **superseded → EP-027** (the split path is rebuilt on EP-027's new on-disk model), along
with I-0069/I-0070/I-0072/I-0071. SP-067's verified tasks (T-0258/259/260/262/263) archive with the sprint
(`Sprints/Closed/Sprint-SP-067.md`). **No ScriviCore work in SP-067**; `scrivi.h` untouched.

**Next:** two Epics Active — **EP-027** `[ScriviCore]` (recommended next: P1 FileSystem rename/move primitive)
and **EP-023** `[Linux]` (remaining SP-068, sequenced against EP-027). Tasks assigned at the next sprint's
activation; next available **T-0264**.

The SP-067 task table below is retained for reference until archived with the sprint.

| ID | Title | Status |
| -- | ----- | ------ |
| T-0258 | **Reorder bridge wrappers** — `ScriviBridge::reorderScene(...)` / `reorderChapter(...)` (`scrivi_reorder_scene`/`_chapter`, `parseEnvelope`, RAII `scrivi_free`, `errorOccurred`, `ready_` guard; no identity). | ✅ Done (not verified) |
| T-0259 | **`SceneDocument::moveScene` re-splice** — lift a scene body span (+ one boundary) and re-insert at the destination with the correct heading/separator; re-promote/demote chapter-first scenes; fix all `bodyStart`s; `reflowAllChapterHeadings()`. Programmatic-window guarded. | ✅ Done (not verified) |
| T-0260 | **`NavigatorTree` scene drag-drop + insertion-line highlight** — new `QTreeView` subclass: drag scene rows; drop indicator; `dragMoveEvent` legality gate; on drop resolve `(draggedSceneID, targetChapterID, afterSceneID)` → `sceneDropRequested`; `EditorShell::onSceneDropped` does `saveDirtyScenes()` + `bridge_->reorderScene` + `moveScene` + `rebuildNavigator()` + re-anchor caret. Chapter-row drag disabled (SP-068). | ✅ Done (not verified) |
| T-0261 | **I-0064 — Ctrl+Shift+Return splits the chapter at the caret** — orchestrate `create_chapter` → `reorder_chapter(K, afterC)` → per-following-scene `reorder_scene` into K → (mid-scene) `save_scene` head/tail (end-of-scene w/ followers drops blank K0; no followers keeps it) → reload → `renumberCreatedChapters` → renumber confirmation. | 🔵 **Superseded → EP-027** (2026-07-16) — VNC: mid-scene worked, end-of-scene failed (I-0069/I-0070); the split path is rebuilt on EP-027's new on-disk model. Not verified on the old model. |
| T-0262 | **I-0063 — renumber created chapters on structural change** — `renumberCreatedChapters()`: `renameChapter` each later chapter whose stored title matches `^Chapter \d+$` to its new ordinal (Option A); custom untouched; untitled already handled. Wired into `deleteChapterByID` + T-0261; also fixes `load()` to copy `chapterMetadataPath`. **Closes I-0063.** | ✅ Done (not verified) |
| T-0263 | **Verify AC4 + I-0063/I-0064 + `scene_reorder_smoke`** — new headless smoke (within/cross-chapter move, chapter-first re-promotion, offset map well-formed, cross-chapter persistence, I-0064 split end/mid-scene, I-0063 renumber) + CMake target + CI step. **Docker build + `scene_reorder_smoke` + all prior smokes green (2026-07-15).** VNC walk-through user-pending; `scrivi.h` untouched; macOS/EP-020–022 + SP-065/066 unaffected. | ✅ Done (smokes green; VNC user-pending) |

**Epic:** EP-023 `[Linux]` (Manuscript Structure Editing) — Active. **Sprint:** SP-067 (`Sprints/Sprint-active.md`).
Previous sprint SP-066 (rename) ✅ closed; T-0254–T-0257 Verified & archived to
`Verified/Task-verified-0254-0257.md`.

---

*Last Updated: 2026-07-15 (SP-067 implemented — T-0258–T-0263 all done; scene drag-reorder (AC4) via new
`NavigatorTree` + `SceneDocument::moveScene`, I-0064 chapter-split, I-0063 renumber; Docker build +
`scene_reorder_smoke` + all prior smokes green; VNC user-verification pending; no ScriviCore work, `scrivi.h`
untouched. Next available T-0264.)*
