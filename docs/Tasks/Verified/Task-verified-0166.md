# T-0166: Co-located dot clustering — hexagonal ring layout, count badge

**Status:** ✅ Implemented - Verified
**Component:** `Scrivi/Views/TimelineStripView.swift`
**Epic:** EP-016 (Timeline)
**Sprint:** SP-042
**Date Verified:** 2026-07-01

**Goal:**
When multiple timeline dots fall on (nearly) the same date at the current zoom, collapse the
co-located group into a single dot with a count and a ring/circle layout, so overlapping dots no
longer clip or hide each other.

**Resolution (reconstructed from code — no separate task detail survived; user verified the feature
correct 2026-07-01):**
- Clustering is built by `buildClusters(usable:panelW:)` producing `DotCluster` groups
  (`TimelineStripView.swift:1375`, `:1387`); a co-located group of ≥2 members renders as a single
  `AggregateDotView` (`:1758`, `:1763`) rather than overlapping individual dots. Imported-row
  clustering has its parallel `ImportedRowCluster` / `ImportedAggregateDotView` (`:1439`, `:1954`).
- Panel height auto-grows to fit the tallest cluster's stacked ring so clusters never clip
  (`requiredClusterHeight`, `tallestClusterStack(usable:panelW:)`, `TimelineStripView.swift:668`,
  `:683-685`, `:707-708`).
- The aggregate dot's hover-opened popover shows the members as a circle-of-dots. This overlaps with
  the T-0174 aggregate-dot/popover work (the code carries `// T-0174` markers around the same
  clustering path, `:759`, `:762`).

**Verification:**
- User verified the clustering/count-badge behavior as correct on 2026-07-01.

**Acceptance Criteria:**
- [x] Co-located dots (≥2 at the same position/zoom) collapse to a single aggregate dot.
- [x] The aggregate exposes its members (ring/circle-of-dots layout via popover) and a count.
- [x] Clusters do not clip; the panel grows to fit the tallest cluster.

*Archived 2026-07-01. Detail reconstructed from source (`TimelineStripView.swift`) as the original
task entry did not survive; feature user-verified.*
