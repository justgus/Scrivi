# Audit Check — 2026-10-04, before the [EP-045] close

⚠️ **THIS IS AN AUDIT CHECK, NOT AN AUDIT.** ✅ A read-only, mechanical sweep (greps and counts) run before an
Epic close, per `Audit-Guidelines.md` §"The Audit Check". ⛔ **It changes nothing.** ⚠️ Its findings are **ruled as
part of the Epic close**, not in a separate rulings session.

**Run by:** Claude, at the user's request 2026-10-04 (*"you have my approval to … do the Audit Check for EP-045"*).
**Scope:** the seven checks, over EP-045's records: SP-153–SP-158, T-0554 + T-0577–T-0583, I-0274, I-0275, and the
Epic files that state EP-045's status.

---

## ✅ Checks that passed

| # | Check | Result |
| - | ----- | ------ |
| **2** | **Evidence exists** | ✅ Every Task cited for an AC has exactly ONE archive heading: T-0554 (`0554-0564`), T-0577, T-0578, T-0579, T-0580, T-0581 + T-0582 (`0581-0582`), T-0583. ✅ Issues cited (I-0196, I-0206, I-0275) all have an entry. |
| **3** | **Sprint status agreement** | ✅ SP-153–SP-158 read **CLOSED** identically in `Sprint-Documentation.md`, `Sprint-active.md`, each `Closed/` record's frontmatter, and EP-045's Sprint table. |
| **4** | **Counts re-derived** | ✅ `Issue-backlog.md` states 3 and has 3 rows (but see F-5). |
| **5** | **Table/entry parity** | ✅ Each of the six Task archive files' headings equals its index rows (1·1·1·1·2·1). ✅ `Issue-closed-0274.md` has its index row. |
| **7** | **ID continuity** | ✅ Highest issued = `next-ids.json` − 1 for every kind (SP-158, T-0583, I-0275, EP-048). ✅ SP-153–158 all closed; T-0577–T-0583 all archived; I-0274 closed; I-0275 in the backlog. ✅ SP-159, T-0584, I-0276 appear nowhere. |

---

## ⚠️ Findings

### ⚠️ F-1 — `Epic-active.md` still says EP-045 has ten ACs and TWO owed rulings (Check 1)

`:6` — *"AC1–AC10 and TWO owed rulings."* `:22-23` — *"TWO RULINGS OWED: PASTE · EXISTING MANUSCRIPTS."*
✅ The Epic has **eleven** ACs, all met; R1, R2 and R3 are ruled, and so are Q-AC7 and SP-156's Q1–Q3. ⚠️ The
status line at `:5` is correct, so the file disagrees with itself.

### ⚠️ F-2 — `Epic-EP-045.md` carries superseded state in four places (Check 1; R-14)

- `:80` — heading *"TWO RULINGS OWED"* over a table of **three**, all ruled.
- `:139` — *"Later Sprints (AC3–AC8, AC10) are NOT pre-allocated"*: every one now exists and is closed.
- **Known trap #1** — *"AC2's CAUSE IS UNKNOWN AND TWO FIXES HAVE ALREADY FAILED"*, stated as current; [T-0554]
  found and fixed it (the record already says the trap is "OUT OF DATE", elsewhere).
- **"AC status at activation (2026-10-03)"** — restates superseded state (*"AC2 — LARGELY MET"*, a struck *"AC1 —
  STILL OPEN"*) beside the current per-AC lines.

### ⚠️ F-3 — `Epic-Documentation.md:124-128` restates a next-available ID and a stale EP-045 summary (Checks 1, 4)

*"Next available Epic ID: EP-049"*: correct today, ⛔ but the user ruled on 2026-09-24 that next-available IDs leave the
tracking documents (they live in `docs/tools/next-ids.json`). ⚠️ The same paragraph says EP-045 is *"🔵 Draft, no
Sprint … TWO owed rulings."* The EP-045 table row (`:111`) is correct, so the index disagrees with itself.

### ℹ️ F-4 — AC11 exists only in the Epic, not in the E1 design doc (Check 1, informational)

`Scrivi_Manuscript_Renderer_E1_Design_v0_1.md`'s AC table ends at AC10; AC11 (added 2026-10-04) is in
`Epic-EP-045.md` alone. ⚠️ `CLAUDE.md` makes the design docs the source of truth, so the two should agree.

### ⚠️ F-5 — `Issue-backlog.md:5` restates a count (Check 4; R-15)

*"Currently: 3"* is correct today, ⛔ but R-15 bans restated summary counts in every layer. ⚠️ **I edited that line
three times this session (I-0274 filed, I-0274 closed, I-0275 filed)** and kept the pattern instead of removing it.

### ℹ️ F-6 — per-Epic record files in `docs/Epics/` are not named by the guidelines (Check 6, informational)

`Epic-EP-044.md` and `Epic-EP-045.md` sit in `docs/Epics/`; `Epic-GUIDELINES.md` names only `Closed/Epic-EP-XXX.md`.
✅ It is an established practice (EP-043's record lived there until its close). EP-045's file moves to `Closed/` at the
close; EP-044's stays.

### ℹ️ O-1 — outside EP-045: `Sprint-Documentation.md:232` (informational)

*"Next available ID: **SP-119**"*. ⛔ Stale (SP-158 is issued) and of the kind the 2026-09-24 ruling removed. Not
EP-045's; recorded so it is not lost.

---

## Recommendation

✅ **No full Audit is warranted.** Every finding is small and mechanical. F-1 to F-3 are exactly the edits an Epic close
makes anyway. ⚠️ Rulings owed, as part of the close:

| # | Recommendation |
| - | -------------- |
| F-1, F-2, F-3 | Correct them in the close step: delete superseded state (R-14) and the next-ID line (2026-09-24 ruling) |
| F-4 | Add an AC11 row to the design doc's AC table |
| F-5 | Delete the count line; the table is the count (R-15) |
| F-6 | Amend `Epic-GUIDELINES.md` to name the in-progress per-Epic record file, or rule it out |
| O-1 | Delete the line (2026-09-24 ruling) |

---

## ✅ Rulings and remediation — 2026-10-04 (user-approved)

✅ User: *"fix all findingd as per your reccomendations. Close the Epic."* → every finding ruled AS RECOMMENDED.

| # | Remediation |
| - | ----------- |
| **F-1** | ✅ `Epic-active.md`'s EP-045 block replaced by a CLOSED pointer (the EP-043 precedent) — the stale "ten ACs / TWO owed rulings" lines are gone with it |
| **F-2** | ✅ In the record (now `Closed/Epic-EP-045.md`): rulings heading → "R1, R2, R3: all RULED"; "Later Sprints … not pre-allocated" deleted; trap #1 → resolved by T-0554, lesson kept; "AC status at activation" → "AC status at close", superseded lines deleted |
| **F-3** | ✅ `Epic-Documentation.md`: the next-ID line and the stale EP-045/EP-043 "Draft, no Sprint" summary replaced by current state + a pointer to `next-ids.json` |
| **F-4** | ✅ AC11 row added to the E1 design's AC table |
| **F-5** | ✅ `Issue-backlog.md`'s "Currently: N" line deleted (R-15) |
| **F-6** | ✅ `Epic-GUIDELINES.md` now names the optional in-progress `Epic-EP-XXX.md` and requires it move to `Closed/` at the close |
| **O-1** | ✅ `Sprint-Documentation.md` "Next available ID: SP-119" removed |

✅ Also in the close step (state the close itself changed): EP-046 marked unblocked and EP-048 re-pointed to EP-046,
in `Epic-Documentation.md` and `Epic-backlog.md`.

