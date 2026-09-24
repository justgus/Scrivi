# Audit Check — 2026-09-24, before the [EP-040] close

⚠️ **THIS IS AN AUDIT CHECK, NOT AN AUDIT.** ✅ **A read-only, mechanical sweep — greps and counts —
run before an Epic close per `Audit-Guidelines.md` §"The Audit Check".** ⛔ **It changes nothing.**
⚠️ **Its findings are RULED AS PART OF THE EPIC CLOSE, not in a separate rulings session.**

**Run by:** Claude, at the user's request 2026-09-24.
**Scope:** the seven mechanical checks the guidelines name.

---

## ✅ CHECKS THAT PASSED

| # | Check | Result |
| - | ----- | ------ |
| **4** | **Counts re-derived** | ✅ **26 open Issues · 133 closed Sprints · 35 closed Epics on disk**, consistent with the layer records |
| **5** | **Table/entry parity** in Issue archives | ✅ **PASS** — ⚠️ *the early files (0001–0140) use `## I-0NNN` headings rather than table rows; ✅ spot-checked `0001-0010` = 10 entries, so the 0-row grep is a FORMAT difference, NOT data loss* |
| **6** | **Orphan files** | ✅ **NONE** — every file in the four tracking folders is one the guidelines name |
| **7** | **ID continuity** (T-0543 – T-0550) | ✅ **ALL EIGHT ACCOUNTED FOR** and archived under `Verified/` |

---

## ⚠️ FINDINGS

⚠️ **FIVE findings: F-1 – F-3 from the checks themselves; ✅ F-4 informational; ⚠️ F-5 surfaced while remediating F-1 and ruled by the user in the same pass.**

### ⚠️ **F-1 — `Epic-Documentation.md` does not know [SP-150] exists.** (Check 1)

⛔ **Two lines still scope [EP-040] as `SP-134–SP-137`:** `:18` (*"SP-134–SP-137"*) and `:122`
(*"🟡 [EP-040] ... (SP-134–SP-137)"*). ✅ **SP-150 closed 2026-09-24 and is [EP-040]'s FIFTH Sprint.**
⚠️ **`:106` also still shows EP-040 as 🟡 ACTIVE**, ✅ **which is CORRECT until the Epic actually
closes — ⛔ but it must change in the same step as the close.**
⚠️ **THIS IS THE F-23 SHAPE THE GUIDELINES CITE: the index layer disagreeing with `Epic-active.md`
about an Epic being closed.** ✅ **Mechanical, not a judgement call.**

### ⚠️ **F-2 — `Sprint-backlog.md:171` still offers SP-150 as next-available.** (Check 3)

⛔ ***"Next available Sprint ID: SP-150."*** ⚠️ **SP-150 is TAKEN and CLOSED.**
✅ **The user ruled 2026-09-24 that next-available figures leave the tracking documents entirely** —
✅ **they now live in `docs/tools/next-ids.json`** — ⚠️ **so this line should be DELETED, not corrected.**

### ⛔ **F-3 — [I-0203] and [I-0205] are cited as EVIDENCE for MET ACs but are still OPEN.** (Check 2)

⚠️ **THE MOST SUBSTANTIVE FINDING, AND THE ONE THE GUIDELINES EXIST FOR** (⚠️ *it is F-03's shape:
an Issue named as AC evidence with no archive entry*).

| Issue | Cited by | Its own status in `Issue-active.md` | Archive |
| ----- | -------- | ----------------------------------- | ------- |
| **[I-0203]** | ✅ **AC5** — *"MET 2026-09-22, BOTH HALVES checked"* | 🟢 **"RESOLVED - Not Verified (2026-09-22, [SP-135]/T-0545)"** | ⛔ **NONE** |
| **[I-0205]** | ✅ **AC9** — *"MET 2026-09-22"* | 🟢 **"DIAGNOSED AND RULED … ⚠️ AWAITING USER VERIFICATION OF THE FIX"** | ⛔ **NONE** |

⚠️ **`Sprint-active.md` records [SP-135] as *"All EIGHT ACs VERIFIED by the user's live pass"* and its
closed record says *"[I-0203] and [I-0205] both RESOLVED"*** — ⛔ **but NEITHER Issue record was ever
moved to Verified, and [I-0205]'s own text still says it awaits user verification.**

⚠️ **SO THE LAYERS DISAGREE, AND ONLY THE USER CAN SETTLE IT** (`feedback_verification`):
- ✅ **IF the SP-135 live pass DID verify them**, ⚠️ **both records are simply stale and should be
  marked Verified and archived to `Issue-verified-0201-0210.md`.**
- ⛔ **IF it did not**, ⚠️ **then AC5 and AC9 rest on unverified fixes**, ✅ **and the Epic close should
  say so plainly rather than inherit the claim.**

⛔ **CLAUDE MAY NOT RULE THIS.** ⚠️ **Marking an Issue Verified requires direct user approval.**

### ✅ **F-4 — [I-0206] is open and correctly so.** (Check 2, informational)

✅ **NOT A DEFECT — recorded so the close does not mistake it for one.** ⚠️ **[I-0206]** (*every
keystroke ~59 ms inside AppKit; `setSelectedRange` linear in document offset*) **is cited by AC11 and
is legitimately OPEN**: ✅ **AC11 took its "ACCEPTED AS A LIMITATION WITH A MEASUREMENT" branch.**
⚠️ **The Epic close must NOT close [I-0206].**

---

## ✅ REMEDIATION — ALL THREE FINDINGS RULED AND APPLIED 2026-09-24

⚠️ **RULED BY THE USER IN ONE PASS, AS THE GUIDELINES INTEND** (⚠️ *an Audit Check's findings are
ruled as part of the Epic close, not in a separate session*).

| Finding | User ruling | Applied |
| ------- | ----------- | ------- |
| **F-1** | ✅ **"fix"** | ✅ **`Epic-Documentation.md:18` and `:122` now read `SP-134–SP-137 · SP-150`** |
| **F-2** | ✅ **"fix"** | ✅ **`Sprint-backlog.md` next-available line DELETED, not corrected** — ⚠️ **per the 2026-09-24 ruling that the figure leaves every tracking document; ✅ the EP-043 reservation and Sprint↔Epic mapping were KEPT** |
| **F-3** | ✅ **"The records are stale. I-203 and I-205 are resolved."** | ✅ **[I-0203] and [I-0205] marked VERIFIED and ARCHIVED** → [`../Issues/Verified/Issue-verified-0201-0210.md`](../Issues/Verified/Issue-verified-0201-0210.md) |
| **F-5** | ✅ **"fix the SP-130 row too"** | ✅ **FOUR stale rows corrected** (SP-130 · SP-141 · SP-142 · SP-143) **and their broken links repaired**; ⚠️ **SIX MISSING rows added** (SP-134–SP-137 · SP-149 · SP-150). ✅ **Coverage validated by script** |

⚠️ **F-3 IS THE ONE WORTH REMEMBERING.** ✅ **[SP-135]'s live pass DID verify both Issues on
2026-09-22 and its closed record said so** — ⛔ **but neither row was moved out of `Issue-active.md`,
so for two days the Epic layer claimed AC5/AC9 MET while the Issue layer still read
"Resolved - Not Verified" and "AWAITING USER VERIFICATION".** ✅ **Nothing was wrong with the CODE;
the TRACKING had drifted** — ⚠️ **precisely what `feedback_archive_on_close` warns about, and what
this Check exists to catch before a close inherits the claim.**

## ⚠️ **F-5 — FOUR STALE ROWS IN `Sprint-Documentation.md`'s ALL-SPRINTS TABLE.** (Check 3)

⚠️ **FOUND WHILE REMEDIATING F-1, AND THE USER RULED "fix the SP-130 row too" 2026-09-24.**
✅ **Pulling that one thread exposed THREE MORE of the same defect** — ⚠️ **so it was swept
systematically rather than one row at a time:**

| Row | Claimed | Actually | Also |
| --- | ------- | -------- | ---- |
| **SP-130** | 🟡 **ACTIVE 2026-09-15** | ✅ **CLOSED 2026-09-18** | ⛔ **broken link** (`Sprint-SP-130.md`, no `Closed/`) |
| **SP-141** | 🟢 **ACTIVE** | ✅ **CLOSED 2026-09-21** | ⛔ **broken link** |
| **SP-142** | 🔵 **Planned** | ✅ **CLOSED 2026-09-21** | ⛔ **broken link** |
| **SP-143** | 🔵 **Planned** | ✅ **CLOSED 2026-09-22** | ⛔ **broken link** |

✅ **ALL FOUR FIXED: status, close date, and the `Closed/` path.** ⚠️ **Start/End columns filled where
the closed record states them.**
✅ **VALIDATED MECHANICALLY AFTER THE FIX:** ⛔ **zero rows in that table still claim ACTIVE or
Planned**, ✅ **and EVERY `Closed/Sprint-SP-NNN.md` link now resolves to a file on disk.**

⚠️ **AND THE SWEEP FOUND A SIXTH PROBLEM THE FOUR ROWS WERE HIDING: THE TABLE STOPPED AT SP-133.**
⛔ **SIX closed Sprints had NO ROW AT ALL — SP-134, SP-135, SP-136, SP-137, SP-149 and SP-150** —
⚠️ **FIVE of them [EP-040]'s, the Epic being closed in this very pass.** ✅ **All six ADDED, with
close dates read from each closed record rather than inferred.**
✅ **COVERAGE NOW VALIDATED BY SCRIPT: every `Closed/Sprint-SP-NNN.md` on disk from SP-119 onward has
a row.** ⚠️ **That check did not previously exist and is worth repeating at future Epic closes.**

⚠️ **WHY THIS MATTERS BEYOND TIDINESS:** ⚠️ **three of the four were [EP-041]'s Sprints, and [EP-041]
CLOSED 2026-09-22** — ⛔ **so its own close left four rows in the index layer disagreeing with it.**
✅ **Same shape as F-1 and F-3: a close that updated the layer it lived in and not the index above it**
(`feedback_archive_on_close`).

## ⚠️ WHAT THIS CHECK DID NOT DO

⛔ **It did not read for judgement, correctness, or drift of MEANING** — ✅ **that is an Audit, and an
Audit begins only when the user requests one.**
⛔ **It did not fix anything.** ✅ **F-1 and F-2 are mechanical and can be remediated as part of the
close; ⚠️ F-3 needs a user ruling first.**
