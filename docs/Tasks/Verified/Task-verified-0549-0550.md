# Verified Tasks — T-0549, T-0550

**Sprint:** [SP-150] · **Epic:** [EP-040] `[Apple]` The Editor Shell
**Verified:** 2026-09-24 (user-authorized, live pass on the real rig)

⚠️ **WORK ITEMS REFERENCED, ONE LINE EACH.**
✅ **[I-0213]** `[Apple]` — *creating a chapter froze the app ~2.7 s on a 1,174-scene manuscript;*
*verified 2026-09-24 at `396.7 ms` of work and ACCEPTED as a limitation.* ⚠️ **These Tasks attacked
that accepted remainder.**
✅ **[EP-039]** `[Cross]` *Project Load Performance* — *replaced a `~300 s` frozen open.* ✅ **Its
**AC4** shipped `scrivi_list_story_times` (the sparse bulk call); ✅ its **AC5b** is the
index-correctness clause both Tasks had to preserve.*
✅ **[EP-040] AC12** (T-0549) and **AC13** (T-0550) — *both added with [SP-150].*

---

## T-0549 — Class A: patch the index for single-scene ops

✅ **`ProjectIndex::patchStoryTime`** + **`StoryTimePatchGuard`**, wired into
`scrivi_set_scene_story_time` and `scrivi_clear_scene_story_time`.
⚠️ **`scrivi_rename_scene` was DROPPED from scope** — ⛔ **it takes a `metadataPath`, not a
`sceneID`, so patching it needs a path→scene lookup the other two do not.** ✅ **Left dropping the
whole index: correct, just not cheap.**

✅ **MEASURED: the patch costs ZERO filesystem calls**, ⛔ **against 866 for the forced rebuild.**
⚠️ **The patch RE-READS the one changed sidecar rather than reconstructing the value from the C
arguments** — ⛔ **`setSceneStoryTime` PRESERVES `bandID`, `bandAssignedAt` and `durationMs` when the
request passes 0, so rebuilding from arguments would store a story time the disk does not hold.**

## T-0550 — Class B: rebuild only the affected chapter

✅ **`ProjectIndex::rebuildChapters`** + **`ChapterRebuildGuard`**, wired into SEVEN endpoints:
`create_scene` · `create_chapter` · `delete_chapter` · `reorder_scene` · `reorder_chapter` ·
`merge_scene` · `merge_chapter`.

⛔ **SIX CLASS B ENDPOINTS DELIBERATELY STILL DROP THE WHOLE INDEX — A RULING, NOT AN OVERSIGHT.**
⚠️ **`delete_scene`, `rename_scene`, `rename_chapter`, the three `fragment_*` calls and
`apply_repair` DO NOT REPORT THE AFFECTED CHAPTER.** ⛔ **The tempting wrong fix — look it up from the
CACHED index BEFORE the write — is the pre-write-state trap that produced the `count:0` defect.**
✅ **Widening those public result structs is honest but SEPARATE work.**

✅ **MEASURED AT THE CORE: a full rebuild costs `861` filesystem calls on 400 scenes / 20 chapters;
`rebuildChapters` for ONE chapter costs `63`** — ✅ **13.7x cheaper.**

⛔ **THE FIRST IMPLEMENTATION WAS INERT, AND A PROBE — NOT A CODE READ — FOUND IT.**
⚠️ **The guard's before-drop (load-bearing for AC5b) destroyed the very index the partial rebuild
needed, so every structural op saw an empty cache and fell back to a full build.** ⛔ **The fast path
NEVER RAN on the op it was written for, while the index-layer perf test still reported 13.7x.**
✅ **FIXED by having the constructor TAKE the index rather than erase it.**
⚠️ **CAUGHT BY A NEGATIVE CONTROL: deliberately dropping carried-over story times passed 30/30 before
the fix and FAILS 7 assertions after it.** ✅ **`feedback_prove_code_is_reached`, fourth occurrence —
and the first time the check ran BEFORE success was reported.**

---

## ✅ LIVE PASS 2026-09-24 — the user's verification

⚠️ **RIG: `dumas-prose-timelines` on the `/Volumes/SCRIVI-OTHE` USB mount, 1,177 → 1,180 scenes.**
✅ **THREE chapter creates in ONE session: two at the START of the manuscript, one at the END.**

| Figure | Before (2026-09-24 USB) | After |
| ------ | ----------------------- | ----- |
| ⚠️ **`reloadSceneDots`** | **235.9 ms** | ✅ **`1.4 ms`** (~170x) |
| ✅ `updateDotTitles` | 1.1–1.2 ms | ✅ `1.2 ms` (held) |
| ⚠️ `engine.createChapter` (C ABI) | 70.6–113.6 ms | ⚠️ `80.5 ms` |
| ⚠️ `createChapter WORK` (insert at START) | 396.7 ms | ⚠️ `316.3 ms` |

## ✅ THE USER EXPLAINED THE REMAINING ~155 ms, AND THE CODE CONFIRMS IT

⚠️ **THE USER'S READING:** *"the first and second create chapter were performed at the beginning of
the document… every chapter following the inserted one must be renamed after the insertion. That
requires walking the index. The third insert at the end… required only one rename."*

✅ **CONFIRMED IN CODE, NOT TAKEN ON TRUST** — `ViewportSceneLoader.renumberChapterTitlesFrom`
(`:795`): ⚠️ **it builds `titleForChapter` for EVERY chapter from the insertion point ONWARD
(`startOrdinalIdx ..< orderedChapterIDs.count`) and then rebuilds the whole `allScenes` array.**
✅ **Insert at the START ⇒ all ~52 chapters renumbered and ~1,178 rows rebuilt.**
✅ **Insert at the END ⇒ effectively one chapter, and the guard `rebuilt[j].chapterTitle != newTitle`
skips every unchanged row.**
⚠️ **THE COST IS THEREFORE POSITION-DEPENDENT BY DESIGN, NOT A DEFECT** — ✅ **and it is SWIFT-SIDE:
`ChapterCreator.cpp` writes only the NEW chapter's sidecar, so the core does not renumber at all.**
⛔ **THIS WAS ALWAYS PRESENT; it was HIDDEN BEHIND `reloadSceneDots` rather than caused by it.**

✅ **`ctest` 637/637 · build clean, no warnings.**
✅ **[EP-039] AC5b's own test passes unchanged (56 assertions, 4 cases).**
