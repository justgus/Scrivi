# SP-053 — In-session undo/redo on macOS (capture, apply, barriers)

**Tasks:** T-0204, T-0205, T-0206 · **Epic:** EP-019 · **Sprint:** SP-053
**Status:** ✅ Implemented - Verified
**Date Verified:** 2026-07-07 (user-confirmed live: ⌘Z / ⇧⌘Z behavior verified in the running macOS
app, including the three bug-fixes from the first verification pass)
**Design:** `docs/Scrivi_UndoRedo_History_and_Copy_Buffers_Design_v0_1.md` (§4.a triggers, §4.d flush
invariant, §4.5 barriers, §5 root floor, §8 apply path + T-0199 ⌘Z routing)

**Delivers EP-019 AC1** (the fix formerly tracked as I-0019).

---

## T-0204 — `HistoryCapture` + commit triggers
- **`Scrivi/App/HistoryCapture.swift`** (new) — `@MainActor final class`, owned by `ProjectSession`
  (opened in `load`, closed in `close`). Single pending-edit latch; all tree/diff/session logic stays
  in ScriviCore. `noteEdit` latches; `flush(soft:)` commits via `engine.historyRecordEvent`;
  `flushThenSave` enforces §4.d; `recordBarrier` for §4.5; `withApplying` brackets undo/redo apply.
- **Coordinator wiring** (`ManuscriptTextView`) — `textDidChange` latches (storage cursor → scene-local
  UTF-8 byte offset, §4.b) and commits on `. ! ?`/Return; `textViewDidChangeSelection` commits on
  scene switch (hard) or in-scene cursor move (soft); the 1 s auto-save `flushThenSave`s first.
- **Paste/cut** (`ManuscriptNSTextView`) commit as distinct events; system pasteboard untouched.

## T-0205 — Apply path + `allowsUndo=false` + ⌘Z routing
- `allowsUndo = false`; **no `UndoManager` proxy** — first-responder `undo(_:)`/`redo(_:)` action
  methods + `validateUserInterfaceItem` (the T-0199-validated mechanism); default SwiftUI Edit ▸
  Undo/Redo and ⌘Z/⇧⌘Z route here.
- Apply = ranged `replaceCharacters` in the scene's boundary under `withApplying`, cursor restored
  from `cursorAfter` (scene-local byte → storage char), immediate `saveScene`. Dividers/headings
  untouched; restored text uses body attrs (no bold/color drift).

## T-0206 — Barriers on structural operations
- The four in-editor structural handlers (create scene/chapter, merge scene/chapter) record barriers;
  undo stops with an `NSAlert` notice. Navigator-triggered delete/reorder barriers: documented
  follow-up.

## Fixes from the first verification pass (user-reported, all resolved)
1. **Whitespace no longer commits as its own event** — soft triggers defer a whitespace-only pending
   delta (`isWhitespaceOnlyDelta`); the whitespace rides into the next event once real text follows.
2. **Undo no longer turns the scene bold** — apply uses `rebuildStorage`'s body attrs exactly.
3. **Undo no longer empties pre-existing scene text; the history floor surfaces a notice** — new
   `scrivi_history_seed_scene` / `HistoryService::seedSceneBaseline` seeds the scene's floor; root
   undo returns `stoppedAtBarrier{kind:"historyStart"}`, note "Can't undo past the start of the
   recorded history." **Root-floor semantics clarified in design §5** (floor, not a per-session wall;
   seeding is one-time-at-history-creation, the SP-053 per-session seed is a documented in-memory
   stand-in; SP-054 persists the root `rec:"floor"` and re-seeds only on head-hash mismatch).

## Concurrency
`HistoryCapture` is `@MainActor`, owned by the `@MainActor ProjectSession`, driven by the `@MainActor`
Coordinator — all same-actor: no cross-actor hops, no Sendable requirements, **zero concurrency/actor/
isolation warnings** in a clean build.

## Files
- `Scrivi/App/HistoryCapture.swift` (new), `Scrivi/App/ProjectSession.swift`,
  `Scrivi/Views/ManuscriptTextView.swift`, `Scrivi/Engine/ScriviEngine.swift`
- `ScriviCore/include/scrivi/scrivi.h`, `ScriviCore/src/public_api/scrivi_c_api.cpp`,
  `ScriviCore/src/history/HistoryService.{hpp,cpp}`
- `ScriviCore/tests/unit/HistoryServiceTests.cpp`, `ScriviCore/tests/integration/HistoryCApiTests.cpp`,
  `Scrivi/Tests/ScriviInteropTests.swift`
- `Scrivi.xcodeproj/project.pbxproj` (new `HistoryCapture.swift` → `C060` + `C060_BF`/`D060_BF`/
  `E060_BF` across all 3 app targets; `plutil -lint` OK)
- `docs/Scrivi_UndoRedo_History_and_Copy_Buffers_Design_v0_1.md` (§5 root-floor clarification)

## Verification
- **User-confirmed live (2026-07-07):** ⌘Z / ⇧⌘Z work in the running macOS editor; the three reported
  issues (whitespace event, bold-on-undo, empties-pre-existing-text) are fixed.
- macOS **BUILD SUCCEEDED**, no concurrency warnings, app launches.
- Backend suite **244/244** (13 `[History]` unit + 7 `[HistoryCApi]` cases incl. seed/floor).
