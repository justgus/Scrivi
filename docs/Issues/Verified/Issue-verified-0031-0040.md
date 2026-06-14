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
