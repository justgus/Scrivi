# Verified Task — T-0608

**Sprint:** [SP-181] · **Epic:** [EP-048] · **Platform:** `[Linux]`
**Implemented:** 2026-10-15 · ✅ **USER-VERIFIED 2026-10-21** (user, 2026-10-21: *"I performed the live pass … on both the Rig and the Mac. all 10 steps passed. I continued with the Scrivi Rig Live Pass … This is build 75 … No steps failed!"*) — SP-181 AC6: rig live pass steps 11 and 21 (Ctrl+Alt+F not taken by GNOME).
**Archived:** 2026-10-21, on the user's report of the pass; the Sprint closed the same day (user-approved).

---

## T-0608 — `[Linux]` EP-048 S4b: Replace and Replace All over the presented text

**Epic:** [EP-048] (S4b, L8's Replace half) · **Status:** ✅ **Verified 2026-10-21** (user, 2026-10-21: *"I performed the live pass … on both the Rig and the Mac. all 10 steps passed. I continued with the Scrivi Rig Live Pass … This is build 75 … No steps failed!"*) · **Sprint:** [SP-181] →
[`../../Sprints/Closed/Sprint-SP-181.md`](../../Sprints/Closed/Sprint-SP-181.md) · **Date Requested:** 2026-10-15 (user: *"plan EP-048 fully."*)

**What:** the find bar's Replace row (Ctrl+Alt+F). Replace writes the replacement escaped and balanced, in the match's
first character's style ([SP-164] Q3). Replace All replaces every match in one pass as ONE undo step across scenes
([SP-164] Q2), through [SP-178]'s grouped edit. Apple's shape (`replaceFound`, `applyReplacements`, `replaceAll`). Replace All
runs its work on a worker, never the GUI thread, and writes only the scenes holding a match (SP-181 Q3). Proven by `find_smoke` through the real key dispatch and a rig
live pass.
