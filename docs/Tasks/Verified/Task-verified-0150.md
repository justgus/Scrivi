# T-0150: Scene Dot Drag — Horizontal Gesture, Story-Time Update

**Status:** ✅ Implemented - Verified
**Component:** `TimelineStripView.swift` — `SceneDotView`
**Priority:** High
**Epic:** EP-016
**Sprint:** SP-040
**Date Requested:** 2026-06-11
**Date Implemented:** 2026-06-11
**Date Verified:** 2026-06-11

**Summary:**
Scene dots respond to horizontal drag via `.simultaneousGesture(DragGesture(...))`. On drag end, `finalPanelX = startX + v.translation.width` is computed in panel coordinates (I-0025 fix) and `computeOffsetMs(finalPanelX)` converts it to a story-time offset. `pendingOffsetMs` is set and `showPicker = true` fires, presenting the Time Delta Picker. Drag does not affect manuscript order. The dot animates to 1.3× scale during drag and returns to 1.0× on release.
