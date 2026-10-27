---
sprint: SP-195
epic: EP-052
status: Active
platform: Cross
created: 2026-10-26
---

# SP-195 — `[Cross]` [EP-052] **S2a**: every scene save through the queue, on the Mac and on Linux

**Status:** 🟡 **ACTIVE 2026-10-26** (user: *"File it as a backlog Task, but under EP-052.  Then start SP-195."*) — created 2026-10-26 (user: *"approve the close.  plan S2."*). ✅ Q1–Q3 ruled 2026-10-26.
**Carries:** [T-0625], [T-0626] → [`../Tasks/Task-active.md`](../Tasks/Task-active.md).
**Epic:** [EP-052] (AC4–AC7, the saves) → [`../Epics/Epic-active.md`](../Epics/Epic-active.md).
**Builds on:** [SP-194] (S1: the two locks and the queue, closed 2026-10-26).
**Authority:** [`../Scrivi_Background_Writes_Design_v0_2.md`](../Scrivi_Background_Writes_Design_v0_2.md) §6 levels 2–3,
§7 (the hazards) · [`../Scrivi_Background_Writes_Design_v0_1.md`](../Scrivi_Background_Writes_Design_v0_1.md) W4–W6.
**Then:** [SP-196] (S2b) puts the structural edits through the queue.

---

## What reading the code found (2026-10-26)

**Where the Mac writes a scene** (`ViewportSceneLoader.swift`, `ProjectSession.swift`):
- `saveSceneBlocking`, on the main thread: the typing autosave (`ManuscriptTextView.swift:1262`), app switch
  (`AppEnvironment.swift:879` → `saveAllDirty`), the drive coming back (`ProjectSession.refreshProjectAvailability`),
  quit (`saveAllDirtyBlocking`), and `stampWritingSurfaceBlocking` (the resume point, one save on quit).
- `saveScenesOffMain`, its own background writes: Replace All (`ManuscriptTextView.swift:2435`), undo and redo writes, and
  the saves ahead of a structural edit (`ViewportSceneLoader.swift:583`). It keeps the ordering rule W6 retires:
  `inFlight`, `saveAfterFlight`, the `offMainWrites` group and the quit path's 30 s wait.
- Every successful write calls `noteScenePersisted` itself; a queued save does it in the core (S1, W2).

**Where Linux writes a scene** (`EditorShell.cpp`):
- `saveScene`, on the GUI thread: the autosave timer and `saveDirtyScenes`, and every undo and redo write
  (`applyHistoryStep`, `:3062`).
- `saveScenesOffMain` (`:2896`): Replace All, with its own re-dirty logic. The structural worker saves first (`:1942`).
- ⚠️ **Linux has no drive-lost handling at all ([I-0322])**: no banner, and no "can't be saved" question at close.

**What S1 left for S2 (v0.2 §7):** ⚠️ the queue does nothing special when a write FAILS. It reports the failure and moves on:
a later job runs on top of it, and the scene's next save is just another job. That is Q1.

## Goal

✅ No scene save runs on the GUI thread on either platform, and nothing the writer types or clicks waits for one. Saves
land in the order made; a failed save keeps its scene unsaved and is retried; close and quit wait for the queue, or ask when
it cannot land. Both platforms' own ordering code is gone (W6).

## Plan

1. **[T-0625] Core: what a failed write does** (Q1), and the queue's state for the platform (held, why, how many
   pending), for the banner and the quit question.
2. **[T-0626] Mac and Linux, in the same Sprint** (`feedback_linux_adopts_apple_shape`):
   - every save above becomes `scrivi_enqueue_save_scenes`; a background waiter (`Task.detached` / `AsyncCall`) calls
     `scrivi_wait_for_writes` and hands each outcome to the main thread: written → clean (unless it changed since);
     failed → stays dirty ([I-0259], [I-0304], now ONE rule);
   - `noteScenePersisted` calls on the save paths are removed (the core does it);
   - W6 deleted: `inFlight` / `saveAfterFlight` / `offMainWrites` / the 30 s wait (Mac); `saveScenesOffMain`'s re-dirty
     logic (Linux). The saves ahead of a structural edit become queued saves (the edit itself stays as today until S2b);
   - Replace All and its undo no longer lock the manuscript for their writes (v0.1 Q3 (a)); *"Saving N scenes…"* shows
     without blocking;
   - close and quit: [I-0305]'s wait becomes `scrivi_wait_for_writes`; a held queue asks as [T-0570] does on the Mac
     (Q3 for Linux);
   - **Linux Replace All's progress** (EP-052 AC7, [SP-185] Q3): a real count, *"Replacing 312 of 2,048"*, the view
     following a few times a second.
3. **Tests:** boundary tests for the failure rule; interop (Mac) and smokes (Linux) for: autosave writes off the main
   thread, a failed queued save leaves its scene dirty, undo of a Replace All makes no main-thread write, quit waits. The
   main-thread inventories re-run on both: the save rows go to 0.
4. **Live passes:** dumas-prose-timelines on the rig over the share, and the Mac (EP-052 AC6): undo of a Replace All does
   not freeze the window; typing never waits on a save; a pulled drive's edits are kept and land when it returns.

## Questions

| # | Question | Recommendation |
| - | -------- | -------------- |
| **Q1** ✅ (a) | **A queued write fails** (the drive is pulled, the share drops). What happens to it and to the jobs behind it? | **(a) The queue HOLDS:** the failed job stays at the head and every later job waits behind it, so order is kept and a structural edit never runs over a lost save. The core retries the held job itself (every 5 s), so neither platform needs a trigger; `scrivi_wait_for_writes` reports `held` with the reason. A synchronous file call that would wait behind a held queue returns an error at once instead of hanging the window. (b) Skip it and run the rest: the scene stays dirty and is saved again later, but a structural job can run on top of the lost save. |
| **Q2** ✅ yes | **Split S2 in two?** | **Yes:** S2a (this Sprint) every scene save; S2b ([SP-196]) the structural edits through the queue, with the gesture cues. The structural path was rebuilt only last week ([SP-191], [SP-188]); keeping it apart keeps each live pass readable. |
| **Q3** ✅ fold in | **Linux at close and quit with writes that cannot land.** Linux has no [T-0570] question ([I-0322]). | **Fold [I-0322] into this Sprint:** the held queue (Q1) is the state it needs, so Linux gets the Mac's banner and its "can't be saved" question here, not as separate work. |

## Acceptance Criteria

- [x] AC1 — `[ScriviCore]` A failed write as Q1 rules, with boundary tests (hold, retry, the jobs behind it, a synchronous call
  behind a held queue)
- [x] AC2 — `[Apple]` Every scene save goes through the queue; none on the main thread (the inventory shows 0); a failed save
  leaves its scene dirty; W6's code removed; Replace All and its undo do not lock the manuscript
- [x] AC3 — `[Linux]` The same (EP-052 AC5), in this Sprint
- [x] AC4 — `[Linux]` Replace All shows its progress (EP-052 AC7)
- [x] AC5 — Close and quit wait for the queue on both; a held queue asks (Mac: [T-0570]; Linux: as Q3 rules)
- [ ] AC6 — ✅ **Rig live pass (user)** over the share: undo of a Replace All on dumas does not freeze; typing never waits; a
  pulled drive keeps its edits and they land when it returns (EP-052 AC6)
- [x] AC7 — ✅ **Mac live pass (user)**, the same steps

## Progress log

### 🔵 2026-10-26 — Sprint CREATED in Planning
- User: *"approve the close.  plan S2."* SP-195 and SP-196, T-0625–T-0627 issued via `next-id.py`.
- Q1–Q3 await rulings.

### 🔵 2026-10-26 — Q1–Q3 RULED
- User: *"Q1 (a), Q2 yes, Q3 fold it in."* The queue holds; S2 split; [I-0322] folded in.
- User raised a follow-on: save a held queue to a local store and push it when the project returns. Not in this
  Sprint's scope until ruled (it reopens v0.1 Q4, "no journal").

### 🟡 2026-10-26 — Sprint ACTIVATED
- User: *"File it as a backlog Task, but under EP-052.  Then start SP-195."* The follow-on is [T-0628] (EP-052, backlog,
  not in this Sprint). T-0625, T-0626 → active.

### 🟡 2026-10-26 — T-0625 (core) implemented, Mac
- `WriteQueue` (`ScriviCore/src/public_api/WriteQueue.cpp`): a save or structural job that fails with `ioError` or
  `permissionDenied` goes back to the head and the queue holds; the writer retries it every 5 s; its first failure and its
  eventual success are outcomes. Any other failure is reported and the queue goes on.
- While held, `waitFor` returns at once (no `-1` wait hangs), and a READ/WRITE endpoint's W3 wait fails the call at once
  with `error.detail = "writeQueueHeld"` (the guard macros, `scrivi_c_api.cpp`). `scrivi_wait_for_writes` adds `held`,
  `heldReason`, `pending`.
- **Decided in the work, for review:** a close does NOT discard a held queue. `scrivi_close_project` (now UNLOCKED, then the
  wait, then D taken explicitly) and `scrivi_history_close` return; the held jobs keep retrying and land if the project
  returns while the app runs. Quitting loses them, after the platform has asked ([T-0570]); keeping them is [T-0628].
- Tests (`WriteQueueCApiTests.cpp`): hold + retry + land; a structural job never runs over a held save; a refused write
  lets the queue go on (W2); close with a held queue. ctest Mac 781/781; ThreadSanitizer `[write-queue]` 5×, no report.
  ⏳ Linux ctest (container, non-root) not yet run.

### 🟡 2026-10-26 — T-0625 → 🟠 Implemented - Not Verified (AC1 ✅)
- Linux ctest 784/784 (container, non-root); `[write-queue]` 10× on Linux; 7 mutations in a copy of the repo, all caught.

### 🟡 2026-10-26 — T-0625 ✅ VERIFIED; close rule ruled; T-0626 begun
- User: *"yes keep the writes retrying rather than discarding them.  T-0625 is verified.  You may begin T-0626"*.

### 🟡 2026-10-26 — T-0626: the Mac half implemented
- `ViewportSceneLoader.swift`: every save is `scrivi_enqueue_save_scenes` (`enqueueSaves`): the typing autosave, app switch,
  undo and redo, Replace All, the drive-returned flush, quit and the resume stamp. One background waiter
  (`startWaiting` → `scrivi_wait_for_writes`) hands each outcome to the main actor: written → clean unless the text
  changed since; failed → dirty; `held` → `writesHeld`. W6 deleted (`inFlight`, `saveAfterFlight`, `offMainWrites`, the
  30 s wait); the save paths no longer call `noteScenePersisted` (the core does). A structural edit queues the unsaved
  scenes and runs at once: its core call waits for them (W3), or is refused behind a held queue.
- Replace All and undo / redo no longer lock the manuscript; *"Saving N scenes…"* shows from two scenes (one is the typing
  autosave, every second). A close waits for queued saves but not for a held queue: it asks ([T-0570]), which now also
  counts a held queue (Q3). Quit waits for the queue (`ProjectSession.saveAllDirtyBlocking`).
- Core: a held failure's outcome says `held: true` (a refused one does not), so the app knows which will be retried.
- **Evidence:** interop 319/319 (new: a held save keeps its scene dirty, a close does not wait on it, it lands when the
  folder is back; the Replace All test now expects an editable manuscript and an undo written off the main thread).
  Inventory, main-thread file operations: autosave 5 → 0, ⌘Z 5 → 0, ⇧⌘Z 5 → 0; app switch now only queues saves (its 127
  are Spotlight, S3).

### 🟡 2026-10-26 — T-0626: the Linux half implemented → 🟠 Implemented - Not Verified (AC2–AC5 ✅)
- `EditorShell.cpp`: `saveScene`, the resume stamp, `saveScenesOffMain` and a structural edit's unsaved scenes are
  `scrivi_enqueue_save_scenes` (`enqueueSaves`); ONE waiter (`startWaiting`, `AsyncCall` → `scrivi_wait_for_writes`)
  applies outcomes on the GUI thread (`applyWriteOutcomes`): written → out of `dirtyScenes_` unless changed since; failed →
  stays. The structural worker no longer saves first (W3 orders the range call behind the queued saves). The save paths'
  `notePersisted` is gone (the core tells history). Bridge: `enqueueSaveScenes`, `waitForWrites`.
- **[I-0322] folded in (Q3):** while the queue is held, the red line says *"Project File Not Available…"* with the count
  waiting; a close and Quit ask (*"Some changes can't be saved"*, Cancel the default) — `EditorShell::confirmDiscardingHeld`,
  from `ScriviWindow::closeEvent` and `AppEnvironment::quitApplication`. The queue counts as background work (the
  [I-0305] wait) except while held.
- **Replace All (EP-052 AC7):** applied in ~50 ms slices, *"Replacing 312 of 2,048"* in the progress row, the view following
  each slice, the caret back where it was at the end; each slice's saves queued; the editor unlocks without waiting for them.
- Found on the way: Quit flushed on every pass of `finishQuit`, and a flush always queues the resume stamp, so a quit waited
  for its own stamp forever (`background_wait_smoke` §4). It flushes once now; each window's close flushes again.
- **Evidence:** smokes 38/38 (Docker, Qt 6.4, offscreen, non-root); `background_wait_smoke` rewritten: a failed save holds
  the queue, stays dirty, the editor unlocked, a close asks and Cancel keeps the window, the save lands when the folder is
  writable (§1, §1b); Replace All's held writes land the same way (§2). Inventory, GUI-thread file operations: autosave
  6 → 0, Ctrl+Z 8 → 0, Ctrl+Shift+Z 7 → 0 (what is left is the Inspector's reads, S3). ctest Mac 781/781, Linux 784/784.
- ✅ **AC7 Mac live pass: user-reported pass (2026-10-26).**
- ⏳ **AC6 rig, blocked (2026-10-26):** the drive behind the share was pulled; to bring it back the share had to be
  remounted, and `umount` refused, *"target is busy"*, with the project open in Scrivi. The held save cannot land until
  the share is back, and the share cannot come back while Scrivi is open. Neither Scrivi's code nor the core keeps a
  file open between writes (grep: no watcher, lock file, `chdir` or long-lived stream), so what holds the mount is not yet
  known: `fuser -vm` / `lsof` on the rig names it.
