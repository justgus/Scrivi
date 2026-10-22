# Verified Task — T-0619

**Sprint:** [SP-191] · **Epic:** [EP-053] · **Platform:** `[Cross]`
**Implemented:** 2026-10-21 · ✅ **USER-VERIFIED 2026-10-22** (user, 2026-10-22: *"Yes, mark them verified and close SP-191"* — after the rig re-pass on build 79: *"all items pass"*; the Mac pass 2026-10-21) — SP-191 AC11: the Mac and rig live passes over the share.
**Archived:** 2026-10-22, on the user's instruction; the Sprint closed the same day (user-approved).

---

## T-0619 — `[Cross]` EP-053 S2b: copy, paste and its undo, split and merge — as deltas, off the main thread, no reload

**Epic:** [EP-053] (S2b) · **Status:** ✅ **Verified 2026-10-22** (user, 2026-10-22: *"Yes, mark them verified and close SP-191"* — after the rig re-pass on build 79: *"all items pass"*; the Mac pass 2026-10-21) · **Sprint:** [SP-191] →
[`../../Sprints/Closed/Sprint-SP-191.md`](../../Sprints/Closed/Sprint-SP-191.md) · **Date Requested:** 2026-10-20 (from [SP-187]'s Q1 ruling: split S2)

**What:** on the Mac and on Linux, copy builds the Range from the in-memory list; paste, paste's undo (a range cut) and redo,
split, chapter split, merge and chapter merge each make one range call (`rangePaste` / `rangeCut`) off the main thread, and
the editor applies the returned delta with no reload. Each undoes in one step, with no barriers; the Range carries scene and
chapter sidecars so an undo restores them; a scene merge is a range cut of the break and a chapter merge is
a range relabel (`scrivi_range_relabel`, which moves the scene files), and `scrivi_merge_scene` / `scrivi_merge_chapter` retire. The clipboard and copy buffers store Ranges (an old fragment reads as empty). Old
`structuredPaste` / `structuredCut` history nodes convert to the same path. Absorbs [SP-185] AC4.

**Done (2026-10-21):** ScriviCore (sidecars in the Range, `scrivi_range_relabel`, the scene-start rule, buffers hold
Ranges; the merge endpoints retired); the Mac (`runStructuralEdit`, `ViewportSceneLoader.apply`) and Linux (`runRangeEdit`,
`SceneDocument::applyRangeDelta`) — every structural edit one range call off the main thread, applied as a delta, undone in
one step, nothing reloaded. ctest 748 (Mac) / Linux, interop 301, Linux smokes; mutations core 13, Mac 10, Linux 11, all caught.

**Done (2026-10-22):** the range call over the share — independent file calls run 8 at a time, and a chapter split, its undo, a chapter merge and a run into a new chapter move the smaller side (SP-191 Plan 19–21). Rig, build 79: chapter split 2,430 → 520–1,061 ms, chapter merge 3,268 → 2,109 ms. ctest 759 Mac / 763 Linux, interop 302, smokes 38; core mutations 17 of 18 caught (the survivor changes only the time).
