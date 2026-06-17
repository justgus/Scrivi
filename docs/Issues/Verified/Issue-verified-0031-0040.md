# Verified Issues — I-0031 to I-0040

---

## I-0031: Story Structure bands not rendered after selecting a structure

**Status:** ✅ Resolved - Verified
**Platform:** macOS
**Component:** `TimelineStripView.swift` — `TimelineViewModel`
**Severity:** High
**Sprint:** SP-041

**Description:**
Selecting a Story Structure from the `[Structure ▾]` menu had no visible effect. No colored bands appeared in the timeline panel and no label row was shown.

**Root Cause Analysis:**
The C++ backend wraps the band array under a `"bands"` key in both directions (`{"bands":[...]}`). Swift was generating a plain array `[...]` outbound and trying to decode a plain array inbound — both silently failed, leaving `activeBands` empty and suppressing all rendering.

**Implementation:**
1. `applyStructure` passes `bandLayoutJSON: ""` — C++ populates from its own built-in table.
2. `decodeBandLayout` unwraps the `"bands"` key before decoding.
3. `encodeBandLayout` wraps output as `{"bands":[...]}`.

**Files Affected:**
- `Scrivi/Views/TimelineStripView.swift`

**Fix Date:** 2026-06-12
**Verification Date:** 2026-06-12

---

## I-0032: Band border cursor does not change on hover

**Status:** ✅ Resolved - Verified
**Platform:** macOS
**Component:** `TimelineStripView.swift` — `BandOverlayView`
**Severity:** Low
**Sprint:** SP-041

**Root Cause:** No cursor-change logic on the band overlay. `Canvas` does not participate in `onHover`.
**Fix:** Added `.onContinuousHover` to `BandOverlayView` — pushes `NSCursor.resizeLeftRight` when within 8pt of a border, pops on exit.
**Fix Date:** 2026-06-12 | **Verification Date:** 2026-06-12

---

## I-0033: Scene dot drag blocked by tooltip popover consuming mouse-down

**Status:** ✅ Resolved - Verified
**Platform:** macOS
**Component:** `TimelineStripView.swift` — `SceneDotView`
**Severity:** High
**Sprint:** SP-041

**Root Cause:** The tooltip was implemented as a `.popover` (NSPanel). The first left-click on the dot was consumed by the popover's click-outside-to-dismiss handler and never forwarded to the dot's gesture recognizer.
**Fix:** Replaced the `.popover` tooltip with a `DotTooltipView` non-interactive overlay rendered outside the clipped ZStack on the GeometryReader. `allowsHitTesting(false)` ensures no click is ever consumed by the tooltip. A `DragGesture(minimumDistance:0)` probe clears hover state on mouse-down so the overlay dismisses automatically on click.
**Fix Date:** 2026-06-12 | **Verification Date:** 2026-06-12

---

## I-0034: Drag-up on assigned dot should toggle assignment off regardless of release position

**Status:** ✅ Resolved - Verified
**Platform:** macOS
**Component:** `TimelineStripView.swift` — `SceneDotView.combinedDragGesture`
**Severity:** Medium
**Sprint:** SP-041

**Root Cause:** The assign branch in `onEnded` ran when `hoveredBandID != nil`, even for already-assigned dots, causing a re-assignment instead of unassignment.
**Fix:** Reordered `onEnded` logic — if `isDraggingUp && !dot.bandID.isEmpty`, always unassign (toggle). The assign-on-release path only fires for unassigned dots.
**Fix Date:** 2026-06-12 | **Verification Date:** 2026-06-12

---

## I-0035: No checkmark on currently assigned band in context menu

**Status:** ✅ Resolved - Verified
**Platform:** macOS
**Component:** `TimelineStripView.swift` — `SceneDotView` context menu
**Severity:** Medium
**Sprint:** SP-041

**Root Cause:** `Label(..., systemImage: "checkmark")` inside a `Menu` button does not render a native macOS menu checkmark — macOS suppresses the SF Symbol icon in menu rendering.
**Fix:** Replaced with a `Picker(.inline)` bound to `dot.bandID`. SwiftUI renders native macOS checkmarks automatically for inline pickers in context menus.
**Fix Date:** 2026-06-12 | **Verification Date:** 2026-06-12

---

## I-0036: Color.clear overlay blocks all timeline interaction; historical event placed at midpoint, not click location; no title-entry popup on creation

**Status:** ✅ Resolved - Verified
**Platform:** macOS
**Component:** `TimelineStripView.swift` — `TimelineStripView.body`, `TimelineViewModel.createHistoricalEvent`
**Severity:** Critical
**Sprint:** SP-042

**Description:**
Three related defects that together made the timeline panel non-interactive:
1. A `Color.clear.contentShape(Rectangle())` placed as the last (topmost) ZStack child consumed every mouse event across the entire panel — no dot or band border ever received an event.
2. "New Historical Event Here" placed the new event at the timeline midpoint regardless of click location.
3. No title-entry popup appeared after creating a historical event; it was silently named "New Historical Event".

**Secondary defect (during verification):** After the initial fix, the empty-area context menu no longer fired on right-click in blank space — blank space had no hit-testable surface.

**Root Cause Analysis:**
`Color.clear.contentShape(Rectangle())` as the last ZStack child made the entire panel hit-testable by that view only; all child views were unreachable. Midpoint placement was a never-updated placeholder. Title popup was never implemented.

**Implementation:**
1. Removed the topmost `Color.clear` overlay.
2. Moved the empty-area context menu to the ZStack itself; child views' own context menus take priority.
3. Added `.onContinuousHover` to track the last mouse X into `contextMenuClickX`.
4. "New Historical Event Here" computes `offsetMs` from `contextMenuClickX` via `offsetMs(fromPanelX:usable:)`.
5. `HistoricalEventEditorSheet` is presented immediately after creation.
6. Deleted three stale `hevent_*.json` files from the test project.
7. (Secondary fix) Added `Color.clear.contentShape(Rectangle())` as the **first** (bottom) ZStack child, giving blank space a hit-testable surface while child views above still win direct hit-testing.

**Files Affected:**
- `Scrivi/Views/TimelineStripView.swift` — `TimelineStripView.body`

**Fix Date:** 2026-06-13 | **Verification Date:** 2026-06-16

**Related Issues:** I-0033 (same ZStack hit-testing class of problem)

---

## I-0037: Hover tooltips for historical event and imported event dots display in wrong position and show wrong content

**Status:** ✅ Resolved - Verified
**Platform:** macOS
**Component:** `TimelineStripView.swift` — `HistoricalEventDotView`, `ImportedEventDotView`, `SimpleTooltipView`
**Severity:** High
**Sprint:** SP-042

**Description:**
1. Historical event dot tooltip was clipped above the panel — only the drop shadow was visible.
2. All imported event dots showed the same tooltip text (the source timeline name) regardless of which dot was hovered.

**Root Cause Analysis:**
- **RC-1:** `HistoricalEventDotView` rendered its tooltip as a local `.overlay` offset upward from a dot inside a `.clipped()` ZStack, so it was clipped by the ZStack frame. Same class of problem as scene dot tooltips — must render via panel-level overlay outside `.clipped()`.
- **RC-2:** `@State var isHovered` on `ImportedEventDotView` did not maintain stable per-instance state when placed with `.position()` inside a `ForEach` inside a clipped ZStack; SwiftUI conflated the state across instances, so the hovered state propagated to all dots and all showed the identical `sourceName`.

**Implementation:**
1. Restored `hoveredHistoricalEventID: String?` to `TimelineViewModel`; historical event tooltip rendered in the panel-level overlay using `lineY` as the Y anchor.
2. Removed `@State var isHovered` from `ImportedEventDotView`; hovered imported event tracked by key (`"timelineID:eventID"`) on the view model and rendered in the panel-level overlay — eliminating the state-identity problem.

**Files Affected:**
- `Scrivi/Views/TimelineStripView.swift`

**Fix Date:** 2026-06-14 | **Verification Date:** 2026-06-16

**Related Issues:** I-0036

---

## I-0038: New scenes created via Cmd-Enter do not appear as dots on the Timeline

**Status:** ✅ Resolved - Verified
**Platform:** macOS
**Component:** `ViewportSceneLoader.swift`, `AppEnvironment.swift`, `TimelineStripView.swift` — `TimelineViewModel`
**Severity:** High
**Sprint:** SP-042
**Epic:** EP-016

**Description:**
Creating a new scene (Cmd-Enter) or chapter with first scene (Shift-Cmd-Enter) wrote the scene to disk and the Navigator but added no dot to the Timeline. The Timeline only reflected the scene list present at project open until close/reopen.

**Root Cause Analysis:**
`TimelineViewModel` was populated once in `AppEnvironment.openProject()`. `createScene`/`createChapter` from `ManuscriptTextView` never updated `TimelineViewModel.dots` — there was no callback path from scene creation to the timeline model.

**Implementation:**
1. Added `reloadSceneDots(engine:projectRootPath:scenes:)` to `TimelineViewModel` — rebuilds only the dot array, preserving band assignments, historical events, imported timelines, story structure.
2. After every successful `createScene`, `createChapter`, scene split, and chapter split in `ManuscriptTextView.Coordinator`, `env.timelineModel?.reloadSceneDots(...)` is called with `loader.allScenes`.

**Files Affected:**
- `Scrivi/Views/TimelineStripView.swift` — `TimelineViewModel.reloadSceneDots`
- `Scrivi/Views/ManuscriptTextView.swift` — Coordinator create/split handlers

**Fix Date:** 2026-06-14 | **Verification Date:** 2026-06-16

**Related Issues:** I-0036, I-0037

---

## I-0039: Clustering only applies to unanchored dots; anchored scene dots with overlapping positions are not grouped

**Status:** ✅ Resolved - Verified
**Platform:** macOS
**Component:** `TimelineStripView.swift` — `buildClusters`, `TimelineStripView.body`
**Severity:** High
**Sprint:** SP-042

**Description:**
Only unanchored ("default" `offsetSource`) dots landing at the same default position were grouped. Anchored dots co-located on screen overlapped without grouping, creating a visual pile-up on the timeline line.

**Implementation:**
Introduced a `MainRowItem` enum wrapping either a `SceneDot` or a `HistoricalEventDot`. `buildClusters` now operates over all main-row items sorted by X position before grouping, so transitive proximity chains are caught regardless of source array order. Both dot types share a single cluster loop; the rendering loop switches on `MainRowItem` to call `SceneDotView` or `HistoricalEventDotView`. (Also fixes I-0040.)

**Files Affected:**
- `Scrivi/Views/TimelineStripView.swift` — `MainRowItem` enum, `DotCluster`, `buildClusters`, `itemX`, main-row rendering loop

**Fix Date:** 2026-06-16 | **Verification Date:** 2026-06-16

**Related Issues:** I-0037, I-0040

---

## I-0040: Historical event dots on the main timeline are not clustered when co-located

**Status:** ✅ Resolved - Verified
**Platform:** macOS
**Component:** `TimelineStripView.swift` — `TimelineStripView.body`, historical event dot rendering
**Severity:** High
**Sprint:** SP-042

**Description:**
Historical event dots on the main row were rendered via a `ForEach` over `model.historicalEvents` with no clustering pass. Co-located historical event dots (or a historical event dot and a scene dot) overlapped, hiding all but the topmost.

**Implementation:**
Fixed as part of I-0039. Historical event dots are now included in the unified `buildClusters` pass via `MainRowItem.historical`. The separate unclustered `ForEach` over `model.historicalEvents` was removed; historical event dots render inside the same cluster loop as scene dots using the `clusterOffset` ring layout.

**Files Affected:**
- `Scrivi/Views/TimelineStripView.swift` — unified main-row rendering loop; separate historical event `ForEach` removed

**Fix Date:** 2026-06-16 | **Verification Date:** 2026-06-16

**Related Issues:** I-0039, I-0037

---
