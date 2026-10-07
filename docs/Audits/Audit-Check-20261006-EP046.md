# Audit Check — 2026-10-06, before the [EP-046] close

⚠️ **THIS IS AN AUDIT CHECK, NOT AN AUDIT.** ✅ A read-only, mechanical sweep (greps and counts) run before an
Epic close, per `Audit-Guidelines.md` §"The Audit Check". ⛔ **It changes nothing.** ⚠️ Its findings are **ruled as
part of the Epic close**, not in a separate rulings session.

**Run by:** Claude, at the user's request 2026-10-06 (*"yes, close SP-164 and run the Audit Check."*).
**Scope:** the seven checks, over EP-046's records: SP-159, SP-161–SP-164; T-0584, T-0585, T-0587, T-0588, T-0589,
T-0590–T-0593; I-0277, I-0278, I-0279; the Epic files and the E2 design doc that state EP-046's status.

---

## ✅ Checks that passed

| # | Check | Result |
| - | ----- | ------ |
| **1** | **AC status agreement** | ✅ All twelve ACs are met per `Epic-Documentation.md:112` (S1: AC1, AC2, AC4 line, AC5, AC7, AC8, AC11 · S2: AC3, AC4 span, AC7 inline, AC12 · S3: AC6, AC9, AC12 across scenes · S4: AC10), and each closed Sprint record ticks the same ACs. `Epic-active.md`'s AC table carries no per-AC status to disagree with. (But see F-1–F-4 for stale prose.) |
| **2** | **Evidence exists** | ✅ Every Task cited for an AC has exactly one archive file: T-0584, T-0585, T-0587, T-0588, T-0590, T-0591, T-0592, T-0593 (`Verified/Task-verified-XXXX.md`), none left in `Task-active.md`. ✅ I-0279 has its row in `Issue-verified-0271-0280.md`. |
| **3** | **Sprint status agreement** | ✅ SP-159, SP-161–SP-164 read **CLOSED** in their `Closed/` frontmatter, `Sprint-Documentation.md`, `Sprint-active.md` and EP-046's Sprint table; none is left in `Sprint-backlog.md`; no Sprint record remains outside `Closed/`. |
| **4** | **Counts re-derived** | ✅ `Issue-verified-0271-0280.md`'s "(5, re-counted)" note = 5 table rows (I-0271, I-0272, I-0273, I-0276, I-0279). |
| **5** | **Table/entry parity** | ✅ Each Task archive file has one heading, equal to its index row; `Task-Documentation.md` points each EP-046 Task at its archive file. |
| **6** | **Orphan files** | ✅ None new. `Epic-EP-044.md` is the known in-progress record (ruled 2026-10-04, F-6). |
| **7** | **ID continuity** | ✅ Highest issued = `next-ids.json` − 1 for every kind (EP-049, SP-164, T-0593, I-0279). ✅ EP-046's IDs all accounted for: Sprints closed; Tasks archived, except **T-0589** (🔵 backlog, re-homed to EP-047 at filing); **I-0277** (VoiceOver) and **I-0278** (Project Settings in `UserDefaults`) in the Issue backlog. |

---

## ⚠️ Findings

### ⚠️ F-1 — `Epic-active.md:23-27` states a superseded mechanism as current (Check 1; R-14)

*"So Model B needs STORAGE attributes … or an `NSTextLayoutFragment` subclass."* ⛔ Superseded by Q-E2-6 = route (a′)
(the content-storage delegate presents styled paragraphs; storage stays plain), which the same file states at `:42`.

### ⚠️ F-2 — `Epic-active.md:77` says *"⛔ No Epic is active."* (Check 1)

Under the closed-EP-045 pointer. ⛔ False since 2026-10-05 (EP-046 activated); `:69` of the same file says so.

### ⚠️ F-3 — `Epic-Documentation.md:112` says *"ACs AC1–AC11 written"* (Check 1)

AC12 was added 2026-10-05 ([SP-162]); the same row later says "AC12 met", so the row disagrees with itself.

### ℹ️ F-4 — AC12 exists in the Epic, not in the E2 design doc's AC table (Check 1, informational)

`Scrivi_Manuscript_Renderer_E2_Design_v0_1.md` §11 ends at AC11; AC12 appears only as prose in §3.6. ⚠️ The design docs
are the source of truth (`CLAUDE.md`). Same shape as EP-045's F-4.

### ℹ️ F-5 — the design doc's §3.8 (AS BUILT, E2-S4) predates SP-164's live-pass fixes (informational)

It does not record: Replace All applied in ONE pass with the find bar reporting its count; a multi-scene undo placing the
caret once; the Navigator's FILTER matching presented text (`MarkdownEmphasis.searchable`); and the jump landing the caret
on the match (approved by the user 2026-10-06). All are in `Closed/Sprint-SP-164.md`.

### ℹ️ F-6 — EP-046 has no Epic record file (Check 6, informational)

EP-045 kept `Epic-EP-045.md` and moved it to `Closed/` at its close; EP-046's whole record lives in `Epic-active.md`. The
close creates `Closed/Epic-EP-046.md` from it (the EP-043 precedent) — not a defect.

### ℹ️ O-1 — outside EP-046: `Sprint-Documentation.md:33` (informational)

*"Next available ID is in Statistics"* — the Statistics figures were removed under [R-15]; next IDs live in
`docs/tools/next-ids.json`. Recorded so it is not lost.

---

## Recommendation

✅ **No full Audit is warranted.** Every finding is small and mechanical, and F-1–F-3 are edits the Epic close makes anyway.
Rulings owed, as part of the close:

| # | Recommendation |
| - | -------------- |
| F-1, F-2 | Replace `Epic-active.md`'s EP-046 block with a CLOSED pointer (the record moves to `Closed/Epic-EP-046.md`, F-1's paragraph corrected there to name route (a′)); delete the "No Epic is active" line under EP-045 |
| F-3 | Correct the row to "AC1–AC12" as part of marking EP-046 closed |
| F-4 | Add an AC12 row to the E2 design's §11 table |
| F-5 | Add the live-pass fixes to §3.8 |
| F-6 | Create `Closed/Epic-EP-046.md` at the close |
| O-1 | Point the line at `next-ids.json` |

---

## ✅ Rulings and remediation — 2026-10-06 (user-approved)

✅ User: *"fix the findings and close EP-046"* → every finding ruled AS RECOMMENDED.

| # | Remediation |
| - | ----------- |
| **F-1** | ✅ `Epic-active.md`'s EP-046 block replaced by a CLOSED pointer; in the record (`Closed/Epic-EP-046.md`) the paragraph now names route (a′) as the resolution, the superseded claim kept as a marked note |
| **F-2** | ✅ The "No Epic is active" line under EP-045 deleted; the true statement now sits under EP-046's pointer |
| **F-3** | ✅ `Epic-Documentation.md` EP-046 row → CLOSED, "AC1–AC12", per-Sprint ACs |
| **F-4** | ✅ AC12 row added to the E2 design's §11 table |
| **F-5** | ✅ §3.8 records the live-pass fixes (one-pass Replace All with its count, single caret placement on multi-scene undo, presented-text Navigator filter, caret on the match) |
| **F-6** | ✅ `Closed/Epic-EP-046.md` created |
| **O-1** | ✅ `Sprint-Documentation.md` now points at `next-ids.json` |

✅ Also in the close step (state the close itself changed): EP-048 marked unblocked in `Epic-Documentation.md` and
`Epic-backlog.md`; EP-046 removed from the index's backlog summaries (with the stale "EP-043 is now ACTIVE" note).
