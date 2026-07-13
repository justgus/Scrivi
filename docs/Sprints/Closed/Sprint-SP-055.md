# SP-055: Undo/Redo — branching: tree ops, fork popover, purge

**Status:** ✅ Closed (2026-07-13, user-approved)
**Epic:** EP-019 — Custom Undo/Redo History & Multiple Copy Buffers
**Goal:** Turn the linear history engine into a full tree per the design (§5) with the inline fork popover
(Trade T2 core interaction). **Delivers EP-019 AC4** and the branch-aware clauses of **AC5** deferred from
SP-054 (branch auto-purge on eviction; stale-branch detection + user-confirmed purge).
**Start Date:** 2026-07-10
**End Date:** 2026-07-13
**Depends on:** SP-052/SP-053/SP-054 (✅ closed) — linear `HistoryService` (built branch-ready: nodes carry
`childIDs` + `primaryChildID`), C ABI, Swift capture/apply, on-disk persistence.
**Design:** `docs/Scrivi_UndoRedo_History_and_Copy_Buffers_Design_v0_1.md` §5 (tree/branching, primary
child, sessions/capacity/floor), §10 T2 (fork popover interaction refinements). Architecture pass:
`docs/Scrivi_SP055_Branching_Architecture_Pass_v0_1.md` (approved; capacity default 20,000).

### Assigned Tasks

| ID | Title | Priority | Outcome |
| -- | ----- | -------- | ------- |
| T-0210 | Tree ops: branching, primary-child, `select_branch`, auto-purge on eviction | High | ✅ Verified → `../../Tasks/Verified/Task-verified-0210-0212.md` |
| T-0211 | Inline fork popover (Trade T2 core interaction) | High | ✅ Verified → `../../Tasks/Verified/Task-verified-0210-0212.md` |
| T-0212 | Stale-branch detection + user-confirmed purge | Medium | ✅ Verified → `../../Tasks/Verified/Task-verified-0210-0212.md` |

### What shipped
- **Tree ops** in `HistoryService` — fork-on-record (`createdBranch`), `selectBranch(fork, child)` re-primary
  (+ `scrivi_history_select_branch`), `forkAhead` envelope (`{nodeID, children:[{eventID,preview,timestamp,
  isPrimary}]}`) surfaced on undo/redo when the landed node has ≥2 children, and persist/restore of the saved
  primary from `ctl:setPrimary` records + `state.json.primaryChildren`.
- **Branch-aware eviction (§4.1)** — non-primary subtrees off the root auto-purge, the on-path child promotes,
  its diff folds into the floor, and eviction DEFERS when current == root. Persisted via `ctl:purge` /
  `ctl:evict` records; `applyLoadedEviction` replays them after `finalizeLoad()` so evicted branches do not
  resurrect on reload.
- **Inline fork popover** (`Scrivi/Views/ForkPopover.swift`, NEW) — transient `NSPopover` at the caret listing
  fork children (preview + timestamp), ↑↓/↩/esc keyboard selection, primary preselected + badged; §10 T2
  dismiss rules wired in `ManuscriptTextView.apply(step:)`; select → `selectBranch` then `redo`.
- **Stale-branch detection + purge** — `listStaleBranches(nowIso, staleBranchDays)` (non-primary tips older
  than threshold, excludes the live path, disabled at ≤0), `purgeBranch(...)` (rejects root/unknown/on-path),
  the `_list_stale_branches` / `_purge_branch` ABI, and a Project Settings "Stale after (days)" field +
  "Stale Branches" section (Find → list → destructive Purge confirmation). Fixed a dangling-child bug in
  `applyLoadedEviction` that broke `rebuildHeadCache` after a user purge on reload.

### Exit criteria — all met
- ✅ Undo-type-fork-reselect scenario verifiable end-to-end.
- ✅ An abandoned branch is fully restorable and re-primaried.
- ✅ Auto-purge on eviction covered by unit tests.
- ✅ Stale-branch purge works with user confirmation.

### Verification
- **Backend suite 264/264** — 10 `[History][branch]` / `[History][branch][stale]` unit cases + 3
  `[HistoryCApi][branch]` relaunch round-trips (re-selected branch survives reopen; evicted branch does not
  resurrect; purged branch does not resurrect, on-path/root rejected).
- **macOS BUILD SUCCEEDED** (ABI changes backward-compatible — new optional ctl records + endpoints only).
- **User manual pass 2026-07-13** — (1) undo onto a fork → popover → pick branch → redo walks it; (2) low
  stale threshold → Find → Purge → confirm → reopen, branch stays gone. Both passed.

*Closed 2026-07-13 on user approval. Next in EP-019: SP-056 (multiple copy buffers).*
