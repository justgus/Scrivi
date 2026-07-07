# SP-051: Undo/Redo — design sign-off, ⌘Z-routing spike, schema spec

**Status:** ✅ Closed (2026-07-06, user-approved)
**Epic:** EP-019 — Custom Undo/Redo History & Multiple Copy Buffers
**Goal:** User signs off the design doc (incl. trade decisions T1–T6); the one high-risk AppKit
assumption (⌘Z/Edit-menu routing without native `NSUndoManager` undo) is validated by a throwaway
spike; the `scrivi.history.v1` / `scrivi.buffers.v1` schemas and the new repair-matrix row are
specified.
**Start Date:** 2026-07-06
**End Date:** 2026-07-06
**Design:** `docs/Scrivi_UndoRedo_History_and_Copy_Buffers_Design_v0_1.md` (✅ Approved baseline 2026-07-06)

### Assigned Tasks

| ID | Title | Priority | Outcome |
| -- | ----- | -------- | ------- |
| T-0198 | Design doc + trade studies | High | ✅ Verified (user design sign-off; trades T1=B, T2=A+refinements, T3=C, T4=B+D+A, T5=C, T6=A) → `../../Tasks/Verified/Task-verified-0198.md` |
| T-0199 | Spike: ⌘Z/⇧⌘Z + Edit-menu routing mechanism | High | ✅ Done — two live user-observed rounds (below); findings in design §8/§12.6; spike code removed, no `.swift` diff remains |
| T-0200 | `scrivi.history.v1` / `scrivi.buffers.v1` schema spec + repair-matrix row | Medium | ✅ Verified → `../../Tasks/Verified/Task-verified-0200.md` |

### T-0199 Spike Findings (the sprint's key technical output)

- **Round 1 — `UndoManager` proxy (design's original §8 proposal): FAILED.**
  (a) The Edit menu never consulted the proxy — zero `canUndo`/`undoMenuItemTitle` queries; items
  stayed disabled with default titles. (b) Even with `allowsUndo = false`, NSTextView's typing
  coalescer (`NSTextViewSharedData coalesceInTextView:` → `_NSUndoStack _setGroupIdentifier:`)
  reached into the proxy's private state and raised `NSInternalInconsistencyException` ("must
  begin a group before registering undo") on every keystroke. AppKit requires a real,
  internally-consistent `NSUndoManager`; a subclass that swallows grouping calls corrupts it
  (design risk §12.6 confirmed real).
- **Round 2 — first-responder action methods: PASSED (adopted).** No `undoManager` override;
  `undo(_:)`/`redo(_:)` implemented on `ManuscriptNSTextView` + `validateUserInterfaceItem`.
  Observed live: validation callbacks fired; **menu clicks AND ⌘Z/⇧⌘Z both delivered to the
  action methods** (sender `SwiftUIMenuItem` — key equivalents route through the SwiftUI menu
  item); typing produced no exceptions and no undo-manager interaction. T-0205 implements this
  mechanism delegating to `HistoryCapture`.
- Spike code fully removed after observation; macOS build green; working tree carried no `.swift`
  changes at sprint close.

### Exit Criteria

- [x] Design doc approved by user (trades T1–T6 ruled; T2 interaction refinements incorporated).
- [x] Spike findings recorded in the design doc (proxy rejected; action-method mechanism confirmed).
- [x] Schema spec reconciled with `Scrivi_Project_Package_Structure_v0_1.md` (§4/§16a/§17) and
      `Scrivi_External_Change_Repair_Matrix_v0_2.md` (§6.21).

### Retrospective

**Completed:**
- Design baseline approved same-day with all six trades ruled; EP-019 activated.
- The one architectural unknown in the Swift layer (⌘Z routing) settled empirically before any
  production code depends on it — the spike caught a real, design-changing failure.
- Full on-disk schema contract (Appendix A) in place for SP-052/SP-054 implementation.
- I-0019 closed as OBE/superseded (user-approved) — requirement carried by EP-019 AC1.

**What went well:**
- Spike-before-build paid off immediately: the original design mechanism was wrong, and finding
  that cost one throwaway file edit instead of a mid-sprint rework of `HistoryCapture`.
- Two-round spike turnaround (fail → fallback → confirm) inside a single day.

**What to improve:**
- Round 1's menu observation was partially confounded by focus (menu consults the first
  responder) — future UI spikes should script the observation steps to pin focus state first.

**Carry-forward notes:**
- **T-0205 must use the confirmed mechanism:** first-responder `undo(_:)`/`redo(_:)` +
  `validateUserInterfaceItem`; `allowsUndo = false`; never override `undoManager` and never
  subclass `UndoManager` for the text view.
- Dev-signed builds spam benign `CSInlineDonation`/`SetStoreUpdateService` 4099 errors (known
  T-0182 environment limitation; verified working on signed builds in T-0189) — ignore in future
  console observations.
- Next sprint: SP-052 (linear C++ `HistoryService` core + C ABI + Swift wrappers), in Planning.

---

*Closed 2026-07-06 with user approval. All exit criteria met; T-0198/T-0200 verified, T-0199 done.*
