## T-0200: `scrivi.history.v1` / `scrivi.buffers.v1` schema spec + repair-matrix row

**Status:** ✅ Implemented - Verified
**Component:** `docs/Scrivi_UndoRedo_History_and_Copy_Buffers_Design_v0_1.md` (Appendix A), `docs/Scrivi_External_Change_Repair_Matrix_v0_2.md` (§6.21), `docs/Scrivi_Project_Package_Structure_v0_1.md` (§4, §16a, §17)
**Priority:** Medium
**Date Requested:** 2026-07-06
**Date Implemented:** 2026-07-06
**Date Verified:** 2026-07-06 (user-approved at SP-051 close)
**Sprint Assigned:** SP-051
**Epic:** EP-019

**Rationale:**
The EP-019 design (§6) specified the history persistence model at design level; implementation
(SP-052/SP-054) needs a field-level schema contract, and the two companion baseline docs (package
structure, repair matrix) had to be reconciled before any code writes into `.scrivi` packages.

**Implementation (2026-07-06):**
1. **Design doc Appendix A** — authoritative field spec:
   - `history/log-NNNNNN.jsonl`: three record kinds discriminated by `rec` with a global monotonic
     `seq` — `floor` (scene text baselines: seed/compaction/eviction, so log replay never reads
     possibly-changed scene files), `event` (§4.b node, flattened; `barrier` variant), and `ctl`
     (undo/redo/setPrimary/purge/evict/session pointer ops — the log alone reconstructs the tree).
   - `history/state.json` checkpoint: settings mirror, `rootID`/`currentNodeID`/`sessionID`,
     `lastSeq`, `activeLogSegment`, forks-only `primaryChildren` map, `sceneHeads` hashes
     (validated at `scrivi_history_open`).
   - `history/buffers.json` (`scrivi.buffers.v1`): `bufferID` `"1"`–`"9"` (string, named registers
     later), reserved `label`, omitted-when-empty slots.
   - `project.json` gains optional `historySettings` `{capacityEvents, staleBranchDays,
     idleRolloverHours}` (Trade T1; defaults when absent).
   - House conventions throughout: camelCase, `*ID` keys (never `id`), ISO-8601, `<kind>_<uuid>`.
2. **Repair matrix §6.21** — new `history issue` condition: per-sub-condition behavior (absent →
   init silently; torn line → truncate; bad checkpoint → replay; bad log → reset + warn; head-hash
   mismatch → `externalChange` barrier; bad buffers → reset + warn). Never Blocking; never touches
   manuscript/metadata files.
3. **Package structure doc** — `history/` added to the §4 layout; new §16a (app-managed derived
   state, travels with the project per Trade T6, safe to delete, gitignored); §17 `.gitignore`
   default gains `history/` with the T-0216 migration note; header Revised line added.

**Verification (2026-07-06 — user-approved):** User reviewed Appendix A and the two companion-doc
changes and verified at SP-051 close.

**Notes:**
Consumed by T-0201/T-0202 (engine + ABI), T-0207 (log/checkpoint implementation), T-0208
(settings), T-0213 (buffers), T-0216 (gitignore migration).
