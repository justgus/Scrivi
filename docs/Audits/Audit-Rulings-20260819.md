# Documentation Audit — Rulings — 2026-08-19

**Rules on:** [`Audit-Findings-20260819.md`](Audit-Findings-20260819.md) — 31 findings (F-01…F-31),
11 guidelines recommendations (G-1…G-11), 8 open questions (Q1…Q8).

**Process:** [`Audit-Guidelines.md`](Audit-Guidelines.md) §"Phase 2 — The Rulings session".

⚠️ **This document records DECISIONS ONLY. Nothing here has been applied.** Remediation is a separate
pass that works from this file and cites ruling numbers.

⚠️ **Rulings are written one at a time, as the session proceeds**, so the session can be interrupted and
resumed exactly where it stopped. **To resume: find the highest `R-nn` below and continue at the next
finding.**

---

## Progress

| | |
| - | - |
| **Findings ruled** | ✅ **31 of 31 — ALL FINDINGS RULED** (F-01…F-31) |
| **Guidelines ruled** | **2 of 11 explicitly** (G-7 — R-25; G-10 — R-29); **G-1…G-6, G-8, G-9, G-11 absorbed** into R-11/R-14/R-15/R-18/R-19/R-22 — see below |
| **Questions ruled** | ✅ **8 of 8** |
| **Session status** | 🟠 **Findings complete** — awaiting a decision on the **remaining G-rulings** (below), then Phase 3 |
| **Last ruling** | **R-31** — add the Audits layer to CLAUDE.md; ⚠️ **do not restate P1–P7 there** (2026-08-19) |

### Remaining G-recommendations — status

Most were **absorbed** by findings rulings rather than needing their own:

| G | Recommendation | Disposition |
| - | -------------- | ----------- |
| G-1 | Fix `Documentation/` → `docs/` paths | ✅ **Absorbed — R-14 + R-19** |
| G-2 | One status-emoji convention (🟠) | ✅ **Absorbed — R-15 + R-19** (adopted in lockstep) |
| G-3 | Remove `DR-`/`ER-` leftovers | ✅ **Absorbed — R-15 + R-19** |
| G-4 | Fix checklists naming `Issue-unverified.md` | ✅ **Absorbed — R-15** (item 4) |
| G-5 | Rule for Sprint planning drafts | ✅ **Absorbed — R-22** (change ①) |
| G-6 | Document "fully planned while 🔵 Planning" | ✅ **Absorbed — R-22** (change ②) |
| G-7 | Index compactness — enforce or drop | ✅ **RULED — R-25**: enforce, unamended |
| G-8 | Statistics re-derived, never adjusted | ✅ **Absorbed — R-18 (P6) + R-22 (P7)** |
| G-9 | New batch file gets its index row in the same edit | ✅ **Absorbed — R-11** (action 4) + **P4** |
| G-10 | Cross-reference the Audit layer | ✅ **RULED — R-29** (+ the Audit Check) |
| G-11 | Record intentionally-unused IDs | ✅ **Absorbed — R-04** (action 1) |

⚠️ **No G-recommendation is outstanding.** Each is either ruled directly or carried inside a findings
ruling that names it.

> ⚠️ **New instrument created at R-29: the AUDIT CHECK.** A read-only, minutes-long, **mechanical** sweep
> (7 greps/counts) run before an Epic close — **not** a formal Audit, which still begins **only on user
> request**. Its findings are **ruled as part of the Epic close**. Already written into
> `Audit-Guidelines.md`.

> ✅ **R-22 DISCHARGED both structural mandates** raised earlier in the session — R-17's (the close
> checklist omitted 3 of the 4 files that drifted) and R-21's (three recurrences of restated-table drift).
> **P7** is the principle that came out of it.

> ⚠️ **Two findings have been corrected or disproved by the rulings themselves** — **F-12** (the "6" was
> deliberate, not stale) and **F-13** (the end-of-decade pattern was an artifact of range-naming, not a
> filing bug, per R-04's git evidence). **A findings file is a hypothesis set, not a verdict set.**

> ⚠️ **Remediation ordering constraints so far:**
> **R-03 before R-05** — move I-0118's primary-source text before deleting the block that holds it;
> deleting first would destroy it, the exact failure that created F-03.
> **R-07 before R-20** — rescue SP-039 before the other 12 loose Sprint files are disposed of.
> **R-06 forces a full re-derivation** of `Task-Documentation.md`'s statistics under R-18 — do not patch
> individual lines to preserve the total.
> **R-08 and R-23 in one file-open** — both correct `Epic-Documentation.md`; R-23 is not yet ruled.

### ⚠️ Standing principles established during this session

*These outlive the audit and are to be written into the guidelines (see R-G rulings).*

| # | Principle | From |
| - | --------- | ---- |
| **P1** | **Parity is DIRECTIONAL. Apple is the source of truth; Linux conforms to Apple.** A difference between platforms is **not** a defect and must not be filed as an Issue. | R-01 |
| **P2** | **NEVER rescope an Issue.** If the behaviour changes, verify and close the original and open a **new** ID. An Issue ID is a permanent handle on one defect. | R-02 |
| **P3** | **Reconstruction ≠ back-filling.** Restoring a **deleted** record from surviving **primary** sources, under **QA observation** and marked as reconstructed, is legitimate. Manufacturing never-written records in bulk from secondary sources is not. | R-03 |
| **P4** | **A batch file's summary table and its full entries are written in the SAME edit.** A row without its entry claims a record exists when it does not. **Mechanical check: table rows must equal `grep -c '^## I-0'`** — this caught both F-03 and F-09. | R-09 |
| **P5** | **A status file states its state ONCE.** If a table already answers the file's question, prose restating the same fact is a second source of truth that will drift from the first. **Do not narrate a table.** | R-16 |
| **P6** | **Statistics are DERIVED, never balanced.** Every line is counted from the files it describes. **A total that "comes out right" is not evidence the lines are right** — it is often evidence that one was adjusted to make it so. | R-18 |
| **P7** | ⚠️ **A summary that restates a table will drift from it. Derive it or delete it.** Keep in a statistics block only what the table **cannot** express. **The single most repeated defect in these docs** — correctly diagnosed in writing at least twice before finally being acted on. | R-22 |

**Status legend:** 🔵 Ruled — not yet applied · 🟠 Applied - Not Verified · ✅ Verified

---

## §0 — Uncommitted state (must be ruled first)

*This is not a numbered finding; it is the precondition for everything else, because seven files carry
uncommitted edits from before the audit and several findings describe those files.*

**Ruling:** **Option (b) — keep Group A, revert Group B.**

**Detail:**

| Group | Files | Disposition |
| ----- | ----- | ----------- |
| **A — SP-100 planning work** | `Sprints/Sprint-active.md`, `Sprints/Sprint-backlog.md`, `Epics/Epic-active.md`, `Tasks/Task-backlog.md`, `Sprints/Sprint-Documentation.md` | ✅ **KEEP** |
| **B — half-finished** | `Issues/Verified/Issue-verified-0131-0140.md`, `Tasks/Task-Documentation.md` | ⛔️ **REVERT** (`git checkout`) |

**Rationale:** Group A is the work that was requested and completed coherently — the SP-100 plan, its four
rulings R1–R4, T-0418, and the AC1/AC10 amendments, which are substantive analysis rather than clerical
edits. Group B is genuinely half-done: `Issue-verified-0131-0140.md` would ship a table listing four
Issues over a file holding two entries, **indistinguishable from the I-0118 defect in F-03**.

**Action for remediation:**
1. `git checkout docs/Issues/Verified/Issue-verified-0131-0140.md docs/Tasks/Task-Documentation.md`
2. Leave the five Group A files in place.
3. ⚠️ **Correct the factual error inside Group A:** `Sprints/Sprint-active.md` §2 states I-0133 and I-0134
   were not adopted into SP-100 because one was `Resolved - Not Verified` and the other 🔴 Open. **Both are
   Verified** (user, 2026-08-19). That reasoning was drawn from the stale rows in F-01 and is wrong on its
   face; the *conclusion* (not EP-031 work) may still hold, but it must be restated on correct grounds.
4. ⚠️ **Group B's reverted content is still owed** — reverting does not archive I-0133/I-0134. That work is
   ruled under **R-01**, not here.

**Status:** 🔵 Ruled — not yet applied
**Date:** 2026-08-19

---

---

## Rulings

*Rulings appear here in findings order as the session proceeds.*

### R-01 — I-0133 and I-0134 shown unverified

**Rules on:** F-01 · **Severity:** Critical

**Ruling:** **Two different dispositions.**

**I-0133 → ✅ VERIFIED. Archive it.** The user verifies the code was removed; that is the whole of the
Issue, so it is verified.

**I-0134 → ⛔️ CLOSED as a NON-ISSUE. Archive to `Issues/Closed/`**, with a note recording that **the
Apple implementation is authoritative.**

**Rationale (user, verbatim in substance):** *"I-0134 should never have been written to start with. It
involves parity with Linux, which is an erroneous requirement. Linux is required to have parity with
Apple, so we're first working on the 'source of truth' for the app and will make changes to Linux later."*

⚠️ **This corrects a premise in the finding itself.** F-01 framed I-0134 as a symmetric cross-platform
disagreement requiring arbitration between two verified behaviours, and offered three readings of what
"Verified" might mean. **The framing was wrong.** Parity is directional: **Apple is the source of truth;
Linux conforms to it later.** There was never a conflict to arbitrate — only Linux work not yet scheduled.
An Apple behaviour and a Linux behaviour differing is therefore **the expected state**, not a defect.

**Action for remediation:**
1. Write I-0133's full entry into `Issues/Verified/Issue-verified-0131-0140.md` (batch 14) and add its
   table row. ⚠️ Batch 14's table must list exactly what the file contains (see **F-09**).
2. Create `Issues/Closed/Issue-closed-0134.md` — one file per closed Issue, per
   `Issue-GUIDELINES.md:208`. Reason for closure: **non-issue; erroneous parity premise.** Record that
   **Apple is authoritative** and that Linux conformance is future work, not a defect.
3. Remove **both** rows from `Issue-active.md`; the active count becomes **0** once F-02 is applied.
4. Add I-0134's row to the **Closed Issues** table in `Issue-Documentation.md`
   (currently lists only `Issue-closed-0019.md` and `Issue-closed-0072-0103.md`).
5. ⚠️ **Do NOT open EP-026 parity work from this Issue.** No Linux change is owed *by this ruling*; Linux
   conformance is scheduled on its own terms.
6. ⚠️ **Correct the SP-100 plan note** (`Sprints/Sprint-active.md` §2) per **§0** action 3 — its stated
   reason for excluding I-0133/I-0134 is void. The conclusion (neither is EP-031 work) stands, but on
   these grounds: **I-0133 is verified and archived; I-0134 is closed as a non-issue.**

**Standing principle established — record beyond this audit:**
> ⚠️ **Parity is DIRECTIONAL. Apple is the source of truth; Linux conforms to Apple.** A difference
> between the two platforms is **not** a defect and must not be filed as one. Linux work is scheduled as
> Linux work (EP-026 and successors), not raised as a cross-platform Issue.

**This principle belongs in `Issue-GUIDELINES.md`** so the same Issue is not filed again — see **R-G**
rulings.

**Status:** 🔵 Ruled — not yet applied
**Date:** 2026-08-19

---

### R-02 — I-0018 shown partly-fixed

**Rules on:** F-02 · **Severity:** Critical

**Ruling:** **Option (a).** I-0018 is ✅ **Verified**. Archive it to
`Issues/Verified/Issue-verified-0011-0020.md` — **with its peers, in its own decade.**

**Plus a second ruling of general effect:**
> ⚠️ **I-0018 should never have been rescoped. It should have been given a new Issue number.**

**Rationale:** The decade-batch rule is mechanical and must stay mechanical — the moment it bends for a
good reason, batch filenames stop being predictable and every future audit has to check contents against
filename (which **F-13** shows is already a problem). Grouping belongs in cross-references, not in filing.

On the rescoping: I-0018 was filed 2026-06-08 for *"no scene selected in the Navigator on load"*; that was
fixed and confirmed. On 2026-08-17 the **same ID was retargeted** at a different behaviour — *"the
manuscript does not scroll to the restored scene."* ⚠️ **This destroys the record of the original defect**:
the fix that closed the first complaint is now filed under an Issue whose title describes something else,
and the ID's verification history spans two unrelated behaviours.

**Action for remediation:**
1. Write I-0018's full entry into `Issues/Verified/Issue-verified-0011-0020.md`. ⚠️ **Both behaviours must
   be recorded** — the original complaint *and* the rescoped one — with the rescoping called out as an
   error of process, so the ID's history is legible.
2. Remove from `Issue-backlog.md`. ⚠️ **The backlog becomes EMPTY** — write that state explicitly
   (*"Currently: 0"*); do not leave a stale *"Currently: 1"* header.
3. Update `Issue-Documentation.md`: backlog count → 0; verified count and batch-2 row (11 → 12 entries).
   ⚠️ Batch 2 is already irregular (holds I-0021–I-0024 per F-13); adding I-0018 is nonetheless correct
   under the decade rule.
4. **Cross-reference it from [[I-0133]]** — the live, verified Issue in the same restore-behaviour cluster.
   ⚠️ **Clarification applied by Claude:** the user said *"reference it in the text of I-0134."* I-0134 is
   being **closed as a non-issue with an erroneous premise (R-01)**, so anchoring a live cross-reference
   there would point at a discarded record. **I-0133 is read as the intended target** — same cluster,
   verified, and already cites I-0131/I-0132. ⚠️ **If I-0134 was actually meant, say so and this reverses.**

**Standing principle established — record beyond this audit:**
> ⚠️ **NEVER rescope an Issue. If the behaviour under investigation changes, VERIFY AND CLOSE the original
> and open a NEW Issue ID for the new behaviour.** An Issue ID is a permanent handle on one defect;
> retargeting it destroys the record of the defect it originally named.

**This principle belongs in `Issue-GUIDELINES.md`** — see R-G rulings.

**Status:** 🔵 Ruled — not yet applied
**Date:** 2026-08-19

---

### R-03 — I-0118 has no archive entry

**Rules on:** F-03 · **Severity:** Critical

**Ruling:** **Option (a).** **Reconstruct** I-0118's full entry into
`Issues/Verified/Issue-verified-0111-0120.md` (batch 12) from the SP-105 sprint archive and the existing
Q1–Q4 ruling text, then close the Issue.

**Rationale (user):** *"We are not 'back-filling' we are 'reconstructing' a deleted primary reference. The
distinction is that we are doing this under the observation of 'Quality Assurance'."*

⚠️ **This resolves a precedent conflict the finding raised.** F-03 noted that reconstructing from
secondary sources cuts against the 2026-08-15 ruling on the 180 unfiled Tasks (**F-26**), which said
*"indexed as-is and flagged, rather than back-filled with archive files reconstructed after the fact from
secondary sources."* **The two acts are not the same act:**

| | Back-filling (F-26, refused) | Reconstruction (R-03, ruled) |
| - | ---------------------------- | ---------------------------- |
| **What happened to the record** | Never written | **Written, then deleted** |
| **Scale** | 180 Tasks | 1 Issue |
| **Sources** | Secondary only | Primary rulings text **survives** in `Issue-active.md:24-85` + SP-105 archive |
| **Oversight** | None — a bulk sweep | ⚠️ **Performed under Quality Assurance observation** |
| **Stakes** | Sprints closed and settled | ⚠️ **An Epic close (SP-100) leans on it as evidence** |

**The Q1–Q4 ruling text is a PRIMARY source, not a secondary one** — it is the user's own words, written
at the time of the ruling. Only the Issue's framing around it was lost.

**Action for remediation:**
1. Reconstruct `## I-0118` into batch 12 (which will then hold **9** entries). It must record: the defect
   (world objects absent from the search index), the fix (SP-105), and the verification.
2. ⚠️ **Move the Q1–Q4 ruling text into that entry** from `Issue-active.md:24-85` — this is **primary
   source material and must not be lost or paraphrased.** ✅ **This also discharges F-05.**
3. ⚠️ **Mark the entry as RECONSTRUCTED**, dated 2026-08-19, naming its sources and stating it was done
   under QA observation. Precedent for the marking: `Closed/Sprint-SP-056.md` is likewise flagged
   reconstructed.
4. Update `Issue-Documentation.md` — batch 12 count 8 → 9, verified total.
5. ✅ **I-0118 may then serve as evidence for SP-100's AC pass**, which `Epic-active.md:36` already
   assumes it can.

**Standing principle established — record beyond this audit:**
> ⚠️ **Reconstruction ≠ back-filling.** Restoring a **deleted** record from surviving primary sources,
> under QA observation and **marked as reconstructed**, is legitimate. Manufacturing records that were
> **never written**, in bulk, from secondary sources, is not. The distinction is *what happened to the
> record*, not *how old it is*.

**Status:** 🔵 Ruled — not yet applied
**Date:** 2026-08-19

---

### R-04 — I-0059 and I-0099 exist nowhere in `docs/`

**Rules on:** F-04 · **Severity:** Critical → ⬇️ **downgraded to Minor** (see below)

**Ruling:** **Option (c) — record as never assigned —** *"unless the git history mentions them."*
**Git history was searched. The condition resolved: no record was lost.**

**What the git search established** (`git log --all -S"<id>" -- docs/`):

| ID | Verdict | Evidence |
| -- | ------- | -------- |
| **I-0059** | ⚪ **NEVER ASSIGNED** | Appears only as *"**Next available:** I-0059"* (`c606bca`, SP-054) and in *"pending I-0059/I-0060"* — the note explaining why batch 6 could not yet be archived. **Never attached to a defect.** |
| **I-0099** | ⚪ **NEVER ASSIGNED** | **Zero commits** touch it in any form. |
| **I-0016** | ⚪ **SUPERSEDED → I-0018** | `8e64bfe` (SP-033): *"I-0016 \| Navigator selection on load \| ⚪ Superseded by I-0018"*, and *"**Superseded:** 1 ⚪ (I-0016 → I-0018)"*. |

⚠️ **Nothing was lost. No reconstruction is owed** — R-03's P3 does not apply here, because these records
were never written rather than deleted.

**⚠️ A second finding is DISPROVED by the same search — F-13's end-of-decade pattern.** F-13 observed that
I-0050, I-0060, I-0100 and I-0120 are each absent from the batch file named for them, and flagged the
end-of-decade regularity as *"a suspicious pattern suggesting an off-by-one at batch-rollover."* **It is
not.** Every one of those four IDs appears in git **only inside filenames and range labels**
(`# Verified Issues — I-0091 to I-0100`, `| 10 | I-0091 – I-0100 | … |`) and **never as an assigned
Issue.** The regularity is an artifact of naming batch files by decade **boundary** while IDs are consumed
one at a time — a decade file is created before its last ID exists, and often that ID is never reached.
**There is no filing bug.** ✅ **This pre-answers Q5.**

**Action for remediation:**
1. Add a **"Never assigned / unaccounted IDs"** line to `Issue-Documentation.md`'s Statistics, recording
   **I-0059** and **I-0099** as never assigned. ⚠️ **This is the durable fix** — every other layer records
   its gaps (*"T-0278 never assigned"*, *"SP-068 skipped"*), and the Issue layer's silence is why an
   unassigned ID and a lost record look identical.
2. Record **I-0016** as ⚪ **Superseded → I-0018** in the same place. Its supersession *was* documented in
   2026-06-08 and was **lost when that index was later rebuilt** — the ⚪ Superseded statistic no longer
   appears anywhere.
3. ⚠️ **Add a note to `Issue-Documentation.md`'s batch table** stating that a decade file is named for its
   ID **range**, not its contents, and that the last ID of a range is frequently never assigned — so the
   next audit does not re-open F-13's pattern.
4. **No git restoration. No reconstruction. No new Issue IDs.**

⚠️ **Note the connection to P2 (R-02):** I-0016 was *superseded by* I-0018 in June, and I-0018 was later
*rescoped* in August. **The same behaviour — "Navigator selection on load" — has now consumed two IDs and
been retargeted once.** R-02's no-rescoping rule would have prevented the second half of that.

**Status:** 🔵 Ruled — not yet applied
**Date:** 2026-08-19

---

### R-05 — The I-0118 design-ruling block is parked in `Issue-active.md`

**Rules on:** F-05 · **Severity:** Critical

**Ruling:** **Option (a).** **Delete the 62-line block** (`Issue-active.md:24-85`) once **R-03**'s
reconstruction has carried the Q1–Q4 ruling text into I-0118's archive entry. **Nothing is copied
elsewhere.**

**Rationale:** `Issue-active.md` holds *Issues awaiting user verification*; a design essay in it is a
category error, and this is the largest block in the file. Its content is preserved by R-03, and its
consequences are already carried where they belong:

| Content | Already lives at | Action |
| ------- | ---------------- | ------ |
| Q1–Q4 rulings | → I-0118's reconstructed archive entry (**R-03** action 2) | Moved |
| Q1's consequence (world lifecycle fork) | `Epic-backlog.md` — **EP-033** | ✅ Already covered |
| *"Implementation shape (not yet built)"* | Shipped as **SP-105** | Now history, not a plan |

⚠️ **EP-033 gets a cross-reference, NOT a copy of the ruling text.** Duplicating it into the Epic would
recreate the restatement-drift problem this entire audit exists to document (see Systemic §S2). A link to
the archived Issue is the correct shape.

**Action for remediation:**
1. ⚠️ **Ordering is mandatory: R-03 first, R-05 second.** Deleting before the text is safely moved
   destroys primary-source material — the exact failure that created **F-03**.
2. Delete `Issue-active.md:24-85` in its entirety, including the *"Retained here as an active design
   ruling"* preamble.
3. Ensure EP-033's `Epic-backlog.md` entry cross-references the archived I-0118 (batch 12).
4. ✅ **After R-01, R-02 and R-05 are applied, `Issue-active.md` is EMPTY** — no active Issues, no parked
   blocks. Write that state explicitly rather than leaving stale headers or counts.

**Status:** 🔵 Ruled — not yet applied
**Date:** 2026-08-19

---

### R-06 — T-0185–T-0188: Verified in the index, Descoped in the backlog

**Rules on:** F-06 · **Severity:** Critical

**Ruling:** **Option (b).** All four Tasks are ⚪ **DESCOPED**. Correct `Task-Documentation.md`, **and move
them to `Tasks/Closed/Task-closed-0185-0188.md`.**

**The record of truth settled the substance — no judgement call was required.**
`Sprints/Closed/Sprint-SP-046.md` states it seven separate ways:

- l.21-24 — all four rows read `⚪ Descoped (I-0057 — CSImportExtension non-functional on macOS)`
- l.12-13 — *"The Layer 2 on-disk importer was **descoped** mid-sprint (I-0057)"*
- l.39-42 — all four acceptance criteria struck through and marked **Descoped**
- l.55-57 — ⚠️ *"**Descoped (not returned to backlog — closed by decision, not incomplete)**: T-0185 /
  T-0186 / T-0187 / T-0188… **Re-openable only if the MDImporter** route is revisited"*

**`Task-backlog.md:39` was correct. `Task-Documentation.md:438` was wrong.** The extension target was
built and codesigned, then **removed** after Apple DTS confirmed the API does not run on macOS (I-0057,
✅ Verified 2026-07-01).

**Rationale for (b):** *"Closed by decision, not incomplete"* is exactly what `Tasks/Closed/` is for; the
precedent files exist (`Task-closed-0125.md`, `Task-closed-0127.md`). It also removes four permanently
dead rows from a file whose stated purpose is **"documented, unstarted"** work, and makes them re-findable
if the MDImporter route is ever revisited — the reopening condition the sprint names explicitly.

**Action for remediation:**
1. Create `Tasks/Closed/Task-closed-0185-0188.md` — one file for the set, since they share a single cause
   and a single closure decision. Record: what was built, that it was **removed**, the I-0057 diagnosis
   (Apple DTS), and ⚠️ **the reopening condition (MDImporter route)**.
2. Correct `Task-Documentation.md:438` and its sibling rows for T-0186/T-0187/T-0188: ✅ Verified →
   ⚪ **Descoped**, and remove the *"⚠️ unfiled"* pointer (nothing is owed — they are closed, not unfiled).
3. Remove the four rows from `Task-Documentation.md`'s **"Verified but unfiled"** section — this drops it
   **180 → 176** and is a correction to **F-26**'s figure.
4. Replace `Task-backlog.md:39`'s pointer row with a link to the new `Closed/` file.
5. ⚠️ **Statistics: Verified 376 → 372; Closed 2 → 6.** See the constraint below.

⚠️ **This ruling forces F-18's hand.** `Task-Documentation.md`'s statistics currently sum to **exactly
418** (376+24+0+9+3+2+2+1+1), which F-18 flags as evidence the block was **balanced arithmetically rather
than derived from the files.** Moving four Tasks from Verified to Closed keeps the total at 418 only if
both lines are corrected together. **Do not adjust one line to make the sum work** — that is the habit
that produced F-18. The whole block must be re-derived by enumeration when F-18 is applied.

**Status:** 🔵 Ruled — not yet applied
**Date:** 2026-08-19

---

*Session opened 2026-08-19.*

---

### R-07 — SP-039 has no archive

**Rules on:** F-07 · **Severity:** Critical

**Ruling:** **Option (b).** **Move** `docs/Sprints/Sprint-SP-039.md` → `docs/Sprints/Closed/Sprint-SP-039.md`,
correct its status **🔵 Planning → ✅ Closed (2026-06-11)**, and **mark it RECONSTRUCTED.**

**Rationale:** (b) preserves what the draft actually says and relabels it honestly as *a plan that became
the record*. (a) would have synthesised a retrospective-shaped archive resembling the other 101 — ⚠️ **but
no retrospective was ever written for SP-039**, and manufacturing one would misrepresent a reconstruction
as a contemporaneous record. Cheaper and more truthful.

**This is P3 (R-03) applied a second time**, and the precedent already exists in-repo:
`Closed/Sprint-SP-056.md` is likewise flagged reconstructed.

**Why it matters:** SP-039 delivered the **entire timeline C ABI**. `Epic-Documentation.md:160` rests on
it — *"The full timeline C ABI already exists (EP-016/SP-039), so this is a pure `[Linux]` UI Epic"* — which
is the premise of EP-025's whole scope. **The only sprint-level record of that work currently says the
sprint was never started.**

**Evidence that SP-039 is the sole genuine gap** (enumerated `Closed/`: 101 archives):

| Sprint | Archive | Verdict |
| ------ | ------- | ------- |
| SP-034 | absent | ✅ Legitimate — **cancelled** |
| **SP-039** | **absent** | ⚠️ **GENUINE GAP — closed 2026-06-11 per `Sprint-Documentation.md:305`** |
| SP-057 | absent | ✅ Legitimate — **superseded → SP-094** |
| SP-068 | absent | ✅ Legitimate — **ID skipped** |
| SP-100 | absent | ✅ Legitimate — **not yet run** |

**Action for remediation:**
1. `git mv docs/Sprints/Sprint-SP-039.md docs/Sprints/Closed/Sprint-SP-039.md`.
2. Correct the status line **🔵 Planning → ✅ Closed**; add the close date **2026-06-11** from
   `Sprint-Documentation.md:305`.
3. ⚠️ **Add a RECONSTRUCTED banner** dated 2026-08-19: state it was recovered from the surviving planning
   draft under QA observation, that **no retrospective was written**, and name `Epic-EP-016.md` as the
   corroborating record.
4. Reconcile the task table against the Task layer — mark each T-ID's real status rather than leaving
   planning-time placeholders.
5. ⚠️ **This is one of the 13 loose files in F-20** and is its named exception. **Apply R-07 BEFORE
   R-20** — R-20 disposes of the other 12, and SP-039 must be moved to safety first, not swept with them.

**Status:** 🔵 Ruled — not yet applied
**Date:** 2026-08-19

---

### R-08 — EP-034 is missing from the Epic index

**Rules on:** F-08 · **Severity:** Critical

**Ruling:** **Option (a) — scope limited to F-08 only.** Add EP-034 to `Epic-Documentation.md` and correct
the figures that omission falsifies. **Do not fold F-23 into this pass.**

**Rationale (user):** *"I haven't read F-23 and so cannot rule on it."* ⚠️ **Claude proposed bundling F-23
into this ruling — that was an error of process.** A ruling must not be requested on a finding the user has
not been presented with; presenting one finding at a time is the whole point of the session format
(`Audit-Guidelines.md` §Phase 2). **F-23 is ruled in its own turn.**

**Why it matters:** EP-034 `[Cross]` Object Detail & Media is the Epic that owes **source creation** — the
reason T-0365 closed as a PARTIAL delivery, and a carried item in the SP-100 plan. It exists in
`Epic-backlog.md:111` (opened 2026-08-18) and appears in `Epic-Documentation.md` **exactly once: as the
"next available" number.**

⚠️ **The numbering collision is the live hazard, not the missing row.** *"Next available: EP-034"* means
**the next Epic opened is assigned an ID already in use.** The Sprint layer has already been bitten by
exactly this — SP-101 was taken, forcing the new sprint to be numbered SP-102 out of sequence.

**Action for remediation:**
1. Add the **EP-034** row to the All-Epics table: `[Cross]` Object Detail & Media, 🔵 Proposed (backlog).
2. Add EP-034 to the **Backlog Epics** section (currently lists EP-026, EP-032, EP-033).
3. Correct the derived figures — ⚠️ **all four, or the file contradicts itself again:**
   - *"Currently: 33 Epics"* → **34**
   - *"Next available: **EP-034**"* → **EP-035**
   - *"Total Epics: 33 (EP-001–EP-033)"* → **34 (EP-001–EP-034)**
   - *"Proposed (backlog): 2"* → **3**
4. ⚠️ **Scope stops here.** F-23's staleness in the same file (*"9 of 11 sprints closed"*, SP-102 shown
   Active, *"AC9 and AC10 are also outstanding"*) is **NOT corrected under this ruling** and awaits R-23.
   **Remediation should apply R-08 and R-23 in the same file-open** once both are ruled.

**Status:** 🔵 Ruled — not yet applied
**Date:** 2026-08-19

---

### R-09 — Batch 14's table lists four Issues; the file holds two

**Rules on:** F-09 · **Severity:** Critical · *(the last Critical finding)*

**Ruling:** **Option (b).** Apply the revert-then-archive sequence below, **and establish the standing
principle** that a batch file's table and its entries are written in a single edit, never separately.

**Origin — recorded plainly:** ⚠️ **Claude caused this defect during this session.** Two table rows
(I-0133, I-0134) were added to `Issues/Verified/Issue-verified-0131-0140.md` and the user stopped the work
before the corresponding entries were written. This is **Group B in §0**, already ruled **revert**.

**Required sequence — the revert and the archiving must not cross:**

| Step | Ruling | State of `Issue-verified-0131-0140.md` |
| ---- | ------ | -------------------------------------- |
| 1 | **§0** — `git checkout` the file | 2 entries / **2 rows** ✅ consistent |
| 2 | **R-01** — write I-0133's entry **and** its row **together** | 3 entries / **3 rows** ✅ consistent |
| — | **R-01** — I-0134 → `Issues/Closed/Issue-closed-0134.md` | ⚠️ **NOT in batch 14 at all** |

⚠️ **The file must never be left in an intermediate state where the counts disagree** — that is the whole
defect, and it is reachable by doing step 2 in two edits instead of one.

**Why this is more than bookkeeping:** the defect shape — **a table advertising an entry that does not
exist** — is *identical* to **F-03**, where I-0118 was asserted Verified in three separate files with no
archive entry anywhere. **Two instances in one audit, from unrelated causes**, one of them months old and
load-bearing for an Epic close.

**Standing principle established — record beyond this audit:**
> ⚠️ **A batch file's summary table and its full entries are written in the SAME edit.** A row without its
> entry is a claim that a record exists when it does not — the same defect as a lost archive, reached by a
> different route.
>
> **Mechanical audit check, to be run on every batch file:** table-row count **must equal**
> `grep -c '^## I-0'`. This check found **both** F-03 and F-09; run it as a matter of course rather than
> on suspicion.

**Action for remediation:**
1. Execute the sequence above in order, treating step 2 as **one atomic edit**.
2. Add the table-vs-entry count check to `Audit-Guidelines.md` §"Checks that reliably find drift" — it is
   currently implied by *"Filename vs. contents"* but **not stated**, and the two findings it caught were
   found by running it ad hoc.
3. ⚠️ **Verify the same invariant across all 14 batch files**, not just batch 14 — F-03 proves the defect
   predates this session. *(Counts as of the audit: batches hold 10, 11, 7, 10, 9, 8, 10, 8, 9, 8, 9, 8,
   10, 2 entries; the index's per-batch figures are separately wrong — see F-11/F-12.)*

**Status:** 🔵 Ruled — not yet applied
**Date:** 2026-08-19

---

### R-10 — Issue index says Active is I-0131 + I-0132

**Rules on:** F-10 · **Severity:** Moderate · *(first Moderate finding)*

**Ruling:** **Option (a).** Rewrite `Issue-Documentation.md`'s **Active Issues** and **Backlog Issues**
sections to the post-remediation state. ⚠️ **Scope stops there — the activity-log question is F-25's and
is NOT ruled here.**

**Rationale:** The index is two days and two Issues behind: it names I-0131/I-0132 as awaiting
verification (both archived 2026-08-18) and **never learned about I-0133/I-0134 at all** — neither ID
appears anywhere in the file. Combined with F-01, ⚠️ **no file in the Issues layer currently states the
true active set.**

**This is not a patch of two stale rows.** The rulings already taken determine the correct end state
completely, so the sections are rewritten to match it rather than nudged:

| File | State after R-01, R-02, R-03, R-05 |
| ---- | ---------------------------------- |
| `Issue-active.md` | ⚠️ **EMPTY** — 0 active Issues, parked I-0118 block deleted |
| `Issue-backlog.md` | ⚠️ **EMPTY** — 0 backlog Issues (I-0018 archived) |
| `Verified/Issue-verified-0131-0140.md` | I-0131, I-0132, **I-0133** — 3 entries / 3 rows (P4) |
| `Verified/Issue-verified-0011-0020.md` | gains **I-0018** → 12 entries |
| `Verified/Issue-verified-0111-0120.md` | gains **I-0118** (reconstructed) → 9 entries |
| `Issues/Closed/` | gains **I-0134** (non-issue, Apple authoritative) |

**Action for remediation:**
1. **Active Issues** section → *"Currently: **0** — no Issues awaiting verification."* Remove the
   I-0131/I-0132 table and the *"Active is now 2"* line (l.113).
2. **Backlog Issues** section → *"Currently: **0**."* Remove the I-0018 row and its rescoping note (its
   substance moves into I-0018's archive entry under R-02).
3. Update the **Verified** batch table and total — ⚠️ **by re-derivation, not arithmetic** (F-11/F-12 show
   the stated counts are already wrong; **P4**'s check applies).
4. Add **I-0134** to the **Closed Issues** table (per R-01 action 4).
5. ⚠️ **Do NOT touch** the *"18 Issues verified in one day"* narrative (l.24-27) or the 36-line activity
   log (l.108-143) under this ruling.

⚠️ **Process note — a conflict Claude flagged rather than resolved:** the natural instinct was to bundle
the narrative/activity-log removal into this ruling, since `Issue-GUIDELINES.md:167` explicitly forbids
activity logs in this file (*"Do NOT add activity logs, summaries, or per-Issue detail tables… The index
must stay compact"*). **That is F-25's decision, and F-25 has not been presented.** Bundling it would
repeat the R-08/F-23 error. **Remediation should apply R-10 and R-25 in the same file-open** once R-25 is
ruled.

**Status:** 🔵 Ruled — not yet applied
**Date:** 2026-08-19

---

### R-11 — Verified total is 116; actual is 119, and batch 14 is missing from the table

**Rules on:** F-11 · **Severity:** Moderate

**Ruling:** **Option (b).** Re-derive the entire batch table and total **by counting**, as the **last**
Issues-layer action of remediation — **and write the P4 invariant into the index itself** so the rule
lives in the file rather than in whoever last did the counting.

**Rationale:** ⚠️ **The file advertises its own method** — *"Counts below are the entries **physically in
each file**, and they sum to 116"* — which is exactly the right discipline, and it **was** genuinely
applied on 2026-08-17. It was simply **not repeated** when batch 14 was created the following day.
**A count that documents its own rigour is more dangerous when stale than a bare number, because it
invites trust it no longer earns.** Stating the invariant in the file is the difference between a
discipline someone remembered once and a rule the next person can follow.

**Two distinct errors in the current table:**
1. ⚠️ **Batch 14 (`Issue-verified-0131-0140.md`) has NO ROW** — created 2026-08-18, never indexed.
2. Batch 3 listed as **6**, holds **7** — see **R-12**.

**Counted at audit time** (`grep -c '^## I-0'`, batches 1→14):
`10, 11, 7, 10, 9, 8, 10, 8, 9, 8, 9, 8, 10, 2` = **119**, not 116.

⚠️ **119 is NOT the number to write.** The final total depends on remediation completing first:

| Batch | Now | After R-01/R-02/R-03 | Why |
| ----- | --- | -------------------- | --- |
| 2 (`-0011-0020`) | 11 | **12** | gains **I-0018** (R-02) |
| 12 (`-0111-0120`) | 8 | **9** | gains **I-0118**, reconstructed (R-03) |
| 14 (`-0131-0140`) | 2 | **3** | gains **I-0133** (R-01) |
| **Total** | **119** | ⚠️ **122** | I-0134 → `Closed/`, **not** a verified batch |

**Action for remediation:**
1. ⚠️ **ORDERING: this is the LAST Issues-layer action.** Counting before the archiving of I-0018, I-0118
   and I-0133 is complete just produces a *different* wrong number.
2. Add the missing **batch 14** row.
3. Re-derive **every** per-batch count and the total by running the count — ⚠️ **do not adjust figures
   arithmetically to make them sum**, which is the F-18 habit.
4. **Write the P4 invariant into `Issue-Documentation.md`** at the batch table: *a batch file's summary
   table rows must equal its entry count (`grep -c '^## I-0'`), and a new batch file gets its index row in
   the same edit that creates it.*
5. Also record there (per **R-04** action 3) that **a decade file is named for its ID range, not its
   contents**, and the last ID of a range is frequently never assigned.

**Status:** 🔵 Ruled — not yet applied
**Date:** 2026-08-19

---

### R-12 — Batch 3 is recorded as 6 entries; the file holds 7

**Rules on:** F-12 · **Severity:** Moderate

**Ruling:** **Option (a).** Count batch 3 as **7** — raw `^## I-0` headings, consistent with every other
row — and **add a footnote** recording that one of those headings is a **pointer stub**, not an Issue.

⚠️ **The finding was partly wrong, and the ruling corrects it.** F-12 reported "6" as a stale miscount.
It is not: it is **deliberate**. `Issue-Documentation.md:114-117` explains it — *"batch 3's first heading
is the pointer stub 'I-0021 through I-0024', so it holds 6 real entries + 1 stub."* The counter was
distinguishing real entries from a stub pointing at the four Issues misfiled into batch 2 (**F-13**).
**The number was defensible. The inconsistency is the defect.**

**What is actually wrong:** every other batch row counts headings raw; **batch 3 alone subtracts a stub**,
and the justification lives in a paragraph ~30 lines away. Under **P4** (*table rows must equal
`grep -c '^## I-0'`*), ⚠️ **batch 3 fails the mechanical check by design** — guaranteeing every future
audit re-opens it exactly as this one did.

**Rationale for (a):** **a check with a hand-maintained exception is not a check.** Keeping P4 purely
mechanical is the entire point of R-09; the footnote preserves the real information (one heading is not an
Issue) without making the *count* carry the nuance.

**Consequence for R-11's arithmetic** — ⚠️ **supersedes the figures recorded there:**

| | Pre-remediation | Post-remediation |
| - | --------------- | ---------------- |
| R-11 as written (batch 3 = 6) | 119 | 122 |
| ⚠️ **R-12 corrected (batch 3 = 7)** | **120** | **123** |

**⚠️ Option (c) was considered and REJECTED** — resolving the underlying misfiling by moving I-0021–I-0024
from batch 2 into batch 3 and deleting the stub. It is the "correct" fix and it is **not worth the risk**.
The 2026-08-16 ruling already settled this: *"Left in place rather than re-cut: moving verified archive
entries risks more than the tidiness buys."* **Moving archive entries between files to satisfy a naming
convention is how records get lost** — and this audit exists because records were lost.

**Action for remediation:**
1. Batch 3's count → **7**, with a footnote: *one heading is a pointer stub for I-0021–I-0024, which are
   physically filed in batch 2 (see F-13); it is counted as a heading but is not an Issue entry.*
2. ⚠️ **Correct R-11's totals to 120 / 123** when the batch table is re-derived.
3. Ensure the P4 invariant as written into the index (R-11 action 4) stays **purely mechanical** — the
   stub is explained by footnote, **never by an exception to the rule.**

**Status:** 🔵 Ruled — not yet applied
**Date:** 2026-08-19

---

### R-13 — Four batch files are titled for IDs they do not contain

**Rules on:** F-13 · **Severity:** Moderate → ⬇️ **downgraded to Minor** (substantially disproved)

**Ruling:** **Option (b).** Close F-13 as **substantially disproved**, and add a **one-line header note to
batch files 2 and 3** stating what each actually contains and why.

⚠️ **Most of this finding was WRONG, and R-04 disproved it.** F-13 reported five mismatched batch files
and flagged the end-of-decade cases as *"a suspicious pattern suggesting an off-by-one at batch-rollover."*
**There is no rollover bug.** R-04's git search (`git log --all -S"<id>" -- docs/`) established that
I-0050, I-0060, I-0100 and I-0120 appear **only inside filenames and range labels** — never as assigned
Issues. **A decade file is named for its ID *range* and created before that range fills, so the last ID of
a range is frequently never used.** The regularity is an artifact of naming, not a filing defect.

**What actually remains — one case, already known and already ruled twice:**

| File | Contains | Status |
| ---- | -------- | ------ |
| `Issue-verified-0011-0020.md` | ⚠️ **also holds I-0021–I-0024** (belong to batch 3) | Known; ruled *leave in place* |
| `Issue-verified-0021-0030.md` | a **pointer stub** for those four, + 6 real entries | Known; **R-12** footnotes it |

The 2026-08-16 ruling stands and was **reaffirmed by R-12**: *"Left in place rather than re-cut: moving
verified archive entries risks more than the tidiness buys."*

**Rationale for (b):** ⚠️ **The index note has existed since 2026-08-16 and did not prevent this audit
from re-opening the question as a near-Critical finding** — because the audit was reading the *files*, not
the index. **An explanation is only useful where the reader actually hits it.** Two lines of text, placed
in the two files themselves, end the recurrence.

**Action for remediation:**
1. Add a header note to `Issue-verified-0011-0020.md`: it also holds **I-0021–I-0024**, filed here
   historically; **deliberately not re-cut** (2026-08-16 ruling, reaffirmed R-12).
2. Add a header note to `Issue-verified-0021-0030.md`: its first heading is a **pointer stub** for
   I-0021–I-0024, which live in batch 2; it is counted as a heading but is **not** an Issue entry (R-12).
3. ⚠️ **No archive entries are moved.** No files renamed.
4. Record in `Issue-Documentation.md` (with R-04 action 3) that **range-naming ≠ contents**, so this is not
   re-opened a third time.

**Status:** 🔵 Ruled — not yet applied
**Date:** 2026-08-19

---

### R-14 — Issue guidelines reference a directory tree that does not exist

**Rules on:** F-14 · **Severity:** Moderate

**Ruling:** **Option (a).** Correct the four wrong paths in `Issue-GUIDELINES.md`. ⚠️ **Scope is this file
only** — `Task-Guidelines.md` carries the identical defect but is **F-19's** to rule.

**Corrections:**

| Line | Currently reads | Correct |
| ---- | --------------- | ------- |
| 31 | `/Scrivi/Documentation/Issues/Issue-Documentation.md` | `docs/Issues/Issue-Documentation.md` |
| 171 | *"Main DR-Reports directory (`Documentation/Issues/`)"* | `docs/Issues/` |
| 177 | *"Issue-verified subfolder (`Documentation/Issues/Verified/`)"* | `docs/Issues/Verified/` |
| 180 | *"Issue-closed subfolder (`Documentation/Issues/Closed/`)"* | `docs/Issues/Closed/` |

**Rationale:** There is **no `Documentation/` directory anywhere in the repo**. Every path in the
authoritative process document is wrong. It has caused no direct harm because people navigate by knowing
where things are — ⚠️ **but the guidelines cannot be followed literally by anyone new**, including Claude
on a cold start, and the rot signals the document has not been maintained alongside the tree it governs.

**Action for remediation:**
1. Apply the four path corrections above.
2. ⚠️ **Do not touch the `DR-`/`DR-Reports` naming leftovers in the same file** — that is **F-15**, not yet
   ruled. (Same species of rot, different finding.)
3. **Suggested sequencing, not a ruling:** `Task-Guidelines.md` has the identical path defect from the
   identical cause (both written against a pre-`docs/` layout). **Doing R-14 and R-19 in one sitting** is
   cheaper and avoids one file being corrected while its twin stays wrong.

**Status:** 🔵 Ruled — not yet applied
**Date:** 2026-08-19

---

### R-15 — Issue guidelines: wrong naming scheme and an incomplete status list

**Rules on:** F-15 · **Severity:** Moderate

**Ruling:** **Option (a).** Fix **all five** defects, with **item 4 resolved as (i)** — correct the
checklists to name the files that actually exist. ⚠️ **`Issue-unverified.md` is NOT created.**

| # | Defect | Correction |
| - | ------ | ---------- |
| 1 | `DR-` naming leftovers (l.133-141, l.171) — *"`Verified/DR-verified-0001-0010.md`"*, *"DR-Reports"* | → `Issue-verified-XXXX-YYYY.md`; drop "DR-Reports" |
| 2 | ⚠️ Status list (l.106-109) defines 🟡 for Resolved-Not-Verified; **practice universally uses 🟠** | Adopt **🟠**; document it |
| 3 | Backlog named `Issue-backlog-XXXX-YYYY.md` (batched) (l.175) | → single `Issue-backlog.md` |
| 4 | ⚠️ Checklists (l.253, l.260) move Issues *"from `Issue-unverified.md`"* — **no such file exists** | → *"from `Issue-active.md` **or** `Issue-backlog.md`"* |
| 5 | Malformed template fence (l.85 vs l.102) | Cosmetic fix |

**⚠️ Item 4 is the one that has actually cost something, and F-02 + F-03 are its consequences.** The
checklist describes archiving through a file that **was never built**, so the real path — *verified while
sitting in `Issue-active.md` or `Issue-backlog.md`* — **has no rule covering it at all.** That is exactly
how **I-0018** sat verified-but-unarchived in the backlog (F-02) and how **I-0118** was verified with no
archive entry ever written (F-03). **Two Critical findings trace to one missing clause in a checklist.**

**Rationale for (i) over (ii) — not building `Issue-unverified.md`:** the Task layer's mirror file
**currently holds zero rows while the statistics claim nine** (**F-18**), so the pattern has not earned its
keep even where it exists. A fourth Issue file is a fourth place to go stale, and the Issues layer has
run 134 Issues without one. ⚠️ **Fix the rule to match the working practice, not the practice to match a
broken rule.**

**Rationale for item 2 being fixed now rather than deferred:** ⚠️ **a status convention the guidelines do
not document is precisely how 🟡/🟠 drift begins.** The same collision exists in `Task-Guidelines.md`,
where 🟡 denotes *both* Active and Implemented-Not-Verified (F-19 item 5). **G-2** proposes settling it
across both layers; this ruling settles the Issue half and **must not contradict G-2** when that is ruled.

**Action for remediation:**
1. Apply all five corrections above.
2. ⚠️ **Do not create `Issue-unverified.md`.**
3. ⚠️ **Coordinate item 2 with R-G2** — 🟠 for Resolved/Implemented-Not-Verified must be adopted in
   `Issue-GUIDELINES.md` **and** `Task-Guidelines.md`, or the collision simply moves.
4. Combine with **R-14** (same file) in one edit.

**Status:** 🔵 Ruled — not yet applied
**Date:** 2026-08-19

---

## Tasks layer

### R-16 — `Task-unverified.md` says T-0389 "is now listed"; the table is empty

**Rules on:** F-16 · **Severity:** Moderate · *(first Tasks-layer finding)*

**Ruling:** **Option (a).** Delete the two stale prose blocks. The empty table and
`_No Tasks awaiting verification._` stand on their own — **no explanatory note is added.**

**What is deleted:**
- l.23-27 — *"T-0389 is also listed in `Task-active.md` because **SP-102 is still active** — it is
  implemented but its Sprint has not closed"* ⚠️ *(its sprint-status clause is **F-17**; same two lines)*
- l.36 — *"T-0389 is the one genuinely unverified Task and is now listed."*

**What is true:** T-0389 was ✅ Verified 2026-08-17 and archived to
`Tasks/Verified/Task-verified-0389-0415.md`. **The table is correct; the prose around it is stale.**

**Rationale for (a):** the file's job is to answer one question — *what is awaiting verification?* When the
answer is *none*, the correct content is **"none."** ⚠️ **Explaining why a status file is empty is how
status files silt up** — the same disease as F-25 (36 lines of activity log in an index that forbids them)
and F-28 (230 lines of closed-sprint history in a backlog holding one row). The Sprint archive and
`Task-Documentation.md` are where the history belongs.

⚠️ **This is the MIRROR of the failure seen everywhere else in this audit.** Elsewhere a table went stale
while prose stayed right; **here the table was maintained and the narrative was not.** The lesson is not
*"keep tables current"* — it is **P5** below: two statements of the same fact in one file will eventually
disagree, whichever one is authoritative.

**Standing principle established — record beyond this audit:**
> ⚠️ **A status file states its state ONCE.** If a table already answers the file's question, prose
> restating the same fact is a second source of truth that will drift from the first. **Do not narrate a
> table.**

**Action for remediation:**
1. Delete both prose blocks.
2. ⚠️ **Apply with R-17** — F-17 rules on the *"SP-102 is still active"* clause inside the very same
   sentence at l.23. One edit, not two.
3. Leave the empty table and `_No Tasks awaiting verification._` in place.
4. ⚠️ The T-0365 caveat block (l.18-21) is **not** part of this ruling — it concerns a *Verified* Task's
   partial delivery, and F-16 does not reach it.

**Status:** 🔵 Ruled — not yet applied
**Date:** 2026-08-19

---

### R-17 — `Task-unverified.md` states SP-102 is still active

**Rules on:** F-17 · **Severity:** Moderate

**Ruling:** **Option (b).** Delete the clause with **R-16** (same sentence, one edit) — **and record that
the sprint-close checklist is incomplete**, as evidence for **R-22**.

**What is deleted:** `Task-unverified.md:23` — *"because **SP-102 is still active** — it is implemented
but its Sprint has not closed."* SP-102 closed **2026-08-18** (user-approved).

**⚠️ The finding beyond the clause: a sprint close is the single most drift-producing event in the
system.** SP-102's close on 2026-08-18 left stale claims in **at least four files**, each surfacing as its
own finding in this audit:

| File | Stale claim | Finding |
| ---- | ----------- | ------- |
| `Tasks/Task-unverified.md` | *"SP-102 is still active"* | **F-17** (this) |
| `Sprints/Sprint-Documentation.md` | All-Sprints row + Statistics both showed 🟡 Active | **F-21** |
| `Tasks/Task-Documentation.md` | Statistics: 3 Active / 372 Verified | **F-18** |
| `Epics/Epic-Documentation.md` | *"9 of 11 sprints closed"*, SP-102 Active | **F-23** |

**Why it recurs — the checklist does not name the files that drift.** `Sprint-GUIDELINES.md:154-163`
*"When Closing a Sprint"* lists eight items naming five files:

> `Sprint-active.md` · `Closed/Sprint-SP-XXX.md` · `Sprint-Documentation.md` · `Epic-active.md` ·
> *"Statistics updated"* · *"Last Updated date updated"*

⚠️ **It names NO Task-layer file, NOT `Epic-Documentation.md`, and NOT `Sprint-backlog.md`** — and
*"Statistics updated"* never says **whose** statistics. **Three of the four files that went stale are
files the checklist never mentions.** This is not carelessness at close time; **it is a checklist that
does not cover its own blast radius.**

**Action for remediation:**
1. Delete the clause — ⚠️ **one edit with R-16**, both halves of the same sentence.
2. ⚠️ **Do NOT rewrite the sprint-close checklist here.** That is **R-22**'s (`Sprint-GUIDELINES.md`), not
   yet ruled. **This ruling supplies the evidence; R-22 makes the change.**
3. Carry into R-22: the checklist must enumerate **every file that restates a sprint's status**, and
   *"Statistics updated"* must name which files' statistics.

**Status:** 🔵 Ruled — not yet applied
**Date:** 2026-08-19

---

### R-18 — Task statistics claim 9 unverified; the unverified file holds 0

**Rules on:** F-18 · **Severity:** Moderate

**Ruling:** **Option (a).** **Re-derive the ENTIRE statistics block by enumeration** — all 418 IDs, every
line counted from the files — as the **last** Tasks-layer action of remediation.
⚠️ **Option (b) is explicitly rejected.**

**The tell that condemns the whole block, not just one line:** the statistics sum to **exactly 418**
(376+24+0+9+3+2+2+1+1), which is precisely the total ID count — **while one of its lines contradicts
another file outright** (`Task-Documentation.md:47` says 9 Implemented-Not-Verified;
`Task-unverified.md:16` says *"No Tasks awaiting verification"*). ⚠️ **A block that lands exactly on its
total while internally contradicting itself was balanced arithmetically, not derived by counting.**

**Therefore the defect is not the number 9.** If the 9 is wrong, **at least one other line absorbed the
error** to keep the sum at 418 — so an unknown second line is also wrong, and **the whole block is
unverified.**

**⚠️ Why option (b) — correct the 9, adjust another line to preserve 418 — is actively harmful:** it is
*precisely the habit that produced this finding.* It treats the total as the thing to protect, when the
total is **the least trustworthy figure in the block**. It is the same error shape as **I-0121** (SP-106):
a guard that kept the arithmetic looking right while the underlying value was wrong, green on one
architecture for 17 days.

**Known corrections that must land in the re-derivation:**

| Line | Stated | Known change | Source |
| ---- | ------ | ------------ | ------ |
| ✅ Verified | 376 | → **372** | **R-06** — T-0185–T-0188 are Descoped, not Verified |
| ⚪ Closed | 2 | → **6** | **R-06** — the same four move to `Tasks/Closed/` |
| 🟠 Implemented - Not Verified | 9 | → **0** *(pending count)* | `Task-unverified.md` is empty; **F-16/R-16** confirm T-0389 archived |
| ⚠️ Verified but unfiled | 180 | → **176** | **R-06** — the four leave the unfiled set |
| 🔵 Backlog | 24 | ⚠️ **recount** | §0 keeps T-0418 in `Task-backlog.md`; not independently verified |

⚠️ **These are inputs to the count, NOT the answer.** Every line is derived by enumeration; no figure is
carried over on trust, including the ones above.

**Action for remediation:**
1. ⚠️ **ORDERING: last Tasks-layer action.** Must run **after R-06** (the Descoped move) and after every
   other Task change, or the count is stale before it is written.
2. Enumerate **T-0001 … T-0419** and assign each ID exactly one status, from the files: `Verified/`,
   `Closed/`, `Task-active.md`, `Task-backlog.md`, `Task-unverified.md`, and the *unfiled* section.
3. Write each statistics line from that enumeration. ⚠️ **Do not adjust any line to make the total sum.**
   If the derived lines do not sum to the ID count, **that discrepancy is a finding**, not something to
   paper over.
4. Record **how** the count was derived, so the next audit can reproduce it — and per **R-11**, note that a
   count advertising its own method must be re-run whenever the underlying files change.

**Standing principle established — record beyond this audit:**
> ⚠️ **Statistics are DERIVED, never balanced.** Every line is counted from the files it describes. **A
> total that "comes out right" is not evidence the lines are right** — it is often evidence that one was
> adjusted to make it so.

**Status:** 🔵 Ruled — not yet applied
**Date:** 2026-08-19

---

### R-19 — Task guidelines: wrong paths, "ER" leftovers, no Closed state in the lifecycle

**Rules on:** F-19 · **Severity:** Moderate

**Ruling:** **Option (a).** Fix **all six** defects, with **item 5 adopting 🟠** to match **R-15**.

| # | Defect | Correction |
| - | ------ | ---------- |
| 1 | Paths `Documentation/Tasks/` (l.126, 177, 184, 187) | → `docs/Tasks/` |
| 2 | "ER" leftovers — *"Verified ERs"* (l.163), *"ER moved from…"* (l.254-256), *"all ERs"* (l.178) | → Task |
| 3 | Typos — *"Taskss"* (l.133, 160-162), *"Sprint Asigned"* (l.74), *"aproval"* (l.119) | Fix |
| 4 | ⚠️ Lifecycle diagram (l.9-14) mangled and **omits `Closed/`** | Redraw incl. Closed |
| 5 | ⚠️ **🟡 denotes BOTH Active and Implemented-Not-Verified** (l.11, 26, 29) | → **🟠** for Implemented-Not-Verified |
| 6 | Version footer *"Last Updated: 2026-01-10"* — 7 months stale | Update |

**⚠️ Item 5 must move in lockstep with R-15, or the collision simply relocates.** R-15 adopted 🟠 for
`Issue-GUIDELINES.md`; leaving `Task-Guidelines.md` at 🟡 would leave **the two layers documenting
contradictory conventions** — worse than the single-file defect. **R-G2** confirms the convention across
both layers; it does not originate it.

**⚠️ Item 4 became more urgent DURING this session.** The lifecycle diagram omits `Closed/` even though the
state is defined at l.22, `Tasks/Closed/` exists with 2 files, and the checklist at l.262-269 covers moving
a Task there. **R-06 adds four more (T-0185–T-0188), taking Closed from 2 files to 6** — the state is about
to triple in use while the diagram still does not show it exists.

**Action for remediation:**
1. Apply all six corrections.
2. ⚠️ **Item 5 in the same pass as R-15** — one convention, both files, or neither.
3. ⚠️ **Item 4 must reflect R-06's outcome** — `Tasks/Closed/` is a live, growing state, not a vestige.
4. **Combine with R-14** (the identical path defect in `Issue-GUIDELINES.md`) in one sitting, per R-14's
   sequencing note — both files were written against the same pre-`docs/` layout.
5. Do **not** invent new process while fixing typos: ⚠️ **this ruling corrects how the guidelines DESCRIBE
   existing practice; it does not change the practice.**

**Status:** 🔵 Ruled — not yet applied
**Date:** 2026-08-19

---

## Sprints layer

### R-20 — Twelve loose Sprint files duplicate their archives

**Rules on:** F-20 · **Severity:** Moderate · *(first Sprints-layer finding)*

**Ruling:** **Option (a).** **Delete the 12 duplicate loose files.** Their `Closed/` archives supersede
them. ⚠️ **`Sprint-SP-039.md` is NOT among them** — R-07 rescues it first.

**Files to delete:**
`Sprint-SP-003-planning.md` · `-004-` · `-005-` · `-006-` · `-007-` · `-008-` · `-026-planning.md` ·
`Sprint-SP-027.md` · `-028.md` · `-029.md` · `-031.md` · `-032.md`

⚠️ **This answers the user's original question** — *"figure out what those erroneous `Sprint-SP-*.md` and
planning documents are doing in the Sprints folder."* **They are pre-close planning drafts** that were
never removed once their sprint closed and its archive was written. `Sprint-GUIDELINES.md:72-82` defines
exactly four file types for this folder; **these match none of them.**

**✅ SAFEGUARD EXECUTED — the deletion is evidence-based, not assumed.** F-20 flagged that "drafts are
shorter" is consistent with supersession but **is not proof**, and that a draft might carry a planning
ruling absent from its retrospective. **Three drafts were spot-checked by diff (SP-003, SP-026, SP-029):**

- Lines unique to the drafts are **planning-time scaffolding**: `**Status:** 🔵 Planning`,
  `Start Date: TBD`, `end_date: —`, and task rows in pre-execution states (`🔵 Planned`,
  `🟠 Implemented - Not Verified`) that the archives record in **final** states.
- ⚠️ **SP-029's draft appeared to carry unique design detail** — the `reorderScene` cross-chapter
  index-write ordering (*"scene is never in two indices simultaneously"*). **Checked directly:**
  `grep -c` returns **2 occurrences in `Closed/Sprint-SP-029.md`**. It was flagged by diff only because of
  reformatting. ✅ **Nothing is lost.**

**Conclusion:** the drafts contain **no information the archives lack** — only earlier, now-false status
values. ⚠️ **That is precisely what makes them dangerous rather than merely untidy:** they read as current
documents. `Sprint-SP-039.md` still declared *"🔵 Planning"* for a sprint closed 2026-06-11.

**Action for remediation:**
1. ⚠️ **APPLY R-07 FIRST.** Move `Sprint-SP-039.md` → `Closed/Sprint-SP-039.md`. **A bulk
   `rm docs/Sprints/Sprint-SP-*.md` before that step destroys the only sprint-level record of the timeline
   C ABI work.**
2. Delete the 12 files listed above. `git rm`, so the deletion is recoverable from history.
3. ⚠️ **Do NOT create a `Drafts/` folder here** — where planning drafts should live, and when they are
   removed, is **F-22**'s question and is not yet ruled.
4. Verify afterwards that `docs/Sprints/` contains exactly: `Sprint-GUIDELINES.md`,
   `Sprint-Documentation.md`, `Sprint-active.md`, `Sprint-backlog.md`, `Closed/`.

**Status:** 🔵 Ruled — not yet applied
**Date:** 2026-08-19

---

### R-21 — Sprint index: All-Sprints table and Statistics showed SP-102 Active

**Rules on:** F-21 · **Severity:** Moderate

**Ruling:** **Option (b).** Confirm the correction already applied under **§0 Group A** — **and formally
record that this is a THIRD recurrence**, which trips `Audit-Guidelines.md`'s own escalation rule and is
carried to **R-22** as a structural change, not a third repair.

**⚠️ Disclosure:** Claude edited these lines **during this session**, before the audit began, as part of
the SP-100 planning work. They fall in **§0 Group A**, which the user ruled **KEEP**. **The correction
therefore already exists in the working tree**; this ruling confirms it stands and records what it means.

**What was corrected:**

| Location | Was | Now |
| -------- | --- | --- |
| All-Sprints row, l.348 | `SP-102 … \| — \| 🟡 **Active**` | `… \| 2026-08-18 \| ✅ Closed` |
| Statistics, l.356-358 | *"Active: 1 🟡 — SP-102"* · *"Closed: 100"* | *"Active: 0"* · *"Closed: 101"* |
| l.222 | *"100 closed, 1 🟡 Active (SP-102)"* | *"101 closed, 0 Active"* |

⚠️ **The file's own header (l.14) already said SP-102 was closed.** The document contradicted itself
internally.

**⚠️ THIRD RECURRENCE — the escalation rule applies.** `Sprint-Documentation.md:193-197` documents the
**previous two** occurrences of this exact pattern. `Epic-Documentation.md:119-127` records the same
pattern in the Epic index and **already reached the correct diagnosis**:

> *"Every one of these was a count or status that drifted while the table beneath it stayed correct —
> which is the argument for **deriving these lines from the table rather than restating them**."*

**That conclusion was written down and then not acted on.** Three occurrences; one correct diagnosis on
record; **zero structural change.**

`Audit-Guidelines.md` §"After the audit" states: *"A finding that recurs across audits is a systemic
problem, not a clerical one, and should be raised as a guidelines change rather than fixed a third time."*
⚠️ **F-21 is the first finding in this audit to actually trip that rule.** Fixing it a third time and
moving on would be the fourth instance of writing down the right answer and not acting on it.

**Action for remediation:**
1. ✅ **No file edit is owed** — the correction is already in the tree under §0 Group A. **Verify it
   survives** the §0 Group B revert (it is a different file, so it should).
2. ⚠️ **Carry to R-22 as a required input:** `Sprint-Documentation.md`'s Statistics block and its l.222
   *"Currently:"* line **restate** what the All-Sprints table already says. **The structural fix is to stop
   restating** — either derive them, or delete them and let the table be the single source.
3. Record in the findings-vs-rulings history that **F-21 recurred three times before triggering a
   structural response**, so the next audit can check whether R-22's change actually held.

**Status:** 🔵 Ruled — ⚠️ **correction already applied under §0; structural fix owed to R-22**
**Date:** 2026-08-19

---

### R-22 — Sprint guidelines have no rule for planning drafts

**Rules on:** F-22 · **Severity:** Moderate → ⬆️ **the audit's most consequential guidelines ruling**

**Ruling:** **Option (b).** Fix F-22's two named gaps **and discharge both structural mandates** handed
here by **R-17** and **R-21**. **Four changes to `Sprint-GUIDELINES.md` / `Sprint-Documentation.md`:**

---

**① Planning drafts — where they live and when they die.**
Practice creates a planning document per sprint; the guidelines (l.70-87) define four file types and
**neither permit nor forbid it**, so nothing ever said when to remove one. **13 sat loose for months
reading as current documents** (F-20/R-20). **Rule to add:** a planning draft is **transient**. It is
either folded into `Sprint-active.md` at activation or deleted at close, **in the same step the archive is
written.** ⚠️ **No draft outlives its sprint's close.**

**② A Sprint may be FULLY PLANNED while still 🔵 Planning.**
Undocumented today, and **SP-100 is in exactly that state right now** — a complete plan in
`Sprint-active.md` behind a banner saying it is not active. It works, but a reader will reasonably assume
`Sprint-active.md` means *active*. **Rule to add:** state that a completed plan may occupy
`Sprint-active.md` **pre-activation**, that it MUST carry a not-yet-active banner, and that **activation
is a separate, user-approved step.**

**③ ⚠️ The close checklist must cover its own blast radius.** *(mandate from R-17)*
`Sprint-GUIDELINES.md:154-163` names five files. **Three of the four that went stale at SP-102's close are
not among them** — `Task-unverified.md`, `Task-Documentation.md`, `Epic-Documentation.md` — and
*"Statistics updated"* never says **whose**. **Rule to change:** enumerate **every file that restates a
sprint's status**, and name each statistics block explicitly:

> `Sprint-active.md` · `Closed/Sprint-SP-XXX.md` · `Sprint-Documentation.md` (header **and** All-Sprints
> table) · `Sprint-backlog.md` · `Epic-active.md` · `Epic-Documentation.md` · `Task-active.md` ·
> `Task-unverified.md` · `Task-Documentation.md` statistics

**④ ⚠️ STOP RESTATING — the third-recurrence structural fix.** *(mandate from R-21)*
`Sprint-Documentation.md`'s **Statistics** block and its l.222 *"Currently:"* line restate what the
All-Sprints table already says. **This has drifted three times, with the correct diagnosis already written
in two files and never acted on.** **Rule to change: the All-Sprints table is the SINGLE SOURCE for
per-sprint status and counts.**

⚠️ **Statistics is REDUCED, not deleted** — deleting a block people find useful has its own cost. **Keep
only what the table cannot express:**

| Keep | Delete (re-totals the table) |
| ---- | ---------------------------- |
| Next available Sprint ID | *"Closed: N"* |
| Cancelled / skipped / superseded IDs + why (SP-034, SP-057, SP-068) | *"Active: N"* |
| Total IDs issued | *"Planning: N"* |

---

**Rationale:** ⚠️ **Option (a) would have left the audit's most-repeated defect unfixed.** **R-21 exists
precisely because the narrow fix was chosen three times running.** The close checklist is where sprint
drift *originates*; the restated Statistics block is where it *lands*. Repairing folder hygiene while
leaving both would be the pattern repeating a fourth time.

**Action for remediation:**
1. Apply ①–④.
2. ⚠️ **④ must be applied AFTER R-21's correction is verified in the tree**, so the table is right before
   it becomes the single source.
3. ⚠️ **Do not extend ④'s "stop restating" rule to other layers in this pass.** `Epic-Documentation.md`
   has the same disease (**F-23**, R-08's file) and `Task-Documentation.md` likely does too — **but each
   is its own finding.** Record the pattern; do not sweep.
4. Cross-reference **P6** (statistics are derived, never balanced) — ④ is P6 applied structurally.

**Standing principle established — record beyond this audit:**
> ⚠️ **A summary that restates a table will drift from it. Derive it or delete it.** Keep in a statistics
> block only what the table **cannot** express. This is the single most repeated defect in the Scrivi
> tracking docs, diagnosed correctly in writing on at least two prior occasions before being acted on.

**Status:** 🔵 Ruled — not yet applied
**Date:** 2026-08-19

---

## Epics layer

### R-23 — Epic index is two days stale on EP-031

**Rules on:** F-23 · **Severity:** Moderate · *(first Epics-layer finding)*

**Ruling:** **Option (b).** Correct the three stale claims **and apply P7 to this file** — the All-Epics
table becomes the single source; Statistics is reduced to what the table cannot express.

⚠️ **Process note:** F-23 is the finding Claude wrongly tried to bundle into **R-08** before the user had
seen it (*"I haven't read F-23 and so cannot rule on it"*). **It is ruled here, in its own turn, as it
should have been.**

**① The three factual corrections:**

| Line | Claims | True |
| ---- | ------ | ---- |
| 15-19 | *"**9 closed** … Two remain: **SP-102** 🟡 Active … then SP-100"* | **10 closed**; **only SP-100 remains** |
| 102-103 | *"9 of 11 sprints closed; SP-102 🟡 Active"* | **10 of 11**; **0 Active** |
| 24 | ⚠️ *"**AC9 and AC10** are also outstanding"* | ⚠️ **AC9 was MET 2026-08-18** — only **AC1 + AC10** are outstanding |

⚠️ **The AC9 clause is the sharp one, and it is WRONG rather than merely stale.** The others are ordinary
drift; this is a false claim about **Epic completion state**. **SP-100's entire job is to verify AC1–AC10
and prepare the close** — an index telling that verification pass that AC9 is unmet, when it was met and
the sprint that met it (SP-102) is closed and archived, invites **re-doing settled work or doubting
correct evidence.** `Epic-active.md:6` has it right; this file contradicts it.

**② P7 applied to `Epic-Documentation.md`** *(the sweep R-22 declined to make unilaterally)*

**R-22 scoped its structural fix to the Sprint layer and said "record the pattern; do not sweep" —
because each file is its own finding. This is that finding, and this is that ruling.**

⚠️ **This file is where the correct diagnosis was FIRST written down** — `Epic-Documentation.md:119-127`:
> *"Every one of these was a count or status that drifted while the table beneath it stayed correct —
> which is the argument for deriving these lines from the table rather than restating them."*

**Leaving this file restating its own table while the Sprint layer is fixed would be incoherent**, and it
is the same small change.

| Keep in Statistics | Delete (re-totals the All-Epics table) |
| ------------------ | -------------------------------------- |
| Next available Epic ID *(⚠️ **EP-035** per R-08)* | *"Closed: N"* |
| Total Epics issued | *"Active: N"* · *"Draft: N"* · *"Proposed (backlog): N"* |
| Any deferred/abandoned IDs + why | *"Complete (pending close): N"* |

**Action for remediation:**
1. ⚠️ **Apply in ONE file-open with R-08** (which adds the EP-034 row and sets next-available to EP-035) —
   both rulings edit `Epic-Documentation.md`, and R-08's row must exist before the table is made
   authoritative.
2. Apply ① then ②, in that order: **the table must be correct before it becomes the single source.**
3. Per-EP-031 status in this file should **point to `Epic-active.md`**, not restate it — that file is
   authoritative for the active Epic's AC state.
4. ⚠️ **Still do not sweep `Task-Documentation.md`** — it likely has the same disease, but it is **F-26/
   R-18's** file and P7 was not ruled for it. **Flag for the next audit.**

**Status:** 🔵 Ruled — not yet applied
**Date:** 2026-08-19

---

### R-24 — `Epic-active.md`'s AC9 note still describes SP-102 as in progress

**Rules on:** F-24 · **Severity:** Minor · *(first Minor finding)*

**Ruling:** **Option (a).** Fix the AC9 note only — remove the struck-through *"In progress — SP-102
active"* line and the forward-looking prose beneath it. ⚠️ **The wider superseded-commentary pass is NOT
done under this ruling.**

**What is corrected:** `Epic-active.md:199-204` — *"🟡 ~~In progress — SP-102 active (2026-08-17).~~"*
followed by *"**AC9 ticks when SP-102 lands**."* AC9 **is met** (2026-08-18); the strikethrough was applied
to one line while the surrounding narrative stayed forward-looking.

**Rationale for deferring the wider pass:** ⚠️ **`Epic-active.md` is 582 lines for a single Epic**, each AC
carrying layers of superseded commentary in exactly this style — strike a line, add a note, keep both,
append *"Prior note follows."* **That file is what SP-100 must read to verify ten acceptance criteria**,
and its signal-to-history ratio is itself a risk to the close (Systemic **§S3**). But:

- A superseded-commentary pass is **a substantial edit to the authoritative record of an ACTIVE Epic.**
- Doing it during *audit remediation* — while SP-100 has not started and the ACs are unverified — ⚠️ **risks
  removing context the verification pass actually needs.**
- **The right moment is SP-100's own AC-by-AC pass**, which must read every entry regardless.

**⚠️ Interaction with §0 — the file is getting LONGER, not shorter.** `Epic-active.md` is in **Group A
(kept)**, and the SP-100 planning edits already added the AC1 and AC10 amendments to it. This ruling
removes a few lines; the net direction of travel is still growth.

**Action for remediation:**
1. Remove the struck line and the *"AC9 ticks when SP-102 lands"* prose. **Leave the ✅ MET line and its
   evidence** — that is the AC's record of truth.
2. ⚠️ **Do NOT touch the other nine AC entries.**
3. **Carry to SP-100 as a scoped work item, not a vague intention:** during **T-0391**'s AC-by-AC pass,
   reduce each AC entry to *current state + evidence*, and move accumulated history into the Epic archive
   **at close**. ⚠️ **This is a deliberate task with its own approval, never a side effect of another
   edit.**

**Status:** 🔵 Ruled — not yet applied
**Date:** 2026-08-19

---

### R-25 — The Issue index carries activity logs its own guidelines forbid

**Rules on:** F-25 · **Severity:** Minor

**Ruling:** **Option (a) — ENFORCE the rule.** Strip the activity logs from `Issue-Documentation.md`
(l.108-143) **and** from `Task-Documentation.md`. The existing prohibition stands as written; ⚠️ **the
rule is not amended, bounded, or relaxed.**

**Rationale (user):** *"The logs are superfluous and unnecessary… keeping logs is a form of documentation
of 'objective evidence' as used in SQA and Test verification and, while that is sometimes necessary, this
project does not require either."*

⚠️ **This corrects the framing of the finding and of Claude's recommendation.** F-25 argued the logs had
*"demonstrably paid for themselves during this audit"* and recommended **(c)**, a bounded log. **That is a
traceability argument — i.e. an objective-evidence argument — and it presumes a QA regime this project is
not operating under.** Absent that mandate, the logs are **cost without a requirement**, and the rule that
already forbids them is correct.

**The rule, in both files, is explicit and marked CRITICAL:**
> `Issue-GUIDELINES.md:167` / `Task-Guidelines.md:173` — *"**CRITICAL:** Do NOT add activity logs,
> summaries, or per-Issue detail tables… The index must stay compact."*

⚠️ **Both indexes have violated it consistently for months** — 36 lines in the Issue index; the Task index
at **700+ lines**. F-25 read that as evidence *the rule had lost an argument with practice.* **The user's
ruling is that practice was simply wrong.**

**Action for remediation:**
1. Delete `Issue-Documentation.md:108-143` (the dated activity narrative).
2. Delete the equivalent activity-log sections from `Task-Documentation.md`.
3. ⚠️ **Preserve nothing to a new "history" file.** The rule's point is that this material is not kept;
   sprint archives and Issue/Task entries already carry what the project needs.
4. ⚠️ **Apply R-25 together with R-10** — both edit `Issue-Documentation.md`, and **R-10 explicitly deferred
   this decision** to avoid ruling on an unpresented finding.
5. ⚠️ **Do NOT delete the `Last Updated:` line itself** — a single current-date line is not an activity log.
6. ⚠️ **This does not reach the "Verified but unfiled" section** of `Task-Documentation.md` (180 rows, F-26)
   — that is a **register**, not a log, and is separately user-ruled to stay.

**Recorded but explicitly NOT ruled here — raised by the user:**
> ⚠️ **There is no traceability between Acceptance Criteria and requirements, and requirements are not
> being added as functionality is added.** The user raised this as a known, larger concern
> (*"I am thinking about these things, but I don't think they are warranted in this project at this
> time"*). **It is out of scope for this audit and is NOT actioned by any ruling.** Recorded so it is not
> lost, and so a future audit does not mistake it for an oversight.

**Status:** 🔵 Ruled — not yet applied
**Date:** 2026-08-19

---

### R-26 — 180 Verified Tasks have no archive file

**Rules on:** F-26 · **Severity:** Minor

**Ruling:** **Option (a).** **No action** beyond R-06's arithmetic correction. **The 2026-08-15 user
ruling stands** — index as-is and flag; do **not** back-fill.

**Why this was in the findings at all:** it is **already known and already user-ruled**, and the section's
own header (l.17-20) flags it as needing a dedicated pass — *"do not assume it was done here."* It was
included because ⚠️ **an audit that omitted the single largest gap in the tree would mislead by
omission**, not because it was newly discovered.

**Independently confirmed during this audit:** 171 archive files cover **263** IDs; **155 IDs in
T-0001–T-0419 have no archive filename**, concentrated in **T-0258–T-0357** (EP-023/024/025/028/029).
✅ **The stated figure of 180 rows matches the actual row count exactly** — one of the few counts in the
tree that is accurate.

**⚠️ P3 (from R-03) draws the line precisely, and this sits on the wrong side of it:**

| | I-0118 (R-03) — reconstruction **allowed** | These 176 — back-filling **refused** |
| - | ------------------------------------------ | ------------------------------------- |
| Record | **Written, then deleted** | **Never written** |
| Scale | 1 Issue | **176 Tasks, in bulk** |
| Sources | Primary text survived | **Secondary only** (sprint archives) |
| Oversight | Under QA observation | A bulk sweep |

**P3 therefore REINFORCES the 2026-08-15 ruling rather than undermining it.** And **R-25** just confirmed
the project does not operate under an objective-evidence regime that would require per-Task archives.

**The register works as-is:** every unfiled Task names its record of truth, and **none of them appears in
the active or backlog lists**, so they are not a source of contradiction.

**Action for remediation:**
1. ⚠️ **180 → 176** — the only change. T-0185–T-0188 leave the unfiled register under **R-06** (they are
   ⚪ Descoped and move to `Tasks/Closed/`).
2. Recount the section's rows when R-18's enumeration runs; ⚠️ **derive the figure, do not assume 176**
   (**P6**).
3. **Keep the section and its "record of truth" pointers.** ⚠️ **It is a REGISTER, not an activity log** —
   **R-25 does not reach it.**
4. Retain the l.17-20 caveat that a dedicated pass was never done, so the next audit does not re-discover
   this as new.

**Status:** 🔵 Ruled — not yet applied
**Date:** 2026-08-19

---

### R-27 — `Task-0118.md` is the only per-Task detail file outside `Verified/`

**Rules on:** F-27 · **Severity:** Minor

**Ruling:** **Option (a).** **Fold the content into the `Task-backlog.md` row and delete the file.** No new
guidelines rule is added to permit a pattern used exactly once.

**Rationale:** `docs/Tasks/Task-0118.md` matches **no documented pattern** — `Task-Guidelines.md` describes
per-Task files only under `Verified/`, and **no other backlog Task has one.** Consolidating removes a
one-off exception rather than writing a rule to legitimise it. Consistent with **R-25**: the project does
not keep documentation it is not required to keep.

⚠️ **The file is more substantial than its 1,966 bytes suggested, and must NOT be discarded wholesale.**
It is a complete Task entry — Rationale, Current/Desired Behavior, 3 Requirements, Design Approach,
Dependencies, 3 Test Steps, and Notes. **"Fold in" means preserve this content in the backlog row, not
summarise it away.**

⚠️ **It also carries one fact the backlog row does NOT: `**Sprint Assigned:** SP-034`.** SP-034 is the
**cancelled** sprint (one of the four legitimately absent from `Closed/`, per R-07's table). **This must be
corrected, not carried across** — T-0118 is unassigned, not assigned to a sprint that never ran.

**Context worth recording:** T-0118 belongs to **EP-011**, closed **2026-06-08** — the same day this file
was written. The Task is legitimately unstarted and legitimately in the backlog, but ⚠️ **its detail file
has sat unreferenced by any active work for over two months**, which is why a future audit would plausibly
mistake it for an orphan. That risk is what F-27 exists to remove.

**Action for remediation:**
1. Move the full content into `Task-backlog.md`'s **Detail** section under `### T-0118`, alongside the
   other backlog Tasks' detail entries (T-0197, T-0249, T-0390, T-0391, T-0418 already live there).
2. ⚠️ **Correct `Sprint Assigned: SP-034` → "Not Assigned"** — SP-034 was cancelled.
3. ⚠️ **Preserve the design substance verbatim** — requirements, the `charCount / totalCharCount` ratio,
   dependencies (T-0114/T-0115), and test steps. **This is a fold, not a summary.**
4. `git rm docs/Tasks/Task-0118.md`; update the link at `Task-backlog.md:53` to point at the in-file
   section.
5. ⚠️ **Do not add a guidelines rule** permitting backlog detail files. If a future backlog Task genuinely
   outgrows a row, that is the moment to decide — **not now, on a sample of one.**

**Status:** 🔵 Ruled — not yet applied
**Date:** 2026-08-19

---

### R-28 — `Sprint-backlog.md` is 260 lines, mostly closed-sprint history

**Rules on:** F-28 · **Severity:** Minor

**Ruling:** **Option (a).** Strip the ~230 lines of closure notes. `Sprint-backlog.md` holds **planned
Sprints only**. ⚠️ **AND — per the user's question — the RULE IS AMENDED so the file cannot refill.**

**⚠️ The user's question is the load-bearing half of this ruling:** *"do we need to adjust the rule so
that the replacement closure notes are removed as well?"* **Yes.** Without it, remediation cleans the file
today and it accretes right back — **the standing rule as written guarantees it.**

**The rule covers only half the lifecycle:**

| Event | Current rule | Result |
| ----- | ------------ | ------ |
| Sprint activates | ✅ *"Planning row/block removed"* — followed correctly | Row leaves |
| Sprint closes | ⚠️ **A closure note is ADDED in its place** | ⚠️ **Never removed — no rule ends it** |

**So every sprint permanently deposits a note in a file whose job is to list what is *planned*.** One
backlog row (SP-100) now sits under closure records for **SP-052 → SP-106**.

**Amended rule — to `Sprint-GUIDELINES.md`, alongside R-22's changes:**
> ⚠️ **A Sprint leaves `Sprint-backlog.md` at ACTIVATION and never returns.** No closure note is written
> there. Whether a Sprint later closed is **`Closed/Sprint-SP-XXX.md`'s** business and
> `Sprint-Documentation.md`'s All-Sprints table — **not the backlog's.** The backlog contains 🔵 Planning
> Sprints and nothing else.

**Rationale — this is P7 and R-25 applied to a third file, and it is the CLEAREST of the three.** With
F-25 Claude argued the activity logs carried traceability value (an argument the user correctly rejected).
**Here there is no such argument to make: the content is verbatim duplicated in `Closed/`.** Removing it
loses **nothing** and eliminates a second copy that can drift from the first.

**Action for remediation:**
1. ⚠️ **SAFEGUARD FIRST — same discipline as R-20's spot-check.** Some closure notes contain **planning
   rulings** (R2 scope decisions taken at activation, content swaps, sprint splits). **Before deleting a
   note, confirm its rulings appear in that sprint's `Closed/` archive.** ⚠️ **If a ruling exists ONLY in
   the backlog note, it is a deleted-record case — P3 applies: reconstruct it into the archive first.**
2. Delete the closure notes; leave the SP-100 Planning row and the file's header.
3. Amend `Sprint-GUIDELINES.md` with the rule above — ⚠️ **in the same pass as R-22**, which is already
   rewriting that file's close checklist and file-structure rules.
4. ⚠️ **Check whether the same one-way rule is needed for `Task-backlog.md` and `Epic-backlog.md`** — both
   are "what is planned" files with the same exposure. **Flag for the next audit; do not sweep now** (the
   R-22/R-23 precedent: each file is its own finding).

**Status:** 🔵 Ruled — not yet applied
**Date:** 2026-08-19

---

### R-29 — Epic guidelines do not reference the Audit layer

**Rules on:** F-29 · **Severity:** Minor

**Ruling:** **Option (b)** for the cross-reference — add it to **all four** GUIDELINES files.
⚠️ **The "audit before an Epic close" rule is REJECTED** and replaced by a new, lighter instrument: the
**Audit Check**.

**⚠️ Claude's proposed rule was wrong, and the user was right to challenge it.** Two reasons, either
sufficient:

1. ⚠️ **It contradicted a change the user had just made.** The user amended `Audit-Guidelines.md`
   §Cadence to read *"Audit only: **Upon request. Claude never begins an audit automatically.**"* A
   mandatory pre-close audit **reintroduces automatic triggering through the back door** — the exact thing
   that amendment removed. Claude proposed it without reconciling against an edit made minutes earlier.
2. **Cost.** *"An audit is a very large very formal thing. Conducting one every time an Epic closes would
   be needlessly time consuming."* — **Correct.** This audit consumed a full session: 4 layers, ~200 files,
   ID enumeration, git archaeology, 31 findings, 31 rulings. **EP-031 alone has eleven sprints.** Mandating
   that per Epic close makes closing an Epic **more expensive than the Epic.**

**✅ The user's counter-proposal is adopted: the AUDIT CHECK** — *"not a formal audit, a quick check of the
documentation state to see if there are any inconsistencies… The findings of the Audit Check can be ruled
on when the Epic closes."*

**Why it works where the rule did not:** the two findings that would have corrupted SP-100's close are
**mechanical, not judgement calls** —
- **F-23** — `Epic-Documentation.md` claimed **AC9 outstanding**; met two days earlier.
- **F-03** — **I-0118**, cited as evidence for the AC pass, had **no archive entry at all.**

*Does every AC's status match the layer below? Does every cited Issue have an archive entry?* ⚠️ **Those
are greps.** They need a checklist, not a session.

⚠️ **And ruling its findings at the close is what keeps the phases intact** — the Check is a miniature
Phase 1 whose Phase 2 folds into the Epic close, rather than a fourth process needing its own ceremony.

**✅ ALREADY WRITTEN into `Audit-Guidelines.md`** — a new *"The Audit Check"* section: the
Audit-vs-Check comparison table, when to run one, **7 mechanical checks**, and the rules (read-only;
findings ruled at the Epic close; no findings file required; ⚠️ **a Check may RECOMMEND a full Audit,
which still only begins on user request**).

**Action for remediation:**
1. Add a **one-line cross-reference** to the Audit layer in all four GUIDELINES files
   (`Issue-GUIDELINES.md`, `Task-Guidelines.md`, `Sprint-GUIDELINES.md`, `Epic-GUIDELINES.md`) — pointing
   at `docs/Audits/Audit-Guidelines.md`. ⚠️ **A pointer only: it carries no policy**, so this is not the
   kind of cross-file sweep R-22/R-23 declined to make.
2. In **`Epic-GUIDELINES.md` only**, add the Epic-close step: **run an Audit Check** (not an Audit) and
   rule its findings as part of the close.
3. ⚠️ **Do NOT add any rule that triggers a full Audit automatically**, from any event.
4. Combine with **R-22** (Sprint) and **R-14/R-19** (Issue/Task) — all four files are being edited anyway.

**Status:** 🔵 Ruled — ⚠️ **the `Audit-Guidelines.md` half is ALREADY APPLIED**; the four cross-references
are not
**Date:** 2026-08-19

---

### R-30 — Guidelines filenames are inconsistently cased

**Rules on:** F-30 · **Severity:** Minor

**Ruling:** **Option (a) — leave as-is. No renaming.** ✅ **`Audit-Guidelines.md` is confirmed correct**
(the user's *"Audi-Guidelines"* was a typo, as Claude read it).

**Current state, accepted deliberately:**

| File | Form |
| ---- | ---- |
| `Issue-GUIDELINES.md` · `Sprint-GUIDELINES.md` · `Epic-GUIDELINES.md` | CAPS |
| `Task-Guidelines.md` · `Audit-Guidelines.md` | Title case |

**Rationale:** ⚠️ **The rename costs more than the inconsistency.** Every guidelines file is referenced by
name across the tracking tree, and **CLAUDE.md names all four paths.** A rename that misses a reference
trades a cosmetic inconsistency for a **broken link — a functional defect.** That risk is worst right now,
while 29 other rulings are being applied to the same files.

**Nothing is broken today:** each file is reachable from its layer, and the naming carries no meaning that
the inconsistency obscures.

**Action for remediation:**
1. **None.** ⚠️ Do not rename any guidelines file.
2. ✅ **`Audit-Guidelines.md` keeps its name** — it is referenced by `Audit-Findings-20260819.md`, by this
   rulings file, and (per **R-29**) will be cross-referenced from all four layer guidelines.
3. If normalisation is ever wanted, ⚠️ **it must be its own pass, AFTER remediation lands**, with a
   complete reference sweep — never tangled with substantive edits.

**Status:** 🔵 Ruled — **no action required**
**Date:** 2026-08-19

---

### R-31 — CLAUDE.md's Agile Tracking section has no Audits layer

**Rules on:** F-31 · **Severity:** Minor · *(the last finding)*

**Ruling:** **Option (a).** Add an **Audits** subsection to CLAUDE.md's Agile Tracking section.
⚠️ **Do NOT copy the P1–P7 standing principles into CLAUDE.md.**

**What the subsection must carry** — the three things that make Audits unlike the other four layers:

1. ⚠️ **The Prime Rule — an Audit CHANGES NOTHING.** It produces findings; it does not fix them.
2. **The three phases** — **Audit** (findings) → **Rulings** (decisions) → **Remediation** (changes), each
   completing before the next begins.
3. ⚠️ **An Audit begins ONLY on user request.** Claude **never** starts one automatically; it may
   *recommend* one. The **Audit Check** (R-29) is the lightweight, mechanical instrument for pre-Epic-close
   verification — **and it is not an Audit.**

⚠️ **Point 3 is the reason this belongs in CLAUDE.md at all.** `Audit-Guidelines.md` already states it, but
**CLAUDE.md is what loads every session** — it is where a standing *"do not do this automatically"*
instruction actually takes effect. In `Audit-Guidelines.md` alone it is only read once an audit is already
underway.

**Placement:** ⚠️ **The Audits layer does NOT belong in the `Epics → Sprints → Tasks / Issues` chain.** It
is **orthogonal** — it operates *on* all four rather than sitting within them. Write it as a **separate
subsection below the four**, not as a fifth item in the flow diagram.

**Files to name:** `docs/Audits/Audit-Guidelines.md` (process),
`docs/Audits/Audit-Findings-YYYYMMDD.md` (Phase 1 output),
`docs/Audits/Audit-Rulings-YYYYMMDD.md` (Phase 2 output).

**⚠️ Why P1–P7 are excluded — this is the ruling's substantive half.** The seven standing principles belong
**in the layer guidelines where they are enforced**: P2 in `Issue-GUIDELINES.md`, P4/P6/P7 in the index
rules, P1 in the Issue-filing rules. **Copying them into CLAUDE.md would restate them in a second
place — exactly what P7 forbids**, and exactly the disease this audit spent a session documenting.
✅ **This rulings file is their record until remediation distributes them to their proper homes.**

**Action for remediation:**
1. Add the **Audits** subsection to CLAUDE.md's Agile Tracking section, below the four existing layers,
   carrying points 1–3 and the three file paths.
2. ⚠️ **State the "user request only" rule explicitly and prominently** — it is the operative instruction.
3. ⚠️ **Do not restate P1–P7**, and do not duplicate `Audit-Guidelines.md`'s content. **A pointer, plus the
   three rules that must be in force every session.**

**Status:** 🔵 Ruled — not yet applied
**Date:** 2026-08-19

---

## Session complete — 2026-08-19

✅ **All 31 findings ruled. All 8 open questions answered. All 11 G-recommendations dispositioned.**
⚠️ **Nothing in this file has been applied.** Phase 3 (Remediation) is separate and not yet begun.

### Seven standing principles came out of this session

They are recorded in the header table (P1–P7) and are to be **distributed to the layer guidelines where
they are enforced** — ⚠️ **not restated in CLAUDE.md** (R-31).

### Four findings were corrected or disproved BY the rulings process

⚠️ **A findings file is a hypothesis set, not a verdict set.**

| Finding | What the audit claimed | What the ruling established |
| ------- | ---------------------- | --------------------------- |
| **F-12** | Batch 3's count of "6" was stale | **Deliberate** — 7 headings minus a pointer stub. The *inconsistency* was the defect, not the number |
| **F-13** | End-of-decade IDs suggested a rollover bug | ⚠️ **Disproved by git** — those IDs were never assigned; an artifact of range-naming |
| **F-01** | I-0134 was a symmetric parity conflict needing arbitration | ⚠️ **Wrong premise** — parity is directional; Apple is authoritative (**P1**) |
| **F-29** | "Audit before every Epic close" | ⚠️ **Rejected** — contradicted the user's own request-only amendment; replaced by the **Audit Check** |

### Remediation ordering constraints — all of them

1. **R-03 → R-05** — move I-0118's primary-source text **before** deleting the block holding it.
2. **R-07 → R-20** — rescue `Sprint-SP-039.md` **before** deleting the 12 loose drafts.
3. **R-06 → R-18** — the Descoped move lands **before** Task statistics are re-derived.
4. **R-21 → R-22(④)** — the All-Sprints table must be correct **before** it becomes the single source.
5. **R-08 + R-23** — one file-open; **R-23(①) before R-23(②)** for the same reason as 4.
6. **R-10 + R-25** — one file-open on `Issue-Documentation.md`.
7. **R-16 + R-17** — one edit; both halves of the same sentence.
8. **R-14 + R-15** (one file) and **R-19** — do the guidelines files in one sitting.
9. ⚠️ **R-11/R-12 LAST in the Issues layer** — count only after all archiving is done. Target: **123**.
10. ⚠️ **R-18 LAST in the Tasks layer** — enumerate all 418 IDs; **never balance to a total**.

### Safeguards that must run during Phase 3

- **R-20** — spot-check drafts against archives before deleting *(already done for SP-003/026/029: ✅ nothing lost)*.
- **R-28** — verify each backlog closure note's planning rulings exist in the sprint archive before deleting; ⚠️ **if a ruling exists only there, P3 applies — reconstruct it first.**
- **R-27** — preserve `Task-0118.md`'s content verbatim; ⚠️ correct its stale `SP-034` assignment.

### Deliberately deferred — not lost

| Item | Owed to |
| ---- | ------- |
| `Epic-active.md`'s 582-line superseded-commentary pass | **SP-100 / T-0391** (R-24) |
| P7 applied to `Task-Documentation.md` | **Next audit** (R-23 action 4) |
| One-way backlog rule for `Task-backlog.md` / `Epic-backlog.md` | **Next audit** (R-28 action 4) |
| ⚠️ **AC-to-requirements traceability; requirements not added as functionality is added** | **User-raised, explicitly out of scope** (R-25) |
| Guidelines filename normalisation | Its own pass, after remediation (R-30) |

### What the next audit should check first

⚠️ **F-21 recurred THREE times** before triggering a structural response, with the correct diagnosis
already written in two files. **Verify R-22(④) and R-23(②) actually held** — that the Sprint and Epic
indexes no longer restate their own tables. If they have drifted again, the structural fix failed and the
problem is not the documents.

---

*Rulings session opened and completed 2026-08-19. 31 rulings, 7 standing principles, 0 files changed.*
