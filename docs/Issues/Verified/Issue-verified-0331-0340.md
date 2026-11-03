# Verified Issues: I-0331 – I-0340

Issues the user has VERIFIED, archived out of `Issue-active.md`.
⚠️ **A new decade.** ✅ **I-0321–I-0330 are in [`Issue-verified-0321-0330.md`](Issue-verified-0321-0330.md).**

| ID | Title | Severity | Sprint | Status |
| -- | ----- | -------- | ------ | ------ |
| **I-0331** | `[Apple]` ⛔ **The Mac runs the undo history on the main thread: the History card re-read the whole tree on every scene change (~1.9 s each on dumas), and the open replayed the log there (1.8–2.6 s).** Found in the SP-197 Mac live pass log, 2026-11-01. → [details](#i-0331) | High | [SP-197] | ✅ **Verified** (user, 2026-11-01: Mac re-check) |
| **I-0332** | `[ScriviCore]` ⛔ **Opening a project's undo history costs ~2 s on dumas (34 MB log) and grows with the history: a whole-manuscript copy per history node, and an unoptimised parse of the log.** Measured 2026-11-01 for the user's long-history work. → [details](#i-0332) | High | [SP-197], [SP-200] | ✅ **Verified 2026-11-02** ([SP-200]'s Mac pass and [T-0653] on the VM (checklist: Pass)) |
| **I-0333** | `[Apple]` ⚠️ **Replace All across 1,174 scenes holds the window: 6 s on dumas-prose-timelines, 18.6 s on dumas-prose-timelines-history (`apply=13427 ms`).** Found 2026-11-01 in [SP-200]'s Mac live pass log (not that Sprint's code). → [details](#i-0333) | High | [SP-201] | ✅ **Verified 2026-11-02** (Mac live pass (user, 2026-11-01) and [T-0655] on the VM (checklist: Pass)) |
| **I-0334** | `[Apple]` ⚠️ **Main-thread core calls waited up to 15 s for the project lock behind Replace All's queued saves.** Found 2026-11-01 in [SP-200]'s Mac live pass log. → [details](#i-0334) | High | [SP-201] | ✅ **Verified 2026-11-02** (Mac live pass (user, 2026-11-01) and [T-0655] on the VM (checklist: Pass)) |
| **I-0335** | `[Linux]` ⛔ **Close Project never finishes: the "Please wait — saving updates…" dialog flashes forever and one scene is rewritten ~3 times a second.** Found 2026-11-02 on the Linux VM, [T-0648] (EP-057). → [details](#i-0335) | High | — | ✅ **Verified 2026-11-02** ([T-0648] passed on the VM (checklist: Pass)) |
| **I-0336** | `[Linux]` ⛔ **Close Project released the project BEFORE asking "Some changes can't be saved": Cancel kept a window whose project was already closed — its held save never landed, its history was closed.** Found 2026-11-02 on the Linux VM, [T-0648] steps 5–6 (EP-057). → [details](#i-0336) | High | — | ✅ **Verified 2026-11-02** ([T-0648] passed on the VM (checklist: Pass)) |
| **I-0337** | `[Linux]` ⚠️ **The Inspector's cards take 3–5 s to fill over the share, and no "Loading…" appears on the Writing or Properties tab: the cue was only on the Worldbuilding tab, and every scene's cards waited for a whole-project metrics count.** Found 2026-11-02 on the Linux VM, [T-0650] (EP-057). → [details](#i-0337) | Medium | — | ✅ **Verified 2026-11-02** ([T-0650] passed on the VM (checklist: Pass)) |
| **I-0338** | `[Linux]` ⚠️ **The timeline chained all 1,175 scenes into ONE aggregate and Zoom to Current Scene could not separate it: Linux still clustered by transitive proximity chaining, the defect [I-0208] fixed on Apple only.** Found 2026-11-02 on the Linux VM, [T-0652] (EP-057). → [details](#i-0338) | High | — | ✅ **Verified 2026-11-02** ([T-0652] passed on the VM (checklist: Pass)) |
| **I-0339** | `[Linux]` ⛔ **Undo of a Replace All held the window for 146 s: each restored scene moved the caret or scroll, and every move re-queued every scene restored so far, on the main thread.** Found 2026-11-02 on the Linux VM, [T-0655] (EP-057). → [details](#i-0339) | High | — | ✅ **Verified 2026-11-02** ([T-0655] passed on the VM (checklist: Pass)) |
| **I-0340** | `[Linux]` ⚠️ **The Timeline could not zoom past 500×, so with imported history framing the strip, Zoom to Current Scene still left ~78 scenes in each aggregate.** Found 2026-11-02 on the Linux VM, [T-0652] (EP-057). → [details](#i-0340) | High | — | ✅ **Verified 2026-11-02** ([T-0652] passed on the VM (checklist: Pass)) |

---

## I-0331

**Title:** The Mac runs the undo history on the main thread (open, and the History card's tree)
**Status:** ✅ Verified 2026-11-01 (user, 2026-11-01: *"The re-check passed, no history calls on main."* The log: `scrivi_history_open` on `[worker]`, no `scrivi_history_get_tree` or `scrivi_list_pending_edges` lines)
**Platform:** `[Apple]` (Mac and iOS: shared code) and `[ScriviCore]`
**Severity:** High. A main-thread block of ~1.9 s on most scene changes and 1.8–2.6 s at every open.
**Sprint:** [SP-197]
**Date Identified:** 2026-11-01, from the console of [SP-197]'s Mac live pass (dumas-prose-timelines on the removable drive):
`[main] core: scrivi_history_get_tree 1857–1887 ms, 0 file ops` after most scene changes, and `[main] core: scrivi_history_open
1840–2593 ms, 5 file ops` at each open. The user reported no freeze. User: *"why does scrivi_history_open run on the main
thread? … We should strive to call it only when absolutely necessary."*

**Root cause:**
1. **The open:** `ProjectSession.finishLoad` called `HistoryCapture.open()` and `validateScenes` on the main actor. Linux
   already does both on its load worker (`EditorShell.cpp:614`). The open itself is called once per project open, which is
   needed; the call count was not the problem.
2. **The tree:** the History card keyed its reload on the scene AND the history revision (`HistoryCard.swift:71`), so every
   scene change re-fetched the tree, on the main actor. The tree is the project's window around the current node, the same for
   every scene; the card filters it by scene itself.
3. **Its cost:** `HistoryService::getTree` rebased the change offset (I-0107) of EVERY node on the spine against every later
   node, O(spine²), though it returns at most 200 nodes.

**Resolution:**
1. `loadAsync`'s worker opens the history and validates the scenes (`HistoryCapture.openOffMain`, `HistoryCapture.swift:97`;
   called at `ProjectSession.swift:227`); the main actor only adopts the result (`adoptOpened`, `:356`). The deprecated
   synchronous `load(at:)` keeps the main-actor path.
2. The card reloads on the history and its revision only (`HistoryCard.swift:66`), and `HistoryCapture.tree` reads off the
   main actor (`HistoryCapture.swift:285`).
3. `getTree` rebases only the nodes it returns (`HistoryService.cpp:826`), the same math per node.
**Verification:** interop test `historyOffMain` (`ScriviInteropTests.swift:8582`): no `scrivi_history_open`,
`scrivi_history_validate_scene` or `scrivi_history_get_tree` on the main thread; fails on a mutation in a copy (the main-actor
open restored). ctest 791/791 (the I-0107 rebase tests unchanged), Mac interop 328/328, iOS 17/17. ⚠️ No timing test of
`getTree` on a long history; the 1.9 s figure is to be re-measured in a live pass. `scrivi_history_open`'s 1.8 s replay
itself is unchanged, only moved off the main thread.

---

## I-0333

**Title:** Replace All across the whole manuscript holds the window for seconds
**Status:** ✅ **Resolved - Verified 2026-11-02** — Mac live pass (user, 2026-11-01) and [T-0655] on the VM (checklist: Pass); archived on the user's authority (user, 2026-11-02: *"on my authority, closed all the objects that have now tested completed"*). ⚠️ PRIOR: 🟠 Resolved - Not Verified (2026-11-01, [SP-201]) — ✅ Mac live pass (user, 2026-11-01: *"the Mac live pass steps all
passed."*); ➡️ the Linux half → [T-0655] ([EP-057], the rig); may be Verified on the Mac pass
**Sprint:** [SP-201]
**Platform:** `[Apple]` (the Mac; ✅ Linux's Replace All already had the fix's shape and gets the same window)
**Severity:** High. A writer's Replace All on a long manuscript freezes the window; a large op may take time only with
visible progress (user, 2026-10-17).
**Date Identified:** 2026-11-01, in [SP-200]'s Mac live pass log (step 4: replace `the` with `thee`, Replace All).
**Measured:** 1,174 scenes changed. dumas-prose-timelines: the window held ~6 s. dumas-prose-timelines-history (the
generated 25,000-step history): 18.6 s, of which `apply=13427 ms`.
**Cause (measured 2026-11-01, SP-201 step 1):** `applyReplacements` made, for EACH match, on the main thread: a core call
(`scrivi_markdown_balanced_edit`) that was sent the match's WHOLE scene, a snap that rebuilt its block analysis, and a
storage edit. On a 1.76 MB manuscript of 110 scenes (84,480 matches) that was 42 s: 37 s the core calls, 2.9 s the snap.
On dumas's own 1,175 scenes (41,296 matches) the core calls were 1.6 s of `apply=4067 ms`.
**Fix (Linux's shape, `EditorShell.cpp`):**
1. The core is sent only the blocks an edit touches (`ScriviEngine.markdownEditWindow`, `ScriviEngine.swift:3752`;
   `ScriviBridge::markdownEditWindow`, `ScriviBridge.cpp:1928`) — the core's `balancedEdit` reads nothing else. Every caller
   on both platforms gains: 37 s → 3.7 s on the 110-scene test; 1.6 s → 1.05 s on dumas.
2. Mac `Coordinator.replaceAll` (`ManuscriptTextView.swift:2171`): the matches are snapped once (one block lookup for the
   whole pass) and grouped by scene on the main thread; each scene's text is computed OFF it (`replaced`, `:2252`); the
   scenes go in back to front in ~50 ms slices, one storage edit per slice, with *"Replacing N of M…"* in the progress row
   (`ViewportSceneLoader.beginLocalEdit`/`endLocalEdit`, `:623`, `:635` — the structural edits' busy state: read-only,
   undo waits its turn, a close waits for the edit and then its saves). One undo step, as before.
**Measured after (84,480 matches, 110 scenes):** main thread 573 ms (snap) + 109 ms (apply) + 1,248 ms (the one history
record), the window answering between; the texts computed off it in 2.4 s. Before: 42 s in one block.
**Tests:** `BalancedEditWindowTests` (`ScriviInteropTests.swift:4189`): windowed == whole scene in >20,000 cases (>1,000
reaching the balancing); a mutation cutting the window at single newlines fails it (1,810 cases). `replaceAllCoreCallsOffMain`
(`:7975`): 121 matches make <20 main-thread core calls (a mutation computing on the main thread fails it). Linux
`format_smoke`: the same equivalence through `ScriviBridge`.
⏳ dumas on the Mac and the rig (live passes); the generated history's ~3× (not re-measured: the harness cannot build 1,175
scenes in one chapter — see the Sprint).

## I-0334

**Title:** The main thread waits for the project lock behind queued saves
**Status:** 🟠 Resolved - Not Verified (2026-11-01, [SP-201]) — ✅ Mac live pass (user, 2026-11-01: *"the Mac live pass steps all
passed."*); ➡️ the Linux half → [T-0655] ([EP-057], the rig); may be Verified on the Mac pass
**Sprint:** [SP-201]
**Platform:** `[Apple]` (seen on the Mac; [EP-052]'s write queue is shared with Linux)
**Severity:** High. The main thread is for UI only (user, 2026-10-26); a main-thread call that waits on the lock freezes
the window for as long as the queue holds it.
**Date Identified:** 2026-11-01, in [SP-200]'s Mac live pass log, after the Replace All of [I-0333].
**Measured:** main-thread core calls that waited up to 15 s for the lock (`lockWaitMs`) while Replace All's 1,174 saves drained
through the write queue.
**Cause (the Mac's log, 2026-11-01, `[SCRIVI-SPAN] [main] core:` lines):** `scrivi_list_worlds` — 36 waits, up to
16,352 ms — from `AppEnvironment.reconnectWorlds` → `activateWorlds`, run when the writer comes back to Scrivi; and
`scrivi_put_inspector_layout` (519 ms), the layout's save. Both on the main thread, both needing the disk lock the queued
saves held.
**Fix:** `reconnectWorlds` reads the worlds off the main actor, in the same task as (and before) the pending sweep and the
cards' re-read, so [I-0123]'s order holds (`activateWorldsOffMain`, `AppEnvironment.swift:483`). The layout's writes run in
order on a serial queue (`InspectorLayoutStore.swift:446`); a close waits for them (`waitForSaves`). Linux's layout store
the same (`InspectorLayoutStore.cpp:214`; a load and destruction wait); Linux already read worlds off the GUI thread.
**Tests:** `reconnectAndLayoutOffMain` (`ScriviInteropTests.swift:8657`): no `scrivi_list_worlds` or
`scrivi_put_inspector_layout` on the main thread; a mutation putting `activateWorlds` back fails it.
⚠️ Not changed: other main-thread `listWorlds` callers that run on a writer's click (the object pickers, the Worlds sheet,
the detail sheet) — none waited in the log.

## I-0332

**Title:** Opening the undo history takes seconds and grows with the history
**Status:** 🟠 Resolved - Not Verified (2026-11-01) — parts 1 and 2 (a) done: 2.05 s → 0.25 s per open on dumas. What
grows with the log is [SP-200]'s ((b), (c)).
**Platform:** `[ScriviCore]` (both platforms open the history through `scrivi_history_open`)
**Severity:** High. Every project open waits for it (off the main thread since [I-0331], but the window waits on the load).
**Sprint:** [SP-197]
**Date Identified:** 2026-11-01. User: *"the reason I'm having you do this is so that we can determine where the app slows
down when the history gets large … We want to immediately figure out how to fix it."* dumas-prose-timelines's history:
34 MB, 12,245 log lines (≈4,600 events, 98 sessions, 1,194 scenes, 13 forks; four Replace All barriers of 3.6–6.4 MB each).

**Measured** (`sample`, Debug build, `scrivi_make_test_history`'s per-call timing; ~2.05 s per open):
1. `HistoryService::pruneInconsistentNodes` (~45%): its DFS pushed every child with a COPY of the whole per-scene text map,
   so a 4,600-node, nearly linear history made thousands of whole-manuscript copies.
2. Parsing the log, line by line through nlohmann (~55%; ~90% after part 1). Most of the bytes are records that store whole
   texts: structural-cut payloads (38%; the four 3.6–6.4 MB lines are `rangeCut`s across ~1,049 scenes, NOT Replace All as
   first said), Replace All's one-change diffs (20%), and barrier before/after texts (27%).

**Part 1 — fixed:** the last child takes the map by move; only extra siblings at a fork get a copy
(`HistoryService.cpp:1232`). Same traversal, same answers. ~2.05 s → ~1.35 s per open. ctest 791/791 (54 `[history]` cases).
**Part 2 (a) — fixed (user, 2026-11-01: *"I'd like to do all three (a), (b), and (c)"*):** `util/Json.cpp` is compiled `-O2` in
every build, Debug included, except MSVC (`/RTC1` cannot combine with `/O2`) (`ScriviCore/CMakeLists.txt`). Every other
file stays Debug. ~1.35 s → ~0.25 s per open. ctest 791/791. (b) records that store changes and (c) compaction are
[SP-200] (design [`Scrivi_History_Load_Design_v0_1.md`]).
**Parts 3 and 4 — found on the generated 25,000-event history** (`scrivi_make_test_history`: 60,151 log lines, 49 MB,
906 branches; its open was 16.7 s before parts 1–2 and still 11.4 s after):
3. **Every `setPrimary` record rebuilt the head cache** at open (`applyPrimaryOverride`), a whole-history walk each — 906
   of them. The heads follow root→current through parent links, which a fork's primary child never changes: the rebuild
   is removed (`HistoryService.cpp:1261`). 11.4 s → 0.72 s.
4. **At capacity, EVERY new event re-walked the whole history twice** (`evictToCapacity`): a head-cache rebuild (removed —
   promotion folds into the floor exactly what the walk adds back) and the walk to find the root's child toward current
   (skipped when the root has one child). ~16 ms → ~0.3 ms per recorded event, and the Mac records on the main thread.
**Tests:** ctest 792/792 — new `[I-0332]` (`HistoryServiceTests.cpp`): 40 events at capacity 5 across two scenes and a
branch that falls off; every head, every undo and, at the root, the floor equal the texts typed. A mutation in a copy (the
fold into the floor dropped) fails it (2 assertions); its first version did NOT catch that mutation (undo reads backwards
from the heads, so only the floor check sees it) and was extended. Mac interop 328/328, Linux smokes 38/38.
**Part 5 — found in [SP-200]:** the load's consistency check copied every scene's text at each fork of the tree — the
whole manuscript per branch. It and SP-200's expansion now share one walk that restores instead of copying
(`HistoryService::walkWithTexts`); 626 → 331 ms on the generated history.
**Fixture:** `/Volumes/SCRIVI-OTHE/Scrivi-Dumas-Fixture/dumas-prose-timelines-history.scrivi` (130 MB). ⚠️ It keeps dumas's
`projectID`, as the other dumas copies do, so it shares the Mac's saved Timeline view and Spotlight domain with them.

---

## I-0335

**Title:** Close Project loops forever behind "Please wait — saving updates…"
**Status:** ✅ **Resolved - Verified 2026-11-02** — [T-0648] passed on the VM (checklist: Pass); archived on the user's authority (user, 2026-11-02: *"on my authority, closed all the objects that have now tested completed"*). ⚠️ PRIOR: 🟠 Resolved - Not Verified (2026-11-02) — ⏳ the user's VM pass: [T-0648] step 5 (Close Project ends)
**Platform:** `[Linux]` only — ✅ checked: Apple's close saves, stamps once and waits once (`Scrivi/App/ProjectSession.swift:496`)
**Severity:** High. The window can never close; only killing the app ends it. The scene text on disk was not harmed.
**Date Identified:** 2026-11-02, the first EP-057 pass on the Linux VM ([T-0648]), build 1.0.314 (`225470d`). The user
opened dumas-prose-timelines from `/mnt/scrivi-net` (the Mac's disk), then **File ▸ Close Project**. The dialog appeared and
disappeared continuously for over 20 minutes.
**Observed (VM, read-only, then SIGTERM — it stopped cleanly):** two worker threads alternate `scrivi_wait_for_writes`
(~135 ms each, from 12:14:34 on); strace over 4 s: 12 writes each of `chapter-5/001-opening-scene.md` and its `.meta.json`,
13 of `workspace-state.json`. The `.md` bytes were identical every time; only the meta changed. Evidence:
`scrivi-vm:~/evidence-2026-11-02/` (`scrivi.log`, `strace.txt`).
**Root cause (from reading the code, not yet proven by a test):** `ScriviWindow::showLanding()`
(`platforms/linux/src/ScriviWindow.cpp:636`) waits for background work and then runs `flushEditor(); showLanding();`.
`flushEditor()` always ends with `EditorShell::stampWritingSurface()` (`EditorShell.cpp:1865`), which **enqueues a save
unconditionally** — even with nothing dirty — so background work is in progress again and `showLanding()` waits again,
forever. The Close Project action (`ScriviWindow.cpp:169`) calls `flushEditor()` first, which starts the loop.
**Resolution:** after the wait, `showLanding()` saves only what is dirty (`saveDirtyScenes()`, which converges) and does not
stamp again — the stamp queued before the wait has landed (`platforms/linux/src/ScriviWindow.cpp:634–644`). The window's X and
Quit were never affected: `closeEvent` flushes AFTER its wait, so nothing re-arms it.
**Files Affected:** `platforms/linux/src/ScriviWindow.cpp`; test `platforms/linux/tests/background_wait_smoke.cpp` §3b (Close
Project through the menu's own `QAction`, nothing in flight: the window must close and the dialog go).
**Verification (VM, a scratch copy `~/Dev/Scrivi-wip`, the clean checkout untouched):** §3b FAILED without the fix (and §4 saw
the looping dialog: 2 dialogs); with it `background-wait-ok` 3/3; ctest `-j6` 804/804; smokes 38/38.
**Also seen:** a leftover `chapter-5/001-opening-scene.md.tmp` (identical to the `.md`) from the interrupted write.

---

## I-0336

**Title:** Close Project releases the project before it asks — Cancel leaves a window with no project
**Status:** ✅ **Resolved - Verified 2026-11-02** — [T-0648] passed on the VM (checklist: Pass); archived on the user's authority (user, 2026-11-02: *"on my authority, closed all the objects that have now tested completed"*). ⚠️ PRIOR: 🟠 Resolved - Not Verified (2026-11-02) — ⏳ the user's VM pass: [T-0648] steps 5–6 (Cancel keeps the project; the
held save lands when the drive returns)
**Platform:** `[Linux]` only — ✅ checked: the Mac's Close Project asks `windowShouldClose` first, then closes
(`Scrivi/App/ProjectWindowManager.swift:246`, from [I-0306] — the same defect, fixed there)
**Severity:** High. After Cancel the writer keeps typing into a window whose project is closed: the held edit never lands
and undo history is no longer recorded; quitting then loses the edit.
**Date Identified:** 2026-11-02, build 1.0.315 (`7254231`), [T-0648] on the VM: SCRIVI-OTHE ejected, a sentence typed,
**File ▸ Close Project** → *"Some changes can't be saved"* → **Cancel** → drive back, share remounted: the warning never cleared.
The log: `scrivi_history_close` at the moment of Close Project (13:00:30), BEFORE the question; afterwards the core's writer
thread sat idle (no retries — its queue had gone with the project).
**Root cause:** `ScriviWindow::showLanding()` (the Close Project path for a project window) called
`editor_->releaseProject()` and only then `close()`, whose `closeEvent` asks `confirmDiscardingHeld`. The window's X asked
first, so [I-0322]'s §1b test (which used the X) never saw it.
**Resolution:** `showLanding()` calls `close()` first and returns if it was refused; `closeEvent` releases the project when
the close goes ahead (`platforms/linux/src/ScriviWindow.cpp:651–660`).
**Files Affected:** `platforms/linux/src/ScriviWindow.cpp`; test `platforms/linux/tests/background_wait_smoke.cpp` §1b (Close
Project through its own action, Cancel: the project is still open and its history alive; the held save then lands).
**Verification (VM scratch copy):** the new §1b check FAILED without the fix (*"its project is still open"*); with it
`background-wait-ok` 3/3, ctest `-j6` 804/804, smokes 38/38.
**Also seen (not this defect):** after the drive came back, the VM's `/mnt/scrivi-other` listed the **ScriviLinux** share's
contents (`appsupport projects worlds`) while the kernel named `ScriviOther`; a fresh mount fixed it. Recorded in
`Scrivi_Linux_Rig_Setup_v0_1.md` §9.3.

---

## I-0337

**Title:** The Inspector shows no "Loading…" on the Writing or Properties tab, and its cards wait for the project's metrics
**Status:** ✅ **Resolved - Verified 2026-11-02** — [T-0650] passed on the VM (checklist: Pass); archived on the user's authority (user, 2026-11-02: *"on my authority, closed all the objects that have now tested completed"*). ⚠️ PRIOR: 🟠 Resolved - Not Verified (2026-11-02) — ⏳ the user's VM pass: [T-0650] steps 1–2
**Platform:** `[Linux]` only — ✅ checked: the Mac draws "Loading…" on every card (`sceneReadGate`,
`Scrivi/Views/Inspector/InspectorCard.swift:280`) and reads the metrics in their own task (`ScenePropertiesView.swift:203`)
**Severity:** Medium. The writer sees dimmed cards for 3–5 s with nothing saying why.
**Date Identified:** 2026-11-02, build 1.0.316 (`d43603c`), [T-0650] on the VM, dumas-prose-timelines-history on
`/mnt/scrivi-other`: *"the cards take about 4-5 seconds to load. No 'Loading..' message appears anywhere."*
The log, every scene: `scrivi_load_scene_inspector` ~1,800 ms (48 file ops), then `scrivi_get_project_metrics` ~1,400 ms
(10 file ops) on the same worker, before the cards are applied.
**Root cause:** (1) the 400 ms "Loading…" and the 5 s "Still reading" lines went to `status_`, which sits at the bottom of the
Worldbuilding tab only; the Writing tab (the cards) and Properties never showed them. (2) T-0647 put
`getProjectMetrics` into the scene read's worker, so the cards waited for a whole-project count.
**Resolution:** a cue line above the tabs (`readCue_`, `platforms/linux/src/SceneInspector.cpp:105`), visible on every tab, carries
both lines (`:412`, `:443`) and clears when the read lands or times out. The metrics are read on their own after the cards are
applied, one at a time (`readMetrics`, `:510`); the scene's Links still fill with the cards.
**Files Affected:** `platforms/linux/src/SceneInspector.cpp`, `SceneInspector.hpp`; test
`platforms/linux/tests/structural_baseline_smoke.cpp` (SP-197 block: the metrics are read, at most once per scene read; the cue
is gone once the read lands).
**Verification (VM scratch copy):** smokes 38/38. ⚠️ No test shows the cards filling BEFORE the metrics land (the offscreen
reads are too fast to tell apart); the VM pass is the check: cards ~1.8 s, the Properties tab's Project figures after.
**Not changed:** the 1.8 s read itself (48 file operations over the share for one scene's Inspector).

---

## I-0338

**Title:** The Linux timeline chains every scene into one aggregate, so Zoom to Current Scene cannot separate the dots
**Status:** ✅ **Resolved - Verified 2026-11-02** — [T-0652] passed on the VM (checklist: Pass); archived on the user's authority (user, 2026-11-02: *"on my authority, closed all the objects that have now tested completed"*). ⚠️ PRIOR: 🟠 Resolved - Not Verified (2026-11-02) — ⏳ the user's VM pass: [T-0652] steps 1–2
**Platform:** `[Linux]` — Apple fixed this in [I-0208] (verified 2026-09-14); Linux never got it
**Component:** `TimelinePanel` (clustering, zoom to scene)
**Severity:** High. At first load the whole story is one dot; [T-0640]'s ◎ zooms in but the dots never come apart.
**Date Identified:** 2026-11-02, build 1.0.316 (`d43603c`), [T-0652] on the VM, dumas-prose-timelines-history (1,175 scenes)

**Description (user):** *"After clicking the target button the timeline zoomed, but the dots failed to separate. At first load
there was one aggregate dot. It should have been large enough to display the number 1175, but it only displayed the number 17.
hovering over the dot produced a circle of aggregate dots, clipped to the timeline boundaries … way more than 17 dots. Possibly
all 1175. We were definitely zoomed in, the ticks showed Week 1, Week 3, Week 5, Week 7 etc."*

**Root cause (read in the code):** `TimelinePanel::computeClusters` (`platforms/linux/src/TimelinePanel.cpp:288`)
grows a run while each member is within 18 px of the PREVIOUS one — the transitive chain [I-0208] defect (1) names. It also
clusters members that are off the visible span, so a chain runs on past both edges. Dense enough, every scene joins one run at
any zoom. `zoomToActiveScene` (`:696`) zooms until the nearest neighbour is 1/12 of the window (capped at 500×), which separates
that neighbour from the scene but not the chain.

**Resolution:** Apple's [I-0208] rule, ported (`Scrivi_Timeline_Clustering_Design_v0_1.md` R1–R3):
`computeClusters` (`platforms/linux/src/TimelinePanel.cpp:288`) cuts the whole story into equal story-time slices, as many as
aggregates fit across the zoomed width (pitch = the aggregate's HIT size + 4 px, per [I-0210]); each slice with ≥2 members is
one aggregate at the slice's centre. The partition never reads the pan. The member count is drawn unclipped, as Apple's `Text`
("1175" read "17").
⚠️ **One difference from Apple, on purpose:** Apple drops aggregates whose centre is off screen; Linux keeps them (Qt clips
the paint), because the hover fan-out holds an aggregate by its INDEX, and a visibility filter would renumber them on every
pan.
**Files Affected:** `platforms/linux/src/TimelinePanel.cpp`, `TimelinePanel.hpp`; test
`platforms/linux/tests/timeline_cluster_smoke.cpp` (1,175 scenes an hour apart: many aggregates at zoom 1, the largest never
grows with zoom, zoom to scene leaves the active scene single).
**Verification (VM scratch copy):** the new checks FAILED with every member in one slice; with the fix, smokes 38/38, ctest
`-j6` 804/804.

**Related:** [I-0286] — the clipped fan-out ring the user also saw (backlog); not a duplicate, and it is still needed after the
bucketing (a co-located cluster can be large at any zoom).

---

## I-0339

**Title:** Undo of a Replace All freezes the window for minutes on a large project
**Status:** ✅ **Resolved - Verified 2026-11-02** — [T-0655] passed on the VM (checklist: Pass); archived on the user's authority (user, 2026-11-02: *"on my authority, closed all the objects that have now tested completed"*). ⚠️ PRIOR: 🟠 Resolved - Not Verified (2026-11-02) — ⏳ the user's VM pass: [T-0655] step 2 (and [T-0653] step 4)
**Platform:** `[Linux]`
**Severity:** High. The writer sees a frozen window and may force-quit with saves in flight.
**Date Identified:** 2026-11-02, build 1.0.317 (`daebc71`), on the VM, dumas-prose-timelines-history on `/mnt/scrivi-other`:
Replace All `the` → `Thee` (74,172 matches in 1,175 scenes) landed in ~4 s; **Edit ▸ Undo** then held the window from
18:45:18 to 18:47:45 (`slow: undo took 146227 ms`). The log: 1,233 `save dirty scenes (main thread)` of ~90 ms each, every one
under `apply history step > caret moved | scrolled > active scene changed`. Log: `scrivi-vm:~/evidence-2026-11-02/scrivi-replace.log`.
**Root cause:** `EditorShell::applyHistoryStep` restored each scene by replacing its WHOLE body; each edit moved the caret or the
scroll, so the hooks promoted a new active scene and flushed. A scene stays dirty until its write LANDS, so each flush re-queued
every scene restored so far — quadratic. Then it saved the scenes one call each.
**Resolution:**
- the caret and scroll hooks stay out while the step's scenes are edited (`applyingHistory_`,
  `platforms/linux/src/EditorShell.cpp:3179`; ⚠️ not `loading_`, which also skips the offset map);
- only the changed span of each scene is rewritten (`replaceSceneSpan`, as Replace All writes it);
- the saves go in ONE queued call (`:3201`);
- `saveDirtyScenes` (`:1932`) no longer queues a scene already queued with the same text, and queues the rest in one call.
  A held write stays queued in the core and is not repeated; a refused one is queued again (I-0304 unchanged).
**Files Affected:** `platforms/linux/src/EditorShell.cpp`, `EditorShell.hpp` (the unused `replaceSceneBody` removed); test
`platforms/linux/tests/replace_smoke.cpp` §4 (the undo's saves queued once, in one call; a flush right after queues nothing).
**Verification (VM scratch copy):** the new check FAILED with the old per-scene save loop (2 calls); with the fix smokes
38/38, ctest `-j6` 804/804. ⚠️ No test runs it at 1,175 scenes; the VM pass is the measurement.
**Not this defect:** the user also saw scene headings run into the following paragraph. ✅ Checked: 90 scene files of the
iCloud copy of this fixture (1 November, before any Replace All) already have the heading merged (first line > 120 characters);
today's copy has 92. Of the difference, 3 are the same scenes renamed by today's splits, 1 was already merged on 1 November
(just under the length test then), and 1 is a new scene with no heading at all. No scene lost its heading's newline today —
the fixture's own data, not Replace All.

---

## I-0340

**Title:** The Linux Timeline's zoom stops at 500×, too short to separate dumas's scenes
**Status:** ✅ **Resolved - Verified 2026-11-02** — [T-0652] passed on the VM (checklist: Pass); archived on the user's authority (user, 2026-11-02: *"on my authority, closed all the objects that have now tested completed"*). ⚠️ PRIOR: 🟠 Resolved - Not Verified (2026-11-02) — ⏳ the user's VM pass: [T-0652] step 2
**Platform:** `[Linux]` — Apple: data-driven since I-0045 (`TimelineStripView.maxZoom`)
**Severity:** High. [T-0640]'s ◎ cannot reach a single scene on the project it was built for.
**Date Identified:** 2026-11-02, build 1.0.319 (`9f2087f`), [T-0652] on the VM, dumas-prose-timelines-history: *"There appears
to be some upper limit to how far the Timeline can zoom. When I click the target button it shows aggregate dots that have about
78 or 79 dots in them."*
**Root cause:** zoom was clamped to [1, 500] in `zoomAbout`, `zoomToActiveScene` and `setViewState`. Visible imported timelines
frame the strip (SP-173, Apple's I-0211), so it spans centuries while dumas's scenes are an hour apart; at 500× the strip still
shows months, and [I-0338]'s slices hold ~78 scenes each.
**Resolution:** Apple's rule: `maxZoom_` = the strip's span ÷ the smallest gap between two project-row dots, floored at 1 hour,
up to 100,000× — set in `recomputeWindow` (`platforms/linux/src/TimelinePanel.cpp:292`) and used by all three.
⚠️ Scenes at the SAME story time stay one aggregate at any zoom (as on Apple); their window lists them ([I-0286]).
**Files Affected:** `platforms/linux/src/TimelinePanel.cpp`, `TimelinePanel.hpp`; test
`platforms/linux/tests/timeline_cluster_smoke.cpp` (1,175 hour-apart scenes framed by an event 500 years earlier: zoom to scene
leaves the active scene single).
**Verification (VM scratch copy):** the new check FAILED with the cap at 500; with the fix smokes 38/38, ctest `-j6` 804/804.

---

