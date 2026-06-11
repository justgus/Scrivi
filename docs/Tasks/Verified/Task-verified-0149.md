# T-0149: TimelineStripView Rebuild — Horizontal Line, Scene Dots, Manuscript-Order Layout

**Status:** ✅ Implemented - Verified
**Component:** `TimelineStripView.swift`, `TimelineViewModel`, `AppEnvironment.swift`, `EditorView.swift`
**Priority:** High
**Epic:** EP-016
**Sprint:** SP-040
**Date Requested:** 2026-06-11
**Date Implemented:** 2026-06-11
**Date Verified:** 2026-06-11

**Summary:**
Replaced the static placeholder `TimelineStripView` with a fully functional view driven by `TimelineViewModel`. `TimelineViewModel` loads scene data from ScriviCore on project open, computes dot positions from `gapMs` and `durationMs` via `recomputeAllOffsets`, and exposes `offsetMs`, `spanMs`, `minOffsetMs`, and `maxEndMs` for layout. Scene dots appear on a horizontal line in manuscript order, spaced proportionally to their story-time positions. Empty state shows "Scenes will appear here as you write." `AppEnvironment` creates and clears `timelineModel` alongside `viewportLoader`. `EditorView` passes the model, engine, and projectRootPath to `TimelineStripView`.
