# Active Epics

⚠️ **TWO Epics are active**, running in parallel by user ruling 2026-08-25:
**EP-035** `[Linux]` Object Foundations · **EP-038** `[Linux]` The Real Hardware Rig.
⚠️ **EP-038's first task is blocked on PHYSICAL ACCESS**; EP-035's early work is blocked on nothing.

> ⚠️ **PRIORITY RESET — 2026-09-10, user ruling.** ✅ **[EP-039] was promoted and every other Epic
> backlogged**: *"It is the most important thing right now."*
>
> ⚠️ **WHY, in one measurement:** ⚠️ **an ORDINARY 1,153-scene manuscript freezes the app for `321 s`
> with no progress and no cancel** — ⚠️ **the user ran it for over an HOUR before killing it.**
> ⚠️ **The cost is QUADRATIC, so it is invisible at 16 scenes and fatal at 1,153.**
>
> ⚠️ **[EP-035] `[Linux]` Object Foundations was DEFERRED mid-Epic** (2 of 4 ACs open) → `Epic-backlog.md`.
> ✅ **Its 2 verified ACs are NOT lost.** ⚠️ **Its open AC5 (card thumbnails) is DEPENDENT on EP-039's AC7**
> — ✅ **the rule that a blob index holds LOCATION AND SHAPE, never the bytes** — ⚠️ **so doing AC5 first
> would design the memory behaviour twice.**
>
> ✅ **[EP-038] was CLOSED 2026-09-11** (all 7 ACs, all 3 Sprints) → `Epics/Closed/Epic-EP-038.md`.
> ✅ **[EP-039] is therefore the ONLY active Epic.**

## EP-039: `[Cross]` Project Load Performance — ⚠️ **the in-memory index, and the async gap**

**Status:** 🟡 **ACTIVE — promoted 2026-09-10 by user ruling**, ⚠️ **preempting EP-035**: ✅ *"the most important thing right now."* ⚠️ **Created the same day from [I-0196]'s root-cause analysis.**
**Codebase:** `[ScriviCore]` (the indexes + bulk endpoints) **+ `[Apple]`** (the async gap). ⚠️ **Linux
already has its half** (SP-128 / T-0499/T-0500).
**Goal:** ⚠️ **A project of ORDINARY SIZE opens without freezing the app**, ✅ **and the core stops
answering per-item questions with full-tree traversals.**
**Design:** [`../Scrivi_Project_Index_Design_v0_1.md`](../Scrivi_Project_Index_Design_v0_1.md)
**Date Created:** 2026-09-10 · **Target Close:** — (⚠️ **estimated 3–4 sprints, before implementation**)
**Sprints:** 🔵 **SP-129** (`[Apple]` the four unbuilt surfaces) · 🔵 **SP-130** (`[Apple]`+`[ScriviCore]` close the ScriviCore bypasses — [I-0197]) — ⚠️ **both in `Sprint-backlog.md`, PLANNING, not activated**

### ⚠️ Why this Epic exists — MEASURED, not estimated

⚠️ **One project open, 1,153 scenes: `321.10 s` frozen, no progress, no cancel.** ⚠️ **The user ran it for
over an HOUR before killing it.**

| phase | measured |
| ----- | -------- |
| `TimelineViewModel.load` | ⚠️ **251.02 s** (78%) |
| `loader.loadAll` (1,153 × `openScene`) | ⚠️ **69.80 s** |
| everything else | ✅ **< 0.3 s** |

⚠️ **1,153 scenes is an ORDINARY MANUSCRIPT, not a stress test.** ⚠️ **The cost is QUADRATIC, so it is
invisible at 16 scenes and fatal at 1,153** — ✅ **which is why no earlier sprint caught it.**

### ✅ Root cause — an OMISSION in ScriviCore, not a coding slip

✅ **`ScriviCore` is STATELESS** (its only member is `CoreServices services_`). ⚠️ **So a per-item question
costs a FULL TRAVERSAL by construction** — `findSceneMetaPath` resolves the ENTIRE manuscript to turn one
`sceneID` into one path (**7 call sites**), and **28 more** sites build a `ManuscriptOrderResolver` and
resolve the whole tree. ⚠️ **The app then loops these per scene.**
✅ **Nobody wrote a nested loop; the quadratic is EMERGENT.** ⚠️ **The core exposes per-item endpoints and
almost no bulk ones, so the app has NO EFFICIENT WAY TO ASK.**

### Acceptance Criteria (draft — ⚠️ to be ruled at promotion)

- **AC1** — ✅ **`SceneLocationIndex`** (`sceneID` → paths, chapter, ordinal) replaces `findSceneMetaPath`'s
  full resolve. ⚠️ **Measured target: `8–116 ms` → O(log N).**
- **AC2** — ✅ **`SceneStoryTimeIndex`** replaces the timeline's per-scene read. ⚠️ **Measured target:
  `251 s` → a single build pass.**
- **AC3** — ✅ **`ManuscriptOrderIndex`** replaces the **28** resolver call sites; ✅ **one traversal at open
  feeds all three indexes.**
- **AC4** — ✅ **`scrivi_list_story_times` — SPARSE**: returns a record ONLY for a scene whose story time is
  EXPLICITLY SET. ⚠️ **MEASURED: on a 1,203-sidecar fixture, ZERO scenes have a `storyTime` block — the key
  is `null`** — ✅ **so the call returns an EMPTY ARRAY and the timeline draws its default chain with NO
  per-scene I/O.** ⚠️ **`251 s` was spent discovering that nothing is set.**
  ⚠️ **THE EMPTY-ARRAY TRAP APPLIES and empty is the COMMON case** (`project_envelope_empty_vs_failed`):
  ⚠️ **the caller MUST use the failure signal, never emptiness**, or a real timeline reads as empty.
- **AC5** — ⚠️ **INVALIDATION IS RULED, not assumed.** ⚠️ **A silently stale index is WORSE than no index** —
  ✅ **[I-0183] destroyed 10 of 12 relationships exactly that way** (a world resolved `available` while its
  index was unreadable). ⚠️ **An index MISS must fall back to a real traversal, NEVER to a negative claim**
  (*absence is never deletion*). ⚠️ **External change (git checkout, Finder rename, a sync client) does NOT
  go through the core** — ✅ **Doc 2's repair matrix owns that question and the index must not invent a
  second answer.**
- **AC6** — ⚠️ **`[Apple]` THE LOAD RUNS OFF THE MAIN THREAD, with a determinate progress bar.**
  ✅ **MEASURED: 93 engine call sites in `Scrivi/App` + `Scrivi/Views`; ZERO run off the main thread** — no
  `Task.detached`, no `DispatchQueue.global`, no `nonisolated` work anywhere. ⚠️ **`ProjectSession` is
  `@MainActor`, so every call it makes blocks the UI.** ✅ **Linux solved this in SP-128; ⚠️ Apple never
  did** — ⚠️ **this is [I-0195] on Apple.**
  ⚠️ **AC6 IS INDEPENDENT OF AC1–AC4 AND BOTH ARE REQUIRED:** ⚠️ **index alone ⇒ fast, but still freezes on
  slow storage; async alone ⇒ 321 s of honest, watchable, unusable waiting.**
- **AC7** — ✅ **`AssetLocationIndex` — the TEXT/BLOB prong.** ⚠️ **The index holds LOCATION AND SHAPE ONLY
  (path, byte size, hash, mtime) — NEVER THE BYTES.** ⚠️ **WHY THIS IS RULED NOW: Scene/Object/Item image
  THUMBNAILS need the same mechanism and are potentially memory-intensive.** ✅ **1,153 scene bodies is a
  few MB; ⚠️ 1,153 thumbnails is NOT** — ⚠️ **a design that "just caches the content" works for text and
  then falls over on images.** ✅ **Bytes fetched on demand, LRU bounded by MEMORY, not by count.**
- **AC8** — ⚠️ **A REGRESSION TEST PINS THE COMPLEXITY, not a duration.** ⚠️ **A timing assertion is flaky
  and explains nothing** — ✅ **assert that opening one scene does not cost work proportional to how many
  OTHER scenes exist** (the read-counting decorator pattern, as [I-0196]'s test already does).

### ⚠️ SCOPE EXPANDED 2026-09-10 (user ruling) — ⚠️ **twice, and both from AUDIT FINDINGS**

⚠️ **This Epic began as "make project open fast."** ⚠️ **Auditing for dead functions found no dead
functions — ✅ it found UNBUILT SURFACES ([SP-129]) and then CORE BYPASSES ([I-0197] → [SP-130]).**

✅ **Both belong here rather than in a separate Epic, for a MEASURED reason, not a scheduling one:**
⚠️ **`TimelineViewModel.loadImportedTimelines` bypasses the core ON THE TIMELINE LOAD PATH** — ⚠️ **the
same path measured at `251 s`.** ✅ **A bypass is a place the core's guarantees do not apply**, ⚠️ **so
indexing the core while the app reads around it would leave the acceleration unreachable.**

### ⚠️ Out of scope

| Item | Where |
| ---- | ----- |
| ⚠️ **Retiring "dead" core functions** | ✅ **AUDITED 2026-09-10: there are NONE.** ⚠️ **All 101 `scrivi_*` endpoints are referenced; the four `ScriviEngine` methods with no Apple caller are ALL LIVE ON LINUX** — ⚠️ **they are APPLE SURFACES NEVER BUILT, a PARITY GAP deserving its own Issue, not a cleanup** |
| ⚠️ **Lazy-loading scene BODIES** | ⚠️ **NOT ruled.** ⚠️ **The editor loads all 1,153 eagerly and Spotlight re-reads all 1,154 on every open AND every resign** — ⚠️ **both real, NEITHER measured in isolation.** ✅ **Measure, then rule** |
| ⚠️ **A core-held open-project SESSION** (beyond the indexes) | ⚠️ **Deliberately not proposed** — ⚠️ **it reintroduces the cache-invalidation and external-change questions EP-027 settled by making the filesystem authoritative** |

### ⚠️ Risks

| Risk | ⚠️ Mitigation |
| ---- | ------------ |
| ⚠️ **The index becomes a SECOND SOURCE OF TRUTH** | ⚠️ **EP-027's filesystem-authoritative ruling STANDS.** ✅ **The index is DERIVED and DISPOSABLE; any doubt ⇒ rebuild from disk** |
| ⚠️ **Silent staleness** | ⚠️ **AC5.** ✅ **[I-0183] is the precedent for how this destroys data** |
| ⚠️ **Fixing the index and declaring victory** | ⚠️ **AC6 is the other half.** ✅ **A fast synchronous load is still a freeze on slow storage** |
| ⚠️ **Thumbnails designed after the fact** | ✅ **AC7 rules the blob shape NOW, while the JSON prong is still soft** |

---

---
