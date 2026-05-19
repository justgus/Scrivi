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
├── Closed/
│   ├── Sprint-SP-001.md
│   ├── Sprint-SP-002.md
│   └── ...
```

- **Sprint-Documentation.md** — Lean index. One row per Sprint, always up to date.
- **Sprint-active.md** — Contains the single active Sprint in full detail. At most one Sprint is Active at a time.
- **Closed/Sprint-SP-XXX.md** — Archive file per closed Sprint, including retrospective.

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
- [ ] Task and Issue "Sprint" fields updated for all assigned items
- [ ] "Last Updated" date updated

### When Closing a Sprint:
- [ ] Sprint moved from Sprint-active.md to Closed/Sprint-SP-XXX.md
- [ ] Retrospective section completed
- [ ] Incomplete items returned to backlog (Sprint field cleared)
- [ ] Status updated to ✅ Closed in Sprint-Documentation.md index
- [ ] Sprint-active.md cleared (or set to next Sprint if immediately starting)
- [ ] Statistics updated
- [ ] "Last Updated" date updated

---

**REMINDER:** Every Sprint operation MUST update Sprint-Documentation.md before marking work as complete.
