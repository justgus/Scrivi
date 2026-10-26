---
sprint: SP-194
epic: EP-052
status: Active
platform: ScriviCore
created: 2026-10-26
---

# SP-194 — `[ScriviCore]` [EP-052] **S1**: two locks per project and the write queue

**Status:** 🟡 **ACTIVE 2026-10-26** (user: *"(a) agreed, activate SP-194"*) — created 2026-10-26 (user: *"Q1–Q7 agreed as recommended"*; v0.2 Q7 makes this S1). ✅ Q1 ruled (a).
**Carries:** [T-0623], [T-0624] (✅ Verified 2026-10-26) → `../Tasks/Verified/Task-verified-0623.md`, `Task-verified-0624.md`.
**Epic:** [EP-052] (AC0–AC3) → [`../Epics/Epic-active.md`](../Epics/Epic-active.md).
**Authority:** [`../Scrivi_Background_Writes_Design_v0_2.md`](../Scrivi_Background_Writes_Design_v0_2.md) §3 (✅ ruled
2026-10-26) · [`../Scrivi_Background_Writes_Design_v0_1.md`](../Scrivi_Background_Writes_Design_v0_1.md) W1–W7 (✅ ruled
2026-10-15; W7 revised 2026-10-20; W7-Q1 ruled (a) here, 2026-10-26).
**Size:** L — every guarded endpoint changes lock; the queue is new. No platform code changes (S2 adopts the queue).

---

## What reading the code found (2026-10-26)

- `ProjectCallGuard` (`ScriviCore/src/public_api/ProjectLock.hpp`) is one exclusive mutex per project; **126** endpoints
  take it through `SCRIVI_PROJECT_READ` / `SCRIVI_PROJECT_WRITE` (`scrivi_c_api.cpp:335`). Read and write differ only in
  whether the revision moves.
- Every envelope already carries `trace: {endpoint, ms, fileOps}` ([SP-188]'s inventory), so which endpoints touch files
  is measured, not guessed. The inventory found the caret, select and scroll calls make 0 file operations.
- `scripts/check-abi-project-guards.sh` already checks every project endpoint takes the guard.

## Goal

✅ A call that touches no file never waits for one: it takes only the memory lock (M). Writes go through one writer thread
per project, in order, with a ticket and a reported outcome. Every existing synchronous endpoint still sees every earlier
write (W3), so the apps work unchanged until S2 adopts the queue.

## Plan

1. **Classify the 126 endpoints** as memory-only (M) or file-touching (D, then M). The guard macro names the kind, and
   `check-abi-project-guards.sh` refuses an endpoint without one. Source: the `trace.fileOps` counts plus reading each body.
   A memory-only endpoint that does a file operation fails a test (the counter is already per thread).
2. **[T-0623] Two locks (v0.2 §3, AC0):** M and D per project, D before M, never M before D. The revision moves under M.
   SP-173's concurrency tests move with it, plus: a memory call returns while another thread holds D.
3. **[T-0624] The queue (W1–W4, W7; AC1, AC2):** one writer thread per open project, FIFO, coalescing a scene queued again
   before its write starts; enqueue endpoints for scene saves and structural operations, returning a ticket; history
   appends join the queue; `scrivi_wait_for_writes` reports every outcome, a failure included; a synchronous file
   endpoint waits for the jobs queued before it (W3); `scrivi_close_project` drains it.
4. **Tests (AC3):** boundary tests through `scrivi_*`: order, coalescing, read-after-queue, failure, close drains, M never
   waits on D. A ThreadSanitizer stress run on macOS and in the Linux container. ctest on both, interop tests on the Mac.
5. **Nothing regressed:** the Linux smokes and a short Mac and rig pass (open, type, undo, a structural edit, quit), since
   every endpoint's lock changes.

## Questions

| # | Question | Recommendation |
| - | -------- | -------------- |
| **Q1** ✅ **(a), user 2026-10-26** | **v0.1 W7-Q1: how a queued structural job's delta reaches the platform.** Never ruled (W7 was revised 2026-10-20). | **(a)** Inside the job's outcome in `scrivi_wait_for_writes`: one call, the same pull model as a save's outcome. (b) is a separate `scrivi_structural_result(ticket)`: two calls per operation, and the core keeps results until fetched. |

v0.1's W7-Q2 (which Epic first) needs no ruling now: EP-053 S2 landed first ([SP-191], closed 2026-10-22), as its (a)
recommended.

## Acceptance Criteria

- [x] AC1 — Every project endpoint names its lock kind; the guard check refuses one that does not; a memory-only endpoint
  making a file operation fails a test
- [x] AC2 — Two locks (EP-052 AC0): a memory call returns while D is held; D before M everywhere; SP-173's tests pass in
  their new form
- [x] AC3 — The queue and its endpoints (EP-052 AC1, AC2); a structural job's delta inside its outcome (Q1 (a))
- [x] AC4 — Boundary tests through `scrivi_*` and a ThreadSanitizer stress run, macOS and Linux (EP-052 AC3)
- [x] AC5 — ✅ **Mac and rig smoke pass (user):** nothing a writer does has changed

## Progress log

### 🔵 2026-10-26 — Sprint CREATED in Planning
- User: *"Q1–Q7 agreed as recommended."* SP-194, T-0623 and T-0624 issued via `next-id.py`.
- S2 and S3 are not created yet: their Tasks depend on the endpoints S1 lands.

### 🟡 2026-10-26 — Sprint ACTIVATED
- User: *"(a) agreed, activate SP-194."* Q1 ruled (a): the delta comes back inside the job's outcome in
  `scrivi_wait_for_writes`. T-0623 and T-0624 → active.

### 🟠 2026-10-26 — AC1–AC4 met; ⏳ AC5, the smoke pass (user)
- **Classification by measurement, then reading.** An instrumented copy of the repo logged each endpoint's file operations
  across the whole suite: 7 made none. Reading them kept 6 (`capture_before_save` can commit a pending edit, a history
  append). Once history's writes went through the queue (T-0624), a second measurement found 20 more history and capture
  endpoints with none on the caller's thread; read, and moved to the memory lock. 26 memory-only endpoints in all.
  `scrivi_close_project` stays a file call on purpose: its W3 wait is what makes a close mean "everything landed".
- **Two endpoints hold no lock** (`SCRIVI_PROJECT_UNLOCKED`): `scrivi_wait_for_writes`, and `scrivi_history_close`, which
  waits for its final records. Holding D while waiting would stop the writer; holding M, every memory call.
- **Found on the way:** the [I-0299] tests fork, and a forked child has no writer thread: the first file call hung. A forked
  child now gets fresh locks and queues (`pthread_atfork`); the child waits for its writes before `_exit` (a crash loses only
  what is still queued: v0.1 Q4). ThreadSanitizer cannot start threads after such a fork, so those two run under TSan with
  `die_after_fork=0`.
- **Found by mutation:** the first W3 test passed with W3 removed (the writer usually won the disk lock). It now reads the
  SECOND queued save: 20/20 with W3, 0/5 without.
- **Evidence:** ctest Mac 778/778, Linux 782/782 (container, non-root); ThreadSanitizer: Mac and Linux, whole suite, no
  report (Linux binary confirmed linked to `libtsan`); queue tests repeated 30×; mutations 6, all caught (a file endpoint
  marked memory; no W3; no coalescing; coalescing across a structural job; history not told; history close not waiting);
  interop 318 tests in 43 suites; Linux smokes 38/38.
- **What the apps already get (no platform code changed):** the Mac inventory re-run, main-thread file operations —
  ending a sentence 1 → 0, ⌘Z 8 → 5, ⇧⌘Z 7 → 5, autosave 6 → 5; the remaining 5 are `scrivi_save_scene` (S2), the
  Inspector's reads and Spotlight (S3).

### ✅ 2026-10-26 — AC5 met: the Mac and rig smoke pass (user, 2026-10-26: *"passes for Linux.  passes for macos."*)
- T-0623 and T-0624 → ✅ Verified and archived. All five ACs met; ⏳ the close awaits the user's approval.

