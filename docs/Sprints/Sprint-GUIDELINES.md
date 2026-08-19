# Sprint Guidelines

## Purpose

Sprints are the core execution unit of Scrivi's Agile development process. A Sprint is a fixed-duration iteration (typically 1–2 weeks) during which a defined set of Tasks and Issues are completed. Sprints provide rhythm, focus, and a clear cadence for reviewing progress.

Sprints do not carry work. If a Task or Issue is not completed by the end of a Sprint, it returns to the backlog and is reassigned to a future Sprint.

## Sprint Lifecycle

```
Planning → Active → Review → Closed
  🔵         🟡        🟠       ✅
```

### Status Definitions

- **🔵 Planning** — Sprint is being assembled. Goals, Tasks, and Issues are being selected but work has not begun.
- **🟡 Active** — Sprint is in progress. Work is underway on assigned items.
- **🟠 Review** — Sprint time-box has ended. Completed items are being verified; incomplete items are being returned to the backlog.
- **✅ Closed** — All items resolved, verified, or returned. Sprint retrospective is complete.

## Sprint Numbering

Sprints are numbered sequentially: `SP-001`, `SP-002`, `SP-003`, etc.

Each Sprint has a short descriptive title summarizing its primary focus area.

## Sprint Planning

Before marking a Sprint as Active:

1. **Define the Sprint Goal** — One or two sentences describing the primary outcome.
2. **Select Items** — Choose Tasks and Issues from the backlog. Assign each a Sprint field (e.g., `Sprint: SP-001`).
3. **Estimate Capacity** — Note rough time available (e.g., "~10 hours over 2 weeks").
4. **Confirm Scope** — User and Claude agree on the Sprint contents before work begins.

## Sprint Execution

During the Sprint:

- Work only on items assigned to the current Sprint.
- Update Task and Issue statuses as work progresses.
- If a new urgent item is discovered, add it to the backlog first; do not silently add it to the active Sprint without noting it.

## Sprint Review and Close

At the end of a Sprint:

1. **Tally completed items** — List what was finished and verified.
2. **Return incomplete items to backlog** — Remove Sprint assignment; set status back to 🔵 Backlog.
3. **Update Sprint status to 🟠 Review**, then **✅ Closed** once retrospective is noted.
4. **Write a brief retrospective** — What went well, what didn't, what to adjust.

## Authorization Rules

### What Claude CAN Do:
- ✅ Create a new Sprint in 🔵 Planning status
- ✅ Add or remove Tasks/Issues from a Sprint during Planning
- ✅ Mark a Sprint as 🟡 Active when work begins
- ✅ Mark a Sprint as 🟠 Review when the time-box ends
- ✅ Move incomplete items back to backlog at close
- ✅ Draft the retrospective for user review

### What Claude CANNOT Do:
- ❌ Mark a Sprint as ✅ Closed without user approval
- ❌ Add items to an Active Sprint without noting the change explicitly
- ❌ Remove items from an Active Sprint silently

## File Organization

```
docs/Sprints/
├── Sprint-GUIDELINES.md          ← This file
├── Sprint-Documentation.md       ← Index of all Sprints
├── Sprint-active.md              ← Currently active Sprint
├── Sprint-backlog.md             ← Planned Sprints in 🔵 Planning status
├── Closed/
│   ├── Sprint-SP-001.md
│   ├── Sprint-SP-002.md
│   └── ...
```

- **Sprint-Documentation.md** — Lean index. ⚠️ **Its All-Sprints table is the SINGLE SOURCE for per-sprint
  status and counts (P7).** Do **not** restate those counts in a Statistics block or a "Currently:" line —
  a summary that restates a table will drift from it. Statistics carries only what the table cannot
  express: next available ID, and IDs that are not sprints (cancelled / superseded / skipped) with reasons.
- **Sprint-active.md** — Contains the single active Sprint in full detail. At most one Sprint is Active at a time.
- **Sprint-backlog.md** — Sprints in 🔵 Planning status. ⚠️ **A Sprint leaves this file at ACTIVATION and
  never returns. No closure note is written here** — whether a Sprint later closed is
  `Closed/Sprint-SP-XXX.md`'s business and the All-Sprints table's. **The backlog contains Planning
  Sprints and nothing else.**
- **Closed/Sprint-SP-XXX.md** — Archive file per closed Sprint, including retrospective.

⚠️ **Nothing else belongs in `docs/Sprints/`.** Anything that is not one of the four files above or
`Closed/` is an orphan. *(The 2026-08-19 audit found **13** loose files there — 12 superseded planning
drafts and one sprint whose only record was a draft still declaring "🔵 Planning" two months after it
closed.)*

### Planning drafts

A planning document may be written while a Sprint is being assembled. ⚠️ **It is TRANSIENT.** It is either
folded into `Sprint-active.md` at activation, or deleted at close — **in the same step the archive is
written.** **No draft outlives its sprint's close.**

⚠️ **A draft left behind reads as a current document.** `Sprint-SP-039.md` sat in `docs/Sprints/` for two
months declaring *"🔵 Planning"* for a sprint that had closed — and it was the **only** sprint-level record
of the entire timeline C ABI.

### A Sprint may be fully planned while still 🔵 Planning

A completed plan may occupy `Sprint-active.md` **before activation**, so the work can be reviewed as a
whole. When it does:

- ⚠️ It **MUST** carry a banner stating it is not yet active.
- ⚠️ **Activation is a separate step and requires direct user approval.**
- Its row stays in `Sprint-backlog.md` until it is activated.

*(SP-100 was in exactly this state on 2026-08-19: plan complete, banner-marked, not activated.)*

## Sprint Entry Template

```markdown
## SP-XXX: [Sprint Title]

**Status:** 🔵 Planning / 🟡 Active / 🟠 Review / ✅ Closed
**Epic:** [EP-XXX: Epic Title / "None"]
**Goal:** [One or two sentences describing the Sprint's primary outcome]
**Start Date:** YYYY-MM-DD
**End Date:** YYYY-MM-DD
**Capacity:** [Estimated hours or days available]

### Assigned Tasks

| ID     | Title | Priority | Status |
| ------ | ----- | -------- | ------ |
| T-XXXX | ...   | High     | 🔵 Backlog |

### Assigned Issues

| ID     | Title | Severity | Status |
| ------ | ----- | -------- | ------ |
| I-XXXX | ...   | Medium   | 🔴 Open |

### Sprint Notes

[Any constraints, dependencies, or focus areas for this Sprint]

### Retrospective

**Completed:**
- [Item 1]
- [Item 2]

**Returned to Backlog:**
- [Item 1 — reason]

**What went well:**
- [Observation]

**What to improve:**
- [Observation]

**Carry-forward notes:**
- [Anything the next Sprint should know]
```

## Sprint-Documentation.md Update Checklist

### When Creating a New Sprint:
- [ ] Sprint entry added to Sprint-Documentation.md index table
- [ ] Sprint entry added to Sprint-active.md (if immediately activated) or noted as Planning
- [ ] Sprint count updated
- [ ] "Last Updated" date updated

### When Activating a Sprint:
- [ ] Status changed to 🟡 Active in Sprint-active.md
- [ ] Status updated in Sprint-Documentation.md index
- [ ] Status updated to 🟡 Active in the Epic's sprint table (Epic-active.md)
- [ ] Task and Issue "Sprint" fields updated for all assigned items
- [ ] Task statuses updated to 🟡 Active in Sprint task table (Sprint-active.md)
- [ ] Task statuses updated to 🟡 Active in Epic task table (Epic-active.md)
- [ ] Task statuses updated to 🟡 Active in Task-active.md and Task-Documentation.md
- [ ] "Last Updated" date updated

### When Closing a Sprint:

⚠️ **A sprint close is the single most drift-producing event in the tracking system**, because a sprint's
status is restated in every layer. **This checklist must name every file that restates it.** The
2026-08-19 audit found SP-102's close left stale claims in **four** files — and **three of them were not
on the old checklist at all.**

**Sprint layer**
- [ ] Sprint moved from `Sprint-active.md` to `Closed/Sprint-SP-XXX.md`
- [ ] Retrospective section completed
- [ ] ⚠️ **Every Task row in the sprint archive reflects its FINAL status** — not the planning-time value
      *(SP-039 and SP-074 both closed with stale task tables; SP-074's sat wrong for four weeks)*
- [ ] ⚠️ **Any planning draft for this sprint is DELETED** (see "Planning drafts" below)
- [ ] Incomplete items returned to backlog (Sprint field cleared)
- [ ] Status updated to ✅ Closed in `Sprint-Documentation.md`'s **All-Sprints table**
- [ ] ⚠️ **`Sprint-backlog.md`** — the sprint's row is gone (it left at *activation*; **no closure note is
      written there**)
- [ ] `Sprint-active.md` cleared (or set to next Sprint if immediately starting)

**Epic layer**
- [ ] Status updated to ✅ Closed in the Epic's sprint table (`Epic-active.md`)
- [ ] ⚠️ **`Epic-Documentation.md`** — Active-Epics prose and any AC state it names

**Task layer** ⚠️ *(none of these were on the checklist before 2026-08-19)*
- [ ] ⚠️ **`Task-active.md`** — the sprint's Tasks removed
- [ ] ⚠️ **`Task-unverified.md`** — reflects reality, and **no prose contradicts its table**
- [ ] ⚠️ **`Task-Documentation.md`** — every Task row's status, **and** its statistics **re-derived by
      counting** (never adjusted to preserve a total)

**Issue layer**
- [ ] Every Issue verified in this sprint is **archived in the same step** — from `Issue-active.md` *or*
      `Issue-backlog.md`
- [ ] `Issue-Documentation.md` counts **re-derived**

**Finally**
- [ ] "Last Updated" date updated on each file actually changed

> ⚠️ **If a retrospective defers verification to a later sprint, it must name WHO closes the loop.**
> SP-074 wrote *"do not mark I-0083 fully Verified until app adoption lands"* — the condition was met the
> next day, and **nobody came back for four weeks.** A condition without an owner is not a plan.

---

**REMINDER:** Every Sprint operation MUST update Sprint-Documentation.md before marking work as complete.

---

## Related: the Audit layer

Documentation consistency across all four tracking layers is maintained by **Audits** —
see [`../Audits/Audit-Guidelines.md`](../Audits/Audit-Guidelines.md).

⚠️ **An Audit is large and formal, and begins ONLY when the user requests it.** Claude never starts one
automatically; it may *recommend* one. A lightweight **Audit Check** — a read-only, mechanical sweep — is
the instrument for routine verification.

---

## ⚠️ Standing principle P7 — derive it or delete it

**A summary that restates a table will drift from it.** Keep in a statistics block **only what the table
cannot express**: next available ID, IDs that are not sprints/Epics (cancelled, superseded, skipped) with
reasons, and pointers to where detail lives.

⚠️ **This is the single most repeated defect in the Scrivi tracking docs.** It recurred **three times**,
and the correct diagnosis was written down — *"deriving these lines from the table rather than restating
them"* — on **two** prior occasions before it was finally acted on in the 2026-08-19 audit.

**Do not reintroduce per-status counts.** To count by status, read the table.

---

## ⚠️ Standing principle P3 — reconstruction ≠ back-filling

Restoring a **deleted** record from surviving **primary** sources, under QA observation and **marked as
reconstructed**, is legitimate. Manufacturing records that were **never written**, in bulk, from secondary
sources, is not. ⚠️ **The distinction is *what happened to the record*, not *how old it is*.**

*[SP-039 closed 2026-06-11 with no archive at all — its only record was a planning draft still declaring
"🔵 Planning", for the sprint that delivered the entire timeline C ABI. It was reconstructed. By contrast,
176 Verified Tasks with no archive file are **left as an indexed register**, because those records were
never written.]*
