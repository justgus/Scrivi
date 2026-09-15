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
Proposed → Draft → Active → Complete → Closed
   🔵        🔵      🟡        🟠         ✅
```

### Status Definitions

- **🔵 Proposed** — Epic is queued in the backlog. Goal and rough scope are noted but acceptance criteria have not been written. No Sprints assigned.
- **🔵 Draft** — Epic is being actively defined. Goal, scope, and acceptance criteria are being established. No Sprints assigned yet.
- **🟡 Active** — At least one Sprint is actively delivering work toward this Epic.
- **🟠 Complete** — All planned Sprints are closed and all required Tasks/Issues are verified. Awaiting final acceptance.
- **✅ Closed** — Epic is fully accepted and archived. No further work planned.

A separate **🔴 Deferred** status may be used when an Epic is paused indefinitely: goals are valid but work is suspended.

## Epic Numbering

Epics are numbered sequentially: `EP-001`, `EP-002`, `EP-003`, etc.

Each Epic has a short descriptive title summarizing the capability it delivers.

## Codebase Tag (required)

Every Epic declares the **codebase it primarily serves** with a tag in its title, e.g.
`## EP-022: [Linux] Writing Surface & Scene Navigator`. The same tag appears in the Epic index table
(`Epic-Documentation.md`).

| Tag | Codebase |
| --- | -------- |
| `[ScriviCore]` | C++23 backend / C ABI (`ScriviCore/`, `scrivi.h`) |
| `[Apple]` | SwiftUI/AppKit macOS + iOS/iPadOS/visionOS app (`Scrivi/`) |
| `[Linux]` | Qt/QML Ubuntu app (`platforms/linux/`) |
| `[Windows]` | Qt/QML Windows app (`platforms/windows/`) |
| `[Cross]` | Genuinely spans multiple codebases |

**Rules:**
- Pick the codebase where **most** of the Epic's work lands — that is its home tag. Most Epics have a
  clear home even if they touch the C ABI incidentally (e.g. a Linux screen that needs one new
  `scrivi_*` endpoint is still `[Linux]`; the endpoint is a Task with a note).
- Reserve `[Cross]` for Epics whose work is genuinely and substantially split across codebases — use it
  sparingly. When in doubt, pick the primary codebase and note the secondary work in Scope Notes.
- A capability delivered on multiple platforms is **multiple Epics**, one per codebase (e.g. the macOS
  timeline is EP-016 `[Apple]`; the Linux timeline is its own `[Linux]` Epic), not one `[Cross]` Epic —
  this keeps each Epic's acceptance criteria verifiable on a single platform.

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

1. ⚠️ **Run an AUDIT CHECK** — see [`../Audits/Audit-Guidelines.md`](../Audits/Audit-Guidelines.md).
2. Review acceptance criteria — are all conditions met and verified?
3. Mark status 🟠 Complete.
4. User reviews and explicitly approves closing the Epic.
5. Move to `Closed/Epic-EP-XXX.md` and mark ✅ Closed.

### ⚠️ Step 1 — the Audit Check, and why it is first

An Epic close **reads every tracking layer as evidence**, so the layers must be confirmed to agree with
each other *before* the acceptance criteria are judged against them.

**This is an Audit CHECK, not an Audit.** A Check is read-only and mechanical — greps and counts, minutes
of work. ⚠️ **A full Audit is large, formal, and begins only when the user requests it. Closing an Epic
does NOT trigger one.**

**Findings from the Check are ruled as part of the Epic close**, not in a separate session. ⚠️ If a Check
turns up something large or systemic, that is grounds to **recommend** a full Audit — which still only
begins on user request.

**Why this exists:** the 2026-08-19 audit found two defects that would have corrupted EP-031's close, both
detectable by simple greps —

- `Epic-Documentation.md` claimed **AC9 was outstanding**; it had been met two days earlier. **SP-100's
  entire job is verifying AC1–AC10.**
- **I-0118**, named in `Epic-active.md` as *"available as evidence for SP-100's AC pass"*, had **no archive
  entry anywhere.**

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
├── Epic-backlog.md             ← Proposed Epics not yet being actively defined
├── Epic-active.md              ← Epics that are Draft, Active, or Complete-pending-close
├── Closed/
│   ├── Epic-EP-001.md
│   ├── Epic-EP-002.md
│   └── ...
```

- **Epic-Documentation.md** — Lean index. One row per Epic, always up to date.
- **Epic-backlog.md** — Proposed Epics queued for future planning. Rough goal and scope only.
- **Epic-active.md** — Full detail on Epics that are Draft, Active, or Complete-pending-close. Multiple Epics may be active simultaneously.
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

### When Proposing a New Epic (Backlog):
- [ ] Epic entry added to Epic-backlog.md with goal and rough scope
- [ ] Epic entry added to Epic-Documentation.md index table with 🔵 Proposed status
- [ ] Epic count updated
- [ ] "Last Updated" date updated

### When Promoting an Epic from Backlog to Draft:
- [ ] Epic entry moved from Epic-backlog.md to Epic-active.md
- [ ] Acceptance criteria written
- [ ] Status changed to 🔵 Draft in Epic-active.md
- [ ] Status updated to 🔵 Draft in Epic-Documentation.md index
- [ ] "Last Updated" date updated

### When Creating a New Epic (directly as Draft):
- [ ] Epic entry added to Epic-active.md with full acceptance criteria
- [ ] Epic entry added to Epic-Documentation.md index table with 🔵 Draft status
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

## ⚠️ A table declared authoritative MUST reach the highest issued ID

✅ **Added 2026-09-15 — audit ruling [R-12]** (`../Audits/Audit-Rulings-20260915.md`).

⚠️ **THE CHECK IS ONE LINE: the highest row in the table == the highest Epic ID issued.**

⚠️ **WHY: this defect has now occurred in TWO layers.** ⚠️ **2026-08-19 F-08 — EP-034 missing from the
Epic index entirely.** ⚠️ **2026-09-15 F-01 — the All-Sprints table, which declares itself the SINGLE
SOURCE, had no row for SP-127 or SP-131–133 while claiming to be complete.**

⛔ **A table that silently stops is worse than no table: a reader trusts it BECAUSE it is declared
authoritative.** ✅ **Run the check whenever a Epic is closed or created.**

## ⚠️ THREE STANDING RULES — audit rulings [R-14], [R-15], [R-16] (2026-09-15)

### [R-14] ⛔ **Do NOT preserve prior text that states STATE**

⚠️ **Preserved prior text is as destructive as omitting it, when what it states is STATE** — ✅ **counts,
statuses, "next available" IDs, verification claims.**
⚠️ **PROOF: `Issue-active.md` carried SIX stacked *"Currently: N records"* lines — twenty-three,
twenty-two, twenty-one, twenty, eighteen, seventeen — ⛔ and NONE was correct** (the real count was 25).
⚠️ **A reader cannot tell which preserved line is current, so preservation MISLEADS.**
✅ **DELETE superseded state. ✅ PRESERVE rationale — *why* a decision was made — which does not decay.**

### [R-15] ⛔ **No restated summary counts, in ANY layer**

⚠️ **2026-08-19 removed per-status totals from the EPIC layer (R-22④, R-23②).** ⛔ **That fix was scoped
too narrowly:** ✅ **the Task and Sprint layers kept theirs, and those are exactly the two that drifted
into the 2026-09-15 audit's criticals** (⚠️ **Sprint claimed next-ID SP-127 when SP-134 was issued;
Task claimed 425 IDs when the highest was T-0534**).
✅ **THE RULE NOW APPLIES TO ALL FOUR LAYERS.** ⚠️ **A summary that restates a table WILL drift from it.**
✅ **Counts are obtained by READING THE ROWS.** ⚠️ **ID-accounting tables are NOT counts and are RETAINED
— they distinguish an unassigned ID from a lost record.**

### [R-16] ⛔ **Do NOT surface status mid-investigation**

✅ **A status line is written when the investigation CONCLUDES, not while it runs.**
⚠️ **PROOF, both caught by the USER and not by Claude:** ⚠️ **[T-0529]'s row claimed a test was "STILL
UNEXERCISED" for a full day AFTER the user had exercised it;** ⚠️ **[T-0521]'s row read
"Implemented - Not Verified" while the code was untouched.**
✅ **An in-flight finding may be recorded as NARRATIVE.** ⛔ **It must NOT be written into a status cell
that a later reader will treat as current.**
