# Active Epics

## EP-019: Custom Undo/Redo History & Multiple Copy Buffers

**Status:** 🟡 Active (design ✅ approved 2026-07-06; SP-051/SP-052/SP-053 ✅ closed — **AC1 delivered & verified** 2026-07-07. Next: SP-054, Planning)
**Goal:** Replace the broken native undo (I-0019) with a from-scratch, sentence-granular undo/redo system backed by a tree-structured, per-project, on-disk persistent history (cross-session undo with session-boundary warning; branching with primary-line selection; capacity eviction and stale-branch purge), plus vim/emacs-register-style multiple copy buffers whose pastes are history events.
**Design:** `docs/Scrivi_UndoRedo_History_and_Copy_Buffers_Design_v0_1.md` (v0.1 ✅ Approved baseline 2026-07-06 — trades ruled: T1=B, T2=A+refinements, T3=C, T4=B+D+A, T5=C, T6=A)
**Supersedes:** I-0019 (Undo/Redo have no effect — ⚪ Closed 2026-07-06, OBE/superseded by this Epic, user-approved; see `docs/Issues/Closed/Issue-closed-0019.md`. AC1 carries the requirement; delivery target SP-053. Re-open I-0019 if EP-019 is cancelled or AC1 descoped.)
**Date Created:** 2026-07-06
**Target Close Date:** TBD (7 sprints)
**Actual Close Date:** —

### Acceptance Criteria

- [x] AC1 — ⌘Z/⇧⌘Z work in the macOS manuscript editor: repeated ⌘Z walks back one history event at a time; ⇧⌘Z re-applies (**delivers the fix formerly tracked as I-0019**). ✅ **Verified live 2026-07-07** (SP-053; `Tasks/Verified/Task-verified-0204-0206.md`).
- [ ] AC2 — Events commit exactly per the design's event model (`.` `!` `?`, Return, cursor-move-with-pending-changes, paste/cut, scene switch, flush); cursor moves/newlines without text changes produce **no** event.
- [ ] AC3 — History persists across quit/relaunch; undoing past the session boundary shows a warning (once per crossing) before proceeding.
- [ ] AC4 — Undo-then-type creates a branch; the new line becomes primary; the old branch is selectable at the fork and becomes primary when selected; abandoned text fully restorable.
- [ ] AC5 — History capacity configurable (per Trade T1); oldest events fall off at capacity; branches auto-purge when their branch point ages off; stale branches detectable and purgeable with user confirmation.
- [ ] AC6 — Copy buffers: ≥ 2 buffers loadable and pasteable at multiple locations (design CONOPS §9.a); each paste is one undo step; system pasteboard unaffected; buffers persist across relaunch.
- [ ] AC7 — Structural operations record barriers; undo stops at a barrier with a clear notice; no text corruption.
- [ ] AC8 — No regression: auto-save, scene navigation, structure ops, external-change scan, Git snapshots unchanged; backend `ctest` + interop suites green.

### Sprints

| Sprint | Title | Status | Dates |
| ------ | ----- | ------ | ----- |
| SP-051 | Design sign-off, ⌘Z-routing spike, schema spec | ✅ Closed (user-approved) | 2026-07-06 – 2026-07-06 |
| SP-052 | Linear history engine core (C++) + C ABI + Swift wrappers | ✅ Closed (user-approved) | 2026-07-07 – 2026-07-07 |
| SP-053 | In-session undo/redo on macOS (capture, apply, barriers) — AC1 (ex-I-0019) delivered here | ✅ Closed (user-approved) | 2026-07-07 – 2026-07-07 |
| SP-054 | Persistence, sessions, capacity, settings | 🔵 Planning | — |
| SP-055 | Branching — tree ops, fork popover, purge | 🔵 Planning | — |
| SP-056 | Multiple copy buffers | 🔵 Planning | — |
| SP-057 | History panel, performance fixtures, verification & Epic close | 🔵 Planning | — |

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
| T-0207 | JSONL log + checkpoint + torn-line recovery + head-hash validation | SP-054 | 🔵 Backlog |
| T-0208 | Capacity/eviction + history settings (T1) + Project Settings row | SP-054 | 🔵 Backlog |
| T-0209 | Session-boundary warning popup | SP-054 | 🔵 Backlog |
| T-0210 | Tree ops: branching, primary-child, `select_branch`, auto-purge on eviction | SP-055 | 🔵 Backlog |
| T-0211 | Inline fork popover (T2 core interaction) | SP-055 | 🔵 Backlog |
| T-0212 | Stale-branch detection + user-confirmed purge | SP-055 | 🔵 Backlog |
| T-0213 | Copy-buffer store (`buffers.json`) + C ABI + engine wrappers | SP-056 | 🔵 Backlog |
| T-0214 | Buffer UX: keyboard HUD + palette + Edit-menu items (T4); paste/cut history integration | SP-056 | 🔵 Backlog |
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

*Last Updated: 2026-07-07 (SP-053 closed with user approval — T-0204/T-0205/T-0206 Verified & archived to `Verified/Task-verified-0204-0206.md`; **EP-019 AC1 delivered & verified live** (⌘Z/⇧⌘Z); three first-pass bugs fixed; root-floor semantics clarified in design §5. Backend 244/244, macOS build clean. Next: SP-054 (persistence) in Planning. Earlier 2026-07-07: SP-052 closed (T-0201–0203 Verified), targets unified to 27.0.)*
