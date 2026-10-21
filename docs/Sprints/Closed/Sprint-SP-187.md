---
sprint: SP-187
epic: EP-053
status: Active
activated: 2026-10-20
platform: Cross
created: 2026-10-20
---

# SP-187 — `[Apple]` + `[Linux]` [EP-053] **S2a**: the scene list as the model of record — no visible change

**Status:** ✅ **CLOSED 2026-10-21** (user, 2026-10-21: *"close all five sprints and keep I-0308 open."*) · ✅ COMPLETE 2026-10-21 (every AC met; ⏳ close awaits the user) · 🟡 **ACTIVE 2026-10-20** (user: *"complete planning for SP-187 and activate it"*) — created 2026-10-20 in Planning
(user: *"complete planning for EP-053"*). ✅ Q1–Q4 ruled 2026-10-20, all as recommended.
**Carries:** [T-0615] → [`../Tasks/Task-active.md`](../../Tasks/Task-active.md).
**Epic:** [EP-053] → [`../Epics/Epic-active.md`](../../Epics/Epic-active.md).
**Builds on:** [SP-186] (S1, ✅ closed): `scrivi_range_read`, `scrivi_range_paste`, `scrivi_range_cut` and the delta they return.
**Followed by:** [SP-191] (S2b): copy, paste and its undo, split, merge and the chapter operations applied as deltas, no reload.
**Authority:** [`../Scrivi_Structured_CutCopyPaste_Design_v0_2.md`](../../Scrivi_Structured_CutCopyPaste_Design_v0_2.md) — R5,
R7, P3, P5, §4.2a.
**Shape:** ✅ the same on both platforms (R5; `feedback_linux_adopts_apple_shape`).
**Size:** L. Needs a **Mac live pass** and a **rig live pass**, each confirming nothing visible changed.

---

## Rulings (user, 2026-10-20)

| # | Question | Ruling |
| - | -------- | ------ |
| **Q1** | Split S2? | ✅ **Split.** This Sprint (S2a) makes the list the model of record with no visible change; [SP-191] (S2b) moves the operations onto deltas. Each ships green and live-passable. |
| **Q2** | Old `structuredPaste` / `structuredCut` history nodes | ✅ **Convert them to the delta path** (`scrivi.fragment.v1` → Range): no reload path survives. Built in [SP-191]. |
| **Q3** | Where the delta is applied on Apple | ✅ **`ViewportSceneLoader`**, beside `splitScene` / `mergeSceneIntoPredecessor`. One owner of the scene list. (Linux: `SceneDocument`.) |
| **Q4** | A literal doubly linked list, as R7 words it? | ✅ **Keep the ordered array.** A neighbour is index ±1 and a run is one splice. The Range stays the linked list on the wire; both sides rebuild its links from order, as S1 does. |

## What reading the code found (2026-10-20)

**Apple: the list already holds the text; the RENDERING is all-or-nothing.**
- `ViewportSceneLoader.segments` (`ViewportSceneLoader.swift:29`) is the ordered scene list with each scene's text, kept
  current on every keystroke (`textDidChange` → `updateText`, `ManuscriptTextView.swift:987`, `:1017`).
- ⚠️ But every structural change re-renders the WHOLE manuscript: `rebuildStorage` (`:773`) clears the text storage and
  appends every scene, divider and heading again. Its callers: split (`:1204`) and merge (`:1514`) through
  `rebuildStorageAndPlaceCursor` (`:1638`), and `:259`, `:1615`, `:1630`, `:1711`, the reload (`:2421`).
- The loader's in-memory patches already exist for split, merge, insert, remove, reorder and chapter split and merge
  (`ViewportSceneLoader.swift:662`–`:1141`). What is missing is a way to re-render just the scenes they touched.

**Linux: the rendering is already surgical; the TEXT lives only in the document.**
- `SceneDocument::segments()` (`SceneDocument.hpp:82`) holds identity and offsets only: `bodyStart` and `bodyLength`
  (`SceneDocument.hpp:20`–`48`). A scene's text exists only in the `QTextDocument` and is extracted on demand
  (`bodyText`, `SceneDocument.cpp:822`; 19 call sites).
- `insertSceneAfter`, `removeScene`, `removeChapter` and `moveScene` already re-splice the document without a rebuild.
- ⚠️ So on Linux the DOCUMENT is the model of record and the list is a map into it: the reverse of §4.2a.

**Neither platform binds the range endpoints yet.** `ScriviEngine.swift` and `ScriviBridge` have no `range*` methods.

## Goal

✅ On the Mac and on Linux, the in-memory scene list holds every scene's identity and stored text and is the model of
record; the text view renders it. One primitive re-renders a run of scenes without touching the rest, and the existing
in-memory structural paths use it. **The writer sees no difference.**

## Plan

0. **Instrument first** (the baseline S2b is measured against). Per structural operation, on both platforms: main-thread core
   calls, project re-opens, and full re-renders. Apple: counters beside `onSceneWritten`; Linux: the diagnostic log
   ([T-0612]) plus test counters. Recorded here before any change.
1. **Bind the range endpoints** (the porting outline's first layer): `rangeRead`, `rangePaste`, `rangeCut` in `ScriviEngine`
   (+ the visionOS stub, `check-engine-stub-parity.sh`) and in `ScriviBridge`. Tested through the binding (Apple interop;
   Linux smoke), against the same cases as S1's C ABI tests.
2. **The delta's platform types.** One Swift type and one C++ type decoding S1's delta, field for field (scenes created,
   rewritten, moved; removed scenes; created and removed chapters). ⚠️ A missing key is an empty list.
3. **Linux: the list holds the text.** `SceneSegment` gains the scene's stored text, kept current from
   `applyContentsChange` (Apple's `updateText` shape). `bodyText(i)` answers from it. An invariant check — each segment's
   text equals its body in the document — runs in every smoke after every edit kind.
4. **Apple: re-render a run.** `ViewportSceneLoader` gains `replaceSegments(_ range:, with:)`; the coordinator gains a partial
   re-render that replaces only those scenes' storage ranges (dividers and headings at their edges included) and updates the
   boundary table. Undo registration and `textDidChange` stay suppressed as `rebuildStorage` does (`isRebuilding`).
5. **Re-point the in-memory paths on Apple** — split (`:1204`), merge (`:1514`), insert and remove scene, chapter split and
   merge — to the partial re-render. The caret keeps its height in the viewport (T-0573) and the viewport lays out
   ([I-0271]), as `revealCaretAfterRebuild` does today. Load and the reload (`:2421`) keep the full render (S2b removes the
   reload).
6. **Tests:** the invariant on both after every structural path — Apple: the storage equals a full render of the list;
   Linux: the document equals `build()` of the list. Mutation-checked. ctest, interop and Linux smokes green.
7. **Live passes (user):** Mac and rig — type, Find/Replace, split, merge, new chapter, merge chapter, undo of each, quit and
   reopen: everything behaves exactly as before.

⛔ **NOT in this Sprint:** copy, paste, cut or undo on the range endpoints, and removing any reload ([SP-191]) · structural
cut ([SP-188]) · moving core calls off the main thread ([SP-191]).

## Acceptance Criteria

- [x] AC1 — `rangeRead` / `rangePaste` / `rangeCut` bound on both platforms and tested through the binding; the stub parity
      check passes — ✅ `ScriviEngine` + the visionOS stub (parity ✅), `ScriviBridge`; `RangeBindingInteropTests` (Apple) and
      `range_smoke` (Linux, new, in CI): read across a chapter, a chapter-opening paste, the cut of what it inserted
      restoring the manuscript, a stale caret refused — every write checked by reopening
- [x] AC2 — The delta decodes into one Swift and one C++ type with the same fields — ✅ `RangeDelta` (`ScriviEngine.swift`) and
      `RangeDelta` (`platforms/linux/src/RangeDelta.hpp`); an absent list decodes empty on both (asserted)
- [x] AC3 — Linux: every segment holds its stored text, current after typing, Find/Replace and Replace All, the Format
      commands, undo and redo, structured paste, split and merge; `bodyText` answers from it; the invariant holds in every smoke
      — ✅ 38/38 Linux smokes with `SCRIVI_VERIFY_SCENE_TEXT=1` (every change checked, a mismatch fatal)
- [x] AC4 — Apple: a partial re-render replaces a run of scenes; split, merge, insert and remove scene, chapter split and
      merge use it instead of `rebuildStorage`; the storage equals a full render after each — ✅ `renderRun`; split, new scene
      at a scene's end and before a scene, merge, chapter split, chapter merge each make ONE run render and NO full render
      (`RunRenderTests`); the storage equals a full render of the list and the boundary table its ranges after each; typing
      after them lands in the right scene. ⚠️ With chapter titles SHOWING, a chapter split or merge renders everything (the
      designed fallback: "Chapter N" titles after it renumber) — asserted. ⚠️ Delete and reorder from the navigator still
      re-render through `updateNSView`'s full rebuild: not in this AC's list of paths, recorded for [SP-191]
- [x] AC5 — The baseline counts (main-thread core calls, re-opens, full re-renders per operation) are recorded here — ✅ see
      the 2026-10-20 log below (Apple after this Sprint's run render; Linux as it stands)
- [x] AC6 — Tests on both, mutation-checked; ctest, interop and Linux smokes green — ✅ interop **297/297**; Linux smokes
      **38/38** (Docker, Qt 6.4, Debug, non-root; `range_smoke` and `structural_baseline_smoke` new); the Mac app builds.
      Mutations: Apple **5/5 caught** (the follower's prefix, the shifted ranges, always-full, the titles fallback, a late
      start); Linux **3/3 caught** (typing, a same-length replacement, the load). ⚠️ Two more Linux mutations SURVIVED and
      were EQUIVALENT: `moveScene` / `moveChapter` re-set a copied scene's text, which the copy already carries (they differ
      only while a pending pair exists, and every caller drops it first) — the lines were dead, and are removed. ScriviCore
      is unchanged by this Sprint
- [x] AC7 — ✅ **Live passes (user):** Mac and rig — no visible change — ✅ 2026-10-21, all 10 steps on both (rig build 75)

## Progress log

### 🔵 2026-10-20 — Sprint CREATED in Planning
- User: *"complete planning for EP-053."* Issued with [SP-188]–[SP-190] via `next-id.py`; [T-0615] filed.

### 🟡 2026-10-20 — Re-planned as S2a, Q1–Q4 RULED, and ACTIVATED
- User: *"lets complete planning for SP-187 and activate it."* Q1–Q4 ruled as recommended (above). Q1 split the Sprint: this
  one is S2a; S2b is [SP-191] (issued via `next-id.py`, [T-0619]), which takes the operations, [SP-185] AC4 and Q2.
- Code read for the plan (see above): Apple's list already holds the text but re-renders everything; Linux renders surgically
  but keeps the text only in the document.

### 🟡 2026-10-20 — Steps 0–5 built
- User: *"yes please implement SP-187."*
- **Step 1, the bindings:** `rangeRead` / `rangePaste` / `rangeCut` in `ScriviEngine` (+ stub) and `ScriviBridge`. Swift
  types `ScriviRange` (Codable, the wire form) and `RangeDelta` / `RangeOperationResult`; C++ `RangeDelta::fromVariant`
  (header-only). Tested through the bindings: `RangeBindingInteropTests` (1 test, passed) and `range_smoke` (Linux, in CI).
- **Step 3, Linux:** `SceneSegment::text` holds the scene's text; `applyContentsChange` keeps it current (⚠️ including a
  same-length replacement, which reports the same counts as a format change); `build`, `moveScene` and `moveChapter` set it;
  `bodyText` answers from it (no document read). With `SCRIVI_VERIFY_SCENE_TEXT=1` — set for the whole Linux CI job and by
  the smoke scripts — every change is checked against the document and a mismatch is fatal.
- **Step 4–5, Apple:** ONE renderer (`renderedPrefix` + `renderedManuscript`) builds the storage for `rebuildStorage` and
  for the new `renderRun`, so the two cannot drift. `renderRun` replaces a run's prefixes and bodies plus the next scene's
  prefix, and splices the boundary table. Split, new scene, merge, chapter split and chapter merge use it.
- **Counters:** Apple `coreCallCounts` (main-thread core calls, `openProject` calls) and the coordinator's `fullRenders` /
  `runRenders`; Linux `ScriviBridge::guiThreadCallCount`, `EditorShell::loadCount`, `SceneDocument::builds`. A new Linux
  smoke, `structural_baseline_smoke` (in CI), runs each operation through a real `EditorShell` and prints its baseline.
- ⚠️ **Found — [I-0313] filed (Apple, not this Sprint's):** typing into an empty scene that sits before another empty scene
  is credited to the next scene. Reproduced after a full render, so not the run render's. ⚠️ An earlier probe "showed it
  fixed" by a full render — that run matched ZERO tests (Swift Testing needs `sceneOperations()` with its parentheses);
  re-run properly, it failed the same way.

**AC5 — the baseline (2026-10-20; harness projects of 2–6 scenes, so the counts are per-operation, not per-manuscript):**

| Operation | Apple main-thread core calls | Apple re-opens | Apple full renders | Linux GUI-thread core calls | Linux loads | Linux document builds |
| --------- | ---: | ---: | ---: | ---: | ---: | ---: |
| Split scene | 33 | 0 | 0 (1 run) | 9 | 1 | 1 |
| New scene at a scene's end | 18 | 1 | 0 (1 run) | — | — | — |
| New scene before a scene | 11 | 0 | 0 (1 run) | — | — | — |
| Merge scene | 5 | 0 | 0 (1 run) | 5 | 1 | 1 |
| Split chapter | 24 | 0 | 0 (1 run) | 10 | 1 | 1 |
| Merge chapter | 18 | 1 | 0 (1 run) | 4 | 1 | 1 |
| Copy across scenes | — | — | — | 4 | 0 | 0 |
| Structured paste | — | — | — | 9 | 1 | 1 |
| Undo of the paste | — | — | — | 6 | 1 | 1 |

- ⚠️ Read with care: Apple's counts include the operation's own saves and the timeline refresh; one "re-open" is a
  `openProject` (merge chapter's `refreshScenePaths`; the end-of-scene case's is the timeline's). Apple's paste and undo
  are not in the harness yet: they reload through `reloadManuscriptFromDisk` today (one re-open + one full render each, by
  reading the code) — measured in [SP-191] before they change.
- ✅ The Linux numbers are what S2b removes: **every structural operation reloads the project and rebuilds the document.**

### 🟡 2026-10-20 — Tests green; ⏳ AC7 live passes (user)
- Interop 297/297; Linux smokes 38/38 with the text check on; mutations as AC6 records. [T-0615] → 🟠 Implemented - Not
  Verified. What is left is AC7: the Mac and rig live passes, confirming nothing visible changed.

### ✅ 2026-10-21 — AC7 passed on the Mac and the rig; COMPLETE

User, 2026-10-21: *"I performed the live pass listed above on both the Rig and the Mac. all 10 steps passed. I continued with the Scrivi Rig Live Pass laid out in the attachemnt. This is build 75. Step 11: Ctrl-Alt-F showed Find-and-Replace. Step 22: The Bold text is acceptable. No steps failed!"*
- All 10 steps on both platforms: typing, the three splits, merge, chapter split and merge (the Mac also with chapter titles), the undo notices, quit and reopen.
- [T-0615] ✅ Verified and archived. ✅ Closed 2026-10-21 (user-approved); next is [SP-191] (S2b).
