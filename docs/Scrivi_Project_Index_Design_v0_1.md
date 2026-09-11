# Scrivi — In-Memory Project Index (design v0.1)

**Status:** 🔵 **DRAFT — for user ruling.** Nothing is implemented.
**Date:** 2026-09-10
**Origin:** [I-0196] root-cause analysis. ⚠️ **Every figure here is MEASURED, not estimated.**

---

## 1. The problem, stated as measurement

One project open, 1,153 scenes in 49 chapters, local SSD, Release build:

| phase | measured |
| ----- | -------- |
| `TimelineViewModel.load` | ⚠️ **251.02 s** |
| `loader.loadAll` (1,153 × `openScene`) | ⚠️ **69.80 s** |
| everything else | ✅ < 0.3 s |
| **WALL CLOCK** | ⚠️ **321.10 s** |

⚠️ **The app was frozen and unusable for the whole 321 s**, with no progress and no cancel.

### 1.1 Why — and it is NOT a coding slip

✅ **`ScriviCore` is STATELESS.** Its only member is `CoreServices services_`
(`ScriviCore.hpp:92`) — ⚠️ **no open-project state, no cache, no index.** Every call
re-derives everything from disk.

⚠️ **So a per-item question costs a FULL TRAVERSAL, by construction:**

- `findSceneMetaPath` (`ScriviCore.cpp:439`) resolves the **entire manuscript** to turn
  ONE `sceneID` into ONE path. ⚠️ **7 call sites.**
- `getSceneStoryTime` calls it, then reads and parses one sidecar — ✅ **measured at a FLAT
  `228 ms` per call** (#0 `228.09`, #577 `224.13`, #1153 `227.41`).
- `openScene` walks chapters until it matches — ✅ **measured `8 ms` → `116 ms` by
  position**, ⚠️ **so summing over N scenes is O(N²).**

⚠️ **The app then loops these per scene.** ✅ **Nobody wrote a nested loop; the quadratic is
emergent** — a stateless core asked per-item questions in a loop produces O(N²) for free.

⚠️ **THIS IS THE OMISSION:** the core exposes **per-item** endpoints and almost no **bulk**
ones, ⚠️ **so the app has no efficient way to ask.**

---

## 2. The ruling this design implements (user, 2026-09-10)

> ✅ **Hybrid.** ⚠️ **Leave the FILE LAYER stateless** — reads and writes stay atomic, to
> disk, filesystem-authoritative. ✅ **On project open, build in-memory B+-like indexes the
> core traverses** instead of re-walking the tree. ⚠️ **Two-pronged: manuscript TEXT and the
> corresponding JSON.** ✅ **At least THREE indexes, each keyed to a specific lookup task.**

⚠️ **Why this resolves the tension a binary choice could not:** ✅ **EP-027 made the
filesystem AUTHORITATIVE and that ruling stands** — ⚠️ **the index is a DERIVED ACCELERATOR,
never a second source of truth.** ⚠️ **Writes still go to disk atomically; the index is
updated from the write, never instead of it.**

✅ **Prior art in this codebase:** `ObjectIndex` already exists for exactly this reason —
its header says a card list must know an object has an image *"without opening it — D8's
thumbnails would otherwise cost one file read per row."* ⚠️ **This generalises a pattern the
project already validated.**

---

## 3. The three indexes — derived from the MEASURED lookup tasks

⚠️ **Each index exists because a specific, measured lookup is currently a full traversal.**
⚠️ **Not one is speculative.**

### 3.1 `SceneLocationIndex` — *"where does this sceneID live?"*

| | |
| - | - |
| **Key** | `sceneID` |
| **Value** | `metadataPath`, `contentPath`, `chapterID`, `chapterMetadataPath`, manuscript ordinal |
| **Replaces** | ⚠️ `findSceneMetaPath`'s full resolve (**7 call sites**) and `openScene`'s chapter walk |
| **Measured saving** | ⚠️ `8–116 ms` → **O(log N)** |

✅ **This one index removes BOTH quadratics' traversal half.**

### 3.2 `SceneStoryTimeIndex` — *"what is this scene's story time?"*

| | |
| - | - |
| **Key** | `sceneID` |
| **Value** | the `storyTime` block (`offsetMs`, `gapMs`, `durationMs`, sources, `bandID`, …) |
| **Replaces** | ⚠️ `getSceneStoryTime`'s traversal + sidecar parse, **called once per scene by the timeline** |
| **Measured saving** | ⚠️ **`251 s` → a single build pass** |

⚠️ **Why story time needs its OWN index rather than living in 3.1:** ✅ **the timeline reads
EVERY scene's story time on open** (`FR-022m`: a scene's offset is *derived* from the
previous scene's `offsetMs + durationMs`, so the chain requires all N), ⚠️ **whereas 3.1 is
a point lookup.** ⚠️ **Different access pattern, different index.**

### 3.3 `ManuscriptOrderIndex` — *"what is the reading order?"*

| | |
| - | - |
| **Key** | ordinal → scene; and chapter → its scenes |
| **Value** | ordered scene identity + titles + chapter membership |
| **Replaces** | ⚠️ **28 call sites** that construct a `ManuscriptOrderResolver` and resolve the whole tree |
| **Users** | `openProject`, fragment cut/copy/paste, Spotlight extract, timeline ordering |

⚠️ **This is the index the OTHER TWO are built from**, ✅ **so one traversal at open feeds
all three.**

### 3.4 ⚠️ The second prong — TEXT

⚠️ **The user's ruling names TWO prongs: the manuscript TEXT and the JSON.** ✅ **3.1–3.3 are
the JSON prong.** ⚠️ **The TEXT prong is NOT specified here** — ⚠️ **it needs its own
measurement first.** ✅ **Known text-side costs already observed:** Spotlight `extract`
re-reads and strips **1,154 scene bodies** on every open *and* on every app resign; the
editor loads every body eagerly (`loadAll`). ⚠️ **Whether that wants an index, lazy loading,
or both is an OPEN QUESTION, and guessing is what cost this investigation its first day.**

---

## 4. ⚠️ What this design must NOT break

| Guarantee | Why it is at risk | ✅ How it is kept |
| --------- | ----------------- | ---------------- |
| ⚠️ **The filesystem is AUTHORITATIVE** (EP-027) | An index is a second copy of the truth | ⚠️ **Index is DERIVED and DISPOSABLE.** ✅ Any doubt ⇒ rebuild from disk, never trust the cache |
| ⚠️ **External change is DETECTED** (`External_Change_Repair_Matrix`) | A stale index would hide an edit made outside Scrivi | ⚠️ **The invalidation rule is the hard part — see §5** |
| ⚠️ **Absence is never deletion** ([I-0183], [I-0181]) | An index miss must not read as "gone" | ✅ A miss ⇒ **fall back to a real traversal**, never to a negative claim |
| ⚠️ **Writes stay atomic** | Index updates could drift from disk | ✅ **Write to disk FIRST, then update the index from what was written** |

---

## 5. ⚠️ THE HARD PART — invalidation. This is where such designs fail.

⚠️ **An index that goes stale silently is WORSE than no index**, because every layer above
it correctly trusts a correct-looking answer — ✅ **which is precisely how [I-0183] destroyed
10 of 12 relationships** (a world resolved `available` while its index was unreadable).

⚠️ **Open questions that must be RULED, not assumed:**

1. ⚠️ **What invalidates?** Scene create/delete/rename/reorder, chapter ditto, fragment
   cut/paste — ✅ **all go through the core and can update the index.** ⚠️ **But an edit made
   OUTSIDE Scrivi (git checkout, Finder rename, a sync client) does not.**
2. ⚠️ **How is external change noticed?** mtime per file? A directory-level generation
   counter? ⚠️ **Doc 2's repair matrix already owns this question for open — the index must
   NOT invent a second answer.**
3. ⚠️ **What is the scope?** Per open project (a session), or per process? ⚠️ **Multi-window
   opens the same project ONCE (R3, EP-018), so per-project is natural.**
4. ⚠️ **Memory.** 1,153 scenes × identity+paths ≈ tens of KB — ✅ **trivial.** ⚠️ **The TEXT
   prong is NOT trivial and is why §3.4 is deferred.**

---

## 6. ⚠️ What this does NOT fix — stated so it is not assumed

⚠️ **The Apple app has NO ASYNCHRONY AT ALL.** ✅ **MEASURED: 93 engine call sites in
`Scrivi/App` + `Scrivi/Views`; ⚠️ ZERO run off the main thread** — no `Task.detached`, no
`DispatchQueue.global`, no `nonisolated` work anywhere. ⚠️ **`ProjectSession` is
`@MainActor`, so `load(at:)` and every call it makes execute on the main thread.**

⚠️ **THIS INDEX MAKES THE WORK SMALL. IT DOES NOT MAKE IT ASYNCHRONOUS.**
⚠️ **Even at 1 s, a synchronous main-thread load is a freeze on a slow volume** — ✅ **which
is [I-0195], fixed on Linux (T-0499/T-0500: `AsyncCall` + a determinate progress bar) and
⚠️ NEVER fixed on Apple.**

✅ **The two are INDEPENDENT and BOTH are required:**
⚠️ **index alone** ⇒ fast, but still freezes on slow storage;
⚠️ **async alone** ⇒ 321 s of honest, watchable, unusable waiting.

---

## 7. ✅ USER RULINGS (2026-09-10) — these are DECIDED

| # | Question | ✅ Ruling |
| - | -------- | -------- |
| 1 | Scope of the first slice | ✅ **ALL THREE indexes.** *"Let's do it right."* |
| 2 | Retire the churning functions? | ✅ **KEEP them, INDEX-BACKED** — ⚠️ **and audit for genuinely dead functions separately (§8)** |
| 3 | Timeline load | ✅ **SPARSE BULK CALL — see §9.** ⚠️ **This is the sharpest ruling in the set** |
| 4 | The TEXT prong | ✅ **DESIGN NOW** — ⚠️ **because Scene/Object/Item image THUMBNAILS will need the same mechanism, and they are potentially memory-intensive (§10)** |

---

## 8. ⚠️ Dead-function audit — MEASURED, and the answer is NOT what it looks like

⚠️ **Checked all 101 `scrivi_*` endpoints against BOTH platforms.**
✅ **ZERO are unreferenced.** ⚠️ **There are no dead endpoints at the ABI.**

⚠️ **At the Apple layer, `ScriviEngine` has FOUR methods no Apple code calls:**

| method | Apple | Linux | ⚠️ Verdict |
| ------ | ----- | ----- | --------- |
| `listImportedTimelines` | 1 | **5** | ⚠️ **NOT DEAD** |
| `promoteObject` | 0 | **2** | ⚠️ **NOT DEAD** |
| `setTimelineEpochLabel` | 0 | **2** | ⚠️ **NOT DEAD** |
| `updateImportedTimelineOffset` | 0 | **4** | ⚠️ **NOT DEAD** |

⚠️ **ALL FOUR ARE LIVE ON LINUX.** ✅ **They are not dead code — they are APPLE SURFACES THAT
WERE NEVER BUILT.** ⚠️ **Deleting them would delete working capability and re-earn
`project_capability_without_surface` from the wrong end.**

✅ **CONCLUSION: there is nothing to retire.** ⚠️ **What the audit actually found is a
PARITY GAP — Linux has surfaces Apple lacks.** ⚠️ **That is a finding worth its own Issue,
NOT a cleanup.**

---

## 9. ✅ `getStoryTimes()` — the SPARSE bulk call (user's design, ruling 3)

> ✅ *"One bulk call that basically tells you where to put all the dots. If there are none,
> then it's even more efficient because it only returns a record for a scene that HAS its
> time set."*

⚠️ **MEASURED ON THE REAL FIXTURE — this ruling is worth more than it looks:**

| | |
| - | - |
| scene sidecars on disk | **1,203** |
| scenes with a non-default `offsetSource`/`durationSource` | ✅ **0** |
| scenes whose `storyTime` block exists at all | ⚠️ **0 — the key is `null`** |
| ⚠️ **time spent reading them on open** | ⚠️ **251 s** |

⚠️ **251 SECONDS TO DISCOVER THAT NOTHING IS SET.** ✅ **A sparse call returns an EMPTY
ARRAY here**, ⚠️ **and the timeline draws its default chain with NO per-scene I/O at all.**

### 9.1 Shape

```c
/* Every scene whose story time is EXPLICITLY SET. Scenes on the default chain
 * are OMITTED -- they are derivable (FR-022m) and cost nothing to omit.
 * ⚠️ An empty array is the COMMON case and is NOT an error. */
const char* scrivi_list_story_times(const char* projectRootPath);
```

```json
{ "ok": true, "result": { "storyTimes": [ { "sceneID": "...", "offsetMs": 0,
  "offsetSource": "explicit", "gapMs": 0, "durationMs": 3600000,
  "durationSource": "explicit", "bandID": "", ... } ] } }
```

⚠️ **"Set" means `offsetSource != "default"` OR `durationSource != "default"` OR a non-empty
`bandID`.** ✅ **Served from `SceneStoryTimeIndex` (§3.2), so it is a memory scan.**

⚠️ **THE EMPTY-ARRAY TRAP APPLIES** (`project_envelope_empty_vs_failed`): `appendToArray`
omits the key entirely for an empty list, ⚠️ **so `{}` means "none set" OR "the call
failed".** ✅ **The empty case is the COMMON one here, so the caller MUST use the failure
signal, never emptiness.** ⚠️ **Getting this wrong would report a real timeline as empty.**

---

## 10. ⚠️ THE TEXT PRONG — designed now, because THUMBNAILS need the same mechanism

⚠️ **User's reason for deciding this now: Scene/Object/Item image thumbnails will need the
same machinery, and they are POTENTIALLY MEMORY-INTENSIVE.** ⚠️ **That constraint changes
the design, so it must be settled before the JSON prong hardens.**

### 10.1 ⚠️ The rule that separates the two prongs

✅ **JSON prong: index holds THE DATA.** Identity, paths, story time — ⚠️ **tens of KB for
1,153 scenes, so holding it all is free.**

⚠️ **TEXT/BLOB prong: the index holds ONLY THE LOCATION AND SHAPE, NEVER THE BYTES.**

⚠️ **WHY, and this is the whole point of deciding it now:** ✅ **1,153 scene bodies is a few
MB — survivable.** ⚠️ **1,153 IMAGE THUMBNAILS IS NOT**, ⚠️ **and a design that "just caches
the content" works for text and then falls over on images.** ✅ **Same index shape, and the
bytes are ALWAYS fetched on demand.**

### 10.2 `AssetLocationIndex` — the third-prong shape

| | |
| - | - |
| **Key** | `objectID` / `sceneID` |
| **Value** | ⚠️ **relative path + byte size + content hash + mtime. ⚠️ NO PIXELS, NO TEXT.** |
| **Answers** | *"does this have an image, and where?"* — ✅ **without opening it** |

⚠️ **This is EXACTLY the question `ObjectIndexEntry` already answers for object images**
(*"so a card list knows an object HAS one without opening it… would otherwise cost one file
read per row"*), ✅ **so the pattern is already proven in this codebase and is being
generalised, not invented.**

⚠️ **The bytes stay on disk and are read on demand, with an LRU bounded by MEMORY, not by
count** — ⚠️ **a count-bounded cache of images is a memory bug waiting for a big image.**

### 10.3 ⚠️ What is still deferred, and honestly

⚠️ **Whether scene BODIES should be eagerly loaded at all is NOT answered here.** ✅ **The
editor currently loads all 1,153** (`loadAll`), ⚠️ **and Spotlight re-reads and strips all
1,154 on every open AND every app resign.** ⚠️ **Both are real costs; NEITHER has been
measured in isolation yet.** ⚠️ **Guessing is what cost this investigation its first day —
so: measure, then rule.**
