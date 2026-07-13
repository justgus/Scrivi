# Epic Backlog

Epics listed here are fully defined but have no active sprint. Epics that were partially delivered and returned here retain their verified AC state. Work resumes when a sprint is assigned.

---

## Backlog Epics

## EP-019: [Apple] Custom Undo/Redo History & Multiple Copy Buffers

> **Codebase:** `[Apple]` — the undo/redo engine (ScriviCore `HistoryService` + C ABI) is delivered &
> verified; all remaining work (AC2/AC6/AC7/AC8) is Apple-platform UI. The parallel Linux undo/redo UI
> is a separate `[Linux]` Epic.

**Status:** 🔴 Deferred (moved to backlog 2026-07-13 on user request — paused mid-delivery to prioritize another effort; ACs and sprint/task history retained. Design ✅ approved 2026-07-06; SP-051/SP-052/SP-053 ✅ closed — **AC1 delivered & verified** 2026-07-07; **SP-054 ✅ closed 2026-07-09 — AC3 + AC5 delivered & verified**; **SP-055 ✅ closed 2026-07-13 — AC4 + AC5's branch clauses delivered & verified**. Resume at SP-056 (copy buffers → AC6).)
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
- [ ] AC6 — Copy buffers: ≥ 2 buffers loadable and pasteable at multiple locations (design CONOPS §9.a); each paste is one undo step; system pasteboard unaffected; buffers persist across relaunch.
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
| T-0207 | JSONL log + checkpoint + torn-line recovery + head-hash validation | SP-054 | ✅ Verified (2026-07-09) |
| T-0208 | Capacity/eviction + history settings (T1) + Project Settings row | SP-054 | ✅ Verified (2026-07-09; linear eviction, full in SP-055) |
| T-0209 | Session-boundary warning popup | SP-054 | ✅ Verified (2026-07-09) |
| T-0210 | Tree ops: branching, primary-child, `select_branch`, auto-purge on eviction | SP-055 | ✅ Verified (2026-07-13) → `Verified/Task-verified-0210-0212.md` |
| T-0211 | Inline fork popover (T2 core interaction) | SP-055 | ✅ Verified (2026-07-13) → `Verified/Task-verified-0210-0212.md` |
| T-0212 | Stale-branch detection + user-confirmed purge | SP-055 | ✅ Verified (2026-07-13) → `Verified/Task-verified-0210-0212.md` |
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

## [Linux] App — Epic Family (EP-021–EP-026)

The Ubuntu Qt/QML port of the macOS app, split one Epic per capability (mirroring how the Apple app was
built Epic-by-Epic). **EP-020 `[Linux]` (Foundation) is Active** in `Epic-active.md`; the Epics below are
🔵 Draft — defined and sequenced, promoted to Active one at a time as EP-020's spine lands. Each is
verified in Docker+VNC (developer) then on real Ubuntu (alpha tester). Each targets near-parity with the
named Apple Epic. Any new `scrivi_*` endpoint a screen needs is a Task with a `[ScriviCore]` note, not a
separate Epic.

### EP-021: [Linux] Project Lifecycle & Landing

**Status:** 🔵 Draft
**Goal:** Create / open / close a `.scrivi` project and bootstrap local identity from the Linux app;
landing view. Mirrors the Apple EP-006 project flows.
**Rough scope:** landing view (recent projects, new/open), `scrivi_ensure_local_identity`,
`scrivi_create_project`, `scrivi_open_project`, close/cleanup, project bookmark persistence (Linux
equivalent of `ProjectBookmarkStore`). **Depends on:** EP-020.

### EP-022: [Linux] Writing Surface & Scene Navigator

**Status:** 🔵 Draft
**Goal:** The core writing loop on Linux — scene navigator sidebar, the manuscript editing surface,
auto-save, quit/reopen. Mirrors Apple EP-009 / EP-011.
**Rough scope:** navigator list (manuscript order, live titles), QML text editor over `scrivi_open_scene`
/ `scrivi_save_scene`, viewport/scroll behavior, cursor/focus rules, auto-save debounce. **Depends on:**
EP-021. *(Largest of the family — the editor is the heart of the app.)*

### EP-023: [Linux] Manuscript Structure Editing

**Status:** 🔵 Draft
**Goal:** Create / delete / rename / reorder scenes and chapters from the Linux navigator. Mirrors Apple
EP-010.
**Rough scope:** `scrivi_create_scene/create_chapter/delete_*/rename_*/reorder_*`, drag-reorder in QML,
chapter title toggle. **Depends on:** EP-022.

### EP-024: [Linux] Scene Inspector Panel

**Status:** 🔵 Draft
**Goal:** The scene inspector side panel on Linux. Mirrors Apple EP-014.
**Rough scope:** dockable inspector, scene-entities tab structure, collapse/expand. **Depends on:**
EP-022.

### EP-025: [Linux] Timeline Panel

**Status:** 🔵 Draft
**Goal:** The timeline panel on Linux — scene dots, story-time, story-structure bands, historical
events, imported timelines. Mirrors Apple EP-015 / EP-016.
**Rough scope:** the timeline `scrivi_*` surface (get_timeline, story-time, bands, historical events,
imported timelines, export), horizontal QML timeline with drag, time delta picker. **Depends on:**
EP-022. *(Large — EP-016 was a substantial Apple Epic.)*

### EP-026: [Linux] Undo/Redo, Menus, Settings & Parity Verification

**Status:** 🔵 Draft
**Goal:** Wire the EP-019 undo/redo history C ABI into the Linux UI (capture + apply + fork popover +
stale-branch purge), the app menu bar, project settings, and run the near-parity verification pass;
prepare the Linux app for the alpha tester on real Ubuntu. Mirrors Apple EP-012 + EP-019.
**Rough scope:** Linux `HistoryCapture` equivalent over `scrivi_history_*`, undo/redo apply, fork
popover, settings sheet (capacity + stale-branch purge), menu bar, full parity checklist, hand-off to
tester. **Depends on:** EP-022 (and benefits from EP-023–EP-025).

---

*Last Updated: 2026-07-13 (EP-019 → backlog 🔴 Deferred [Apple], retro-tagged. NEW: [Linux] app Epic family EP-021–EP-026 added as 🔵 Draft (EP-020 [Linux] Foundation is Active); Ubuntu Qt/QML port for the no-Mac alpha tester, one Epic per capability mirroring the Apple build. All non-Apple GUI Epics now codebase-tagged per Epic-GUIDELINES.md.)*
