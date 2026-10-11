# Verified Task — T-0604

**Sprint:** [SP-177] · **Epic:** [EP-026] (S1) · **Platform:** `[Cross]`
**Filed:** 2026-10-11 · **Implemented:** 2026-10-11 · ✅ **USER-VERIFIED 2026-10-11 by the Mac live pass** (*"Live Pass (Mac): 1-10 passes."* — *"Everything behaved as it should. That verifies the Sprint."*)
**Archived:** 2026-10-11, with the Sprint close (*"Close SP-177."*).

---

## T-0604 — `[Cross]` EP-026 S1: the undo capture rules move into ScriviCore; Apple re-pointed

**Epic:** [EP-026] (S1) · **Status:** ✅ **Verified 2026-10-11** (user: *"Live Pass (Mac): 1-10 passes."*) — archived with the Sprint close · **Sprint:** ✅ [SP-177] →
[`../../Sprints/Closed/Sprint-SP-177.md`](../../Sprints/Closed/Sprint-SP-177.md) · **Date Requested:** 2026-10-11 (user: *"plan EP-026 S1."*)

**What:** the rules that decide when an undo step ends — the pending latch, floor seeding, the 45 s idle boundary, the
sentence boundary, the soft whitespace deferral, scene switch vs caret move, paste/cut kinds, autosave-never-seals, grouped
edits — move from `HistoryCapture.swift` and `ManuscriptTextView.swift` into a ScriviCore `CaptureSession` behind
`scrivi_capture_*`; `HistoryService` unchanged. Apple calls them, with no visible change. Proven by an Apple-generated parity
fixture, EP-019's interop tests unchanged, and a Mac live pass.
