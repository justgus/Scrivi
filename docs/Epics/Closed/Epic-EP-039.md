# Closed Epic — EP-039 `[Cross]` Project Load Performance

**Closed:** 2026-09-15 (user-approved) · **Archived from `Epic-active.md`:** 2026-09-15,
audit ruling **[R-07]**.

✅ **HEADLINE: a 1,174-scene project opens in `0.34 s`, from `~300 s`.**
⚠️ **The user ran the original for over an HOUR before killing it.**

⚠️ **CARRIED INTO [EP-040], NOT FIXED: [I-0206]** (offset-linear `setSel`, `57–81 ms` near the
document end) **and [I-0213]** (chapter create `~305 ms`). ✅ **Both are rows in EP-040's own Issues
table, so this move does not break the carry** — ✅ **verified before the move, per [R-07].**

⚠️ **ALSO CARRIED, AND NOT COVERED BY THE EPIC'S ACs: SP-132 closed PARTIAL.** ⚠️ **T-0519/T-0520 were
REVERTED (they shipped a build with no window) and T-0521 was never started.** ✅ **T-0521 is tracked in
`Tasks/Task-unverified.md`.**

---

## EP-039: `[Cross]` Project Load Performance — ⚠️ **the in-memory index, and the async gap**

**Status:** ✅ **CLOSED 2026-09-15 (USER-APPROVED)**
**Actual Close Date:** 2026-09-15

### ✅ CLOSE SUMMARY — what this Epic actually delivered

⚠️ **THE HEADLINE, AND IT IS THE GOAL VERBATIM:** ✅ **a 1,174-scene project opens in `0.34 s` wall
clock, from `~300 s`.** ⚠️ **The user ran the original for over an HOUR before killing it.**

| measure | before | after |
| ------- | ------ | ----- |
| project open (wall clock) | ⚠️ **~300 s** | ✅ **0.34 s** |
| `TimelineViewModel.load` | ⚠️ **113 ms** (was part of 234–251 s pre-index) | ✅ **3.6 ms** |
| `recomputeBoundaries` | ⚠️ **40.8 ms** per call, NINE call sites | ✅ **0.202 ms** (202x) |
| navigator click | ⚠️ **35–45 s** | ✅ **~0.3 s** |
| Spotlight on a no-change resign | ⚠️ **1,159 records re-read** | ✅ **SKIPPED** |
| chapter create (work only) | ⚠️ **~880 ms** | ✅ **~305 ms** |

✅ **ALL TEN ACs MET.** ⚠️ **TWO were met in ways worth recording honestly:**
⚠️ **[AC9] was ALREADY satisfied by [AC6]'s async work — ✅ I initially mis-attributed it to a change
of my own and corrected the record.**
⚠️ **[AC4] shipped at the CORE in [SP-131] but its Apple surface was NEVER BOUND** — ✅ **a capability
with no surface, found while chasing [I-0213] and adopted 2026-09-15** (`scrivi_list_story_times` →
`ScriviEngine.listStoryTimes`). ⚠️ **The 32x timeline-load win is that adoption, not the original AC.**

### ⚠️ CARRIED INTO [EP-040] — ✅ USER-RULED 2026-09-15

⚠️ **[I-0206]** — ✅ **`setSelectedRange` is LINEAR IN DOCUMENT OFFSET.** ⚠️ **Measured `57–81 ms` at
offsets near 1.85 M, against `0.8–1.2 ms` near the start.** ⛔ **I WRONGLY CLOSED THIS ONCE** — ⚠️ **on
a run whose cursor sat at ~1% into the document.** ✅ **ANY TEST FOR IT MUST EXERCISE THE END OF THE
DOCUMENT.**
⚠️ **[I-0213]** — ✅ **chapter create is `~305 ms` of WORK (modal excluded), down from `~880 ms`.**
⚠️ **`reloadSceneDots` is ~200 ms of that and is NOT yet attributed further.** ⛔ **Do not guess it —
instrument inside the function.**

⚠️ **NEITHER BLOCKS THIS EPIC'S GOAL** — ✅ **the open does not freeze** — ⚠️ **but both are real
O(DOCUMENT) costs on the manuscript surface, which is [EP-040]'s subject.**

### ⚠️ WHAT THIS EPIC COST IN WRONG ANSWERS — ✅ recorded so the pattern is visible

⚠️ **SEVEN root causes were asserted from CODE READING and later disproven by measurement:**
✅ **the navigator `flatRows` rebuild, the `updateNSView` guard, \"the remaining half is TextKit\",
the I-0203 starvation hypothesis, the I-0210 clipping hypothesis, the I-0206 \"resolved\" call, and
\"scene create costs 2.7 s\" (it was CHAPTER create, and the timer spanned a MODAL).**
✅ **EVERY ONE was corrected by a measurement, and in five cases the USER supplied the falsifying
observation.** ⚠️ **The standing lesson — ✅ `sample`/instrument BEFORE attributing a cost, and
⚠️ CHECK WHAT THE TIMER ACTUALLY SPANS.**

⚠️ **PRIOR STATUS:** 🟡 **ACTIVE**
**Codebase:** `[ScriviCore]` (the indexes + bulk endpoints) **+ `[Apple]`** (the async gap). ⚠️ **Linux
already has its half** (T-0499/T-0500).
**Goal:** ⚠️ **A project of ORDINARY SIZE opens without freezing the app**, ✅ **and the core stops
answering per-item questions with full-tree traversals.**
**Design:** [`../Scrivi_Project_Index_Design_v0_1.md`](../Scrivi_Project_Index_Design_v0_1.md)
**Date Created:** 2026-09-10 · **Target Close:** — (⚠️ **estimated 3–4 sprints**)

**Sprints:** 🟡 **SP-133 ACTIVE** (`[Apple]` **TextKit 2** — [I-0200]'s remaining half; T-0525–T-0531).
✅ **SP-131 VERIFIED** (`[ScriviCore]` the indexes — **AC1–AC5, AC8**; T-0511–T-0517). ⚠️ **SP-132 open,
its unlanded tasks superseded.**
🔵 **SP-129** · 🔵 **SP-130** — ✅ **both in `Sprint-backlog.md`, NOT activated.**

**Order of work:** ✅ ~~SP-131 (indexes)~~ → 🟡 **SP-133 (TextKit 2)** → ⚠️ **SP-132's remainder**
(**AC6** async, **AC9** batching, **AC10** Spotlight) → **SP-129 / SP-130**.
⚠️ **SP-133 was inserted AHEAD of SP-132's remainder because the measurement said so:** ✅ **the
manuscript surface is O(DOCUMENT) on TextKit 1, and async would only make that watchable.**

✅ **THE OPEN IS FIXED AND VERIFIED: `~300 s` → `1.06 s`** (user console, 2026-09-12).
⚠️ **THE APP IS STILL UNUSABLE, AND THAT IS A DIFFERENT SET OF DEFECTS** — ✅ **[I-0200]:** ⚠️ **the
navigator rebuilds ~1,200 rows per body pass, the `updateNSView` guard is itself O(N) and runs on
every pass, and scrolling drives both.** ⚠️ **Plus a `355 ms` blocking Spotlight re-index on EVERY
resign, and the app remains FULLY SYNCHRONOUS (AC6).** ✅ **All owned by [SP-132].**

### ⚠️ Why this Epic exists

⚠️ **One project open, 1,153 scenes in 49 chapters: the app is FROZEN for `~300 s`, with no progress and
no cancel.** ⚠️ **The user ran it for over an HOUR before killing it.**

| phase | 1,153 scenes |
| ----- | ------------ |
| `TimelineViewModel.load` | ⚠️ **`234–251 s`** — ✅ **78% of wall clock** |
| `loader.loadAll` (1,153 × `openScene`) | ⚠️ **`64–70 s`** |
| everything else | ✅ **< 0.5 s** |
| **WALL CLOCK** | ⚠️ **`299–321 s`** |

⚠️ **1,153 scenes is an ORDINARY MANUSCRIPT, not a stress test.** ⚠️ **The cost is QUADRATIC, so it is
invisible at 16 scenes and fatal at 1,153** — ✅ **which is why no earlier sprint caught it.**

✅ **THE QUADRATIC IS VISIBLE IN THE PER-SCENE TREND**: ⚠️ **the running average of `openScene` climbs
`12.4 → 16.4 → 20.5 → … → 53.3 ms` across a single load** (max `117 ms`). ⚠️ **A per-item cost that grows
with how many items came before it IS the defect.**

### ✅ Root cause — an OMISSION in ScriviCore, not a coding slip

✅ **`ScriviCore` is STATELESS** (its only member is `CoreServices services_`). ⚠️ **So a per-item question
costs a FULL TRAVERSAL by construction** — `findSceneMetaPath` resolves the ENTIRE manuscript to turn one
`sceneID` into one path (**7 call sites**), and **28 more** sites build a `ManuscriptOrderResolver` and
resolve the whole tree. ⚠️ **The app then loops these per scene.**
✅ **Nobody wrote a nested loop; the quadratic is EMERGENT.** ⚠️ **The core exposes per-item endpoints and
almost no bulk ones, so the app has NO EFFICIENT WAY TO ASK.**

### ⚠️ Two further costs, on the same manuscript

⚠️ **Neither is explained by the traversal defect, and both persist AFTER the load completes.**

1. ⚠️ **A NOTIFICATION STORM** — `Detected potentially harmful notification post rate of 1135.66
   notifications per second`, ✅ **≈ one per scene**, ⚠️ **fired during `restoreWritingSurface`.**
2. ⚠️ **SPOTLIGHT RE-INDEXES THE WHOLE PROJECT ON EVERY RESIGN** — `extract 1154 records` +
   `donate 1154 items` ⚠️ **ran FOUR FULL CYCLES in one session**: ✅ **once per open AND once per
   `willResignActive`**, ⚠️ **each re-reading and stripping all 1,154 scene bodies.**

⚠️ **`_NSDetectedLayoutRecursion` also fires during `restoreWritingSurface`, with `tvLen=1823706`** —
✅ **a single 1.8 MB text view holding the whole manuscript.**

⚠️ **KNOWN-BAD INSTRUMENTATION:** ⚠️ **the `rebuildStorage` probe reports a false rising
`projected=3519s`.** ✅ **Its `elapsed` column reports wall-clock-since-OPEN, not since-loop-start, so it
never advances and `avg`/`projected` are noise.** ✅ **The true figure is `<<< rebuildStorage took 0.0 s`.**
⚠️ **FIX THE PROBE — it has already sent one investigation chasing a non-defect.**

### Acceptance Criteria

- **AC1** — ✅ **VERIFIED (SP-131).** ✅ **`SceneLocationIndex`** (`sceneID` → paths, chapter, ordinal) replaces `findSceneMetaPath`'s
  full resolve. ⚠️ **Target: `8–116 ms` → O(log N).**
- **AC2** — ✅ **VERIFIED (SP-131).** ✅ **`SceneStoryTimeIndex`** replaces the timeline's per-scene read. ⚠️ **Target: `234–251 s` →
  a single build pass.**
- **AC3** — ✅ **VERIFIED (SP-131).** ✅ **`ManuscriptOrderIndex`** replaces the **28** resolver call sites; ✅ **one traversal at open
  feeds all three indexes.**
  > ⚠️ **AC1–AC3 ARE ONE DELIVERY.** ⚠️ **They are NOT independently shippable** — ✅ **`ManuscriptOrderIndex`
  > is what the other two are built from, and ONE open-time traversal feeds all three** (Design §3.3).
  > ⚠️ **They are three ACs ONLY because each carries its own target.** ⚠️ **Partial delivery is NOT
  > acceptable.**

- **AC4** — ✅ **VERIFIED (SP-131).** ✅ **`scrivi_list_story_times` — SPARSE**: returns a record ONLY for a scene whose story time is
  EXPLICITLY SET (`offsetSource != "default"` OR `durationSource != "default"` OR a non-empty `bandID`).
  ⚠️ **MEASURED: on a 1,203-sidecar fixture, ZERO scenes have a `storyTime` block — the key is `null`** —
  ✅ **so the call returns an EMPTY ARRAY and the timeline draws its default chain with NO per-scene I/O.**
  ⚠️ **`234–251 s` is spent discovering that nothing is set.**
  ⚠️ **THE EMPTY-ARRAY TRAP APPLIES and empty is the COMMON case** (`project_envelope_empty_vs_failed`):
  ⚠️ **the caller MUST use the failure signal, never emptiness**, or a real timeline reads as empty.
  ⚠️ **A dense variant and an explicit `count` field were both rejected** — ✅ **the failure signal is the
  project-wide answer to this trap and a second mechanism would be a second answer.**

- **AC5** — ✅ **VERIFIED (SP-131).** ✅ **THE INVALIDATION DESIGN — SATISFIED.**
  ✅ **Design §5 is RULED**: ⚠️ **write path — ANY DOUBT DROPS THE WHOLE INDEX**, and the next query
  rebuilds from disk (⚠️ **never per-entry; ⚠️ an index update NEVER fails a write**); ⚠️ **read path —
  BOUNDED STALENESS + VALIDATE-ON-USE, NO FILESYSTEM WATCHING** (✅ **the index is authoritative for
  LOCATION ONLY**; ⚠️ **the caller still opens the file, and the open IS the validation**); ✅ **scope —
  PER OPEN PROJECT**; ✅ **memory — UNBOUNDED for the JSON prong, which is provably under 1 MB.**
  ⚠️ **A stale path therefore cannot produce a WRONG ANSWER — only an EXTRA TRAVERSAL.**
  ⚠️ **Filesystem watching and a directory generation counter were both REJECTED** — ✅ **watching is
  weakest on network mounts, exactly where Scrivi already hurts; ⚠️ a directory mtime does not change
  when a FILE'S CONTENTS change, so it misses edits, which is the common case.**

  **Derived criteria — ✅ these are the testable form of AC5:**

  | # | Criterion |
  | - | --------- |
  | **AC5a** | ✅ **An index hit pointing at a MISSING or UNREADABLE file causes a REAL TRAVERSAL and a rebuild** — ⚠️ **never an empty or negative result** (*absence is never deletion*) |
  | **AC5b** | ✅ **A failed index update DROPS THE WHOLE INDEX**; ⚠️ **the next query rebuilds and returns the CORRECT answer** |
  | **AC5c** | ⚠️ **An index update NEVER fails a write** |
  | **AC5d** | ✅ **External mid-session change is CORRECTED ON USE**, at the cost of one traversal |
  | **AC5e** | ✅ **Two projects open at once do not share or collide in index state** |

- **AC6** — ✅ **VERIFIED 2026-09-14 (USER-APPROVED).** ✅ **EVIDENCE — the user's console:** ⚠️ **`WALL CLOCK 0.40 s`** for the whole open of the 1,158-segment `dumas-prose` fixture, ✅ **with `donateSpotlight` (`280.0 ms`) and `TimelineViewModel.load` (`108.8 ms`) accounted as named phases and `unaccounted 0.00`.** ✅ **`[SCRIVI-UPD] updateNSView total=459.6 guard=0.4 ms`** — ⚠️ **the guard is `0.4 ms`, confirming [T-0520]'s two-`Hasher` replacement of the 1,153-element `map(\.id)` holds.** ✅ **`rebuildStorage` `0.0 s` across all 1,158 segments.** ⚠️ **ORIGINAL TASK TEXT FOLLOWS.** ---- 🔵 **[SP-132] T-0523.** ⚠️ **`[Apple]` THE LOAD RUNS OFF THE MAIN THREAD, with a determinate progress bar.**
  ✅ **MEASURED: 93 engine call sites in `Scrivi/App` + `Scrivi/Views`; ZERO run off the main thread** — no
  `Task.detached`, no `DispatchQueue.global`, no `nonisolated` work anywhere. ⚠️ **`ProjectSession` is
  `@MainActor`, so every call it makes blocks the UI.** ✅ **Linux has this; ⚠️ Apple does not** — ⚠️ **this
  is [I-0195] on Apple.**
  ⚠️ **AC6 IS INDEPENDENT OF AC1–AC4 AND BOTH ARE REQUIRED:** ⚠️ **index alone ⇒ fast, but still freezes on
  slow storage; async alone ⇒ 300 s of honest, watchable, unusable waiting.**
  ⚠️ **SEQUENCED AFTER THE INDEXES, IN ITS OWN SPRINT** — ✅ **async over a 300 s load is 300 s of watchable
  waiting; the indexes make the progress bar honest.** ⚠️ **Linux's equivalent earned TWO defects
  ([I-0198] worker-thread widget touches, [I-0199] a progress bar on a hidden page)** — ⚠️ **BOTH are
  predicted to recur on Apple in some form** (`feedback_live_pass_finds_what_suites_cannot`).

- **AC7** — ✅ **THE BLOB-INDEX RULE, binding on this Epic and [EP-035]:** ⚠️ **a blob/asset index holds
  LOCATION AND SHAPE ONLY (path, byte size, hash, mtime) — NEVER THE BYTES.** ✅ **Bytes are fetched ON
  DEMAND, with an LRU bounded by MEMORY, NOT BY COUNT** — ⚠️ **a count-bounded cache of images is a memory
  bug waiting for a big image.**
  ⚠️ **WHY THE RULE IS HERE:** ⚠️ **Scene/Object/Item image THUMBNAILS need the same mechanism and are
  potentially memory-intensive.** ✅ **1,153 scene bodies is a few MB; ⚠️ 1,153 thumbnails is NOT** —
  ⚠️ **a design that "just caches the content" works for text and then falls over on images.**
  ⚠️ **THE BUILD (`AssetLocationIndex`) IS NOT IN THIS EPIC.** ✅ **[EP-035] AC5 (card thumbnails)
  implements it, constrained by this rule** — ⚠️ **building it here would ship an index with no consumer**
  (`project_capability_without_surface`).

- **AC8** — ✅ **VERIFIED (SP-131).** ⚠️ **A REGRESSION TEST PINS THE COMPLEXITY, not a duration.** ⚠️ **A timing assertion is flaky
  and explains nothing** — ✅ **assert that opening one scene does not cost work proportional to how many
  OTHER scenes exist** (the read-counting decorator pattern, as [I-0196]'s test already does).

- **AC9** — ✅ **MET — ⚠️ AND IT WAS ALREADY MET BY [AC6]'s WORK, NOT BY A NEW FIX.** ✅ **EVIDENCE — the user's post-AC6 console shows NO `harmful notification post rate` warning anywhere, on a 1,174-scene open.** ---- ✅ **WHY: the live open path is the ASYNC one** (`ProjectSession.swift:169-201`) — ✅ **`loadSegmentsOffMain` accumulates into LOCALS off the main thread, and `adoptLoadedSegments` then assigns `segments` and `liveTitles` WHOLESALE: TWO observation notifications for the entire load, regardless of scene count.** ✅ **That is the AC verbatim — the requirement is the BATCHING, not a number.** ---- ⚠️ **I INITIALLY MIS-ATTRIBUTED THIS.** ⚠️ **I optimised the SYNCHRONOUS `loadAll`, believing the `[SCRIVI-DIAG] loadAll: activeSceneID=` console line proved it ran.** ✅ **IT DOES NOT: that line lives in `resolveActiveScene`, which the code comment states is *"Shared by `loadAll` and `adoptLoadedSegments`"* — ⚠️ and `loader.loadAll (TOTAL)` was ABSENT from the user's timing table, which is the actual proof the sync path did NOT run.** ✅ **The sync-path change was KEPT anyway** (⚠️ **it removed a genuine ~2,348-notification storm from a path that other callers can still take, e.g. `replaceScenes` after a structural fragment op**) — ⚠️ **but it is NOT what satisfies this AC.** ---- ⚠️ **ORIGINAL TASK TEXT FOLLOWS.** ---- 🔵 **[SP-132] T-0524.** ✅ **NOTIFICATIONS ARE BATCHED.** ⚠️ **A load must not post one notification per scene.**
  ⚠️ **`1135.66/second` on 1,153 scenes is the symptom; the AC is the BATCHING, not the number.**
  ✅ **Acceptance: opening the 1,153-scene fixture emits no `harmful notification post rate` warning, and
  ⚠️ **notification volume does not scale linearly with scene count.**

- **AC10** — ✅ **VERIFIED 2026-09-14 (USER-APPROVED).** ✅ **EVIDENCE — the user's console: `onAppResign` → `dirty=0` → `spotlight: SKIPPED re-index (no dirty scene bodies)`, with NO `extract OK` and NO `donate` line.** ✅ **A no-change resign now performs ZERO scene-body reads, which is the AC verbatim.** ⚠️ **PRIOR RECORD:** 🟢 **IMPLEMENTED 2026-09-14, AWAITING A LIVE PASS.** ✅ **The gate now exists** (`ProjectSession.saveAllDirty()`, `hadDirtyBodies` captured before the saves). ⚠️ **ACCEPTANCE EVIDENCE TO LOOK FOR: a resign with `dirty=0` must log `spotlight: SKIPPED re-index` and must NOT log `extract OK: 1159 records`.** ---- ⚠️ **PRIOR RECORD RETAINED:** ⛔ **WAS FALSIFIED 2026-09-14 by the user's console, ⚠️ NOT verified.** ⚠️ **The same log that verifies AC6 DISPROVES this AC:** ✅ **`onAppResign` → `dirty=0` → `extract OK: 1159 records` → `donate accepted: 1159 items submitted`.** ⚠️ **A no-change resign re-extracted EVERY record** — ✅ **AC10's acceptance is *"a no-change resign performs zero scene-body reads"*.** ✅ **ROOT CAUSE: `ProjectSession.saveAllDirty()` calls `donateSpotlight` UNCONDITIONALLY (`:388-395`); ⛔ NO dirty gate exists on the resign path, and `git log -S` finds NO commit that ever added one.** ⚠️ **[T-0522] was recorded "Implemented - Not Verified" for code that was never written** — ✅ **corrected in [SP-132].** ⚠️ **ORIGINAL TASK TEXT FOLLOWS.** ---- 🔵 **[SP-132] T-0522.** ✅ **SPOTLIGHT INDEXING IS CHANGE-DRIVEN, NOT LIFECYCLE-DRIVEN.** ⚠️ **A resign with no
  changes must trigger NO re-extraction.** ⚠️ **Today every open AND every `willResignActive` re-reads and
  strips all 1,154 scene bodies.**
  ✅ **Acceptance: a no-change resign performs zero scene-body reads; re-extraction is scoped to what
  actually changed.**

  > ✅ **AC9 AND AC10 ARE NOT GATED ON MEASUREMENT.** ⚠️ **Complexity mitigation and notification batching
  > are correct regardless of what a profile would say** — ✅ **so the ACs are satisfied by the OVERALL
  > load/parse/query performance, not by a per-cause attribution.**

### ⚠️ Also in scope — the ScriviCore bypasses

⚠️ **`TimelineViewModel.loadImportedTimelines` does not call the core at all** — ⚠️ **it reads
`objects/imported-timelines/` with `FileManager` + `JSONDecoder` in Swift**, ⚠️ **while
`scrivi_list_imported_timelines` exists, is bound, and is what Linux calls.**
✅ **It sits ON the timeline load path measured at `234–251 s`.** ⚠️ **A bypass is a place the core's
guarantees do not apply**, ✅ **so indexing the core while the app reads around it would leave the
acceleration unreachable.** ⚠️ **Owned by [SP-129] (T-0502) and [SP-130] ([I-0197]).**

### ⚠️ Out of scope

| Item | Why |
| ---- | --- |
| ⚠️ **Retiring "dead" core functions** | ✅ **There are NONE.** ⚠️ **All 101 `scrivi_*` endpoints are referenced; the four `ScriviEngine` methods with no Apple caller are ALL LIVE ON LINUX** — ⚠️ **they are APPLE SURFACES NEVER BUILT, a PARITY GAP, not a cleanup** |
| ⚠️ **Lazy-loading scene BODIES in the EDITOR** | ⚠️ **NOT ruled.** ⚠️ **`loadAll` builds one 1.8 MB text view eagerly.** ✅ **AC10 covers Spotlight's share; the editor's does not yet have an owner** |
| ⚠️ **`AssetLocationIndex` (the build)** | ✅ **[EP-035] AC5, bound by AC7's rule** |
| ⚠️ **A core-held open-project SESSION** (beyond the indexes) | ⚠️ **Deliberately not proposed** — ⚠️ **it reintroduces the cache-invalidation and external-change questions EP-027 settled by making the filesystem authoritative** |

### ⚠️ Risks

| Risk | ⚠️ Mitigation |
| ---- | ------------ |
| ⚠️ **The index becomes a SECOND SOURCE OF TRUTH** | ⚠️ **EP-027's filesystem-authoritative ruling STANDS.** ✅ **The index is DERIVED and DISPOSABLE; any doubt ⇒ rebuild from disk** |
| ⚠️ **Silent staleness** | ✅ **AC5 is RULED: validate-on-use, and any doubt drops the index.** ⚠️ **A stale path costs a traversal, never a wrong answer.** ✅ **[I-0183] is the precedent for how this destroys data** |
| ⚠️ **Fixing the index and declaring victory** | ⚠️ **AC6 is the other half.** ✅ **A fast synchronous load is still a freeze on slow storage** |
| ⚠️ **Thumbnails designed after the fact** | ✅ **AC7 rules the blob shape NOW, while the JSON prong is still soft** |

---

