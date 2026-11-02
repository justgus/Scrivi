---
sprint: SP-201
epic: EP-052
status: Active
platform: Cross
created: 2026-11-01
---

# SP-201 — `[Cross]` [EP-052] Replace All without holding the window, and no main-thread lock waits

**Status:** 🟡 **ACTIVE 2026-11-01** (user, 2026-11-01: *"activate and implement SP-201 please."*) — created 2026-11-01
(user: *"Create a sprint for I-0333 and I-0334"*). ⏳ Q1–Q2 below, after step 1's profile.
**Carries:** [I-0333], [I-0334] → [`../Issues/Issue-backlog.md`](../Issues/Issue-backlog.md).
**Epic:** [EP-052] (AC4 *"no project write on the main thread"*, AC6, AC8 *"a click, select or scroll makes no file
operation on the main thread"*) → [`../Epics/Epic-active.md`](../Epics/Epic-active.md).
**Found in:** [SP-200]'s Mac live pass log, 2026-11-01.
**Size:** M (one profile decides it).

---

## Where it starts (2026-11-01, Mac, Debug)

| Project | Scenes changed | Window held | `apply` |
|---|---|---|---|
| dumas-prose-timelines | 1,174 | ~6 s | — |
| dumas-prose-timelines-history (generated, 25,000 steps) | 1,174 | 18.6 s | 13,427 ms |

Then main-thread core calls waited up to 15 s for the project lock (`lockWaitMs`) while the 1,174 queued saves drained.

## What reading the code found (2026-11-01)

- **Mac** `Coordinator.replaceAll` (`ManuscriptTextView.swift:2168`) logs `[SCRIVI-FIND] replaceAll … apply= … history=`.
  `apply` is `ManuscriptNSTextView.applyReplacements` (`:2471`) alone: inside ONE `beginEditing`/`endEditing`, every edit
  calls `MarkdownEscapes.snapSelection` with `ManuscriptEditRules.runLookup`, then `presenter.balancedEdit`. ⚠️ If either
  looks beyond the edit's own paragraph, the cost is edits × manuscript — unmeasured, a hypothesis for step 1 only.
- The saves already leave the main thread (`loader.saveScenesOffMain`, [SP-195]). So I-0334 is the OTHER side: some
  main-thread call takes the lock the writer thread holds. Which calls, and whether each needs the lock, is unknown.
- ⚠️ Not yet known why the generated project is ~3× slower on the same manuscript (the edit count may differ: its texts
  were changed by 25,000 generated steps).
- **Linux** has its own Replace All ([SP-185]; [EP-052] AC7 adds its progress) — not measured here.

## Plan

1. **Measure, no fix yet.** An Instruments time profile of one Replace All (`the` → `thee`) on both projects on the Mac,
   and each main-thread call that waited, by name (⚠️ corrected 2026-11-01: the log ALREADY names them —
   `[SCRIVI-SPAN] [main] core: <endpoint> … lock wait N ms`; nothing had to be added). Record the edit counts. The same
   Replace All timed on Linux in Docker over the SMB share (5 ms netem), so the Linux shape is decided with the data.
2. **Rule Q1 and Q2** from what step 1 names.
3. **[I-0333]** the fix Q1 rules — on both platforms in the same work if the shape changes (Apple is the reference).
4. **[I-0334]** the fix Q2 rules.
5. **Tests:** a boundary or interop test that fails before each fix (a count of lookups, or of main-thread lock waits —
   not a timing), and a mutation in a copy of the repo that it catches.
6. **Live passes:** the Mac, then the rig (steps added to [`Rig-Live-Pass-Held.md`](Rig-Live-Pass-Held.md)).

## Questions (ruled after step 1)

| # | Question | Recommendation |
|---|----------|----------------|
| **Q1** | [I-0333]: what does Replace All change? | ⏳ After the profile. If the per-edit lookup is the cost: compute it once per paragraph (or per scene) instead of per edit, keeping one undo step. If AppKit's layout of a whole-manuscript edit is the cost: apply in batches with the progress row the writer already sees. |
| **Q2** | [I-0334]: what stops a main-thread call waiting on the lock? | ⏳ After step 1 names the calls. Memory-only calls take the memory lock only ([EP-052] AC0); a call that needs the disk moves off the main thread as [SP-197] did for reads. |

## Acceptance Criteria

- [x] AC1 — Step 1's measurements recorded here: where `apply` goes, the edit counts, each main-thread call that waited
  (Mac; ⚠️ the generated project and Linux not measured — see below)
- [x] AC2 — Q1 and Q2 ruled as implemented (user, 2026-11-01: *"your changes are confirmed."*)
- [x] AC3 — [I-0333] Replace All's long work runs off the main thread with *"Replacing N of M…"* in the row; undo is one
  step and reads as before (tests) — ⏳ dumas on the Mac and the rig
- [x] AC4 — [I-0334] the two calls that waited no longer run on the main thread (test) — ⏳ live passes
- [x] AC5 — tests that fail before the fixes (mutations in a copy of the repo); ctest 800/800, Mac interop 333/333,
  iOS 17/17, Linux smokes 38/38
- [ ] AC6 — Mac live pass; rig live pass (held in [`Rig-Live-Pass-Held.md`](Rig-Live-Pass-Held.md))

## Progress log

### ✅ 2026-11-01 — AC2: Q1 and Q2 RULED as implemented
- User: *"your changes are confirmed. I didn't rule because you stated that the rulings would be based on data you had not
  collected yet."* Q1: both branches (the core sent only the touched blocks; computed off the main thread, applied in slices
  with the count). Q2: the two calls that waited moved off the main thread.

### 🟠 2026-11-01 — I-0333, I-0334 Resolved - Not Verified (AC1, AC3–AC5)
**Step 1, measured** (a temporary interop test on a 1.76 MB manuscript of 110 scenes, 84,480 matches of `the`; dumas's own
1,175 scene texts for the core call; the Mac's log for the waits):
| Part of Replace All (Mac, main thread) | 110 scenes, 84,480 matches | dumas, 41,296 matches |
|---|---|---|
| `scrivi_markdown_balanced_edit` per match, sent its WHOLE scene | 37.2 s | 1.6 s |
| the snap per match (its block analysed again each time) | 2.9 s | — |
| finding the scene per match | 0.6 s | — |
| `apply` in all | 42 s | 4.1 s (the window held 6 s) |
- **I-0334's calls** (the log): `scrivi_list_worlds` from `reconnectWorlds` → `activateWorlds` when the writer came back to
  Scrivi (36 waits, up to 16.4 s), and `scrivi_put_inspector_layout` (0.5 s). No other main-thread call waited.
- **Done (Q1, Q2 as recommended, both branches of Q1):** the core is sent only the blocks an edit touches (both platforms'
  one wrapper, so every caller gains); the Mac's Replace All takes Linux's shape — snap and group on the main thread, each
  scene's text computed off it, the scenes put in in 50 ms slices with the count; the world read and the layout write left
  the main thread (Linux: the layout write; it read worlds off the GUI thread already). Details in [I-0333], [I-0334].
- **After** (110 scenes, 84,480 matches): main thread 573 ms + 109 ms + 1,248 ms (the history record), the window
  answering between; 2.4 s off it. Before: 42 s in one block. Putting each scene in as its own storage edit cost 10.5 s
  (the presented map rebuilt per edit) — one storage edit per slice took it to 109 ms.
- ⚠️ **Not measured:** dumas end to end and the generated history's ~3× (the harness could not build 1,175 scenes — next
  point); Linux's Replace All times. Both are the live passes'.
- ⚠️ **Found, not this Sprint's:** creating ~118 scenes one after another at the end of ONE chapter fails — each order key
  grows by `.Y` until the scene's file name passes the filesystem's limit (*"could not open temp file for writing"*,
  `…/chapter-001/Y.Y.Y.…H-scene.meta.json.tmp`). `OrderKey.hpp` calls key growth expected and its rebalance manual. Not filed.

### 🟡 2026-11-01 — Sprint ACTIVATED (user, 2026-11-01: *"activate and implement SP-201 please."*)

### 🔵 2026-11-01 — Sprint CREATED in Planning
- User: *"Create a sprint for I-0333 and I-0334"*. Both Issues stay in the backlog until activation.

*Last Updated: 2026-11-01 (I-0333, I-0334 Resolved - Not Verified.)*
