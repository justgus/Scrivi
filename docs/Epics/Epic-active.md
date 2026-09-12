# Active Epics

**EP-039** — `[Cross]` Project Load Performance. ⚠️ **Sole active Epic.** Every other Epic is in
[`Epic-backlog.md`](Epic-backlog.md).

---

## EP-039: `[Cross]` Project Load Performance — ⚠️ **the in-memory index, and the async gap**

**Status:** 🟡 **ACTIVE**
**Codebase:** `[ScriviCore]` (the indexes + bulk endpoints) **+ `[Apple]`** (the async gap). ⚠️ **Linux
already has its half** (T-0499/T-0500).
**Goal:** ⚠️ **A project of ORDINARY SIZE opens without freezing the app**, ✅ **and the core stops
answering per-item questions with full-tree traversals.**
**Design:** [`../Scrivi_Project_Index_Design_v0_1.md`](../Scrivi_Project_Index_Design_v0_1.md)
**Date Created:** 2026-09-10 · **Target Close:** — (⚠️ **estimated 3–4 sprints**)

**Sprints:** ✅ **SP-131 VERIFIED** (`[ScriviCore]` the indexes — **AC1–AC5, AC8**; T-0511–T-0517).
🔵 **SP-132** (`[Apple]` the interaction cost + the async gap — **AC6, AC9, AC10**, ⚠️ **[I-0200]**;
T-0518–T-0524) · 🔵 **SP-129** · 🔵 **SP-130** — ✅ **all three in `Sprint-backlog.md`, NOT activated.**

**Order of work:** ✅ ~~SP-131 (indexes)~~ → **SP-132** (interaction + async) → **SP-129 / SP-130**.

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

- **AC6** — 🔵 **[SP-132] T-0523.** ⚠️ **`[Apple]` THE LOAD RUNS OFF THE MAIN THREAD, with a determinate progress bar.**
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

- **AC9** — 🔵 **[SP-132] T-0524.** ✅ **NOTIFICATIONS ARE BATCHED.** ⚠️ **A load must not post one notification per scene.**
  ⚠️ **`1135.66/second` on 1,153 scenes is the symptom; the AC is the BATCHING, not the number.**
  ✅ **Acceptance: opening the 1,153-scene fixture emits no `harmful notification post rate` warning, and
  ⚠️ **notification volume does not scale linearly with scene count.**

- **AC10** — 🔵 **[SP-132] T-0522.** ✅ **SPOTLIGHT INDEXING IS CHANGE-DRIVEN, NOT LIFECYCLE-DRIVEN.** ⚠️ **A resign with no
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
