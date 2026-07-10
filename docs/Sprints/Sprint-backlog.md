# Sprint Backlog

Sprints listed here are in 🔵 Planning status — defined and ready to activate, but not yet started.

---

| Sprint | Title | Epic | Status |
| ------ | ----- | ---- | ------ |
| SP-055 | Undo/Redo — branching: tree ops, fork popover, purge | EP-019 | 🔵 Planning |
| SP-056 | Copy buffers — store, ABI, HUD/palette UX, history integration | EP-019 | 🔵 Planning |
| SP-057 | Undo/Redo — history panel, perf fixtures, verification & Epic close | EP-019 | 🔵 Planning |

Design reference for all: `docs/Scrivi_UndoRedo_History_and_Copy_Buffers_Design_v0_1.md` (✅ Approved baseline 2026-07-06).
Epic-level ACs: `docs/Epics/Epic-active.md` (EP-019). Task detail: `docs/Tasks/Task-backlog.md` (T-0198–T-0217).

> **SP-051 activated 2026-07-06** (design signed off at activation) — moved to `Sprint-active.md`.
> **SP-052 activated 2026-07-07**, ✅ **closed 2026-07-07** — `Closed/Sprint-SP-052.md`.
> **SP-053 activated 2026-07-07**, ✅ **closed 2026-07-07** — `Closed/Sprint-SP-053.md` (EP-019 AC1 delivered & verified).
> **SP-054 activated 2026-07-07** — moved to `Sprint-active.md`; all tasks ✅ Verified 2026-07-09, ready to close pending user approval.

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

*Last Updated: 2026-07-07 (SP-053 closed with user approval — `Closed/Sprint-SP-053.md` (EP-019 AC1 delivered & verified). SP-054 (persistence) is next in Planning. SP-054–SP-057 remain in Planning.)*
