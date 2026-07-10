# SP-054: Undo/Redo — persistence, sessions, capacity, settings

**Status:** ✅ Closed (2026-07-09, user-approved)
**Epic:** EP-019 — Custom Undo/Redo History & Multiple Copy Buffers
**Goal:** History survives quit/relaunch; the session-boundary warning works; capacity + eviction are
enforced; settings are exposed per Trade T1. **Delivers EP-019 AC3 + AC5** (AC5: linear eviction now;
full branch-aware purge lands in SP-055).
**Start Date:** 2026-07-07
**End Date:** 2026-07-09
**Depends on:** SP-052/SP-053 (✅ closed) — in-memory `HistoryService` + C ABI + Swift capture layer.
**Design:** `docs/Scrivi_UndoRedo_History_and_Copy_Buffers_Design_v0_1.md` §6 (persistence), §5
(sessions/capacity/floor), Appendix A (`rec:"floor"|"event"|"ctl"`, `state.json`, `historySettings`).

### Assigned Tasks

| ID | Title | Priority | Outcome |
| -- | ----- | -------- | ------- |
| T-0207 | JSONL log + checkpoint + torn-line recovery + head-hash validation | High | ✅ Verified → `../../Tasks/Verified/Task-verified-0207-0209.md` |
| T-0208 | Capacity/eviction + history settings (T1) + Project Settings row | Medium | ✅ Verified → `../../Tasks/Verified/Task-verified-0207-0209.md` |
| T-0209 | Session-boundary warning popup | Medium | ✅ Verified → `../../Tasks/Verified/Task-verified-0207-0209.md` |

### What shipped
- **`HistoryStore`** (`ScriviCore/src/history/HistoryStore.{hpp,cpp}`) — on-disk `history/` store per
  project: append-only `log-NNNNNN.jsonl` (`rec:"floor"|"event"|"ctl"`), atomic `state.json`
  checkpoint, load-by-replay with torn-final-line truncation, and head-hash validation (§6.b:
  mismatch ⇒ `externalChange` barrier + re-seed that scene's floor from disk, never touching the
  manuscript). Adds `FileSystem::appendTextFile`.
- **Linear capacity eviction** in `HistoryService` — past `capacityEvents` the oldest node folds into
  the per-scene floor; the root→current path is never evicted. `historySettings` via
  `scrivi_history_get/set_settings` + a "Maximum undo events" Project Settings row.
- **Session-boundary warning** — `crossedSessionBoundary` surfaces an `NSAlert` once per crossing,
  formatted by `HistoryTimestamp.friendly`.

### Exit criteria — all met
- ✅ Quit/relaunch then undo a previous-session edit, with the boundary warning shown.
- ✅ `kill -9` mid-write loses at most the last event; manuscript intact.
- ✅ An external scene edit produces an `externalChange` barrier, never a manuscript change.
- ✅ Project Settings capacity field persists.

### Verification
- **Backend suite 249/249** (15 `[History]` unit + 10 `[HistoryCApi]` integration) — persist-across-
  relaunch undo, torn-line recovery, cross-session warning, external-change barrier, capacity eviction.
- **macOS BUILD SUCCEEDED**, no concurrency/actor/Sendable warnings.
- **User manual pass 2026-07-09** — all 6 checklist items passed (relaunch undo, once-per-crossing
  warning, `kill -9` durability, external-change barrier, settings persistence, eviction).

*Closed 2026-07-09 on user approval. Next in EP-019: SP-055 (branching — tree ops, fork popover,
full branch-aware purge).*
