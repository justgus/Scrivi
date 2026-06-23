# T-0171: Cluster count badge when ring height exceeds panel height

**Status:** ✅ Implemented - Verified
**Component:** `TimelineStripView.swift`
**Priority:** Medium
**Epic:** EP-016
**Sprint:** SP-043
**Date Implemented:** 2026-06-16
**Date Verified:** 2026-06-18

**Description:**
When a dot cluster's ring stack is taller than the available space above the timeline line
(from the line up to the label row or the top of the content area), a count badge appears on
the cluster centre dot showing the total number of events in the cluster.

**Design Reference:** FR-035

**Implementation:**
The badge renders as a white number on an accent-coloured capsule, positioned above the
tallest ring, and is `allowsHitTesting(false)` so it does not interfere with dot interaction.
Identical logic is applied to clusters on imported timeline rows.

The condition is:
```
tallest = ringCount * (dotDiameter + 4)
if tallest > lineY - (activeBands.isEmpty ? 0 : labelRowHeight)  →  show badge
```

**Files Affected:**
- `Scrivi/Views/TimelineStripView.swift` — main-row badge, imported-row badge

**Acceptance Criteria (all verified):**
- [x] Count badge appears on cluster centre when ring stack height > available panel space above line.
- [x] Badge shows the total count (not ring count).
- [x] Badge is non-interactive (does not block dot hover/drag).
- [x] Badge disappears when panel is tall enough to show all rings.
- [x] Badge disappears when zoom level resolves the cluster into individual dots.

**Verification note:** Verified working as designed against `the-twisted-remains-of-myself`.
The visual legibility of the multi-ring cluster itself is tracked separately as a follow-up
design effort (the two-ring "blob" is hard to read) — see T-0174.
