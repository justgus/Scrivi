---
sprint: SP-190
epic: EP-053
status: Planning
platform: Cross
created: 2026-10-20
---

# SP-190 — `[Cross]` [EP-053] **S5**: Mac and rig live passes over the share, timed

**Status:** 🔵 **PLANNING** — created 2026-10-20 (user: *"complete planning for EP-053"*). ⏳ Q1 below.
**Carries:** [T-0618] → [`../Tasks/Task-backlog.md`](../Tasks/Task-backlog.md).
**Epic:** [EP-053] → [`../Epics/Epic-active.md`](../Epics/Epic-active.md).
**Builds on:** [SP-187], [SP-188], [SP-189].
**Authority:** [`../Scrivi_Structured_CutCopyPaste_Design_v0_2.md`](../Scrivi_Structured_CutCopyPaste_Design_v0_2.md) §6
step 5 · EP-053's Goal (the same behaviour and performance on the Mac and on Linux, over a network share as on a local disk).
**Size:** S — mostly the user's passes; the code work is a Mac-side timing log if Q1 rules one.

---

## What reading the code found (2026-10-20)

- Linux has the diagnostic log ([T-0612]): every structural step's start, finish and duration, and any main-thread stall
  over 250 ms.
- ⚠️ **The Mac has no equivalent.** Its timings today are `print` lines and test hooks (`onSceneWritten`). An Apple
  live pass cannot be timed the same way.
- Test data: dumas-prose-timelines (1,186 scenes) over the share, as in [SP-185].

## Goal

✅ On dumas-prose-timelines over the share, on the Mac and on the rig: copy, cut, paste, Backspace over scenes, split,
merge, and the undo and redo of each, across scenes and chapters. Each step is timed by the platform's log, never freezes
the window, and leaves the manuscript reading as a flat edit would.

## Plan

1. **Q1 first:** the Mac's timing log.
2. **Script the pass** in the chat reply when the Sprint activates, naming every dialog each step shows, with no step's undo
   leaving a fork for the next. Back up the test project first.
3. **Run it on both,** read both logs, and record each step's time here.
4. **Anything found** is filed as an Issue and ruled before this Sprint closes.

## Questions

| # | Question | Recommendation |
| - | -------- | -------------- |
| **Q1** | **The Mac's timing source.** | ⏳ **A minimal Mac diagnostic log** (structural steps and main-thread stalls only, the same line format as Linux's), built here. Alternative: time it by hand, which cannot measure a stall. |

## Acceptance Criteria

- [ ] AC1 — The Mac's timing source as Q1 rules
- [ ] AC2 — ✅ **Mac live pass (user)** over the share: every step timed, no freeze
- [ ] AC3 — ✅ **Rig live pass (user)** over the share: every step timed, no freeze
- [ ] AC4 — Every finding filed and ruled

## Progress log

### 🔵 2026-10-20 — Sprint CREATED in Planning
- User: *"complete planning for EP-053."* Issued with [SP-187]–[SP-189] via `next-id.py`; [T-0618] filed.
- Q1 awaits a ruling.
