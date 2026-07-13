# T-0210 / T-0211 / T-0212 — ✅ Verified (2026-07-13)

**Epic:** EP-019 · **Sprint:** SP-055 · Delivers **AC4** (undo-then-type branching; abandoned branch
restorable + re-primaried) and **AC5**'s branch-aware clauses deferred from SP-054 (branch auto-purge
on eviction; stale-branch detection + user-confirmed purge).

**Status:** ✅ Verified (2026-07-13, user-approved). Implemented 2026-07-10; backend machine-verified
264/264 ctest at implementation time; macOS app builds.

**Design:** `docs/Scrivi_UndoRedo_History_and_Copy_Buffers_Design_v0_1.md` §5 (tree/branching, primary
child, sessions/capacity/floor), §10 T2 (fork popover interaction refinements). Architecture pass:
`docs/Scrivi_SP055_Branching_Architecture_Pass_v0_1.md` (approved; capacity default 20,000).

---

## T-0210 — Tree ops: branching, primary-child, `select_branch`, auto-purge on eviction
- **D1 `createdBranch`** — `record()` sets it true when the current node already had a child (fork).
- **D2 `selectBranch(fork, child)`** — `HistoryService::selectBranch` + `scrivi_history_select_branch`
  ABI; re-primaries a fork without moving the pointer; rejects a non-child.
- **D3 `forkAhead`** — `HistoryService::forkAheadAt` populates `StepResult.forkAhead` on undo/redo when
  the landed node has ≥2 children; ABI serializes
  `{nodeID, children:[{eventID,preview,timestamp,isPrimary}]}` (the T-0211 contract). Preview trimmed on
  a scalar boundary.
- **D4 persist/restore primary** — `finalizeLoad` keeps "last child wins" as default;
  `applyPrimaryOverride` restores the saved primary from `ctl:setPrimary` log records **and**
  `state.json.primaryChildren`. `HistoryStore::persistSetPrimary`; `JsonDoc::objectKeys()` added.
- **D5 branch-aware eviction (§4.1)** — in-memory auto-purge (`evictToCapacity` + `rootChildTowardCurrent`
  + `eraseSubtree`) purges non-primary subtrees off the root, promotes the on-path child, folds its diff
  into the floor, and DEFERS when current == root. **Persisted:** `evictToCapacity` reports an
  `EvictionDetail` on `RecordResult`; the ABI writes `ctl:purge` / `ctl:evict` records after any
  evicting record; replay collects them and `HistoryService::applyLoadedEviction` drops purged subtrees +
  advances the root AFTER `finalizeLoad()`, so evicted branches do **not** resurrect from the log on
  reload (`HistoryStore.persistEviction`).

## T-0211 — Inline fork popover (Trade T2 core interaction)
Inline transient popover at the caret listing fork children (preview + timestamp), keyboard selection
(Trade T2 option A), driven by the `forkAhead` envelope field. Approved T2 refinements (design §10 T2):
appears when undo *lands on* a fork; dismisses (never obstructs) when the writer undoes past it; appears
immediately when redo reaches a fork; redoing without selecting follows the primary child.
- **Engine (`ScriviEngine.swift`):** `HistoryStepResult` decodes `forkAhead`; new
  `HistoryForkAhead`/`HistoryForkChild` (eventID, preview, timestamp, isPrimary); new
  `historySelectBranch(...)` over `scrivi_history_select_branch` + `HistorySelectBranchResult`; `#else`
  unavailable stub.
- **Capture (`HistoryCapture.swift`):** `selectBranch(forkNodeID:childEventID:)` passthrough that refreshes
  `engineCanRedo`.
- **UI (`Scrivi/Views/ForkPopover.swift`, NEW):** `ForkPopoverController` owns a transient `NSPopover`
  anchored at the caret rect (`firstRect(forCharacterRange:)`); SwiftUI content lists branches with ↑↓
  highlight, ↩ select, esc dismiss, primary preselected + badged.
- **Wiring (`ManuscriptTextView.swift`):** `apply(step:)` shows the popover on `forkAhead`, closes it when a
  step lands off any fork; a genuine keystroke (`textDidChange`, non-apply) also closes it (§10 T2 "never
  obstructs"); select → `selectBranch` then `redo` (nested-fork-safe via `apply`).
- **pbxproj:** `ForkPopover.swift` registered in all three targets (C061/D061/E061).

## T-0212 — Stale-branch detection + user-confirmed purge
Detection thresholds + user-confirmed purge flow (completes AC5's branch clauses). The
`_list_stale_branches` / `_purge_branch` ABI (stubbed-in-comment before this task) got real
implementations.
- **Engine (`HistoryService`):** `listStaleBranches(nowIso, staleBranchDays)` — every non-primary subtree
  whose newest node (tip) is older than the threshold; excludes the root→current (live) path;
  `staleBranchDays <= 0` disables. `purgeBranch(branchRootEventID)` — detaches from parent + erases subtree
  via existing `eraseSubtree`; **rejects** the root, an unknown node, or any on-path node. New
  `StaleBranch`/`PurgeResult`, `subtreeNodeCount`, ISO-8601 UTC parser (`parseIso8601Utc`).
- **Persistence:** `HistoryStore::persistPurge` writes the same `ctl:purge` record eviction uses, so replay
  drops the subtree with no new replay logic. **Bug found + fixed:** `applyLoadedEviction` erased purged
  subtrees but left a dangling child in the parent's `childIDs`; harmless for eviction (parent was the
  erased old root) but a user purge keeps the parent → `rebuildHeadCache` threw "unknown node" on reload.
  Now detaches from the parent (and repoints its primary) before erasing.
- **ABI:** `scrivi_history_list_stale_branches` → `{staleBranchDays, branches:[{branchRootEventID,
  forkNodeID, preview, tipTimestamp, nodeCount}]}`; `scrivi_history_purge_branch` → `{ok, branchRootEventID,
  purgedCount, canUndo, canRedo}`.
- **Swift:** `historyListStaleBranches` / `historyPurgeBranch` bindings + `HistoryStaleBranch` /
  `HistoryStaleBranchesResult` / `HistoryPurgeResult` + `#else` stubs; `HistoryCapture.listStaleBranches` /
  `purgeStaleBranch` passthroughs.
- **UI (`ProjectSettingsSheet`):** editable "Stale after (days)" threshold + a "Stale Branches" section
  (Find/Rescan → list with preview + age + node count → Purge with a destructive `confirmationDialog`).

## Files
- New: `Scrivi/Views/ForkPopover.swift`, `docs/Scrivi_SP055_Branching_Architecture_Pass_v0_1.md`
- C++: `ScriviCore/src/history/HistoryService.{hpp,cpp}`, `HistoryStore.{hpp,cpp}`,
  `public_api/scrivi_c_api.cpp`, `include/scrivi/scrivi.h`, `util/Json.{hpp,cpp}` (`objectKeys`)
- Swift: `Engine/ScriviEngine.swift`, `App/HistoryCapture.swift`, `Views/ManuscriptTextView.swift`,
  `Views/ProjectSettingsSheet.swift`, `Views/ForkPopover.swift`
- Tests: `tests/unit/HistoryServiceTests.cpp`, `tests/integration/HistoryCApiTests.cpp`
- `Scrivi.xcodeproj/project.pbxproj` (`ForkPopover.swift` in all three targets)

## Verification
- **Backend suite 264/264** — 10 `[History][branch]` / `[History][branch][stale]` unit cases
  (branch/reselect, 2 eviction, 5 stale detection/purge/rejection) + 3 `[HistoryCApi][branch]` relaunch
  round-trips (re-selected branch survives reopen; evicted branch does not resurrect; purged branch does
  not resurrect, on-path/root rejected). macOS **BUILD SUCCEEDED**.
- **Manual pass (2026-07-13, user-approved):**
  1. Undo onto a fork → fork popover appears → pick a branch → redo walks it. ✅
  2. Project Settings → low "Stale after (days)" → Find stale branches → Purge → confirm; reopen project,
     branch stays gone. ✅
