---
sprint: SP-196
epic: EP-052
status: Planning
platform: Cross
created: 2026-10-26
---

# SP-196 — `[Cross]` [EP-052] **S2b**: structural edits through the queue, and the gesture cues

**Status:** 🔵 **PLANNING** — created 2026-10-26 (user: *"approve the close.  plan S2."*). Waits on [SP-195].
**Carries:** [T-0627] → [`../Tasks/Task-backlog.md`](../Tasks/Task-backlog.md).
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

- [ ] AC1 — `[Apple]` Every structural edit through the queue; no structural worker of its own
- [ ] AC2 — `[Linux]` The same, in this Sprint
- [ ] AC3 — Gesture cues as v0.2 Q5 (EP-052 AC10)
- [ ] AC4 — ✅ **Mac and rig live passes (user):** the structural steps and their undo over the share, none freezing

## Progress log

### 🔵 2026-10-26 — Sprint CREATED in Planning
- User: *"approve the close.  plan S2."* Waits on [SP-195] and its Q1 (a held queue is what a refused job meets).
