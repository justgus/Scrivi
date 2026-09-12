# Active Sprints

🟡 **SP-132 is ACTIVE.** ✅ **SP-131 is VERIFIED (2026-09-12, user-approved)** — ⚠️ **awaiting close approval.**
⚠️ **Its Tasks T-0511–T-0517 are archived** → [`../Tasks/Verified/Task-verified-0511-0517.md`](../Tasks/Verified/Task-verified-0511-0517.md).
✅ **Next available Sprint ID: SP-133 · next available Task: T-0525.**

⚠️ **THE LOAD IS FIXED; THE APP IS NOT YET USABLE.** ✅ **[I-0200]** — ⚠️ **a navigator click and a
scroll each beachball on the same manuscript.** ✅ **[SP-132] is planned for it** and sits in
[`Sprint-backlog.md`](Sprint-backlog.md), ⚠️ **NOT activated.**

---

## SP-132 — `[Apple]` ⚠️ **The interaction cost** — and the async gap

**Status:** 🟡 **ACTIVE — activated 2026-09-12.**
**Epic:** [EP-039](../Epics/Epic-active.md) — `[Cross]` Project Load Performance
**Codebase:** `[Apple]` — ⚠️ **Swift/SwiftUI + AppKit only.** ✅ **No ScriviCore change expected.**
**ACs:** **AC6**, **AC9**, **AC10** · **Issues:** ⚠️ **[I-0200]**
**Date Created:** 2026-09-12
**Tasks:** **T-0518 – T-0524** (seven) · **Next available:** T-0525

### ⚠️ Why this sprint exists

✅ **[SP-131] made the project OPEN fast — `~300 s` → `1.06 s`, user-confirmed.** ⚠️ **The app is still
UNUSABLE**, and the user said so in the same breath: ✅ *"it takes much less time to load, but it is
still unuseable."*

⚠️ **THE LOAD WAS NEVER THE ONLY COST.** ⚠️ **What remains is the cost of EVERY INTERACTION AFTER it:**
a navigator click beachballs for seconds, ⚠️ **and scrolling beachballs on its own.**

### ✅ Root cause — READ IN THE CODE, ⚠️ **not yet proven at runtime**

⚠️ **THREE INDEPENDENT COSTS, ONE SYMPTOM.** ⚠️ **They must not be assumed to share a fix.**

**1. ⚠️ THE NAVIGATOR REBUILDS ~1,200 ROWS ON EVERY BODY PASS.**
`SceneNavigatorView.flatRows` (`:641`) is a COMPUTED PROPERTY allocating ~1,200 `FlatRow` structs
across two arrays every time the body runs. ⚠️ **Three things make it run constantly:**
⚠️ **(a)** it reads `loader.allScenes` AND `loader.liveTitles`, both `@Observable`;
⚠️ **(b)** `sceneRow` (`:304`) reads `loader.viewportSceneID` **PER ROW** — ✅ **so ONE CLICK
INVALIDATES ALL 1,200**;
⚠️ **(c)** the `List` carries `.onMove` (`:117`), which forces SwiftUI to MATERIALISE EVERY ROW rather
than lazily — ⚠️ **and each row also builds a `.contextMenu` and `.swipeActions` closure set.**

**2. ⚠️ THE UPDATE GUARD IS ITSELF O(N), AND IT RUNS ON EVERY UPDATE PASS.**
✅ **`ManuscriptTextView.updateNSView` correctly guards `rebuildStorage` behind a change check** —
⚠️ **but computing the guard costs `loader.segments.map(\.id)` (a 1,153-element array) PLUS
`chapterHeadingFingerprint` (`:205`), which walks ALL `allScenes` building a Set and a joined
String.** ⚠️ **Paid in full to discover that nothing changed.**

**3. ⚠️ SCROLLING DRIVES BOTH OF THE ABOVE.**
⚠️ **`ManuscriptTextView.swift:553` calls `loader.setViewportScene(sceneID)` as the visible scene
changes** — ⚠️ **an `@Observable` write, so a SwiftUI update pass follows EVERY scroll tick**, ✅ **which
re-runs cost 1 and cost 2.** ⚠️ **THAT IS THE SCROLL BEACHBALL, and it needs no click at all.**

✅ **ALSO CONFIRMED — `rebuildStorage` is O(N²) internally** (`:612-617`): ⚠️ **at each chapter boundary
it does `allScenes.first(where:)` AND a nested `allScenes.map(\.chapterID)`.** ⚠️ **This is [I-0196]'s
third defect — recorded then as "real, not yet biting."** ✅ **It bites now.**

⚠️ **AND THE APP IS STILL FULLY SYNCHRONOUS.** ✅ **RE-CONFIRMED 2026-09-12: ZERO engine calls run off
the main thread** — the only 9 `nonisolated`/`Task.detached`/`DispatchQueue.global` hits in
`Scrivi/App` + `Scrivi/Views` are ⚠️ **`nonisolated(unsafe)` statics and a test flag**, ✅ **not
concurrency.** ⚠️ **`ProjectSession` is `@MainActor`.**

### ⚠️ MEASURE FIRST — this is T-0518 and it GATES the fixes

⚠️ **THE ROOT CAUSE ABOVE WAS READ, NOT MEASURED.** ⚠️ **I have been wrong TWICE in [SP-131] by reading
instead of measuring** — ✅ **the before-only invalidation, and two tests that could not fail.**
⚠️ **`feedback_prove_code_is_reached` applies: prove the code is REACHED, and how often, BEFORE
explaining its behaviour.**
⚠️ **A probe on `flatRows` was written during triage and REMOVED UNRUN** — ⚠️ **driving synthetic input
at a window near the user's real fixture is forbidden** (`feedback_never_drive_synthetic_input_at_real_work`).
✅ **T-0518 lands the probe properly, on a COPY, with the user driving.**

### Tasks

| ID | Task | Priority | Status |
| -- | ---- | -------- | ------ |
| **T-0518** | ⚠️ **MEASURE the interaction path — GATES T-0519–T-0521.** ✅ **Count and time: `flatRows` rebuilds, `updateNSView` guard cost, `rebuildStorage` calls, per click and per scroll tick.** ⚠️ **On a COPY of the Dumas fixture, with the USER driving** — ⚠️ **never synthetic input at real work** | **High** | 🔵 Not started | ⚠️ **PROBE LANDED — awaiting a USER-DRIVEN run** |
| **T-0519** | ✅ **`[I-0200]` Make the navigator cost O(changed), not O(N).** ⚠️ **Cache `flatRows` and recompute only when `allScenes`/`liveTitles` actually change; ⚠️ take the per-row `viewportSceneID` read OUT of the row body so a selection change does not invalidate 1,200 rows.** ⚠️ **`.onMove` blocks lazy rows — ✅ RULE whether reorder-by-drag survives at this scale or moves to an explicit command** | **High** | 🔵 Not started | 🟢 **Implemented - Not Verified** |
| **T-0520** | ✅ **`[I-0200]` Make the `updateNSView` guard CHEAP.** ⚠️ **Replace the 1,153-element `map(\.id)` + full-`allScenes` fingerprint with a monotonic revision counter the loader bumps on real structural change** | **High** | 🔵 Not started | 🟢 **Implemented - Not Verified** |
| **T-0521** | ✅ **`[I-0200]` Remove `rebuildStorage`'s O(N²) chapter lookup** (`:612-617`) — ⚠️ **precompute chapterID → title and chapter ordinals ONCE per rebuild.** ✅ **[I-0196]'s third defect** | **Medium** | 🔵 Not started | 🟢 **Implemented - Not Verified** |
| **T-0522** | ✅ **AC10 — Spotlight indexing becomes CHANGE-DRIVEN.** ⚠️ **MEASURED: `scrivi_extract_searchable_text` costs `355 ms` of BLOCKING main-thread work and runs on EVERY `willResignActive`** — ⚠️ **the user's own log shows FOUR full cycles with `dirty=0` every time.** ✅ **A no-change resign must do NOTHING** | **High** | 🔵 Not started | 🟢 **Implemented - Not Verified** |
| **T-0523** | ✅ **AC6 — the load runs OFF the main thread, with a determinate progress bar.** ⚠️ **Linux solved this (T-0499/T-0500) and it earned TWO defects doing so** — ⚠️ **[I-0198] worker-thread widget touches, [I-0199] a progress bar on a hidden page.** ✅ **PLAN FOR BOTH; do not rediscover them** | **High** | 🔵 Not started |
| **T-0524** | ✅ **AC9 — notification batching**, ⚠️ **and FIX THE `rebuildStorage` PROBE.** ⚠️ **`1135.66 notifications/second` (≈ one per scene) fires during `restoreWritingSurface`.** ⚠️ **The probe reports a FALSE `projected=3519s` because its `elapsed` is wall-clock-since-OPEN** — ✅ **it has already sent one investigation chasing a non-defect** | **Medium** | 🔵 Not started | 🟡 **Probe fix DONE; batching PENDING T-0518** |

### 🟢 Progress — 2026-09-12

✅ **FOUR fixes implemented and building on macOS, iOS and visionOS.** ⚠️ **NONE measured yet** —
✅ **T-0518's probe is in and env-gated (`SCRIVI_DIAG_TIMING=1`), ⚠️ but it needs a USER-DRIVEN run**
(`feedback_never_drive_synthetic_input_at_real_work`). ✅ **A COPY of the fixture is staged at
`~/Desktop/Scrivi-SP132-Probe/`.**

| task | what changed |
| ---- | ------------ |
| **T-0519** | ✅ **`flatRows` CACHED** behind a fingerprint of its real inputs (scene identity+order, titles, chapter titles) — ⚠️ **NOT the selection.** ✅ **`FlatRow.scene` no longer carries a whole `ChapterGroup`** (it copied a chapter's entire scene list onto EVERY scene row and `sceneRow` never read it). ✅ **`isActive` moved INTO `NavigatorSceneRow`**, so a click invalidates the two rows that change, ⚠️ **not all ~1,200.** ✅ **`.onMove` KEPT** — ⚠️ **the reorder tradeoff did not have to be spent; `performMove` resolves a scene's chapter at DROP time, which happens once** |
| **T-0520** | ✅ **The `updateNSView` guard is now two `Hasher` digests** instead of a 1,153-element `map(\.id)` array plus a Set + `[String]` + `joined` walk of `allScenes`. ⚠️ **Deliberately NOT a hand-bumped revision counter** — ⚠️ **`segments` is mutated at 16 sites and one missed bump would silently stop the editor rebuilding, which is worse than a slow guard** |
| **T-0521** | ✅ **`rebuildStorage`'s chapter lookup is O(1)** — ⚠️ **was `allScenes.first(where:)` per chapter boundary WITH a nested `allScenes.map`.** ✅ **[I-0196]'s third defect, closed** |
| **T-0522** | ✅ **AC10 — Spotlight re-indexes ONLY when a scene BODY changed.** ⚠️ **Gating on "did we write anything" would never skip**, ✅ **because the current scene is always written to carry the cursor (I-0058)** — ⚠️ **so the gate is `segments.contains { $0.isDirty }`, captured BEFORE the saves clear the flags** |
| **T-0524** (probe half) | ✅ **The lying `tick` probe is FIXED.** ⚠️ **`elapsed` came from `runStart` (the whole OPEN), not the loop's own start** — ✅ **correct for `loadAll`, ⚠️ pure fiction for any later loop.** ⚠️ **That is why `rebuildStorage` printed `projected=3519s` while truly costing `0.0 s`, and it cost a real investigation** |

⚠️ **`ctest` 594/594 — the core was NOT touched, as planned.**

⚠️ **FALSIFIABLE PREDICTION for the T-0518 run**: ✅ **`navigator.flatRows.cacheHit` should dominate
with `navigator.flatRows.rebuild` at or near ZERO per click, and `editor.updateNSView.guard` in
microseconds.** ⚠️ **If `rebuild` still fires per click, the T-0519 diagnosis is WRONG** and the fix
must be rethought rather than tuned.

### ⚠️ T-0518 RUN 1 (2026-09-12) — ✅ THE PROBE FAILED, AND THAT IS THE FINDING

⚠️ **The user ran the probe on the staged copy and interacted for ~8 minutes.** ⚠️ **NOT ONE
`[SCRIVI-INTERACT]` LINE WAS PRINTED.**

⚠️ **MY PROBE WAS BROKEN, AND ITS SILENCE WAS AMBIGUOUS — which is the worse defect.**
⚠️ **A burst only flushed when a LATER hit arrived after the gap, or when `flushIdleBursts()` ran —
✅ and I had placed that call inside `updateNSView`, ⚠️ ONE OF THE VERY PATHS UNDER SUSPICION.**
⚠️ **So "no output" meant EITHER "the instrumented path is cold" (a real, important finding) OR "the
probe never reported" — ✅ opposite conclusions, indistinguishable.**
✅ **FIXED: a runloop `Timer` in `.common` mode drains bursts independently, plus a HEARTBEAT that
prints every 5 s** — ⚠️ **so silence now MEANS something.**

⚠️ **WHAT THE RUN STILL PROVED, from `[SCRIVI-DIAG]` lines that DID print:**
⚠️ **`setViewportScene` fired repeatedly during scrolling and its probe printed nothing** — ✅ **so the
probe, not the path, was the failure.** ⚠️ **DO NOT read run 1 as evidence about `flatRows`.**

✅ **A STRONGER SUSPECT APPEARED IN THE LOG, and it is NOT what T-0519 fixed:**
⚠️ **`WARNING: Application performed a reentrant operation in its NSTableView delegate.`** — ⚠️ **twice,
both during interaction.**
✅ **THE LOOP, read in the code:** ⚠️ **scrolling writes `viewportSceneID` → `SceneNavigatorView`'s
`onChange` (`:219`) writes `highlightedRowID` → that IS the `List`'s selection binding → `NSTableView`
mutates its selection WHILE SwiftUI is mid-update.** ⚠️ **Reentrancy in a table delegate is a known
stall source, and macOS says it will become an ASSERT.**
⚠️ **THIS IS A HYPOTHESIS. Run 2 must confirm it** — ✅ **`navigator.syncHighlight` and
`navigator.fingerprint` are now instrumented for exactly that.**

⚠️ **STATUS OF T-0519/T-0520 IS HONESTLY UNKNOWN.** ⚠️ **They are sound improvements on their own terms
(a 1,153-element array and a whole-`ChapterGroup`-per-row copy are real waste), ✅ but NOTHING has
demonstrated they touch the user's symptom.** ⚠️ **They must NOT be claimed as the fix until run 2.**

### ⚠️ T-0519 WAS REVERTED IN PART — ✅ **it broke the app, and the user found it**

⚠️ **The T-0519 build LAUNCHED WITH NO WINDOW AT ALL** — ⚠️ **from Xcode as well as the terminal, so
this was NOT launch mechanics.** ⚠️ **TWO DEFECTS, BOTH MINE:**

1. ⚠️ **`flatRows` CACHED INTO A `@State` REFERENCE BOX FROM INSIDE THE BODY EVALUATION.**
   ⚠️ **That is a mutation of shared state during SwiftUI's own update pass.** ✅ **REVERTED — the
   property is a PURE computed property again**, ⚠️ **and the comment now says why, so it is not
   re-attempted as an obvious win.**
2. ⚠️ **`.listRowBackground` WAS MOVED INSIDE `NavigatorSceneRow`.** ✅ **That modifier only works
   applied to the row IN THE LIST'S CONTEXT.** ⚠️ **It moved because `isActive` moved with it** —
   ✅ **so scoping the selection dependency needs the row background solved FIRST; it is not the
   one-line move I treated it as.** ✅ **REVERTED.**

✅ **WHAT SURVIVES FROM T-0519, because it is pure data shape and carries no SwiftUI risk:**
⚠️ **`FlatRow.scene` no longer carries a whole `ChapterGroup`** — ⚠️ **it copied a chapter's entire
scene list onto EVERY scene row and `sceneRow` never read it.** ✅ **`performMove` resolves the group
at DROP time, which happens once.**

⚠️ **THE HONEST POSITION: T-0519's ACTUAL TARGET — stopping one click from invalidating ~1,200 rows —
IS NOT DONE.** ⚠️ **Both mechanisms I reached for were wrong.** ✅ **T-0518's measurement must come
first, and the `NSTableView` reentrancy remains the stronger hypothesis.**

⚠️ **A LESSON WORTH THE COST: I shipped a change that removed the app's window and reported it as
"builds on macOS, iOS and visionOS."** ✅ **A SwiftUI build succeeding says NOTHING about whether the
app renders** (`feedback_live_pass_finds_what_suites_cannot`). ⚠️ **A UI change is unverified until a
window is seen.**

### ⚠️ Sequencing

⚠️ **T-0518 FIRST and it GATES T-0519–T-0521.** ✅ **T-0522 and T-0524's probe fix are independent and
may run in parallel** — ⚠️ **the Spotlight cost is already MEASURED (`355 ms`) and needs no further
proof.** ⚠️ **T-0523 (async) LAST of the performance work:** ✅ **making the work small first means the
progress bar is honest**, ⚠️ **and async over a still-expensive interaction would only make the
beachball watchable.**

### Definition of Done

- [ ] ⚠️ **A LIVE PASS BY THE USER on the 1,153-scene fixture: click the navigator, click the editor,
      and SCROLL — ✅ with NO beachball.** ⚠️ **THIS IS THE ONLY DEFINITION THAT MATTERS**
      (`feedback_live_pass_finds_what_suites_cannot`). ⚠️ **A green suite cannot see a beachball.**
- [ ] ✅ **MEASURED before AND after** for each of the three costs — ⚠️ **not "it feels faster".**
- [ ] ⚠️ **A no-change resign performs ZERO scene-body reads.**
- [ ] ⚠️ **The load runs off the main thread and the window is interactive during it.**
- [ ] ⚠️ **The `rebuildStorage` probe reports a TRUE elapsed.**
- [ ] ⚠️ **A REGRESSION GUARD for the navigator cost** — ✅ **assert rebuild count per selection change
      does not scale with scene count** (⚠️ **the AC8 pattern, applied to the UI layer**).
- [ ] ✅ **`xcodebuild` green for macOS, iOS AND visionOS.**
- [ ] ✅ **`ctest` still green** (⚠️ **macOS 594, Linux 598**) — ⚠️ **this sprint should not touch the core.**

### ⚠️ Risks

| Risk | ⚠️ Mitigation |
| ---- | ------------ |
| ⚠️ **Fixing the navigator and declaring victory** | ⚠️ **THREE independent costs; the scroll beachball needs NO click.** ✅ **The live pass must exercise click AND scroll** |
| ⚠️ **The root cause is READ, not measured** | ✅ **T-0518 gates the fixes.** ⚠️ **[SP-131] cost two rounds to exactly this** |
| ⚠️ **`.onMove` removal breaks drag-reorder** | ⚠️ **A real feature, and reorder is how a writer restructures a manuscript.** ✅ **T-0519 must RULE the tradeoff, not silently drop it** |
| ⚠️ **AC6 re-earns [I-0198]/[I-0199]** | ⚠️ **Linux earned BOTH.** ✅ **Read those Issues BEFORE writing T-0523** |
| ⚠️ **Making the UI async instead of cheap** | ⚠️ **Async does not make 1,200 row rebuilds free — it makes them a spinner.** ✅ **T-0519–T-0521 come FIRST** |
| ⚠️ **The Navigation Bar defect is assumed to be in scope** | ⚠️ **It is NOT in this sprint — see below** |

### ⚠️ NOT IN THIS SPRINT

⚠️ **The Navigation Bar does not render when the window is NOT maximised** ([I-0200], last paragraph).
⚠️ **The user expected all four symptoms to share one cause; ✅ THE EVIDENCE DOES NOT SUPPORT THAT.**
⚠️ **This is a LAYOUT defect and the other three are COST defects.** ⚠️ **It may share a cause with the
`_NSDetectedLayoutRecursion` warning — ✅ that is a HYPOTHESIS, and it needs its own diagnosis rather
than a fix bolted onto a performance sprint.**

⚠️ **Lazy-loading scene BODIES / chunking the 1.8 MB `NSTextView`.** ⚠️ **`tvLen=1823706` is ONE text
view holding the whole manuscript**, ⚠️ **and it is the likely floor under scroll cost even after
T-0519/T-0520.** ✅ **DELIBERATELY DEFERRED: it is an architectural change to the writing surface, not
a cost fix**, ⚠️ **and T-0518's measurement should say whether it is still needed once the cheap wins
land.** ⚠️ **If it is, it wants its own sprint.**

---

## SP-131 — `[ScriviCore]` ⚠️ **The in-memory indexes** — and the session that owns them

**Status:** ✅ **VERIFIED 2026-09-12 (user-approved).** ⚠️ **Awaiting close approval.**
**Epic:** [EP-039](../Epics/Epic-active.md) — `[Cross]` Project Load Performance · ✅ **the Epic's FIRST
sprint, and the one that closes the measured freeze**
**Codebase:** `[ScriviCore]` ⚠️ **+ a small `[Apple]`/`[Linux]` lifecycle change** (T-0512).
**ACs:** **AC1, AC2, AC3, AC4, AC5a–AC5e, AC8**
**Design:** [`../Scrivi_Project_Index_Design_v0_1.md`](../Scrivi_Project_Index_Design_v0_1.md) — ✅ **§5
is RULED; this sprint implements it**
**Date Created:** 2026-09-12
**Tasks:** **T-0511 – T-0517** (seven) · **Next available:** T-0518

### ✅ The target, in one line

⚠️ **`234–251 s` of a `~300 s` project open is `getSceneStoryTime` called once per scene**, ✅ **and
every one of those calls walks the entire manuscript to find one sidecar.** ⚠️ **AC2 + AC4 remove that
whole phase.** ✅ **AC1 + AC3 remove the `64–70 s` `openScene` loop's traversal half.**

### ⚠️ What reading the code changed about the plan

⚠️ **Three things are NOT as the design doc describes, and each changes a task:**

1. ⚠️ **`ScriviCore` IS CONSTRUCTED PER CALL AT THE C ABI** (`scrivi::CoreServices svc = abiServices();`
   in each endpoint). ✅ **So an index CANNOT be a `ScriviCore` member** — ⚠️ **it would be built and
   destroyed within a single call and make things strictly slower.** ✅ **It must live in a
   REGISTRY KEYED BY PROJECT ROOT**, ⚠️ **which is exactly AC5's "per open project" scope ruling.**
2. ⚠️ **THERE IS NO `scrivi_close_project`.** ✅ **`scrivi_history_close` is the ONLY close endpoint
   (1 of 100).** ⚠️ **A registry with no close LEAKS ONE INDEX PER PROJECT OPENED** — ✅ **and the
   user's own session opened two projects back to back.** ⚠️ **T-0512 exists because of this.**
3. ✅ **THE PRIOR ART IS BETTER THAN CITED.** ⚠️ **`HistoryRegistry` (`scrivi_c_api.cpp:267`) is ALREADY
   a mutex-guarded `byRoot` map with an explicit open/close pair**, ✅ **and its comment already states
   the reason for the mutex: *"ScriviEngine may call from arbitrary Swift threads."*** ⚠️ **Do NOT
   invent a second session mechanism** (`feedback_look_for_existing_pattern_first`).

✅ **ALSO ALREADY PRESENT, and it shortens T-0516:** ⚠️ **`ReadRecordingFileSystem`**
(`MultiSceneTests.cpp:381`) ✅ **is a `FileSystem` decorator that records every path read.** ✅ **AC8 and
AC5a–AC5e are all expressible with it.** ⚠️ **Do not write a second one.**

✅ **AND THE TRAVERSAL ALREADY EXISTS:** ⚠️ **`ManuscriptOrderResolver::resolve()` walks
chapters→scenes in order and returns `ResolvedScene{sceneID, chapterID, title, chapterTitle, slug,
status, metadataPath, contentPath, chapterMetadataPath}`.** ✅ **That is ALREADY nearly the whole
`SceneLocationIndex` value shape.** ⚠️ **The index is BUILT FROM this one call — not from new walking
code.**

### Tasks

| ID | Task | Priority | Status |
| -- | ---- | -------- | ------ |
| **T-0511** | ✅ **`ProjectIndexRegistry` — the session that owns the indexes.** ⚠️ **Mutex-guarded `byRoot` map, MIRRORING `HistoryRegistry`** (`scrivi_c_api.cpp:267`), ⚠️ **NOT a new mechanism.** ✅ **Holds the three indexes for one open project.** ⚠️ **AC5's scope ruling (per open project) is THIS task** | **High** | ✅ **Verified** |
| **T-0512** | ⚠️ **`scrivi_close_project` — a NEW ENDPOINT (100 → 101).** ⚠️ **WITHOUT IT THE REGISTRY LEAKS.** ✅ **Wire the app's EXISTING `ProjectSession.close()`** (`ProjectSession.swift:267,304` — ⚠️ **it already calls `historyCapture?.close()`, so the hook exists**) ⚠️ **and Linux's equivalent.** ⚠️ **Closing an unopened project MUST be a no-op, not an error** | **High** | ✅ **Verified** |
| **T-0513** | ✅ **Build all three indexes from ONE `ManuscriptOrderResolver::resolve()` pass** — **AC1, AC2, AC3.** ⚠️ **`SceneStoryTimeIndex` needs the `storyTime` block, which `ResolvedScene` does NOT carry** — ✅ **extend `ResolvedScene` rather than adding a second walk.** ⚠️ **ONE DELIVERY: partial is not acceptable** | **High** | ✅ **Verified** |
| **T-0514** | ✅ **Route the 6 `findSceneMetaPath` call sites and the resolver sites through the index** — ⚠️ **index hit ⇒ a PATH, and the caller STILL OPENS THE FILE** (**AC5a**). ⚠️ **A miss or a failed open ⇒ REAL TRAVERSAL + rebuild, NEVER a negative result** | **High** | ✅ **Verified** |
| **T-0515** | ✅ **`scrivi_list_story_times` — the SPARSE bulk call** (**AC4**). ⚠️ **A record ONLY where `offsetSource != "default"` OR `durationSource != "default"` OR `bandID` is non-empty.** ⚠️ **EMPTY IS THE COMMON CASE AND IS NOT AN ERROR** — ⚠️ **the caller MUST use the failure signal** (`project_envelope_empty_vs_failed`) | **High** | ✅ **Verified** |
| **T-0516** | ✅ **The invalidation tests — AC5a–AC5e — using the EXISTING `ReadRecordingFileSystem`.** ⚠️ **Each must be VERIFIED FAILING against an unfixed core before it counts** (`feedback_boundary_tests_not_facade`) | **High** | ✅ **Verified** |
| **T-0517** | ✅ **The complexity regression test** (**AC8**). ⚠️ **PIN THE COMPLEXITY, NOT A DURATION** — ✅ **extend the I-0196 pattern: opening one scene must not cost work proportional to how many OTHER scenes exist**, ⚠️ **and the same must now hold for a STORY-TIME query** | **High** | ✅ **Verified** |

### ✅ MEASURED RESULT — the reason this sprint existed

⚠️ **Measured at the C ABI on a COPY of the real 1,153-scene Dumas fixture** (`dumas-prose.scrivi`,
1,203 sidecars), ✅ **before and after, same harness, same machine** — ⚠️ **the "before" column is the
shipped behaviour, produced by disabling the locator, NOT an estimate.**

| phase | ⚠️ before | ✅ after | change |
| ----- | -------- | ------- | ------ |
| `get_scene_story_time` × 1154 | ⚠️ **251,800 ms** | ✅ **328 ms** | ✅ **767×** |
| `open_scene` × 1154 | ⚠️ **70,552 ms** | ✅ **356 ms** | ✅ **198×** |
| ↳ first-100 average | `12.81 ms` | `0.33 ms` | |
| ↳ **last-100 average** | ⚠️ **`110.29 ms`** | ✅ **`0.30 ms`** | ⚠️ **quadratic → FLAT** |
| `list_story_times` (bulk, AC4) | `215.6 ms` | ✅ **`0.2 ms`** | |
| **the two hot phases** | ⚠️ **322.4 s** | ✅ **0.68 s** | ✅ **~474×** |

✅ **THE BASELINE INDEPENDENTLY REPRODUCES THE USER'S CONSOLE** (`234–251 s` for the timeline phase),
⚠️ **from a harness that shares no code with the app.**
✅ **AC8's real proof is the last-100 row:** ⚠️ **`110.29 ms` against a first-100 of `12.81 ms` is the
quadratic, visible within a single run** — ✅ **after, both are `~0.3 ms`, so per-scene cost no longer
depends on how many scenes precede it.**

⚠️ **`open_project` itself is UNCHANGED (~5.0 s) and was never in scope** — ✅ **it is one traversal,
which is correct.**

### ⚠️ TWO DEFECTS FOUND BY THE NEW TESTS — both mine, both in this sprint's own work

1. ⚠️ **INVALIDATING ONLY *BEFORE* THE WORK WAS WRONG, AND SHIPPED GREEN AT FIRST.**
   ⚠️ **Nearly every mutation calls `findSceneMetaPath` to locate its target, which goes THROUGH THE
   LOCATOR and REBUILDS THE INDEX FROM PRE-WRITE DISK STATE.** ⚠️ **So a before-only drop left a
   freshly-built STALE index behind the moment the write landed:** ✅ **`scrivi_set_scene_story_time`
   wrote correctly to disk and `scrivi_list_story_times` then reported `count:0`.**
   ✅ **FIX: `ProjectIndexInvalidation`, an RAII guard that drops on BOTH sides.**
2. ⚠️ **MY FIRST AC5b AND AC5e TESTS ASSERTED THROUGH `openScene` AND COULD NOT FAIL.**
   ⚠️ **AC5a's validate-on-use fallback SILENTLY REPAIRS staleness**, ✅ **so both passed against a
   deliberately broken core** (before-only invalidation; a registry keyed by a constant).
   ✅ **FIX: both now assert through `scrivi_list_story_times`, which is served STRAIGHT FROM THE
   INDEX with no per-scene disk read to accidentally correct the answer.**
   ⚠️ **This is `feedback_boundary_tests_not_facade` one layer in: the tests were at the right
   BOUNDARY and still could not see the defect, because the endpoint they chose self-heals.**

### ✅ Verification performed

| check | result |
| ----- | ------ |
| `ctest` macOS | ✅ **594/594** (⚠️ baseline 586 + 8 new) |
| `ctest` Linux (Docker, ⚠️ **NON-ROOT**) | ✅ **598/598** |
| ⚠️ Linux `ctest` as ROOT | ⚠️ **596/598** — ✅ **the 2 failures are the I-0183 unreadable-package tests, which root defeats by being able to read a `chmod 000` directory.** ⚠️ **Pre-existing and unrelated to SP-131** (`project_linux_container_tests_off`) |
| Linux Qt smokes | ✅ **23/23** (⚠️ the 24th script is a fixture GENERATOR, not a smoke) |
| `xcodebuild` macOS / iOS / visionOS | ✅ **all three BUILD SUCCEEDED** |
| Linux Qt app image | ✅ **builds clean** |
| ⚠️ **Every new test VERIFIED FAILING first** | ✅ **AC4, AC5a, AC5b, AC5d, AC5e each re-run against a deliberately regressed core and confirmed RED** |

⚠️ **NOT DONE — the `rebuildStorage` probe fix** (a DoD item). ⚠️ **It is `[Apple]` instrumentation, not
core, and belongs with [EP-039] AC6/AC9's app-side work.** ⚠️ **It still reports a false
`projected=3519s`.**

⚠️ **NOT DONE — a LIVE PASS.** ⚠️ **Everything above is measured at the ABI and in suites.** ✅ **The
app builds, ⚠️ but no one has yet opened the Dumas fixture in the real Scrivi UI**
(`feedback_live_pass_finds_what_suites_cannot`). ⚠️ **THE APP IS STILL FULLY SYNCHRONOUS ([EP-039]
AC6), so the open will still BLOCK — it should simply block for far less time.**

### ⚠️ Sequencing

⚠️ **T-0511 → T-0512 first.** ✅ **The registry and its close are the container; the indexes have
nowhere to live until they exist.** ⚠️ **T-0513 then fills it, T-0514/T-0515 consume it, T-0516/T-0517
pin it.**
⚠️ **T-0512 IS NOT OPTIONAL AND IS NOT LAST.** ⚠️ **Shipping T-0511+T-0513 without it means every
project open leaks an index for the life of the process.**

### Definition of Done

- [ ] ⚠️ **The 1,153-scene Dumas fixture opens WITHOUT the `234–251 s` timeline phase.** ✅ **MEASURED
      with `ScriviDiag`, before AND after** — ⚠️ **a "fix" must not be able to quietly make it slower.**
- [ ] ⚠️ **`openScene`'s per-scene cost NO LONGER RISES with position.** ⚠️ **The `12.4 → 53.3 ms`
      running-average climb is the live symptom** — ✅ **it must be FLAT.**
- [ ] ✅ **AC1–AC4 delivered TOGETHER; AC5a–AC5e and AC8 all have passing tests.**
- [ ] ⚠️ **Every new test VERIFIED FAILING against the unfixed core.** ⚠️ **A test written after the fix
      that has never been red proves nothing.**
- [ ] ⚠️ **`scrivi_close_project` is wired on BOTH Apple and Linux** — ⚠️ **not just added to the ABI.**
      ✅ **`project_capability_without_surface` cuts both ways.**
- [ ] ✅ **`ctest` green** (⚠️ **baseline `586/586`**) ⚠️ **AND run on LINUX, not only macOS**
      (`project_linux_container_tests_off`: ⚠️ **the Docker image builds `SCRIVI_BUILD_TESTS=OFF`**).
- [ ] ⚠️ **The `rebuildStorage` probe is FIXED** — ✅ **it reports wall-clock-since-open as `elapsed`,
      making `avg` and `projected` noise.** ⚠️ **It has already sent one investigation chasing a
      non-defect.**

### ⚠️ Risks

| Risk | ⚠️ Mitigation |
| ---- | ------------ |
| ⚠️ **THE REGISTRY LEAKS** — no `close_project` exists today | ✅ **T-0512, sequenced SECOND, not last.** ⚠️ **The user's own session opened two projects back to back** |
| ⚠️ **A second session mechanism gets invented** | ✅ **`HistoryRegistry` is the pattern and it already handles the Swift-threading question** (`feedback_look_for_existing_pattern_first`) |
| ⚠️ **An index MISS is reported as "not found"** | ⚠️ **This is [I-0183]'s exact shape — it destroyed 10 of 12 relationships.** ✅ **AC5a, and T-0516 tests it directly** |
| ⚠️ **`scrivi_list_story_times` returns empty and a real timeline reads as EMPTY** | ⚠️ **`appendToArray` OMITS the key for an empty list, so `{}` is ambiguous.** ✅ **The caller MUST use the failure signal** (`project_envelope_empty_vs_failed`) |
| ⚠️ **Tests written green** | ⚠️ **A test that has never failed proves nothing.** ✅ **Every one VERIFIED FAILING first** |
| ⚠️ **"Green suite" mistaken for "the freeze is gone"** | ⚠️ **`ctest` cannot see a 300 s app freeze.** ✅ **The DoD requires a MEASURED open of the real 1,153-scene fixture** (`feedback_live_pass_finds_what_suites_cannot`) |
| ⚠️ **Declaring victory at the core** | ⚠️ **The app is STILL SYNCHRONOUS — [EP-039] AC6.** ✅ **This sprint makes the work SMALL; it does NOT make it ASYNCHRONOUS** |

---

## ⚠️ Which EP-039 AC is NOT in this sprint

| AC | Codebase | Where |
| -- | -------- | ----- |
| **AC6** — `[Apple]` load off the main thread + progress | `[Apple]` | ✅ **[SP-132] — T-0523** |
| **AC9** — notification batching | `[Apple]` | ✅ **[SP-132] — T-0524** |
| **AC10** — Spotlight indexing is change-driven | `[Apple]` | ✅ **[SP-132] — T-0522** |
| **AC7** — the blob-index RULE | — | ✅ **Ruled in [EP-039]; ⚠️ the BUILD is [EP-035] AC5** |

## 🔵 Planned, not activated

| Sprint | Codebase | ⚠️ Scope |
| ------ | -------- | ------- |
| **SP-129** | `[Apple]` | ⚠️ **The four UNBUILT Apple surfaces** + the timeline's direct-filesystem bypass. **T-0502–T-0506** |
| **SP-130** | `[Apple]`+`[ScriviCore]` | ⚠️ **Close the ScriviCore bypasses** ([I-0197]). **T-0507–T-0510** |

⚠️ **BOTH ARE AUDIT-FINDINGS SPRINTS, NOT PERFORMANCE WORK**, ✅ **and both sequence behind SP-131 and
the `[Apple]` async sprint.** ✅ **In [`Sprint-backlog.md`](Sprint-backlog.md).**
