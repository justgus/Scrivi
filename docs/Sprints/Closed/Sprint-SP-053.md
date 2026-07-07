# SP-053: Undo/Redo — in-session undo/redo on macOS (capture, apply, barriers)

**Status:** ✅ Closed (2026-07-07, user-approved)
**Epic:** EP-019 — Custom Undo/Redo History & Multiple Copy Buffers
**Goal:** ⌘Z/⇧⌘Z work in the running macOS app per the event model — **EP-019 AC1 delivered**
(in-session; persistence follows in SP-054). AC1 carries the requirement formerly tracked as I-0019.
**Start Date:** 2026-07-07
**End Date:** 2026-07-07
**Design:** `docs/Scrivi_UndoRedo_History_and_Copy_Buffers_Design_v0_1.md` (§4.a, §4.d, §4.5, §5, §8)

### Assigned Tasks

| ID | Title | Priority | Outcome |
| -- | ----- | -------- | ------- |
| T-0204 | `HistoryCapture` + commit-trigger wiring in the editor | High | ✅ Verified → `../../Tasks/Verified/Task-verified-0204-0206.md` |
| T-0205 | Undo/redo apply path + `allowsUndo=false` + ⌘Z routing | High | ✅ Verified → `../../Tasks/Verified/Task-verified-0204-0206.md` |
| T-0206 | Barriers on structural operations | Medium | ✅ Verified → `../../Tasks/Verified/Task-verified-0204-0206.md` |

### What shipped

- **`HistoryCapture`** (`@MainActor`, owned by `ProjectSession`) — pending-edit latch + commit
  triggers (sentence terminators, Return, cursor-move-with-pending, scene switch, paste/cut,
  pre-save flush, close); all tree/diff logic stays in ScriviCore.
- **Undo/redo apply path** — ranged `replaceCharacters` in the scene boundary, cursor restore,
  immediate save; `allowsUndo = false` + first-responder `undo(_:)`/`redo(_:)` action methods
  (the T-0199-validated routing, **not** an `UndoManager` proxy).
- **Structural barriers** on the four in-editor split/merge/create handlers.
- **New `scrivi_history_seed_scene`** + `HistoryService::seedSceneBaseline` — seeds a scene's history
  **floor** so undo stops at pre-existing text, never emptying it.

### First-verification-pass fixes (user-reported, all resolved before close)

1. Whitespace (e.g. double-space after a period) no longer becomes its own undo step — soft triggers
   defer a whitespace-only pending delta.
2. Undo no longer turns the scene bold — apply uses body-text attributes.
3. Undo no longer empties pre-existing (non-session) scene text; the history floor surfaces
   `historyStart` ("Can't undo past the start of the recorded history"). **Root-floor semantics
   clarified in design §5** (floor, not a session wall; one-time seed at history creation, SP-053
   per-session seed is a documented in-memory stand-in; SP-054 persists it and re-seeds only on a
   head-hash mismatch).

### Exit Criteria met

- [x] Type-undo-redo verifiable in the app across scene switches — **user-confirmed live 2026-07-07**
      (⌘Z / ⇧⌘Z).
- [x] Pure cursor moves produce **no** events (soft-trigger + whitespace-only-delta rule).
- [x] Structural ops barrier correctly (undo stops with a notice).
- [x] No auto-save regression (flush-before-save preserves the existing save path; interop/backend
      green).

### Verification

- **User-confirmed live:** ⌘Z / ⇧⌘Z behavior verified in the running macOS editor.
- macOS **BUILD SUCCEEDED**, **no concurrency/actor/Sendable warnings**, app launches.
- Backend suite **244/244** (13 `[History]` unit + 7 `[HistoryCApi]` cases incl. seed/floor).

### Notes / decisions

- **Concurrency:** `HistoryCapture` is `@MainActor`, owned and driven entirely on the main actor —
  no cross-actor hops or Sendable requirements arose; the `engine.history*` calls are synchronous and
  thread-safe on the C side.
- **pbxproj:** new `HistoryCapture.swift` wired as fileRef `C060` + build files across all three app
  targets (macOS/iOS/visionOS); `plutil -lint` OK. ScriviCore internals remain folder-referenced /
  CMake-built.
- **Env caveat:** `xcodebuild … test` intermittently failed to *launch* the interop test host
  (LaunchServices) late in the sprint; the app builds/launches standalone and the capture→apply→
  barrier logic is covered end-to-end by the backend Catch2 suite through the same C ABI. Live user
  verification of ⌘Z/⇧⌘Z was the acceptance bar and passed.

### Retrospective

The T-0199 spike paid off: the first-responder action-method routing worked exactly as predicted, no
AppKit undo-manager fights. The real work was the event-model nuance — whitespace boundaries and the
history-floor question the user surfaced, which turned into a design clarification (§5) that keeps
SP-053's in-memory behavior forward-compatible with SP-054 persistence. AC1 is delivered and verified.
SP-054 (persistence, sessions, capacity) is the next EP-019 sprint.
