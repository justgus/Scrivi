# T-0152: Time Delta Picker — Spinner Pre-Population; Anchor Deltas Relative to Previous Scene End

**Status:** ✅ Implemented - Verified
**Component:** `TimelineStripView.swift` — `TimeDeltaPicker.initialiseSpinner`, `bestFit`
**Priority:** High
**Epic:** EP-016
**Sprint:** SP-040
**Date Requested:** 2026-06-11
**Date Implemented:** 2026-06-11
**Date Verified:** 2026-06-11

**Summary:**
`initialiseSpinner()` seeds both the position spinner and the duration spinner on `.onAppear`. Position spinner delta = `abs(rawOffsetMs - previousSceneEndMs)`; direction initialises to Later/Before based on sign. `bestFit(ms:)` selects the largest natural unit (years → months → weeks → days → hours → minutes). Duration spinner seeds from `currentDurationMs`. All anchor and spinner arithmetic uses `previousSceneEndMs` as the base, not the drag-computed absolute position (I-0027 fix).
