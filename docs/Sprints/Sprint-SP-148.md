---
sprint: SP-148
epic: EP-043
status: Active
platform: Linux
created: 2026-09-30
activated: 2026-09-30
---

# SP-148 — `[Linux]` **Verification** (S4 of [EP-043])

**Status:** 🟡 **ACTIVE — 2026-09-30.** ✅ The rig is up.
**Epic:** ✅ **[EP-043]** → [`../Epics/Epic-EP-043.md`](../Epics/Epic-EP-043.md)
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
- [ ] **AC2 — AC-build on the final build.** ✅ `ctest` + the Linux smokes GREEN **as non-root ON THE RIG**
      (`rig-build.sh --test`), ⚠️ and the build number confirmed (`scrivi_linux --version`).
- [ ] **AC3 — AC-live, end to end.** ✅ The user's pass of R1–R8 on that build (checklist below).
- [ ] **AC4 — The Epic's Issues reconciled.** ✅ [I-0176], [I-0177], [I-0178] — the three rig findings this Epic
      exists for — marked from the live pass, ⛔ not from the code.
- [ ] **AC5 — The Audit Check** (CLAUDE.md: a read-only mechanical sweep before an Epic close). ⚠️ Its findings
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

⚠️ **2026-09-30 — Sprint created and ACTIVATED** (the reserved ID; the rig is available).
