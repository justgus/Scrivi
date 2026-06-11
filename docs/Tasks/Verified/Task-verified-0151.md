# T-0151: Time Delta Picker — Amount/Unit/Direction Spinner, Named Anchors, Dismiss Behavior

**Status:** ✅ Implemented - Verified
**Component:** `TimelineStripView.swift` — `TimeDeltaPicker`
**Priority:** High
**Epic:** EP-016
**Sprint:** SP-040
**Date Requested:** 2026-06-11
**Date Implemented:** 2026-06-11
**Date Verified:** 2026-06-11

**Summary:**
`TimeDeltaPicker` presents a spinner row (`[amount] [Unit▾] [Later/Before▾] [Set]`), a duration row (`Scene duration: [amount] [Unit▾]`), six named time-of-day anchor buttons ("that morning," "that evening," "that night," "before dawn," "around 3am," "the next morning"), and a footer with "Immediately after," "Keep position," and "Reset." All position computations use `previousSceneEndMs` as the base (I-0027 fix). Anchors add their fixed deltas to `previousSceneEndMs`. "Immediately after" triggers `resetDefault`. "Keep position" accepts the raw drag offset. Dismissing without selection = keep position (SwiftUI default popover dismiss behaviour). Result and chosen duration are passed back as `(TimeDeltaPickerResult, Int64)`.
