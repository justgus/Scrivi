# Task Backlog

Tasks listed here are documented and ready for Sprint assignment. All items are 🔵 Backlog.

---

## Future — Manuscript Scroll Bar Fidelity

| Task | Title | Status |
| ---- | ----- | ------ |
| T-0096 | Per-scene character-ratio position and size for full-manuscript scroll bar fidelity | 🔵 Backlog |

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

**Deferred because:**
SP-025 / EP-009 must close first. Scroll bar fidelity is a polish feature, not a correctness requirement for the writing surface.

---

*Last Updated: 2026-06-03*
