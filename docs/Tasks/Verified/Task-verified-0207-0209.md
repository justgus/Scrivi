# T-0207 / T-0208 / T-0209 — ✅ Verified (2026-07-09)

**Epic:** EP-019 · **Sprint:** SP-054 · Delivers **AC3** (persistence + session warning) and **AC5**
(capacity — linear eviction now; full branch-aware in SP-055).

**Status:** ✅ Verified (2026-07-09, user-approved after a full manual pass — all 6 checklist items
passed). Implemented 2026-07-07; backend machine-verified 249/249 at implementation time.

---

## T-0207 — Log + checkpoint + recovery + head-hash validation
- **New `HistoryStore`** (`ScriviCore/src/history/HistoryStore.{hpp,cpp}`) — owns the on-disk
  `history/` store for a project and keeps a `HistoryService` in sync:
  - Append-only `history/log-NNNNNN.jsonl` with `rec:"floor"|"event"|"ctl"` records (Appendix A.1),
    one JSON object per line, written via `FileSystem::appendTextFile`.
  - Atomic `history/state.json` checkpoint (A.2) every ~200 records + on settings change + on close.
  - **Load = replay:** reads the log, **truncates a torn final line** (parse-fail → stop), replays
    records to rebuild the tree; state.json supplies pointers/settings/head-hashes as an accelerator.
  - **Head-hash validation (§6.b):** `scrivi_history_validate_scene(root, sceneID, diskText)` compares
    the scene's on-disk text against the persisted head hash; a mismatch records an **`externalChange`
    barrier** and re-seeds that scene's floor from disk — **never modifying the manuscript**.
- **One-time root floor:** persisted `rec:"floor"` record written once when a scene first enters
  history (design §5); replaces SP-053's per-session seed stand-in.
- **Session boundary (§5):** `undo` warns when stepping into a node from a session other than the
  current open one, **once per crossing** (`warnedSessions_`).

## T-0208 — Capacity + settings + UI
- **Linear eviction** in `HistoryService` (SP-055 does full branch-aware): past `capacityEvents`, the
  oldest node is evicted from the root — its diff folds into the per-scene floor, the child becomes the
  new root — and **never the root→current path**. `record` returns `evictedCount`.
- **`historySettings{capacityEvents,staleBranchDays,idleRolloverHours}`** via
  `scrivi_history_get/set_settings` + engine wrappers; mirrored in `state.json`.
- **Project Settings UI:** an "Undo History" section (`ProjectSettingsSheet`) with a "Maximum undo
  events" field, loaded on appear and saved on Done via the engine.

## T-0209 — Session-boundary warning popup
- The apply path surfaces `crossedSessionBoundary` as an `NSAlert` ("Undoing changes from a previous
  session… made yesterday at 9:42 PM"), formatted by `HistoryTimestamp.friendly`. Once per crossing.

## Files
- New: `ScriviCore/src/history/HistoryStore.{hpp,cpp}`
- C++: `HistoryService.{hpp,cpp}`, `LocalFileSystem.{hpp,cpp}` + `Services.hpp` (`appendTextFile`),
  `scrivi.h` + `scrivi_c_api.cpp` (`validate_scene`, `get/set_settings`; registry holds `HistoryStore`)
- Swift: `ScriviEngine.swift`, `HistoryCapture.swift`, `ProjectSession.swift`,
  `ManuscriptTextView.swift`, `ProjectSettingsSheet.swift`

## Verification
- **Backend suite 249/249** (15 `[History]` + 10 `[HistoryCApi]`) — persist-across-relaunch undo,
  torn-line recovery, cross-session warning, external-change barrier, capacity eviction.
- **Manual pass (2026-07-09, user-approved) — all passed:**
  1. Relaunch → ⌘Z undoes previous-session edit (history survived quit). ✅
  2. Session-boundary popup shown once per crossing. ✅
  3. `kill -9` mid-write → at most last event lost, manuscript intact. ✅
  4. External scene edit → `externalChange` barrier, manuscript never modified. ✅
  5. "Maximum undo events" setting persists across sheet/project reopen. ✅
  6. Capacity eviction drops oldest events; root→current undo path survives. ✅
