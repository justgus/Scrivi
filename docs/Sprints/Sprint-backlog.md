# Sprint Backlog

Sprints listed here are in 🔵 Planning status — defined and ready to activate, but not yet started.

---

| Sprint | Title | Epic | Status |
| ------ | ----- | ---- | ------ |
| SP-052 | Undo/Redo — linear history engine core (C++) + C ABI + Swift wrappers | EP-019 | 🔵 Planning |
| SP-053 | Undo/Redo — in-session undo/redo on macOS (capture, apply, barriers) | EP-019 | 🔵 Planning |
| SP-054 | Undo/Redo — persistence, sessions, capacity, settings | EP-019 | 🔵 Planning |
| SP-055 | Undo/Redo — branching: tree ops, fork popover, purge | EP-019 | 🔵 Planning |
| SP-056 | Copy buffers — store, ABI, HUD/palette UX, history integration | EP-019 | 🔵 Planning |
| SP-057 | Undo/Redo — history panel, perf fixtures, verification & Epic close | EP-019 | 🔵 Planning |

Design reference for all: `docs/Scrivi_UndoRedo_History_and_Copy_Buffers_Design_v0_1.md` (✅ Approved baseline 2026-07-06).
Epic-level ACs: `docs/Epics/Epic-active.md` (EP-019). Task detail: `docs/Tasks/Task-backlog.md` (T-0198–T-0217).

> **SP-051 activated 2026-07-06** (design signed off at activation) — moved to `Sprint-active.md`.

---

## SP-052: Undo/Redo — linear history engine core (C++) + C ABI + Swift wrappers

**Status:** 🔵 Planning
**Epic:** EP-019
**Goal:** A linear (non-branching) history engine lives in ScriviCore with full unit coverage and is callable from Swift.

### Assigned Tasks
| ID | Title | Priority | Status |
| -- | ----- | -------- | ------ |
| T-0201 | `HistoryService` core (record/undo/redo, snapshot-diff, sessions) + Catch2 tests | High | 🔵 Backlog |
| T-0202 | C ABI: `scrivi_history_open/record_event/record_barrier/undo/redo/close` | High | 🔵 Backlog |
| T-0203 | `ScriviEngine.swift` history wrappers + interop tests | High | 🔵 Backlog |

**Exit criteria:** `ctest` green including new history unit tests; interop test round-trips record→undo→redo through the C ABI.

---

## SP-053: Undo/Redo — in-session undo/redo on macOS (capture, apply, barriers)

**Status:** 🔵 Planning
**Epic:** EP-019
**Goal:** ⌘Z/⇧⌘Z work in the running macOS app per the event model — **EP-019 AC1 delivered** (in-session; persistence follows in SP-054). AC1 carries the requirement formerly tracked as I-0019 (⚪ Closed 2026-07-06 — OBE/superseded, `docs/Issues/Closed/Issue-closed-0019.md`).

### Assigned Tasks
| ID | Title | Priority | Status |
| -- | ----- | -------- | ------ |
| T-0204 | `HistoryCapture` + commit-trigger wiring in the editor | High | 🔵 Backlog |
| T-0205 | Undo/redo apply path + `allowsUndo=false` + ⌘Z routing | High | 🔵 Backlog |
| T-0206 | Barriers on structural operations | Medium | 🔵 Backlog |

**Exit criteria:** type-undo-redo verifiable in the app across scene switches; pure cursor moves produce no events; structural ops barrier correctly; no auto-save regression.

---

## SP-054: Undo/Redo — persistence, sessions, capacity, settings

**Status:** 🔵 Planning
**Epic:** EP-019
**Goal:** History survives quit/relaunch; session-boundary warning works; capacity + eviction enforced; settings exposed per Trade T1.

### Assigned Tasks
| ID | Title | Priority | Status |
| -- | ----- | -------- | ------ |
| T-0207 | JSONL log + checkpoint + torn-line recovery + head-hash validation | High | 🔵 Backlog |
| T-0208 | Capacity/eviction + history settings (T1) + Project Settings row | Medium | 🔵 Backlog |
| T-0209 | Session-boundary warning popup | Medium | 🔵 Backlog |

**Exit criteria:** quit/relaunch then undo yesterday's edit with the boundary warning shown; kill -9 mid-write loses at most the last event; external scene edit produces an `externalChange` barrier, never a manuscript change.

---

## SP-055: Undo/Redo — branching: tree ops, fork popover, purge

**Status:** 🔵 Planning
**Epic:** EP-019
**Goal:** Full tree mechanics per the design (§5) with the inline fork popover (Trade T2 core interaction).

### Assigned Tasks
| ID | Title | Priority | Status |
| -- | ----- | -------- | ------ |
| T-0210 | Tree ops: branching, primary-child, `select_branch`, auto-purge on eviction | High | 🔵 Backlog |
| T-0211 | Inline fork popover (T2) | High | 🔵 Backlog |
| T-0212 | Stale-branch detection + user-confirmed purge | Medium | 🔵 Backlog |

**Exit criteria:** undo-type-fork-reselect scenario verifiable end-to-end; abandoned branch fully restorable and re-primaried; auto-purge on eviction covered by unit tests.

---

## SP-056: Copy buffers — store, ABI, HUD/palette UX, history integration

**Status:** 🔵 Planning
**Epic:** EP-019
**Goal:** Multiple copy buffers usable for the two-buffer replacement-scan CONOPS (design §9.a); pastes are history events.

### Assigned Tasks
| ID | Title | Priority | Status |
| -- | ----- | -------- | ------ |
| T-0213 | Copy-buffer store (`buffers.json`) + C ABI + engine wrappers | High | 🔵 Backlog |
| T-0214 | Buffer UX: keyboard HUD + palette + Edit-menu items (T4); paste/cut history integration | High | 🔵 Backlog |

**Exit criteria:** CONOPS walkthrough verifiable; each buffer paste is one undo step; system pasteboard untouched; buffers persist across relaunch.

---

## SP-057: Undo/Redo — history panel, perf fixtures, verification & Epic close

**Status:** 🔵 Planning
**Epic:** EP-019
**Goal:** Management surface (Trade T2 option B), performance validation, and EP-019 acceptance-criteria verification.

### Assigned Tasks
| ID | Title | Priority | Status |
| -- | ----- | -------- | ------ |
| T-0215 | History panel (T2 management surface) | Medium | 🔵 Backlog |
| T-0216 | Perf/integration fixtures (100k events, 500 KB scene); gitignore migration | Medium | 🔵 Backlog |
| T-0217 | Doc updates + EP-019 acceptance-criteria verification | High | 🔵 Backlog |

**Exit criteria:** all EP-019 ACs pass user verification; docs updated (`Scrivi_Project_Package_Structure`, repair matrix); Epic ready for close approval.

---

*Last Updated: 2026-07-06 (SP-051 activated and moved to Sprint-active.md — design approved by user. SP-052–SP-057 remain in Planning.)*
