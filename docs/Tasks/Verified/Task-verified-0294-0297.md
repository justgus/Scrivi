# Verified Tasks: T-0294 – T-0297 (SP-073, EP-023 `[Linux]`)

All four tasks delivered SP-073 — **chapter drag-reorder (AC5) + full EP-023 verification** — and were
✅ **Verified 2026-07-19** (user VNC walkthrough round 2: full checklist + AC7 loop passed, no errors).
Sprint closed 2026-07-19 (Human-approved); **this closed EP-023** (`Epics/Closed/Epic-EP-023.md`).
Sprint detail + retrospective: `Sprints/Closed/Sprint-SP-073.md`.

---

## T-0294: [Linux] `NavigatorTree` chapter drag

**Status:** ✅ Verified (2026-07-19, VNC round 2)
**Sprint:** SP-073 | **Epic:** EP-023 `[Linux]`

Chapter rows drag as containers: `kChapterIDRole` latched at `startDrag` (manual `QDrag`, CopyAction-only
per the I-0067/I-0068 discipline — Qt never mutates the model), `resolveChapterDrop` restricts landings to
chapter boundaries (Above a heading → before that chapter; below the manuscript's last scene / empty
viewport → after the last chapter), with no-op guards; emits `chapterDropRequested(chapterID,
afterChapterID)`. Round 1 found **I-0082** — chapter rows were non-selectable (SP-061), and
`QAbstractItemView` enters DraggingState only when the pressed row is among the selected draggable
indexes, so the drag path was unreachable (rubber-band selected the nearest scene instead). Fixed by
making chapter rows selectable; chapter clicks remain inert (`onNavigatorActivated` ignores rows without a
sceneID).

**Files:** `platforms/linux/src/NavigatorTree.{hpp,cpp}`, `EditorShell.cpp` (`rebuildNavigator`).

## T-0295: [Linux] `EditorShell::onChapterDropped` + `SceneDocument::moveChapter`

**Status:** ✅ Verified (2026-07-19, VNC round 2 — persists across quit→reopen)
**Sprint:** SP-073 | **Epic:** EP-023 `[Linux]`

`onChapterDropped`: save dirty scenes → `ScriviBridge::reorderChapter` → **post-reslug path refresh**
(`SceneDocument::refreshChapterPaths` off the reorder envelope's new `metadataPath` — the reorder reslugs
the chapter's folder, the I-0074/I-0079 stale-path class) → `SceneDocument::moveChapter` contiguous
block re-splice (bodies + member order preserved; `reflowBoundaryAt` fixes the moved heading + follower;
`reflowAllChapterHeadings` renumbers ordinals) → I-0063 renumber → navigator rebuild → caret re-anchor.
ScriviCore: `ReorderChapterResult`/envelope gained `metadataPath` (payload-only; `scrivi.h` untouched).
The same sprint also closed **I-0081** (scene drag left stale segment paths → rename/save failed until
reopen): `ReorderSceneResult`/envelope now report post-move `metadataPath`/`contentPath`/
`chapterMetadataPath`, applied via `SceneDocument::refreshScenePaths` in `onSceneDropped`. And the
persistence half surfaced **I-0080** (`[ScriviCore]` open-time migration reslugged folders back to the
stale index-array order, undoing the reorder) — fixed with a digits-only legacy gate in
`migrateChapterOrderKeys` + eager index-cache rebuild in `ChapterCreator`/`ChapterReorderer`.

**Files:** `platforms/linux/src/EditorShell.{hpp,cpp}`, `SceneDocument.{hpp,cpp}`;
`ScriviCore/include/scrivi/Results.hpp`, `src/manuscript/{ChapterReorderer,ChapterCreator,SceneReorderer}.cpp`,
`src/manuscript/ChapterIndex.{hpp,cpp}`, `src/public_api/scrivi_c_api.cpp`.

## T-0296: [Linux] Headless chapter-reorder smoke

**Status:** ✅ Verified (2026-07-19 — 11/11 container smokes; CI step added; surfaced + locked I-0080)
**Sprint:** SP-073 | **Epic:** EP-023 `[Linux]`

New `chapter_reorder_smoke` (4-scene/3-chapter fixture): `moveChapter` block splice (map well-formed,
bodies + member order preserved, no-op landings refused), `reorderChapter` persistence (reopen order +
every body), and the stale-path refresh (a save **through the refreshed paths** lands). Case C
(move-to-front) caught I-0080 before any user saw it. `scene_reorder_smoke` Case C extended for I-0081
(rename + save through refreshed paths after a cross-chapter scene reorder). Core regressions:
`ReorderTests` `[SP-073]` (move-between/front/last with `openProject` interleaved — RED pre-I-0080-fix)
and `[I-0081]` (post-move paths reported + rename through them); I-0077's fixture reshaped to a genuine
all-numeric legacy project.

**Files:** `platforms/linux/tests/chapter_reorder_smoke.{cpp,sh}`, `scene_reorder_smoke.cpp`,
`platforms/linux/CMakeLists.txt`, `.github/workflows/scrivi-linux-ci.yml`,
`ScriviCore/tests/integration/{ReorderTests,SceneIdentityTests}.cpp`.

## T-0297: [Linux] Full EP-023 verify + Epic close prep

**Status:** ✅ Verified (2026-07-19, VNC round 2 — full checklist + AC7 loop, no errors)
**Sprint:** SP-073 | **Epic:** EP-023 `[Linux]`

AC6 create parity (Ctrl+Return / Ctrl+Shift+Return) ✓; AC7 full
create→rename→reorder(scene+chapter)→delete→quit→reopen loop over VNC with on-disk persistence ✓; AC8
regression sweep — `scrivi.h` untouched, ctest **306/306 macOS + 313/313 Linux**, macOS `ScriviApp`
BUILD SUCCEEDED, EP-020–EP-022 flows exercised throughout ✓. EP-023 Completion Summary drafted
(`Epics/Closed/Epic-EP-023.md`); Epic closed with user approval 2026-07-19.
