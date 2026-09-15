# Closed Sprint — SP-132 (`[Apple]` the interaction cost + the async gap)

**Epic:** [EP-039](../../Epics/Epic-active.md) — `[Cross]` Project Load Performance
✅ **EP-039 CLOSED 2026-09-15 (user-approved).**
**Sprint closed:** 2026-09-15 (user-approved) · **Archived from** `Sprint-active.md`

⚠️ **OUTCOME: PARTIAL, AND THE RECORD SAYS SO.**
✅ **LANDED: the `recomputeBoundaries` rewrite (`40.8 ms` → `0.202 ms`, 202x), `SCRIVI_NO_PROJECT_LOAD`,
[T-0522]'s change-driven Spotlight (⚠️ written 2026-09-14 after the first attempt was found never to
have existed), [T-0523]/[AC6] the async load, and [T-0524]'s probe fix.**
⚠️ **DID NOT LAND: [T-0519]/[T-0520] — ✅ REVERTED because the build launched with NO WINDOW, and the
user found it.** ⚠️ **Their actual target — stopping ONE click invalidating ~1,200 navigator rows —
IS NOT DONE.** ✅ **It was also never needed: [I-0204]'s `sample` proved the freeze was the TIMELINE,
not the navigator.**
⚠️ **[AC9] was satisfied by [AC6]'s async path, NOT by [T-0524]'s batching** — ✅ **recorded in the
Epic close.**

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
| **T-0523** | ✅ **AC6 — the load runs OFF the main thread, with a determinate progress bar.** ⚠️ **Linux solved this (T-0499/T-0500) and it earned TWO defects doing so** — ⚠️ **[I-0198] worker-thread widget touches, [I-0199] a progress bar on a hidden page.** ✅ **PLAN FOR BOTH; do not rediscover them.** ---- ✅ **SCOPE EXTENDED 2026-09-14 (user-directed): THIS TASK ALSO CARRIES [I-0207] HALF (b)** — ⚠️ **getting the SYNCHRONOUS WORLD SWEEP (`reconnectWorlds` → `worldWarning.reload` → `listPendingEdges`) off the main thread.** ✅ **WHY MERGED RATHER THAN ITS OWN TASK:** ✅ **(1) it is the SAME mechanism** — ⚠️ **a blocking ScriviCore C ABI call on the UI thread;** ✅ **(2) it would re-earn the SAME TWO DEFECTS** — ⚠️ **[I-0198]'s off-thread widget touch and [I-0199]'s invisible progress surface are not load-specific, they are hazards of ANY worker path in this app;** ✅ **(3) doing them separately means building the off-main-thread machinery TWICE for one subsystem.** ---- ⚠️ **THEY ARE NOT IDENTICAL AND MUST NOT BE COLLAPSED INTO ONE CODE PATH BLINDLY:** ⚠️ **the LOAD is a one-shot with a determinate item count (a progress bar is right);** ⚠️ **the WORLD SWEEP is a RECURRING background refresh triggered by activation/mount — ✅ a progress bar would be WRONG for it** (⚠️ **a bar flashing on every click back into the app is worse than the beachball**). ✅ **Share the threading machinery; decide the SURFACE separately.** ---- ⚠️ **[I-0207] halves (a) and (c) ARE ALREADY LANDED** (T-0533, T-0534) — ✅ **the cache and the coalescing.** ⚠️ **Half (b) is what remains, and [I-0207] MUST NOT be closed until this task lands.** | **High** | 🟢 **Implemented - Not Verified (2026-09-14)** — ✅ **BUILD SUCCEEDED, ✅ ZERO Swift-6 concurrency warnings** (⚠️ `SWIFT_VERSION = 6.0`, strict checking on). ⚠️ **NEEDS A LIVE PASS: a build that compiles is not evidence (T-0529's own rule).** |
| **T-0524** | ✅ **CLOSED 2026-09-14 — ⚠️ AC9 WAS ALREADY MET BY [T-0523]/AC6.** ✅ **The async open path (`loadSegmentsOffMain` + `adoptLoadedSegments`) assigns `segments` and `liveTitles` wholesale — TWO notifications per load, not two per scene** — ✅ **and the user's post-AC6 console carries NO `harmful notification post rate` warning on 1,174 scenes.** ⚠️ **A batching change WAS also made to the SYNCHRONOUS `loadAll` (it wrote both `@Observable` properties once per scene, ~2,348 notifications) — ✅ kept because `replaceScenes` still takes that path, ⚠️ but it is NOT what satisfies the AC.** ✅ **`loadScene` + `InsertPosition` removed as dead code; `liveTitles` MERGE semantics preserved deliberately.** ---- ⚠️ **ORIGINAL:** ✅ **AC9 — notification batching**, ⚠️ **and FIX THE `rebuildStorage` PROBE.** ⚠️ **`1135.66 notifications/second` (≈ one per scene) fires during `restoreWritingSurface`.** ⚠️ **The probe reports a FALSE `projected=3519s` because its `elapsed` is wall-clock-since-OPEN** — ✅ **it has already sent one investigation chasing a non-defect** | **Medium** | 🔵 Not started | 🟡 **Probe fix DONE; batching PENDING T-0518** |

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
| **T-0522** | ✅ **NOW ACTUALLY IMPLEMENTED 2026-09-14** (`ProjectSession.saveAllDirty()`): ✅ **`let hadDirtyBodies = loader.segments.contains { $0.isDirty }` captured BEFORE the saves, and `donateSpotlight` runs only if true.** ⚠️ **Captured BEFORE because saving CLEARS `isDirty` — reading it after always sees zero and would skip forever.** ⚠️ **Gating on "did we write anything" does NOT work: `saveAllDirtyBlocking` ALWAYS writes the current scene to carry the cursor (I-0058).** ✅ **A skip logs `[SCRIVI-DIAG] spotlight: SKIPPED re-index (no dirty scene bodies)` — ✅ that line is the acceptance evidence.** ✅ **The OPEN-path donate stays UNCONDITIONAL (a first open must index).** ✅ **BUILD SUCCEEDED.** ---- ⚠️ **PRIOR FALSIFICATION RECORD RETAINED:** ⚠️ **FALSIFIED 2026-09-14 BY THE USER'S CONSOLE — ✅ THE GATE WAS NOT IN THE CODE.** ⚠️ **The log shows `onAppResign` → `dirty=0` → `extract OK: 1159 records` → `donate accepted: 1159 items` — ✅ a no-change resign re-extracting EVERY record, which is exactly what AC10 forbids.** ✅ **ROOT CAUSE READ: `ProjectSession.saveAllDirty()` (`:388-395`) calls `donateSpotlight` UNCONDITIONALLY; ⚠️ there is NO dirty gate at that call site, in `onAppResign` (`AppEnvironment.swift:838`), or in `ViewportSceneLoader.saveAllDirty`.** ⛔ **`git log -S` finds NO COMMIT that ever introduced `contains { $0.isDirty }`** — ⚠️ **so the change was never written, not merely lost.** ⚠️ **THE PRIOR STATUS LINE CLAIMED "Implemented - Not Verified" FOR CODE THAT DOES NOT EXIST.** ✅ **Original task text follows.** ---- ✅ **AC10 — Spotlight re-indexes ONLY when a scene BODY changed.** ⚠️ **Gating on "did we write anything" would never skip**, ✅ **because the current scene is always written to carry the cursor (I-0058)** — ⚠️ **so the gate is `segments.contains { $0.isDirty }`, captured BEFORE the saves clear the flags** |
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

