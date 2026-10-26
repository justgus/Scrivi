# Verified Task — T-0625

**Sprint:** [SP-195] · **Epic:** [EP-052] (S2a) · **Platform:** `[ScriviCore]`
**Implemented:** 2026-10-26 · ✅ **USER-VERIFIED 2026-10-26** (user, 2026-10-26: *"T-0625 is verified."*)
**Archived:** 2026-10-26.

---

## ✅ T-0625 — `[ScriviCore]` EP-052 S2a: what a failed queued write does

**Epic:** [EP-052] · **Status:** ✅ **Verified 2026-10-26** (user, 2026-10-26: *"T-0625 is verified."*) · **Sprint:** [SP-195] (🟡 Active) →
[`../../Sprints/Sprint-SP-195.md`](../../Sprints/Sprint-SP-195.md) · **Date Requested:** 2026-10-26 (user: *"approve the close.  plan S2."*)

**What:** v0.2 §7's failure rule as [SP-195] Q1 (a) ruled 2026-10-26: the queue holds at the failed job, every later job waits
behind it, and the core retries the held job itself every 5 s. A synchronous call that would wait behind a held queue fails
at once. `scrivi_wait_for_writes` reports the queue's state (held, the reason, how many are pending). Boundary tests
through `scrivi_*`.

**Done (2026-10-26):** `WriteQueue.cpp` holds on `ioError` / `permissionDenied` only (any other failure is reported and the queue
goes on); READ/WRITE guards fail at once with `error.detail = "writeQueueHeld"`; `scrivi_wait_for_writes` adds `held`,
`heldReason`, `pending`; a close returns with a held queue and its jobs land if the project returns ([SP-195] log).
**Evidence:** ctest Mac 781/781, Linux 784/784 (container, non-root); `[write-queue]` 10× on Linux; ThreadSanitizer
`[write-queue]` 5× on the Mac, no report; 7 mutations, all caught (never hold, hold on any failure, no retry, a wait that
ignores the hold, every retry reported, the held job sent to the back, a guard that never refuses).

**Ruled at verification (2026-10-26):** a close with a held queue keeps its writes retrying, never discards them (user: *"yes keep the writes retrying rather than discarding them."*).
