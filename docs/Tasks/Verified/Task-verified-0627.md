# Verified Task — T-0627

**Sprint:** [SP-196] · **Epic:** [EP-052] · **Platform:** `[Cross]`
✅ **USER-VERIFIED 2026-11-02** (Mac live pass (user, 2026-10-27) and [T-0649] passed on the Linux VM (the live-pass checklist: Pass))
**Archived:** 2026-11-02, on the user's authority (user, 2026-11-02: *"on my authority, closed all the objects that have now tested completed"*).

---

## T-0627 — `[Cross]` EP-052 S2b: structural edits through the queue, and the gesture cues

**Epic:** [EP-052] (AC4, AC5, AC10) · **Status:** ✅ **Verified 2026-11-02** (Mac live pass (user, 2026-10-27) and [T-0649] passed on the Linux VM (the live-pass checklist: Pass)) · **Sprint:** [SP-196] (🟡 Active) →
[`../../Sprints/Closed/Sprint-SP-196.md`](../../Sprints/Closed/Sprint-SP-196.md) · **Date Requested:** 2026-10-26 (user: *"approve the close.  plan S2."*)

**What:** on both platforms a structural edit is `scrivi_enqueue_range_op` behind the queued saves; the delta comes back in
its outcome and is applied on the main thread; its history node is recorded from it. Each platform's structural worker is
removed (W7h). The v0.2 Q5 gesture cues.

**Done (2026-10-27):** each structural edit — paste, split, chapter split, merge, chapter merge, cut / Backspace / Delete over
scenes, and every undo and redo of them — is queued (`enqueueRangePaste` / `Cut` / `Relabel`, `ScriviEngine.swift:1334`,
`ScriviBridge.cpp:397`) behind the unsaved scenes' saves (W7b), OFF the main thread; its outcome comes through the ONE write
waiter (`ViewportSceneLoader.swift` `apply(_ wait:)`, `EditorShell.cpp:1689`), and its history node and story times are made
off the main thread before the delta is applied. The synchronous structural call and its worker are gone (W7h)
(`ViewportSceneLoader.swift:551`, `ManuscriptTextView.swift:1372`, `EditorShell.cpp:2037`). Gesture cues (v0.2 Q5): the
cue at once, a repeat ignored, and ⌘Z / Ctrl+Z pressed while an edit is in flight WAITS ITS TURN
(`ManuscriptTextView.swift:390`, `EditorShell.cpp:2502`). A structural edit behind a held queue is refused at once; one that
is itself held keeps the editor read-only until it lands, and a close asks rather than waits. **Evidence:** interop 321/321
(2 new: `ScriviInteropTests.swift:7799`, `:7828`); Linux smokes 38/38 (`background_wait_smoke` §2b new, caught by a mutation
in a copy; `structural_baseline_smoke` Q5 updated). Details: [SP-196]'s log.
