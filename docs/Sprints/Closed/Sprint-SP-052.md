# SP-052: Undo/Redo — linear history engine core (C++) + C ABI + Swift wrappers

**Status:** ✅ Closed (2026-07-07, user-approved)
**Epic:** EP-019 — Custom Undo/Redo History & Multiple Copy Buffers
**Goal:** A linear (non-branching) history engine lives in ScriviCore with full unit coverage and is
callable from Swift — the foundation SP-053's in-session undo/redo builds on.
**Start Date:** 2026-07-07
**End Date:** 2026-07-07
**Design:** `docs/Scrivi_UndoRedo_History_and_Copy_Buffers_Design_v0_1.md` (§3.a, §3.b, §4, §5, §7, §8)

### Assigned Tasks

| ID | Title | Priority | Outcome |
| -- | ----- | -------- | ------- |
| T-0201 | `HistoryService` core (record/undo/redo, snapshot-diff, sessions) + Catch2 tests | High | ✅ Verified → `../../Tasks/Verified/Task-verified-0201-0203.md` |
| T-0202 | C ABI: `scrivi_history_open/record_event/record_barrier/undo/redo/close` | High | ✅ Verified → `../../Tasks/Verified/Task-verified-0201-0203.md` |
| T-0203 | `ScriviEngine.swift` history wrappers + interop tests | High | ✅ Verified → `../../Tasks/Verified/Task-verified-0201-0203.md` |

### What shipped

- **In-memory linear `HistoryService`** (`ScriviCore/src/history/`): snapshot-diff via UTF-8
  scalar-safe prefix/suffix trim (§3.a); undo/redo return full scene text + cursor (§3.b); structural
  **barriers** stop undo (§4.5); one session per open with a `crossedSessionBoundary` flag (§5). Nodes
  carry `parentID`/`primaryChildID` so SP-055 branching drops in without a rewrite. In-memory only —
  disk persistence is SP-054.
- **Six `scrivi_history_*` C ABI functions** with standard JSON envelopes, backed by a per-project
  `HistoryService` registry (mutex-guarded). The design's branching/settings/buffers functions are
  deliberately deferred to their later sprints.
- **`ScriviEngine.swift` pure-decode wrappers** + Decodable result types; matching visionOS stubs.

### Exit Criteria met

- [x] `ctest` green including the new history unit tests — **ScriviCore suite 241/241** (11 `[History]`
      unit + 6 `[HistoryCApi]` integration cases added; no regression from the pre-SP-052 224).
- [x] An interop test round-trips **record → undo → redo through the C ABI** — and through the Swift
      wrappers: **macOS `xcodebuild … test` 32/32**, incl. text+cursor correctness, barrier-stop, and
      pre-open error throwing.

### Notes / decisions during the sprint

- **Deployment targets unified to 27.0 (user-directed at close).** The committed `ScriviInteropTests`
  target was at `MACOSX_DEPLOYMENT_TARGET = 26.6`, blocking the test bundle from linking the 27.0
  `ScriviApp` module. All app + test targets (macOS/iOS/visionOS) raised to 27.0; **I-0052's "macOS
  left at 26.6" note marked OBE**; CLAUDE.md version block updated 26.2+ → 27.0.
- **No pbxproj entries for ScriviCore internals** — `ScriviCore/` is a folder reference built by the
  CMake run-script into `libScriviCore.a`; the modulemap includes `scrivi.h` wholesale (new C ABI
  functions auto-visible to Swift).

### Retrospective

Clean sprint — the layered plan (C++ core → C ABI → Swift wrappers) meant each layer was independently
testable, and the only surprise (the 26.6/27.0 deployment mismatch) was a pre-existing config drift
surfaced by running the interop suite, not new work. SP-053 (in-session ⌘Z/⇧⌘Z, delivering AC1) is
unblocked and activated.
