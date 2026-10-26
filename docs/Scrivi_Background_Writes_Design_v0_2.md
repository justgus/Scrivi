# Scrivi — Background Writes Design v0.2: the main thread is for the screen

✅ **RULED 2026-10-26** (§8): Q1–Q7 as recommended (user: *"Q1–Q7 agreed as recommended"*). Written 2026-10-26. **Supersedes** `Scrivi_Background_Writes_Design_v0_1.md` where they
differ; v0.1's rulings (Q1–Q5, 2026-10-15) and its W1–W7 stand unless a section below changes them. **Epic:** [EP-052].

**Request (user, 2026-10-26),** after [SP-188]'s AC11 Mac pass over a two-hop share took minutes per step and beachballed on
clicks and scrolls:
- *"we are being naive with regard to operations that must be performed on the main thread … the operations that must be
  performed are largely related to UI rendering and update. … we can split complex operations up into 'the part that requires
  disk access', 'the part that requires building local memory', and 'the part that requires the main thread'."*
- *"a select operation or a click to set the caret should not require disk access. We can update local memory so that the
  app remains responsive and queue the subsequent disk operations. It would be the equivalent to 'saving in the background'.
  I do not know how deep we can go with this. And we have to be careful implementing it."*
- *"Do the Spotlight updates need to be done on the main thread? … I would think a background thread is permissible since
  there is no visual element to the updates."*
- *"we should have two lock mechanisms. One for disk accesses and another for in memory accesses."*

---

## 1. The inventory (measured 2026-10-26)

**How:** the core now reports, in every project envelope, which endpoint ran, how long it ran after taking the lock, and
how many file operations it made (`trace`: `endpoint`, `ms`, `fileOps`; `ProjectLock.cpp`, `scrivi_c_api.cpp`). Each app
records the calls made ON ITS MAIN THREAD. A probe drives each gesture through the real editor:
- Mac: `MainThreadInventoryTests` (interop, on demand), a 40-scene project.
- Linux: `structural_baseline_smoke` with `SCRIVI_INVENTORY=1`, through a real `EditorShell` (it includes the real
  Inspector, which the Mac probe builds by hand).

**Why file operations are the number:** on a share each one is at least one network round trip. The rig is ~8 ms per round
trip; the two-hop loop was 700 ms for one save (write + rename) and 1 s to list a folder. Times on the local disk are all
under 20 ms and say nothing about a share.

### 1a. Mac (main thread)

| Gesture | Core calls | File ops | What touches disk |
| ------- | ---------- | -------- | ----------------- |
| Click in the same scene | 2 | **0** | — (`capture_caret_moved`, memory) |
| Click into another scene | 1 | **0** | — |
| Drag-select across scenes | 1 | **0** | — |
| Scroll to another scene | 0 | **0** | — |
| Navigator click | 1 | **0** | — |
| The Inspector follows to a new scene (every card) | 32 | **78** | `list_edges_for` ×11 (33), `list_objects` ×10 (33), `list_worlds` ×10 (10), `get_scene_notes` (2) |
| Type a word (6 keys) | 12 | 1 | `capture_note_edit` (the history log) |
| End a sentence | 12 | 1 | `capture_note_edit` (the history log) |
| Autosave (1 s after typing) | 3 | **6** | `save_scene` (5), `history_note_scene_persisted` (1) |
| Switch to another app, after typing | 5 | **138** | `extract_searchable_text` (127, Spotlight), `save_scene` ×2 (10), persisted (1) |
| Switch to another app, nothing typed | 3 | 5 | `save_scene` (5: the resume point) |
| ⌘Z (typing) | 8 | 8 | `capture_undo` (2), `save_scene` (5), persisted (1) |
| ⇧⌘Z (typing) | 9 | 7 | `capture_redo` (1), `save_scene` (5), persisted (1) |
| ⌘X across scenes (main thread's part) | 1 | 0 | — (the range call runs off the main thread) |
| ⌘Z of that cut (main thread's part) | 4 | 1 | `capture_undo` (1) |
| Opening: Spotlight donation | 1 | **127** | `extract_searchable_text` (~3 per scene; dumas: 353 s over the loop) |

### 1b. Linux (GUI thread)

A 4-scene project with no objects (the smoke's own), through a real `EditorShell` and its real Inspector.

| Gesture | Core calls | File ops | What touches disk |
| ------- | ---------- | -------- | ----------------- |
| Click in the same scene | 3 | **2** | `get_scene_notes` (the Inspector) |
| Click into another scene (the Inspector follows) | 2 | **2** | `get_scene_notes` |
| Drag-select across scenes | 2 | **2** | `get_scene_notes` |
| Scroll | 1 | **2** | `get_scene_notes` |
| Type a word (6 keys) | 12 | 0 | — |
| End a sentence | 12 | 1 | `capture_note_edit` (the history log) |
| Autosave (the save timer) | 3 | **6** | `save_scene` (5), persisted (1) |
| Ctrl+Z (typing) | 8 | 8 | `capture_undo` (2), `save_scene` (5), persisted (1) |
| Ctrl+Shift+Z (typing) | 7 | 7 | `capture_redo` (1), `save_scene` (5), persisted (1) |
| Ctrl+X across scenes (GUI thread's part) | 4 | 4 | `get_scene_notes` ×2 |
| Ctrl+Z of that cut (GUI thread's part) | 5 | 3 | `capture_undo` (1), `get_scene_notes` (2) |

Linux has no save on switching apps and no Spotlight. ⚠️ With no objects, its Inspector read only the scene's notes here; on
dumas over the share it took 50–290 ms per scene ([I-0317], the rig log), so its object cards read more on a real project.

### 1c. What the inventory says

1. **On the Mac, a click, a selection and a scroll make no file operation** (on Linux, only the Inspector's notes read, 2).
   They still froze over the loop, so what they waited for was **the project lock**: SP-173's lock is EXCLUSIVE (`ProjectLock.hpp`), so a caret move's
   in-memory history call queues behind any call holding the lock across a slow write (the structural edit's worker, an
   off-main save). Moving writes off the main thread (v0.1) does NOT remove that wait. → §3, the two locks.
2. **The disk work left on the main thread is five things (both platforms, Spotlight Mac only):** the Inspector's reads (78 ops a scene change), each sentence's
   history append, every scene save (autosave, app switch, undo and redo), and Spotlight's extraction (whole project).
3. **The Inspector is the largest per-gesture cost**, and it runs on every scene the caret or the scroll enters. The core
   has no in-memory copy of objects, edges or worlds: each `list_objects` builds an `ObjectIndex` from disk, each
   `list_edges_for` reads `relationships.jsonl`, each `list_worlds` reads the bindings.
4. **Spotlight is the largest single cost:** it reads every scene at open, and again at every app switch after typing.

---

## 2. The split (the user's rule, as a design rule)

Every operation is three parts, run in this order and on these threads:

| Part | Thread | Example (autosave) |
| ---- | ------ | ------------------ |
| **Memory**: update the in-memory state, decide what must reach disk | any thread, under the **memory lock** (short) | the scene's text is now X; history knows it; a write job is queued |
| **Disk**: read or write files | the project's **writer thread**, under the **disk lock** | write the scene file, its sidecar, the history append |
| **Screen**: draw the result | main thread only | (nothing: the text is already on screen) |

**R1 — Nothing a writer does with the caret, the mouse or the scroll touches the disk on the main thread** (click, select,
scroll, navigator, the Inspector following). ⛔ It also never WAITS for the disk: it takes only the memory lock.
**R2 — Every write is queued** (v0.1's W1–W7, unchanged): autosave, app switch, undo and redo writes, Replace All,
structural edits, the history appends.
**R3 — Reads the screen needs run off the main thread** and hand their result to it (the Inspector, the timeline,
Spotlight). A result that arrives after the state changed is dropped (SP-173's revision rule, L3).
**R4 — The main thread never holds a lock while it waits.** It takes the memory lock briefly, never the disk lock.

---

## 3. Two locks per project (the user's proposal)

Today (SP-173, D1) one exclusive mutex per project wraps every endpoint. v0.2 replaces it with two:

- **The memory lock (M):** guards the project's in-memory state: the history tree, the scene index (`registrySceneLocator`),
  the revision (D2), and the write queue's bookkeeping. Held only while that state is read or changed: microseconds,
  never across a file operation.
- **The disk lock (D):** guards the project's files. Held by the project's writer thread for each job, and by any read that
  must see the files (a read job, §4). One holder at a time, so writes and reads stay in queue order.

**L-order rule: D before M, never M before D.** The writer thread holds D, and may take M briefly to record what landed.
A memory call holds M and never asks for D: it only queues a job. No cycle, so no deadlock. The main thread takes M only
(R4).

**What each kind of call becomes:**

| Call | Today (one lock) | v0.2 |
| ---- | ---------------- | ---- |
| Memory-only (caret moved, capture state, note applied) | waits for the project lock | M only: never waits on a file |
| A write (save scene, history append, undo's writes) | the main thread writes, under the lock | M: update memory, queue the job, return a ticket. D: the writer thread writes |
| A read the screen needs (Inspector, timeline) | the main thread reads, under the lock | a read job, off the main thread, under D, in queue order (sees every earlier write) |
| A structural edit | its worker holds the lock for the whole call | unchanged for now (§6): a queued job under D, read-only editor until its delta |

**Revision (D2) moves under M:** it changes when memory changes, not when the disk catches up. A read job's result carries
the revision it was read at; the platform drops it if the revision moved since (L3, unchanged).

**What memory must hold for R1 to be true:** today the history tree and the scene index are in memory. The caret, selection
and scroll calls already touch only those (§1a: 0 file ops). ✅ So R1 needs **no new cache**, only M instead of the
exclusive lock.

---

## 4. Reads: the Inspector, the timeline, Spotlight

- **R3a — The Inspector loads off the main thread.** On a scene change it shows the scene's cards at once (their last
  contents, greyed, or empty with "Loading…"), asks for the card data as ONE read job, and fills the cards when it lands
  (revision-checked). Linux's [I-0317] (autoscroll freezes, 50–290 ms per scene) is the same fix.
- **R3b — Fewer reads per scene change:** one call that returns every card's data (the scene's edges with their far
  objects' names and kinds) replaces 32 calls. ⏳ Measure before deciding whether the core should also keep the object
  index in memory (Q4).
- **R3c — Spotlight is entirely off the main thread** (the user's question; §5).

## 5. Spotlight

**Can it leave the main thread? Yes.** Apple's documentation for `CSSearchableIndex` sets no main-thread requirement. Its one
rule: *"Modify a CSSearchableIndex object from only one thread or task at a time … It's a programming error to access a
custom index from multiple threads simultaneously."*
([developer.apple.com/documentation/corespotlight/cssearchableindex](https://developer.apple.com/documentation/corespotlight/cssearchableindex);
`indexSearchableItems` and `deleteSearchableItems` take completion handlers and are not async.) Nothing is drawn.

**Design:**
- **S1 — One serial background queue for every Spotlight call** (donate, re-index, delete on close): it satisfies Apple's
  one-at-a-time rule.
- **S2 — The extraction is a read job** (`extract_searchable_text`): off the main thread, after the queue's earlier writes.
- **S3 — Re-index only the scenes that changed**, not the project: the resign path today re-extracts all of it (127 ops for
  40 scenes) whenever any scene was typed in. The donor already keys items by scene, so a per-scene update replaces them.
- **S4 — It yields:** a Spotlight job runs behind the writer's own saves, and a close cancels a pending one.
- Only `spotlightDomainIdentifier` is main-actor state; it is set when the job lands.

---

## 6. How deep (levels)

| Level | What the writer gets | Status |
| ----- | -------------------- | ------ |
| **1** | Click, select, scroll, navigator never touch or wait on the disk | §3 (M); Mac ✅ already 0 file ops; Linux: the Inspector's notes read moves off (§4) |
| **2** | Every write queued: autosave, app switch, undo/redo writes, history appends | v0.1 W1–W7 |
| **3** | No main-thread call waits on disk I/O | §3 (two locks) |
| **4** | Reads off the main thread: Inspector, timeline, Spotlight | §4, §5 |
| **5** | Structural edits applied in memory first, written behind | ⛔ **Not now (Q6)** |

**Why not level 5 yet:** a structural edit's core call checks its range against the files and creates scene IDs and
filenames. Applying it in memory first means the core must plan it from memory alone, and a write that fails afterwards
leaves the screen ahead of the disk with structure, not just text. Levels 1–4 remove every freeze the inventory found
except the structural edit's own (already off the main thread, with its progress row).

---

## 7. What makes it careful

| Hazard | Rule |
| ------ | ---- |
| Writes land out of order | One FIFO queue per project (v0.1 W1). Coalescing never crosses a structural job (W7e) |
| A queued write fails (drive pulled, share drops) | Memory keeps the writer's text; the scene stays unsaved ([I-0259]/[I-0304], now ONE rule in the core); the banner ([T-0570]) shows; the queue retries when the project is reachable. ⚠️ Later jobs that depend on it wait (a structural job behind a failed save must not run) |
| A read sees a file before its queued write | Read jobs run in queue order under D (v0.1 W3, now for reads off the main thread) |
| Memory ahead of disk when the app crashes | Accepted (v0.1 Q4: no journal); the window is the queue's length. Close and quit drain it |
| Another machine changes a file (external change) | "The disk wins" ([EP-053] Q3) must skip a file with a queued write: ours is newer |
| The writer repeats a gesture with no visible effect | Immediate acknowledgment (the 2026-10-21 discussion): ⏳ Q5 |
| Linux drifts from the Mac | Both adopt each level in the same Sprint (`feedback_linux_adopts_apple_shape`) |

---

## 8. Questions for ruling — ✅ ruled 2026-10-26

✅ **User, 2026-10-26:** *"Q1–Q7 agreed as recommended"*: every row below is ruled as its Recommendation column reads.
S1 is [SP-194].

| # | Question | Recommendation |
| - | -------- | -------------- |
| **Q1** | Two locks per project (M, D), D-before-M, the main thread on M only (§3)? | **Yes**, as the user proposed. It replaces SP-173's single exclusive lock; the concurrency tests move with it |
| **Q2** | Read jobs for the Inspector and timeline, results dropped when the revision moved (§4)? | **Yes** |
| **Q3** | Spotlight: one serial background queue, per-scene re-index, behind the saves (§5)? | **Yes** |
| **Q4** | Keep objects, edges and world bindings in memory in the core, so the Inspector reads memory? | **Not yet.** Do R3a/R3b first (off the main thread, one call), then measure on the rig; cache only if still slow |
| **Q5** | The 2026-10-21 gesture rules: an immediate cue for every queued gesture; a repeated gesture absorbed; ⌘Z behind a pending job? | **Cue: yes** (it exists for structural edits: read-only + progress row). **Repeat: ignored while pending** (as Q5 of EP-053). **⌘Z: waits its turn in the queue** (never cancels a write) |
| **Q6** | Level 5 (structural edits in memory first)? | **Not now** (§6) |
| **Q7** | Sprint order? | **S1** core: the two locks + the queue (v0.1 AC1–AC3) · **S2** Mac + Linux: writes queued (levels 1–3) · **S3** Mac + Linux: reads off the main thread + Spotlight (level 4) · each with a live pass over the loop |

---

## 9. Measurement record

- 2026-10-26, Mac, local disk, 40 scenes × 6 paragraphs: `TEST_RUNNER_SCRIVI_INVENTORY=1 scripts/run-interop-tests.sh
  -only-testing:ScriviInteropTests/MainThreadInventoryTests` (§1a).
- 2026-10-26, Linux (Docker, Qt 6.4, offscreen, non-root): `SCRIVI_INVENTORY=1 platforms/linux/tests/structural_baseline_smoke.sh build/platforms/linux/scrivi_linux_structural_baseline_smoke` (§1b).
- The SP-188 loop pass (2026-10-26, the Mac's unified log): the open spent 352.7 s in `donateSpotlight`; app switches
  3.5 s to 3 min 18 s (`stampWritingSurfaceBlocking`, the resign re-index); a caret scene change's save ~4.5 s.
