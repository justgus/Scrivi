# Audit Check — 2026-10-04, before the [EP-049] close

⚠️ **THIS IS AN AUDIT CHECK, NOT AN AUDIT.** ✅ A read-only, mechanical sweep (greps and counts) run before an
Epic close, per `Audit-Guidelines.md` §"The Audit Check". ⛔ **It changes nothing.** ⚠️ Its findings are ruled as
part of the Epic close.

**Run by:** Claude, at the user's request 2026-10-04 (*"close SP-160, run the Audit Check, and close EP-049"*).
**Scope:** the seven checks, over EP-049's records: SP-160, T-0586, I-0276, and every file that states EP-049's
status. (EP-045's check, earlier the same day, is `Audit-Check-20261004.md`.)

---

## ✅ Checks that passed

| # | Check | Result |
| - | ----- | ------ |
| **2** | **Evidence exists** | ✅ T-0586 has ONE archive heading (`Task-verified-0586.md`); ✅ I-0276 has its row and section in `Issue-verified-0271-0280.md`. Neither remains in an active or backlog file. |
| **3** | **Sprint status agreement** | ✅ SP-160 reads **CLOSED** in `Sprint-Documentation.md`, `Sprint-active.md` and its `Closed/` frontmatter. |
| **4** | **Counts re-derived** | ✅ No restated counts in the files touched (the `Issue-backlog.md` count line was removed by EP-045's F-5). |
| **5** | **Table/entry parity** | ✅ `Issue-verified-0271-0280.md`: index says 4, table has 4 rows. |
| **7** | **ID continuity** | ✅ Highest issued = `next-ids.json` − 1 for every kind (EP-049, SP-160, T-0586, I-0276); ✅ EP-050, SP-161, T-0587, I-0277 appear nowhere. |

---

## ⚠️ Findings

### ⚠️ F-1 — `Epic-Documentation.md:115` says *"SP-160 active"* (Check 1)

✅ SP-160 closed today. ⚠️ The index row disagrees with the Sprint layer. ✅ Mechanical: the close rewrites this row.

### ⚠️ F-2 — EP-049's record states no per-AC status, and omits I-0276 (Check 1)

The Epic's AC table lists AC1–AC8 + AC4b + AC-live with no met/unmet state. ✅ The state lives only in SP-160's
record (all met). ⚠️ **[I-0276]** was added to SP-160 under EP-049 by the user's standing rule, but the Epic record
never mentions it. ✅ Mechanical: the close records both.

### ℹ️ F-3 — per-Sprint working files in `docs/Sprints/` are not named by the guidelines (Check 6, informational)

`Sprint-GUIDELINES.md` names four files plus `Closed/`, and allows transient *planning drafts*. ✅ `Sprint-SP-159.md`
(🔵 Planning) qualifies as one. ⚠️ **But since SP-153 every Sprint has lived as `docs/Sprints/Sprint-SP-XXX.md` while
ACTIVE** (SP-153–SP-160), with `Sprint-active.md` carrying a pointer. That practice is not named. ⚠️ **The same shape
as EP-045's F-6** (per-Epic record files), which was ruled: name it in the guidelines.

---

## Recommendation

✅ **No full Audit is warranted.**

| # | Recommendation |
| - | -------------- |
| F-1, F-2 | Correct them in the close step (the close's own edits) |
| F-3 | ⚠️ **Needs a ruling.** Amend `Sprint-GUIDELINES.md` to name the optional per-Sprint record file and require it move to `Closed/` at the close, as EP-045's F-6 did for Epics — or rule the practice out |

---

## ✅ Remediation — 2026-10-05, in the close step

✅ User: *"close SP-160, run the Audit Check, and close EP-049"*

| # | Done |
| - | ---- |
| **F-1** | ✅ The EP-049 index row now reads CLOSED (SP-160 closed) |
| **F-2** | ✅ The closed record (`Closed/Epic-EP-049.md`) carries "AC status at close" and [I-0276] |
| **F-3** | ✅ **RULED 2026-10-05** (user: *"F-3: amend the guidelines as recommended"*) → `Sprint-GUIDELINES.md` now names the optional `Sprint-SP-XXX.md` record of a Planning/Active Sprint and requires it move to `Closed/` at the close |

