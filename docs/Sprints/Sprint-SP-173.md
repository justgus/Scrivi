---
sprint: SP-173
epic: EP-048
status: Active
activated: 2026-10-09
platform: Cross
created: 2026-10-09
---

# SP-173 — `[Cross]` [EP-048] Project load: no frozen UI, built once, measured, synchronised ([I-0285])

**Status:** 🟡 **ACTIVE 2026-10-09** (user: *"close SP-166 and activate SP-173"*) — created 2026-10-09 (user: *"Schedule I-0285 into a new Sprint"*).
**Epic:** [EP-048] `[Linux]` Manuscript Renderer Parity → [`../Epics/Epic-active.md`](../Epics/Epic-active.md). I-0285 was found in
[SP-166] and is not caused by this Epic; it is scheduled here so this Epic's work does not make it worse (I-0285 AC2).
**Carries:** [I-0285] → [`../Issues/Issue-active.md`](../Issues/Issue-active.md#i-0285).
**Size:** M. ⚠️ Needs the rig: every figure is taken on dumas over the share, where the cost lives (Docker's page cache hides it).

---

## Goal

✅ **Opening dumas on Linux never freezes the UI, the timeline is built once, every second of the load is accounted for and on the
progress bar, and the load is as short as the measurements allow.**

## What is already known (2026-10-09)

- **Measured** (build 62, `SCRIVI_LOAD_LOG`, dumas over the share): core open 55.6 s · 1,186 scene bodies 57.4 s (worker, on the
  progress bar) · navigator rebuild 28.1 s · timeline reload 27.4 s (both main thread, UI frozen) · presenter's deferred first
  highlight 3.4 s · total 172 s.
- **The 28 s "navigator" is the timeline:** `rebuildNavigator()` ends in `reloadTimeline()`, whose `loading_` guard never fires
  because `applyLoadedProject` clears `loading_` first. The timeline is built twice.
- **Apple already has the shape Linux lacks:** Apple fetches every scene's story time in ONE call, `scrivi_list_story_times`
  (EP-039 AC4 / [I-0213]: 1,174 per-scene calls had cost ~950 ms on a chapter create). Linux still calls `getSceneStoryTime`
  per scene, up to twice per scene per build, and has no binding for the batched call (`Scrivi_ABI_Binding_Gap_Audit_v0_1.md`).

## I-0285 ACs this Sprint meets

| AC | Criterion (short) |
| -- | ----------------- |
| **AC1** | The timeline is not rebuilt during the load |
| **AC2** | EP-048 must not make it worse: load phases recorded before and after; the presenter's ~3.4 s is the baseline |
| **AC3** | Every phase measured, then reduced or ruled irreducible (open, bodies, story time, navigator, highlight, layout) |
| **AC4** | The timeline's data is fetched in the load's worker; the main thread only builds widgets |
| **AC5** | Whatever moves into the worker is counted on the determinate progress bar |
| — | Apple's shape checked first |

## Plan

1. **Apple's shape at open.** Read how Apple opens a project end to end: which core calls, on which thread, what is loaded eagerly
   and what lazily (scene bodies, story times, historical events, story structure). Record it here; any difference Linux keeps is
   ruled, not assumed.
2. **Baseline.** The build-62 log is the "before". Add the missing phase boundaries: the core open's own sub-phases, and the
   first paint.
3. **AC1 — built once.** Stop the mid-load `reloadTimeline()`. One-line class of fix; the load log must show one timeline phase.
4. **Bind `scrivi_list_story_times` on Linux** (Apple's shape) and replace every per-scene `getSceneStoryTime` read in
   `reloadTimeline()` with one map lookup. Empty list = no explicit story times, never failure (the ABI's documented trap).
5. **AC4 + AC5 — off the main thread, on the bar.** Fetch the timeline's data (`getTimeline`, the story-time list, historical
   events, story structure) in the existing `AsyncCall` worker beside the scene bodies; hand it to the main thread in the payload;
   count it on the progress bar.
6. **AC3 — the two big phases.** Measure what the core open (56 s) and the per-scene body reads (57 s, ~48 ms each) spend over
   the share (file-system call counts, as in SP-144's probe). Propose reductions with figures; ⛔ any new or changed C ABI
   endpoint is `[Cross]` and needs a ruling before it is built.
7. **AC2 — the presenter's first highlight** (3.4 s, main thread): measure it on the rig and, if it can be split so it does not
   hold the UI, propose how.
8. **Smokes + Docker ctest green;** the load log before/after recorded here.
9. **Live pass on the rig (user):** open dumas; the UI stays responsive throughout; the progress bar covers the whole wait; the
   timeline draws correctly; the load log shows the new figures.

## ✅ Plan 1 — Apple's shape at open (read 2026-10-10)

| Step | Apple | Linux today |
| ---- | ----- | ----------- |
| Core open | `engine.openProject`, off the main thread (`ProjectSession.loadAsync` → `Task.detached`) | `bridge->openProject`, off the main thread (`AsyncCall` worker) — ✅ same |
| Scene bodies | `ViewportSceneLoader.loadSegmentsOffMain`: ONE `openSceneForBulkLoad` per scene, same worker, progress per scene | ONE `openSceneForBulkLoad` per scene, same worker, progress per scene — ✅ same |
| Timeline | `finishLoad` → `TimelineViewModel.load`, **main thread, ONCE**, about five calls: `getTimeline`, **`listStoryTimes` (one call for every scene)**, `getStoryStructure`, `listHistoricalEvents`, imported timelines | **main thread, TWICE**, `getSceneStoryTime` **per scene** (up to twice per scene per build) — ⛔ differs |
| Other main-thread work | `HistoryCapture.open` + `validateScenes` (undo history) | none (Linux has no undo history) |

- ✅ **The 113 s in the worker has Apple's shape.** Its cost is the core open and one ABI call per scene over the share, so
  reducing it is core work, `[Cross]` (→ Q1), not a Linux shape fix.
- ⛔ **The timeline is where Linux left Apple's shape**, in two ways: it is built twice, and it reads story time per scene instead
  of `listStoryTimes` (adopted on Apple in [I-0213]). Plans 3 and 4 restore Apple's shape.
- ⚠️ **AC4 goes beyond Apple's shape:** Apple builds the timeline on the main thread, because it is about five calls. Moving it to
  the worker on Linux alone would be a shape divergence (`feedback_linux_adopts_apple_shape`) → **Q3**.

## ✅ Scope widened 2026-10-10 (user)

*"Put the Apple half in SP-173, design a cross platform sync so that timeline loads and other core operations such as manuscript
edits occur atomically."* → the Sprint is `[Cross]`. Design: [`../Scrivi_Core_Concurrency_Design_v0_1.md`](../Scrivi_Core_Concurrency_Design_v0_1.md)
(⚠️ DRAFT, Q1–Q4 for ruling there). It adds, once ruled:

10. **Core (D1–D3):** a per-project lock around every project endpoint; a project revision in every envelope; one endpoint for
    the timeline's reads (`scrivi_load_timeline`). Boundary tests through `scrivi_*`; a ThreadSanitizer stress test.
11. **Linux bridge (D5):** the failure flag travels with each call's result (prerequisite to any new background call).
12. **Both platforms (D4):** the timeline's data is read in the background with the open's other reads, on the progress bar,
    and applied only if the revision is still current; the same pattern for a timeline reload after an edit.
13. **Apple live pass** (user, Mac): open dumas; the timeline draws; an edit during a timeline reload is never overdrawn.

## Questions for ruling (before activation or at the step that raises them)

- ✅ **Q3 RULED 2026-10-10 (user): (a)** — *"I will agree to (a) but I still think it should be loaded off the main thread in both
  places."* Linux adopts Apple's shape first (once, `listStoryTimes`, main thread) and measures what remains. ➡️ **Direction
  recorded:** the timeline's data belongs off the main thread on BOTH platforms. Apple's reason for the main thread was looked
  for and not found: `TimelineViewModel` is `@MainActor` (UI state), its `load` fetches as a side effect, and the engine is
  already called off-main at open (`Task.detached` in `ProjectSession.loadAsync`). The fetch can move to the worker with only the
  assignment left on the main thread. ⏳ Not yet verified: whether ScriviCore is safe for concurrent calls (it matters only for a
  background reload after the load).
- ~~**Q3** (from Plan 1): AC4 asks for the timeline's data off the main thread; Apple does it on the main thread in ~5 calls.
  (a) Linux adopts Apple's shape exactly (once, batched, main thread); measure what remains on the rig. If it is small, AC4 is
  ruled met by the measurement; if not, move it to the worker on BOTH platforms. (b) Move it to the worker on Linux now, and
  record the divergence or make the same change on Apple in this Sprint.~~

- **Q1** (Plan 6): if the body reads need a batched read endpoint, is that in this Sprint (`[Cross]`) or a follow-on?
- **Q2**: the measurement log (`SCRIVI_LOAD_LOG`) — keep it as a permanent, env-gated diagnostic, or remove it at close?

⛔ **NOT in this Sprint:** EP-048's S3–S5 scope · the Linux double open (SP-144 AC5, deferred) unless Plan 1 shows it on this path.

## Progress log

- **2026-10-09** — created in 🔵 Planning; I-0285 assigned.
- **2026-10-09** — activated; I-0285 → `Issue-active.md`. Plan 1 next.
- **2026-10-10** — Plan 1 done: Apple's shape recorded; Q3 raised.
- **2026-10-10** — Q3 ruled (a); off-main on both platforms recorded as the direction.
- **2026-10-10** — scope widened to `[Cross]` (user): the Apple half, plus the core concurrency design (DRAFT, awaiting ruling).
- **2026-10-10** — design Q1–Q4 ruled (exclusive lock, measured; world locks for writes only; timeline composite only; Linux failure flag in this Sprint). Recorded in the design doc §6–§7.
- **2026-10-10 — Plan 3 done (I-0285 AC1).** `applyLoadedProject` now clears `loading_` AFTER `rebuildNavigator()`, so the
  navigator's trailing `reloadTimeline()` skips and the load builds the timeline once, at its end (`EditorShell.cpp`). ✅
  `EditorShell::timelineBuildCount()` + `open_progress_smoke` asserts 1 build per load: fixed → 1, PASS; mutant (flag cleared
  first) → 2, FAIL. ⏳ Rig figure owed (the load log should show one timeline phase).
- ➡️ **Order from here:** the core first (D1 lock + lock-wait timing, D2 revision, D3 `scrivi_load_timeline`), then D5 (Linux
  failure flag), then D4 on both platforms — so Plan 4's per-scene reads are replaced by the composite endpoint directly, not by
  `listStoryTimes` and then again.
- **2026-10-10 — Core D1–D3 done (macOS ctest 679/679).**
  - **D1:** `ProjectLock.{hpp,cpp}` (`ScriviCore/src/public_api/`): one exclusive `std::mutex` per project root, key lexically
    normalised (never canonicalised: a stat per call over the share). All **107** project endpoints open with
    `SCRIVI_PROJECT_READ` (25) or `SCRIVI_PROJECT_WRITE` (82) — write unless proven read-only. No endpoint calls another (checked),
    so the lock cannot deadlock on itself. `scripts/check-abi-project-guards.sh`, registered in ctest as `AbiProjectGuards`, fails
    on an unguarded project endpoint.
  - **D2:** every project envelope (ok or error) carries `revision`; `lockWaitMs` appears when a call waited ≥ 1 ms (Q1's measure).
    Documented in `scrivi.h`'s envelope contract.
  - **D3:** `scrivi_load_timeline` — the five timeline reads under one lock; each part built by the SAME function its standalone
    endpoint now uses (no copy to drift); a failing part becomes `<part>Error`, the others still return.
  - ✅ Tests (`ProjectConcurrencyCApiTests.cpp`, through `scrivi_*`): revision bumps on writes only, also on errors; two spellings
    share a lock; a call waits while another holds its project's lock (and not another project's); writer + reader stress;
    `load_timeline` parts equal the standalone results with real story-time and event data.
  - ✅ Mutation-checked: no bump · no lock · un-normalised key · an endpoint without its guard · a wrong part builder — each caught.
    ⚠️ Honest gap: with the lock removed, only the "waits while held" test fails; the stress test passes, because each settings write
    is one atomic file write.
  - ✅ ThreadSanitizer (macOS, separate build): clean. With the lock removed, TSan flags the revision counter in
    `ProjectCallGuard`. ⚠️ My first version asserted from worker threads; TSan's 13 reports were all in Catch2, now fixed.
  - ⚠️ Found on the way: the design's §1 said the core is built per call; it is ONE singleton (corrected in the design).
    `Scrivi_ABI_Binding_Gap_Audit_v0_1.md`'s count (100) is now one short; it is updated with the bindings.
- ⏳ **Next:** Q2's world write locks (in `ObjectStore`); D5 (Linux failure flag); D4 on both platforms (bindings, the timeline read
  in the background, applied if current, on the progress bar); the Linux container ctest; then the rig and the Mac.
- **2026-10-10 — Q2 world write locks (design D6).** The lock already existed (`WorldWriteGuard`, a lock FILE, reads never lock),
  but a second writer in the SAME process was REFUSED ("worldLocked"). ⛔ Measured: two projects sharing a world, 30 creates each
  at once → 46–47 of 60 refused (three runs). ✅ An in-process recursive mutex per package, taken before the lock file → 60 of 60.
  Test `Q2: two projects in ONE process …`; mutation (mutex removed) → 46–47 refused, caught every run. ctest 680/680; TSan clean.
- **2026-10-10 — D5, D4 on both platforms, Q1 measurement: code complete; ⏳ the rig and the Mac.**
  - **D5:** Linux's failure flag is `thread_local` (`ScriviBridge.cpp`), with a per-thread `lastRevision()`. `bridge_parity_smoke`
    adds 3 checks (a worker's failure does not overwrite the main thread's answer); mutation (shared flag) → caught.
  - **D4 core:** `scrivi_get_project_revision` (no file touched). ctest 681/681 (macOS), 685/685 (Linux, non-root).
  - **D4 Linux:** `EditorShell::fetchTimelineData` (worker-safe, one `loadTimeline`) / `applyTimelineData` (UI thread, no core
    call); the load reads the timeline in its worker as the bar's last step; applied if `projectRevision` matches, else re-read.
    `open_progress_smoke`: 1 build, **0 UI-thread timeline reads**, total = scenes + 1; mutation (always re-read) → caught. The
    per-scene `getSceneStoryTime` reads are gone (they were up to 2 per scene per build). All 27 Linux smokes green.
  - **D4 Apple:** `ScriviEngine.loadTimeline` / `projectRevision` (+ `revision`, `lockWaitMs` on `Envelope`);
    `TimelineViewModel.apply(_:scenes:)` + `apply…` per part (each keeping its old failure behaviour); `ProjectSession.loadAsync`
    reads the timeline in its `Task.detached`, `finishLoad` applies it if current. `CoreConcurrencyTests` (3); mutation (always
    re-read) → caught. Full interop suite **270/270**.
  - **Progress (AC5), both:** the bar counts scenes + the timeline step; the label still counts scenes only.
  - **Q1:** both platforms sum the core's `lockWaitMs` and log it at each load's end.
  - ⚠️ **Found, not fixed — owed a ruling:** Apple's `setSceneStoryTime` binding decodes the reply as a full `SceneStoryTimeResult`,
    but the C ABI returns `{sceneID, updated}`: the call THROWS after a successful write. The app's two callers use `try?`, so it
    has been invisible.
  - ⚠️ A timeline reload after an edit stays on the main thread (one call now), as Apple already did — recorded in the design.
  - ⚠️ `Scrivi_ABI_Binding_Gap_Audit_v0_1.md` (and CLAUDE.md) say 100 endpoints: an as-of 2026-08-24 snapshot, already stale
    before this Sprint (109); now 111. Left for EP-051.
- **2026-10-10 — Live passes (user).** Rig, build 63: ✅ the window stayed responsive throughout; ✅ the bar covered the wait
  and its label never exceeded the scene count. Load log against build 62: **timeline 0.18 s in the worker** (was 27 s + 28 s on
  the UI thread), navigator 0.001 s, timeline build on the UI thread 0.1 s; total **122 s** (was 172 s); **lock waits 91 ms over 2
  waits, longest 59 ms**. Mac: *"timeline applied from the worker's read; lock waits so far: 0 ms total over 0 waits"*.
  ⏳ The background reads (open 56 s, bodies 62 s) remain → Plan 6.
- **2026-10-10 — Two untracked fixes (user: *"fix both in SP-173 as untracked fixes"*), not filed.**
  - ⛔ **Linux never adopted Apple's I-0211** (imported timelines FRAME the main timeline, user ruling 2026-09-14). Linux's window
    was scenes + historical events only, so dumas's four imported rows (years to centuries from its ~49 days of scenes) drew as
    bare lines (user: *"They never have"*; on the Mac the scenes sit inside the historical range). ✅ `TimelinePanel::recomputeWindow`
    includes the VISIBLE imported rows' events; `setImportedTimelines` recomputes. `timeline_cluster_smoke` (+2 checks: frames;
    hiding narrows); mutation → caught. ⏳ Rig check owed.
  - ⛔ **Apple bindings that THREW after every successful write** (decoding a reply shape the core never sends; invisible because
    every caller used `try?`): the four story-time writes (set / clear / assign band / unassign band → new
    `SceneStoryTimeWriteResult`), and five of the ten calls sharing `TimelineBoolResult` (`setStoryStructure`,
    `removeStoryStructure`, `deleteHistoricalEvent`, `importExternalTimeline`, `removeImportedTimeline` → it now reads whichever
    confirmation key the reply carries). `CoreConcurrencyTests.storyTimeWritesDecode`; it failed on each original type. Interop
    271/271.
- **2026-10-10 — Rig check, build 64: ✅ all three passed** (user): the four imported rows show their dots with dumas's scenes
  inside the historical range; hiding a row narrows the range; showing it widens it back.
- **2026-10-10 — Plan 6, measured (rig, dumas over `/mnt/scrivi-net`, `sp173-load-cost-probe.sh`).** Build 63: open 57.5 s,
  bodies 58.4 s (49 ms/scene), timeline 0.18 s. Attribution (strace, per phase, by what each call touched): open — object index
  rebuilt AND rewritten 270× (24 s; every relationship endpoint into the unavailable world missed, each miss rewrote the index and
  dropped the open's read cache), sidecars 22 s, a stat of every scene text 9 s; bodies — a full `ProjectIndex::build` on the first
  bulk read (every chapter + sidecar again, ~31 s), then scene text ~25 ms/scene. Local SSD for comparison: ~1 s in all. ⚠️ An
  attribution bug of mine (reads' DATA strings taken for paths) first blamed local app support; corrected by attributing by fd.
  - User (2026-10-10): *"do 1 and 2 now, 3 in this sprint, investigate 4."* Linux and Apple make the SAME core calls for loading
    (checked), so these are core fixes for both.
  - ✅ **Fix 1:** `ObjectIndex::rebuild` writes only when the scan changed the index (`ObjectIndex.cpp`). A "rebuild at most once
    per pass" half was written, measured as having no effect (the open's read cache already serves repeat rebuilds once nothing
    writes), and REMOVED. Test `ObjectIndexRebuildTests` (20 edges into an unavailable world): 0 index writes; original code → 40.
  - ✅ **Fix 2:** `scrivi_open_project` runs the open AND builds the project index through ONE read-through cache, leaving the
    index in the registry for the bulk reads. ⚠️ Not unit-testable (the registry sits behind the ABI's own file system); evidence is
    the rig run.
  - **Build 65 on the rig:** open **31.7 s** (−25.8), bodies **38.1 s** (−20.3, now 32 ms/scene, scene text only), total
    **70 s** (was 116). ctest 682/682, Linux smokes 27/27, interop 271/271.
  - ⏳ Fix 3 (parallel bulk read, `[Cross]` endpoint); item 4 (the open's stat of every scene text; 270 calls per edge left).

