# T-0217 — EP-019 Doc Updates + Acceptance-Criteria Verification (SP-094)

**Status:** ✅ **Verified 2026-08-11**
**Epic:** EP-019 — Undo/Redo History & Copy Buffers · ✅ **Closed 2026-08-11**
→ [`Epic-EP-019.md`](../../Epics/Closed/Epic-EP-019.md)
**Sprint:** SP-094 — EP-019 + EP-030 verification & Epic close · **Record of truth:**
[`Sprint-SP-094.md`](../../Sprints/Closed/Sprint-SP-094.md)

> ⚠️ **Originally scoped to SP-057, which was superseded.** SP-057 no longer built a history panel
> (T-0215 → T-0366, EP-030 SP-092) and T-0216 closed OBE, so its remaining verification work merged
> into **SP-094**, jointly with EP-030's T-0369. Both Epics' ACs were verified in one pass and then
> **closed independently**, each on its own direct user approval.

---

## What was delivered

### Design-doc amendments (landed 2026-08-11, ahead of SP-094 activation)

Deliberately landed *before* the AC2 live verify, so it ran against the amended wording rather than a
superseded list.

- **§4.a** — trigger 6 (auto-save flush) retired; trigger 7 = the ≥ 45 s idle boundary. New
  **§4.a.1** explains the save/idle split, backspace-does-not-commit, and why record-and-reopen was
  not implementable.
- **§4.d** — rewritten as a **relaxed** invariant, with the changed-invariant warning, its bound, and
  the failure mode. **§12.2** resolved; **§15 AC2** and the doc header amended. `Epic-active.md`
  **AC2** was rewritten (removing its stale "the §4.d disk invariant is preserved" claim) and flagged
  so that approving EP-019's close was **explicitly an approval of the relaxed §4.d**.
- **§12.8 resolved** — the scanner cannot flag `history/` because it only walks `manuscript/`; no
  ignore-set entry exists or is needed (with a caveat if that scope ever widens). §14 table corrected.

### Package-structure + repair-matrix updates (2026-08-11, design change user-approved same day)

- **`Scrivi_Project_Package_Structure_v0_1.md` §16a** — reconciled with the shipped implementation:
  checkpoint cadence (close *and* every 200 records), the **§4.d relaxation** stated as a
  package-level property (scene files may lead history by one open session; prose is still canonical
  and complete — it is *history* that trails), lazy creation, and the pre-EP-019 case retired per
  T-0216.
- **`Scrivi_External_Change_Repair_Matrix_v0_2.md` §6.21** — head-hash mismatch causes restated: a
  **hard crash mid-session is now an expected cause**, so a mismatch is **not by itself evidence of
  external editing** and the notice must not claim it is. New replayed-purge sub-condition (I-0110);
  torn-line behaviour corrected to the *first* unparseable line; I-0104's false-positive history
  recorded as a caution for anyone touching the checkpoint.

### Verification

EP-019 AC1–AC8 verification pass, run in SP-094 and completed 2026-08-11. EP-019 closed the same day
on user approval.

---

## Found while documenting — T-0400 raised, not folded in

⚠️ **History log-segment rotation is specified but not implemented.** `activeSegment_` is hard-fixed
to `log-000001.jsonl` (`HistoryStore.hpp:114`); capacity/eviction bounds the **tree**, not the
**log**, so a segment grows unbounded (a real project was already ~3.4 MB). The reader already
honours `activeLogSegment` from `state.json`, so rotation is **additive and needs no format change**.
Documented as not-implemented in §16a rather than papered over, and **deliberately raised as its own
task (T-0400) rather than folded into a verification sprint.** T-0400 remains 🔵 Backlog — 🟢 *nice to
have*, not a blocker (user ruling 2026-08-11).

---

⚠️ *Archived 2026-08-18 during the tracking-documentation audit. `Task-backlog.md` had carried this
row as 🔵 **Backlog under the superseded SP-057** — i.e. as unstarted work — despite its having been
completed in SP-094 and closed with EP-019 on 2026-08-11. A related claim in `Task-active.md` that
T-0217 was "still owed before EP-019 closes" was corrected 2026-08-16.*
