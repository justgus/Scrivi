# Active Epics

## EP-019: [Apple] Custom Undo/Redo History & Multiple Copy Buffers — RESUMED

> **Un-deferred 2026-07-24 (user request):** moved back from the backlog to Active to **lock the copy-buffer
> implementation on Apple first**, so it becomes the reference the Linux side (EP-026) mirrors rather than
> being designed twice. Resumes at **SP-056** (multiple copy buffers → AC6). The undo/redo *engine* (C++
> `HistoryService` + C ABI + Swift wrappers) is delivered & verified (AC1/AC3/AC4/AC5); remaining:
> **AC6 (copy buffers, SP-056)**, then AC2/AC7/AC8 (SP-057 verify + close).

---

> **Codebase:** `[Apple]` — the undo/redo engine (ScriviCore `HistoryService` + C ABI) is delivered &
> verified; all remaining work (AC2/AC6/AC7/AC8) is Apple-platform UI. The parallel Linux undo/redo UI
> is a separate `[Linux]` Epic.

**Status:** 🟡 Active — **HELD PENDING** its final sprint SP-057 (user decision 2026-07-27). SP-051–SP-056 ✅ all
closed: AC1 (2026-07-07), AC3+AC5 (2026-07-09), AC4+AC5-branch (2026-07-13), **AC6 (2026-07-27, SP-056 — copy
buffers)**. Remaining: **SP-057** verifies AC2/AC7/AC8, adds the history panel (T-0215) + perf fixtures (T-0216),
then closes the Epic. Held (not continued immediately) so the newly-opened **EP-029** `[Cross]` (cross-boundary
structured Cut/Copy/Paste — the gap surfaced while verifying SP-056) can be planned/tackled first, per the user.
**Goal:** Replace the broken native undo (I-0019) with a from-scratch, sentence-granular undo/redo system backed by a tree-structured, per-project, on-disk persistent history (cross-session undo with session-boundary warning; branching with primary-line selection; capacity eviction and stale-branch purge), plus vim/emacs-register-style multiple copy buffers whose pastes are history events.
**Design:** `docs/Scrivi_UndoRedo_History_and_Copy_Buffers_Design_v0_1.md` (v0.1 ✅ Approved baseline 2026-07-06 — trades ruled: T1=B, T2=A+refinements, T3=C, T4=B+D+A, T5=C, T6=A)
**Supersedes:** I-0019 (Undo/Redo have no effect — ⚪ Closed 2026-07-06, OBE/superseded by this Epic, user-approved; see `docs/Issues/Closed/Issue-closed-0019.md`. AC1 carries the requirement; delivery target SP-053. Re-open I-0019 if EP-019 is cancelled or AC1 descoped.)
**Date Created:** 2026-07-06
**Target Close Date:** TBD (7 sprints)
**Actual Close Date:** —

### Acceptance Criteria

- [x] AC1 — ⌘Z/⇧⌘Z work in the macOS manuscript editor: repeated ⌘Z walks back one history event at a time; ⇧⌘Z re-applies (**delivers the fix formerly tracked as I-0019**). ✅ **Verified live 2026-07-07** (SP-053; `Tasks/Verified/Task-verified-0204-0206.md`).
- [ ] AC2 — Events commit exactly per the design's event model (`.` `!` `?`, Return, cursor-move-with-pending-changes, paste/cut, scene switch, flush); cursor moves/newlines without text changes produce **no** event.
- [x] AC3 — History persists across quit/relaunch; undoing past the session boundary shows a warning (once per crossing) before proceeding. ✅ **Verified 2026-07-09** (SP-054; `Tasks/Verified/Task-verified-0207-0209.md`).
- [x] AC4 — Undo-then-type creates a branch; the new line becomes primary; the old branch is selectable at the fork and becomes primary when selected; abandoned text fully restorable. ✅ **Verified 2026-07-13** (SP-055; `Tasks/Verified/Task-verified-0210-0212.md`).
- [x] AC5 — History capacity configurable (per Trade T1); oldest events fall off at capacity; branches auto-purge when their branch point ages off; stale branches detectable and purgeable with user confirmation. ✅ **Verified 2026-07-09** (SP-054 — capacity config + linear eviction of the root→current path; `Tasks/Verified/Task-verified-0207-0209.md`). ✅ **Branch clauses verified 2026-07-13** (SP-055 — branch-aware auto-purge on eviction + stale-branch detection/user-confirmed purge; `Tasks/Verified/Task-verified-0210-0212.md`).
- [x] AC6 — Copy buffers: ≥ 2 buffers loadable and pasteable at multiple locations (design CONOPS §9.a); each paste is one undo step; system pasteboard unaffected; buffers persist across relaunch. ✅ **Verified live 2026-07-27** (SP-056; T-0213 + T-0214). Delivered as explicit chords **⌘1–9 copy / ⌃1–9 paste / ⌥1–9 cut** (design refinement, user-approved 2026-07-25/27 — the originally-designed single context-sensitive chord + ⌥⌘C/⌥⌘V HUD was replaced; buffer 0 = the system pasteboard). Palette (app-global, per-project reload), Edit-menu + Scene/Chapter menu items, cut-into-buffer `cut` event tagged with bufferID (backend schema extended).
- [ ] AC7 — Structural operations record barriers; undo stops at a barrier with a clear notice; no text corruption.
- [ ] AC8 — No regression: auto-save, scene navigation, structure ops, external-change scan, Git snapshots unchanged; backend `ctest` + interop suites green.

### Sprints

| Sprint | Title | Status | Dates |
| ------ | ----- | ------ | ----- |
| SP-051 | Design sign-off, ⌘Z-routing spike, schema spec | ✅ Closed (user-approved) | 2026-07-06 – 2026-07-06 |
| SP-052 | Linear history engine core (C++) + C ABI + Swift wrappers | ✅ Closed (user-approved) | 2026-07-07 – 2026-07-07 |
| SP-053 | In-session undo/redo on macOS (capture, apply, barriers) — AC1 (ex-I-0019) delivered here | ✅ Closed (user-approved) | 2026-07-07 – 2026-07-07 |
| SP-054 | Persistence, sessions, capacity, settings | ✅ Closed (user-approved) — AC3 + AC5 | 2026-07-07 – 2026-07-09 |
| SP-055 | Branching — tree ops, fork popover, purge | ✅ Closed (user-approved) — AC4 + AC5 branch clauses | 2026-07-10 – 2026-07-13 |
| SP-056 | Multiple copy buffers | ✅ Closed (user-approved) — **AC6** | 2026-07-24 – 2026-07-27 |
| SP-057 | History panel, performance fixtures, verification & Epic close | 🔵 Planning (**EP-019 held pending this sprint**) | — |

### Tasks

| ID | Title | Sprint | Status |
| -- | ----- | ------ | ------ |
| T-0198 | Design doc + trade studies (this Epic's design) | SP-051 | ✅ Verified (2026-07-06) → `Verified/Task-verified-0198.md` |
| T-0199 | Spike: ⌘Z/⇧⌘Z + Edit-menu routing mechanism | SP-051 | ✅ Done (2026-07-06) — proxy rejected; action-method mechanism confirmed live; design §8/§12.6 updated |
| T-0200 | `scrivi.history.v1` / `scrivi.buffers.v1` schema spec + repair-matrix row | SP-051 | ✅ Verified (2026-07-06) → `Verified/Task-verified-0200.md` |
| T-0201 | `HistoryService` core (record/undo/redo, snapshot-diff, sessions) + unit tests | SP-052 | ✅ Verified (2026-07-07) → `Verified/Task-verified-0201-0203.md` |
| T-0202 | C ABI: `scrivi_history_open/record_event/record_barrier/undo/redo/close` | SP-052 | ✅ Verified (2026-07-07) → `Verified/Task-verified-0201-0203.md` |
| T-0203 | `ScriviEngine.swift` history wrappers + interop tests | SP-052 | ✅ Verified (2026-07-07) → `Verified/Task-verified-0201-0203.md` |
| T-0204 | `HistoryCapture` + commit-trigger wiring in the editor | SP-053 | ✅ Verified (2026-07-07) → `Verified/Task-verified-0204-0206.md` |
| T-0205 | Undo/redo apply path + `allowsUndo=false` + ⌘Z routing | SP-053 | ✅ Verified (2026-07-07) → `Verified/Task-verified-0204-0206.md` |
| T-0206 | Barriers on structural operations | SP-053 | ✅ Verified (2026-07-07) → `Verified/Task-verified-0204-0206.md` |
| T-0207 | JSONL log + checkpoint + torn-line recovery + head-hash validation | SP-054 | ✅ Verified (2026-07-09) |
| T-0208 | Capacity/eviction + history settings (T1) + Project Settings row | SP-054 | ✅ Verified (2026-07-09; linear eviction, full in SP-055) |
| T-0209 | Session-boundary warning popup | SP-054 | ✅ Verified (2026-07-09) |
| T-0210 | Tree ops: branching, primary-child, `select_branch`, auto-purge on eviction | SP-055 | ✅ Verified (2026-07-13) → `Verified/Task-verified-0210-0212.md` |
| T-0211 | Inline fork popover (T2 core interaction) | SP-055 | ✅ Verified (2026-07-13) → `Verified/Task-verified-0210-0212.md` |
| T-0212 | Stale-branch detection + user-confirmed purge | SP-055 | ✅ Verified (2026-07-13) → `Verified/Task-verified-0210-0212.md` |
| T-0213 | Copy-buffer store (`buffers.json`) + C ABI + engine wrappers | SP-056 | ✅ **Verified (2026-07-27)** |
| T-0214 | Buffer UX: palette + Edit/Scene/Chapter menu items + explicit ⌘/⌃/⌥1–9 chords; paste/cut history integration (cut tagged w/ bufferID) | SP-056 | ✅ **Verified (2026-07-27)** |
| T-0215 | History panel (T2 management surface) | SP-057 | 🔵 Backlog |
| T-0216 | Performance/integration fixtures (100k events, 500 KB scene); gitignore migration | SP-057 | 🔵 Backlog |
| T-0217 | Doc updates + EP-019 acceptance-criteria verification | SP-057 | 🔵 Backlog |

### Issues

| ID | Title | Status |
| -- | ----- | ------ |
| I-0019 | Undo and Redo have no effect in the manuscript editor | ⚪ Closed 2026-07-06 — OBE/superseded by this Epic (user-approved); requirement lives on as AC1 → `docs/Issues/Closed/Issue-closed-0019.md` |

### Scope Notes

- Structural undo (scene/chapter create/delete/merge/reorder) is explicitly **out of scope** — those operations record history *barriers* in v1 (design §4.5); full structural undo is a documented future extension.
- iOS/iPadOS capture layer deferred (editor is a stub); the C++ engine and ABI are platform-neutral by construction.
- All six trade-study decisions **approved by the user 2026-07-06**: T1=B (capacity in Project
  settings), T2=A with interaction refinements (popover on undo-landing-at-fork, suppressed when
  undoing past, immediate on redo, redo-past takes primary, branch relegation — design §10 T2, §5),
  T3=C (copy-into-buffer not an event; cut is), T4=B+D with A for discoverability, T5=C
  (project-open session + idle rollover), T6=A (history inside package, gitignored).

### Completion Summary

_(filled in when the Epic reaches 🟠 Complete)_


---

*Last Updated: 2026-08-03 (**EP-029 ✅ CLOSED (Human-approved) — cross-boundary structured Cut/Copy/Paste
complete.** Final sprint **SP-089** delivered all four `[Apple]` tasks: T-0354 (Pass A clipboard, Verified
2026-07-29), T-0357 (title-capture + chapter promotion, Verified 2026-07-29), T-0355 (Pass B structured buffers,
Verified 2026-08-03), T-0356 (AC6 reversible structured undo, Verified 2026-08-03). AC1–AC7 all met; the AC6 undo
runs **app-side** (`HistoryService` stays in-memory; new core endpoint `scrivi_fragment_uncut_paste` is the exact
inverse of paste). ScriviCore `ctest` **369/369**, macOS interop **45/45**, app **BUILD SUCCEEDED**. **EP-029
archived to `Closed/Epic-EP-029.md`; SP-089 archived to `Sprints/Closed/Sprint-SP-089.md`.** Active Epics: EP-019
(held pending SP-057). Prior note follows.)*

*2026-07-27 (**SP-085 ✅ closed (Human-approved) — EP-029 design doc APPROVED.** The EP-029
`[Cross]` design sprint delivered + got approval for `docs/Scrivi_Structured_CutCopyPaste_Design_v0_1.md`: the
`scrivi.fragment.v1` ordered-pieces schema, extract / paste-splice / cut-merge behaviour (composing EP-027
create/split + EP-028 `SceneMerger`/`ChapterMerger`), buffer-schema evolution (extend `scrivi.buffers.v1` in
place), and a one-reversible-event history shape (`structuredCut`/`structuredPaste`, undo = inverse op). Trades
ruled **T1=A · T2=A · T3=A · T4=A**; Open Questions #1–#3 resolved — no cross-window/cross-project structured
paste in v1; **caret-in-heading paste = refuse + flash the screen** (user override of the drafted silent-retarget);
divider-anchored selection normalised to the adjacent scene body. Task T-0350 Verified; docs-only. **EP-029 → SP-086**
(ScriviCore extract-fragment) next, awaiting go-ahead. Epics Active 1 (EP-029) + held/draft (EP-019 held, EP-026
Linux draft). Next available Sprint **SP-086**, Task **T-0351**. Prior note follows.)*

*2026-07-27 (**SP-056 ✅ closed (Human-approved) — AC6 Verified; EP-019 held pending SP-057; new
Epic EP-029 opened.** T-0213 + T-0214 both ✅ Verified live: multiple copy buffers delivered as explicit
⌘1–9/⌃1–9/⌥1–9 (copy/paste/cut) chords + app-global per-project palette + Edit/Scene/Chapter menu items +
bufferID-tagged cut event (backend schema extended); ctest 327 + interop 43 green. AC6 met. EP-019 → **held
pending** its final sprint SP-057 (AC2/AC7/AC8 verify + history panel + close). **EP-029** `[Cross]`
(cross-boundary structured Cut/Copy/Paste) opened to capture the gap surfaced during SP-056 verification —
manuscript-as-monolithic-document copy/cut/paste with structured buffers, cut-that-merges, paste-that-splits;
5 sprints planned (SP-085 design → SP-086 extract → SP-087 paste-splice → SP-088 cut-merge → SP-089 Apple wiring);
design doc to be written & approved first (T-0350). Epics Active 1 (EP-019, held) + Draft 2 (EP-026 Linux, EP-029);
next available Epic EP-030, Sprint SP-085, Task T-0351. Prior note follows.)*

---

*Last Updated: 2026-07-24 (**EP-025 ✅ CLOSED (Human-approved) — the final sprint SP-084 delivered AC6b and
closed the Epic.** SP-084 (co-located dot **clustering** — aggregate dot: larger core + count + segmented arc
ring + selection arc; hover fan-out; zoom-resolve) + a persistence sweep + full EP-025 verify; T-0346–T-0349 all
Verified live over VNC. Four clustering findings surfaced + fixed + re-verified same session (larger-diameter
aggregate-of-aggregates; fan overlay + grey backing; tighter dismiss; the **phantom double-draw** defect — a
fanned aggregate's members drew twice because the skip-set excluded the fanned aggregate; fixed so all members
are skipped from the baseline loops and the ring is the sole draw). Final container build **green (216/216)** +
new `timeline_cluster_smoke` + 14 regression smokes + app-launch PASS; `scrivi.h` untouched; no pbxproj
(Linux-only). **EP-025 archived to `Closed/Epic-EP-025.md`; Active Epics 1→0.** The full six-sprint completion
summary lives in the archive. Prior note follows.)*

*2026-07-24 (**EP-025 SP-082 ✅ closed (Human-approved) + SP-084 planned + activated** — the final sprint of
EP-025. SP-082 delivered **AC5** (historical events + imported timelines + export + File-menu items,
T-0340–T-0345 all Verified; I-0090/I-0091 fixed + re-verified; container green 211/211 + `timeline_events_smoke`
+ 11 smokes PASS; archived `Closed/Sprint-SP-082.md`). SP-084 then delivered **AC6b** clustering (above) and
closed the Epic. Prior EP-025 history — SP-079 [AC1/AC2], SP-080 [AC3], SP-081 [AC4], SP-083 [AC6a] — lives in
`Closed/Epic-EP-025.md`.)*
