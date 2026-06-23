# T-0172: Zoom in resolves clusters into individual dots

**Status:** ✅ Implemented - Verified
**Component:** `TimelineStripView.swift`
**Priority:** Medium
**Epic:** EP-016
**Sprint:** SP-043
**Date Implemented:** 2026-06-16
**Date Verified:** 2026-06-18

**Description:**
The timeline supports pinch-to-zoom (trackpad) and horizontal pan, so that as the writer
zooms in, co-located dots spread apart and clusters resolve into individual dots that can be
independently interacted with.

**Design Reference:** FR-009, FR-032, NFR-007

**Implementation:**
- `@State zoomFactor` (base zoom), `@GestureState magnifyGestureScale` (live pinch),
  `@State scrollOffsetFraction` (pan), and `effectiveZoom` combining them.
- `visibleSpanMs()` / `visibleMinMs()` drive all coordinate math (`dotX`, `eventX`,
  `offsetMs(fromPanelX:)`).
- `MagnifyGesture` for pinch; `TimelineScrollCaptureView` (NSViewRepresentable + local
  NSEvent monitor) for trackpad pan and ⌘/vertical scroll-to-zoom.
- Visible window centre preserved during zoom.
- `buildClusters` uses zoomed coordinates, so clusters dissolve/reform naturally with zoom.

**Note on max zoom:** the original fixed `maxZoom = 50` was replaced during verification with a
data-driven cap (see I-0046) so year-spanning timelines can fully resolve a 24-hour cluster.

**Files Affected:**
- `Scrivi/Views/TimelineStripView.swift`

**Acceptance Criteria (all verified):**
- [x] Pinch outward zooms in; clusters resolve into individual dots at sufficient zoom.
- [x] Pinch inward zooms out; dots re-cluster when within one diameter.
- [x] Horizontal trackpad scroll pans the visible window when zoomed in.
- [x] Dot interactions (hover, drag, context menu) work correctly at any zoom level.
- [x] Zoom does not alter underlying `offsetMs` values — purely a display transform.
- [x] All dots return to clustered view when zoom is returned to 1.0.

**Verification note:** Verified working as designed against `the-twisted-remains-of-myself`.
