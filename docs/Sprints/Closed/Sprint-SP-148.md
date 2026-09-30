---
sprint: SP-148
epic: EP-043
status: Closed
platform: Linux
created: 2026-09-30
activated: 2026-09-30
closed: 2026-09-30
---

# SP-148 — `[Linux]` **Verification** (S4 of [EP-043])

**Status:** ✅ **CLOSED 2026-09-30 (user-approved).** ✅ All five ACs met. ✅ **[EP-043] S4 of 4 COMPLETE.**
**Epic:** ✅ **[EP-043]** → [`../Epics/Epic-EP-043.md`](../../Epics/Closed/Epic-EP-043.md)
**Serves:** ✅ **AC-build · AC-live**, and the sweep of **R1–R8**
**Depends on:** ✅ **[SP-147] CLOSED 2026-09-30** → [`Closed/Sprint-SP-147.md`](Closed/Sprint-SP-147.md)
**Size:** ✅ **LOW** — ⛔ no new features. ⚠️ A defect found here is filed as an Issue, not fixed silently.

---

## ⚠️ What this Sprint is for

✅ **Each earlier Sprint verified its own ACs on the rig.** ⚠️ But [SP-147] changed the window code AFTER
R1–R3, R7 and R8 were verified in [SP-146] — ⛔ so the Epic cannot close on those older passes alone.
✅ This Sprint is ONE end-to-end pass of the FINISHED build, plus the paperwork an Epic close needs.

---

## Acceptance Criteria

- [x] **AC1 — The AC sweep.** ✅ 2026-09-30 — see §AC1. ✅ Every Epic AC (R1–R8, AC-build, AC-live) traced to the closed Sprint record and
      verified Task/Issue that met it — ⚠️ with the evidence CITED, not restated.
- [x] **AC2 — AC-build on the final build.** ✅ 2026-09-30 — ON THE RIG, non-root: `ctest` **645/645**, smokes **25/25**, commit `dde2370`, build 53. ✅ `ctest` + the Linux smokes GREEN **as non-root ON THE RIG**
      (`rig-build.sh --test`), ⚠️ and the build number confirmed (`scrivi_linux --version`).
- [x] **AC3 — AC-live, end to end.** ✅ 2026-09-30 — the user: *"They all pass"* — all seven, incl. #2 (R3's in-flight hold). ✅ The user's pass of R1–R8 on that build (checklist below).
- [x] **AC4 — The Epic's Issues reconciled.** ✅ 2026-09-30 — [I-0176], [I-0177], [I-0178] VERIFIED from the live pass → `../Issues/Verified/Issue-verified-0171-0180.md`. ✅ [I-0176], [I-0177], [I-0178] — the three rig findings this Epic
      exists for — marked from the live pass, ⛔ not from the code.
- [x] **AC5 — The Audit Check** ✅ 2026-09-30 — RUN; 8 findings, ⚠️ AWAITING RULING (see §AC5) (CLAUDE.md: a read-only mechanical sweep before an Epic close). ⚠️ Its findings
      are ruled as part of the close; ⛔ it changes nothing.

---

## ✅ AC1 — THE AC SWEEP (2026-09-30)

| Epic AC | Met by | Evidence | ⚠️ Changed since? |
| ------- | ------ | -------- | ----------------- |
| **R1** two projects at once | [SP-146] AC4 | `Closed/Sprint-SP-146.md` §AC4; rig pass 2026-09-29 | — |
| **R2** own window each | [SP-146] AC3/AC5 ([R-Q3]) | `Closed/Sprint-SP-146.md` §AC3, §AC5 | — |
| **R3** non-reentrant, raises | [SP-146] AC3 | `Closed/Sprint-SP-146.md` §AC3, `ShellController::openEditor` | ⚠️ **YES** — [T-0567] added the in-flight hold (`pendingOpens_`) so a restore still LOADING cannot be opened twice → **re-checked in AC3 #2** |
| **R4** reopen at launch | [SP-147] AC4 / [T-0567] | `Closed/Sprint-SP-147.md` rig pass #1, #5 | — |
| **R5** size/position/maximized/splitters | [SP-147] AC5 / [T-0567] | rig pass #2, #3 | ⚠️ **POSITION ruled out on Wayland** ([I-0264], user ruling) — ✅ size, maximized, splitters restore |
| **R6** the test guard | [SP-147] AC6/AC7 / [T-0566] | rig pass #6; `restore_guard_smoke` proven red | — |
| **R7** quit flushes every project | [SP-146] AC7 | `Closed/Sprint-SP-146.md` §AC7; [I-0257] | ⚠️ **YES** — [SP-147] made quit also RECORD every window → **re-checked in AC3 #3** |
| **R8** `scrivi_close_project` per window | [SP-146] AC8 | `Closed/Sprint-SP-146.md` §AC8 | ⚠️ **YES** — [T-0566] now records state in `closeEvent` BEFORE the release → **re-checked in AC3 #4** |
| **AC-build** | every Sprint + **AC2 here** | each record's AC-build | ⚠️ must be re-run on the FINAL build |
| **AC-live** | **AC3 here** | — | — |

✅ **No Epic AC is unmet on paper.** ⚠️ **Three were touched after their verification (R3, R7, R8)**, which is
exactly why this Sprint re-runs them rather than trusting the older passes.

---

## ✅ AC5 — THE AUDIT CHECK (2026-09-30) — ⛔ read-only; nothing was changed

| # | Check | Finding | Proposed remedy |
| - | ----- | ------- | --------------- |
| **A1** | 2. evidence archived | ⛔ **[I-0257]** — USER-VERIFIED on the rig 2026-09-29, **still a row in `Issue-active.md`, with NO archive entry.** ⚠️ The SAME miss already recorded for [I-0251] at [SP-146]'s close — this one was not caught then | Archive into `Issue-verified-0251-0260.md` |
| **A2** | 1. AC state | ⚠️ `Epic-EP-043.md`: **all 10 ACs still `[ ]`** although every one is met (§AC1, AC2, AC3) | Tick them when EP-043 is marked Complete |
| **A3** | 3. status agreement | ⚠️ `Epic-Documentation.md` EP-043 row still reads *"[SP-147] NEXT"* | Update to S1–S4 complete |
| **A4** | 3. status agreement | ⚠️ `Sprint-Documentation.md` has **no rows for [SP-145], [SP-146], [SP-148]** (SP-147's was added today) | Add the three rows |
| **A5** | 5. table/entry parity | ⚠️ The Verified batch files do NOT follow one shape: `0211-0220`, `0261-0270` and today's additions to `0251-0260` are table-rows only; `0171-0180` is one row + five `##` sections. ⚠️ Includes MY archiving today | Rule the shape; then conform (or record the exception, as batches 2/3 were) |
| **A6** | 4. counts | ⚠️ `Issue-Documentation.md`'s batch table stops at **batch 14 (I-0131–0140)** | Extend it (it is an index; the rows are mechanical) |
| **A7** | 3. status agreement | ⚠️ `Sprint-active.md` still carries the old *"⛔ NO SPRINT IS ACTIVE… [SP-147] IS NEXT"* block beneath a note calling it superseded | Remove the stale block |
| **A8** | 6. orphans | ⚠️ `Sprints/Sprint-SP-130-RESTRUCTURE.md`, `Sprint-SP-130-VERIFICATION.md` — not named by the guidelines | Rule: keep (named exception) or move to `Closed/` |

✅ **7. ID continuity:** every ID issued in this Epic's final stretch and today's SP-151 work (T-0565–T-0570,
I-0257–I-0265) is filed; registry peek `EP-049 · SP-152 · T-0571 · I-0266`.
⚠️ **None of A1–A8 changes an Epic AC's truth** — they are records that disagree with each other.

---

## AC3 — the live checklist (final build)

| # | Epic AC | Check |
| - | ------- | ----- |
| 1 | **R1 · R2** | Open TWO different projects → each in its OWN window |
| 2 | **R3** | Open one of them AGAIN (Landing / recents) → its window is RAISED, ⛔ no second copy |
| 3 | **R7** | Type in BOTH windows, `File ▸ Quit` → relaunch → BOTH edits are there |
| 4 | **R8** | Close one window, reopen that project → it opens normally (no "already open") |
| 5 | **R4** | Quit with both open → relaunch → BOTH come back; one on an unplugged drive is skipped |
| 6 | **R5** | Sizes, maximized, splitters restore PER WINDOW (⚠️ position ruled out on Wayland, [I-0264]) |
| 7 | **R6** | `SCRIVI_NO_RESTORE=1 scrivi_linux` → Landing only; next normal launch restores everything |

---

## Progress log

### ✅ 2026-09-30 — CLOSED (user-approved); Audit Check findings APPLIED

✅ **User ruling: "A1-A8 fix all as proposed. SP-148 close approved and mark the Epic complete."**
| # | Applied |
| - | ------- |
| A1 | [I-0257] archived → `Issue-verified-0251-0260.md` |
| A2 | `Epic-EP-043.md`: all 10 ACs ticked, Epic marked COMPLETE |
| A3 | `Epic-Documentation.md` EP-043 row updated |
| A4 | `Sprint-Documentation.md`: rows added for [SP-145], [SP-146], [SP-148] |
| A5 | 4 batch files brought to row/section parity (P4): `0171-0180` 8/8, `0211-0220` 6/6, `0251-0260` 7/7, `0261-0270` 5/5 — pointer sections, ⛔ no content duplicated |
| A6 | `Issue-Documentation.md` batch table extended 15→27, counts RE-DERIVED. ⚠️ Batch 16 holds **11** IDs in a ten-ID range — recorded, not "corrected" (cf. batches 2/3) |
| A7 | stale *"NO SPRINT IS ACTIVE… [SP-147] IS NEXT"* block removed from `Sprint-active.md` |
| — | ✅ **[EP-043] CLOSED 2026-09-30 (user-approved)** → `../../Epics/Closed/Epic-EP-043.md` |
| A8 | `Sprint-SP-130-RESTRUCTURE.md` / `-VERIFICATION.md` → `Closed/`; 5 links fixed — ⚠️ incl. `Epics/Closed/Epic-EP-040.md`'s, which was ALREADY broken (`../Sprints/…` from `Epics/Closed/`) |


### ✅ 2026-09-30 — AC2, AC3, AC4 MET

✅ **AC2:** `rig-build.sh --no-bump --test` — rig on `dde2370` (= local HEAD), build 53; ⚠️ the commit after
SP-147's close changed DOCS only, so this is the code SP-147 closed on. ✅ `ctest` 645/645, smokes 25/25, NON-ROOT.
⚠️ A first `rig-build.sh --test` REFUSED to run on an uncommitted tree — ✅ by design (the rig lands on a
nameable commit); ⛔ I had told the user it would commit only the stamp and ignore the rest.
✅ **AC3:** the user's live pass — all seven checks pass, including R3 with a recent clicked while a restore
was still loading.
✅ **AC4:** [I-0176], [I-0177], [I-0178] verified and archived.

⚠️ **2026-09-30 — Sprint created and ACTIVATED** (the reserved ID; the rig is available).
