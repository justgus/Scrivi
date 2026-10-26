# Verified Task — T-0624

**Sprint:** [SP-194] · **Epic:** [EP-052] (S1) · **Platform:** `[ScriviCore]`
**Implemented:** 2026-10-26 · ✅ **USER-VERIFIED 2026-10-26** (user, 2026-10-26: *"passes for Linux.  passes for macos."*) — [SP-194] AC5, the Mac and rig smoke pass.
**Archived:** 2026-10-26.

---

## ✅ T-0624 — `[ScriviCore]` EP-052 S1: the per-project write queue and its endpoints

**Epic:** [EP-052] (AC1–AC3) · **Status:** ✅ **Verified 2026-10-26** (user, 2026-10-26: *"passes for Linux.  passes for macos."*) · **Sprint:** [SP-194] (✅ Closed) →
[`Closed/Sprint-SP-194.md`](../../Sprints/Closed/Sprint-SP-194.md) · **Date Requested:** 2026-10-26 (user: *"Q1–Q7 agreed as recommended"*)

**What:** v0.1 W1–W4 and W7 under D: one writer thread per open project, FIFO, coalescing; enqueue endpoints for scene saves
and structural operations with a ticket; history appends in the same queue; `scrivi_wait_for_writes` reports every
outcome; synchronous file endpoints wait for earlier jobs (W3); `scrivi_close_project` drains. Boundary tests through
`scrivi_*` and a ThreadSanitizer stress run on macOS and Linux. A structural job's delta comes back inside its outcome in `scrivi_wait_for_writes` ([SP-194] Q1 (a), ruled 2026-10-26).

**Done (2026-10-26):** `WriteQueue` (`ScriviCore/src/public_api/WriteQueue.hpp`, `.cpp`), one per project, owned by its
`ProjectLockEntry`. New endpoints `scrivi_enqueue_save_scenes`, `scrivi_enqueue_range_op` (paste, cut, relabel; the delta in
the outcome) and `scrivi_wait_for_writes` (`scrivi.h`). A queued save calls `scrivi_save_scene`, then tells history itself.
Every file call waits for earlier jobs first (W3, in `ProjectCallGuard`). `HistoryStore::setWriter` hands the log's appends
and checkpoints to the queue, so 20 history / capture endpoints became memory-only; `scrivi_history_close` waits for its
final records; a forked child gets fresh queues. Boundary tests `WriteQueueCApiTests.cpp` (9); the I-0268 and I-0299
tests wait for writes where they need them. ThreadSanitizer clean on macOS and Linux; mutations 5 of 5 caught (no W3, no
coalescing, coalescing across a structural job, history not told, close not waiting); interop 318; Linux smokes 38.
