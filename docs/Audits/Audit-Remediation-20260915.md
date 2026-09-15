# Documentation Audit — Remediation — 2026-09-15

**Rulings file:** [`Audit-Rulings-20260915.md`](Audit-Rulings-20260915.md) — ✅ **verified by the user's
approval to begin remediation** (`Audit-Guidelines.md` §"Verification is carried by the PHASE GATES").
**Executed by:** Claude, 2026-09-15, from the approved rulings. **Baseline:** `60443bd`.
**Status:** ✅ **COMPLETE — all 18 rulings executed.**

---

## ⚠️ ONE FINDING WAS PARTLY WRONG AND IS CORRECTED HERE

⚠️ **F-01 claimed *"the All-Sprints table stops at SP-115; SP-116–SP-134 have no row."***
⛔ **THAT WAS OVERSTATED.** ✅ **`Sprint-Documentation.md` has TWO sprint tables with different column
counts; I checked only the first.** ✅ **SP-116–SP-126 and SP-128–SP-130 DO have rows in the second.**

✅ **THE REAL GAP WAS FOUR SPRINTS: SP-127, SP-131, SP-132, SP-133** (⚠️ **SP-134 is not yet issued**).
⚠️ **SP-127 appeared ONLY in the stale Statistics line, never as a row, despite having a closed archive.**

✅ **[R-01] was executed against the CORRECTED fact, not the finding as written.** ⚠️ **Recorded because
remediating an overstated finding would have been a second defect.**

---

## Execution log

| Ruling | Action | Result |
| ------ | ------ | ------ |
| **R-01** | Append missing rows to the All-Sprints table | ✅ **4 rows** (SP-127, 131, 132, 133), each linking its closed archive |
| **R-02** | Remove Sprint Statistics figures | ✅ **"Next available: SP-127"** and **"Total: 130"** deleted; replaced with how to derive them |
| **R-03** | Remove the Task total | ✅ **"425 — T-0001 … T-0426"** deleted |
| **R-04** | Account for 5 missing Issues; ⛔ do NOT rename | ✅ **I-0181, I-0187, I-0188, I-0189, I-0190** added to the Issue accounting table; ✅ **filename left intact with a note saying why** |
| **R-05** | Account for T-0138; count 1 → 2 | ✅ **row added**; ✅ **"Never assigned" corrected to 2** |
| **R-06** | Correct EP-038's row; re-check siblings | ✅ **row now links `Closed/Epic-EP-038.md`**; ✅ **EP-035/036/037 CHECKED — all three correct** |
| **R-07** | Move EP-039 to `Epics/Closed/` | ✅ **moved**; ✅ **[I-0206]/[I-0213] verified present in EP-040's own table BEFORE the move** |
| **R-08** | Delete the stacked counts | ✅ **6 lines deleted** (none was correct; actual 25) |
| **R-09** | Rewrite the known-gap note | ✅ **rewritten: gap CLOSED, 139 not 133, and HOW it recurred** |
| **R-10** | Fix Statistics only | ✅ **done via R-02**; ✅ **9 changelog instances left as dated history** |
| **R-11** | Remove per-status totals **+ backfill 139** | ✅ **3 count rows removed**; ✅ **139 tasks backfilled into 13 Epic-batched files** |
| **R-12** | G-1 — table must reach highest ID | ✅ **added to Sprint + Epic guidelines** |
| **R-13** | G-2 — ID check must know both formats | ✅ **added to `Audit-Guidelines.md`** |
| **R-14** | G-3 — remove prior STATE text | ✅ **added to ALL FOUR guidelines**; ✅ **applied in R-08** |
| **R-15** | S-1 — no restated counts, all layers | ✅ **added to ALL FOUR guidelines** |
| **R-16** | S-2 — no mid-investigation status | ✅ **added to ALL FOUR guidelines** |
| **R-17** | S-3 — surface state without token cost | ⚠️ **PARTIAL — see below** |
| **R-18** | S-4 — no action | ✅ **n/a** |

---

## R-11 — the backfill, in detail

✅ **139 Tasks, all ✅ Verified, batched by Epic into 13 files** under `Tasks/Verified/`:

| Epic | n | ⚠️ inferred attribution |
| ---- | - | ---------------------- |
| EP-025 `[Linux]` Timeline Panel | 29 | 0 |
| EP-027 `[ScriviCore]` Filesystem-Authoritative Chapter/Scene | 29 | 0 |
| EP-016 Timeline Panel — Full Implementation | 20 | 0 |
| EP-028 `[Cross]` Scene & Chapter Merging | 16 | ⚠️ **8** |
| EP-010 Manuscript Structure Editing | 12 | ⚠️ **9** |
| EP-029 `[Cross]` Cut/Copy/Paste | 8 | 0 |
| EP-031 `[ScriviCore]` Worldbuilding Object Model | 7 | ⚠️ **2** |
| EP-009 Writing Surface & Scene Navigator | 5 | ⚠️ **5** |
| EP-023 `[Linux]` Manuscript Structure Editing | 5 | 0 |
| EP-024, EP-030 Scene Inspector | 3 each | 0 |
| EP-018, EP-019 | 1 each | 0 |

⚠️ **24 rows carry an UNCERTAIN Epic attribution and are marked ⚠️ INFERRED — verify in the tables.**
⛔ **[R-11] prohibits writing an inference as fact; ✅ each such row names the Sprint archive to check.**
⚠️ **EP-009's five (T-0090–T-0094) are the clearest suspects** — ✅ **they concern removing the
ScriviCoreAdapter, attributed to "Writing Surface and Scene Navigator", which reads wrong.**

---

## ✅ R-17 IS COMPLETE — ⚠️ **the "partial" note below is SUPERSEDED**

✅ **CLARIFIED BY THE USER 2026-09-15, after remediation:**
> ⚠️ ***"R-17 automatic trigger is not necessary. I was simply trying to reduce the number of tokens
> expended at each sprint close."***

✅ **THE INTENT WAS TOKEN COST AT SPRINT/EPIC CLOSE — not a per-turn feature.** ✅ **`/state` satisfies
it: the user invokes it before a close, the output renders in their terminal and NEVER enters Claude's
context, and Claude does not spend tokens re-deriving state by piecemeal `grep`.**
⛔ **NO automatic trigger is required.** ⚠️ **Open item 1 below is CLOSED.**

⚠️ **RECORDED because I had over-read the original ruling:** ✅ **"surfaced every substantive turn" was
my framing of the option, and the user's actual concern was narrower and cheaper to satisfy.**

---

## ⚠️ ORIGINAL (superseded) — R-17 IS PARTIAL

✅ **DELIVERED: `/state`** (`.claude/commands/state.md`) runs `scripts/work-state.sh`. ✅ **When the USER
invokes it, the output renders in their terminal and does NOT enter Claude's context** — ✅ **which is
the token constraint the ruling exists to satisfy.**

⛔ **NOT DELIVERED: automatic surfacing every substantive turn.** ⚠️ **The user ruled option 3 —
*"surfaced every turn"* — and a slash-command is USER-INVOKED, not automatic.**
⚠️ **The honest position: a mechanism that fires automatically and keeps output out of Claude's context
requires a harness feature I have NOT verified exists** (a hook whose output goes only to the user).
⛔ **I did not build a workaround that pipes output through Claude, because [R-17] explicitly warns that
would *"defeat the ruling while appearing satisfied."***

✅ **WHAT WORKS TODAY: the user types `/state`.** ⚠️ **WHAT IS OWED: an automatic trigger, if the harness
supports one.** ✅ **Recorded as an open item, not as done.**

---

## ✅ Attribution verification — the 24 inferred Epics (completed 2026-09-15)

⚠️ **[R-11] prohibited writing an inference as fact.** ✅ **The user ruled the inferences be FIXED, so
all 24 were checked by OPENING the seven sprint archives they come from.**

| Sprint | n | Inferred | ✅ **TRUE** | Evidence |
| ------ | - | -------- | ---------- | -------- |
| SP-026 | 5 | EP-009 | ✅ **EP-009** | frontmatter `epic: EP-009` |
| SP-027 | 3 | EP-010 | ✅ **EP-010** | frontmatter `epic: EP-010` |
| SP-028 | 3 | EP-010 | ✅ **EP-010** | frontmatter `epic: EP-010` |
| SP-029 | 3 | EP-010 | ✅ **EP-010** | frontmatter `epic: EP-010` |
| SP-030 | — | EP-010 | ✅ **EP-010** | frontmatter `epic: EP-010` |
| SP-103 | 2 | EP-031 | ✅ **EP-031** | Sprint index row `EP-031 [ScriviCore]` |
| **SP-077** | **8** | ⛔ **EP-028** | ⛔ **NO EPIC** | ⚠️ **archive states `**Epic:** —`; Sprint index agrees** |

✅ **RESULT: 16 of 24 inferences were CORRECT. ⛔ 8 were WRONG** — ⚠️ **all eight from SP-077
(T-0310–T-0317), filed under [EP-028] because the archive MENTIONS EP-028.**
✅ **SP-077 is standalone `[Linux]` app UX that UNBLOCKED EP-028's AC5/AC6 Linux verification** —
⛔ **a DEPENDENCY, not membership.** ✅ **They now live in `Task-verified-backfill-NO-EPIC.md`, which
states why belonging to no Epic is CORRECT there and not a gap.**

⚠️ **WHY THE ORIGINAL METHOD FAILED, recorded so it is not repeated:** ⛔ **my checker looked for a
`**Epic:**` markdown line and fell back to "first `EP-0xx` mention in the text".** ✅ **FIVE of the
seven archives declare the Epic in YAML FRONTMATTER (`epic: EP-0xx`)** — ⚠️ **a format the checker
never looked for.** ✅ **The fallback then guessed from prose, which is how a dependency was read as
membership.**

✅ **AFTER: 139 rows across 14 files; ⛔ ZERO rows marked INFERRED; ✅ 24 marked ✅ VERIFIED with the
evidence that established them.**

---

## Verification

✅ **Re-derived after the edits:**
- ✅ SP-127/131/132/133 each have exactly **1** row in the All-Sprints table
- ✅ `Issue-active.md` — **0** "Currently: N records" lines remain; **25** Issue rows
- ✅ `Epic-active.md` — **1** Epic (EP-040); `Closed/Epic-EP-039.md` exists
- ✅ `I-0206`/`I-0213` — still **2** rows in EP-040's Issues table after the move
- ✅ Task accounting — "Never assigned" now **2**; T-0138 present
- ✅ Backfill — **139** rows across **13** files, totals reconciled against the source CSV

✅ **ALSO VERIFIED (2026-09-15, user-directed): the 24 inferred Epic attributions** — ⚠️ **by opening
the seven sprint archives.** ✅ **16 confirmed, ⛔ 8 corrected.** ⛔ **No row in the backfill is now
attributed by inference.**

---

## Open items after this audit

1. ✅ **CLOSED 2026-09-15 — R-17 needs no automatic trigger.** ⚠️ **The user's concern was token cost
   at sprint close; ✅ `/state` addresses it on demand.**
2. ✅ **CLOSED 2026-09-15 — all 24 inferred Epic attributions VERIFIED against their sprint archives.**
   ⚠️ **16 were right; ⛔ 8 were WRONG.** ✅ **See "Attribution verification" below.**
3. ⚠️ **SP-132's unfinished work** — [T-0521] never started, [T-0519]/[T-0520] reverted. ✅ **Recorded in
   `Task-unverified.md` and in the closed Sprint record;** ⛔ **no Epic owns them.**
