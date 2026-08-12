# Closed Epic: EP-019

## EP-019: [Apple] Custom Undo/Redo History & Multiple Copy Buffers

**Codebase:** `[Apple]` UI over a `[ScriviCore]` engine — the undo/redo engine (C++ `HistoryService` + C ABI)
is platform-neutral; the capture layer, menus, and history card are Apple-platform. The parallel Linux
undo/redo UI is a separate Epic (EP-026).

**Status:** ✅ **CLOSED (Human-approved 2026-08-11).** All acceptance criteria **AC1–AC8 Verified** across
7 sprints (SP-051–SP-057 → SP-093/SP-094). ctest **413/413** · macOS interop **56/56** · TEST SUCCEEDED.
Delivers the fix formerly tracked as **I-0019**.

**Goal:** Replace the broken native undo (I-0019) with a from-scratch, sentence-granular undo/redo system
backed by a tree-structured, per-project, on-disk persistent history (cross-session undo with
session-boundary warning; branching with primary-line selection; capacity eviction and stale-branch purge),
plus vim/emacs-register-style multiple copy buffers whose pastes are history events.

**Design:** `docs/Scrivi_UndoRedo_History_and_Copy_Buffers_Design_v0_1.md` (v0.1 ✅ Approved baseline
2026-07-06 — trades ruled: T1=B, T2=A+refinements, T3=C, T4=B+D+A, T5=C, T6=A; **amended 2026-08-11**, see
below).

**Supersedes:** I-0019 (Undo/Redo have no effect — ⚪ Closed 2026-07-06 as OBE/superseded by this Epic;
`docs/Issues/Closed/Issue-closed-0019.md`). AC1 carried the requirement and is Verified, so I-0019 stays
closed.

**Date Created:** 2026-07-06
**Actual Close Date:** 2026-08-11

---

## Acceptance Criteria — all Verified

| AC | Criterion | Verified |
| -- | --------- | -------- |
| AC1 | ⌘Z/⇧⌘Z work in the macOS manuscript editor; repeated ⌘Z walks back event by event; ⇧⌘Z re-applies | ✅ 2026-07-07 (SP-053) |
| AC2 | Events commit exactly per the event model **(amended — see below)**; cursor moves/newlines without text changes produce no event, **and neither does an auto-save** | ✅ 2026-08-11 (SP-094) |
| AC3 | History persists across quit/relaunch; undoing past a session boundary warns once per crossing | ✅ 2026-07-09 (SP-054) |
| AC4 | Undo-then-type branches; new line primary; old branch selectable at the fork; abandoned text restorable | ✅ 2026-07-13 (SP-055) |
| AC5 | Capacity configurable; oldest events fall off; branches auto-purge; stale branches detectable/purgeable | ✅ 2026-07-09 + 2026-07-13 |
| AC6 | Copy buffers: ≥2 loadable and pasteable; each paste one undo step; system pasteboard unaffected; persist across relaunch | ✅ 2026-07-27 (SP-056) |
| AC7 | Structural operations record barriers; undo stops with a clear notice; no text corruption | ✅ 2026-08-11 (SP-094) |
| AC8 | No regression: auto-save, navigation, structure ops, external-change scan, Git snapshots; suites green | ✅ 2026-08-11 (SP-094) |

**AC6 delivery note:** shipped as explicit chords **⌘1–9 copy / ⌃1–9 paste / ⌥1–9 cut** (design refinement,
user-approved 2026-07-25/27), replacing the originally-designed single context-sensitive chord + ⌥⌘C/⌥⌘V HUD.
Buffer 0 = the system pasteboard.

---

## ⚠️ AC2 was amended and §4.d was relaxed — read this before trusting older drafts

**AC2's original wording listed "flush" as a commit trigger and omitted any idle boundary.** That made the 1 s
autosave debounce a silent commit trigger: a ~1 s typing pause sealed a history entry, fragmenting
continuously-typed prose. The reference case — *"Now is the winter of our discontent made glorious summer by
this son of york"*, typed in one unbroken run — recorded as **three** entries, one break falling **mid-word**
(`"…made glo"` / `"rious…"`), which no intentional trigger can produce.

**Amended (T-0396 implementation, T-0217 documentation; user-ruled 2026-08-10, approved 2026-08-11):** the
save-time commit is **retired**; a **45 s idle-session boundary** is the real session end. Every trigger the
original AC named is **kept** — sentence terminators, Return, cursor-move-with-pending-changes, paste/cut,
scene switch — plus project close. Backspace does not split an entry.

**§4.d — a changed invariant, not a documentation gap.** The design previously guaranteed *"disk never
contains text no history node describes."* It now reads: **disk may lead history by at most one open typing
session's text.** The strict form is precisely what forced the save to be a commit trigger, so it was given up
deliberately.

- Bounded: `close()` commits pending text, so a clean quit loses nothing.
- The only exposed window is a **hard crash mid-session**, where the head-hash check (I-0104) raises an
  `externalChange` barrier so undo stops at the last node it can honestly describe.
- Failure mode is **"undo stops early"**, never **"undo corrupts the manuscript."**

**Why record-and-reopen was rejected:** `HistoryService::record` always appends a node and diffs against the
current head (`HistoryService.cpp:204-206`) — there is no amend/extend path — so a save that records anything
necessarily creates a node, and undo walks one node per step. The writer would have seen one row but three
⌘Z stops. Coalescing is **app-side**; `HistoryService` stays pure.

**Documented in:** design §4.a/§4.a.1, §4.d, §12.2, §12.8, §14, §15 AC2 · package-structure §16a ·
repair-matrix §6.21.

---

## Sprints

| Sprint | Title | Closed |
| ------ | ----- | ------ |
| SP-051 | Design + schema (T-0198–T-0200) | ✅ 2026-07-06 |
| SP-052 | `HistoryService` core + tree model | ✅ 2026-07-07 |
| SP-053 | Capture layer + ⌘Z/⇧⌘Z (AC1) | ✅ 2026-07-07 |
| SP-054 | Persistence + capacity (AC3, AC5) | ✅ 2026-07-09 |
| SP-055 | Branching + primary-line selection (AC4) | ✅ 2026-07-13 |
| SP-056 | Multiple copy buffers (AC6) | ✅ 2026-07-27 |
| SP-057 | Verification & close | ⚪ Superseded → merged into SP-094 (2026-08-07) |
| SP-093 | `[Cross]` History capture granularity + presentation (11 items) | ✅ 2026-08-11 |
| SP-094 | EP-019 + EP-030 verification & Epic close (merged) | ✅ 2026-08-11 |

---

## Known gaps carried past close — deliberate, recorded

1. **No large-scale performance coverage.** T-0216's fixtures (100k-event history, 500 KB single scene) were
   never built; the task was closed **whole** as OBE 2026-08-05 (user-directed), which dropped them.
   `HistoryService` is tested at ordinary sizes only. **If history performance is ever suspect at scale, this
   is the missing evidence — open a new task rather than reviving T-0216.**

2. **`HistoryCapture` is not compiled into the test target**, so T-0396's timing logic — the 45 s idle
   boundary, the deferred save, and backspace-does-not-split — has **no automated coverage**. AC2 items 5–8
   rest entirely on live user verification (2026-08-10/11). A regression here would not be caught by CI.

3. **History log-segment rotation is not implemented.** `activeSegment_` is hard-fixed to `log-000001.jsonl`
   (`HistoryStore.hpp:114`); capacity/eviction bounds the **tree**, not the **log**, so a segment grows
   unbounded (a real project is already ~3.4 MB). The reader already honours `activeLogSegment` from
   `state.json`, so rotation is additive with no format change. Tracked as **T-0400, 🟢 nice-to-have**
   (user ruling 2026-08-11).

---

## The lesson worth carrying forward

**Five of SP-093's eleven items were found by live verification, not by CI** — and two were severe: I-0110
stopped history opening at all (the project ran with no undo/redo), and I-0104's real cause survived four
wrong hypotheses and two shipped-then-reverted fixes, one of which made the symptom measurably worse.

**Three separate times a test passed while testing nothing.** The I-0107 rebase tests only ever inserted at
offset 0 — always "before", the one shape the naive rule gets right. The I-0104 tests all called
`scrivi_history_close`, which *was* the missing fix, so they could not fail. The first I-0111 test left its
bogus node unparented, so the prune never reached it. **Each suite exercised the shape its author already had
in mind**, and green tests certified a broken rule twice.

What resolved it was an instrumented probe driving the real C ABI against a **copy of the writer's actual
project** — it answered in one run what four rounds of reasoning had not. **Reach for real-data
instrumentation earlier, especially when a hypothesis needs a test that cannot fail.**
