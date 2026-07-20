## SP-073: [Linux] Chapter drag-reorder (AC5) + full EP-023 verification & Epic close

**Status:** ✅ **VERIFIED — closed (Human-approved 2026-07-19).**
**Epic:** EP-023 `[Linux]` Manuscript Structure Editing (final sprint — closes the Epic)
**Goal:** Chapter rows drag as **containers** (chapter + all its scenes move as a unit) via
`scrivi_reorder_chapter`, with a boundary-unambiguous insertion-line drop highlight (**AC5**); then run the
full EP-023 verification pass (**AC6/AC7/AC8** — create parity, full
create→rename→reorder→delete→quit→reopen loop over VNC, no-regression sweep) and prepare EP-023 for close.
**Start Date:** 2026-07-19 | **End Date:** 2026-07-19 | **Capacity:** ~1–2 days (actual: 1 day)

> **Renumbered from SP-068** (ID skipped when EP-027 claimed SP-069–SP-072). EP-027 sequencing dependency
> satisfied — EP-027 ✅ closed 2026-07-18.

### Assigned Tasks

| ID     | Title | Priority | Status |
| ------ | ----- | -------- | ------ |
| T-0294 | `[Linux]` `NavigatorTree` chapter drag — chapter rows draggable (latch `kChapterIDRole` at `startDrag`, CopyAction-only), chapter-boundary drop resolution + insertion-line highlight, no-op guard; emit `chapterDropRequested(chapterID, afterChapterID)` | High | ✅ **Verified (2026-07-19, VNC round 2)** — incl. the I-0082 selectable-rows fix |
| T-0295 | `[Linux]` `EditorShell::onChapterDropped` + `SceneDocument::moveChapter` — save dirty scene, bridge `reorderChapter`, contiguous chapter-block re-splice (bodies/caret/scroll preserved), navigator rebuild, **post-reslug path refresh** | High | ✅ **Verified (2026-07-19, VNC round 2)** — chapter move persists across quit→reopen (exercises the I-0080 fix) |
| T-0296 | `[Linux]` Headless chapter-reorder smoke — reorder among ≥3 chapters (incl. move-to-front and move-to-last), reopen, assert order + all scene bodies intact + no stale paths; wired to CI | Medium | ✅ **Verified (2026-07-19)** — 11/11 container smokes green; CI step added; surfaced I-0080 |
| T-0297 | `[Linux]` Full EP-023 verify + Epic close prep — AC6 create parity, AC7 full structure loop over VNC with on-disk checks, AC8 regression sweep (ctest both platforms, macOS build, EP-020–EP-022 flows), draft EP-023 completion summary | High | ✅ **Verified (2026-07-19, VNC round 2)** — full checklist + AC7 loop passed, no errors; completion summary drafted in `Epic-active.md` |

### Implementation summary (2026-07-19)

- **ScriviCore (payload + I-0080 fix; `scrivi.h` untouched):** `ReorderChapterResult` gains
  `metadataPath` (the chapter's post-reslug sidecar path) and the `reorder_chapter` envelope reports it —
  the app's stale-path refresh contract. **I-0080** found & fixed: `migrateChapterOrderKeys` was undoing
  legitimate reorders on the next open (no legacy gate; stale index array hijacked as order authority) —
  now gated to digits-only legacy folder sets, and `ChapterCreator`/`ChapterReorderer` keep the index
  array coherent via `rebuildIndexIfInconsistent`. New `[SP-073]` core regression (proven RED pre-fix);
  I-0077 fixture reshaped to a genuine all-numeric legacy project.
- **Linux app:** `NavigatorTree` — chapter rows drag via a manual `QDrag` (rows are non-selectable, so
  the base selected-index drag can't run), CopyAction-only, `resolveChapterDrop` = chapter boundaries
  only (Above a heading → before it; below the last scene / empty area → after the last chapter), no-op
  guards. `SceneDocument::moveChapter` — contiguous block splice (bodies + member order preserved, reflow
  fixes headings/ordinals) + `refreshChapterPaths` (re-bases the moved chapter's scene paths onto the
  reslugged folder). `EditorShell::onChapterDropped` — save-dirty-first, bridge reorder, path refresh,
  block re-splice, I-0063 renumber, navigator rebuild, caret re-anchor.
- **Verification so far:** new `chapter_reorder_smoke` (block splice + persistence + stale-path
  refresh — the save-through-refreshed-paths case) + CI step; **11/11** Linux smokes in the Qt 6.4
  container; ctest **306/306 macOS**, **313/313 Linux**; macOS `ScriviApp` BUILD SUCCEEDED.
  Remaining: user VNC walkthrough (AC5 drag feel/highlight, AC6 create parity, AC7 full loop) + Epic
  close approval.

### VNC walkthrough round 1 (2026-07-19) — two defects found, both fixed same day

The user's first walkthrough passed create / rename (unmoved items) / scene reorder / delete /
quit→reopen persistence, and surfaced two defects:

- **I-0081** `[ScriviCore]`+`[Linux]` — a scene could not be renamed after a drag-reorder (until
  quit→reopen). Under EP-027 §8 the reorder renames/relocates the scene's files, but the envelope
  reported no paths and `onSceneDropped` (SP-067-era, pre-§8) kept the stale segment paths. **Fix:**
  `reorder_scene` envelope now reports post-move `metadataPath`/`contentPath`/`chapterMetadataPath`
  (payload-only); `SceneDocument::refreshScenePaths` applies them. Core regression + smoke coverage
  (rename+save through refreshed paths).
- **I-0082** `[Linux]` — the chapter drag never started (heading un-draggable; rubber-band selected the
  nearest scene; pale-blue selection box). Qt enters DraggingState only when the pressed row is among
  the SELECTED draggable indexes — chapter rows were non-selectable, so the SP-073 drag path was
  unreachable. **Fix:** chapter rows are now selectable; chapter clicks remain inert.

Post-fix: ctest **306/306 macOS + 313/313 Linux**, **11/11** smokes, macOS app builds.

### VNC walkthrough round 2 (2026-07-19) — ✅ PASSED, no errors

User re-verified the full checklist on the rebuilt container: chapter drag-as-container (selection now
teal; moves with all scenes; **persists across quit→reopen** — the I-0080 failure mode), scene drag +
**immediate rename** (I-0081 fixed, no restart needed), create parity, and the complete AC7
create→rename→reorder→delete→quit→reopen loop. **All four tasks + I-0080/I-0081/I-0082 marked Verified;
EP-023 ACs AC5–AC8 Verified → Epic 🟠 Complete.**

### Retrospective

**Completed:** T-0294–T-0297 (all ✅ Verified); AC5/AC6/AC7/AC8 delivered → **EP-023 Complete**; three
defects found & fixed & Verified en route (I-0080 core migration hijack, I-0081 stale scene paths,
I-0082 undraggable chapter rows). New CI coverage: `chapter_reorder_smoke` + extended
`scene_reorder_smoke` (rename/save through refreshed paths).

**Returned to backlog:** none.

**What went well:** the planned stale-path risk (Note 2) was real and the planned mitigation (envelope
paths + refresh) worked first try for chapters — and the same pattern closed I-0081 for scenes within
hours of the VNC report. The new smoke caught I-0080 (a latent EP-027 defect that had been masked by the
no-clobber guard) before the user ever saw it. The two VNC-only defects were exactly the class headless
smokes can't reach (real mouse drags, Qt selection machinery) — the split verification model (smokes +
VNC) did its job.

**What to improve:** two of the three defects were *interaction* gaps between EP-027's new on-disk
behavior and pre-EP-027 app code (SP-067's scene drag, SP-061's non-selectable rows). When a core Epic
changes on-disk semantics, sweep the app layer for every consumer of the changed contract (paths, in this
case) instead of trusting the original sprint's verification.

**Carry-forward notes:** I-0073 (VNC drag-hover lag) remains 🔵 Open — needs a real-hardware datapoint
from the alpha tester; chapter drag showed the same environmental lag profile. The stale Statistics
blocks in `Issue-Documentation.md`/`Task-Documentation.md` predate a re-audit (flagged in both files).

### Sprint Notes

1. **Drag discipline inherited from SP-067 (I-0067/I-0068):** `Qt::CopyAction` only; `rebuildNavigator` is
   the only thing that mutates the tree; a backend-rejected reorder leaves it untouched; dragged chapterID
   latched at `startDrag`. Legal landings are **chapter boundaries only**; no-op landings refused.
2. **Stale-path hazard (I-0074/I-0079 lesson):** `reorder_chapter` reslugs the moved chapter's folder;
   envelope returned only `{chapterID, reordered}`. **Decision (activation, 2026-07-19): recommended option
   adopted** — add the chapter's new paths to the reorder envelope (payload-only ScriviCore change,
   `scrivi.h` untouched) and rewrite the moved chapter's segment path prefixes in the app.
3. **"No ScriviCore work" softened, not broken** — Note 2 touches envelope payload code only; `scrivi.h`
   unchanged (AC8 as written).
4. **`ScriviBridge::reorderChapter` already exists** (SP-067/T-0258) — no new bridge wrapper needed.
5. **I-0073 (VNC drag-hover lag)** stays 🔵 Open — not an SP-073 exit criterion; note whether chapter drag
   shows the same lag.
6. **Epic close needs user approval** (T-0297 drafts the summary; ✅ Closed is the user's call).

**Exit criteria:** a chapter dragged in the navigator moves with all its scenes to the indicated position,
the viewport re-splices, and the new order **persists across quit→reopen** (VNC-verified, incl. saves to the
reslugged folder); the full create→rename→reorder(scene+chapter)→delete→quit→reopen loop passes with real
on-disk checks; headless smokes green in Docker/CI; ScriviCore ctest green on both platforms; macOS app
unaffected; EP-023 completion summary drafted, Epic awaiting close approval.
