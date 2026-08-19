# Audit Remediation — 2026-08-19

**Applies:** [`Audit-Rulings-20260819.md`](Audit-Rulings-20260819.md) — §0 + 31 rulings (R-01…R-31)
**Findings:** [`Audit-Findings-20260819.md`](Audit-Findings-20260819.md) — 31 findings
**Process:** [`Audit-Guidelines.md`](Audit-Guidelines.md) §"Phase 3 — Remediation"

**Status:** ✅ **VERIFIED — 2026-08-19 (user-approved)**

⚠️ **This log is written as the work proceeds.** It records **which ruling authorised each change**, what
was actually done, **what was deliberately not done**, and how each change was verified — none of which
the git history captures. It is also the session state: **remediation is resumable from this file.**

---

## Progress

| | |
| - | - |
| **Rulings applied** | ✅ **32 of 32** (§0 + R-01…R-31) + the user's T-0298–T-0301 ruling |
| **Session status** | ✅ **COMPLETE AND VERIFIED** — all four layers + guidelines + CLAUDE.md |
| **Files changed** | 2 reverted · ~30 edited · 4 created · **13 deleted** · 1 moved |

---

## Ordering constraints — from the rulings file

| # | Constraint | Satisfied? |
| - | ---------- | ---------- |
| 1 | **R-03 → R-05** — move I-0118's text before deleting the block holding it | ✅ **Satisfied** |
| 2 | **R-07 → R-20** — rescue `Sprint-SP-039.md` before deleting the 12 drafts | ✅ **Satisfied** |
| 3 | **R-06 → R-18** — Descoped move lands before Task statistics are re-derived | ✅ **Satisfied** |
| 4 | **R-21 → R-22(④)** — All-Sprints table correct before it becomes single source | ✅ **Satisfied** |
| 5 | **R-08 + R-23** — one file-open; R-23(①) before R-23(②) | ✅ **Satisfied** |
| 6 | **R-10 + R-25** — one file-open on `Issue-Documentation.md` | ✅ **Satisfied** |
| 7 | **R-16 + R-17** — one edit; both halves of the same sentence | ✅ **Satisfied** |
| 8 | **R-14 + R-15 + R-19** — guidelines files in one sitting | ✅ **Satisfied** |
| 9 | ⚠️ **R-11/R-12 LAST in Issues layer** | ✅ **Satisfied** — ⚠️ derived **122**, not the projected 123 (see R-11 deviation) |
| 10 | ⚠️ **R-18 LAST in Tasks layer** | ✅ **Satisfied** — 417 IDs enumerated; ⚠️ Verified **379**, not the projected 372 |

---

## Safeguards required during this pass

| Ruling | Safeguard | Status |
| ------ | --------- | ------ |
| **R-20** | Spot-check drafts against archives before deleting | ✅ **Done twice** — SP-003/026/029 at ruling time; **all 9 remaining** token-diffed at apply time. ⚠️ SP-005's 5 API symbols investigated in context: a planning **risk note**, not a decision. Nothing lost |
| **R-28** | Verify each backlog closure note's planning rulings exist in the sprint archive before deleting | ✅ **Done** — SP-095/096/097/098/099/102/103/105/106 rulings + T-0410's OBE removal all corroborated in `Closed/`. **Nothing existed only in the backlog; P3 not invoked** |
| **R-27** | Preserve `Task-0118.md` content verbatim; correct its stale `SP-034` assignment | ✅ **Done** — folded verbatim; SP-034 → Not Assigned |

---

## Applied

### §0 — Uncommitted state

**Ruling:** Keep Group A (SP-100 planning work); revert Group B (two half-finished files).

**Files changed:**
- `docs/Issues/Verified/Issue-verified-0131-0140.md` — reverted
- `docs/Tasks/Task-Documentation.md` — reverted

**What was done:** `git restore --staged --worktree` on both Group B files, returning them to their
committed state. The five Group A files were left untouched.

⚠️ **Deviation from the ruling's literal instruction — and the reason for it:** the ruling specified
`git checkout <paths>`. **That command reported "Updated 0 paths from the index" and changed nothing**,
because the user had **staged** the modifications — deliberately, to ensure the new Audit documents were
captured. `git checkout` restores the working tree *from the index*, so with the bad content already in
the index it was a no-op. **`git restore --staged --worktree` was required** to discard both the staged
and working-tree copies. **The ruling's intent was applied exactly; only the mechanism differed.**

**Verification — both checks passed:**

| Check | Before | After | Verdict |
| ----- | ------ | ----- | ------- |
| Batch 14 **P4 parity** — `grep -c "^\| I-01"` vs `grep -c "^## I-01"` | ⚠️ **4 rows / 2 entries** | ✅ **2 / 2** | **PASS** — the F-09 defect is gone |
| `Task-Documentation.md` vs HEAD — `git diff HEAD --stat` | modified | ✅ **no output** | **PASS** — byte-identical to committed state |

⚠️ **A prediction in this log was WRONG and is corrected here.** This entry originally stated that
`grep -c "T-0418" docs/Tasks/Task-Documentation.md` *"must be 0"*. **It returns 3, and 3 is correct.**
Those three hits are **pre-existing** and were never part of Group B:

- l.38-39 — *"Total Task IDs: 417 — T-0001 … T-0418 … T-0418 unassigned"*
- l.668 — `| T-0418 | (next available Task ID — unassigned) | — | — | ⚪ Unassigned | — |`

**T-0418 already existed in this file as the "next available" placeholder** before SP-100 planning; the
Group B edit *converted* that placeholder into an assigned row. Reverting restores the placeholder, so a
non-zero count is the expected outcome. **`git diff HEAD` is the authoritative check, and it is clean** —
the grep count was a poorly chosen proxy, not a failure of the revert.

⚠️ **Group B's content is still owed** — reverting did **not** archive I-0133. That is **R-01**'s work.
T-0418's *assigned* row is **not** owed while SP-100 remains 🔵 Planning (§0 keeps it in
`Task-backlog.md` only).

**Status:** ✅ **Applied - Verified (2026-08-19)**

---

### R-01 — I-0133 archived Verified; I-0134 closed as a non-issue

**Files changed:**
- `docs/Issues/Verified/Issue-verified-0131-0140.md` — I-0133 entry + table row
- `docs/Issues/Closed/Issue-closed-0134.md` — **new**
- `docs/Issues/Issue-active.md` — both rows removed
- `docs/Issues/Issue-Documentation.md` — Closed Issues table
- `docs/Sprints/Sprint-active.md` — SP-100 plan note corrected

**What was done:**
1. **I-0133 → batch 14 as ✅ Verified.** ⚠️ **Table row and full entry written in ONE edit each (P4).**
   The entry records the defect, why impact was low *by design* (centring supersedes the fraction — the two
   contradict), the user's delete-don't-revive ruling, that removal sites carry by-design comments, and
   ⚠️ that **the schema field was deliberately kept** because Linux consumes it.
2. **I-0134 → `Closed/Issue-closed-0134.md` as a NON-ISSUE.** Records the erroneous parity premise, the
   user's ruling verbatim, and ⚠️ **"Re-open condition: NONE"** — this Issue does not become valid later;
   Linux changes are scheduled Linux work, not a re-opened cross-platform Issue. The observation is
   preserved; the framing is explicitly repudiated.
3. **Both rows removed** from `Issue-active.md`, replaced by a pointer note; the file now states
   **"Currently: 0 Issues awaiting verification."**
4. **Closed Issues table** in `Issue-Documentation.md` gained the I-0134 row.
5. ⚠️ **SP-100's plan note corrected** (§0 action 3 / R-01 action 6). It had claimed I-0133 was
   `Resolved - Not Verified` and I-0134 🔴 Open — **both read out of the stale rows in F-01 and propagated
   into a planning document.** The note now states the true dispositions and **records that its prior
   reasoning was wrong**, so the error is visible rather than silently overwritten.

**Deviations:** none.

**Verification:**

| Check | Result |
| ----- | ------ |
| Batch 14 **P4 parity** — rows vs `^## I-01` entries | ✅ **3 / 3** |
| `Issue-active.md` active rows for I-0133/I-0134 | ✅ **0** — the single `^\| I-013` hit is l.101, the **Archive-map** table (`I-0131 → batch 14`), not an active row |
| `Issues/Closed/` contents | ✅ `Issue-closed-0019.md`, `-0072-0103.md`, **`-0134.md`** |

⚠️ **R-01 action 5 observed:** no EP-026 parity work was opened. **No Linux change is owed by this ruling.**

**Status:** ✅ **Applied - Verified (2026-08-19)**

---

### R-02 — I-0018 archived Verified to its own decade

**Files changed:**
- `docs/Issues/Verified/Issue-verified-0011-0020.md` — I-0018 entry appended (batch 2 → **12 entries**)
- `docs/Issues/Issue-backlog.md` — entry removed; file is now **empty**
- `docs/Issues/Issue-Documentation.md` — backlog count 1 → **0**
- `docs/Issues/Verified/Issue-verified-0131-0140.md` — I-0133's cross-reference to I-0018

**What was done:**
1. **I-0018 → batch 2** (its own decade, per the ruling — grouping belongs in cross-references, not
   filing). ⚠️ **Both behaviours are recorded**, as the ruling required: the original 2026-06-08 complaint
   (no navigator selection on load — `viewportSceneID` left nil) **and** the rescoped one (manuscript not
   scrolled to the selection), the latter delivered by **I-0131**'s centring, verified 2026-08-18.
2. ⚠️ **The rescoping is called out as an error of process** inside the entry, with **P2** stated, plus the
   observation that **the same behaviour consumed two IDs** — I-0016 was superseded into I-0018, which was
   then retargeted.
3. **`Issue-backlog.md` is now EMPTY** and says so explicitly (*"Currently: 0"*), rather than being left
   with a stale count — the ruling's specific instruction.
4. **Cross-referenced from [[I-0133]]**, per the ruling's clarification.

⚠️ **Clarification applied, as flagged at ruling time:** the user said *"reference it in the text of
I-0134."* **I-0134 is closed as a non-issue with a repudiated premise (R-01)**, so anchoring a live
cross-reference there would point at a discarded record. **I-0133 was used instead** — same cluster,
verified, already citing I-0131/I-0132. ⚠️ **This remains reversible if I-0134 was in fact intended.**

**Deviations:** ⚠️ **Batch 2 has no summary table** — unlike batch 14, it is entries-only. **P4's
table/entry parity check does not apply to this file**; the entry was appended in the file's own format.
No table row was invented for consistency with a pattern this file does not use.

**Verification:**

| Check | Expected | Result |
| ----- | -------- | ------ |
| `grep -c "^## I-00"` batch 2 | 12 (was 11) | ✅ **12** |
| `grep -c "^## I-0018"` in `Issue-backlog.md` | 0 | ✅ **0** |
| `grep -l "^## I-0018" Verified/*.md` | batch 2 only | ✅ `Issue-verified-0011-0020.md` |

**Status:** ✅ **Applied - Verified (2026-08-19)**

---

### R-03 + R-05 — I-0118 reconstructed, then the parked block deleted

⚠️ **Applied as a pair, in this order, to satisfy ordering constraint #1.** Deleting first would have
destroyed the primary-source ruling text — **the exact failure that created F-03.**

**Files changed:**
- `docs/Issues/Verified/Issue-verified-0111-0120.md` — I-0118 entry (batch 12 → **9 entries**)
- `docs/Issues/Issue-active.md` — 62-line block deleted; "Full entries" preamble replaced; Archive map corrected
- `docs/Epics/Epic-backlog.md` — EP-033's origin line now links the archived Issue

**What was done — R-03 (reconstruction):**
Reconstructed I-0118 into batch 12 from **primary** sources: the user's own **Q1–Q4 ruling text** (which
survived verbatim in `Issue-active.md`) and [`Sprint-SP-105.md`](../Sprints/Closed/Sprint-SP-105.md). The
entry records the defect (⚠️ after T-0409, **nothing in a world was Spotlight-findable** — a regression the
test suite was *concealing*), all four rulings, the ScriviCore + Apple fix, the real-project probe
evidence, and the `world_world_<uuid>` defect **that every test passed with** because the tests asserted
the same expression the code computed.

⚠️ **Marked RECONSTRUCTED** with its date, sources, and the note that it was done **under QA observation** —
and that **P3 distinguishes this from back-filling**: the record was *written and lost*, not never written.

**What was done — R-05 (deletion):**
Deleted `Issue-active.md`'s 62-line *"I-0118 — design ruling"* block and its stale *"Full entries"*
preamble (which still described I-0134 as *"🔴 Open and awaiting a ruling"*). ⚠️ **Nothing was copied
elsewhere** — EP-033 gets a **cross-reference**, not a duplicate of the ruling text, per **P7**.

**Also corrected in passing:** the **Archive map** was stale — it listed `I-0051–I-0119`,
`I-0121–I-0122, I-0130` and `I-0131` only. Now `I-0051–I-0120`, `I-0121–I-0130`, `I-0131–I-0133`, plus a
row for **I-0134** in `Closed/`.

**Deviations:** none.

**Verification:**

| Check | Expected | Result |
| ----- | -------- | ------ |
| `grep -l "^## I-0118" Verified/*.md` | batch 12 | ✅ `Issue-verified-0111-0120.md` |
| Q1–Q4 ruling text present in the archive | 4 | ✅ **4** |
| `grep -c "I-0118"` in `Issue-active.md` | 0 | ✅ **0** |
| Batch 12 entry count | 9 (was 8) | ✅ **9** |
| ⚠️ **Ordering: R-03 applied before R-05** | required | ✅ **satisfied** |

✅ **`Issue-active.md` is now EMPTY of Issues** (0 active, 0 parked blocks) — R-01 + R-02 + R-05 combined.
⚠️ Its trailing activity log remains; that is **R-25**'s to remove.

**Status:** ✅ **Applied - Verified (2026-08-19)**

---

### R-04 + R-10 + R-11 + R-12 + R-13 + R-25 — the Issue index rebuilt

⚠️ **Applied together: five rulings edit `Issue-Documentation.md`** (constraint #6 pairs R-10 with R-25),
and **R-11 was held to LAST in the Issues layer** (constraint #9) so the count ran after all archiving.

**Files changed:**
- `docs/Issues/Issue-Documentation.md` — Active → 0; Verified re-derived; **ID accounting** added; activity log stripped
- `docs/Issues/Verified/Issue-verified-0011-0020.md` — header note (R-13)
- `docs/Issues/Verified/Issue-verified-0021-0030.md` — header note (R-13)
- `docs/Issues/Issue-active.md` — Archive map corrected

**What was done:**
- **R-10** — Active Issues → **0**; the *"18 Issues verified in one day"* narrative removed.
- **R-11** — batch table **re-derived by counting**, batch **14 added** (it had gone un-indexed for a day),
  and the **P4 + same-edit rules written into the file** so the rule lives where the next person adds a batch.
- **R-12** — batch 3 counted as **7** (raw headings) with the pointer-stub footnote; batch 2 as **12**.
- **R-13** — header notes on batches 2 and 3 explaining the irregularity **in the files themselves**, since
  the index note had existed since 2026-08-16 and did not stop this audit re-opening it.
- **R-04** — new **ID accounting** table: I-0059/I-0099 ⚪ never assigned, I-0016 ⚪ superseded → I-0018,
  and the four end-of-range IDs, each with its git evidence.
- **R-25** — activity log stripped (**157 → ~115 lines**), plus three stale narrative blocks in the Backlog
  section, replaced by the one durable rule they were circling: *archive in the same step you verify.*

⚠️ **DEVIATION — R-11/R-12's projected total of 123 was WRONG. The derived total is 122.**
The rulings reasoned: *baseline 120 + I-0018 + I-0118 + I-0133 = 123.* **The true baseline was 119**
(`git show HEAD` on every batch file). The error: **R-12 said to count batch 3 as 7 "instead of 6" and I
added 1 to the total — but the audit's original enumeration had already counted it as 7.** The "6" existed
only in the index's *prose*, never in the count. **The correction was applied twice.**

> ⚠️ **This is precisely what P6 exists to catch** — *statistics are derived, never balanced.* Had I
> written 123 to match the ruling's arithmetic, the index would have been wrong **and would have looked
> right.** The count was taken from the files; **the files won.**

**Verification:**

| Check | Expected | Result |
| ----- | -------- | ------ |
| Derived total across all 14 batches | — | ✅ **122** |
| Index's stated total | must equal derived | ✅ **122** |
| Active Issue rows | 0 | ✅ **0** |
| Backlog entries | 0 | ✅ *"Currently: 0"* |
| Per-batch counts vs files | all 14 match | ✅ |

**Status:** ✅ **Applied - Verified (2026-08-19)**

---

## ✅ Issues layer complete

**All 11 Issues-layer rulings applied** — §0, R-01, R-02, R-03, R-04, R-05, R-10, R-11, R-12, R-13, R-25
*(R-14/R-15 are guidelines, held for the one-sitting pass with R-19 per constraint #8)*.

**End state:** `Issue-active.md` **empty** · `Issue-backlog.md` **empty** · **122** verified across 14
batches · **3** closed files · every ID accounted for.

---

### R-06 + R-16 + R-17 + R-18 + R-25 + R-26 + R-27 — the Tasks layer

⚠️ **R-06 applied BEFORE R-18** (constraint #3) and **R-18 held to LAST** (constraint #10);
**R-16 + R-17 applied as one edit** (constraint #7).

**Files changed:**
- `docs/Tasks/Closed/Task-closed-0185-0188.md` — **new**
- `docs/Tasks/Task-Documentation.md` — 4 rows reclassified · statistics **re-derived** · unfiled register 180 → 176 · activity log stripped (**700 → 679 lines**)
- `docs/Tasks/Task-backlog.md` — T-0118 detail folded in; T-0185–T-0188 pointer updated
- `docs/Tasks/Task-unverified.md` — stale prose removed
- `docs/Tasks/Task-0118.md` — **`git rm`**

**What was done:**
- **R-06** — T-0185–T-0188 filed to `Closed/` as ⚪ **Descoped**, with the I-0057 / Apple-DTS cause and
  ⚠️ **the sole re-open condition (the `MDImporter` route)**. Index rows corrected ✅ Verified → ⚪ Descoped.
- **R-16 + R-17** — both stale prose blocks deleted in one edit (T-0389 *"is now listed"* against an empty
  table; *"SP-102 is still active"*).
- **R-18** — statistics **re-derived by enumerating all 417 IDs** from the All-Tasks Status column.
- **R-25** — activity log stripped from `Task-Documentation.md`.
- **R-26** — unfiled register **180 → 176**, matching the four that left under R-06. ✅ **Rows recounted:
  exactly 176.**
- **R-27** — `Task-0118.md` folded into the backlog **verbatim** (rationale, requirements, design approach,
  dependencies, test steps), then `git rm`'d. ⚠️ **Its stale `Sprint Assigned: SP-034` was corrected to
  "Not Assigned"** — SP-034 was cancelled.

**Derived statistics (P6 — counted, never balanced):**

| Status | Count |
| ------ | ----- |
| ✅ Verified | **379** |
| 🔵 Backlog | 21 |
| 🟠 Implemented - Not Verified | 4 |
| ⚪ Descoped | 4 |
| ⚪ Superseded | 3 · ⚪ Closed | 2 · ⛔️ OBE | 2 · ⚪ Never assigned | 1 · ⚪ Unassigned | 1 |
| **Total** | **417** |

⚠️ **DEVIATION — R-18's projected numbers were wrong, and the enumeration corrected them.** The ruling
carried **Verified 376 → 372**. The derived figure is **379**, because the enumeration found **eight rows
that were themselves stale** — a defect no ruling had identified:

| Rows | Was | Corrected to | Authority |
| ---- | --- | ------------ | --------- |
| T-0365, T-0389, T-0415 | 🟡 Active (2026-08-17) | ✅ **Verified** | `Sprint-SP-102.md` l.80-83 |
| T-0417 | 🟠 Impl-Not-Verified | ✅ **Verified 2026-08-18** | `Sprint-SP-102.md` l.83 |

**SP-102's four Tasks were all Verified on 2026-08-17/18 and the index never learned.** ⚠️ **This is the
same defect class as F-01** — a closed sprint's outcomes not propagating — found here only because R-18
required reading **every** row rather than adjusting a total.

**⚠️ NOT resolved — flagged for the user (see "Not applied / deferred"):** **T-0298–T-0301** remain
🟠 Implemented - Not Verified. `Sprint-SP-074.md` is **closed** yet its task table still reads *"🟢
Implemented, Not Verified"*, and no `Verified/` file exists for them. **The archive does not settle it, so
I did not.**

**Verification:**

| Check | Result |
| ----- | ------ |
| Enumeration total | ✅ **417** — every ID classified, none unclear |
| Statistics vs enumeration | ✅ identical (**not** reconciled to a target) |
| Unfiled register rows | ✅ **176** — matches the stated figure |
| `Task-0118.md` dangling links | ✅ **0** |
| `Tasks/Closed/` | ✅ 3 files (`0125`, `0127`, **`0185-0188`**) |

**Status:** ✅ **Applied - Verified (2026-08-19)**

---

## ✅ Tasks layer complete

**R-06, R-16, R-17, R-18, R-25, R-26, R-27 applied.** *(R-19 is guidelines — held for the one-sitting pass
with R-14/R-15, constraint #8.)*

---

### R-07 + R-20 — SP-039 rescued, then the 12 duplicate drafts deleted

⚠️ **Applied as a pair, in this order, to satisfy ordering constraint #2.** A bulk delete first would have
destroyed the only sprint-level record of the timeline C ABI.

**Files changed:**
- `docs/Sprints/Sprint-SP-039.md` → **`docs/Sprints/Closed/Sprint-SP-039.md`** (`git mv`), relabelled
- 12 loose drafts — **`git rm`**
- `docs/Tasks/Task-Documentation.md` — T-0139–T-0148 corrected; statistics re-derived; register 176 → 186

**R-07 — what was done:**
1. `git mv` to `Closed/`; **Status 🔵 Planning → ✅ Closed 2026-06-11**.
2. ⚠️ **RECONSTRUCTED banner** naming its sources, stating **no retrospective was ever written**, and
   citing the precedent (`Sprint-SP-056.md`).
3. ⚠️ **Task table reconciled** — all ten Tasks were frozen at planning-time **🔵 Backlog**. Corrected to
   ✅ Verified, in the summary table **and** in all ten per-Task detail blocks.

**Corroboration before relabelling — two independent primary sources:**

| Evidence | Result |
| -------- | ------ |
| `Epic-EP-016.md` l.42 | SP-039 ✅ **Closed** |
| `Epic-EP-016.md` l.52-61 | **all ten Tasks T-0139–T-0148 ✅ Verified** |
| `scrivi.h` | **18 `timeline` symbols** — the C ABI physically shipped |

⚠️ **A THIRD instance of the F-01 defect class, found by this ruling.** `Task-Documentation.md` carried
**T-0139–T-0148 as 🔵 Backlog** — unstarted work — for ten Tasks verified in June, on an Epic closed
2026-06-23. **A closed sprint's outcomes had never propagated to the index.** Corrected here.

⚠️ **Consequent statistics change (P6 — the count follows the files):**

| Line | After R-18 | After R-07 |
| ---- | ---------- | ---------- |
| ✅ Verified | 379 | **389** |
| 🔵 Backlog | 21 | **11** |
| ⚠️ Verified-but-unfiled register | 176 | **186** |

**The ten were added to the unfiled register**, since they are Verified with no `Verified/` file — leaving
them out would have made the register disagree with the statistics.

**R-20 — what was done:** deleted the 12 duplicate drafts with `git rm` (recoverable from history).

**⚠️ SAFEGUARD EXECUTED — and it nearly changed the outcome.** A token-level diff of **all nine**
previously-unchecked drafts found **13–39 tokens per file absent from the archives**, well above what
planning scaffolding alone explains. Investigated rather than assumed:

- Most are **planning vocabulary** — *Capacity*, *Depends*, *end_date*, *Backlog*, *Execution order*.
- ⚠️ **SP-005's draft names five API symbols absent from its archive** — `JsonDoc::appendToArray`,
  `arraySize`, `arrayItem`, `util::replaceExtension`, `GitSnapshotTests`. **Read in context**, l.30 is a
  planning-time **risk note**: *"Key risks: … existence. If either is missing, the fix goes into that task
  — not a new task."* **A contingency, not a design decision or a ruling** — and the archive records the
  outcome. ✅ **Nothing lost.**

**Verification:**

| Check | Result |
| ----- | ------ |
| SP-039 safe in `Closed/` before deletion | ✅ confirmed, ordering held |
| `Sprint-SP-039.md` residual 🔵 Backlog statuses | ✅ **0** (1 summary table + 10 detail blocks fixed) |
| Loose `Sprint-SP-*.md` remaining | ✅ **0** |
| `docs/Sprints/` contents | ✅ exactly the 4 guideline files + `Closed/` |
| `Closed/` archives | ✅ **102** (was 101) |
| Register rows vs stated figure | ✅ **186 = 186** |

**Status:** ✅ **Applied - Verified (2026-08-19)**

---

### ⚠️ T-0298–T-0301 — RESOLVED by user ruling (2026-08-19)

**Not an audit ruling** — this was deferred by remediation, referred to the user, and ruled on the spot.

> **User ruling:** *"You may record these Tasks as Verified and make sure they are properly archived.
> Also update SP-074 so that the Task state matches reality."*

**Files changed:**
- `docs/Tasks/Verified/Task-verified-0298-0301.md` — **new**
- `docs/Sprints/Closed/Sprint-SP-074.md` — task table, I-0083 row, retrospective, verification heading, correction note
- `docs/Tasks/Task-Documentation.md` — 4 rows → ✅ Verified; statistics re-derived

**What was done:**
1. **Archive written** covering all four Tasks, preserving SP-074's implementation detail — the T-0298 red
   repro, `SceneMerger`, and ⚠️ **T-0300's ordering fix for I-0083: *files move BEFORE the folder is
   deleted, and that ordering is the whole fix***.
2. **SP-074 reconciled with reality** — 4 task rows and the I-0083 row → ✅ Verified; retrospective and
   verification heading corrected; a **correction note** appended explaining the four-week gap.
3. **Index rows** → ✅ Verified with archive links.

**Why the records could not settle it, and what actually happened:** SP-074's retrospective (l.80) set a
gate — *"Do not close EP-028 or mark I-0083 fully Verified until app adoption lands and is
user-verified."* ⚠️ **That gate was passed the very next day:** SP-075 (Apple adoption) and SP-076 (Linux
parity) both closed 2026-07-21/22, and **EP-028 closed 2026-07-22**. **Nothing came back to update
SP-074.**

> ⚠️ **The lesson is the gate, not the delay.** A retrospective that defers verification to a later sprint
> must name **who closes the loop**. This one named the condition but no owner — so the condition was met
> and nobody returned. Recorded in `Sprint-SP-074.md` itself.

⚠️ **DEVIATION — a regex error caught by re-deriving.** The first pass corrected only **2 of 4** rows:
`T-0(298|299|300|301)` mis-grouped and matched T-0300/T-0301 only. **The enumeration exposed it
immediately** (Verified 391, ImplNotVerified still 2) and it was fixed. ✅ **P6 catching a mistake made
*during* remediation, not merely one inherited from before it.**

**Statistics after this change** — Verified **389 → 393**, ⚠️ **Implemented-Not-Verified 4 → 0**, matching
`Task-unverified.md`'s empty table. **The F-18 contradiction that started this is now fully closed.**

**Verification:**

| Check | Result |
| ----- | ------ |
| Residual *"Not Verified"* in `Sprint-SP-074.md` | ✅ **0** |
| Enumeration: ImplNotVerified | ✅ **0** — was 9 (claimed) / 4 (actual) |
| Enumeration total | ✅ **417** |
| Statistics vs enumeration | ✅ identical |

**Status:** ✅ **Applied - Verified (2026-08-19)**

---

### R-21 + R-22 + R-28 — the Sprint index, guidelines, and backlog

⚠️ **R-21 verified before R-22(④)** (constraint #4) — the All-Sprints table had to be correct **before**
it became the single source.

**Files changed:**
- `docs/Sprints/Sprint-Documentation.md` — Statistics reduced; "Currently:" line removed
- `docs/Sprints/Sprint-GUIDELINES.md` — close checklist rewritten; file-structure rules; planning-draft and fully-planned rules
- `docs/Sprints/Sprint-backlog.md` — closure notes stripped (**273 → 50 lines**)

**R-21 — verified, not re-applied.** The correction was already in the tree under §0 Group A: SP-102's
All-Sprints row reads ✅ Closed 2026-08-18. ✅ **Confirmed it survived the Group B revert.**

**R-22 ④ — STOP RESTATING (the third-recurrence structural fix).** `Sprint-Documentation.md` now states
plainly that **the All-Sprints table is the single source**. Removed: *Closed: N*, *Active: N*,
*Planning: N*, *Superseded/Cancelled: N*, *Paused: N*, and the All-Sprints *"Currently: 104 issued — 101
closed…"* line. **Kept** — what the table cannot express: next available ID, total issued, and the IDs
that are **not** sprints (SP-034 cancelled, SP-057 superseded, SP-068 skipped) with reasons, plus the
out-of-sequence executions. ⚠️ **A note forbids reintroducing per-status counts**, citing the three
recurrences.

⚠️ **The old "Closed: 101" was ALREADY wrong again** — SP-039's archive (R-07) made it 102 within the
hour. **Derived and cross-checked:** 102 `Closed/` archives ↔ 102 ✅ Closed rows. *The restated figure
went stale during the very session that removed it.*

**R-22 ①②③ — guidelines.** ① planning drafts are **transient** and never outlive a close; ② a Sprint may
be **fully planned while 🔵 Planning**, banner-marked, activation separate and user-approved; ③ the close
checklist rewritten **by layer** — Sprint / Epic / **Task** / Issue — naming every file that restates a
sprint's status. ⚠️ **The three Task-layer files that drifted at SP-102's close were on no checklist
before today.** Added: *if a retrospective defers verification, it must name **who** closes the loop*
(the SP-074 lesson).

**R-28 — backlog stripped, and the rule amended so it cannot refill.** *"A Sprint leaves the backlog at
ACTIVATION and never returns. No closure note is written there."*

**⚠️ SAFEGUARD EXECUTED — the R-28 check, and it was the one most likely to lose something.** Several
notes carried substantive **planning rulings**, not just status. Each was checked against its archive
**before** deletion:

| Ruling in the note | Archive | Present? |
| ------------------ | ------- | -------- |
| SP-103 scope ruling (all kinds world-scoped) | `Sprint-SP-103.md` | ✅ 5 hits |
| **T-0410 removed as OBE** + reasoning | `Sprint-SP-103.md` l.117 | ✅ verbatim |
| SP-096 R1 — ID-prefix rule verified broken | `Sprint-SP-096.md` | ✅ 20 hits |
| SP-095 — world kinds "declared but gated" | `Sprint-SP-095.md` | ✅ 6 hits |
| SP-097/098/099/102/105/106 rulings | each archive | ✅ 3–29 hits each |

✅ **Nothing existed only in the backlog. P3 was not invoked — nothing needed reconstructing.**

⚠️ **DEVIATION — a first pass left two closure notes behind.** Cutting at the first note in the wall
missed SP-106's and SP-102's, which sat *above* it. **Caught by verification** (`grep -c "✅ CLOSED"`
returned 2, not 0) and removed. **273 → 50 lines.**

**Verification:**

| Check | Result |
| ----- | ------ |
| `Closed/` archives ↔ ✅ Closed rows | ✅ **102 ↔ 102** |
| Per-status counts in Statistics | ✅ **0** — table is the single source |
| Closure notes in `Sprint-backlog.md` | ✅ **0** (was ~230 lines) |
| `docs/Sprints/` contents | ✅ 4 guideline files + `Closed/` |
| Planning rulings lost | ✅ **none** — all corroborated before deletion |

**Status:** ✅ **Applied - Verified (2026-08-19)**

---

## ✅ Sprints layer complete

**R-07, R-20, R-21, R-22, R-28 applied.**

---

### R-08 + R-23 + R-24 — the Epic layer

⚠️ **One file-open; R-23(①) applied before R-23(②)** (constraint #5) — the table had to be correct before
it became the single source.

**Files changed:** `docs/Epics/Epic-Documentation.md`, `docs/Epics/Epic-active.md`

- **R-23 ①** — three corrections: *"9 closed … SP-102 🟡 Active"* → **10 closed, only SP-100 remains**;
  Statistics likewise; and ⚠️ **the false claim that AC9 was outstanding** → **AC9 was MET 2026-08-18;
  only AC1 and AC10 remain.** That one was **wrong, not merely stale**, and SP-100's whole job is
  verifying AC1–AC10.
- **R-08** — **EP-034** added to the All-Epics table and the Backlog section, with its ⚠️ *owes source
  creation* note. Next available ID **EP-034 → EP-035**, closing the collision hazard.
- **R-23 ②** — **P7 applied**: the All-Epics table is now the single source. Per-status counts removed;
  Statistics keeps only next-available ID, total issued, and where per-Epic detail lives. ⚠️ A note records
  that **this file is where the correct diagnosis was first written and then not acted on.**
- **R-24** — AC9's struck-through *"In progress — SP-102 active"* and *"AC9 ticks when SP-102 lands"*
  replaced with a plain past-tense delivery account. ⚠️ **The other nine AC entries were NOT touched** —
  the 582-line commentary pass is deferred to SP-100/T-0391.

**Derived:** 34 rows — 29 Closed, 1 Active, 1 Draft, 3 Proposed.
⚠️ **Deviation:** EP-034's row was first written with `**EP-034**` bolding, which broke the `^| EP-0`
pattern every count depends on. **Caught by re-deriving** (34 rows returned 1), unbolded, re-derived.

**Status:** ✅ **Applied - Verified (2026-08-19)**

---

### R-29 + R-14 + R-15 + R-19 + R-31 — guidelines and CLAUDE.md

⚠️ **All four guidelines files edited in one sitting** (constraint #8).

**Files changed:** all four `*-GUIDELINES.md` / `*-Guidelines.md`, `CLAUDE.md`

- **R-29** — Audit-layer cross-reference added to **all four** layer guidelines, each stating that an Audit
  **begins only on user request**. In `Epic-GUIDELINES.md`, *"Completing an Epic"* gains **step 1: run an
  Audit Check** — with the explicit warning that ⚠️ **a Check is not an Audit and an Epic close does not
  trigger one**, and the two EP-031 near-misses as its justification.
- **R-14** — four wrong paths in `Issue-GUIDELINES.md` (`Documentation/Issues/` → `docs/Issues/`).
- **R-15** — all five defects: `DR-verified-*` naming, 🟡 → **🟠** for Resolved-Not-Verified, the batched
  backlog filename, and ⚠️ **the checklists naming `Issue-unverified.md` — a file that does not exist.**
  Now: *archive from `Issue-active.md` **or** `Issue-backlog.md`, in the same step you verify.* **That
  missing clause caused both F-02 and F-03.**
- **R-19** — all six: paths, `ER`/`Taskss`/`Asigned`/`aproval`, the mangled lifecycle diagram (now showing
  ⚪ Closed as a live state), 🟡→🟠 **in lockstep with R-15**, and the seven-month-stale version footer.
- **R-31** — CLAUDE.md gains an **Audits** subsection, placed **below** the four layers as orthogonal
  rather than as a fifth link in the chain, carrying the three always-in-force rules. ⚠️ **P1–P7 were
  deliberately NOT copied there** — they belong in the layer guidelines that enforce them, and restating
  them would be the very disease P7 names.

**Verification:** residual defect greps → **0** in both guidelines files.

**Status:** ✅ **Applied - Verified (2026-08-19)**

---

## ✅ Remediation complete — final verification

| Layer | Check | Result |
| ----- | ----- | ------ |
| **Issues** | derived verified total ↔ index | ✅ **122 ↔ 122** |
| **Issues** | active / backlog | ✅ **0 / 0** |
| **Tasks** | IDs enumerated | ✅ **417**, every one classified |
| **Tasks** | Implemented-Not-Verified ↔ `Task-unverified.md` | ✅ **0 ↔ 0** *(the F-18 contradiction, closed)* |
| **Sprints** | `Closed/` archives ↔ ✅ Closed rows | ✅ **102 ↔ 102** |
| **Sprints** | orphan files in `docs/Sprints/` | ✅ **0** *(was 13)* |
| **Epics** | All-Epics rows | ✅ **34** incl. EP-034 |
| **All** | per-status counts restated outside their table | ✅ **0** — P7 applied to both indexes |

### What the enumerations found that no ruling had

⚠️ **Three further instances of the F-01 defect class — a closed sprint's outcomes never reaching the
index — surfaced only because P6 required counting every row rather than adjusting a total:**

| Rows | Shown as | Actually | Stale since |
| ---- | -------- | -------- | ----------- |
| T-0365, T-0389, T-0415, T-0417 | 🟡 Active / 🟠 Not Verified | ✅ **Verified** | 2026-08-17/18 |
| T-0139–T-0148 | 🔵 **Backlog** *(unstarted!)* | ✅ **Verified** | **2026-06** |
| T-0298–T-0301 | 🟠 Not Verified | ✅ **Verified** *(user ruling)* | 2026-07-21 |

**Verified moved 372 (projected) → 379 → 389 → 393** as each was found. ⚠️ **Had the statistics been
"corrected" to the ruling's projected figure, all three would still be hidden** — and the index would have
looked right.

### Deviations, in full

1. **§0** — `git checkout` was a no-op (changes were staged); used `git restore --staged --worktree`.
2. **§0** — a stated verification expectation was wrong (`T-0418` count); corrected in place.
3. **R-11/R-12** — projected 123 verified Issues; **derived 122**. The batch-3 correction had been applied
   twice.
4. **R-18** — projected 372 verified Tasks; **derived 393** after three stale-row discoveries.
5. **R-20** — first cut missed two closure notes; caught by verification.
6. **T-0298–T-0301** — a regex mis-grouping corrected only 2 of 4 rows; caught by re-deriving.
7. **R-23** — bolded table row broke the count pattern; caught by re-deriving.

⚠️ **Every one was caught by a mechanical check, not by review.** That is the case for P4 and P6 being
rules rather than habits.

---

## Not applied / deferred

*(nothing outstanding — the one deferred item, T-0298–T-0301, was ruled by the user and applied above.)*

<details>
<summary>Original deferral text, retained for the record</summary>

### ⚠️ T-0298–T-0301 — status unresolved, needs a user ruling

**Surfaced by:** R-18's enumeration. **Not covered by any ruling in this audit.**

Four SP-074 Tasks (`scrivi_merge_scene` / `scrivi_merge_chapter` and their coverage) are recorded
🟠 **Implemented - Not Verified** in `Task-Documentation.md`. The evidence conflicts:

- ✅ **SP-074 is closed** — *"Status: ✅ Closed (Human-approved 2026-07-21)"*
- ⚠️ **Its own task table still reads** *"🟢 Implemented, Not Verified"* for all four (l.15-18)
- ⚠️ **Its retrospective repeats it** (l.64) and adds (l.80): *"Do not close EP-028 or mark I-0083 fully
  Verified until app adoption lands and is user-verified."*
- ⚠️ **No `Verified/Task-verified-029*.md` exists**
- ✅ **EP-028 has since closed** (2026-07-22), which is the condition l.80 named

**The question:** did EP-028's close verify these four, or were they closed-with-the-sprint while never
individually verified? ⚠️ **Claude cannot mark a Task Verified**, and the archive does not state it, so the
rows are **left as-is**.

**Recommendation:** treat as a single ruling — either they were verified by EP-028's close (and get
archived), or they are genuinely unverified work inside a closed Epic, which is a finding in its own right.

---

*Nothing else deferred.*

---

*Log opened 2026-08-19.*

</details>

---

## ✅ Cycle closed — verified 2026-08-19

> **User, 2026-08-19:** *"The approval to begin collection of Rulings was evidence of the Verification of
> the Audit Findings. The approval to begin Remediation was evidence of the verification of the Audit
> Rulings. I have completed my review of the Remediation Log and I agree that the Audit has been properly
> verified."*

⚠️ **Verification is carried by the PHASE GATES, not by a separate sign-off step.** Each approval to
proceed verified the artifact behind it:

| Artifact | Verified by | When |
| -------- | ----------- | ---- |
| `Audit-Findings-20260819.md` | ✅ approval to **begin the rulings session** | 2026-08-19 |
| `Audit-Rulings-20260819.md` | ✅ approval to **begin remediation** | 2026-08-19 |
| `Audit-Remediation-20260819.md` *(this file)* | ✅ **review of the log** | 2026-08-19 |

**All 32 rulings are ✅ Applied - Verified.** The three-phase cycle opened and closed on 2026-08-19.

*This structural property was not designed in — it was named by the user at close, and belongs in
`Audit-Guidelines.md` so the next cycle does not invent a redundant sign-off step.*
