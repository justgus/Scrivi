# Scrivi — Background Writes Design v0.1

✅ **RULED 2026-10-15** (§7): Q1 (a), Q2 (c) + autosave, Q3 (a), Q4 (a), Q5 (a) → [EP-052]. Written 2026-10-15, from [SP-181]'s ruling 1.
⚠️ **W7 REVISED 2026-10-20, DRAFT for ruling** (W7-Q1, W7-Q2): structural jobs return [EP-053]'s delta; no reload.
**Request (user, 2026-10-15):** *"Perhaps we should consider doing all undos in a background thread (at least the ScriviCore
part). Perhaps we can create the beginnings of a persistent thread pool (if the OS doesn't already do that) or we can
instantiate a dedicated thread for performing undo/history update operations."* Then: *"write the design note ruling."*
**Extends:** `Scrivi_Core_Concurrency_Design_v0_1.md` (SP-173: one lock per project around every endpoint, D1). ⚠️ That
document's §4 **rejected** serialising on the platform side; this design keeps that rejection (§4 below).
**Scope:** writes to ONE project from ONE app process: scene saves, the history log, and the undo and redo that produce
them. ⛔ Not in scope: two processes or machines on one project (external change detection), world writes (D6 of the
concurrency design), and structural operations' own speed ([I-0297]).

---

## 1. What runs where today (read from the code, 2026-10-15)

| Work | Apple | Linux |
| ---- | ----- | ----- |
| Typing autosave (debounce, scene change, resign) | main thread, `saveSceneBlocking` | GUI thread, `saveDirtyScenes` |
| Replace All's scene writes | ✅ background ([I-0303]), `saveScenesOffMain` | ✅ background ([SP-181]), `saveScenesOffMain` |
| Undo / redo: the history step | main thread, `HistoryCapture.undo` | GUI thread, `HistoryCapture::undo` |
| Undo / redo: the scene writes | main thread, one per changed scene (`applySceneChange`) | GUI thread, one per changed scene (`applyHistoryStep`) |
| Typing capture (sentence events) | main thread | GUI thread |
| Structural operations (split, merge, cross-scene paste) | main thread | background ([I-0297], `runStructural`) |
| Close and quit | wait for background writes ([I-0305]), then save on the main thread | the same |

What one undo costs, by reading:
- **The history step:** in memory (`HistoryService::undo`), plus ONE append to the history log
  (`store.persistCtl("undo", …)`, `scrivi_c_api.cpp:3616`; redo `:3639`). The log lives inside the project, so over the
  rig's share that append is a network write. ⏳ Its time is not measured.
- **Putting the text back in the editor:** GUI-only on both platforms (AppKit's text storage, Qt's document). It cannot move
  and is fast.
- **The scene writes:** one file write (plus its sidecar) per changed scene. Undoing a Replace All across ~1,170 scenes is
  ~1,170 writes on the GUI thread, which is the [I-0303] freeze again, in reverse.

What already keeps order, and where:
- **The core:** every `scrivi_*` call on a project holds that project's lock (`ProjectCallGuard`, `ProjectLock.cpp:43`).
  ✅ Two calls never overlap. ⛔ But the lock gives no ORDER: `std::mutex` is not first-come-first-served, so a main-thread
  save and a background save of the same scene can land in either order.
- **The platforms, separately:** each has its own rule to stop an older write landing over a newer one.
  - Apple: `inFlight` and `saveAfterFlight` (`ViewportSceneLoader.swift:61–62`): a save of a scene in flight waits for it.
  - Linux: a scene in flight is taken out of the dirty set.
  - ⚠️ **They have already drifted once:** Linux dropped a failed save ([I-0304]) while the Mac kept it ([I-0259]). That is
    the concurrency design's §4 warning coming true: one rule, written twice, drifts.

## 2. The thread-pool question

- **Both operating systems already provide pools.** Apple: Grand Central Dispatch, which Swift concurrency (`Task.detached`)
  runs on. Linux: Qt's global `QThreadPool`, which `QtConcurrent` and `AsyncCall` run on. A pool of our own would add
  nothing.
- **A pool is the wrong tool for writes anyway.** It runs jobs in any order, on any thread. One project's writes must land in
  the order the writer made them, so what is missing is a **queue with an order**, not more threads.
- ✅ **So: your second idea, a dedicated thread.** One writer thread per open project, owned by ScriviCore (§3). Per project
  rather than one for the whole app, so a project on a dead share cannot hold up a project on the local disk.

## 3. Design (recommended): a per-project write queue in ScriviCore

### W1 — The queue

- Each open project gets one **writer thread** with a first-in-first-out queue, in a registry beside the project lock
  (`ProjectLock.cpp`). It starts when the project opens and is drained and stopped by `scrivi_close_project`.
- Each job takes the project lock (D1) as any call does, so a queued write and a synchronous call never overlap.
- **Coalescing:** a scene queued again before its write has started keeps one job, with the newer text. A Replace All
  followed at once by its undo writes each scene once, not twice.

### W2 — What goes through it

- **Scene body saves:** a new endpoint, `scrivi_enqueue_save_scenes(projectRootPath, scenesJSON)`, taking a list of
  `{sceneID, metadataPath, contentPath, markdown, selection…}`. It returns at once, with a **ticket** (a number that
  increases with every job).
- **The history log's appends:** the in-memory history step returns at once; its append (`persistCtl`, and the capture's
  event records) joins the same queue, in order with the scene writes it belongs to. ✅ This is the *"ScriviCore part"* of an
  undo. The existing failed-append buffer (`unwrittenLines_`, `HistoryStore.hpp:120`, [I-0268]) still applies.
- ✅ **History's bookkeeping moves inside.** The core writes the scene, so it knows when the write succeeded and with which
  bytes. It records the persisted text itself, so the platforms no longer call `noteScenePersisted`. That removes the
  [I-0104] / [I-0259] obligation from both platforms.

### W3 — Synchronous calls still see every earlier write

- Every existing endpoint that reads or writes scene files first waits until the queue has finished the jobs queued BEFORE
  it was called (their tickets), and only then takes the project lock. Structural operations, `scrivi_save_scene`, reloads
  and scene reads all behave as today, with no change at their call sites.
- ⚠️ **No deadlock:** the wait happens before the lock is taken, and the writer thread never waits on a caller.

### W4 — Learning that a write landed, or failed

- `scrivi_wait_for_writes(projectRootPath, ticket, timeoutMs)` blocks until that ticket has landed (or the time runs out),
  and returns every job's outcome since the last report: written, or failed with the reason.
- The platforms call it on a background thread (`Task.detached` / `AsyncCall`) and handle the answer on the main thread:
  - a written scene is marked clean (unless it changed meanwhile);
  - ⛔ a failed scene is marked dirty again ([I-0259], [I-0304]): now ONE rule, in the core's report;
  - a failure is shown as it is today.
- ✅ Pull, not push: no C callbacks from a core thread into Swift or Qt, and the boundary stays JSON over strings.

### W5 — What the writer sees

- **⌘Z / Ctrl+Z stays instant.** The editor takes the undone text at once; the writes queue behind it.
- **Order is guaranteed, so the editor need not lock.** Replace All and its undo would no longer make the manuscript
  read-only while they save. The existing *"Saving N scenes…"* indicator stays, without blocking. (⚠️ Linux's Replace All
  still locks briefly while it computes the new texts, [SP-181] Q3. That is computing, not saving.)
- **Close and quit:** [I-0305]'s dialog waits until the queue is empty. Each platform keeps its dialog; the waiting itself
  becomes one call, `scrivi_wait_for_writes`.

### W6 — What each platform loses

- Apple: `saveScenesOffMain`, `inFlight` / `saveAfterFlight`, the `DispatchGroup`, and the quit path's 30 s wait.
- Linux: `EditorShell::saveScenesOffMain` and its re-dirty logic. [I-0305]'s background-work count shrinks to the work that
  is not a write (Replace All's compute, structural steps).
- Typing autosave can switch to the queue too, so no ordinary save waits on the network on the GUI thread. (⚠️ Q2.)

## 4. Rejected

- **A queue on each platform** (a serial dispatch queue on Apple, a dedicated `QThread` on Linux). This is what I first
  recommended in reply to the ruling. ⛔ It is the design `Scrivi_Core_Concurrency_Design_v0_1.md` §4 rejected: the same
  rule written twice, in two languages, drifts (and [I-0304] shows it already has). It also does nothing for a third
  platform. Ordering is the core's job; the core is the only shared code.
- **A thread pool of our own:** see §2.
- **Making the existing write endpoints quietly asynchronous.** Every caller relies on `scrivi_save_scene` having written
  when it returns; changing that under them would turn every failure path into a silent one. A new endpoint is explicit.
- **Moving the editor apply off the main thread.** Neither toolkit allows it.

## 5. Risks

- **A crash loses what is queued.** Today a crash loses the write in progress; with a queue it can lose several. Jobs start
  at once, so the window is the time the writes take. A clean close or quit drains the queue first ([I-0305]). → **Q4.**
- **A dead share holds the queue.** A write that blocks (the [I-0193] case) blocks every later write to that project, though
  not to other projects. The writer sees *"Saving…"* that does not finish, which is honest. The 10-minute backstop
  (`AsyncCall`) becomes the timeout of `scrivi_wait_for_writes`.
- **Reads wait for writes (W3).** A reload right after a large Replace All waits for its writes. That is correct (it must read
  them), but it moves the wait from the save to the reload.

## 6. Tests

- **Boundary, through `scrivi_*`:** FIFO order per project; coalescing; a synchronous read after an enqueue sees the queued
  text; a failure is reported and the history is not told; `scrivi_close_project` drains.
- **Stress under ThreadSanitizer** (macOS and Linux): one thread enqueues saves and undoes while another reads and runs
  structural operations; every read sees a state the writer produced.
- **Each platform:** undoing a Replace All writes no scene on the main thread and does not lock the editor; a failed queued
  write leaves its scene dirty; close and quit wait.
- **Rig:** undo of a Replace All on dumas over the share, timed before and after.

## 7. ✅ Rulings — 2026-10-15 (user)

| # | Ruling | What it changes in §3 |
| - | ------ | --------------------- |
| **Q1** | ✅ *"ScriviCore."* (a) | As written: one writer thread per open project, in the core |
| **Q2** | ✅ *"everything and autosave."* (c), with every typing autosave | ⚠️ **Wider than recommended.** Structural operations (split, merge, chapter create, reorder, cross-scene cut and paste, their undo and redo) are queued jobs too, not only synchronous calls ordered by W3. See W7 |
| **Q3** | ✅ *"No no no."* (a): the editor does NOT lock while writes run | Replace All and undo stop locking the manuscript for their writes; *"Saving N scenes…"* shows without blocking. ⚠️ Read as an emphatic *No* to the question asked (*"Does the editor still lock?"*) |
| **Q4** | ✅ *"accept it."* (a) | No journal. Close and quit drain the queue |
| **Q5** | ✅ *"A new Epic."* (a) | [EP-052] |

### W7 — Structural operations through the queue (Q2 (c)) — ⚠️ REVISED 2026-10-20, DRAFT for ruling

⚠️ **Why revised:** the 2026-10-15 text had the platform wait for the ticket and then **reload the manuscript**, read-only
for the whole wait. [EP-053]'s design (`Scrivi_Structured_CutCopyPaste_Design_v0_2.md`, ruled 2026-10-17) removes that
reload: every structural call returns its **delta**, the platform applies it to its in-memory scene list, and a background
sync verification follows (v0.2 P3, P4, R3). This revision gives W7 that shape. Nothing else in §3 changes. ⏳ Awaiting
ruling: W7-Q1 and W7-Q2 below.

- **W7a — A structural operation is a queued job with a ticket**, like a save: `scrivi_range_paste`, `scrivi_range_cut`,
  split, merge, chapter create and reorder, and the undo and redo of each (v0.2 §4.5: undo of a paste is a range cut, undo of
  a cut is a range paste). Each gets an enqueue form. The synchronous endpoints stay, ordered by W3, for tests, tools and
  any platform that has not adopted the queue yet.
- **W7b — The queue's order replaces "save first".** v0.2 §4.2b says the range's scenes are *"saved already, or saved
  first."* With the queue, the platform enqueues the dirty scenes' saves and then the structural job; first in, first out
  means the job runs on what was saved. No synchronous save comes before a structural call on either platform.
- **W7c — The range is checked when the job RUNS, not when it is queued.** v0.2 P1 refuses a range that does not match the
  index. Earlier jobs in the queue can change the index, so the check runs at the head of the queue. A refusal is a failed
  outcome (W4) with its reason; the platform has applied nothing yet, so its memory is unchanged.
- **W7d — The delta comes back with the outcome (W7-Q1).** `scrivi_wait_for_writes` reports a structural job's outcome
  with its full delta (v0.2 P3: each scene created, removed, moved or rewritten; each chapter created or removed). The
  platform applies it to its in-memory list on the main thread and re-renders only the scenes it names. ⛔ No reload, no
  re-read.
- **W7e — Coalescing never crosses a structural job.** A save queued before the job and one queued after it are never
  merged, and a save is never moved past a structural job. (A scene the job deletes cannot receive a later save: the
  editor is read-only until the delta is applied, W7f.)
- **W7f — What the writer sees: unchanged from v0.2 Q5.** The manuscript is read-only from the enqueue until the delta is
  applied, with the progress row after 400 ms. That is waiting for a **result**, not for a write, so it does not contradict
  Q3 (a). v0.2 §4.4's disk budget (a two-scene paste: 4 reads + 4 writes) is what keeps the pause short; the queue adds only
  the wait for any saves ahead of the job.
- **W7g — History and verification follow the job in the queue.** The structural step's history append (W2) is queued
  directly behind its job. v0.2 P4's sync verification reads only the files the delta names, through W3, so it sees the
  job's writes; *"the disk wins, silently"* (v0.2 Q3).
- **W7h — What each platform loses (adds to W6).** Apple: the `Task.detached` that EP-053 S2 adds for the structural call
  becomes a wait for the ticket. Linux: `runStructural`'s worker becomes the same wait. Both: the save-before-structural
  step (W7b).

#### W7 questions for ruling

- **W7-Q1 — How the delta reaches the platform.** (a) ✅ recommended: inside the job's outcome in `scrivi_wait_for_writes`,
  one call, the same pull model as W4. (b) A separate `scrivi_structural_result(ticket)`: outcome and delta fetched apart,
  a second call per operation, and a result the core must keep until it is fetched.
- **W7-Q2 — Order of the two Epics.** EP-053 S2 (no reload, the structural call off the main thread on both platforms)
  and EP-052 S2 (the queue's adoption) both rewrite the same call sites.
  (a) ✅ recommended: **EP-053 S2 first.** It lands the delta and the in-memory apply on the synchronous endpoints, and
  EP-052 S2 then only swaps the background call for a ticket wait (W7h).
  (b) EP-052 first: the queue would carry today's fragment operations and their reload, which EP-053 then deletes. Work
  done twice.

## 8. Questions for ruling (original text)

### Q1 — Where the write order lives

- (a) **In ScriviCore**: one writer thread per open project (§3).
- (b) On each platform: a serial queue on Apple, a dedicated thread on Linux (§4, previously rejected).
- (c) Leave it as today: each heavy operation saves in the background on its own, and add undo's writes the same way.

✅ **Recommendation: (a).** It is the only option where the order rule exists once. (b) is the shape the concurrency design
rejected, and (c) multiplies the platform-side rules that have already drifted ([I-0304]).

### Q2 — What goes through the queue

- (a) Scene body saves from Replace All and from undo and redo, plus the history log's appends.
- (b) (a), plus every typing autosave.
- (c) Everything that writes, structural operations included.

✅ **Recommendation: (b).** An autosave is the most frequent write, and over the share it is a network write on the GUI thread
every 1.5 s of idle typing. Structural operations stay synchronous calls (W3 orders them correctly): their result decides
what the editor shows next, so the platform must wait for them anyway.

### Q3 — Does the editor still lock while writes run

- (a) No. Order is guaranteed, so the writer keeps typing; *"Saving N scenes…"* shows without blocking.
- (b) Yes, as [SP-181] does now for Replace All.

✅ **Recommendation: (a).** The lock exists today only because the platforms could not otherwise keep order. ⚠️ It reverses
part of [SP-181]'s shape on both platforms, so it is a deliberate change, not a side effect.

### Q4 — The crash window

- (a) Accept it: jobs start at once, and close and quit drain the queue.
- (b) Also write a small *"pending"* journal before each job, so a relaunch can redo what was lost.

✅ **Recommendation: (a).** (b) is a second write per write, which over the share is the cost this design removes. Revisit only
if a lost queued write is ever observed.

### Q5 — Tracking

- (a) A new `[Cross]` Epic: S1, ScriviCore (the queue, the two endpoints, W3, the boundary and stress tests); S2, Apple and
  Linux adopt it in the SAME Sprint (`feedback_linux_adopts_apple_shape`), removing W6's code.
- (b) Fold it into [EP-026] (undo/redo on Linux).

✅ **Recommendation: (a)**, sequenced after [SP-181] closes. It is a core change touching every write path, larger than any
one EP-026 slice, and both platforms change together.
