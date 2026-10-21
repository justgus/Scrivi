---
sprint: SP-191
epic: EP-053
status: Planning
platform: Cross
created: 2026-10-20
---

# SP-191 — `[Apple]` + `[Linux]` [EP-053] **S2b**: copy, paste and its undo, split and merge — as deltas, off the main thread, no reload

**Status:** 🔵 **PLANNING** — created 2026-10-20 by [SP-187]'s Q1 ruling (user: split S2). ✅ Its questions were ruled in
[SP-187] (Q2: old history nodes convert; Q3: `ViewportSceneLoader` applies the delta).
**Carries:** [T-0619] → [`../Tasks/Task-backlog.md`](../Tasks/Task-backlog.md) · [SP-185] AC4 (user, 2026-10-20: *"AC4 goes
to EP-053 S2"*) · [I-0297]'s reload half · [I-0311] (the apps reach `scrivi_range_paste`).
**Epic:** [EP-053] → [`../Epics/Epic-active.md`](../Epics/Epic-active.md).
**Builds on:** [SP-186] (S1, the endpoints) · [SP-187] (S2a: the list as the model of record, the bindings, the delta types,
the partial re-render, the baseline counts).
**Authority:** [`../Scrivi_Structured_CutCopyPaste_Design_v0_2.md`](../Scrivi_Structured_CutCopyPaste_Design_v0_2.md) — R1,
R3, R5, R7, P3, P5, §4.2b, §4.4, §4.5, Q5.
**Size:** L.

---

## What reading the code found (2026-10-20)

**Apple**
- **Paste** (`ManuscriptTextView.swift:2696`): `fragmentPaste` on the **main thread**, then `reloadManuscriptFromDisk`
  (`:2407`), which re-opens the project and re-renders everything.
- **Paste's undo and redo** (`applyStructuralInverse`, `:615`): `fragmentPaste` or `fragmentUncutPaste` on the main thread,
  then the same reload (`:639`, `:656`).
- **Copy** (`structuredCopyIfCrossBoundary`, `:2471`): `fragmentExtract` reads the scenes from disk.
- **Merge** (`handleMergeScene`, `:1446`): `mergeScene` on the main thread inside a `@MainActor` task.

**Linux**
- **Paste** (`EditorShell.cpp:2544`): `fragmentPaste` on a worker (`runStructural`, `:2919`), then `reloadThen` (`:2979`):
  a full `load()`, ~1,700 file calls over the share even with SP-185's in-memory texts.
- **Paste's undo and redo** (`applyStructuralInverse`, `:2573`): the same, after `saveDirtyScenes()` on the GUI thread.
- **Merge** (`:2192`): `mergeScene` on the GUI thread (`:2224`), then `reloadThen` (`:2240`). **Merge chapter** reloads
  (`:2321`). **Split** (`:1960`–`:2010`): `createScene` and two saves on the GUI thread, then a reload. **Chapter create**
  (`:2038`) reloads (`:2179`).

## Goal

✅ On the Mac and on Linux: copy builds the Range from memory; paste, paste's undo and redo, split, merge, chapter create
and chapter merge each make **one core call off the main thread**, and the editor applies the returned delta through
[SP-187]'s partial re-render. **Nothing reloads the manuscript.**

## Plan

1. **Copy** builds the Range from the list (§4.2b): no core call, no disk read. The clipboard and copy buffers take the Range
   (`scrivi.fragment.v1` stays readable).
2. **Paste** → `rangePaste` off the main thread on both (Apple: a detached task; Linux: `runStructural`), read-only with the
   progress row after 400 ms (Q5). The delta is applied; no reload. Recorded as a `rangePaste` history node (✅ op name
   approved 2026-10-20).
3. **Paste's undo = `rangeCut`** of the Range the paste inserted; redo = the paste again. ✅ Old `structuredPaste` /
   `structuredCut` nodes convert their fragment to a Range and take the same path ([SP-187] Q2).
4. **Split, merge, chapter create, chapter merge:** off the main thread, delta applied, no reload, on both. If an existing
   endpoint does not return enough to build the delta, it is extended in ScriviCore here, with boundary tests.
5. **Retire** Apple's `reloadManuscriptFromDisk`, Linux's structural `reloadThen` and SP-185's `reloadTexts_` (design §4.6).
6. **Tests** on both, against [SP-187]'s baseline: the list equals the disk afterwards; zero re-opens; zero main-thread core
   calls; the caret where the operation leaves it. Mutation-checked.
7. **Live passes (user):** Mac and rig, over the share, each operation timed by the diagnostic logs.

⛔ **NOT in this Sprint:** structural cut and Backspace / Delete over scenes ([SP-188]) · the background sync verification
([SP-189]) · EP-052's write queue (W7-Q2 unruled).

## Acceptance Criteria

- [ ] AC1 — Copy across scenes builds the Range from memory: zero core calls, zero disk reads, on both
- [ ] AC2 — Paste across scenes and chapters: one `rangePaste` off the main thread, delta applied, no reload, on both;
      [I-0311]'s order right in the editor
- [ ] AC3 — Paste's undo and redo: a `rangePaste` node; undo = `rangeCut`, redo = the paste; delta applied, no reload; old
      `structuredPaste` / `structuredCut` nodes still undo, through the same path
- [ ] AC4 — Split, merge, chapter create and chapter merge: off the main thread, delta applied, no reload, on both
- [ ] AC5 — [SP-185] AC4: a structured paste's undo over the share makes no project re-open
- [ ] AC6 — Tests on both: memory equals disk; zero re-opens; zero main-thread core calls; mutation-checked; ctest, interop
      and Linux smokes green
- [ ] AC7 — ✅ **Live passes (user):** Mac and rig, over the share, each operation timed

## Progress log

### 🔵 2026-10-20 — Sprint CREATED in Planning
- From [SP-187]'s Q1 ruling (user: split S2). Issued via `next-id.py`; [T-0619] filed. Steps 3–8 of the original S2 plan
  and its AC2–AC8 moved here.
