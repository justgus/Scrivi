# Active Tasks

**Sprint SP-054** — Undo/Redo: persistence, sessions, capacity, settings. Design:
`docs/Scrivi_UndoRedo_History_and_Copy_Buffers_Design_v0_1.md` §6, §5, Appendix A.

| ID | Title | Status |
| -- | ----- | ------ |
| T-0207 | JSONL log + checkpoint + torn-line recovery + head-hash validation | 🟡 Implemented — Not Verified |
| T-0208 | Capacity/eviction + history settings (T1) + Project Settings row | 🟡 Implemented — Not Verified |
| T-0209 | Session-boundary warning popup | 🟡 Implemented — Not Verified |

---

## T-0207 / T-0208 / T-0209 — persistence, capacity, sessions — 🟡 Implemented, Not Verified (2026-07-07)

**Epic:** EP-019 · **Sprint:** SP-054 · Delivers **AC3** (persistence + session warning) and **AC5**
(capacity — linear eviction now; full branch-aware in SP-055).

### T-0207 — Log + checkpoint + recovery + head-hash validation
- **New `HistoryStore`** (`ScriviCore/src/history/HistoryStore.{hpp,cpp}`) — owns the on-disk
  `history/` store for a project and keeps a `HistoryService` in sync:
  - Append-only `history/log-NNNNNN.jsonl` with `rec:"floor"|"event"|"ctl"` records (Appendix A.1),
    one JSON object per line, written via the new `FileSystem::appendTextFile`.
  - Atomic `history/state.json` checkpoint (A.2) every ~200 records + on settings change + on close.
  - **Load = replay:** reads the log, **truncates a torn final line** (parse-fail → stop), replays
    records to rebuild the tree; state.json supplies pointers/settings/head-hashes as an accelerator.
  - **Head-hash validation (§6.b):** `scrivi_history_validate_scene(root, sceneID, diskText)` compares
    the scene's on-disk text against the persisted head hash; a mismatch records an **`externalChange`
    barrier** and re-seeds that scene's floor from disk — **never modifying the manuscript**. Swift
    calls it per loaded scene right after open (`HistoryCapture.validateScenes`, from
    `ProjectSession.load`).
- **One-time root floor:** the SP-053 per-session seed stand-in is now backed by a persisted
  `rec:"floor"` record written once when a scene first enters history (design §5). `HistoryService`
  gained `floorTexts_` (immutable baseline), a persistence surface (`nodes()`, `floorTexts()`,
  `addLoadedFloor/Node`, `setPointers`, `finalizeLoad`), and `reseedSceneFloor`.
- **Session boundary (§5):** `undo` now warns when stepping into a node from a session other than the
  current open one, **once per crossing** (`warnedSessions_`) — fires even when the whole loaded chain
  predates this launch.
- `HistoryRegistry` now holds `HistoryStore` (owning the service). The C-ABI tests use unique temp
  roots (persistence made a shared fixed root leak state between tests).

### T-0208 — Capacity + settings + UI
- **Linear eviction** in `HistoryService` (SP-055 does full branch-aware): past `capacityEvents`, the
  oldest node is evicted from the root — its diff folds into the per-scene floor, the child becomes the
  new root — and **never the root→current path** (`rebuildHeadCache` skips the root's own diff so a
  promoted root isn't double-applied). `record` returns `evictedCount`.
- **`historySettings{capacityEvents,staleBranchDays,idleRolloverHours}`** via
  `scrivi_history_get/set_settings` + engine wrappers; settings mirrored in `state.json` and applied to
  the engine capacity at open/set.
- **Project Settings UI:** an "Undo History" section (`ProjectSettingsSheet`) with a "Maximum undo
  events" field, loaded on appear and saved on Done via the engine.

### T-0209 — Session-boundary warning popup
- The apply path surfaces `crossedSessionBoundary` as an `NSAlert` ("Undoing changes from a previous
  session… made yesterday at 9:42 PM"), formatted by the new `HistoryTimestamp.friendly` helper.
  Once per crossing (engine-enforced).

### Files
- New: `ScriviCore/src/history/HistoryStore.{hpp,cpp}`
- C++: `ScriviCore/src/history/HistoryService.{hpp,cpp}` (floor/persistence surface, eviction,
  session-boundary, reseed), `ScriviCore/src/platform/LocalFileSystem.{hpp,cpp}` +
  `ScriviCore/include/scrivi/Services.hpp` (`appendTextFile`),
  `ScriviCore/include/scrivi/scrivi.h` + `src/public_api/scrivi_c_api.cpp` (`validate_scene`,
  `get/set_settings`; registry now holds `HistoryStore`)
- Swift: `Scrivi/Engine/ScriviEngine.swift` (validate/settings wrappers + result types),
  `Scrivi/App/HistoryCapture.swift` (`validateScenes`, `HistoryTimestamp`),
  `Scrivi/App/ProjectSession.swift` (validate on load),
  `Scrivi/Views/ManuscriptTextView.swift` (session-boundary alert),
  `Scrivi/Views/ProjectSettingsSheet.swift` (Undo History section)
- Tests: `ScriviCore/tests/unit/HistoryServiceTests.cpp` (+eviction),
  `ScriviCore/tests/integration/HistoryCApiTests.cpp` (+relaunch, +torn-line, +external-change)
- **No pbxproj change** — no new `.swift`; `HistoryStore.*` are folder-referenced/CMake-built.

### Verification performed (dev)
- Backend suite **249/249** (15 `[History]` unit + 10 `[HistoryCApi]` cases) — incl.
  **persist-across-relaunch undo**, **torn-line recovery**, **cross-session warning**,
  **external-change barrier**, and **capacity eviction**.
- macOS **BUILD SUCCEEDED**, **no concurrency/actor/Sendable warnings**.
- **Interop test host will not launch** in this environment (LaunchServices) — per user direction,
  Swift/app layer is verified by the backend suite (same C ABI) + user manual check.

### Not verified (user manual pass)
Quit/relaunch then undo yesterday's edit with the session-warning popup; `kill -9` mid-write loses at
most the last event; an external scene edit produces an `externalChange` barrier (no manuscript
change); the Project Settings capacity field persists.

---

*Last Updated: 2026-07-07 (SP-054 implemented — persistence (`HistoryStore`: JSONL log + checkpoint +
torn-line replay + head-hash validation), linear capacity eviction + settings + Project Settings row,
and the session-boundary warning popup. Backend 249/249; macOS build clean, no concurrency warnings.
Interop host launch env-blocked. Awaiting user manual verification.)*
