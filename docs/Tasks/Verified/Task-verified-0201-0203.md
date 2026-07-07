# SP-052 — Undo/Redo linear history engine core + C ABI + Swift wrappers

**Tasks:** T-0201, T-0202, T-0203 · **Epic:** EP-019 · **Sprint:** SP-052
**Status:** ✅ Implemented - Verified
**Date Verified:** 2026-07-07 (user-approved; SP-052 closed and SP-053 activated at the same sign-off)
**Design:** `docs/Scrivi_UndoRedo_History_and_Copy_Buffers_Design_v0_1.md` (§3.a, §3.b, §4, §5, §7, §8)

---

## T-0201 — `HistoryService` core (in-memory, linear)

**Files:** `ScriviCore/src/history/HistoryService.{hpp,cpp}` (new),
`ScriviCore/tests/unit/HistoryServiceTests.cpp` (new, 11 cases), `ScriviCore/CMakeLists.txt`,
`ScriviCore/tests/CMakeLists.txt`.

UI-agnostic C++ (no JSON/nlohmann/UI types). `record` diffs the supplied full scene text against a
cached per-scene head text (common-prefix/suffix trim on **UTF-8 scalar boundaries**, §3.a), storing
the minimal `Diff`; identical text is a no-op. `undo`/`redo` return full scene text + cursor (§3.b);
undo restores the parent text and the departed node's `cursorBefore`. `recordBarrier` adds structural
barrier nodes (§4.5) — **undo stops at a barrier without moving**, reporting kind/note. One `sessionID`
per open; undo landing on an earlier session flags `crossedSessionBoundary` + timestamp (§5). Nodes
carry `parentID`/`primaryChildID`/`childIDs` so SP-055 branching drops in without a rewrite. Disk
persistence is deferred to SP-054 (this layer is purely in-memory).

## T-0202 — `scrivi_history_*` C ABI

**Files:** `ScriviCore/include/scrivi/scrivi.h` (6 declarations + doc block),
`ScriviCore/src/public_api/scrivi_c_api.cpp` (registry, helpers, 6 functions),
`ScriviCore/tests/integration/HistoryCApiTests.cpp` (new, 6 cases), `ScriviCore/tests/CMakeLists.txt`.

Six functions with the standard `{"ok":…,"result"/"error":…}` envelopes: `scrivi_history_open`,
`_record_event`, `_record_barrier`, `_undo`, `_redo`, `_close`. Backed by a **per-project
`HistoryService` registry** (`projectRootPath → unique_ptr<HistoryService>`, mutex-guarded for
cross-thread Swift calls) — the ABI's first stateful-per-open registry (§7). Local `ses_…`/`evt_…` ID
minter (UUID-v7 format) and timestamp source kept in the ABI layer so the shared `UUIDProvider`
interface stays untouched. Undo/redo envelopes carry `moved`, `nodeID`, `canUndo`, `canRedo`,
`crossedSessionBoundary` (+`boundaryTimestamp`), and `stoppedAtBarrier:{kind,note}`. The design's later
functions (`_get_tree`, `_select_branch`, stale/purge, settings, `scrivi_buffers_*`) are deliberately
**not** declared yet — they land with their SP-054–SP-057 sprints.

## T-0203 — `ScriviEngine.swift` history wrappers + interop tests

**Files:** `Scrivi/Engine/ScriviEngine.swift` (6 real methods + 6 stubs + 6 Decodable result types),
`Scrivi/Tests/ScriviInteropTests.swift` (6 new Swift-Testing cases),
`Scrivi.xcodeproj/project.pbxproj` (see deployment-target note below).

Pure-decode wrappers over the C ABI (no history logic in Swift, §8): `historyOpen`,
`historyRecordEvent`, `historyRecordBarrier`, `historyUndo`, `historyRedo`,
`historyClose` (`@discardableResult`); params built with `JSONEncoder` for safe escaping; matching
stub methods in the `#else` (visionOS) block. Result types (`HistoryOpenResult`, `HistoryRecordResult`,
`HistoryStepResult` with `[HistorySceneChange]`/optional `boundaryTimestamp`/optional
`HistoryBarrierStop`, `HistoryBarrierResult`, `HistoryCloseResult`) are all `decodeIfPresent`-tolerant.

**Deployment-target change (user-directed 2026-07-07):** all app targets and `ScriviInteropTests`
raised to **27.0** (`MACOSX_DEPLOYMENT_TARGET` / iOS / visionOS uniformly 27.0). The prior
`ScriviInteropTests = 26.6` blocked linking the 27.0 app module. I-0052's "macOS left at 26.6" note
marked **OBE**; CLAUDE.md updated to 27.0. `plutil -lint` OK.

---

## Verification (2026-07-07, user-approved)

- **Backend `ctest`/suite:** `HistoryServiceTests` 11 cases + `HistoryCApiTests` 6 cases pass; full
  ScriviCore suite **241 cases / 1364 assertions**, no regression (was 224 pre-SP-052).
- **macOS interop:** `xcodebuild -scheme ScriviApp -destination 'platform=macOS' test` → **32/32
  pass**, including the 6 new history cases and the **record→undo→redo round-trip through the Swift
  layer** (text + cursor correct), barrier-stops-undo, and pre-open-throws-`ScriviError`.
- **SP-052 exit criteria met:** ctest green incl. history unit tests; an interop test round-trips
  record→undo→redo through the C ABI (and through the Swift wrappers).

**No pbxproj entries for ScriviCore internals** — `ScriviCore/` is a folder reference built by the
CMake run-script into `libScriviCore.a`; new C++ files appear in Xcode automatically. The modulemap
includes `scrivi.h` wholesale, so the six new functions are visible to Swift without a modulemap edit.
