# T-0174: Cluster aggregate-dot representation redesign (readability)

**Status:** ✅ Implemented - Verified
**Component:** `TimelineStripView.swift`
**Epic:** EP-016
**Sprint:** SP-043
**Date Requested:** 2026-06-18
**Date Implemented:** 2026-06-18
**Date Verified:** 2026-06-23
**Design Reference:** Timeline Panel Design v0.3 §4.6 revised — FR-030, FR-031, FR-031a, FR-032, FR-033, FR-034, FR-035, FR-035b
**Related:** T-0166, T-0171 (count badge — superseded by always-on count), T-0172 (zoom-resolve — retained), T-0173 (selection model — reused), I-0045 (auto-grow — main-row growth becomes unnecessary once rings are gone)

**Problem:**
The current co-located cluster renders members as concentric hexagonal rings of dots
(`clusterOffset`). A single ring (≤7) is readable; a second ring produces an illegible
"blob," made worse because per-dot adornments (band rings, manual/inferred offset rings,
T-0173 selection highlight) appear inconsistently across members. The representation does
not scale.

**Requirements (from user):**
1. **Compact** — one timeline X position regardless of member count.
2. **Scales** — holds any number of co-located members; no blob.
3. **Selection-aware** — when a member is the selected scene, the aggregate shows it, and
   shows *which* member.
4. **Drill-in** — a way to reach/pick an individual member.
5. **No text on the line; numbers OK.**
6. **Tooltips for members** must remain reachable.

**Design (agreed):**

*Collapsed (resting) state:*
- A co-located group of ≥2 members renders as a single **aggregate dot** at the shared X.
  (Decision: **always aggregate when co-located** — no "one clean ring first" exception.)
- The aggregate dot's **core is slightly larger** than a regular timeline dot (which is now
  radius 3.5 after I-0045). The larger core marks it as a group and keeps it distinct from
  the individual-dot decoration rings — the placement ring (`manual`/`inferred`, FR-028) and
  the band-membership ring (FR-007/FR-047) — so those are not confused with the aggregate's
  own arc ring. (Suggested ~radius 5–6; tune during build.)
- A **count number** is shown on/at the dot (total member count).
- A **segmented arc ring** surrounds the dot: N equal segments, one per member, in story
  order (the order from `buildClusters`). 23 members → ~15.6°/segment. All members get a
  segment — scenes **and** historical events. The arc is **display-only, non-interactive.**
- **Selection arc (FR-031a):** if a member is the selected scene, its segment lights in the
  selection color. Segment position encodes *which* member. Selecting from the Navigator/
  manuscript lights the segment but does **not** auto-expand (decision: just light the arc).

*Drill-in — hover popover with a circle-of-dots (FR-034):* **(revised after first build — the
original fan-out-in-place "blob" was rejected)**
- Hovering the aggregate opens a **SwiftUI `.popover`** (arrow points at the dot) — NOT an
  in-place expansion.
- The popover contains a **circle of dots**: members arranged evenly around the popover centre,
  ring radius scaling with member count, so it never blobs.
- Each popover dot renders like a timeline dot (band ring, placement ring, selection
  highlight), **grows on hover with a tooltip**, and is **not draggable**.
- Click a popover dot → selects that scene (T-0173 path) and **dismisses the popover**.
- Popover behaviour: **open on hover, stays open** (hover is just the opener), dismisses on
  outside-click or member selection.

*Drag-to-join:*
- Dragging another timeline dot **onto an aggregate** makes it join the group: the dropped
  scene snaps to the aggregate's `offsetMs` (manual offset). The clustering merge threshold is
  size-aware so this lands inside the aggregate's footprint.

*Unchanged:*
- Co-location detection (FR-030/FR-032) and zoom-resolve (T-0172) stay as-is; the aggregate
  is the zoomed-out representation only.

**Implementation (as built — popover redesign):**
`Scrivi/Views/TimelineStripView.swift`
- `aggregateDotRadius = 11` (large enough for the count; clearly bigger than the 3.5 regular
  dot and its decoration rings).
- `AggregateDotView<Popover>` — large core + count (11pt bold) + thick segmented arc
  (`Path.addArc`, 12-o'clock clockwise; 4pt segments, 5pt + orange for the selected member;
  `minSegmentDegrees = 6` falls back to a continuous ring with only the selected slot lit).
  Grows ~15% on hover. Owns a `.popover` opened on hover, dismissed on outside-click.
- `AggregateMembersPopover<Member>` — lays members in a **circle of dots** (radius scales with
  count) using the caller's member builder.
- `PopoverMemberDotView` — non-draggable dot mirroring the timeline dot look (band ring,
  placement ring, selection highlight), grows on hover with a native `.help()` tooltip, click
  selects.
- `ImportedAggregateDotView` — same pattern for imported rows (count + uniform ring, popover of
  `ImportedEventDotView`s; no selection segment — imported events aren't selectable scenes).
- Main render loop: size 1 → `memberDot(...)` (extracted `@ViewBuilder`); size ≥2 →
  `AggregateDotView` + `AggregateMembersPopover`. `@State openAggregateID` /
  `openImportedAggregateID` drive the popovers.
- **Drag-to-join:** `SceneDotView` gained `aggregateJoinOffsetMs` closure; on horizontal
  drag-end the parent's `aggregateJoinOffsetMs(finalPanelX:excludingSceneID:…)` returns the
  snap target if released on an aggregate, and the dot commits a manual snap instead of opening
  the picker.
- **Size-aware merge threshold (fix after user feedback):** `buildClusters` /
  `buildImportedRowClusters` use `mergeThreshold(currentSize:)` — once a running cluster has ≥2
  members (so it renders as a large aggregate), the absorption distance grows to the aggregate
  footprint, so adjacent aggregates no longer overlap.
- `tallestClusterStack` simplified — aggregates are compact and their members live in a
  floating popover, so the panel needs only ~one ring of extra height (the I-0045 ring-stack
  auto-grow is now obsolete for clustering). Removed the dead `clusterStackHeight` helper.
- Removed the old `clusterOffset` ring-of-dots resting render and the conditional count badge.
  (`clusterOffset` retained — still used elsewhere/for reference.)

**Files Affected:**
- `Scrivi/Views/TimelineStripView.swift`

**Build/Test status:** macOS build SUCCEEDED; `ctest` 205/205 pass (Swift-only change). No new
files, so no `project.pbxproj` change.

**Test Steps:**
1. Open `the-twisted-remains-of-myself`; the main group renders as one large aggregate dot
   with its count and a segmented arc — no blob, no edge clipping.
2. Select a scene in that group from the Navigator → the matching arc segment lights; the
   aggregate does not expand.
3. Hover the aggregate → a popover opens with a circle of dots; hover a dot → it grows +
   tooltip; click a dot → navigates + Navigator highlights and the popover dismisses.
4. Click outside → popover dismisses.
5. Drag another scene dot onto the aggregate → it joins (count increments).
6. Two adjacent aggregates do not visually overlap.
7. Zoom in → aggregate resolves into individual dots (T-0172 still works).
8. Imported-timeline-row aggregate behaves the same (popover of event dots, no selection arc).

**Acceptance Criteria:**
- [x] A co-located group of ≥2 renders as one large aggregate dot with a count number.
- [x] The aggregate core is clearly larger than a regular dot and distinct from its rings.
- [x] A segmented arc shows one segment per member in story order (scenes + historical).
- [x] The selected member's arc segment is highlighted; its position identifies the member.
- [x] Selecting from the Navigator lights the arc without expanding.
- [x] Hover opens a circle-of-dots popover; popover dots show tooltips and click-to-select;
      they are not draggable.
- [x] Popover dismisses on outside-click or member selection.
- [x] Dragging a dot onto an aggregate joins it (snaps to the aggregate's offset).
- [x] Adjacent aggregates do not overlap (size-aware merge threshold).
- [x] Zoom-resolve still spreads aggregates into individual dots.
- [x] Applies to imported-timeline-row aggregates as well.

*User confirmed visual approval 2026-06-18; confirmed Verified 2026-06-23.*
