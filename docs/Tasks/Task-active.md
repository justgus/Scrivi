# Active Tasks

---

## 🟡 T-0604 — `[Cross]` EP-026 S1: the undo capture rules move into ScriviCore; Apple re-pointed

**Epic:** [EP-026] (S1) · **Status:** 🟡 Active (2026-10-11) · **Sprint:** [SP-177] (🟡 Active) →
[`../Sprints/Sprint-SP-177.md`](../Sprints/Sprint-SP-177.md) · **Date Requested:** 2026-10-11 (user: *"plan EP-026 S1."*)

**What:** the rules that decide when an undo step ends — the pending latch, floor seeding, the 45 s idle boundary, the
sentence boundary, the soft whitespace deferral, scene switch vs caret move, paste/cut kinds, autosave-never-seals, grouped
edits — move from `HistoryCapture.swift` and `ManuscriptTextView.swift` into a ScriviCore `CaptureSession` behind
`scrivi_capture_*`; `HistoryService` unchanged. Apple calls them, with no visible change. Proven by an Apple-generated parity
fixture, EP-019's interop tests unchanged, and a Mac live pass.
