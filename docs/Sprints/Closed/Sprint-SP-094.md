# SP-094: EP-019 + EP-030 verification & Epic close (merged) ✅ CLOSED

**Status:** ✅ **Closed 2026-08-11 (Human-approved)** — verification pass complete.
**Epics:** EP-019 (Undo/Redo History & Copy Buffers) + EP-030 (Scene Inspector Card Framework).
**Tasks:** T-0217 (EP-019 doc updates + AC verification) + T-0369 (EP-030 AC1–AC7).
**Dates:** 2026-08-11 (single day).
**Suites at close:** ctest **413/413** · macOS interop **56/56** (from 53) · **TEST SUCCEEDED**.

Merged from the old SP-093 (EP-030 close) and SP-057 (EP-019 close) — both were one-task, no-build-work
verification passes gated on the same live session and app build. Both Epics' ACs were verified in one pass.

### Outcome

| Epic | Result |
| ---- | ------ |
| **EP-019** | ✅ **AC1–AC8 all Verified.** AC-complete; awaiting close approval. |
| **EP-030** | 🟡 **6 of 7 Verified.** AC4 partial — its AC12 clause was rescoped and implemented as **T-0399**; re-verification pending. |

### EP-019 — the three remaining ACs, all user-verified 2026-08-11

- **AC2** (as amended by T-0217) — all eight checks pass, **including items 5–8** (≥45 s idle boundary,
  close-commit, auto-save does **not** commit, backspace does not split). Those four have **no automated
  coverage** (`HistoryCapture` isn't in the test target), so they rest entirely on this live confirmation.
- **AC7** — structural ops record barriers; undo stops with a clear notice; no text corruption.
- **AC8** — live no-regression sweep: auto-save, scene navigation, structure ops, external-change scan,
  Git snapshots.

### T-0217 — documentation, completed in this sprint

Two design-doc amendments were mandatory before EP-019 could close, one of them a **changed invariant**:

1. **§4.a trigger list** — auto-save flush **retired** as trigger 6; the **≥ 45 s idle-session boundary**
   added as trigger 7; new **§4.a.1** covering the save/idle split, backspace-does-not-commit, and why
   record-and-reopen was not implementable. §15 AC2 and `Epic-active.md` AC2 amended to match.
2. **§4.d relaxed** — rewritten with the superseded strict wording quoted, the new bound (*disk may lead
   history by at most one open session's typing*), and the safety argument. **Explicitly approved by the
   user as a design change on 2026-08-11**, separately from the Epic close.

Downstream docs reconciled with the shipped implementation:

- **`Scrivi_Project_Package_Structure_v0_1.md` §16a** — checkpoint cadence (close *and* every 200 records);
  the §4.d relaxation as a package-level property (the *manuscript* stays canonical and complete — it is
  *history* that can trail); lazy creation; pre-EP-019 case retired per T-0216.
- **`Scrivi_External_Change_Repair_Matrix_v0_2.md` §6.21** — a **hard crash mid-session is now an expected
  cause** of a head-hash mismatch, so a mismatch is **not by itself evidence of external editing** and the
  notice must not claim it is; replayed-purge sub-condition added (I-0110); torn-line behaviour corrected to
  *first* unparseable line; I-0104's false-positive loop recorded as a caution.
- **Design §12.8 resolved** — the scanner cannot flag `history/` because it only walks `manuscript/`
  (`ExternalChangeScanner.cpp:278-290`); no ignore-set entry exists or is needed, with a caveat if that
  scope ever widens.

### EP-030 — AC4's failure-isolation clause, and what it exposed

The user reported AC12 (*one card's failure never blocks the stack*) as **unverifiable live** — correctly:
there is no UI path to make a card fail. Investigating why surfaced a real gap.

- **The per-card half works and predates T-0368** — `CardErrorView` (`WritingToolCards.swift:330-344`) is
  called by the three writing cards on `model.loadError`.
- **The framework half never landed.** `CardBodyBoundary` (`InspectorCardStackView.swift:319-327`) applied a
  frame and **nothing else** — no error handling, no fallback. A card that did not self-report had no
  backstop. The comment above its call site claimed failure isolation as "a FRAMEWORK guarantee (AC12)";
  that comment was **false** and has been corrected.
- **The AC was also unachievable as written.** SwiftUI cannot catch a trapping view body — a card that traps
  terminates the process, and no wrapper can contain it.

**Rescoped (user-approved 2026-08-11) to soft failures** — Doc 2 §7.1 rewritten: a card that fails to load or
produce content renders an inline warning in place of its content while the stack keeps rendering; hard
failures are explicitly out of scope, caught by tests and review rather than absorbed at runtime.

**Implemented as T-0399** in this sprint — see `Task-active.md`.

### ⚠️ Follow-up found while documenting — log-segment rotation is not implemented

`activeSegment_` is hard-fixed to `log-000001.jsonl` (`HistoryStore.hpp:114`); nothing rotates it. Capacity
and eviction bound the **tree**, not the **log**, so a segment grows unbounded — a real project is already at
**~3.4 MB** in one file. The reader already honours `activeLogSegment` from `state.json`, so rotation is
additive and needs no format change. Documented as not-implemented in §16a rather than papered over.
**Not an EP-019 blocker; needs its own task.**

### Process note

SP-092's exit criteria were left unchecked at the time of this sprint even though eight of nine were
satisfied — they were confirmed here via EP-030's ACs and checked off retroactively. **A sprint whose work is
verified through a later sprint's AC pass should still have its own criteria closed out in the same step**,
or the Epic ends up unable to close over a sprint that is actually done.
