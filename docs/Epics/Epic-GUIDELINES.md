# Epic Guidelines

## Purpose

Epics are large, strategic bodies of work that span multiple Sprints and represent a meaningful product milestone or capability. An Epic groups related Tasks and Issues under a single goal. Sprints executed against that goal are linked to the Epic.

Epics are not scheduled directly — they are delivered through a sequence of Sprints. Think of an Epic as a destination and each Sprint as a leg of the journey.

## Epic vs. Task vs. Sprint

| Concept | Scope | Duration | Tracked By |
| ------- | ----- | -------- | ---------- |
| Task    | Single feature or improvement | Hours to days | Task system |
| Issue   | Single bug or defect | Hours to days | Issue system |
| Sprint  | A batch of Tasks/Issues | 1–2 weeks | Sprint system |
| Epic    | A major capability or milestone | Multiple Sprints | Epic system |

## Epic Lifecycle

```
Draft → Active → Complete → Closed
  🔵      🟡        🟠         ✅
```

### Status Definitions

- **🔵 Draft** — Epic is being defined. Goal, scope, and acceptance criteria are being established. No Sprints assigned yet.
- **🟡 Active** — At least one Sprint is actively delivering work toward this Epic.
- **🟠 Complete** — All planned Sprints are closed and all required Tasks/Issues are verified. Awaiting final acceptance.
- **✅ Closed** — Epic is fully accepted and archived. No further work planned.

A separate **🔴 Deferred** status may be used when an Epic is paused indefinitely: goals are valid but work is suspended.

## Epic Numbering

Epics are numbered sequentially: `EP-001`, `EP-002`, `EP-003`, etc.

Each Epic has a short descriptive title summarizing the capability it delivers.

## Defining an Epic

Before marking an Epic as Active:

1. **Write the Epic Goal** — A clear, user-facing statement of what will be true when the Epic is done.
2. **Define Acceptance Criteria** — Specific, testable conditions that signal completion.
3. **Identify Initial Scope** — List known Tasks and Issues that belong to this Epic. This list may grow.
4. **Estimate Span** — Rough number of Sprints expected (can be revised).
5. **Link to Strategy** — Note how this Epic serves the product vision (optional but recommended).

## Epic Execution

During delivery:

- Each Sprint that contributes to the Epic references it in its `Epic:` field.
- New Tasks or Issues that emerge during delivery may be added to the Epic's scope with a note.
- Epic status is updated as Sprints complete.

## Completing an Epic

When all planned Sprints are closed:

1. Review acceptance criteria — are all conditions met and verified?
2. Mark status 🟠 Complete.
3. User reviews and explicitly approves closing the Epic.
4. Move to `Closed/Epic-EP-XXX.md` and mark ✅ Closed.

## Authorization Rules

### What Claude CAN Do:
- ✅ Create a new Epic in 🔵 Draft status
- ✅ Update Epic scope (add/remove Tasks, Issues, Sprints)
- ✅ Mark Epic as 🟡 Active when first Sprint begins
- ✅ Mark Epic as 🟠 Complete when all planned Sprints are closed
- ✅ Draft the completion summary for user review

### What Claude CANNOT Do:
- ❌ Mark an Epic as ✅ Closed without user approval
- ❌ Defer or close an Epic without user approval
- ❌ Remove acceptance criteria without user approval

## File Organization

```
docs/Epics/
├── Epic-GUIDELINES.md          ← This file
├── Epic-Documentation.md       ← Index of all Epics
├── Epic-active.md              ← Currently active Epic(s) in detail
├── Closed/
│   ├── Epic-EP-001.md
│   ├── Epic-EP-002.md
│   └── ...
```

- **Epic-Documentation.md** — Lean index. One row per Epic, always up to date.
- **Epic-active.md** — Full detail on Epics that are Active or Complete-pending-close. Multiple Epics may be active simultaneously.
- **Closed/Epic-EP-XXX.md** — Archive file per closed Epic.

## Epic Entry Template

```markdown
## EP-XXX: [Epic Title]

**Status:** 🔵 Draft / 🟡 Active / 🟠 Complete / ✅ Closed
**Goal:** [Clear, user-facing statement of what will be true when this Epic is done]
**Date Created:** YYYY-MM-DD
**Target Close Date:** YYYY-MM-DD (estimated)
**Actual Close Date:** YYYY-MM-DD (filled in at close)

### Acceptance Criteria

- [ ] [Condition 1 — specific and testable]
- [ ] [Condition 2]
- [ ] [Condition 3]

### Sprints

| Sprint | Title | Status | Dates |
| ------ | ----- | ------ | ----- |
| SP-XXX | ...   | ✅ Closed | YYYY-MM-DD – YYYY-MM-DD |

### Tasks

| ID     | Title | Status |
| ------ | ----- | ------ |
| T-XXXX | ...   | ✅ Implemented - Verified |

### Issues

| ID     | Title | Status |
| ------ | ----- | ------ |
| I-XXXX | ...   | ✅ Resolved - Verified |

### Scope Notes

[Any items added or removed from original scope, with brief rationale]

### Completion Summary

[Filled in when Epic reaches 🟠 Complete — what was delivered, what was deferred, notable decisions]
```

## Epic-Documentation.md Update Checklist

### When Creating a New Epic:
- [ ] Epic entry added to Epic-Documentation.md index table
- [ ] Epic entry added to Epic-active.md (if immediately activated) or noted as Draft
- [ ] Epic count updated
- [ ] "Last Updated" date updated

### When Activating an Epic:
- [ ] Status changed to 🟡 Active in Epic-active.md
- [ ] Status updated in Epic-Documentation.md index
- [ ] Sprint entries linked to Epic
- [ ] "Last Updated" date updated

### When Completing an Epic:
- [ ] All acceptance criteria reviewed and checked
- [ ] Status changed to 🟠 Complete in Epic-active.md
- [ ] Completion summary drafted
- [ ] Status updated in Epic-Documentation.md index
- [ ] "Last Updated" date updated

### When Closing an Epic (user-approved):
- [ ] Epic moved from Epic-active.md to Closed/Epic-EP-XXX.md
- [ ] Status changed to ✅ Closed
- [ ] Status updated in Epic-Documentation.md index
- [ ] Statistics updated
- [ ] "Last Updated" date updated

---

**REMINDER:** Every Epic operation MUST update Epic-Documentation.md before marking work as complete.
