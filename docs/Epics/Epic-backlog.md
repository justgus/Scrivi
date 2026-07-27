# Epic Backlog

Epics listed here are fully defined but have no active sprint. Epics that were partially delivered and returned here retain their verified AC state. Work resumes when a sprint is assigned.

---

## Backlog Epics

_(EP-019 `[Apple]` Undo/Redo was un-deferred back to Active 2026-07-24 — now in `Epic-active.md`.)_

## [Linux] App — Epic Family (EP-024–EP-026)

The Ubuntu Qt/QML port of the macOS app, split one Epic per capability (mirroring how the Apple app was
built Epic-by-Epic). **EP-020 `[Linux]` (Foundation), EP-021 `[Linux]` (Project Lifecycle & Landing),
EP-022 `[Linux]` (Writing Surface & Scene Navigator), EP-023 `[Linux]` (Manuscript Structure Editing), and
EP-024 `[Linux]` (Scene Inspector Panel) are all ✅ closed** (→ `Epics/Closed/`); **EP-025 `[Linux]`
(Timeline Panel) is now 🟡 Active** (→ `Epics/Epic-active.md`). The Epic below is 🔵 Draft, promoted to Active
when EP-025 lands. Each is verified in Docker+VNC (developer) then on real Ubuntu (alpha tester). Each targets
near-parity with the named Apple Epic. Any new `scrivi_*` endpoint a screen needs is a Task with a
`[ScriviCore]` note, not a separate Epic.

### EP-026: [Linux] Undo/Redo, Menus, Settings & Parity Verification

**Status:** 🔵 Draft
**Goal:** Wire the EP-019 undo/redo history C ABI into the Linux UI (capture + apply + fork popover +
stale-branch purge), the app menu bar, project settings, and run the near-parity verification pass;
prepare the Linux app for the alpha tester on real Ubuntu. Mirrors Apple EP-012 + EP-019.
**Rough scope:** Linux `HistoryCapture` equivalent over `scrivi_history_*`, undo/redo apply, fork
popover, settings sheet (capacity + stale-branch purge), menu bar, full parity checklist, hand-off to
tester. **Depends on:** EP-022 (and benefits from EP-023–EP-025).

---

*Last Updated: 2026-07-22 (**EP-025 [Linux] Timeline Panel promoted from this backlog to 🟡 Active** (full
detail now in `Epic-active.md`); its first sprint SP-079 activated same day. The largest Linux Epic (full
Apple EP-016 parity, ~5 sprints). Remaining [Linux] family: **EP-026** stays 🔵 Draft (promoted when EP-025
lands). EP-019 [Apple] remains 🔴 Deferred. Prior: 2026-07-22 (EP-024 ✅ closed same-day); 2026-07-15 (EP-022
✅ closed; EP-023 promoted to Active).)*
