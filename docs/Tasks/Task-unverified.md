# Unverified Tasks

Tasks that are **implemented and awaiting user verification** before being archived to `Verified/`.

**Claude may mark a Task `Implemented - Not Verified`. Only the user can mark it Verified.**

⚠️ This file must not disagree with [`Task-active.md`](Task-active.md) or
[`Task-backlog.md`](Task-backlog.md). A Task implemented but unverified belongs **here** — not left
in the backlog carrying a 🟠 status.

---

## 🟠 T-0623 — `[ScriviCore]` EP-052 S1: two locks per project (memory and disk)

**Epic:** [EP-052] (AC0) · **Status:** 🟠 **Implemented - Not Verified** (2026-10-26) — ⏳ [SP-194] AC5, the Mac and rig smoke pass · **Sprint:** [SP-194] (🟡 Active) →
[`../Sprints/Sprint-SP-194.md`](../Sprints/Sprint-SP-194.md) · **Date Requested:** 2026-10-26 (user: *"Q1–Q7 agreed as recommended"*)

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

---

## 🟠 T-0624 — `[ScriviCore]` EP-052 S1: the per-project write queue and its endpoints

**Epic:** [EP-052] (AC1–AC3) · **Status:** 🟠 **Implemented - Not Verified** (2026-10-26) — ⏳ [SP-194] AC5, the Mac and rig smoke pass · **Sprint:** [SP-194] (🟡 Active) →
[`../Sprints/Sprint-SP-194.md`](../Sprints/Sprint-SP-194.md) · **Date Requested:** 2026-10-26 (user: *"Q1–Q7 agreed as recommended"*)

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

---

## 🟠 T-0616 — `[Cross]` EP-053 S3: structural cut, Backspace / Delete over scenes, and their undo

**Epic:** [EP-053] (S3) · **Status:** 🟠 **Implemented - Not Verified** (2026-10-22) — ✅ rig over the share, ✅ Mac on a local disk; ⏳ the Mac over a share → [SP-190] · **Sprint:** [SP-188] (✅ Closed 2026-10-26) →
[`../Sprints/Closed/Sprint-SP-188.md`](../Sprints/Closed/Sprint-SP-188.md) · **Date Requested:** 2026-10-20 (user: *"complete planning for EP-053"*)

**What:** on the Mac and on Linux, a cut, buffer cut, or Backspace / Delete across scenes removes the structure (design R4,
§4.3, Q2, Q4) through one `scrivi_range_cut` off the main thread, the delta applied, no reload; undo is a range paste
(`rangeCut` node), redo the cut. The core balances the cut's join and returns the markers it added, so the undo is exact.
The clipboard (or slot) holds the cut's returned Range with its sidecars; a selection crossing a divider takes that scene
whole; Backspace over one scene divider merges; paste over a cross-scene selection is the cut, then the paste. Old
`structuredCut` nodes and [I-0270]'s grouped deletes still undo. Retires both platforms' `deleteAcrossScenes`.

**Done (2026-10-22):** ScriviCore (`RangeJoin`, the balanced join in `cut()`, applied and checked in `paste()`); the Mac
(`selectionRun`, `cutAcrossScenes(_:keeping:then:)`) and Linux (`ManuscriptEditor::selectionRun`,
`EditorShell::cutAcrossScenes`) — every cross-scene cut, Backspace / Delete and paste-over one range call off the main
thread, the delta applied, undone in one step; `deleteAcrossScenes` retired on both. ctest 763 (Mac) / 767 (Linux),
interop 311, Linux smokes 38; mutations core 6, Mac 8, Linux 7, all caught.

---

*Last Updated: 2026-10-26 (**T-0616**: [SP-188] closed; its last check, the Mac pass over a share, → [SP-190].)*

*Last Updated: 2026-10-25 (**T-0622 → ✅ Verified** (user) and archived → `Verified/Task-verified-0622.md`, with [SP-193]'s close.)*

*Last Updated: 2026-10-25 (**T-0622**: ✅ Mac half passed (user: About shows the version and hash); ⏳ the rig pass.)*

*Last Updated: 2026-10-24 (**T-0622 → 🟠 Implemented - Not Verified** ([SP-193]); ⏳ Mac and rig live passes.)*

*Last Updated: 2026-10-24 (**T-0620 → ✅ Verified** (user) and archived → `Verified/Task-verified-0620.md`, with [SP-192]'s close.)*

*Last Updated: 2026-10-24 (**T-0620 → 🟠 Implemented - Not Verified** ([SP-192]); ⏳ Mac live pass.)*

*Last Updated: 2026-10-22 (**T-0616 → 🟠 Implemented - Not Verified** ([SP-188]); ⏳ Mac and rig live passes.)*

*Last Updated: 2026-10-22 (**T-0612 → ✅ Verified** (user) and archived → `Verified/Task-verified-0612.md`, with [SP-185]'s close.)*

*Last Updated: 2026-10-22 (**T-0619 → ✅ Verified** (user) and archived → `Verified/Task-verified-0619.md`, with [SP-191]'s close.)*

*Last Updated: 2026-10-21 (**T-0619 → 🟠 Implemented - Not Verified** ([SP-191]); ⏳ Mac and rig live passes.)*

*Last Updated: 2026-10-21 (**T-0605, T-0607, T-0608, T-0609, T-0613, T-0615 → ✅ Verified** (user: Mac + rig live passes, build 75) and archived → `Verified/Task-verified-XXXX.md`.)*

*Last Updated: 2026-10-20 (**T-0615 → 🟠 Implemented - Not Verified** ([SP-187]); ⏳ live passes.)*

*Last Updated: 2026-10-20 (**T-0614 → ✅ Verified** (user) and archived → `Verified/Task-verified-0614.md`, with SP-186's close.)*

*Last Updated: 2026-10-17 (**T-0613 → 🟠 Implemented - Not Verified** ([SP-185]); ⏳ rig live pass.)*

*Last Updated: 2026-10-17 (**T-0612 → 🟠 Implemented - Not Verified** ([SP-185]); ⏳ rig live pass.)*

*Last Updated: 2026-10-15 (**T-0609 → 🟠 Implemented - Not Verified** ([SP-182]); ⏳ rig live pass.)*

