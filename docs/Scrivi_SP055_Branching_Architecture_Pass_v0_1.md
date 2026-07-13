# SP-055 Branching — Step-0 Architecture Pass (v0.1)

**Status:** 🟠 Draft for user review (2026-07-10)
**Sprint:** SP-055 · **Epic:** EP-019 · **Task:** T-0210 (blocks T-0211/T-0212)
**Design source of truth:** `Scrivi_UndoRedo_History_and_Copy_Buffers_Design_v0_1.md` §5, §7, §10 T2, Appendix A
**Purpose:** Settle the `forkAhead` envelope contract and the tree-ops plan against the *existing*
`HistoryService` before writing engine code. No code changed by this document.

---

## 1. Headline finding — branching is already 90% structural

The engine was built branch-ready (SP-052 note). The tree **physically forms correctly today**:
when the writer undoes (pointer → parent) and then records, `record()` appends a new child to that
parent and sets it primary (`HistoryService.cpp:143–147`). The old chain is preserved under the
parent's `childIDs`; it is simply no longer `primaryChildID`. So **fork-on-record already happens** —
what's missing is *surfacing* it and *navigating back* to it.

Concretely, five deltas remain. None is a rewrite.

| # | Gap | Where | Task |
|---|-----|-------|------|
| D1 | `createdBranch` never set true (the popover's trigger signal) | `record()` | T-0210 |
| D2 | No `select_branch` — can't re-primary an old child | new method | T-0210 |
| D3 | `forkAhead` not computed/emitted on undo/redo | `undo()`/`redo()` + ABI | T-0210 |
| D4 | `finalizeLoad()` infers primary as "last child wins" — wrong once forks persist | `finalizeLoad()` + store | T-0210 |
| D5 | Eviction + stale detection are linear-only | `evictToCapacity()` + new | T-0210 / T-0212 |

---

## 2. The `forkAhead` envelope contract (the thing T-0211 depends on)

The design already specifies it (§7 line 172). This pass **confirms it verbatim** — no new invention —
and pins the emit rules so the Swift popover (T-0211) can be built against a fixed shape.

### 2.1 Shape (added to the undo/redo result envelope, optional)

```json
"forkAhead": {
  "nodeID": "evt_fork…",              // the fork node the pointer is AT
  "children": [
    { "eventID": "evt_child…",         // a child branch's first node
      "preview":  "…short text…",      // first ~N chars of that branch's scene text delta
      "timestamp": "2026-07-06T…Z",
      "isPrimary": true }              // exactly one child is primary
  ]
}
```

- Present **only** when the pointer lands on a node with **≥ 2 children** (a real fork). Absent
  otherwise — a single child is implicitly primary and needs no popover.
- `children` order = `childIDs` creation order (stable); the popover renders in that order.
- `preview` is computed engine-side from each child's `diff.inserted` (falling back to the child's
  head text head-slice for deletes), capped to a small length constant. Scene-local, UTF-8 safe.
- `isPrimary` marks the current `primaryChildID` so the popover can show the default selection.

### 2.2 When it is emitted (maps 1:1 to §10 T2 refinements 1/2/4/5)

| Interaction | Engine behavior | `forkAhead`? |
|---|---|---|
| **T2.1** Undo *lands on* a fork | pointer moved to a node with ≥2 children | **emit** |
| **T2.2** Undo *past* the fork | further undo moves pointer to the fork's parent | absent (popover dismisses) |
| **T2.4** Redo *reaches* a fork | pointer moved onto a node with ≥2 children | **emit** |
| **T2.5** Redo *without* selecting | redo follows `primaryChildID` as today | absent unless the child is itself a fork |

Key subtlety, resolved: `forkAhead.nodeID` is the node the pointer is **AT after the step**, and its
`children` are the *forward* branches. On undo this is the fork the writer just arrived at; on redo it
is the fork just reached. T2.3 (branch relegation) needs **no** special mechanic — it falls out of
record-on-a-node-with-children (D1). The engine only *reports* forks; all popover show/dismiss timing
lives in Swift (`HistoryCapture`), driven purely by presence/absence of `forkAhead`.

---

## 3. Tree ops plan (T-0210)

### 3.1 `record()` — set `createdBranch` (D1)
Before appending: `createdBranch = parent.childIDs.size() >= 1` (the parent already had a child ⇒ this
record forks). Everything else about record is unchanged; the new node still becomes primary per §5.
`RecordResult.createdBranch` already exists — just populate it. ABI already emits it.

### 3.2 `selectBranch(forkNodeID, childEventID)` (D2)
New `HistoryService` method + `scrivi_history_select_branch` ABI (design §7):
- Validate `childEventID ∈ nodeRef(forkNodeID).childIDs`; set
  `nodeRef(forkNodeID).primaryChildID = childEventID`.
- Does **not** move the current pointer by itself — it re-primaries the fork. The Swift layer then
  issues redo steps to walk the now-primary branch (matches design §5 "restored branch becomes
  primary again"). Returns `{ok, forkNodeID, childEventID, canRedo}`.
- Persist as `ctl {op:"setPrimary", forkNodeID, childEventID}` (Appendix A.1.c — the record already
  exists in the spec; HistoryStore.persistCtl gains the two fields, see §5).

### 3.3 `forkAhead` computation (D3)
Add a private `std::optional<ForkAhead> forkAheadAt(nodeID)` returning populated data only when that
node has ≥2 children. Called at the end of `undo()` and `redo()` after the pointer moves; result
carried on `StepResult.forkAhead`. ABI serializes it into the undo/redo envelope per §2.1.

### 3.4 Persistence: restore the *saved* primary (D4)
Today `finalizeLoad()` sets `primaryChildID = last child wins`. Correct for a linear log, **wrong**
once `setPrimary` overrides exist. Fix:
- `state.json.primaryChildren` (forks-only map, already in Appendix A.2) is applied after
  `finalizeLoad()` derives childIDs: for each `{forkID: childID}`, override `primaryChildID`.
- On replay of a `ctl {op:"setPrimary"}` record, apply the same override (log is source of truth;
  checkpoint is the accelerator). This keeps A.1.c and A.2 consistent.

---

## 4. Capacity & stale branches

### 4.1 Branch-aware eviction (T-0210, completes AC5 auto-purge)
`evictToCapacity()` today bails unless the root has exactly one child (`HistoryService.cpp:337`).
Per design §5: at the root, **auto-purge non-primary subtrees hanging off the root first**, then
promote the primary child to root and fold its diff into the floor. Never evict a node on the
root→current path; if the current pointer sits in a subtree that would be purged, purge *other*
branches first and **defer** eviction (design §5). Emit `ctl {op:"purge", branchRootEventID}` per
purged subtree and `ctl {op:"evict", newRootID}` per promotion.

### 4.2 Stale-branch detection + purge (T-0212)
- `listStaleBranches()` — a branch = any non-primary subtree; stale = tip older than
  `staleBranchDays` (default 7) **or** branch point > K nodes behind head. Returns
  `[{branchRootEventID, tipTimestamp, nodeCount, reason}]`. ABI: `scrivi_history_list_stale_branches`.
- `purgeBranch(branchRootEventID)` — removes the subtree (refuses if it contains the current pointer);
  `ctl {op:"purge"}`. ABI: `scrivi_history_purge_branch`. User-confirmed in Swift (T-0212 UI).

---

## 5. C ABI additions (all in `scrivi.h` + `scrivi_c_api.cpp`)

New (design §7 — currently *not declared*, the header comment lists them as deferred):
- `scrivi_history_select_branch(root, forkNodeID, childEventID)`
- `scrivi_history_get_tree(root, paramsJSON)`  *(windowed tree — primarily for T-0215 panel in SP-057; a minimal form may land here to test navigation)*
- `scrivi_history_list_stale_branches(root)`
- `scrivi_history_purge_branch(root, branchRootEventID)`

Changed (backward-compatible — new optional field only):
- `scrivi_history_undo` / `scrivi_history_redo` results gain optional `forkAhead{…}`.

`HistoryStore.persistCtl` gains `forkNodeID`/`childEventID`/`branchRootEventID`/`newRootID` params
(currently `HistoryStore.cpp:132` ignores setPrimary/evict/purge). All within the existing A.1.c spec.

---

## 6. Swift layer (T-0211/T-0212 — named here so the contract is complete)

- **`HistoryCapture`** gains fork-popover state: on an undo/redo result carrying `forkAhead`, publish
  it; on a result without it, clear it (drives T2.1/T2.2/T2.4/T2.5 show/dismiss). Engine stays
  UI-agnostic — Swift owns all popover timing.
- **Fork popover view** (T-0211): transient, non-modal, at the caret; lists `forkAhead.children`
  (preview + relative timestamp); ↑↓/1–9 + Return calls `select_branch` then redoes onto the branch;
  Esc keeps primary.
- **Stale-branch purge UI** (T-0212): a count surfaced (e.g. in Project Settings or a prompt) →
  confirm → `purge_branch`.

---

## 7. Test plan (T-0210 unit; §5 canonical scenario)

`[History]` unit cases (in `HistoryServiceTests.cpp`):
1. **undo-type-fork-reselect** (design §5 / AC4): record A→B→C; undo×2 to A; type D; assert B is now a
   non-primary child of A and D is primary; `createdBranch==true`; `forkAhead` at A lists {B,D}.
2. **select_branch re-primaries**: from the above, `selectBranch(A, B)`; redo walks B-chain; abandoned
   D fully restorable by re-selecting D.
3. **forkAhead emit rules**: undo lands on fork ⇒ present; undo past ⇒ absent; redo reaches fork ⇒
   present.
4. **branch-aware eviction**: capacity forces root eviction with a non-primary branch off the root ⇒
   branch auto-purged, primary promoted, current pointer never evicted.
5. **persistence round-trip** (`[HistoryCApi]`): setPrimary survives close/reopen (D4) — the *saved*
   primary is restored, not "last child wins".

Plus the existing 249 must stay green (no regression to linear behavior when there are no forks).

---

## 8. Sequencing recommendation

1. **D1 + D3 + D4** (createdBranch, forkAhead, saved-primary restore) + `select_branch` (D2) — the core
   of T-0210; unblocks T-0211 immediately (the popover has a stable contract).
2. **Branch-aware eviction** (D5, §4.1) — completes AC5 auto-purge.
3. Hand off to **T-0211** (popover) and **T-0212** (stale purge), which can then proceed in parallel.

**Open question for you:** §5 sets the capacity default at **20,000** events; SP-054 shipped the
setting plumbing but the Project Settings row currently exposes an arbitrary "Maximum undo events"
value. Confirm we keep 20,000 as the default here, or adjust. (Not a blocker — I'll default to 20,000
per the design unless you say otherwise.)
