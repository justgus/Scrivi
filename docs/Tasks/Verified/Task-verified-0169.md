# T-0169: Hover tooltips for historical event and imported event dots

**Status:** ✅ Implemented - Verified
**Component:** `Scrivi/Views/TimelineStripView.swift`
**Epic:** EP-016 (Timeline)
**Sprint:** SP-042
**Date Verified:** 2026-07-01

**Goal:**
Hovering a timeline dot (scene, historical event, or imported event) shows a tooltip with that
item's details.

**Resolution (reconstructed from code — no separate task detail survived; user verified the feature
correct 2026-07-01):**
- Hover state is tracked on the model — `hoveredDotID`, `hoveredHistoricalEventID`,
  `hoveredImportedEventKey` — set by child dot views and read by `TimelineStripView` to render
  tooltips (`TimelineStripView.swift:206-210`).
- Tooltips are rendered **outside** `.clipped()` so they are never cut off at the panel edge
  (`TimelineStripView.swift:889`): scene dots use `DotTooltipView` (`:898`, def `:2484`); historical
  and imported event dots use `SimpleTooltipView` (`:916`, `:937`, def `:2447`).
- Imported-event hover is propagated via `onHoverChanged` → `model.hoveredImportedEventKey`
  (`TimelineStripView.swift:834-835`); a continuous-hover handler drives positioning
  (`.onContinuousHover`, `:859`).
- Note: co-located dots collapse into an aggregate whose hover opens a popover circle-of-dots
  (T-0166 / T-0174) rather than a plain tooltip; the plain tooltips here cover single (non-clustered)
  dots.

**Verification:**
- User verified the hover-tooltip behavior for historical and imported event dots as correct on
  2026-07-01.

**Acceptance Criteria:**
- [x] Hovering a historical event dot shows a tooltip with its details.
- [x] Hovering an imported event dot shows a tooltip with its details.
- [x] Tooltips are not clipped by the panel bounds.

*Archived 2026-07-01. Detail reconstructed from source (`TimelineStripView.swift`) as the original
task entry did not survive; feature user-verified.*
