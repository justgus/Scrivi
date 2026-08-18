# Active Tasks

Tasks currently being worked by an **active Sprint**. Each row names the Sprint it belongs to; the
Sprint's own documentation lives in [`../Sprints/Sprint-active.md`](../Sprints/Sprint-active.md) and is
**not** duplicated here.

A Task leaves this file when it is Verified (→ `Verified/Task-verified-XXXX.md`) or when its Sprint
closes. Tasks that are implemented but awaiting user verification move to
[`Task-unverified.md`](Task-unverified.md).

**Claude may mark a Task `Implemented - Not Verified`. Only the user can mark it Verified.**

⚠️ **Active Tasks live here, never in `Task-backlog.md`.** A Sprint may be **deactivated without
deactivating its Tasks** — the Sprint's status changes on its own layer while its Tasks stay in this
file. The backlog is for unstarted, unassigned work only.

---

## Currently: **3 active Tasks** — SP-102 `[Apple]` Pending presentation + warning view + `sources` card

| ID | Title | Sprint | Priority | Status |
| -- | ----- | ------ | -------- | ------ |
| T-0389 | Pending **footer** + warning view + the `offline`/`unmounted` refinement (**AC24**) | SP-102 | High | 🟠 **Implemented - Not Verified (2026-08-17)** |
| T-0365 | Aggregate `sources` card + citation popup (final third — `cites` type ✅ SP-096, `source` kind ✅ SP-098) | SP-102 | Medium | 🔵 Planned |
| T-0415 | ⚠️ **AC23 live verification on the real USB world rig** — both branches (**new at planning**) | SP-102 | High | 🔵 Planned — **unblocked by T-0389** |

**Delivers EP-031 AC23 + AC24**, the last two clauses of AC9. ✅ **Planning completed 2026-08-17.**

> ⚠️ **The planning audit cut T-0389 substantially — most of its staged scope already shipped.** Pending rows
> shown-not-hidden, named cached entries, the ⚠ badge, disabled-and-explained removal, the typed
> `worldPending:<status>` decode **and the Worlds menu** all landed in **SP-099** (T-0386/T-0407/T-0408) and
> **SP-104** (I-0117). **AC18/19/20 also already landed** as T-0388.
>
> **What is genuinely unbuilt:** the card-level **§7.2 footer** (the card names no world today — only a
> hover tooltip), the **warning view** (`listPendingEdges` is wrapped with **zero call sites**), the **AC24
> volume refinement** (`offline`/`unmounted` are still declared-but-never-produced), and the **`sources`
> card** (not registered; no citation popup).

> **T-0415 is new.** AC23 verification was an exit-criterion line inside T-0389; it is now its own Task
> because ⚠️ **only a live ejectable-volume run can establish "restores with no writer intervention"** — a
> fixture cannot. Burying that inside an implementation task is how it gets reported done on a fixture.

Full plan, rulings **R1/R2**, and the audit table: [`../Sprints/Sprint-active.md`](../Sprints/Sprint-active.md).

---

## Closed: SP-106 `[Cross]` Test integrity & CI trust (2026-08-17)

T-0412, T-0413 and T-0414 were all ✅ **Verified 2026-08-17** and archived to
[`Verified/Task-verified-0412-0414.md`](Verified/Task-verified-0412-0414.md). Issues **I-0121** and **I-0122**
were Verified in the same step → [`../Issues/Verified/Issue-verified-0121-0130.md`](../Issues/Verified/Issue-verified-0121-0130.md).
Sprint record: [`../Sprints/Closed/Sprint-SP-106.md`](../Sprints/Closed/Sprint-SP-106.md).

**Standing practice adopted from its exit criterion 4:** ⚠️ **`ctest` figures must name their architecture.**
"516/516" without a platform is the habit that let a divide-by-zero run red on x86-64 CI for 17 days.

---

## Cleanup note (2026-08-15)

This file previously carried **full sprint documentation for eight Sprints** — SP-090, SP-091, SP-092,
SP-093, SP-095, SP-096, SP-097 and SP-101 — including retrospectives, suite counts, design rulings and
planning narrative. That content belongs to the Sprint layer, not the Task layer.

**All eight Sprints were already closed and archived** to `../Sprints/Closed/`, and every Task and Issue
listed under them was already Verified. Nothing was active. The sections were removed rather than moved:
each Sprint's archive file is the authoritative record and already contains the same material.

| Sprint | Archive | Tasks (all Verified) |
| ------ | ------- | -------------------- |
| SP-090 | [`Sprint-SP-090.md`](../Sprints/Closed/Sprint-SP-090.md) | T-0359–T-0362 |
| SP-091 | [`Sprint-SP-091.md`](../Sprints/Closed/Sprint-SP-091.md) | T-0392, T-0393, T-0363, T-0364 |
| SP-092 | [`Sprint-SP-092.md`](../Sprints/Closed/Sprint-SP-092.md) | T-0394, T-0395, T-0366, T-0367, T-0368 |
| SP-093 | [`Sprint-SP-093.md`](../Sprints/Closed/Sprint-SP-093.md) | T-0396, T-0397, T-0398 (+ I-0104–I-0111) |
| SP-095 | [`Sprint-SP-095.md`](../Sprints/Closed/Sprint-SP-095.md) | T-0370, T-0371, T-0372, T-0401 |
| SP-096 | [`Sprint-SP-096.md`](../Sprints/Closed/Sprint-SP-096.md) | T-0402, T-0373, T-0374, T-0375, T-0376 |
| SP-097 | [`Sprint-SP-097.md`](../Sprints/Closed/Sprint-SP-097.md) | T-0403, T-0381–T-0385, T-0404 |
| SP-101 | [`Sprint-SP-101.md`](../Sprints/Closed/Sprint-SP-101.md) | T-0399 |

Two sections were **stale as well as misplaced**: SP-101 was shown 🟡 active and SP-092 🔵 planning, but
both closed 2026-08-11 (Human-approved). T-0399 is ✅ Verified; SP-092's T-0368 closed as delivered by
T-0399.

**Carried forward, not lost:** T-0365 (`sources` card) remains 🔵 Backlog, unblocked 2026-08-12, and is
scoped into SP-102.

⚠️ **Correction (2026-08-16):** this section previously said T-0217 "is still owed **before EP-019 closes**."
That was impossible on its face — **EP-019 closed 2026-08-11**, five days before. T-0217 was in fact
**completed in SP-094 and Verified 2026-08-11** (design §4.a/§4.a.1/§4.d/§12.2/§12.8/§14/§15,
package-structure §16a, repair-matrix §6.21); its record of truth is
[`Sprint-SP-094.md`](../Sprints/Closed/Sprint-SP-094.md). Nothing is owed. `Task-backlog.md` carried it as
🔵 Backlog under the superseded SP-057 and has been corrected to match.

---

*Last Updated: 2026-08-18 (**Documentation audit.** ⚠️ **`Task-backlog.md` was carrying this file's
three active Tasks** (T-0389, T-0365, T-0415) as backlog rows, alongside ~60 Verified Tasks and 7
unverified ones. The backlog was rebuilt to hold unstarted work only; the layer rule is now stated at
the top of both files: **a Sprint may be deactivated without deactivating its Tasks.** Separately,
**T-0218–T-0222** were listed in the backlog as "🟢 Active → `Task-active.md`" — they have been
✅ Verified and archived since **2026-07-13**, and were never in this file. Prior note follows.)*

*2026-08-17, later same day (**SP-102 planning completed.** ⚠️ **A code audit found most of
T-0389's staged scope already shipped in SP-099/SP-104** and struck it; **T-0415 added** for AC23 live
verification. Two rulings: **R1** the warning view gets its own toggle rather than being anchored to the
hidden-by-default Timeline; **R2** AC24 refined via URL volume keys, degrading to `unavailable` and never
guessing `missing` — **no ABI change needed**. Prior note follows.)*

*2026-08-17 (**SP-106 ✅ closed (user-approved) — T-0412/T-0413/T-0414 all Verified and
archived**, together with Issues I-0121 + I-0122. **SP-102 activated**: T-0389 + T-0365, delivering EP-031
AC23 + AC24. Prior note follows.)*

*2026-08-16 (SP-106 activated and implemented — T-0412/T-0413/T-0414 all 🟠 Implemented - Not
Verified. This file carries active Tasks again for the first time since 2026-08-15.)*
