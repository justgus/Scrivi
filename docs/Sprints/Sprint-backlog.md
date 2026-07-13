# Sprint Backlog

Sprints listed here are in 🔵 Planning status — defined and ready to activate, but not yet started.

---

| Sprint | Title | Epic | Status |
| ------ | ----- | ---- | ------ |
| SP-056 | Copy buffers — store, ABI, HUD/palette UX, history integration | EP-019 | 🔵 Planning |
| SP-057 | Undo/Redo — history panel, perf fixtures, verification & Epic close | EP-019 | 🔵 Planning |

Design reference for all: `docs/Scrivi_UndoRedo_History_and_Copy_Buffers_Design_v0_1.md` (✅ Approved baseline 2026-07-06).
Epic-level ACs: `docs/Epics/Epic-active.md` (EP-019). Task detail: `docs/Tasks/Task-backlog.md` (T-0198–T-0217).

> **SP-051 activated 2026-07-06** (design signed off at activation) — moved to `Sprint-active.md`.
> **SP-052 activated 2026-07-07**, ✅ **closed 2026-07-07** — `Closed/Sprint-SP-052.md`.
> **SP-053 activated 2026-07-07**, ✅ **closed 2026-07-07** — `Closed/Sprint-SP-053.md` (EP-019 AC1 delivered & verified).
> **SP-054 activated 2026-07-07**, ✅ **closed 2026-07-09** — `Closed/Sprint-SP-054.md` (EP-019 AC3 + AC5 delivered & verified).
> **SP-055 activated 2026-07-10**, ✅ **closed 2026-07-13** — `Closed/Sprint-SP-055.md` (EP-019 AC4 + AC5's deferred branch clauses delivered & verified).
> **SP-056/SP-057 parked 2026-07-13** — EP-019 `[Apple]` was deferred to the Epic backlog; these sprints remain 🔵 Planning but are **not** next-up. The active line of work is now the `[Linux]` app (**SP-058**, EP-020) — see `Sprint-active.md`.

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

*Last Updated: 2026-07-13 (SP-055 closed with user approval — `Closed/Sprint-SP-055.md` (EP-019 AC4 + AC5's deferred branch clauses delivered & verified). SP-056 (copy buffers) is next in Planning. SP-056–SP-057 remain in Planning.)*
