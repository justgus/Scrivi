# Task Backlog

Tasks listed here are documented and ready for Sprint assignment. All items are 🔵 Backlog.

---

## Future — Manuscript Scroll Bar Fidelity

| Task | Title | Status |
| ---- | ----- | ------ |
| T-0096 | Per-scene character-ratio position and size for full-manuscript scroll bar fidelity | 🔵 Backlog |
| T-0112 | Global manuscript cursor position tracking + scene start position map | 🔵 Backlog |

---

## T-0096: Per-scene character-ratio position and size for full-manuscript scroll bar fidelity

**Status:** 🔵 Backlog
**Component:** `ViewportSceneLoader`, scroll bar overlay (future)
**Priority:** Medium
**Date Requested:** 2026-06-03
**Sprint Assigned:** Not Assigned

**Rationale:**
The current `NSScrollView` scroll bar thumb reflects only the loaded text buffer, not the author's position in the full manuscript. An author 60% through a 200-scene novel sees the thumb near the top because only a few scenes are loaded. The scroll bar should communicate the author's true position in the manuscript.

**Desired Behavior:**
The scroll bar thumb position and size reflect the scene's proportional position and size within the entire manuscript, not just within the loaded viewport buffer.

**Design Notes:**
- **Size ratio:** `scene.charCount / manuscript.totalCharCount` — gives a rough proportional height for the scene. Character count includes newlines counted as 1.
- **Position offset:** sum of size ratios of all preceding scenes — gives the scene's fractional offset from the top of the manuscript (0.0 to 1.0).
- **Storage:** In-memory on `ViewportSceneLoader` (or a companion struct). Persisting to `scene.meta.json` is worth considering since completed scenes rarely change size, which avoids recomputation on every launch. However, this requires a write on every save and schema versioning.
- **Unloaded scenes:** Character counts for scenes not in the viewport buffer must come from stored metadata (`wordCount`/`characterCount` fields already written to `scene.meta.json` by `SceneWriter`). These can be used as a proxy without loading scene content.
- **Limitation:** Character count ≠ rendered height (font size, window width, and line wrapping affect actual glyph height). This ratio is an approximation. A rendered-height approach using `NSLayoutManager` is more accurate but only works for loaded scenes; unloaded scenes would still require estimation.
- **Open question:** Whether to implement a custom scroll bar overlay (drawing the thumb at the computed position) or hook into `NSScroller` — to be resolved at implementation time.

**Dependencies:**
- `scene.meta.json` already stores `wordCount` and `characterCount` (written by `SceneWriter`) — these can serve as the per-scene size source without additional schema changes.
- Full-manuscript total requires summing `characterCount` across all scenes in `allScenes` at open time.
- T-0112 (cursor position tracking) is a natural companion — both require knowing where each scene begins in the full text storage.

**Deferred because:**
SP-025 / EP-009 must close first. Scroll bar fidelity is a polish feature, not a correctness requirement for the writing surface.

---

## T-0112: Global manuscript cursor position tracking + scene start position map

**Status:** 🔵 Backlog
**Component:** `ViewportSceneLoader`, `ManuscriptTextView.swift`
**Priority:** Medium
**Date Requested:** 2026-06-05
**Sprint Assigned:** Not Assigned

**Rationale:**
Cursor placement after structural operations (scene delete, scene navigation) must be precise and predictable. Currently there is no global record of where the cursor is in the manuscript or where each scene begins in the full `NSTextStorage`. This gap causes imprecise cursor placement after delete and makes scroll bar fidelity (T-0096) harder to implement correctly.

**Current Behavior:**
No global cursor position is tracked. Scene boundary positions (`sceneBoundaries`) are computed locally in `ManuscriptTextView` for the currently loaded buffer, but are not available to the rest of the app. After a scene is deleted, cursor placement is undefined.

**Desired Behavior:**
- The app maintains a global record of the current manuscript cursor position (character offset into the full `NSTextStorage`).
- A scene start position map is maintained — for each `sceneID`, the character offset of the first character of that scene in the full text storage.
- After deleting a scene:
  - If the deleted scene was not the last scene: cursor is placed at the first character of the next scene (the character immediately following the divider after the deleted scene's position).
  - If the deleted scene was the last scene: cursor is placed at the end of the manuscript.
  - In both cases, keyboard focus transfers to `ManuscriptNSTextView` so the writer can type immediately without a second click.

**Design Notes:**
- Scene start positions are derivable from `sceneBoundaries` in `ManuscriptTextView.Coordinator` — these should be surfaced to `ViewportSceneLoader` or a shared observable so the rest of the app can read them.
- Global cursor position can be tracked via `textViewDidChangeSelection` in the `NSTextViewDelegate`.
- Both values should update on every text storage rebuild (`rebuildStorage`) and on every selection change.
- This feature is a prerequisite for correct delete-of-open-scene cursor behavior (T-0110) and a natural enabler for scroll bar fidelity (T-0096).

**Dependencies:**
- T-0096 (scroll bar fidelity) is a natural consumer of scene start positions.
- T-0110 (delete-of-open-scene edge case) depends on cursor placement being correct after delete — this task provides the mechanism.

---

*Last Updated: 2026-06-06 (SP-032/EP-010 closed; T-0096/T-0112 remain — candidates for EP-011)*
