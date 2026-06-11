# Verified Issues — I-0021 to I-0030

---

## I-0021 through I-0024

See: [Issue-verified-0011-0020.md](Issue-verified-0011-0020.md) — these issues were archived in batch 2.

---

## I-0025: Timeline dot drag moves at half the expected distance

**Status:** ✅ Resolved - Verified
**Platform:** macOS
**Component:** `TimelineStripView.swift` — `SceneDotView`
**Severity:** High
**Sprint:** SP-040
**Date Identified:** 2026-06-11
**Date Verified:** 2026-06-11

**Description:**
When dragging a scene dot on the timeline, the dot visually trailed the mouse pointer at approximately half the distance. On release, the dot was placed at a position far from where the pointer was.

**Root Cause Analysis:**
Three compounding coordinate-space bugs caused by a `GeometryReader` inside `SceneDotView`: (A) `geo.frame(in: .global).width` returned the dot's own frame width (~22pt) instead of the panel width, producing `usable = -10`; (B) `localCenter ≈ 11pt` anchored `finalX` near zero regardless of dot position; (C) `.offset` inside a `GeometryReader` layered under `.position` in the parent `ZStack` caused halved visual motion via AppKit transform compositing.

**Resolution:**
Removed the inner `GeometryReader` from `SceneDotView`. Parent now passes `startX` (panel coordinates) and `computeOffsetMs` closure directly. Drag computes `finalPanelX = startX + v.translation.width`. Visual `.offset(x: dragOffsetX)` applied to the `ZStack` directly, same transform layer as `.position`.

**Files Affected:** `Scrivi/Views/TimelineStripView.swift`

---

## I-0026: Time Delta Picker popover appears empty and unsized

**Status:** ✅ Resolved - Verified
**Platform:** macOS
**Component:** `TimelineStripView.swift` — `SceneDotView`, `TimeDeltaPicker`
**Severity:** High
**Sprint:** SP-040
**Date Identified:** 2026-06-11
**Date Verified:** 2026-06-11

**Description:**
After releasing a scene dot drag, a small empty circular popover appeared near the timeline with no content.

**Root Cause Analysis:**
(1) `.popover` attached to a `GeometryReader` gave macOS `NSPopover` an indeterminate `sourceRect`, rendering at minimum chrome size. (2) `if let pendingDotID` inside the popover closure could silently fail before state propagated, producing a zero-size content view. (3) `.frame(width: 360)` was outside the `if let`, leaving no content to constrain.

**Resolution:**
Moved all popover presentation into `SceneDotView`. Both `.popover` modifiers now anchor on the concrete filled `Circle()`. `pendingOffsetMs` is set on the same dot that owns the popover — no optional unwrap, no race.

**Files Affected:** `Scrivi/Views/TimelineStripView.swift`

---

## I-0027: Timeline positions and picker display relative to epoch instead of previous scene end

**Status:** ✅ Resolved - Verified
**Platform:** macOS
**Component:** `TimelineStripView.swift` — `SceneDotView.storyTimeDescription`, `TimeDeltaPicker`
**Severity:** High
**Sprint:** SP-040
**Date Identified:** 2026-06-11
**Date Verified:** 2026-06-11

**Description:**
Scene tooltips showed "X time after Story Open" instead of "X time after [previous scene] ends." Picker spinner and anchors used the previous scene's start as the base instead of its end.

**Root Cause Analysis:**
`previousSceneOffsetMs` (start) was passed everywhere instead of `previousSceneEndMs` (= `offsetMs + durationMs`). The tooltip called `humanReadableDuration(ms: dot.offsetMs, ...)` — an absolute epoch offset — instead of the gap from the previous scene's end.

**Resolution:**
Introduced `previousSceneEndMs` computed at the `ForEach` call site in `TimelineStripView`. Tooltip computes `gap = dot.offsetMs - previousSceneEndMs`. Spinner initialises from that gap. All anchor and spinner arithmetic uses `previousSceneEndMs` as the base.

**Files Affected:** `Scrivi/Views/TimelineStripView.swift`

---

## I-0028: Timeline dot drag requires right-click before left-click drag will work

**Status:** ✅ Resolved - Verified
**Platform:** macOS
**Component:** `TimelineStripView.swift` — `SceneDotView`
**Severity:** High
**Sprint:** SP-040
**Date Identified:** 2026-06-11
**Date Verified:** 2026-06-11

**Description:**
Scene dots could not be dragged with a left-click drag on first attempt. The drag only worked after a right-click elsewhere in the panel.

**Root Cause Analysis:**
`.popover(isPresented:)` on the inner `Circle()` installed a hit-test interceptor that consumed the initial mouse-down before the `DragGesture` on the outer `ZStack` could receive it. Right-click resolved the interceptor's pending state via a different AppKit path, after which drag worked.

**Resolution:**
Changed `.gesture(DragGesture(...))` to `.simultaneousGesture(DragGesture(...))`. Moved both `.popover` modifiers from the inner `Circle()` to the outer `ZStack` so they no longer intercept events in the gesture's path.

**Files Affected:** `Scrivi/Views/TimelineStripView.swift`

---

## I-0029: Right-click on scene dot shows no context menu

**Status:** ✅ Resolved - Verified
**Platform:** macOS
**Component:** `TimelineStripView.swift` — `SceneDotView`
**Severity:** Medium
**Sprint:** SP-040
**Date Identified:** 2026-06-11
**Date Verified:** 2026-06-11

**Description:**
Right-clicking a scene dot produced no context menu.

**Root Cause Analysis:**
No `.contextMenu` modifier had been added to `SceneDotView`. The context menu was specified in the design doc but not implemented.

**Resolution:**
Added `.contextMenu { ... }` to the `ZStack` in `SceneDotView`. "Set Time Delta…" is fully implemented. "View Scene," "Assign to Act…," and "Unassign from Act" are present but disabled (pending SP-041 band assignment implementation).

**Files Affected:** `Scrivi/Views/TimelineStripView.swift`

---

## I-0030: Manually-positioned scene does not maintain relative gap when predecessor moves

**Status:** ✅ Resolved - Verified
**Platform:** macOS
**Component:** `TimelineStripView.swift` — `TimelineViewModel`, `SceneMetaJson`, C++ schema
**Severity:** High
**Sprint:** SP-040
**Date Identified:** 2026-06-11
**Date Verified:** 2026-06-11

**Description:**
Moving Scene 3 caused Scene 4 (set to "2 hours after Scene 3") to show "15 hours after Scene 3" instead of maintaining the 2-hour gap.

**Root Cause Analysis:**
Absolute `offsetMs` was stored as canonical for manual scenes. When a predecessor moved, `propagateChain` stopped at the first manual scene. The stored absolute position became stale but the gap the writer intended was not persisted anywhere.

**Resolution:**
Added `gapMs` to `SceneStoryTime` as the canonical stored value (gap from `previousSceneEnd` to this scene's start). `offsetMs` is now derived for all scenes via `recomputeAllOffsets`. `recomputeAndPersistFrom` walks the entire chain from the changed scene forward, recomputing and persisting absolute positions for every subsequent scene regardless of `offsetSource`. The picker captures and stores `gapMs = targetOffsetMs - previousSceneEndMs`.

**Files Affected:**
- `ScriviCore/src/schemas/SceneMetaJson.hpp/.cpp`
- `ScriviCore/include/scrivi/Results.hpp`
- `ScriviCore/include/scrivi/Requests.hpp`
- `ScriviCore/src/public_api/ScriviCore.cpp`
- `ScriviCore/src/public_api/scrivi_c_api.cpp`
- `Scrivi/Engine/ScriviEngine.swift`
- `Scrivi/Views/TimelineStripView.swift`
