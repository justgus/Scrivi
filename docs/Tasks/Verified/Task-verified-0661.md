# Verified Task — T-0661

**Sprint:** none · **Epic:** [EP-057] (for [EP-053]) · **Platform:** `[Linux]`
✅ **USER-VERIFIED 2026-11-04** (user, 2026-11-04: *"all the checkes passed"* — [T-0661] re-run on scrivi-vm, build 1.0.348)
**Archived:** 2026-11-04.

---

## T-0661 — `[Linux]` Rig pass: EP-053 over the share — every structural edit timed, verified, no freeze

**Epic:** [EP-057] ([SP-189] AC5, [SP-190] AC3) · **Status:** ✅ **Verified 2026-11-04** · **Sprint:** none ·
**Date Requested:** 2026-11-04 (user: *"Put all the live pass steps in the EP-057 Live pass"*)

**What:** on scrivi-vm over the 5 ms share, dumas (1,175 scenes): split, merge, chapter split / merge, paste, cut, Backspace over
scenes, each undone and redone; quit and reopen.

**Run 1 (build 1.0.342): FAILED** — the view jumped on a split ([I-0347]); the reopen landed chapters away ([I-0348]); the log
showed a 345–632 ms stall after every edit ([I-0346]).
**Run 2 (build 1.0.348 `40950aa`): ✅ PASS** (user: *"all the checkes passed"*). From the log (`scrivi.log.1` on the VM): **24
structural edits, undos and redos, NO stall after any of them**; 23 verifications `0 corrected, 0 skipped, 0 not correctable`,
1 superseded (an undo pressed before the previous one's verification came back — discarded, as designed). Times, `… ms in all`:
scene split / merge and their undo / redo 328–370 · chapter split / merge 673–810 · paste 364–378 · Backspace 340–354 · the cut
2,192 and its paste back 1,690 (across chapters). Load: 6.7 s. ⚠️ Not in this Task's scope, recorded: a **5.5 s stall right
after the load** (the first 146 ms in the accessibility page build, the rest unmarked) — in run 1's log too (5.5 s).
