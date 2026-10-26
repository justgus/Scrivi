# Verified Task — T-0623

**Sprint:** [SP-194] · **Epic:** [EP-052] (S1) · **Platform:** `[ScriviCore]`
**Implemented:** 2026-10-26 · ✅ **USER-VERIFIED 2026-10-26** (user, 2026-10-26: *"passes for Linux.  passes for macos."*) — [SP-194] AC5, the Mac and rig smoke pass.
**Archived:** 2026-10-26.

---

## ✅ T-0623 — `[ScriviCore]` EP-052 S1: two locks per project (memory and disk)

**Epic:** [EP-052] (AC0) · **Status:** ✅ **Verified 2026-10-26** (user, 2026-10-26: *"passes for Linux.  passes for macos."*) · **Sprint:** [SP-194] (🟡 Active) →
[`../Sprints/Sprint-SP-194.md`](../../Sprints/Sprint-SP-194.md) · **Date Requested:** 2026-10-26 (user: *"Q1–Q7 agreed as recommended"*)

**What:** replace SP-173's single exclusive project lock (`ScriviCore/src/public_api/ProjectLock.hpp`) with a memory lock (M)
and a disk lock (D), per v0.2 §3: D before M, never M before D; the revision moves under M. Each of the 126 guarded
endpoints names its kind, and `scripts/check-abi-project-guards.sh` enforces it. A memory-only endpoint that makes a file
operation fails a test. SP-173's concurrency tests move to the new form.

**Done (2026-10-26):** `ProjectLockEntry` holds a disk lock and a memory lock (`ScriviCore/src/public_api/ProjectLock.hpp`,
`.cpp`). `SCRIVI_PROJECT_READ` / `WRITE` hold the disk lock for the call and the memory lock only to read or bump the
revision; `SCRIVI_PROJECT_MEMORY_READ` / `MEMORY_WRITE` hold the memory lock only (26 endpoints, chosen by measuring each
endpoint's file operations across the suite, then reading it); `SCRIVI_PROJECT_UNLOCKED` for the two that wait on the queue.
A memory call that makes a file operation is reported, and aborts the test run (`ScriviCore/tests/StrictMemoryCallsListener.cpp`).
`check-abi-project-guards.sh` accepts the new forms. ctest 778 (Mac) / 782 (Linux); mutation (a file-touching endpoint
marked memory) caught.
