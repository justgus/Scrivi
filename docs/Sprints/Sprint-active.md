# Active Sprint

## SP-054: Undo/Redo — persistence, sessions, capacity, settings

**Status:** 🟢 Active
**Epic:** EP-019
**Activated:** 2026-07-07
**Depends on:** SP-052/SP-053 (✅ closed) — the in-memory `HistoryService` + C ABI + Swift capture
layer are in place; this sprint makes the history durable across quit/relaunch.
**Goal:** History survives quit/relaunch; the session-boundary warning works; capacity + eviction are
enforced; settings are exposed per Trade T1.

**Design reference:** `docs/Scrivi_UndoRedo_History_and_Copy_Buffers_Design_v0_1.md` §6 (persistence),
§5 (sessions/capacity/floor), Appendix A (field spec: `rec:"floor"|"event"|"ctl"`, `state.json`,
`historySettings`). Epic ACs: `docs/Epics/Epic-active.md` (AC3, AC5). Task detail: `Task-backlog.md`.

### Assigned Tasks
| ID | Title | Priority | Status |
| -- | ----- | -------- | ------ |
| T-0207 | JSONL log + checkpoint + torn-line recovery + head-hash validation | High | 🟡 Implemented — Not Verified (2026-07-07) |
| T-0208 | Capacity/eviction + history settings (T1) + Project Settings row | Medium | 🟡 Implemented — Not Verified (2026-07-07) |
| T-0209 | Session-boundary warning popup | Medium | 🟡 Implemented — Not Verified (2026-07-07) |

### Plan notes
- **T-0207** — new `HistoryStore` (C++, `src/history/`) owns serialization: append-only
  `history/log-NNNNNN.jsonl` (`rec:"floor"|"event"|"ctl"`, Appendix A.1), atomic `history/state.json`
  checkpoint (A.2), torn-final-line truncation + log-tail replay, and head-hash validation at open
  (§6.b: mismatch ⇒ `externalChange` barrier, re-seed that scene from disk, **never** touch the
  manuscript). Adds `FileSystem::appendTextFile` (only one impl, `LocalFileSystem`; no FS mock).
  `scrivi_history_open` gains persistence: load-or-create the store for the project; the one-time root
  **floor** seed moves here (replaces SP-053's in-memory per-session `seedSceneBaseline` stand-in —
  design §5). `history/` added to the external-change scanner ignore set.
- **T-0208** — capacity eviction from the root (auto-purge aged-off non-primary subtrees; never evict
  the root→current path); `historySettings{capacityEvents,staleBranchDays,idleRolloverHours}` in
  `project.json` (Trade T1) surfaced via `scrivi_history_get/set_settings` + a Project Settings UI row.
- **T-0209** — the Swift session-warning popup: `crossedSessionBoundary` (already returned by undo)
  drives a once-per-crossing alert showing the boundary wall-clock time; session definition Trade T5
  (project-open span + 8 h idle rollover) is minted at open and on idle rollover.
- **New `.cpp/.hpp/.swift` files → CMake + `project.pbxproj` in the same step** (CLAUDE.md).

**Exit criteria:** quit/relaunch then undo yesterday's edit with the boundary warning shown; `kill -9`
mid-write loses at most the last event; an external scene edit produces an `externalChange` barrier,
never a manuscript change.

---

*Last Updated: 2026-07-07 (SP-054 activated on user approval, immediately after SP-053 closed —
`Closed/Sprint-SP-053.md`. SP-054 makes the history durable + adds sessions/capacity/settings,
delivering EP-019 AC3 and AC5.)*
