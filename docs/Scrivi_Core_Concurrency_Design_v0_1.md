# Scrivi — Core Concurrency Design v0.1

⚠️ **DRAFT, awaiting ruling.** Written 2026-10-10 for [SP-173] / [I-0285].
**Request (user, 2026-10-10):** *"design a cross platform sync so that timeline loads and other core operations such as manuscript
edits occur atomically."*
**Scope:** calls into ScriviCore from more than one thread of ONE app process, on every platform. ⛔ Not in scope: two processes or
two machines on one project (e.g. the rig opening a project from the Mac's share while the Mac has it open). That is external
change detection's job (`Scrivi_External_Change_Repair_Matrix_v0_2.md`).

---

## 1. What exists today (read from the code, 2026-10-10)

| Fact | Where |
| ---- | ----- |
| The facade is **one process-wide singleton** (`CoreSingleton`; `core()` returns it). ✅ It holds only `services_` (pointers), and the services hold no mutable state: the clock reads the system time, the UUID generator seeds a local generator per call (`SystemUUIDProvider.cpp:32`), the file system has no cache. **So the only state shared between calls is the registries.** (⚠️ Corrected 2026-10-10: this row first said the core was built per call, from the `ProjectIndexRegistry` comment, which is stale) | `scrivi_c_api.cpp:148–175`, `ScriviCore.hpp:82` |
| Per-project state lives in registries keyed by `projectRootPath`, **each with its own `std::mutex`** (history, project index, scene locator). Each mutex guards its own map, nothing more | `scrivi_c_api.cpp:306–347` |
| **No lock spans an endpoint.** Two endpoints on one project can run at the same time, and a multi-file write (merge, reorder, chapter create) can interleave with a read | — |
| A single file write is atomic (`AtomicWrite`: write, then rename), so a reader never sees a torn file, but **can see a mix of old and new files** from one multi-file operation | `ScriviCore/src/util/AtomicWrite` |
| **There is no project change counter.** (`generation` in `ObjectIndex.cpp` is the index file's FORMAT version) | — |
| **Both platforms already call the core from background threads after the load**, while the main thread saves edits: Apple `WorldWarningView` (`listWorlds`, `listPendingEdges`); Linux `SceneInspector` reload (`listEdgesFor`, `listObjects`, `listWorlds`) and `WorldsDialog`. The project load runs `openProject` and every scene read in the background on both | `WorldWarningView.swift:73`, `SceneInspector.cpp:389`, `WorldsDialog.cpp:196`, `ProjectSession.swift:201`, `EditorShell.cpp:449` |
| ⚠️ **Linux's bridge keeps "did the last call fail" in one plain `bool`** (`ScriviBridge::lastCallFailed_`), read AFTER the call. A worker's call and a main-thread call can overwrite each other's flag: a data race, and a wrong answer even without one | `ScriviBridge.hpp:569, 588` |

**So the hazard is not new.** Background reads already overlap main-thread edits on both platforms; SP-173 would add one more (the
timeline). Nothing has failed visibly, which is what an unsynchronised read of mostly-unchanged files looks like until it does not.

## 2. What "atomic" has to mean — three levels

| Level | Meaning | Example of the failure it prevents |
| ----- | ------- | ---------------------------------- |
| **L1 — one call** | Each `scrivi_*` call sees the project before or after any other call, never part-way through it | A timeline read during a scene merge sees the merged scene's file but the old chapter's metadata |
| **L2 — one group of reads** | A load that needs several reads sees ONE state of the project | The timeline's five reads straddle a story-time edit: dots from before it, structure from after |
| **L3 — applying the result** | A background result is put on screen only if nothing changed since it was read | The writer edits a story time while the timeline loads; the load finishes and draws the OLD value over the new |

L1 alone does not give L2, and L1 + L2 do not give L3: the lock is released before the main thread applies the result.

## 3. Design

### D1 — A per-project lock in the core, around every endpoint (→ L1)

- A `ProjectLockRegistry` in `scrivi_c_api.cpp`, the same shape as the history and index registries: one lock per
  `projectRootPath` (canonicalised, so two spellings of one path share a lock).
- **Every endpoint that takes a project root runs inside it**, through one wrapper: `withProjectRead(root, …)` for reads,
  `withProjectWrite(root, …)` for writes. The classification sits at each endpoint's definition, in one word.
- ✅ **Kind-list rule applied:** no separate list of "which endpoints are writes" is kept anywhere. A boundary test enumerates the
  exported `scrivi_*` symbols (100 — `Scrivi_ABI_Binding_Gap_Audit_v0_1.md`) and fails if any project endpoint is not wrapped.
- **Lock order:** the project lock first, then a registry's own mutex. Registry mutexes never call back into an endpoint, so no
  cycle exists.
- ⚠️ **Shared worlds.** A world package is shared by several projects. A write to a world object under project A's lock can race a
  read of the same world under project B's lock (two windows). → **Q2.**
- **Cost:** a lock costs microseconds against the share's ~48 ms per file read. A writer waits at most for the reads in flight;
  the 57 s bulk load holds the lock **per scene call**, not for the whole load, so a save waits at most one scene read.

### D2 — A project revision (→ L3)

- Each open project gets an in-memory **revision**, a counter in the same registry, bumped by every `withProjectWrite` call.
- **Every envelope carries `"revision"`**: the revision a read saw, or the revision a write produced. No extra call is needed to
  learn it.
- In memory only: it describes this process's view of the project, which is what L3 needs. A reopen starts at 0.

### D3 — One endpoint for the timeline's reads (→ L2)

- `scrivi_load_timeline(projectRootPath)` returns, under ONE read lock: the timeline settings (`getTimeline`), every explicit story
  time (`listStoryTimes`), the story structure, the historical events, the imported timelines, and the revision.
- ✅ Replaces five calls with one on both platforms: one crossing, one lock, one consistent state.
- It is a new C ABI endpoint, so it is `[Cross]` by definition, and both platforms adopt it in the same Sprint.
- The other multi-read loads (the Scene Inspector reload, the world warning) get the same treatment only when a measurement or a
  defect asks for it (**Q3**). D1 and D2 already protect them.

### D4 — The platform pattern: fetch in the background, apply if current (→ L3, the same on both)

1. The main thread knows the project's current revision: every write it makes returns one (D2).
2. Background: call the read (`scrivi_load_timeline`), which returns data plus the revision it saw.
3. Back on the main thread: **apply only if that revision is still current.** Otherwise discard the result and fetch again
   (bounded: a third miss falls back to one synchronous read on the main thread, so the screen can never stay stale).
4. Edits stay on the main thread, as now. Nothing about typing changes.

| | Apple | Linux |
| - | ----- | ----- |
| Background step | `Task.detached` (as `ProjectSession.loadAsync`) | `AsyncCall::run` (as `EditorShell::load`) |
| Back to the main thread | `await MainActor.run` | `AsyncCall`'s main-thread callback |
| At open | the timeline read joins the existing background load, after the scene bodies, and counts on the progress bar (I-0285 AC5) | the same |

### D4 as built (2026-10-10)

- One more endpoint, **`scrivi_get_project_revision`**: the project's current revision in the envelope, touching no file. Step 3's
  "is it still current?" is one cheap call on the main thread, not a guess from the main thread's own writes (the background load
  itself makes calls that count as writes).
- **At open, both platforms:** the worker reads the timeline after the scene bodies (one `scrivi_load_timeline`), counted as the
  progress bar's last step (total = scenes + 1; the label still counts scenes only). The main thread applies it if
  `projectRevision` still equals the read's revision, otherwise reads again — one call. Linux: `EditorShell::fetchTimelineData` /
  `applyTimelineData`; Apple: `ProjectSession.loadAsync` + `TimelineViewModel.apply`.
- ⚠️ **A reload AFTER an edit stays on the main thread** on both platforms (as Apple already did), but is now one call, not
  ~2 per scene on Linux. Moving it to the background as well is not done; it is a smaller cost and changes when the panel redraws.
- **Lock cost (Q1):** both platforms sum every `lockWaitMs` the core reports, across threads, and log it at the end of each load
  (Linux `SCRIVI_LOAD_LOG`; Apple `[SCRIVI-TIMING]`).

### D5 — Linux's bridge: the failure travels with the result

`lastCallFailed_` becomes per call: each bridge method returns its failure with its result (or, minimally, the flag becomes
`thread_local`). Required before any new background call on Linux; it is also a latent defect in today's background reads (→ **Q4**).
✅ **As built:** `thread_local` (the flag's contract is "read immediately after the call", always on the same thread), plus a
per-thread `lastRevision()`. All 191 call sites unchanged.

### D6 — World writes: the existing lock file, plus an in-process queue (Q2, as built 2026-10-10)

- ✅ **A world write lock already existed:** `WorldWriteGuard` (I-0144, SP-116) takes a lock FILE on the package for every
  world write (`ObjectStore`, `ObjectIndex`, `AssetStore`), works across processes and on network volumes, and never locks reads —
  exactly Q2's ruling. ⚠️ But a contended acquire is **refused** ("worldLocked"), not queued (§4.5 of the world design: a writer
  must never stall on another PROCESS).
- ⛔ **Inside one process that refusal is a defect:** two windows on projects sharing a world, writing at nearly the same time.
  Measured: two threads × 30 object creates into one shared world → **46–47 of 60 refused**, 13–14 saved (three runs).
- ✅ **Fix:** `WorldWriteGuard` first takes an in-process `std::recursive_mutex` per package (lexically normalised path), then the
  lock file. Same-process writers queue; other processes still meet the file and are refused. Recursive because `WorldLock` is not
  reentrant: a nested acquire behaves as before (the file refuses it) instead of hanging. Lock order: project → world mutex →
  world lock file; two worlds are taken in worldID order (as before). → 60 of 60 saved.

## 4. Rejected

- **Serialising on the platform side** (a Swift actor and a Qt worker thread that own all core calls). The same rule written
  twice, in two languages, drifts; and it does nothing for a third platform. Synchronisation is the core's job; the core is the only
  shared code.
- **Making every caller fetch on the main thread.** That is today's freeze (I-0285).
- **File locks (`flock`)**. They address two processes, which is out of scope, and behave differently over SMB.

## 5. Tests

- **Boundary, through `scrivi_*`** (not the facade — a facade test cannot see a boundary gap): every exported project endpoint is
  wrapped; every envelope carries `revision`; writes bump it, reads do not.
- **Stress:** one thread repeatedly merges and splits scenes while another repeatedly calls `scrivi_load_timeline` and
  `openSceneForBulkLoad`; every read result must be a state the writer actually produced (scene counts and IDs consistent).
  Run under ThreadSanitizer on macOS and Linux.
- **L3, on each platform:** an edit made while a background timeline read is in flight must not be overdrawn.
- **Cost:** the I-0285 load log on the rig before and after D1 — the lock must not add measurable time.

## 6. ✅ Rulings — 2026-10-10 (user)

| # | Ruling |
| - | ------ |
| **Q1** | ✅ **Exclusive lock** (`std::mutex`). ⚠️ User concern: *"am concerned that that will increase delays."* → the lock's wait time is measured (§7), not assumed |
| **Q2** | ✅ **World locks for WRITES only** — *"A World is essentially an object database, therefore if two projects are working on the same world simultaneously and one reads a different set of objects than another temporarily, it will not break Scrivi (I believe)."* A world write takes the world's lock (project first, then worlds sorted by path); world reads take none. ⏳ The "will not break" belief is checked by a test (§7), not assumed |
| **Q3** | ✅ One composite endpoint, for the timeline only; others when measured |
| **Q4** | ✅ Linux's failure flag fixed in SP-173, before any new background call |

## 7. Measurements the rulings require

- **Lock wait (Q1):** every call records how long it waited for the project lock; the I-0285 load log prints any wait over 5 ms
  and a total per load. Run on the rig (dumas over the share) and on the Mac, before and after D1. Where an exclusive lock could
  cost: (1) during the open, a main-thread call waits for at most one background scene read (~48 ms over the share); (2) a long
  write (merge, chapter create) delays background reads; (3) ⚠️ a main thread making MANY calls while the background load runs
  pays up to one background call per call. If (3) shows up, the answer is fewer main-thread calls (as D3 does for the timeline),
  or a reader/writer lock, decided on the figures.
- **World reads without a lock (Q2):** a stress test writes world objects from one thread while another lists and reads them;
  every read must either succeed or report the object missing, never fail the project or return a corrupt object.
  - ✅ **Create and update write the object file BEFORE the index** (`ObjectStore.cpp:278 → 291`, `:382 → :393`; user's point,
    confirmed), so the index never names an unwritten object.
  - ⚠️ **Delete runs the other way** (file, then index entry — `ObjectStore.cpp:436`): briefly the index names a deleted object and a
    read reports it missing. Within Q2's ruling.
  - ⚠️ **A world READ can WRITE:** on a miss, `ObjectIndex::find` rebuilds and rewrites the index (`ObjectIndex.cpp:396`ff), so an
    unlocked read can race a locked write on the index file. The index is derived and rebuilds on the next miss, so this
    self-heals; the stress test must show it does.

## 8. Questions for ruling (original text)

- **Q1 — lock kind.** (a) **Exclusive** (`std::mutex`): every call waits for every other. Simplest, and a read never waits more
  than one call. (b) **Reader/writer** (`std::shared_mutex`): reads in parallel. ⚠️ glibc's default prefers readers, so a writer
  could wait through a run of back-to-back reads (the bulk load). Recommend **(a)**, measured; move to (b) only if a measurement
  shows readers waiting on readers.
- **Q2 — shared worlds.** (a) Also lock each world the call touches, in a fixed order (project, then worlds sorted by path).
  (b) Defer: one window per world today is the common case. Recommend **(a)**. It is the same mechanism, and a world shared by two
  open projects is a supported setup.
- **Q3 — other multi-read loads.** Composite endpoints only for the timeline now (D3), others when measured. Recommend **yes**.
- **Q4 — Linux bridge failure flag (D5).** Fix in SP-173, as a prerequisite to the new background call. Recommend **yes**.
