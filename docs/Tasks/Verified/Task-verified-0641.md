# Verified Task — T-0641

**Sprint:** [SP-199] · **Epic:** [EP-055] · **Platform:** `[Cross]`
✅ **USER-VERIFIED 2026-11-02** (iPad and Mac live passes (user, 2026-11-01) and [T-0652] passed on the Linux VM (the live-pass checklist: Pass))
**Archived:** 2026-11-02, on the user's authority (user, 2026-11-02: *"on my authority, closed all the objects that have now tested completed"*).

---

## T-0641 — `[Cross]` A time scale along the top of the Timeline

**Epic:** [EP-055] · **Status:** ✅ **Verified 2026-11-02** (iPad and Mac live passes (user, 2026-11-01) and [T-0652] passed on the Linux VM (the live-pass checklist: Pass)) · **Sprint:** [SP-199] (🟡 Active) → [`../Sprints/Sprint-SP-199.md`](../../Sprints/Sprint-SP-199.md) · **Date Requested:** 2026-10-31

**What (user):** ticks for decades, years, months, weeks, days, hours as the zoom allows, as a reference for where a scene
sits in time and as a sign of movement while scrolling across empty stretches. **Ruling (2026-10-31):** labels are offsets
from Story Open (no calendar, Timeline design principle 5).
**Done (Apple):** `TimelineScale` (`TimelineStripView.swift:3712`) picks the smallest step from 1 min to 1,000 years that keeps
labels 90 pt apart and labels each tick "Day 3 14:00", "Day 3", "Week 7", "Month 15" or "Year 2", counted from 1 at Story Open
(month = 30 days, year = 365, the Time Delta Picker's units). Drawn as a 16 pt row under the header (`:1840`), in the dots' x
mapping. Checks: iOS 17/17 (`TimelineScaleTests` 3 new), Mac interop 327/327, Mac build, `check-layout-convergence.sh` clean.
**Linux (2026-11-01):** `TimelinePanel::scaleTicks` / `scaleLabel` (`platforms/linux/src/TimelinePanel.cpp:729`), the same steps
and labels as Apple, painted as a 16 px row along the top of the strip (`:948`), above the bands; the strip's minimum height
80 → 96. Checks: `timeline_cluster_smoke` (the cases of Apple's `TimelineScaleTests`), Linux smokes 38/38 (Docker, Qt 6.4,
offscreen, non-root). ⏳ Rig live pass.
