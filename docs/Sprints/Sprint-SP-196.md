---
sprint: SP-196
epic: EP-052
status: Active
platform: Cross
created: 2026-10-26
---

# SP-196 — `[Cross]` [EP-052] **S2b**: structural edits through the queue, and the gesture cues

**Status:** 🟡 **ACTIVE 2026-10-27** (user: *"Let's proceed with number 2.  SP-196"*) — created 2026-10-26 (user: *"approve the close.  plan S2."*). Activated while [SP-195] waits only on its rig pass (AC6).
**Carries:** [T-0627] → [`../Tasks/Task-active.md`](../Tasks/Task-active.md).
**Epic:** [EP-052] (AC4, AC5 for the structural edits; AC10) → [`../Epics/Epic-active.md`](../Epics/Epic-active.md).
**Authority:** [`../Scrivi_Background_Writes_Design_v0_1.md`](../Scrivi_Background_Writes_Design_v0_1.md) W7 (W7-Q1 (a),
ruled 2026-10-26) · [`../Scrivi_Background_Writes_Design_v0_2.md`](../Scrivi_Background_Writes_Design_v0_2.md) Q5 (ruled).

---

## What reading the code found (2026-10-26)

- **The Mac** (`ViewportSceneLoader.runStructural`, `startStructural`): saves every dirty scene off the main thread, waits for
  them, then runs ONE range call in a `Task.detached`, applies the delta, records the history node.
- **Linux** (`EditorShell::runRangeEdit`, `:1895`): one `AsyncCall` worker does the saves, the capture flush, the range call,
  the history node and the timeline's read.
- Both already give the Q5 cue (read-only at once, the progress row after 400 ms) and ignore a repeat while one is in flight.

## Goal

✅ A structural edit is `scrivi_enqueue_range_op` behind the scene saves already queued (W7b: no save-first step); its delta
arrives in its outcome and is applied on the main thread (W7d); its history node is recorded from that outcome. The
platforms' own structural workers are gone (W7h).

## Plan

1. **[T-0627] Mac and Linux, in the same Sprint:** enqueue the range op; wait for its ticket in the background; apply the
   delta; record the node (`scrivi_capture_record_barrier`, now a memory call). A refused range (W7c) or a held queue
   (SP-195 Q1) restores the editor with nothing applied.
2. **Gesture cues (v0.2 Q5, EP-052 AC10):** every queued gesture shows a cue at once; a repeat is ignored while one is
   pending; ⌘Z / Ctrl+Z waits its turn and never cancels a write.
3. **Tests and live passes:** the EP-053 structural steps (cut, paste, Backspace over scenes, split, merge, and their undo
   and redo) on dumas over the share, timed, on the Mac and the rig.

## Acceptance Criteria

- [x] AC1 — `[Apple]` Every structural edit through the queue; no structural worker of its own
- [x] AC2 — `[Linux]` The same, in this Sprint
- [x] AC3 — Gesture cues as v0.2 Q5 (EP-052 AC10)
- [ ] AC4 — ✅ **Mac and rig live passes (user):** the structural steps and their undo over the share, none freezing

## Progress log

### 🟡 2026-10-27 — Sprint ACTIVATED
- User: *"Let's proceed with number 2.  SP-196"* — beside [SP-195], whose rig pass (AC6) is blocked on the share; T-0627 → active.

### 🟠 2026-10-27 — T-0627 Implemented - Not Verified (AC1–AC3)
- **The shape, both platforms:** the unsaved scenes queued (W7b), then OFF the main thread the pending typing committed and
  the job queued (`scrivi_enqueue_range_op`, W7a) — memory calls. Its outcome arrives through the editor's ONE write waiter
  (an outcome taken before its ticket is known is kept until it is claimed: the job is queued off the main thread). Its result
  → off the main thread: the history node (`scrivi_capture_record_barrier`) and the story times → the delta applied on the
  main thread. ⛔ The synchronous range call and the structural worker are gone (W7h).
- **AC3 (v0.2 Q5):** the cue at once (read-only, the row after 400 ms) and a repeat ignored, as before; ✅ **⌘Z / Ctrl+Z (and
  redo) pressed while an edit is in flight WAITS ITS TURN** and runs once it has landed — ⛔ it was ignored (SP-191 Q5). A
  failed edit drops the undos pressed for it.
- **Held queue ([SP-195] Q1 (a)) — decisions taken, for review:**
  - A structural edit pressed while the queue is held is **refused at once** (a beep, nothing queued, nothing applied).
  - A structural job that is ITSELF held (or queued behind a save that becomes held) **keeps the editor read-only until it
    lands** (W7f), with the Not Available banner; a close or quit does not wait for it, it asks. ⚠️ The plan said such an edit
    "restores the editor with nothing applied" — but the job stays queued and will run when the drive returns, so a restored
    editor would diverge from what lands. The core has no way to take a queued job back.
  - Linux's banner drops its *"N scenes waiting"* count when no scene is dirty (a held structural edit alone).
- **Evidence:** interop 321/321 (new: a structural edit refused behind a held queue; a held structural edit lands and is
  applied when the folder is writable again; the cue test now proves the undo waits its turn and the repeat was ignored).
  Linux smokes 38/38: `background_wait_smoke` §2b new (a held split: read-only, a close not waiting, applied once writable)
  — a mutation in a copy (the held flag ignoring a pending structural job) fails it; `structural_baseline_smoke` Q5 updated
  to the undo waiting its turn. ScriviCore unchanged (no ctest run). Docker: the VM's disk was full; obsolete one-off images
  (mutation, ASan, debug, measurement) and the unused build cache were pruned.
- ⏳ AC4: the live passes (user).

### 🔵 2026-10-26 — Sprint CREATED in Planning
- User: *"approve the close.  plan S2."* Waits on [SP-195] and its Q1 (a held queue is what a refused job meets).
