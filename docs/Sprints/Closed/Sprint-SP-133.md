# Closed Sprint — SP-133 (`[Apple]` TextKit 2)

**Epic:** [EP-039](../../Epics/Epic-active.md) — ✅ **CLOSED 2026-09-15 (user-approved).**
**Sprint closed:** 2026-09-15 (user-approved) · **Archived from** `Sprint-active.md`
**Tasks:** T-0525–T-0534 — ✅ **ALL VERIFIED AND ARCHIVED**
→ [`Task-verified-0525-0534.md`](../../Tasks/Verified/Task-verified-0525-0534.md)
→ [`Task-verified-0528-0533.md`](../../Tasks/Verified/Task-verified-0528-0533.md)

### ✅ OUTCOME

✅ **THE MIGRATION LANDED AND HOLDS.** ⚠️ **Two `layoutManager` call sites were the ENTIRE downgrade;**
✅ **the console now reports `TextKit 2` at construction AND at every `rebuildStorage`, and [T-0527]'s
CI guard makes a silent re-downgrade a build failure.**

✅ **MEASURED AGAINST THE TEXTKIT 1 BASELINE:** ⚠️ **initial layout `272.8 ms` → `0.0 s`; `rebuildStorage`
`270.0 ms` → `0.0 s` on EVERY structural op; resize `222.5 ms` → a live edge drag with no hitch.**

⚠️ **THE SPRINT ALSO ABSORBED WORK IT WAS NOT PLANNED FOR** — ✅ **[T-0532] (the timeline stall, 73% of
main-thread samples), [T-0533]/[T-0534] ([I-0207]'s binding cache and coalescing).** ✅ **Those came from
MEASUREMENT overturning the plan: ⚠️ the sprint was created believing the remaining half of [I-0200] was
TextKit, and `sample` proved it was the TIMELINE.**

⚠️ **WHAT IT DID NOT FIX, AND SAID SO:** ⚠️ **[I-0203] and [I-0205] were RE-TESTED by [T-0531] and found
to be REAL defects rather than starvation symptoms** — ✅ **both carried by [EP-040].**
⚠️ **[I-0206] and [I-0213] are live O(DOCUMENT) costs on this same surface** — ✅ **also carried.**

⚠️ **ONE PROCESS FAILURE WORTH KEEPING:** ⚠️ **[T-0529]'s row claimed the `rebuildStorage` case was
"STILL UNEXERCISED" for a full day AFTER the runs that exercised it.** ✅ **The user challenged the
row and was right.** ⚠️ **A status line written mid-investigation goes stale the moment the next run
lands.**

---

## SP-133 — `[Apple]` ⚠️ **TextKit 2** — the manuscript surface, and the iOS path

**Status:** 🟡 **ACTIVE — activated 2026-09-13.**
**Epic:** [EP-039](../Epics/Epic-active.md) — `[Cross]` Project Load Performance
**Codebase:** `[Apple]` — ⚠️ **`ManuscriptTextView.swift` and its attachment cell.** ✅ **No ScriviCore change.**
**Design:** [`../Scrivi_Manuscript_Text_Surface_Trade_Study_v0_1.md`](../Scrivi_Manuscript_Text_Surface_Trade_Study_v0_1.md) — ✅ **RULED: Option C**
**Issues:** ⚠️ **[I-0200]** (the remaining half) · ⚠️ **[I-0203]** (⚠️ **re-test, do NOT fix**)
**Date Created:** 2026-09-13
**Tasks:** **T-0525 – T-0531** (seven) · **Next available:** T-0532

### ✅ Why — MEASURED on the REAL manuscript (1,831,770 chars, 14,985 paragraphs)

| operation | ⚠️ **TextKit 1 (today)** | ✅ **TextKit 2** |
| --------- | ------------------------ | ---------------- |
| initial layout | ⚠️ **272.8 ms** | ✅ **0.9 ms** |
| window RESIZE (width change) | ⚠️ **222.5 ms** | ✅ **viewport-bounded** |
| `rebuildStorage` (`setAttributedString`) | ⚠️ **270.0 ms** | ✅ **viewport-bounded** |
| 1-char edit | ✅ `0.3 ms` | ✅ `0.1 ms` |
| hit-test (the scroll handler's query) | ✅ `0.01 ms` | ✅ `0.005 ms` |

⚠️ **TYPING WAS NEVER THE PROBLEM.** ⚠️ **The recurring costs are the ones proportional to the WHOLE
DOCUMENT: layout, EVERY RESIZE, and EVERY `rebuildStorage`** — ✅ **and `rebuildStorage` runs on any
segment-list change, so every scene create/delete/merge/split pays `270 ms`.**

⚠️ **THE `204 ms` KEYSTROKE IN THE TRADE STUDY WAS MY ARTIFACT** (§8): ⚠️ **the benchmark generated
one 1.8 MB paragraph.** ✅ **Corrected before planning, not after.**

### ⚠️ USER OBSERVATION 2026-09-13 — ✅ **the resize case is NOT yet confirmed as user-visible**

⚠️ **The user toggled FULL SCREEN a couple of times and reported NO ISSUES.** ✅ **A full-screen
toggle IS a container-width change, so it exercises the `222.5 ms` relayout path.**
⚠️ **SO THE RESIZE COST IS MEASURED BUT NOT YET FELT.** ✅ **`222 ms` is a visible hitch, not a
freeze** — ⚠️ **and a one-off toggle is far gentler than a LIVE DRAG of the window edge, which
re-lays-out CONTINUOUSLY.** ⚠️ **T-0529 must test a live drag, not just a toggle.**

⚠️ **THE USER COULD NOT TEST TYPING OR SPLIT/MERGE AT ALL:** ✅ ***"I was unable to get a cursor long
enough to type."*** ⚠️ **That is itself the symptom this sprint exists to remove**, ⚠️ **and it means
the `270 ms` `rebuildStorage` cost on scene create/delete/merge/split is STILL UNOBSERVED IN USE.**
✅ **Honest position: the DOCUMENT-PROPORTIONAL COSTS ARE MEASURED AT THE ENGINE; ⚠️ which of them a
writer actually FEELS is not yet established, because the app has not been usable enough to find
out.**

### ⚠️ THE CONSTRAINT THAT MUST SURVIVE THIS SPRINT

⚠️ **TEXTKIT 2'S LAZINESS IS PER PARAGRAPH.** ✅ **On real prose (14,985 paragraphs) it is ~300x
better and a far-from-viewport edit costs `0.0 ms`.** ⚠️ **On ONE 1.8 MB paragraph it collapses to
`201 ms` — NO BETTER THAN TEXTKIT 1.**
⚠️ **A writer CAN legitimately produce very long paragraphs.** ⚠️ **This is a NAMED RISK, not a
discovery for later** — ✅ **T-0530 measures it deliberately.**

### Tasks

| ID | Task | Priority | Status |
| -- | ---- | -------- | ------ |
| **T-0525** | ✅ **Replace the TWO TextKit 1 call sites.** ⚠️ **`ManuscriptTextView.swift:541`** (scroll handler: `glyphIndex(for:)` + `characterIndexForGlyph`) → `textLayoutFragment(for:)`; ⚠️ **`:1591`** (`centerStorageOffset`: `glyphRange` + `boundingRect`) → fragment geometry. ⚠️ **These two lines are the ENTIRE downgrade** — ✅ **touching `.layoutManager` at all forces TextKit 1** | **High** | 🔵 Not started | 🟡 **Active** | 🟢 **Implemented - Not Verified** | ✅ **VERIFIED 2026-09-14 (USER-APPROVED)** — ✅ **the two TextKit 1 call sites replaced**; ⚠️ **console confirms `TextKit 2` at construction AND at `rebuildStorage`.**
| **T-0526** | ✅ **Port the scene divider off `NSTextAttachmentCell`** → `NSTextAttachmentViewProvider` / `attachmentBoundsForAttributes`. ⚠️ **`NSTextAttachmentCell` is TextKit 1 AND AppKit-ONLY — it has NO UIKit equivalent**, ✅ **so it blocks iOS independently of performance.** ✅ **The replacement is `macos(12.0), ios(15.0)`** | **High** | 🔵 Not started | 🟡 **Active** | 🟢 **Implemented - Not Verified** | ✅ **VERIFIED 2026-09-14 (USER-APPROVED)** — ✅ **the divider ported off `NSTextAttachmentCell`**; ⚠️ **console confirms `TextKit 2` at construction AND at `rebuildStorage`.**
| **T-0527** | ✅ **A GUARD so the downgrade cannot return** (5.7 ruling). ⚠️ **A CI grep that FAILS on `.layoutManager` / `NSLayoutManager` / `NSTextAttachmentCell` in `Scrivi/Views`**, ✅ **with an allow-list carrying reasons.** ⚠️ **MUST LAND WITH THE MIGRATION, not after** — ⚠️ **one future access silently re-downgrades the whole view** | **High** | 🔵 Not started | 🟡 **Active** | 🟢 **Implemented - Not Verified** | ✅ **VERIFIED 2026-09-14 (USER-APPROVED)** — ✅ **the CI downgrade guard**; ⚠️ **console confirms `TextKit 2` at construction AND at `rebuildStorage`.**
| **T-0528** | ✅ **VERIFY `sceneBoundaries` and the nine `recomputeBoundaries` callers still hold.** ⚠️ **They are CHARACTER-INDEXED over the whole storage; TextKit 2 is range/fragment-based.** ✅ **`NSTextContentStorage` still exposes `textStorage`, so the character model SHOULD survive unchanged** — ⚠️ **that is an assumption and it must be TESTED, not assumed** (⚠️ **[I-0131] is what two coordinate spaces cost**) | **High** | 🔵 Not started | 🟡 **Active** | ✅ **PASS — verified by differential test** ✅ **VERIFIED 2026-09-15 (USER-APPROVED).** ✅ **The character model SURVIVED the migration: `NSTextContentStorage` still exposes `textStorage`, and the differential test PROVED the nine `recomputeBoundaries` callers produce byte-identical ranges.** ⚠️ **This was the [I-0131] risk — two coordinate spaces — and it did NOT recur.** |
| **T-0529** | ✅ **ALL FIVE MEASUREMENTS DELIVERED — ⚠️ CORRECTED 2026-09-15 AFTER THE USER CHALLENGED THIS ROW.** ⚠️ **The text below was written 2026-09-14 and went STALE the same day** — ⚠️ **it says the `rebuildStorage` case is "STILL UNEXERCISED", but the runs of 2026-09-14/15 exercised it REPEATEDLY, and I did not revisit the row.** ✅ **THE USER WAS RIGHT TO CHECK.** ---- ✅ **THE EVIDENCE, ALL FIVE:** ✅ **(1) INITIAL LAYOUT — `rebuildStorage` at open reports `took 0.0 s` on 1,172–1,179 segments** (⚠️ **TextKit 1 baseline `272.8 ms`**); ✅ **(2) RESIZE — user-verified live edge drag, *"the window drags"*** (⚠️ **baseline `222.5 ms` per relayout**); ✅ **(3) `rebuildStorage` — logged on EVERY structural op at `took 0.0 s`, every tick `avg=0.0ms/item`** (⚠️ **baseline `270.0 ms`**); ✅ **(4) SCROLL — `scrollToVisible` `0.0–1.1 ms` throughout**; ✅ **(5) CREATE/DELETE/MERGE/SPLIT — the user performed a scene create, THREE chapter creates, scene deletes (`1174→1173→1172`), a chapter delete, and backspace-at-scene-start + `⌘⌫` merge; `rebuildStorage` fired on each and cost `0.0 s` every time.** ---- ⚠️ **ONE HONEST QUALIFIER: `rebuildStorage`'s OWN loop is `0.0 s`, but the STRUCTURAL OPS AROUND IT ARE NOT FREE** — ✅ **chapter create is `~305 ms` of work, tracked separately as [I-0213] and carried into [EP-040].** ⚠️ **T-0529 measured the TextKit 2 surface, which is what it asked; ⛔ it does not claim structural ops are cheap.** ---- ⚠️ **SUPERSEDED TEXT FOLLOWS.** ---- ✅ **RESIZE HALF VERIFIED 2026-09-14 (USER-APPROVED): *"the window drags"*** — ✅ **this closes the one case SP-133 flagged as MEASURED BUT NOT FELT** (⚠️ **a live edge-drag re-lays-out CONTINUOUSLY, unlike the full-screen toggle already tried**). ⚠️ **THE TASK IS NOT FULLY DONE:** ⚠️ **it asks for FIVE measurements — layout, resize, `rebuildStorage`, scroll, AND a scene create/delete/merge/split.** ⛔ **The `rebuildStorage` case (`270 ms` on TextKit 1, fired by EVERY scene create/delete/merge/split) is STILL UNEXERCISED** — ✅ **SP-133 records the user *"could not test typing or split/merge at all"* because the app was unusable then; ✅ it is usable now, so that test is newly possible.** ---- ⚠️ **ORIGINAL:** ✅ **Measure BEFORE and AFTER on the real fixture** — ⚠️ **layout, resize, `rebuildStorage`, scroll, and a scene create/delete/merge/split.** ⚠️ **A build that compiles is NOT evidence** | **High** | 🔵 Not started | 🟡 **Active** | 🟢 **MEASURED — awaiting live pass** |
| **T-0530** | ⚠️ **MEASURE THE LONG-PARAGRAPH CASE DELIBERATELY** — ✅ **a manuscript whose scenes are single unbroken paragraphs.** ⚠️ **If it collapses to TextKit 1 numbers, RECORD IT as a known limitation with a mitigation** (⚠️ **e.g. soft-wrapping very long paragraphs into layout units**), ✅ **do not leave it to be re-discovered** | **Medium** | 🔵 Not started | 🟡 **Active** | ✅ **MEASURED — risk is NARROWER than feared** ✅ **VERIFIED 2026-09-15 (USER-APPROVED).** ✅ **MEASURED ACROSS FOUR SHAPES at 1.82 MB: normal prose (4,041 paras) `13.9 ms` first layout / `0.2 ms` edit; long paras (~5,000 ch) `0.5 / 0.5 ms`; VERY long (~50,000 ch) `3.9 / 3.7 ms`.** ⚠️ **ONLY the pathological ONE-PARAGRAPH document collapses: `200.9 ms` layout, `199.3 ms` per edit — TextKit 1 numbers.** ✅ **THE RISK IS REAL BUT NARROW, and it is now RECORDED rather than left to be re-discovered — which is exactly what this task asked for.** ⚠️ **No mitigation built: a 1.8 MB single paragraph is not a shape a writer produces by accident, ✅ and inventing a soft-wrap layer for it now would be speculative.** |
| **T-0531** | ✅ **RE-TEST [I-0203]** (the Scene Navigator vanishing) ⚠️ **and the `_NSDetectedLayoutRecursion` warning.** ⚠️ **Both were hypothesised to be SYMPTOMS of main-thread starvation** — ✅ **if they survive a responsive UI they are real defects and need their own diagnosis; ⚠️ if they vanish, say so and close them** | **Medium** | 🔵 Not started | 🟡 **Active** | ✅ **ANSWERED 2026-09-14 — ⚠️ THEY SURVIVED.** ✅ **The live pass ran on a RESPONSIVE main thread ([I-0204] fixed) and BOTH persisted**, ⚠️ **so the starvation hypothesis is DISPROVEN and they are REAL defects.** ✅ **[I-0203] rewritten with the measured trigger (the world-unavailable banner); ✅ [I-0205] filed for the banner itself.** ⚠️ **This task's QUESTION is closed; the DEFECTS remain open in their own records** ✅ **VERIFIED 2026-09-15 (USER-APPROVED) — ⚠️ AS AN ANSWERED QUESTION, NOT AS A FIX.** ✅ **The task asked whether the two symptoms survive a responsive main thread; ✅ they DO, so the starvation hypothesis is DISPROVEN and both are real defects.** ⚠️ **THE DEFECTS THEMSELVES REMAIN OPEN: [I-0203] (chrome vanishes while the world-warning banner is up) and [I-0205] (the banner itself) are carried by [EP-040], where [AC5] is I-0203's acceptance test.** ⛔ **Verifying this task does NOT verify those Issues.** |

| **T-0532** | ✅ **[I-0204] — CACHE THE TIMELINE'S DERIVED BOUNDS AND KILL THE QUADRATIC CLUSTER SORT.** ✅ **MEASURED via `sample Scrivi` during a live stall (2026-09-14): 4,796 / 6,595 main-thread samples — 73% — in `TimelineStripView.buildClusters`.** ⚠️ **ZERO in the Scene Navigator; TextKit barely registers.** ✅ **THREE FIXES:** ✅ **(a) `minOffsetMs`/`maxEndMs`/`spanMs` cached on `TimelineViewModel` via `didSet` on `dots`+`historicalEvents` (`:202`, `:213`, `recomputeDerivedBounds()` `:249`);** ✅ **(b) `smallestMainRowGapMs` moved to the model and cached the same way — it BUILT AND SORTED a 1,156-element array per call, and `maxZoom` calls it (`:682`);** ✅ **(c) decorate-sort-undecorate in `buildClusters` (`:1390`) — `itemX` was called INSIDE the sort comparator, ~23,000 calls → 1,156.** ⚠️ **This SUPERSEDES the belief that the remaining half of [I-0200] was TextKit** | ⚠️ **HIGH** | ✅ **VERIFIED 2026-09-14 (USER-APPROVED).** ⚠️ **PRIOR:** 🟢 **Implemented - Not Verified — ✅ CONFIRMED BY LIVE RUN 2026-09-14.** ⚠️ **`NSClickGestureRecognizer … possible phase ~33 s` ABSENT from the whole session**; ✅ **click `35–45 s` → `~0.3 s`; user reports the app USABLE.** ⚠️ **Awaiting USER verification** |

| **T-0533** | ✅ **[I-0207] half (a) — CACHE THE PARSED WORLD BINDING.** ⚠️ **`WorldStore::loadBinding` re-read AND re-parsed the same `binding.json` ONCE PER ENDPOINT inside `RelationshipStore::listPending`** (✅ **measured: ~860 JSON-parse samples per activation; 81% of the main thread across two activations**). ✅ **FIX: an optional caller-owned `WorldStore::BindingCache`, keyed on the RESOLVED PATH (⚠️ not on `worldID` — the same ID under another projectRoot is a DIFFERENT binding), threaded `listPending` → `EndpointResolver::resolve` → `loadBinding`.** ⚠️ **WHY CALLER-OWNED, NOT A MEMBER: `WorldStore` is constructed FRESH PER CALL — `EndpointResolver::resolve` builds one PER ENDPOINT — ✅ so a member cache would die each iteration and save nothing; ⚠️ and `CoreServices` is a stateless pointer bundle, so a cache there would change its contract.** ⚠️ **Lifetime is exactly ONE read-only operation, which is what makes caching FAILURES safe and what forbids hoisting it without invalidation.** | ⚠️ **HIGH** | 🟢 **Implemented - Not Verified — ✅ 596/596 ctest (594 + 2 new); ✅ the read-count test was verified to FAIL with the cache disabled.** ⚠️ **[I-0207] halves (b) off-main-thread and (c) double-fire REMAIN OPEN** |

| **T-0534** | ✅ **[I-0207] half (c) — STOP `reconnectWorlds` RUNNING ONCE PER WINDOW.** ⚠️ **The `didBecomeActive` observer lived on `ManuscriptEditorView` (PER WINDOW) while `reconnectWorlds()` loops EVERY session — ✅ N windows × N sessions = N² session-reloads per activation.** ⚠️ **Same duplication on the `didMount`/`didUnmount` observers beside it.** ✅ **FIX: `AppEnvironment.coalescedReconnectWorlds()`; all three observers call it.** ⚠️ **COALESCING, NOT MERELY ONE SUBSCRIBER — `didBecomeActive` is legitimately delivered more than once per activation (✅ the sample shows it BOTH nested in `_NXFinishActivation` AND standalone), so a single subscriber would still double-run.** ⚠️ **A FLAG, NOT A DEBOUNCE: a timer would delay world re-acquisition, which is exactly what [I-0123] exists to prevent.** ---- ⚠️ **RECORDED: my first reading of the sample was WRONG** — ✅ **I claimed two distinct `sendEvent:` offsets proved two activations; they are the SAME offset (`+2232`), one nested in the other.** | ⚠️ **HIGH** | ✅ **VERIFIED 2026-09-14 (USER-APPROVED).** ⚠️ **THE PRIOR STATUS LINE WAS STALE AND IS CORRECTED:** ⚠️ **it said *"half (b) (off-main-thread) is still not done"*** — ✅ **half (b) HAD ALREADY LANDED with [T-0523]'s scope extension.** ✅ **VERIFIED IN THE CODE, not from the record: `WorldWarningModel.reloadAsync` runs the sweep on `Task.detached(priority: .utility)` (`WorldWarningView.swift:73`) and publishes on the main actor; ✅ its ONLY callers are `AppEnvironment.swift:518` and `EditorView.swift:113`, both `await`ing it** — ⛔ **NO synchronous `reload` caller remains.** ✅ **[I-0207] is VERIFIED AND ARCHIVED with ALL THREE halves confirmed by the user's live pass: (a) the binding cache [T-0533], (b) off-main-thread [T-0523], (c) the coalescing [T-0534].** ✅ **The focus-click beachball is GONE.** |

### 🟢 Progress — 2026-09-14 (T-0523)

✅ **AC6 LANDED — the load runs off the main thread with a determinate progress bar, and [I-0207] half (b) landed with it.**

**The shape, and why:**
- ✅ **`ViewportSceneLoader.loadSegmentsOffMain`** — ⚠️ **a `nonisolated static` func, NOT a method.** ⚠️ **The loader is `@MainActor` + `@Observable`, so worker access to `self` would be [I-0198] exactly.** ✅ **It takes only `Sendable` inputs, returns plain values, touches nothing on the loader.**
- ✅ **`ProjectSession.loadAsync`** — ✅ **`openProject` + the per-scene loop on a `Task.detached`; everything observable published AFTER the await, on the main actor.**
- ✅ **`ProjectSession.finishLoad`** — ⚠️ **EXTRACTED, not duplicated.** ⚠️ **The order is load-bearing** (`validateScenes` before `loader.historyCapture`; `inspectorVisible` before `inspectorLayout`), ✅ **so two copies would drift.**
- ✅ **`WorldWarningModel.reloadAsync`** — ✅ **[I-0207](b): the `listPendingEdges` sweep (81% of the main thread) off-main, same compute/publish discipline.**

**The two Linux defects, planned for rather than rediscovered:**
- ✅ **[I-0198] (worker-thread widget touches): the worker returns PLAIN VALUES; every publish is an explicit main-actor hop; `onProgress` is `@Sendable` and forwards only numbers.** ✅ **VERIFIED: no observable mutation inside the detached task; ✅ zero Swift-6 concurrency warnings.**
- ✅ **[I-0199] (a bar on a hidden page): the bar lives in `LandingView`, NOT the editor.** ⚠️ **On Apple the editor WINDOW DOES NOT EXIST during a load** — ✅ **every caller `await`s `loadProject` and only then calls `requestOpenWindow`** — ⚠️ **so putting it in the editor would have reproduced I-0199 in a new shape.**

**Two rulings made here:**
- ✅ **The bar is revealed only after `250 ms`** (user-ruled). ⚠️ **A threshold, NOT a debounce: counting starts at scene one regardless, so when the bar appears it is already accurate.** ✅ **A ~1 s local open shows nothing; a slow/network open (⚠️ [I-0195]'s real scenario) shows an honest bar.**
- ✅ **NO bar for the world sweep.** ⚠️ **The load is a one-shot with a determinate count; the sweep is a RECURRING refresh on every activation/mount — a bar flashing on every click back into the app would be worse than the stall.**

⚠️ **`ProjectSession.load(at:)` is now UNUSED and marked `@available(*, deprecated)`.** ✅ **Kept as the reference load ORDER and as a fallback if `loadAsync` must be reverted;** ⚠️ **DELETE IT AT SP-133 CLOSE if still unused — a second path WILL drift from `finishLoad`.**

### 🟢 Progress — 2026-09-13

✅ **ALL SEVEN TASKS IMPLEMENTED OR MEASURED.** ⚠️ **The LIVE PASS is outstanding and T-0531 is blocked
on it.**

#### ✅ T-0529 — MEASURED before/after, real manuscript (1,831,770 chars, 14,985 paragraphs)

| operation | ⚠️ **TextKit 1 (before)** | ✅ **TextKit 2 (after)** | gain |
| --------- | ------------------------- | ----------------------- | ---- |
| initial layout | ⚠️ **280.9 ms** | ✅ **0.9 ms** | ✅ **312x** |
| ⚠️ **LIVE WINDOW DRAG** (10 width steps) | ⚠️ **2,236.7 ms** (`223.7 ms`/step) | ✅ **4.2 ms** (`0.4 ms`/step) | ✅ **532x** |
| `rebuildStorage` (every scene create/delete/merge/split) | ⚠️ **272.3 ms** | ✅ **5.0 ms** | ✅ **54x** |
| scroll hit-test | `0.01 ms` | ✅ **0.006 ms** | — |

⚠️ **THE LIVE DRAG IS THE HEADLINE** — ✅ **and it is the case the user's full-screen toggle could NOT
have exposed**: ⚠️ **a toggle is ONE width change (`224 ms`, a hitch); ✅ a DRAG is a continuous
stream of them (`2.2 s` for ten).**

#### ✅ T-0530 — the long-paragraph risk is REAL but NARROW

| shape (1.82 MB) | paragraphs | first layout | 1-char edit |
| --------------- | ---------- | ------------ | ----------- |
| normal prose (~450 ch/para) | 4,041 | ✅ **13.9 ms** | ✅ **0.2 ms** |
| long paras (~5,000 ch) | 362 | ✅ **0.5 ms** | ✅ **0.5 ms** |
| very long paras (~50,000 ch) | 37 | ✅ **3.9 ms** | ✅ **3.7 ms** |
| ⚠️ **pathological: ONE paragraph** | 1 | ⚠️ **200.9 ms** | ⚠️ **199.3 ms** |

✅ **THE RISK IS NARROWER THAN THE SPRINT PLAN FEARED.** ⚠️ **Even 50,000-character paragraphs cost
`3.7 ms`.** ⚠️ **Only a manuscript that is ONE 1.8 MB paragraph — a writer who never presses Return —
degrades to TextKit 1 numbers.** ✅ **RULED: acceptable, documented, NO mitigation built.**

#### ✅ T-0528 — the character model SURVIVES

⚠️ **`sceneBoundaries` and the nine `recomputeBoundaries` callers use whole-storage CHARACTER
offsets; TextKit 2 is range/fragment-based.** ✅ **Differential test: boundaries match
`rebuildStorage`'s own truth with titles ON and OFF, and every boundary round-trips
character-offset → fragment → character-offset.** ⚠️ **This was the [I-0131] risk and it is closed.**

#### ⚠️ A THIRD DOWNGRADE SITE WAS FOUND — ✅ **by the guard, in a different file**

⚠️ **`ForkPopover.swift:117` read `textView.layoutManager`** — ⚠️ **on the SAME text view, so it
silently undid the entire migration.** ⚠️ **WORSE: it sat inside an `if let` whose branch is rarely
taken, but Swift evaluates the property REGARDLESS — ✅ so it downgraded on EVERY call.**
✅ **This is exactly why T-0527 checks `Scrivi/Views` as a whole rather than one file**, ⚠️ **and it
means the migration would have silently failed without the guard.**

#### ✅ T-0527 — the guard is VERIFIED FAILING

⚠️ **A guard that cannot fail is worthless.** ✅ **`scripts/check-textkit2.sh` was re-run against a
deliberately reintroduced `_ = textView.layoutManager` and exited `1` with the offending line;
✅ restored, it exits `0`.**

#### ✅ Also verified

⚠️ **The downgrade was confirmed EMPIRICALLY, not from documentation:** ✅ **a fresh `NSTextView`
reports `textLayoutManager != nil`; after one `_ = tv.layoutManager` it reports `nil`.**
✅ **`xcodebuild` green on macOS, iOS AND visionOS. `ctest` 594/594 — the core was not touched.**

### ⚠️ LIVE PASS 1 (2026-09-13) — ✅ **THE MIGRATION WORKED. ⚠️ THE PREMISE WAS WRONG.**

✅ **THE APP REPORTS `[SCRIVI-TK] engine at construction: TextKit 2` AND `at rebuildStorage: TextKit 2
(segments=1155)`.** ⚠️ **So T-0525–T-0527 succeeded — and the app is STILL UNUSABLE.**

⚠️ **USER-OBSERVED, all four DoD items FAILED:**
⚠️ **(1) NO DIVIDER RENDERED in Dark Mode** — ⚠️ **a REGRESSION from T-0526.**
⚠️ **(2) window-edge drag ⇒ ~15 s beachball** (predicted `4.2 ms`).
⚠️ **(3) navigator click ⇒ ~30 s beachball.**
⚠️ **(4) typing ⇒ 7–10 characters then a stall; ⚠️ BACKSPACE ⇒ 15–30 s PER CHARACTER.**

### ⚠️ MY T-0529 MEASUREMENT WAS INVALID, AND I REPORTED IT AS IF IT WERE NOT

⚠️ **It benchmarked a BARE `NSTextLayoutManager` — no `NSTextView`, no `NSScrollView`, no SwiftUI.**
⚠️ **I labelled it "before/after on the real manuscript" without saying it was SYNTHETIC.**
⚠️ **THE `532x` LIVE-DRAG FIGURE DESCRIBES MY HARNESS, NOT SCRIVI.**
⚠️ **This is the SECOND invalid benchmark this sprint** — ✅ the first was the `204 ms` keystroke
(§8 of the trade study), also from text that did not resemble the real document.

### ✅ WHAT THE EVIDENCE NOW SAYS

⚠️ **LAYOUT WAS NOT THE BOTTLENECK.** ✅ **The engine is demonstrably TextKit 2 and the app is no
better** — ⚠️ **so the remaining cost is in SCRIVI'S OWN CODE on the interaction paths, not in AppKit's
layout.**
⚠️ **BACKSPACE AT 15–30 s PER CHARACTER IS THE SHARPEST CLUE:** ⚠️ **deletion is WORSE than insertion,
which no layout profile explains** — ✅ **it points at per-keystroke work whose cost depends on the
whole document.**
⚠️ **`textDidChange` (`:777`) is the prime suspect and runs on EVERY keystroke:** it calls
`recomputeBoundaries`, then `segmentIndex`, then ⚠️ **`(tv.string as NSString).substring(with:)`** —
⚠️ **`tv.string` on a 1.8 MB view, bridged to `NSString`, per character.**

✅ **INSTRUMENTED, NOT ASSUMED (T-0531):** ⚠️ **`[SCRIVI-KEY]` breaks the keystroke into
bounds/segIdx/extract/update/rest; `[SCRIVI-NAV]` breaks the click into
setSelectedRange/scrollRangeToVisible/centerStorageOffset/setViewportScene.**
⚠️ **NOTHING FURTHER IS TO BE CHANGED UNTIL THOSE NUMBERS EXIST.**

### ⚠️ ALSO FOUND — the `tick` probe was lying AGAIN

⚠️ **A scene split printed `rebuildStorage … elapsed=328.2s … projected=3788s` while its own closing
line said `took 0.0 s`.** ⚠️ **The fix for this was written in [SP-132], LOST IN THE BISECT, and never
reapplied.** ✅ **Restored.** ⚠️ **A probe that lies has now cost two investigations.**

### ⚠️ Sequencing

⚠️ **T-0525 and T-0526 together, or neither** — ✅ **the divider is rendered BY the layout engine, so a
half-migrated view has no working scene separator.**
⚠️ **T-0527 lands WITH them.** ✅ **T-0528 gates the live pass** — ⚠️ **a wrong boundary misplaces the
caret, which is worse than a slow one.** ⚠️ **T-0529/T-0530 measure; T-0531 is last.**

### Definition of Done

- [ ] ⚠️ **A LIVE PASS BY THE USER on the 1,153-scene fixture: click, scroll, type, RESIZE THE WINDOW,
      and create/delete/merge/split a scene — ✅ with NO beachball.** ⚠️ **THE RESIZE AND THE SPLIT ARE
      THE NEW ONES** — ✅ **they are where the `222 ms` and `270 ms` live.**
- [ ] ✅ **MEASURED before AND after** (T-0529), ⚠️ **not "it feels faster".**
- [ ] ⚠️ **The caret lands correctly after a navigator click, a split, and a merge** — ⚠️ **[I-0131]'s
      exact failure, and the one T-0528 exists to prevent.**
- [ ] ⚠️ **`grep -c "\\.layoutManager" Scrivi/Views` == 0**, ✅ **and the guard (T-0527) FAILS if it returns.**
- [ ] ⚠️ **The long-paragraph limitation is MEASURED and WRITTEN DOWN** (T-0530), ✅ **whatever it says.**
- [ ] ✅ **`xcodebuild` green for macOS, iOS AND visionOS.** ⚠️ **`ctest` still 594/594** — ⚠️ **this sprint must not touch the core.**

### ⚠️ Risks

| Risk | ⚠️ Mitigation |
| ---- | ------------ |
| ⚠️ **A build that compiles but renders NOTHING** | ⚠️ **THIS ALREADY HAPPENED IN [SP-132]: I shipped a navigator change that removed the app's window and reported it as "builds on three platforms".** ✅ **A UI change is UNVERIFIED until a window is SEEN** |
| ⚠️ **Long paragraphs defeat TextKit 2** | ✅ **T-0530 measures it ON PURPOSE.** ⚠️ **It is a property of the engine, not a bug to fix** |
| ⚠️ **`sceneBoundaries` breaks silently** | ⚠️ **A wrong boundary MISPLACES THE CARET and can send an edit to the wrong scene.** ✅ **T-0528, and the DoD tests split/merge explicitly** |
| ⚠️ **The divider disappears or mis-sizes** | ⚠️ **`NSTextAttachmentViewProvider` sizes differently from `cellFrame(for:)`.** ✅ **Visual check in the live pass, both Light and Dark** (⚠️ **[I-0112] is the precedent**) |
| ⚠️ **The downgrade returns later** | ✅ **T-0527's guard, landed WITH the work.** ⚠️ **The current downgrade was TWO LINES nobody noticed** |
| ⚠️ **Declaring [I-0203] fixed because it did not appear** | ⚠️ **Absence in one pass is not a fix.** ✅ **T-0531 must state which it is** |

### ⚠️ NOT in this sprint

⚠️ **GUARD-BANDING (trade-study Option B).** ✅ **Option C rules it out until open time or MEMORY — not
layout — is the constraint**, ⚠️ **and it has TWO unbuilt prerequisites by user ruling: core-side ⌘F
across unloaded scenes (5.5) and AC6 async (5.6).**

⚠️ **THE iOS PORT ITSELF.** ✅ **This sprint makes it POSSIBLE** — ⚠️ **`NSTextLayoutManager`,
`NSTextContentStorage` and `NSTextAttachmentViewProvider` are the SAME TYPES on iOS, whereas
`NSLayoutManager` + `NSTextAttachmentCell` are not** — ⚠️ **but the `2,198`-line view and its
`UIViewRepresentable` wrapper are a separate sprint** (5.3: after Linux, before v1.0).

⚠️ **SP-132's UNLANDED WORK.** ⚠️ **T-0519/T-0520 (navigator + guard caching) were REVERTED and are
NOT in `a50ebc9`; ✅ T-0522 (change-driven Spotlight) and T-0524's batching were also lost.**
⚠️ **T-0522 is still worth doing — `355 ms` of blocking Spotlight work per resign is MEASURED** —
✅ **but it belongs to SP-132's close-out, not here.**

---

