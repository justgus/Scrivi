# Verified Issues — I-0071 to I-0080


---

## I-0071: [Linux] Dragging a chapter's last remaining scene orphans an empty chapter

**Status:** ✅ Resolved - **Verified (2026-07-18, VNC)** — last-scene drag leaves no empty/orphaned chapter (P6 scene model).
is allowed, and the vacated chapter gets a **new empty replacement scene** so it never becomes an empty
chapter. If the writer then wants the chapter gone, she deletes it explicitly.
**Platform:** Linux (`platforms/linux/`)
**Component:** `platforms/linux/src/EditorShell.cpp` (`onSceneDropped`); `SceneDocument::moveScene`
(the vacated-chapter case, `SceneDocument.cpp:443–444` already anticipates "target chapter has no segments
left"); `ScriviBridge` (needs a create-scene call in the vacated chapter).
**Severity:** Medium (data-structure hazard: `scrivi_reorder_scene` leaves the source chapter with zero
scenes; the navigator projects nothing for it, so it silently vanishes from the UI while it may persist on
disk — a chapter the writer can neither see nor manage)
**Sprint:** SP-067
**Epic:** EP-023 `[Linux]`
**Related:** I-0067/I-0068 (the drag path this rides on); I-0069 (same empty-chapter hazard on the split path
— apply the same "never leave an empty chapter" rule). Cross-platform: check whether macOS has the same gap.

**Description:**
Dragging the **only** scene of a chapter into a different chapter reassigns that scene's `chapterID` on disk
but does nothing about the now-empty source chapter — `scrivi_reorder_scene` neither deletes it nor backfills
a scene. The navigator, projecting from segments, renders nothing for the empty chapter, so it disappears from
view while potentially remaining on disk.

**Expected Behavior (user decision 2026-07-16):** the move succeeds; the vacated chapter is immediately given
a **new blank scene** (so it stays a visible, valid, empty-bodied chapter). Deleting the chapter is a separate,
explicit user action.

**Actual Behavior:** the source chapter is left with zero scenes (orphaned/empty); no replacement scene is
created.

**Root Cause:**
`scrivi_reorder_scene` is a pure reassignment primitive with no source-chapter bookkeeping, and the Linux drop
path (`onSceneDropped`) doesn't detect "this was the source chapter's last scene" to backfill. `moveScene`
already tolerates an emptied target chapter (`SceneDocument.cpp:443`) but nothing creates the replacement
scene on disk.

**5-Whys:**
1. *Why can a chapter end up empty?* — A reorder can remove a chapter's last scene with no compensating action.
2. *Why is there no compensating action?* — `scrivi_reorder_scene` only moves the scene; the app drop path
   doesn't check for or handle the "last scene of the source chapter" case.
3. *Why doesn't the app handle it?* — SP-067's scope covered moving scenes, not the empty-chapter side effect;
   the case wasn't enumerated in the AC4 exit criteria.
4. *Why wasn't it enumerated?* — Drag-reorder was specified around within/cross-chapter placement of the moved
   scene, not the state left behind in the source chapter.
5. *Why does an empty chapter matter?* — The navigator projects only from scene segments, so an empty chapter
   is invisible and unmanageable — a chapter the user can't see, select, or delete. **Root cause: reorder has
   no source-chapter-emptied policy, and the UI can't represent an empty chapter.**

**Fix direction (per user decision):** in `onSceneDropped`, after a successful cross-chapter `reorderScene`,
detect that the source chapter now has no scenes and `createScene` a blank scene in it (bridge call), then
re-splice/rebuild. Same "never leave an empty chapter" guard applies to the I-0069 split path. (Alternative
rejected by user: forbid the drag, or auto-delete the chapter.)

---

> ⚠️ **Entry/table conflict corrected at archive time (2026-08-15).** This entry's body still
> described the issue as open/root-caused; the Issue-active.md table row recorded it Verified with
> dated VNC evidence. Per user ruling the table is authoritative. Body status line updated; the
> original diagnostic narrative below is retained as written.

---

## I-0074

**Status:** ✅ **Verified (2026-07-18, VNC)** — all four split cases (mid-scene, mid-scene-with-followers, end-of-scene-with-followers, end-of-last-scene) update in the UI and **reopen cleanly with no warnings/errors**. ctest 302/302 macOS.
**Severity:** High
**Sprint:** **SP-071**

**Description / Resolution:**
`[Linux]` Chapter split (Ctrl+Shift+Return) corrupted the manuscript. **TWO defects.** (a) **Lost tail / stray folder:** the app did `createChapter` (append → `chapter-w`) then `reorderChapter` (rename → `chapter-c`), so the tail `saveScene` wrote to the **stale pre-rename path** and K0 stayed empty. Fix = **create-in-place** (`createChapter(afterChapterID)`). (b) **Same-named scene STOLEN from another chapter → "needs repair, missing metadata" on reopen:** after a follower reordered OUT of Ch2, `rebuildChapterScenesIfInconsistent` **preserved the stale ref**, then `migrateScenes` orphan-repair **matched orphans by filename** and dragged Ch1's identically-named `k-scene` into Ch2 (order-key scene filenames repeat across chapters). Fix = scene cache is a **pure disk mirror** (drop stale refs) + **removed filename-based orphan relocation** (ownership follows physical location; a project always opens self-consistent). Also: removed obsolete "renumber N chapters" dialog (C1), stopped deleting K0 at end-of-scene (C3), surfaced create-failure + **abort-on-any-failed-split-step** instead of silent continue (C4); **removed the split confirmation dialog entirely** (Ctrl+Shift+Return is the approval — a modal prompt broke drafting flow; the split is non-destructive).

> *Archived from the Issue-active.md table row (2026-08-15). This issue never had a separate
> full entry; the row above is the complete record as written at the time.*

---

## I-0075

**Status:** ✅ **Verified (2026-07-18, VNC)** — arrows cross scene/chapter boundaries in both directions.
**Severity:** Low
**Sprint:** **SP-071**

**Description / Resolution:**
`[Linux]` Arrow keys can't cross a scene/chapter boundary: pressing Down at a scene's end or Up at a chapter's start leaves the caret stuck. `ManuscriptEditor::normalizeCaret` snapped to the **nearest** editable edge, which (tie → previous) always snapped the caret **back** the way it came. **Fix:** directional snap — `SceneDocument::editablePositionInDirection(pos, movingForward)` snaps to the NEXT body start when travelling forward (Down/Right) and the PREV body end when backward (Up/Left); `normalizeCaret` picks direction from `pos` vs. the previous caret position. `nearestEditablePosition` kept for clicks/paste. Smoke-test assertions added (`editor_map_smoke`).

> *Archived from the Issue-active.md table row (2026-08-15). This issue never had a separate
> full entry; the row above is the complete record as written at the time.*

---

## I-0078

**Status:** ✅ **Verified (2026-07-18)** — fix applied to `Scrivi/Engine/ScriviEngine.swift`; `xcodebuild ScriviApp` BUILD SUCCEEDED and the user ran the rebuilt app (opened a project).
**Severity:** High
**Sprint:** **EP-027**

**Description / Resolution:**
`[Apple]` **macOS app failed to build — `ScriviEngine.swift` `createChapter` wrapper drifted behind the C ABI.** SP-071 (I-0074 create-in-place) added a 7th `afterChapterID` parameter to `scrivi_create_chapter` in `scrivi.h` and updated only the Linux bridge; `ScriviEngine.createChapter` still called it with 6 args → `error: missing argument for parameter #7` (`ScriviEngine.swift:601`). The macOS target had not compiled since c949d0b. **Fix:** thread `afterChapterID: String = ""` (empty ⇒ append, preserving prior macOS behavior) through the wrapper via `withCString`; both macOS callers only append, so no other change. Build succeeds. Found at the start of P5.

> *Archived from the Issue-active.md table row (2026-08-15). This issue never had a separate
> full entry; the row above is the complete record as written at the time.*

---

## I-0079

**Status:** ✅ **Verified (2026-07-18)** — fix in `platforms/linux/tests/scene_reorder_smoke.cpp` (test-only) committed as `1ca59eb`; **Linux App CI #15 (run 29662554648) SUCCESS**, ending the red streak that ran since 2026-07-16.
**Severity:** Medium
**Sprint:** —

**Description / Resolution:**
`[Linux]` **Scene-reorder smoke test fails on CI** (`FAIL: E: tail landed in K's first scene`) — Linux App CI red since "EP-027 P1 and P2 complete" (2026-07-16). `scene_reorder_smoke.cpp` Case E (mid-scene chapter-split) replayed the **old** split orchestration: `createChapter()` (append) → `reorderChapter(K, ch1)` → `saveScene(tail, firstSceneMetadataPath/ContentPath captured from the createChapter result)`. Under EP-027 P2, `reorderChapter` **reslugs K's folder**, so the captured scene paths went stale and the `saveScene` wrote to the vanished path → the tail was lost on reopen (the same I-0074 stale-path class, but in the test's own manual orchestration). **Fix:** both Case D and Case E now **create K in place** — `createChapter(projectID, afterChapterID=ch1)` (the app's post-I-0074 orchestration) — so K's folder is born in its final position and no reslug invalidates the paths. Reproduced + fixed in the Qt 6.4 Docker container; all 10 Linux console smokes green.

> *Archived from the Issue-active.md table row (2026-08-15). This issue never had a separate
> full entry; the row above is the complete record as written at the time.*

---

## I-0080

**Status:** ✅ Resolved - **Verified (2026-07-19, VNC round 2)** — chapter reorder persists across quit→reopen (the exact failure mode); ctest **306/306 macOS, 313/313 Linux**; **11/11** Linux smokes
**Severity:** High
**Sprint:** **SP-073**

**Description / Resolution:**
`[ScriviCore]` **Open-time chapter migration UNDOES a legitimate chapter reorder** — `migrateChapterOrderKeys` ran on every open with **no legacy gate**: after a `scrivi_reorder_chapter` (whose index-array order is stale by design under B3 — disk is the authority, the array a cache), the next open treated the disk-vs-array disagreement as "legacy project needing migration" and **reslugged the folders back to the stale array order**, reverting the user's reorder. Previously masked only because the reslug usually crashed into `renameChapterFolder`'s no-clobber guard and silently aborted (`ProjectOpener` discards its result). Found by SP-073's new `chapter_reorder_smoke` (Case C move-to-front) and reproduced at core level (`ReorderTests` `[SP-073]` — move-between survived only by no-clobber luck; move-to-front was undone by the following open). **Fix:** (a) **legacy gate** — the reslug runs only when EVERY on-disk chapter folder key is digits-only (the pre-EP-027 creator's shape); any letter key = new scheme → skip (a stale array is `rebuildIndexIfInconsistent`'s job); (b) **eager cache coherence** — `ChapterReorderer` + `ChapterCreator` call `rebuildIndexIfInconsistent` after their folder ops so the array order always matches disk (create-in-place's blind append had the same staleness). I-0077's regression fixture reshaped to a genuine all-numeric legacy project (matching the real `the-twisted-remains` case). Regression test proven RED without the fix.

> *Archived from the Issue-active.md table row (2026-08-15). This issue never had a separate
> full entry; the row above is the complete record as written at the time.*
