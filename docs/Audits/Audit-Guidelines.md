# Audit Guidelines

## Purpose

An **Audit** is a complete, read-only examination of every tracking document in `docs/` — Issues, Tasks,
Sprints, and Epics — checking each file for entries that are **stale**, **missing**, **misplaced**, or
**contradictory**, and recording what was found without changing anything.

The tracking system has four layers (Epics → Sprints → Tasks/Issues) spread across \~200 files, and the
same fact is restated in many of them. Restatements drift. An Audit is the mechanism that finds the drift
before it is acted on.

---

## ⚠️ The Prime Rule: an Audit CHANGES NOTHING

**An Audit produces exactly one artifact: a findings file. It does not fix what it finds.**

This is not a stylistic preference. Three things go wrong when an audit repairs as it goes:

1. **Partial repair is worse than none.** An audit interrupted halfway leaves the tree in a state that is
   neither the old consistent-but-stale one nor a new correct one, and no record of which is which.
2. **A repair needs a decision the auditor may not have.** "This row is stale" is a finding.
   *"…so delete it"* is a ruling. Many findings have two defensible fixes.
3. **The findings file is the review surface.** The user reads findings and rules on them. A fix applied
   during the audit was never reviewed.

**Fixes are a separate, subsequent pass, working from an approved findings file.**

> ⚠️ **This rule exists because it was broken.** The 2026-08-19 audit began editing files mid-examination
> and had to be stopped by the user. The edits it had already made became a finding *about themselves*
> (see §"Uncommitted state" in `Audit-Findings-20260819.md`).

---

## Authorization

### What Claude CAN do during an Audit:
- ✅ Read every file in `docs/`
- ✅ Run read-only shell commands (`grep`, `ls`, `diff`, counts)
- ✅ Create the findings file in `docs/Audits/`
- ✅ Recommend changes to any GUIDELINES file

### What Claude CANNOT do during an Audit:
- ❌ Edit, create, move, or delete **any** tracking document
- ❌ Change any status, count, or date
- ❌ Mark anything Verified, Closed, or Archived
- ❌ Apply a fix "because it is obviously right"

---

## Audit Order

Audits proceed **bottom-up**: the layers that carry the most detail first, so that by the time the summary
layers are read, the auditor already knows what they should say.

Within each layer: **guidelines first** (they define correct), then **archives** (the record of truth),
then **the summary/working files** (the things that go stale).

### 1. Issues
1. `Issue-GUIDELINES.md`
2. `Issues/Verified/` — every batch file
3. `Issues/Closed/` — every file
4. `Issue-active.md`
5. `Issue-backlog.md`
6. `Issue-Documentation.md`

### 2. Tasks
1. `Task-Guidelines.md`
2. `Tasks/Verified/` — every file
3. `Tasks/Closed/` — every file
4. `Task-backlog.md`
5. `Task-active.md`
6. `Task-unverified.md`
7. `Task-Documentation.md`

### 3. Sprints
1. `Sprint-GUIDELINES.md`
2. `Sprints/Closed/` — every archive
3. `Sprint-backlog.md`
4. `Sprint-active.md`
5. `Sprint-Documentation.md`
6. ⚠️ **Any file in `Sprints/` that is not one of the above** — loose `Sprint-SP-*.md` and
   `*-planning.md` files. Establish for each: is it a duplicate of an archive, a pre-close draft, or the
   only record of something? Do not assume.

### 4. Epics
1. `Epic-GUIDELINES.md`
2. `Epics/Closed/` — every archive
3. `Epic-backlog.md`
4. `Epic-active.md`
5. `Epic-Documentation.md`

### 5. Audits
1. `Audit-Guidelines.md` (this document)
---

## What to look for

Every finding is classified as one of five kinds:

| Kind              | Definition                                       | Typical example                                  |
| ----------------- | ------------------------------------------------ | ------------------------------------------------ |
| **Stale**         | Was true, no longer is                           | An index says a Sprint is Active after it closed |
| **Missing**       | Should exist, does not                           | A Verified Issue with no archive entry           |
| **Misplaced**     | Exists in the wrong file/layer                   | An Active Task sitting in the backlog            |
| **Contradiction** | Two files assert incompatible facts              | Backlog says *Descoped*, index says *Verified*   |
| **Guidelines**    | The rule itself is wrong, unclear, or unfollowed | A checklist naming a file that does not exist    |

### Checks that reliably find drift

**Counts vs. reality.** Never trust a stated count. Re-derive it from the files. Statistics blocks are
the single most reliable place to find staleness.

**Cross-layer status agreement.** A Sprint's status appears in `Sprint-active.md`, `Sprint-backlog.md`,
`Sprint-Documentation.md` (twice — header *and* All-Sprints table), and `Epic-active.md`. All five must
agree. ⚠️ **The lower tables of a long index drift while its own header stays correct** — always read the
whole file, not the top.

**ID continuity.** For each of I/T/SP/EP, enumerate every ID from 1 to the highest issued and account for
each one: filed, archived, closed, superseded, skipped, or ⚠️ **unaccounted**. An ID that appears nowhere
is either a lost record or a numbering error, and both matter.

**Filename vs. contents.** A batch file named `-0011-0020` should contain those IDs. Check.

**The "record of truth" claim.** When a row says *"see the Sprint archive"*, open the archive and confirm
it says what the row claims.

**Orphan files.** Anything in a tracking folder that the guidelines do not name.

---

## The findings file

**Name:** `docs/Audits/Audit-Findings-YYYYMMDD.md`

**Required structure:**

```markdown
# Documentation Audit — YYYY-MM-DD

**Scope:** [what was examined]
**Method:** [read-only; how counts were derived]
**Result:** N findings — X critical, Y moderate, Z minor

## Summary table
| # | Layer | File | Kind | Severity | One-line finding |

## Findings
### F-NN — [title]
**Layer / File / Kind / Severity**
**What is there:** [the current text, quoted]
**What is true:** [the verified fact, with how it was established]
**Why it matters:** [consequence if acted on]
**Options:** [the defensible fixes — NOT a decision]

## Guidelines recommendations
## Systemic observations
## Open questions for the user
```

**Every finding must be independently checkable.** Cite `file:line`. State how the fact was established
(*"counted `^## I-0` headings across `Verified/`"*), never "I noticed."

**Severity:**
- **Critical** — a reader acting on this document would make a wrong decision, or a record is lost
- **Moderate** — contradiction or staleness that misleads but does not destroy
- **Minor** — cosmetic, count drift, formatting

**Findings are numbered `F-01…` within a single audit.** They are not a tracked layer and do not get
Issue or Task IDs unless the user rules that a finding becomes one.

---

## After the audit — the three phases

An audit is phase one of three. Each phase produces its own artifact and \*\*must complete before the next
begins.\*\*

| Phase              | Artifact                                               | Who decides             | Changes tracking docs? |
| ------------------ | ------------------------------------------------------ | ----------------------- | ---------------------- |
| **1. Audit**       | `Audit-Findings-YYYYMMDD.md`                           | Claude observes         | ❌ Never                |
| **2. Rulings**     | `Audit-Rulings-YYYYMMDD.md`                            | **User decides**        | ❌ Never                |
| **3. Remediation** | `Audit-Remediation-YYYYMMDD.md` + the fixes themselves | Claude executes rulings | ✅ Yes                  |

⚠️ **Phase 2 changes nothing either.** A ruling is a *decision recorded*, not a fix applied. This keeps
the decision record intact even if remediation is interrupted, redone, or done months later.

The findings file is **kept**, not deleted — it is the record of what the tree looked like on that date,
and the next audit reads it to see what recurred.

### ⚠️ Verification is carried by the PHASE GATES

**Each approval to proceed IS the verification of the artifact behind it.** There is no separate sign-off
step, and none should be invented:

| Artifact | Verified by |
| -------- | ----------- |
| `Audit-Findings-YYYYMMDD.md` | the user's approval to **begin the rulings session** |
| `Audit-Rulings-YYYYMMDD.md` | the user's approval to **begin remediation** |
| `Audit-Remediation-YYYYMMDD.md` | the user's **review of the log** |

This follows from the phase rule itself: **a phase cannot begin until the previous one is complete**, so
authorising the next phase is an assertion that the previous artifact is sound. Adding a fourth ceremony
would restate a decision already made — the same disease as **P7**.

⚠️ **What this does NOT relax:** Claude still marks entries 🟠 **Applied - Not Verified** and never ✅
Verified on its own initiative. The gate is the *user's* approval; Claude records it, having been told.

> *Named by the user at the close of the 2026-08-19 cycle — the process already had this property; it had
> simply not been written down.*

---

## Phase 2 — The Rulings session

**Name:** `docs/Audits/Audit-Rulings-YYYYMMDD.md` — same date as the findings file it answers.

**Rulings correspond ONE-TO-ONE with findings.** `R-07` rules on `F-07`, always. If a finding needs no
action, it still gets a ruling saying so. **A findings file with 31 findings ends with 31 rulings** —
never fewer, so a gap is immediately visible as unfinished work rather than a silent skip.

### How the session runs

**Claude presents findings ONE AT A TIME, in order, and asks for the ruling.** Not in batches, not as a
questionnaire. Each turn:

1. Claude states the finding: what is there, what is true, why it matters, and the defensible options.
2. Claude gives **a recommendation and its reasoning** — the user is entitled to Claude's judgement, not
   just a menu. ⚠️ **A recommendation is not a decision**; if the user rules otherwise, that is the ruling.
3. The user rules.
4. ⚠️ **Claude writes the ruling to the file immediately, before presenting the next finding.**

### ⚠️ Why the file is written incrementally

**So the session can be interrupted at any point and resumed exactly where it stopped.** The rulings file
*is* the session state. A session that batches its writes to the end loses everything on interruption,
and these sessions are long — 31 findings is 31 decisions, and a user may not have them all in one
sitting.

**On resume:** read the rulings file, find the highest `R-nn` present, present finding `nn+1`. No
re-litigation of what is already ruled.

### Ruling entry format

```markdown
### R-NN — [finding title]
**Rules on:** F-NN · **Severity:** [carried from the finding]
**Ruling:** [the user's decision, stated as an instruction]
**Rationale:** [why — the user's reasoning, in their terms, when given]
**Action:** [what remediation must do; "none" is valid]
**Status:** 🔵 Ruled — not yet applied
**Date:** YYYY-MM-DD
```

**Status values:** 🔵 Ruled (decided, not applied) → 🟠 Applied - Not Verified → ✅ Verified.
⚠️ **Only the user marks a ruling Verified**, as with every other layer.

### Rules for the session

- ✅ Claude may record the user's ruling verbatim, and may ask a clarifying question before writing.
- ✅ Claude **must** flag when a ruling contradicts an earlier ruling in the same session.
- ✅ A ruling may **defer** — *"leave as-is, re-examine next audit"* is a legitimate decision and should be
  recorded as one rather than left blank.
- ✅ A ruling may **create tracked work** — *"this becomes Issue I-XXXX"* or *"this becomes a Task."*
  ⚠️ Record the new ID in the ruling so the finding and the tracked item stay connected.
- ❌ Claude must **not** apply any fix during the rulings session, including "obvious" ones.
- ❌ Claude must **not** re-order findings to group them, unless the user asks. \*\*The order is the
  findings file's order\*\* — it is how both parties keep their place.

### Guidelines recommendations and open questions

The findings file's **Guidelines recommendations** (`G-n`) and **Open questions** are ruled the same way,
after the `F-nn` findings, numbered `R-G1…` and `R-Q1…`. They are part of the same session and the same
file.

---

## Phase 3 — Remediation

Works **only** from an approved rulings file. Each change cites its ruling number. When a ruling's action
is complete, its status moves to 🟠 **Applied - Not Verified**; the user moves it to ✅ **Verified**.

⚠️ **A finding that recurs across audits is a systemic problem, not a clerical one**, and should be
raised as a guidelines change rather than fixed a third time.

### ⚠️ The Remediation Log is MANDATORY

**Every remediation pass creates `docs/Audits/Audit-Remediation-YYYYMMDD.md`** — matching the date of the
findings and rulings files it completes. \*\*It is written as the work proceeds, not reconstructed
afterwards.\*\*

**Why a log, when git already records every change:** git records *what bytes changed*. It does not record
**which ruling authorised a change**, **why a ruling was applied the way it was**, or ⚠️ \*\*what was
deliberately NOT done and on whose authority.\*\* A remediation pass touches dozens of files under thirty-odd
rulings with ordering constraints between them; the git history of that is a wall of edits with no
decision trail. **The log is the record a human can read.**

It also makes remediation **resumable**. Like the rulings session, a remediation pass can be interrupted —
and the ordering constraints mean *where it stopped* determines what is safe to do next. ⚠️ \*\*A half-applied
remediation with no log is worse than one not started\*\*, because the tree is in a state neither the old
records nor the new ones describe.

**Required structure:**

```markdown
# Audit Remediation — YYYY-MM-DD

**Applies:** Audit-Rulings-YYYYMMDD.md (N rulings)
**Status:** 🟡 In progress | 🟠 Complete - Not Verified | ✅ Verified

## Progress
| Rulings applied | n of N |
| Session status  | where it stopped, and what is safe to do next |

## Ordering constraints observed
[the constraints from the rulings file, ticked as satisfied]

## Applied
### R-NN — [title]
**Files changed:** [paths]
**What was done:** [the actual change]
**Deviations:** [anything not done as ruled, and why — or "none"]
**Verification:** [how it was confirmed: grep, count, diff]
**Status:** 🟠 Applied - Not Verified

## Not applied / deferred
[ruling number, reason, what is still owed]
```

**Rules:**
- ⚠️ **Write each entry as its ruling is applied**, before moving to the next — the log is the session
  state.
- ⚠️ **Record deviations honestly.** If a ruling could not be applied as written, say so and say why.
  A remediation log that reports only successes is not evidence, it is decoration.
- **Verification is per-ruling**, and mechanical where possible (grep, count, diff) — cite it.
- ⚠️ **Claude marks entries 🟠 Applied - Not Verified. Only the user marks them ✅ Verified.**
- The log is **kept**, not deleted. With the findings and rulings files it forms the complete record of one
  audit cycle.

---

# The Audit Check — a lightweight alternative

⚠️ **An Audit is large and formal. Most situations do not warrant one.** A full Audit reads four layers
across \~200 files and produces dozens of findings; the 2026-08-19 audit consumed an entire session and
produced 31 findings and 31 rulings. **Running that on a schedule would cost more than it saves.**

An **Audit Check** is the lightweight instrument: a **mechanical, read-only sweep** for documentation
inconsistency, taking minutes rather than a session.

|             | **Audit**                                        | **Audit Check**                        |
| ----------- | ------------------------------------------------ | -------------------------------------- |
| **Trigger** | ⚠️ **Only when the user requests it**            | Before an Epic close; or any time      |
| **Scope**   | Every file in all four layers                    | Only the mechanical checks below       |
| **Method**  | Reading, judgement, git archaeology              | **Greps and counts**                   |
| **Cost**    | A full session                                   | Minutes                                |
| **Output**  | `Audit-Findings-YYYYMMDD.md` + a rulings session | A short list of inconsistencies        |
| **Rulings** | Its own Phase 2 session                          | ⚠️ **Ruled as part of the Epic close** |

### When to run an Audit Check

**Before an Epic close.** ⚠️ **This is a Check, NOT an Audit** — an Epic close does not trigger a formal audit. The close reads every layer as evidence, so it is worth confirming mechanically that the layers agree with each other first.

**The 2026-08-19 audit is the evidence for why.** Two findings would have corrupted EP-031's close:
- **F-23** — `Epic-Documentation.md` claimed **AC9 was outstanding**; it had been met two days earlier.
  SP-100's whole job is verifying AC1–AC10.
- **F-03** — **I-0118**, named in `Epic-active.md` as *"available as evidence for SP-100's AC pass"*, had
  **no archive entry anywhere.**

⚠️ **Both are mechanical, not matters of judgement.** \*Does every AC's stated status match the layer
below it? Does every Issue cited as evidence have an archive entry?\* **Those are greps.** They do not need
a session; they need a checklist.

### The checks

1. **AC status agreement** — every AC's state in `Epic-Documentation.md` matches `Epic-active.md`.
2. **Evidence exists** — every Issue/Task cited as evidence for an AC has an archive entry (not merely a
   table row — **P4**).
3. **Sprint status agreement** — each Sprint's status is identical in every file that states it.
4. **Counts** — every statistics figure is re-derived, never trusted (**P6**).
5. **Table/entry parity** — batch-file table rows equal `grep -c '^## I-0'` (**P4**).
6. **Orphan files** — anything in a tracking folder the guidelines do not name.
7. **ID continuity** — every ID accounted for: filed, archived, closed, superseded, or skipped.

### Rules

- ⚠️ **Read-only.** The Prime Rule applies unchanged: **a Check changes nothing.**
- ⚠️ **Findings from a Check are ruled AS PART OF THE EPIC CLOSE**, not in a separate rulings session.
- **No findings file is required.** A short list in the close discussion is enough. ⚠️ \*\*If a Check turns
  up something large or systemic, that is grounds to RECOMMEND a full Audit\*\* — which still only begins
  when the user requests it.

---

## Cadence

Audit only:
Upon request.  Claude never begins an audit automatically.  If Claude detects possible stale, misplaced, or missing entries, contradictions or drift, Claude will recommend an audit be performed.  

---

\*Version 1.0 · Created 2026-08-19 · Process defined by the user after four consecutive days of documentation defects.\*
