# Documentation Audit — 2026-09-15

**Scope:** All four tracking layers in `docs/` — Issues, Tasks, Sprints, Epics — plus `docs/Audits/`.
Read bottom-up per `Audit-Guidelines.md` §Audit Order.
**Method:** ⚠️ **READ-ONLY. NOTHING WAS CHANGED.** All counts re-derived from the files by `grep`/
`python3` over `docs/**`, never taken from a stated figure. Working tree clean at `60443bd`
("Prior to Audit 20260915") — ✅ **so no uncommitted-state finding applies, unlike 2026-08-19.**
**Requested by:** the user, 2026-09-15, after asking *"How can I understand what is done and what is
not done"* — ✅ **the trigger was a real inability to read state off these files.**

**Result: 11 findings — 4 critical, 5 moderate, 2 minor.**

## Summary table

| # | Layer | File | Kind | Severity | One-line finding |
| - | ----- | ---- | ---- | -------- | ---------------- |
| F-01 | Sprints | `Sprint-Documentation.md:383` | Missing | **Critical** | The All-Sprints table — declared the SINGLE SOURCE — stops at SP-115; **SP-116–SP-134 have no row** |
| F-02 | Sprints | `Sprint-Documentation.md:406-407` | Stale | **Critical** | Statistics claims next-ID **SP-127** and **130 total**; actual next is **SP-135**, 134 issued |
| F-03 | Tasks | `Task-Documentation.md:61` | Stale | **Critical** | Statistics claims **425 Task IDs (T-0001–T-0426)**; actual highest is **T-0534** |
| F-04 | Issues | `Issue-verified-0181-0190.md` | Missing | **Critical** | Filename claims I-0181–I-0190; **file contains only I-0182–I-0186**. I-0181, I-0187–I-0190 are in NO archive |
| F-05 | Tasks | `Task-Documentation.md:555` | Contradiction | Moderate | Accounting names **T-0278** as the only never-assigned ID; the actual gap in the tree is **T-0138** |
| F-06 | Epics | `Epic-Documentation.md:108` | Contradiction | Moderate | EP-038's row says *"no Epic record file"*; `Epics/Closed/Epic-EP-038.md` **exists** |
| F-07 | Epics | `Epic-active.md` | Misplaced | Moderate | **EP-039 is CLOSED but still lives in `Epic-active.md`** |
| F-08 | Issues | `Issue-active.md` | Stale | Moderate | Stacked count lines say *twenty-three* and *twenty-two*; actual row count is **25** |
| F-09 | Tasks | `Task-Documentation.md:18` | Stale | Moderate | Known-gap note says **133 unfiled rows**; actual is **139** |
| F-10 | Sprints | `Sprint-Documentation.md` | Guidelines | Minor | *"Next available Sprint ID"* appears **13 times**, against a rule that it is recorded once |
| F-11 | Tasks | `Task-Documentation.md:52-54` | Stale | Minor | Active claimed **8**, actual **0**; Backlog claimed **13**, actual **5** |

---

## Findings

### F-01 — The All-Sprints table stops at SP-115

**Layer:** Sprints · **File:** `Sprint-Documentation.md:383` · **Kind:** Missing · **Severity: Critical**

**What is there:** The last `| SP-…` row in the All-Sprints table is **SP-115**. The file states at
`:400`: *"The All-Sprints table above is the SINGLE SOURCE for per-sprint status and counts (P7)."*

**What is true:** Sprints **SP-116 … SP-134** exist. `grep -cE "^\| SP-1[0-9]{2}"` returns rows only to
SP-115; `ls docs/Sprints/Closed/` holds **120** archives including SP-131, SP-132 and SP-133, all
closed 2026-09-15.

**Why it matters:** ⚠️ **Nineteen sprints — including every sprint of EP-039 — are invisible in the
file that declares itself the single source of sprint status.** A reader answering *"what sprints
exist and what state are they in"* from this table gets an answer that is nine months out of date.
⚠️ **This is the SAME shape as 2026-08-19's F-08** (*"EP-034 is missing from the Epic index
entirely"*) — ✅ **recurrence, in a different layer.**

**Options:** (a) append SP-116–134 rows from the closed archives; (b) regenerate the table from
`Sprints/Closed/` mechanically and keep a script; (c) demote the "single source" claim if the table
will not be maintained.

---

### F-02 — Sprint Statistics is stale in every figure

**Layer:** Sprints · **File:** `Sprint-Documentation.md:406-407` · **Kind:** Stale · **Severity: Critical**

**What is there:**
> *"**Next available Sprint ID:** **SP-127**"* and *"**Total Sprint IDs issued:** 130 (SP-001–SP-130)
> — 118 executed, ⚠️ **0 ACTIVE**"*

**What is true:** Highest sprint referenced anywhere is **SP-134**; next available is **SP-135**.
Closed archives number **120**. "0 ACTIVE" happens to be correct **today** — ⚠️ **by coincidence, not
maintenance**, since SP-131–133 were active after this line was written and it never changed.

**Why it matters:** ⚠️ **Someone creating the next sprint would take SP-127, which is already used.**
An ID collision is not recoverable by editing — it splits a record.

**Options:** (a) re-derive all three figures now; (b) delete the figures and point to the table
(the R-22(④) precedent that removed per-status totals for exactly this reason).

---

### F-03 — Task Statistics is stale by ~110 IDs

**Layer:** Tasks · **File:** `Task-Documentation.md:61` · **Kind:** Stale · **Severity: Critical**

**What is there:** *"**Total Task IDs** | **425** — T-0001 … T-0426; T-0278 never assigned;
**T-0426 unassigned**"*

**What is true:** Highest task ID in `docs/` is **T-0534** (re-derived by regex over all `.md`).
`Sprint-active.md` states next-available **T-0535**.

**Why it matters:** ⚠️ **Same collision risk as F-02**, and the figure understates the task corpus by
roughly a quarter. ⚠️ **A reader sizing the project from this file would be wrong by ~110 tasks.**

**Options:** (a) re-derive; (b) replace the total with a pointer to the ID-accounting table.

---

### F-04 — A Verified-Issues batch file does not contain the Issues its name claims

**Layer:** Issues · **File:** `Issues/Verified/Issue-verified-0181-0190.md` · **Kind:** Missing ·
**Severity: Critical**

**What is there:** A file titled *"Verified Issues — I-0181 … I-0190"*.

**What is true:** It contains entries for **I-0182, I-0183, I-0184, I-0185, I-0186 only**.
**I-0181, I-0187, I-0188, I-0189 and I-0190 appear in NO archive file.** Established by extracting
`^## I-0…` headings from the file. ⚠️ **I-0187 is referenced as real** (`Sprint-SP-127.md:81`,
*"Issue **I-0187**"*), ⚠️ **I-0190 is referenced as a decade boundary** (`Issue-verified-0191-0200.md:3`,
*"The previous decade closed at **I-0190**"*), ⛔ **and I-0188/I-0189 appear NOWHERE in `docs/` at all.**

**Why it matters:** ⚠️ **This is the filename-vs-contents check the guidelines name explicitly, and it
fails.** ⚠️ **I-0187 is cited by a closed Sprint as its Issue — if that record is lost, the sprint's
justification is unverifiable.** ⚠️ **2026-08-19's F-04 was the same defect** (*"I-0059 and I-0099 exist
in no file in `docs/` at all"*) — ✅ **and F-09 that year was the same again** (*"Batch 14's table lists
four Issues; the file holds two"*). ⚠️ **THIRD OCCURRENCE OF THIS CLASS.**

**Options:** (a) reconstruct the five from git history and file them; (b) record each in the Issue
ID-accounting table as never-assigned/superseded **if** history shows that; ⛔ **(c) do NOT rename the
file to match its contents — that would hide a lost record rather than surface it.**

---

### F-05 — The never-assigned Task ID is recorded as T-0278; the real gap is T-0138

**Layer:** Tasks · **File:** `Task-Documentation.md:555` · **Kind:** Contradiction · **Severity: Moderate**

**What is there:** *"| T-0278 | (never assigned — ID skipped) | … | ⚪ Never assigned |"*, and the
statistics line *"⚪ Never assigned | 1"*.

**What is true:** ⚠️ **BOTH T-0138 and T-0278 are absent** from every file in `docs/`. T-0137 and T-0139
both exist (`Task-verified-0137.md`; `Audit-Remediation-20260819.md`). **T-0138 is mentioned zero
times in `Task-Documentation.md`.**

**Why it matters:** ⚠️ **An unaccounted ID and a lost record are indistinguishable** — which is the
exact rationale the Issue layer gives for keeping an accounting table. One of these two is a numbering
skip and the other may be a lost task; the file asserts there is only one and names the wrong one.

**Options:** (a) establish T-0138's history from git and record its true disposition; (b) if it was
never assigned, add the row and correct the count to 2.

---

### F-06 — EP-038's index row denies a record file that exists

**Layer:** Epics · **File:** `Epic-Documentation.md:108` · **Kind:** Contradiction · **Severity: Moderate**

**What is there:** *"| EP-038 | `[Linux]` Linux Test Rig | 🟠 **Complete** (2026-09-10 ruling) —
⚠️ **no Epic record file** | — | — |"*

**What is true:** **`docs/Epics/Closed/Epic-EP-038.md` exists.**

**Why it matters:** ⚠️ **THIS FINDING IS ABOUT MY OWN EDIT.** I added that row on 2026-09-14 while
filling gaps in the index, asserted the record was missing without checking `Epics/Closed/`, and
flagged it as reconstructed. ✅ **The flag was honest; ⛔ the claim was false.** ⚠️ **A reader would
conclude a closed Epic's record was lost when it is on disk.**

**Options:** (a) correct the row and link the record; (b) additionally re-check the other rows added in
that same edit (EP-035–EP-037), which were written by the same unverified method.

---

### F-07 — A closed Epic still lives in `Epic-active.md`

**Layer:** Epics · **File:** `Epic-active.md` · **Kind:** Misplaced · **Severity: Moderate**

**What is there:** EP-039's full record, headed *"**Status:** ✅ **CLOSED 2026-09-15**"*, inside
`Epic-active.md`. The file's own opening line describes it as holding active Epics.

**What is true:** EP-039 closed 2026-09-15. `Epics/Closed/` contains no `Epic-EP-039.md`.
✅ **By the pattern of every other closed Epic, its record belongs there.**

**Why it matters:** ⚠️ **`Epic-active.md` now holds one active Epic (EP-040) and one closed one**, so
the file no longer answers *"what is active"* without reading each status line. ⚠️ **This is the same
layer-discipline failure the memory note `feedback_archive_on_close` records as having rotted four
files by 2026-08-15.**

**Options:** (a) move EP-039 to `Epics/Closed/Epic-EP-039.md`; (b) leave it pending the EP-040 close
and accept the ambiguity (⛔ **not recommended — that is how the rot started**).

---

### F-08 — `Issue-active.md` carries two stale count lines and neither is right

**Layer:** Issues · **File:** `Issue-active.md` · **Kind:** Stale · **Severity: Moderate**

**What is there:** Two stacked lines — *"## Currently: **twenty-three records**"* followed by a
preserved *"(prior count line follows) ## Currently: **twenty-two records**"*, and further stacked
predecessors below.

**What is true:** **25** rows match `^\| \*\*I-0`.

**Why it matters:** ⚠️ **The preserve-the-prior-line habit has produced a stack of contradicting
counts in one file**, none of which is correct. ✅ **The intent (keep history) is good**; ⚠️ **the effect
is that the top line cannot be trusted and the reader must count manually — which is what the count
exists to avoid.**

**Options:** (a) re-derive and collapse to one line, moving history to a changelog section; (b) delete
the count and state *"count by reading the rows"* (the R-23(②) precedent in the Epic layer).

---

### F-09 — The known-gap note undercounts the unfiled tasks

**Layer:** Tasks · **File:** `Task-Documentation.md:18` · **Kind:** Stale · **Severity: Moderate**

**What is there:** *"**133 rows still read ⚠️ unfiled**, all older Tasks (roughly T-0090–T-0357)"*

**What is true:** **139** rows carry *unfiled* in the archive column (extracted 2026-09-15). ✅ **All
139 are status ✅ Verified** — ⚠️ **this is a filing gap, not unfinished work.** They span 13 Epics;
EP-025 and EP-027 carry 29 each.

**Why it matters:** ⚠️ **Minor as a number, but the note is the ONLY place the gap is described**, and
it was used this session to decide the gap was already known and scoped out. ✅ **That decision was
right; ⚠️ the figure supporting it was not.**

**Options:** (a) re-derive the count; (b) attach the extracted inventory
(`~/Desktop/Scrivi-Unfiled-Tasks.csv`, generated 2026-09-15) so the set is enumerable, not just counted.

---

### F-10 — "Next available Sprint ID" is stated 13 times

**Layer:** Sprints · **File:** `Sprint-Documentation.md` · **Kind:** Guidelines · **Severity: Minor**

**What is there:** 13 lines contain *"Next available Sprint ID"*, with values SP-085, SP-090, SP-106,
SP-117, SP-119, SP-120, SP-121, SP-127.

**What is true:** ⚠️ **Most are inside dated changelog entries and are historically correct in place.**
⚠️ **But `:537-538` of that same file records a prior fix for exactly this** — *"header carried the
'Next available Sprint ID' sentence twice, contradicting the rule that it is recorded once, in
Statistics."*

**Why it matters:** ⚠️ **The rule is stated and the file still violates its spirit** — a `grep` for the
next ID returns eight different answers, and only one is current. ⚠️ **F-02 shows that one is wrong.**

**Options:** (a) leave changelog instances (they are dated history) and fix only Statistics;
(b) add a convention that changelog entries write *"next available at the time: SP-xxx"*.

---

### F-11 — Task status counts do not match the layers

**Layer:** Tasks · **File:** `Task-Documentation.md:52-54` · **Kind:** Stale · **Severity: Minor**

**What is there:** *"🟡 Active | **8** — re-derived 2026-08-25"*, *"🔵 Backlog | **13**"*,
*"🟠 Implemented - Not Verified | **0**"*

**What is true:** `Task-active.md` holds **0** task rows; `Task-backlog.md` holds **5**;
`Task-unverified.md` holds **1** (T-0521).

**Why it matters:** ⚠️ **Low consequence — the figures are dated and self-describe as a 2026-08-25
re-derivation.** ✅ **Recorded for completeness; ⚠️ note that "Implemented - Not Verified = 0" is now
wrong, and `scripts/work-state.sh` finds six tasks carrying that status across the tree.**

**Options:** (a) re-derive; (b) remove per-status totals per the R-22(④) precedent.

---

## Guidelines recommendations

**G-1 — Add "the table must extend to the highest issued ID" to the Sprint and Epic guidelines.**
⚠️ **F-01 and 2026-08-19's F-08 are the same defect in two layers:** a table declared authoritative
that silently stops. ✅ **A one-line check** (*highest row == highest issued ID*) **catches both.**

**G-2 — `Audit-Guidelines.md` should require the ID-continuity check to use BOTH row formats.**
⚠️ **My first pass reported 167 missing Issues** because archived entries use `## I-0xxx:` headings
while active ones use `| **I-0xxx** |` table rows. ✅ **A checker that knows only one format reports a
catastrophe that is not there** — ⚠️ **and might prompt a destructive "fix".**

**G-3 — Record a convention for preserved prior text.** ⚠️ **F-08 shows the
*"(prior line follows)"* habit stacking contradictory counts in one file.** ✅ **Prior text is worth
keeping; ⚠️ it should live under a dated heading, not immediately beneath the line it contradicts.**

---

## Systemic observations

**S-1 — Statistics blocks are the single most reliable place to find staleness, again.**
⚠️ **Three of the four criticals (F-01, F-02, F-03) are summary figures that drifted from the files
they summarise.** ✅ **The 2026-08-19 audit reached the same conclusion and removed per-status totals
under R-22(④)/R-23(②).** ⚠️ **Those removals were NOT applied to the Task and Sprint layers, and those
are exactly the layers that drifted.** ✅ **The earlier fix was right; ⛔ it was applied too narrowly.**

**S-2 — Status lines written mid-investigation go stale within hours.**
⚠️ **Two cases this session, both caught by the USER, not by me:** ⚠️ **T-0529's row claimed a test was
"STILL UNEXERCISED" for a day after the user had exercised it;** ⚠️ **T-0521's row read
"Implemented - Not Verified" while the code was untouched.** ✅ **A status line is a claim about code and
must be re-verified against the code, not carried forward.**

**S-3 — The layers cannot currently answer "what is done?" without manual counting.**
⚠️ **This is why the user requested the audit.** ✅ **`scripts/work-state.sh` (written 2026-09-15) reads
every layer and reports disagreements**; ⚠️ **it is NOT a tracking document and this audit does not
propose it as one** — ✅ **recorded because it is how F-11 and the six mis-stated "Implemented" rows
were found.**

**S-4 — Recurrence is real and measurable.** ⚠️ **F-01 recurs 2026-08-19's F-08; F-04 recurs its F-04
AND F-09.** ✅ **Keeping the findings file, as the guidelines require, is what made this visible.**

---

## Open questions for the user

**Q-1 — I-0187, I-0188, I-0189, I-0190, I-0181: reconstruct or account?** ⚠️ **I-0187 is cited by a
closed Sprint; I-0188/I-0189 appear nowhere at all.** ⚠️ **Reconstructing from git is real work;
recording them as never-assigned is cheap but may bury a lost record.** ⛔ **I have not looked at git
history for these — that is remediation, not audit.**

**Q-2 — Should EP-039 move to `Epics/Closed/` now, or after EP-040 closes?** ⚠️ **F-07.**

**Q-3 — Do the Task and Sprint Statistics blocks get re-derived, or removed?** ⚠️ **The Epic layer
REMOVED them in 2026-08-19 precisely because summaries drift (S-1).** ✅ **Consistency argues removal;
⚠️ but these blocks carry ID-accounting that the tables do not.**

**Q-4 — Does the 139-row unfiled backlog get filed, or formally accepted?** ⚠️ **It has now been
deferred by two audits.** ✅ **All 139 are Verified work with a sprint-archive record** — ⚠️ **the
question is whether a Task-level archive is required for completeness or is ceremony.**
