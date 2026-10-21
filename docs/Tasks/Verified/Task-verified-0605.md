# Verified Task — T-0605

**Sprint:** [SP-178] · **Epic:** [EP-026] · **Platform:** `[Linux]`
**Implemented:** 2026-10-11 · ✅ **USER-VERIFIED 2026-10-21** (user, 2026-10-21: *"I performed the live pass … on both the Rig and the Mac. all 10 steps passed. I continued with the Scrivi Rig Live Pass … This is build 75 … No steps failed!"*) — SP-178 AC6 and AC8: rig live pass steps 6–8 (a Mac history undone and redone on Linux).
**Archived:** 2026-10-21, on the user's report of the pass; the Sprint closed the same day (user-approved).

---

## T-0605 — `[Linux]` EP-026 S2: undo/redo on Linux — Ctrl+Z / Ctrl+Shift+Z, persistence, session warning, barriers, grouped edits

**Epic:** [EP-026] (S2) · **Status:** ✅ **Verified 2026-10-21** (user, 2026-10-21: *"I performed the live pass … on both the Rig and the Mac. all 10 steps passed. I continued with the Scrivi Rig Live Pass … This is build 75 … No steps failed!"*) · **Sprint:** [SP-178] →
[`../../Sprints/Closed/Sprint-SP-178.md`](../../Sprints/Closed/Sprint-SP-178.md) · **Date Requested:** 2026-10-11 (user: *"We are ready to plan EP-026 S2."*)

**What:** Linux reports to the capture rules SP-177 moved into ScriviCore (`scrivi_capture_*`) and applies undo/redo, in
Apple's shape (`HistoryCapture.swift`, `ManuscriptTextView.swift` apply path): bridge the S2 history/capture endpoints; a
plumbing-only `HistoryCapture` (idle timer at the core's deadline, `isApplying`); open once per project open (never on a
reload) and close in `ScriviWindow::closeEvent`; report edits, caret moves, paste/cut, grouped cross-scene edits and saves;
barriers on split/merge/delete ([I-0287]: split and merge record theirs AFTER the op, carrying the scenes' new texts); Edit ▸ Undo (Ctrl+Z) / Redo (Ctrl+Shift+Z) with Apple's barrier and session-boundary
notices. Proven by Linux smokes through the real key dispatch and a rig live pass.
