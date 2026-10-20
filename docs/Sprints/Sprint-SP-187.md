---
sprint: SP-187
epic: EP-053
status: Planning
platform: Cross
created: 2026-10-20
---

# SP-187 — `[Apple]` + `[Linux]` [EP-053] **S2**: the scene list as the model of record; copy, paste and its undo, split and merge — no reload

**Status:** 🔵 **PLANNING** — created 2026-10-20 (user: *"complete planning for EP-053"*). ⏳ Q1–Q3 below.
**Carries:** [T-0615] → [`../Tasks/Task-backlog.md`](../Tasks/Task-backlog.md) · [SP-185] AC4 (user, 2026-10-20: *"AC4 goes
to EP-053 S2"*) · [I-0297]'s reload half.
**Epic:** [EP-053] → [`../Epics/Epic-active.md`](../Epics/Epic-active.md).
**Builds on:** [SP-186] (S1): the Range, `scrivi_range_read`, `scrivi_range_paste`, `scrivi_range_cut`, and the delta they
return.
**Authority:** [`../Scrivi_Structured_CutCopyPaste_Design_v0_2.md`](../Scrivi_Structured_CutCopyPaste_Design_v0_2.md) — R1,
R3, R5, R7, P3, P5, §4.2a, §4.2b, §4.4, §4.5, Q5.
**Rulings that shaped it (user, 2026-10-20):**
- ✅ **Each operation ships with its undo.** Paste and its undo (a range cut) are here; structural cut and its undo are
  [SP-188]. (The Epic's original table split copy/cut from their undo across S2 and S4.)
- ✅ **Split, merge and chapter create/merge are in scope, here.** They apply deltas like paste, off the main thread, with
  no reload.
- ✅ **New history op names are approved, backward-compatible:** `rangePaste` / `rangeCut` beside the old
  `structuredPaste` / `structuredCut`, which keep their undo path.
**Size:** XL — the model-of-record change touches both editors. Q1 asks whether to split it.

---

## What reading the code found (2026-10-20)

**Apple**
- **The model today:** `ViewportSceneLoader.segments` (`ViewportSceneLoader.swift:29`), an array of `SceneSegment` (ID,
  chapter, paths, text, dirty), kept beside the text view's `NSTextStorage`, plus `allScenes` for the navigator and
  timeline.
- **Paste** (`ManuscriptTextView.swift:2696`): `fragmentPaste` on the **main thread**, then `reloadManuscriptFromDisk`
  (`:2407`). That reload re-opens the project, replaces every scene (`loader.replaceScenes`) and rebuilds the whole text
  storage (`rebuildStorage`, `:773`).
- **Paste's undo and redo** (`applyStructuralInverse`, `:615`): `fragmentPaste` or `fragmentUncutPaste` on the main
  thread, then the same full reload (`:639`, `:656`).
- **Copy** (`structuredCopyIfCrossBoundary`, `:2471`): `fragmentExtract` reads the scenes from disk.
- **Split** already patches memory (`loader.splitScene`, `ViewportSceneLoader.swift:885`), then rebuilds the whole
  storage. **Merge** (`handleMergeScene`, `ManuscriptTextView.swift:1446`) calls `mergeScene` inside a `@MainActor` task,
  so it runs on the main thread, then patches memory (`mergeSceneIntoPredecessor`, `:931`) and rebuilds the whole storage.

**Linux**
- **The model today:** `SceneDocument::segments()` (`SceneDocument.hpp:82`) beside the `QTextDocument`.
- **Paste** (`EditorShell.cpp:2544`): `fragmentPaste` on a worker (`runStructural`, `:2919`), read-only meanwhile, then
  `reloadThenPlaceCaret` → `reloadThen` (`:2979`), a full `load()`. SP-185 made that reload take unwritten texts from
  memory, but it still re-opens the project (~1,700 file calls over the share).
- **Paste's undo and redo** (`applyStructuralInverse`, `:2573`): the same worker, then the same reload. It first calls
  `saveDirtyScenes()` on the GUI thread.
- **Merge** (`onMergeSceneRequested`, `:2192`): `mergeScene` on the **GUI thread** (`:2224`), then a synchronous
  `readSceneTexts` for the barrier, then `reloadThen` (`:2240`). **Merge chapter** (`:2272`) reloads too (`:2321`).
- **Split** (`:1960`–`:2010`): `createScene` and two `saveScene` calls on the GUI thread, then `reloadThenPlaceCaret`.
  **Chapter create** (`:2038`) reloads (`:2179`).

⚠️ **So the two platforms differ for split and merge today.** Apple patches memory but calls the core on the main
thread. Linux reloads, and also calls the core on the GUI thread. R5 requires one shape.

## Goal

✅ On the Mac and on Linux, copy, paste, paste's undo and redo, split, merge, and chapter create and merge each make **one
core call off the main thread**. The editor applies the returned delta to its in-memory scene list and re-renders only the
scenes it names. **Nothing reloads the manuscript.**

## Plan

0. **Instrument first.** Count main-thread core calls and project re-opens per operation on both platforms (Apple: a test
   hook beside `onSceneWritten`; Linux: the diagnostic log, [T-0612]). Turn each target into a red test before changing
   code.
1. **The delta's platform type, on each side.** Decode S1's delta (scenes created, removed, moved, rewritten; chapters
   created, removed) into one Swift type and one C++ type, with the same fields.
2. **The scene list as the model of record (§4.2a).** Apple: `ViewportSceneLoader` holds the doubly linked list; Linux:
   `SceneDocument`. The text view becomes a rendering. Apply a delta = re-link nodes + re-render only the touched scenes'
   ranges in `NSTextStorage` / `QTextDocument`. ⚠️ This is the largest step (Q1).
3. **Copy** builds the Range from the list (§4.2b): no core call, no disk read. The clipboard and copy buffers take the
   Range (`scrivi.fragment.v1` stays readable).
4. **Paste** → `scrivi_range_paste` off the main thread on both (Apple: a detached task; Linux: `runStructural`), read-only
   with the progress row after 400 ms (Q5). The delta is applied; no reload. Recorded as a `rangePaste` history node.
5. **Paste's undo = `scrivi_range_cut`** of the range the paste inserted; redo = the paste again. Same thread rule, delta
   applied. Old `structuredPaste` nodes keep `fragmentUncutPaste` / `fragmentPaste` — but take the delta path too, if Q2
   rules so.
6. **Split, merge, chapter create, chapter merge:** off the main thread, delta applied, no reload, on both. If an existing
   endpoint does not return enough to build the delta, it is extended in ScriviCore here (this Sprint is then `[Cross]`),
   with boundary tests through `scrivi_*`.
7. **Retire** Apple's `reloadManuscriptFromDisk` and Linux's structural `reloadThen` for these paths, and SP-185's Linux-only
   `reloadTexts_` (design §4.6).
8. **Tests:** interop (Apple) and smokes (Linux) for each operation: the in-memory list equals the disk afterwards; zero
   project re-opens; zero main-thread core calls; the caret where the operation leaves it. Mutation-checked.

## Questions

| # | Question | Recommendation |
| - | -------- | -------------- |
| **Q1** | **Split this Sprint?** Step 2 (the model of record) is a refactor of both editors on its own. | ⏳ **Split:** S2a = steps 0–2 (the list as the model, no visible change, both platforms), S2b = steps 3–8. Each ships green and live-passable. Alternative: one Sprint, larger and riskier to verify. |
| **Q2** | **Old `structuredPaste` / `structuredCut` nodes:** keep the full reload after their undo, or convert them to the delta path? | ⏳ **Convert:** `scrivi.fragment.v1` converts one-to-one to a Range (§4.2), so an old node can undo through `scrivi_range_cut` / `scrivi_range_paste` with no reload. Keeping the reload leaves one reload path alive. |
| **Q3** | **Where the delta is applied on Apple:** inside `ViewportSceneLoader` (it already owns `splitScene`, `mergeSceneIntoPredecessor`) or in a new type? | ⏳ **The loader**, beside its existing patch functions. One owner of the scene list. |

⛔ **NOT in this Sprint:** structural cut, Backspace / Delete over scenes and their undo ([SP-188]) · the background sync
verification ([SP-189]) · EP-052's write queue (W7-Q2 unruled).

## Acceptance Criteria

- [ ] AC1 — The scene list is the model of record on both platforms (§4.2a); the text view renders it; ordinary typing,
      Find and Replace, navigation and the Inspector behave as today
- [ ] AC2 — Copy across scenes builds the Range from memory: zero core calls, zero disk reads, on both
- [ ] AC3 — Paste across scenes and chapters: one `scrivi_range_paste` off the main thread, delta applied, no reload, on both;
      [I-0311]'s order right in the editor
- [ ] AC4 — Paste's undo and redo: `rangePaste` history node; undo = `scrivi_range_cut`, redo = the paste; delta applied, no
      reload; old `structuredPaste` nodes still undo (Q2)
- [ ] AC5 — Split, merge, chapter create and chapter merge: off the main thread, delta applied, no reload, on both
- [ ] AC6 — [SP-185] AC4: a structured paste's undo over the share makes no project re-open
- [ ] AC7 — Tests on both platforms: memory equals disk; zero re-opens; zero main-thread core calls; mutation-checked;
      ctest, interop and Linux smokes green
- [ ] AC8 — ✅ **Live passes (user):** Mac and rig, over the share, each operation timed by the diagnostic logs

## Progress log

### 🔵 2026-10-20 — Sprint CREATED in Planning
- User: *"complete planning for EP-053."* Issued with [SP-188]–[SP-190] via `next-id.py`; [T-0615] filed.
- Rulings the same day: each operation ships with its undo; split and merge in scope here; `rangePaste` / `rangeCut`
  history op names approved. Q1–Q3 await rulings.
