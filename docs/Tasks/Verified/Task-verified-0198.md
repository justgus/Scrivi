## T-0198: Undo/redo + copy buffers design doc & trade studies

**Status:** ✅ Implemented - Verified
**Component:** `docs/Scrivi_UndoRedo_History_and_Copy_Buffers_Design_v0_1.md`
**Priority:** High
**Date Requested:** 2026-07-06
**Date Implemented:** 2026-07-06
**Date Verified:** 2026-07-06 (user design sign-off)
**Sprint Assigned:** SP-051
**Epic:** EP-019

**Rationale:**
I-0019 (⌘Z/⇧⌘Z dead in the manuscript editor) has a confirmed root cause — the editor's
authoritative state lives in `ViewportSceneLoader.segments[].text` and the storage is rebuilt around
`NSTextView`, so `NSUndoManager` never accumulates actions. Per user direction, native undo is
replaced by a from-scratch system: sentence-granular events, tree-structured per-project persistent
history (branching, sessions, capacity/purge), and multiple copy buffers. Design must precede
implementation (docs are the source of truth).

**Requirements:**
1. Full design document in house format covering event model, tree model, persistence, C ABI
   boundary, Swift capture/apply split, copy buffers, risks, and EP-019 acceptance criteria.
2. Six embedded trade studies (house format: options, criteria, scoring, decision).
3. Design conforms to the approved architecture (all logic/persistence in ScriviCore; JSON-over-string
   C ABI; Swift UI-only).

**Implementation (2026-07-06):**
`docs/Scrivi_UndoRedo_History_and_Copy_Buffers_Design_v0_1.md` v0.1 — 15 sections + 6 trades.
Key decisions: snapshot-diff computed in C++ (Swift sends whole scene text per commit); undo/redo
return full scene text; manuscript-global tree with scene-scoped events; structural ops are barriers
in v1; append-only JSONL log + atomic checkpoint in `history/` (`scrivi.history.v1`); commit-before-save
invariant. EP-019 scaffold created alongside (SP-051–SP-057, T-0198–T-0217).

**Verification (2026-07-06 — user-approved):**
User reviewed the document and approved it as the baseline, ruling all six trades:
- **T1 = B** (capacity in Project settings / `project.json`)
- **T2 = A with refinements** (fork popover: appears when undo *lands on* a fork; suppressed/dismissed
  when undoing past it; appears immediately on redo reaching a fork; redoing past the choice takes the
  primary; undone chains — including embedded fork points — relegate to a single non-primary branch
  when new typing forks)
- **T3 = C** (copy-into-buffer not a history event; cut-into-buffer is)
- **T4 = B + D, with A for discoverability** (palette + keyboard HUD + Edit-menu items)
- **T5 = C** (session = project-open span with 8 h idle rollover)
- **T6 = A** (history inside the package, gitignored — via the overall design approval)

Refinements incorporated into the doc (§5 branch relegation, §10 T2) the same day; document status
moved Draft → ✅ Approved baseline.

**Notes:**
The T2 refinements flow into T-0211 (fork popover) and the `forkAhead` envelope semantics (design §7).
Design sign-off satisfies the first of SP-051's exit criteria; the sprint's remaining scope is T-0199
(⌘Z-routing spike) and T-0200 (schema spec + repair-matrix row).
