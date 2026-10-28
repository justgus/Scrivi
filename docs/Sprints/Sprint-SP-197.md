---
sprint: SP-197
epic: EP-052
status: Active
platform: Cross
created: 2026-10-27
---

# SP-197 — `[Cross]` [EP-052] **S3**: reads off the main thread, and Spotlight in the background

**Status:** 🟡 **ACTIVE 2026-10-27** (user: *"activate and implement SP-197"*) — created 2026-10-27 (user: *"plan S3"*). Activated
beside [SP-195] and [SP-196], which need only their rig passes. ✅ Q1–Q3 ruled 2026-10-27 (user: *"Q1–Q3 agreed as recommended"*).
**Carries:** [T-0629], [T-0630], [T-0631] → [`../Tasks/Task-unverified.md`](../Tasks/Task-unverified.md).
**Epic:** [EP-052] (AC8, AC9) → [`../Epics/Epic-active.md`](../Epics/Epic-active.md).
**Authority:** [`../Scrivi_Background_Writes_Design_v0_2.md`](../Scrivi_Background_Writes_Design_v0_2.md) R1, R3, §4
(R3a, R3b), §5 (S1–S4); Q2, Q3 ruled, Q4 "not yet" · `Scrivi_Core_Concurrency_Design_v0_1.md` D2, D4 (the revision rule).

---

## What reading the code found (2026-10-27)

- **The Mac Inspector reads on the main thread.** Each card's model reads in its `init`: `ObjectCard.swift:157,179`
  (`listEdgesFor`, `listObjects` per kind), `:614` (`listWorlds`), `SourcesCard.swift:89,107` (the scene's edges, then
  each source's citations), `WritingToolCards.swift:37` and `ScenePropertiesView.swift:133` (`getSceneNotes`). The v0.2
  inventory: 32 calls and **78 file operations** each time the Inspector follows to a new scene.
- **The Linux Inspector is half done.** Its object cards already read on a worker (`SceneInspector.cpp:399`, `AsyncCall`
  with its own `WorkerBridge`); the scene's notes still read on the GUI thread (`SceneInspector.cpp:856`), which is the
  2 file operations every click, select and scroll makes in the Linux inventory.
- **Each `list_objects` builds the object index from disk** (v0.2 §1c.3): ten kinds, ten builds. Moving the same calls
  off the main thread stops the freeze but not the wait for the cards to fill over the share.
- **The timeline:** the open already reads it on a worker and applies it if the revision has not moved
  (`ProjectSession.swift:323`), but its fallback reads again on the main thread (`:328`). Linux's `reloadTimeline()`
  (`EditorShell.cpp:3351`) reads on the GUI thread.
- **Spotlight is all on the main thread** (`ProjectSession.donateSpotlight`, `:550`): `extract_searchable_text` reads the
  whole project at open (dumas over the loop: 353 s) and again on an app switch after typing (`:541`). The core's
  extraction takes a project path only (`scrivi.h:684`), so a per-scene re-index needs a scene filter.
- **The revision check exists** (`scrivi_get_project_revision`, `scrivi.h:667`, touches no file), and synchronous calls
  already wait for the writes queued before them (EP-052 AC1), so a background read sees every earlier write. No new
  queue is needed for reads.

## Goal

A click, select, scroll or navigator click makes **no file operation on the main thread**, on either platform: the
Inspector and the timeline each load as one read job off the main thread, and their result is dropped if the revision
moved. Spotlight never runs on the main thread: one serial queue, per-scene re-index, behind the saves.

## Plan

1. **[T-0629] `[ScriviCore]` Two reads:** (a) one call that returns everything the Inspector shows for a scene (its notes,
   its edges with each far object's name, kind, image path and pending state, the sources' citations, the bound worlds'
   status), building the object index once (R3b; Q1 ✅); (b) `extract_searchable_text` for a list of scenes. Boundary
   tests through `scrivi_*`, with the read count asserted (CountingFileSystem).
2. **[T-0630] `[Cross]` Mac and Linux, in the same Sprint:** the Inspector shows the new scene's cards at once and fills
   them from ONE read job (Q2 ✅); the timeline's reloads are read jobs; a result whose revision moved is dropped and read
   again. The inventory probes (`MainThreadInventoryTests`, `SCRIVI_INVENTORY=1 structural_baseline_smoke`) assert **0
   file operations** for click, select, scroll, navigator and the Inspector following.
3. **[T-0631] `[Apple]` Spotlight (S1–S4):** one serial background queue for every `CSSearchableIndex` call; the open's
   donation and the app-switch re-index run there, after the queue's saves; the app switch re-indexes only the scenes
   typed in; a close cancels a pending job. The inventory's open and app-switch rows read 0 on the main thread.
4. **Live passes:** dumas over the share, Mac and rig: scroll, click and navigate through the manuscript with the
   Inspector open, none freezing; the cards fill; the Mac switches apps after typing with no beachball.
5. **Then measure Q4** (v0.2: "cache only if still slow"): time the Inspector filling on the rig. A slow fill becomes a
   ruling for after this Sprint, not work in it.

## Questions for ruling — ✅ ruled 2026-10-27

✅ **User, 2026-10-27:** *"Q1–Q3 agreed as recommended"*: every row below is ruled as its Recommendation column reads.

| # | Question | Recommendation |
| - | -------- | ---------------- |
| **Q1** | R3b: one core call for the Inspector, or the same calls batched in one background job? | **One core call.** It builds the object index once (≈10 builds today), so the cards fill sooner over the share; it is also what lets Q4 be measured on its own. One new endpoint |
| **Q2** | What the Inspector shows while its read is out | **The previous scene's contents dimmed at once; "Loading…" after 400 ms** (the Q5 progress-row pattern). Editing a card is off until its data lands |
| **Q3** | Spotlight on Linux? | **None.** Linux has no Spotlight (v0.2 §1b); AC9 is `[Apple]` only |

## Acceptance Criteria

- [x] AC1 — `[ScriviCore]` The Inspector read and the per-scene extraction, with boundary tests and read counts
- [x] AC2 — `[Apple]` The Inspector and the timeline load off the main thread, revision-checked; the inventory reads 0
  file operations for click, select, scroll, navigator and the Inspector following (EP-052 AC8)
- [x] AC3 — `[Linux]` The same (EP-052 AC8), in this Sprint
- [x] AC4 — `[Apple]` Spotlight on one serial background queue, per-scene, behind the saves; a close cancels it (EP-052 AC9)
- [ ] AC5 — ✅ **Mac and rig live passes (user)** over the share, as plan step 4

## Progress log

### 🟠 2026-10-27 — Rig failure fixed (user: *"go ahead with the fixes"*), not yet re-passed
- **Measured** (local copy of dumas): one `scrivi_load_scene_inspector` = 25 file ops, 45 ms. It reads ONE scene's notes
  and edges; its objects part loads the whole object index (672 objects) and keeps the linked few (a later narrowing).
- **Cause:** every scrolled-past scene started a read and none was stopped (a cancelled Swift task, a superseded Qt
  worker), each holding the disk lock; on Linux they also filled the global thread pool.
- **Fix, both platforms:** one Inspector read out at a time; a scene change while it is out reads the scene wanted when
  it lands (`SceneInspector::startRead`, `SceneInspectorReader.start`). A slow read is waited for (a "Still reading…"
  line at 5 s on Linux), never abandoned; the 10-minute backstop stays.
- **Logging:** a background core call ≥ 100 ms or waiting ≥ 100 ms for the lock is logged (Linux
  `workerCoreCallObserver`, Mac `core (background):`).
- **Tests:** interop 324/324 (new: ten scene changes → ≤ 4 reads, the last lands); smokes 38/38 (same check, Linux); the
  Linux check fails on a mutation in a copy (guard removed: 10 reads). No Mac mutation run.

### ❌ 2026-10-27 — Rig live pass FAILED (AC5)
- User: A.1 and A.2 (scroll, click) *"passed ok. But the app never enabled the Scene Inspector elements"*; *"The scene's
  notes are taking longer…"* never went away (dumas-prose-timelines on the removable drive, over the share; build 303,
  `0be59bc`). The window stayed responsive.
- Rig log: the Inspector's read timed out and never retried; a late result is dropped (`SceneInspector.cpp:447`), so the
  cards stay off for good. The log does not record worker calls, so what held the project's disk lock is NOT proven.
  Suspected: every superseded scene's composite read still runs, one after another, each holding the lock for its whole
  length over a slow share.

### 🟠 2026-10-27 — T-0629, T-0630, T-0631 Implemented - Not Verified (AC1–AC4)
- **Core:** `scrivi_load_scene_inspector` (one read, parts shared with the standalone endpoints, an empty part `{}`) and
  `scrivi_extract_searchable_scenes`; each envelope's trace names its project. ctest 786/786 (5 new). Fixture: 67 file ops
  for the one read against 353+ for the calls it replaces. ⚠️ One unidentified ctest failure in the first full run after a
  rebuild; 18 full runs since are clean.
- **Mac:** `SceneInspectorReader` — one read per scene off the main actor, every card and the Properties tab built from it,
  revision-checked, the last contents dimmed while it is out, "Loading…" after 400 ms, a card's write re-reads. The open's
  timeline fallback re-read off the main actor. Spotlight on one serial queue, per-scene on an app switch, a close cancels.
  Interop 323/323 (2 new: 0 main-thread file ops for click, select, scroll, navigator and the Inspector; a write re-reads).
- **Linux:** the Inspector worker makes the one call, the notes included; revision-checked; dimmed + "Loading…" after
  400 ms; the load's stale timeline re-read on a worker. Smokes 38/38; the new SP-197 gate in `structural_baseline_smoke`
  is caught by a mutation in a copy (the notes read back on the GUI thread). Docker build clean.
- **Also fixed:** the visionOS engine stub lacked five SP-195/196 methods (`check-engine-stub-parity.sh` now clean).
- **Decisions for review:**
  1. A read whose revision moved is read again up to 3 times, then applied anyway (autosave can bump the revision faster
     than a slow share answers). A card's own write always re-reads.
  2. The cards' own writes (tags, outline, todo, relate, rename, remove) still write on the main thread, as before;
     after each the Inspector re-reads. They are not click/select/scroll (AC8) and are not in this Sprint.
  3. The timeline's re-reads after the writer's own timeline edits (structure, events, imported timelines) stay where they
     were; only the open's fallback moved, on both platforms.
  4. The Object picker still reads when it opens (a deliberate action, not a scene change).
- ⏳ AC5: the live passes (user).

### 🟡 2026-10-27 — Sprint ACTIVATED
- User: *"activate and implement SP-197"*. T-0629–T-0631 → active.

### ✅ 2026-10-27 — Q1–Q3 ruled
- User: *"Q1–Q3 agreed as recommended"*: one core call for the Inspector; the previous cards dimmed, "Loading…" after
  400 ms, editing off until the data lands; no Spotlight on Linux. Waits only on the SP-195 and SP-196 rig passes.

### 🔵 2026-10-27 — Sprint CREATED in Planning
- User: *"plan S3"*.
