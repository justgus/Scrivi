# Documentation Audit — Rulings — 2026-09-15

**Findings file:** [`Audit-Findings-20260915.md`](Audit-Findings-20260915.md) — ✅ **verified by the
user's approval to begin this rulings session** (`Audit-Guidelines.md` §"Verification is carried by the
PHASE GATES").
**Ruled by:** the user, 2026-09-15.
**Status:** ✅ **COMPLETE — all 11 findings ruled.**

⚠️ **PHASE 2 CHANGES NOTHING.** ✅ **A ruling is a decision recorded, not a fix applied.** ⚠️ **No
tracking document was edited in producing this file.** ✅ **Remediation is Phase 3 and needs its own
approval.**

---

## Ruling summary

| # | Severity | Ruling | Action |
| - | -------- | ------ | ------ |
| **R-01** | Critical | F-01 → **extend the All-Sprints table to SP-134** | Append rows from `Sprints/Closed/` |
| **R-02** | Critical | F-02 → **option (b): REMOVE the figures** | Delete Sprint Statistics counts; point to the table |
| **R-03** | Critical | F-03 → **option (b): REMOVE the total** | Delete the Task total; point to ID-accounting |
| **R-04** | Critical | F-04 → **options (b) + (c)** | Account for the five as never-assigned; ⛔ **do NOT rename the file** |
| **R-05** | Moderate | F-05 → **option (b)** | Add T-0138 to accounting; correct count 1 → **2** |
| **R-06** | Moderate | F-06 → **correct the row** | Link `Epic-EP-038.md`; re-check EP-035–037 |
| **R-07** | Moderate | F-07 → **move EP-039 NOW** | → `Epics/Closed/Epic-EP-039.md` |
| **R-08** | Moderate | F-08 → **option (b): REMOVE the count** | Collapse the stacked lines |
| **R-09** | Moderate | F-09 → **backfill (see R-11)** | Count is superseded by the backfill itself |
| **R-10** | Minor | F-10 → **option (a)** | Fix Statistics only; changelog entries are dated history |
| **R-11** | Minor | F-11 → **option (b): REMOVE per-status totals** | ⚠️ **AND backfill the 139 unfiled Tasks** |

---

## Rulings

### R-01 — F-01: extend the All-Sprints table to SP-134

✅ **RULED: the table is the single source and must reach the highest issued ID.**
⚠️ **SP-116–SP-134 get rows, sourced from `Sprints/Closed/`.**
✅ **Rationale: this recurs 2026-08-19's F-08 in a different layer.** ⚠️ **Demoting the "single source"
claim (option c) was rejected implicitly by ruling the table authoritative.**

### R-02 — F-02: REMOVE the Sprint Statistics figures — **option (b)**

✅ **RULED by the user: "Q3. remove."**
⚠️ **Delete "Next available Sprint ID", "Total Sprint IDs issued", and the executed/active counts.**
✅ **Replace with a pointer to the All-Sprints table (R-01 makes that table complete enough to carry it).**
✅ **Rationale: this applies the 2026-08-19 R-22(④) precedent — already applied to Epics — to the
Sprint layer**, ⚠️ **which finding S-1 identified as the reason these drifted at all.**
⚠️ **CARRY-FORWARD: the next-available ID must still be discoverable.** ✅ **Remediation must state
where it now lives** (⚠️ **`Sprint-active.md` already carries it**) — ⛔ **removal must not make ID
allocation guesswork, which is the collision risk F-02 raised.**

### R-03 — F-03: REMOVE the Task total — **option (b)**

✅ **RULED by the user: "Q3. remove."** ⚠️ **Same treatment as R-02, same rationale.**
⚠️ **The ID-accounting table is RETAINED** — ✅ **it is not a restated count; it is the record that
distinguishes an unassigned ID from a lost one.**

### R-04 — F-04: account for the five missing Issues; do not rename — **options (b) + (c)**

✅ **RULED by the user: "The info you seek is not in git. Therefore F4 is option b and c."**
⚠️ **Option (a) — reconstruct from git — is FORECLOSED: the user states the information is not there.**
✅ **ACTION: record **I-0181, I-0187, I-0188, I-0189, I-0190** in the Issue ID-accounting table.**
⛔ **DO NOT rename `Issue-verified-0181-0190.md` to match its contents.**
✅ **Rationale for (c): a filename that admits a gap SURFACES the loss; one that matches its contents
HIDES it.** ⚠️ **The file keeps its name as a standing marker.**
⚠️ **NOTE FOR REMEDIATION: I-0187 is cited by `Sprint-SP-127.md:81` as that sprint's Issue.** ✅ **The
accounting row must record that citation**, ⚠️ **so a future reader finds the reference rather than
concluding the sprint cited nothing.**

### R-05 — F-05: add T-0138 to accounting; correct the count to 2 — **option (b)**

✅ **RULED by the user** (clarified 2026-09-15: F-05 had no option (c); the user confirmed **(b)**).
⚠️ **Option (a) — git — is FORECLOSED for the same reason as R-04.**
✅ **ACTION: add a T-0138 row to the Task ID-accounting table as ⚪ Never assigned, and change
"⚪ Never assigned | 1" to "| 2".**
⚠️ **T-0278's existing row stands** — ✅ **both are gaps; the file was wrong that there was only one.**

### R-06 — F-06: correct EP-038's row and re-check its siblings

✅ **RULED: the row's claim is false and must be corrected to link `Epics/Closed/Epic-EP-038.md`.**
⚠️ **REMEDIATION MUST ALSO re-check EP-035, EP-036 and EP-037** — ✅ **they were added in the same
2026-09-14 edit, by the same method (asserting from the index without opening `Epics/Closed/`).**
⚠️ **One verified-false claim from that batch is reason to verify the rest, not to assume they are fine.**

### R-07 — F-07: move EP-039 to `Epics/Closed/` now

✅ **RULED by the user: "Q2. move now."**
⚠️ **Deferring until the EP-040 close (option b) is REJECTED** — ✅ **the memory note
`feedback_archive_on_close` records that deferral rotted four files by 2026-08-15.**
⚠️ **REMEDIATION NOTE: EP-039's record carries [I-0206] and [I-0213] as CARRIED INTO EP-040.** ✅ **The
move must not break those references** — ⚠️ **EP-040's own Issues table already lists both, so the
carry survives the move; ✅ verify that before moving, not after.**

### R-08 — F-08: remove the stacked count from `Issue-active.md` — **option (b)**

✅ **RULED by the user: "F2, F3, F8, and F11 are all option b."**
⚠️ **Delete the stacked "Currently: N records" lines; state that the count is obtained by reading the
rows.** ✅ **This is the R-23(②) precedent from the Epic layer.**
⚠️ **The prior-count text is NOT simply deleted** — ✅ **see G-3 in the findings: where a preserved line
carries a dated ruling or explanation, remediation moves it under a dated heading rather than
discarding it.** ⛔ **Do not lose history to tidy a count.**

### R-09 — F-09: the count is superseded by the backfill

✅ **RULED: the "133 rows" figure is not re-derived** — ⚠️ **R-11's backfill removes the gap it
describes.** ✅ **The note is rewritten to record what the gap WAS and when it was closed.**

### R-10 — F-10: fix Statistics only — **option (a)**

✅ **RULED: the 13 occurrences are mostly dated changelog entries and are historically correct in place.**
⚠️ **Only the Statistics instance is wrong — ✅ and R-02 deletes it.**
⚠️ **No convention change is mandated** (the findings' option b), ⛔ **but see G-2/G-3 for the
guidelines pass.**

### R-11 — F-11: remove per-status totals **AND backfill the 139 unfiled Tasks**

✅ **RULED by the user: "F11 ... option b"** (remove per-status totals) ⚠️ **AND, on Q-4:**
> ⚠️ ***"One audit has deferred this, and one Quick audit surfaced it again without checking prior
> audit findings. This audit will not defer it. Backfill them."***

✅ **THE USER'S CORRECTION IS RECORDED AS PART OF THE RULING:** ⚠️ **I described this as "deferred by
two audits."** ✅ **It was deferred by ONE audit (2026-08-19).** ⚠️ **The second surfacing was my
Audit CHECK of 2026-09-15, which re-raised a known, documented, already-scoped-out condition BECAUSE I
DID NOT READ THE PRIOR FINDINGS FILE FIRST** — ✅ **which the guidelines require precisely so that
recurrence is visible rather than re-discovered.**

✅ **BACKFILL FORMAT, ruled by the user: BATCHED ARCHIVE FILES BY EPIC/SPRINT** — ⚠️ **not 139
per-task files, and not one consolidated file.** ✅ **Mirrors the Issue layer's decade-batching.**
⚠️ **The sprint archive REMAINS the cited record of truth;** ✅ **the batch file makes the Task layer
enumerable without duplicating the sprint's detail.**

⚠️ **CONSTRAINT CARRIED FROM THE FINDINGS: 24 of the 139 carry an EPIC ATTRIBUTION MARKED
"INFERRED - verify".** ⚠️ **Those sprints have no declared `**Epic:**` header, so the attribution came
from the first `EP-0xx` mention in the text** — ✅ **T-0090–T-0094 are the clearest suspects
(ScriviCoreAdapter removal, attributed to "Writing Surface and Scene Navigator").**
⛔ **REMEDIATION MUST NOT WRITE AN INFERRED EPIC AS FACT.** ✅ **Either verify each against its sprint
archive, or record the attribution as uncertain in the batch file.**

---

## Ruled-out options, recorded

⛔ **F-04 (a) — reconstruct the five Issues from git.** ⚠️ **User states the information is not in git.**
⛔ **F-05 (a) — establish T-0138 from git.** ⚠️ **Same.**
⛔ **F-07 (b) — defer the EP-039 move.** ⚠️ **Rejected; deferral is the documented rot pattern.**
⛔ **F-02/F-03 (a) — re-derive the figures.** ⚠️ **Rejected in favour of removal (S-1).**
⛔ **F-04 — renaming the batch file.** ⚠️ **Explicitly prohibited by R-04.**

---

## Guidelines recommendations — carried to Phase 3 as proposals, NOT rulings

⚠️ **SUPERSEDED — see "Guidelines rulings" below; all three WERE ruled.** ⚠️ **Original text:**
⚠️ **G-1, G-2 and G-3 from the findings file were NOT ruled on.** ✅ **They are recommendations about
`Audit-Guidelines.md`, `Sprint-GUIDELINES.md` and `Epic-GUIDELINES.md`** — ⚠️ **the user's rulings
addressed findings, not guidelines.**
✅ **Remediation should raise them for a separate decision** ⛔ **and must not apply them unruled.**

⚠️ **G-2 deserves emphasis: my first ID-continuity pass reported 167 missing Issues** because archived
entries use `## I-0xxx:` headings while active ones use table rows. ✅ **A false catastrophe of that
size could prompt a destructive "repair"** — ⚠️ **the check must know both formats.**

---

## Guidelines rulings — ✅ RULED 2026-09-15, superseding the "not ruled" note above

⚠️ **The section above states G-1/G-2/G-3 were NOT ruled.** ✅ **The user ruled them immediately after;
✅ THIS SECTION IS AUTHORITATIVE.**

### R-12 — G-1 ✅ **APPROVED**
✅ **Add to the Sprint and Epic guidelines: a table declared authoritative MUST extend to the highest
issued ID.** ✅ **One-line check: highest row == highest issued ID.** ⚠️ **Catches F-01 and
2026-08-19's F-08, which are the same defect in two layers.**

### R-13 — G-2 ✅ **APPROVED**
✅ **`Audit-Guidelines.md` ID-continuity check must recognise BOTH row formats** — ⚠️ **`## I-0xxx:`
headings (archives) AND `| **I-0xxx** |` table rows (active/backlog).** ⚠️ **A single-format checker
reported 167 missing Issues in this very audit** — ✅ **a false catastrophe that could prompt a
destructive "repair".**

### R-14 — G-3 ✅ **APPROVED WITH A STRONGER RULE THAN PROPOSED**
⚠️ **I proposed moving preserved prior text under a dated heading.** ✅ **THE USER RULED FURTHER:**
> ⚠️ ***"sometimes preserved prior text is as destructive as omitting it. We recommend removing the
> prior text, especially for state comparison."***

✅ **RULING: REMOVE prior text rather than preserve it, wherever it states STATE** — ⚠️ **counts,
statuses, "next available" IDs, verification claims.** ✅ **F-08 is the proof: stacked
"Currently: N records" lines produced three contradictory counts in one file and NONE was correct.**
⚠️ **A reader cannot tell which preserved line is current, so preservation actively misleads.**
⚠️ **This CONSTRAINS R-08:** ⛔ **do NOT move the stacked count lines under a dated heading —
✅ DELETE them.** ⚠️ **Preservation remains acceptable only for RATIONALE (why a decision was made),
never for STATE.**

### R-15 — S-1 ✅ **BROADEN THE SCOPE**
✅ **The user ruled: "S1. broaden the scope."** ⚠️ **2026-08-19's R-22(④)/R-23(②) removed per-status
totals from the EPIC layer only** — ✅ **and the Task and Sprint layers, left untouched, are exactly
the two that drifted into this audit's criticals.**
✅ **RULING: the "no restated summary counts" rule applies to ALL FOUR LAYERS**, ⚠️ **not case-by-case.**
✅ **R-02, R-03, R-08 and R-11 are instances of this rule, not independent decisions.**

### R-16 — S-2 ✅ **DO NOT SURFACE STATUS MID-INVESTIGATION**
✅ **The user ruled: "S2. do not surface status mid investigation."**
⚠️ **The failure this addresses: [T-0529]'s row claimed a test was "STILL UNEXERCISED" for a day after
the user had exercised it; [T-0521]'s row read "Implemented - Not Verified" while the code was
untouched.** ⚠️ **BOTH were caught by the user, not by me.**
✅ **RULING: a status line is written when the investigation CONCLUDES, not while it runs.**
⚠️ **An in-flight finding may be recorded as narrative; ⛔ it must NOT be written into a status cell
that a later reader will treat as current.**

### R-17 — S-3 ✅ **THE STATE OVERVIEW IS SURFACED EVERY SUBSTANTIVE TURN — WITHOUT CLAUDE READING IT**
✅ **The user ruled option 3, with a constraint that changes the mechanism:**
> ⚠️ ***"But we should ensure that its output is surfaced by Claude without processing tokens. That way
> it will have minimal impact on usage quotas. Claude can then state what was done normally."***

✅ **RULING:** ✅ **(1) `scripts/work-state.sh` output is surfaced to the USER directly;** ⛔ **(2) Claude
does NOT read it back into context and does NOT re-narrate it** — ⚠️ **that is the token cost the user
is avoiding;** ✅ **(3) Claude then reports what was done, normally, on top of it.**
✅ **RESOLVED 2026-09-15: `/state` (`.claude/commands/state.md`) IS the mechanism, and NO automatic
trigger is needed** — ⚠️ **the user clarified the intent was token cost AT SPRINT CLOSE, not per-turn
surfacing.** ⚠️ **Original note follows.** ⚠️ **IMPLEMENTATION IS NOT RULED HERE** — ✅ **a hook, a `/`-command, or a terminal alias are all
candidates; ⛔ the mechanism must genuinely avoid round-tripping the output through Claude's context,
or the ruling is defeated while appearing satisfied.**

### R-18 — S-4 ✅ **OK, no action**
✅ **Recurrence tracking works and the findings file is kept, as the guidelines require.**

---

## ⚠️ CORRECTION — the two "order dependencies" below were WRONG

⚠️ **I asserted two ordering constraints in the Phase 3 preconditions.** ✅ **THE USER CORRECTED BOTH,
AND THE CORRECTION IS RIGHT:**

⛔ **"R-01 before R-02" — FALLACY.** ⚠️ **I claimed the pointer needs a complete table to point at.**
✅ **But extending the table and deleting the Statistics figures are INDEPENDENT EDITS — nothing is
computed across them, and no statistical determination happens between them.** ✅ **Either order.**

⛔ **"R-07 reference check before the move" — NOT A DEPENDENCY.** ✅ **EP-039's record moves as a FILE;
[I-0206] and [I-0213] are rows in EP-040's OWN Issues table and do not reference EP-039's location.**
✅ **The move is safe regardless of state or location.**

✅ **RECORDED because inventing a dependency is its own defect** — ⚠️ **it makes remediation look
fragile and constrains work that is not actually constrained.**

## Phase 3 preconditions

⚠️ **Remediation does NOT begin until the user approves it.** ✅ **That approval verifies this rulings
file** (§"Verification is carried by the PHASE GATES").

✅ **Remediation produces `Audit-Remediation-20260915.md`, citing ruling numbers.**
⛔ **NO ORDER DEPENDENCIES.** ⚠️ **The two I asserted were wrong and are corrected above.**
✅ **All rulings may be executed in any order.**
