# Verified Issues — I-0041 to I-0050

---

## I-0041: Imported timeline dots on secondary and tertiary rows are not clustered when co-located

**Status:** ✅ Resolved - Verified
**Platform:** macOS
**Component:** `TimelineStripView.swift` — `ImportedEventDotView`, imported timeline row rendering
**Severity:** Medium
**Sprint:** SP-042

**Description:**
Imported timeline rows (secondary and tertiary) displayed event dots via a `ForEach` with no clustering or overlap detection. Co-located events from the same imported timeline overlapped, leaving only the topmost visible.

**Implementation:**
Added `ImportedRowCluster` struct and `buildImportedRowClusters` helper (parallel to `buildClusters`). Each visible imported row runs its own clustering pass over its events sorted by X; dots from different rows are never mixed. The inner `ForEach` over raw events is replaced with a clustered loop using `clusterOffset` for ring layout and a count badge when the cluster height exceeds the row's half-spacing budget. The `hoveredImportedEventKey` tooltip path is unchanged.

**Files Affected:**
- `Scrivi/Views/TimelineStripView.swift` — `ImportedRowCluster`, `buildImportedRowClusters`, imported row rendering loop

**Fix Date:** 2026-06-16 | **Verification Date:** 2026-06-16

**Related Issues:** I-0039, I-0040, I-0037

---

## I-0042: Timeline tooltip shows "Scene N" fallback title instead of first-line text; scene rename in Navigator is not reflected in Timeline

**Status:** ✅ Resolved - Verified
**Platform:** macOS
**Component:** `TimelineStripView.swift` — `SceneDotTooltipView`, `TimelineViewModel`; `ViewportSceneLoader.swift` — `liveTitles`
**Severity:** Medium
**Sprint:** SP-042

**Description:**
1. The tooltip used `dot.title` (`info.title.isEmpty ? "Scene N" : info.title`) and never consulted `loader.liveTitles`, so untitled scenes showed "Scene N" while the Navigator showed the actual first line.
2. Renaming a scene in the Navigator updated `liveTitles` but `SceneDot.title` was only populated at load time, so the tooltip stayed stale until close/reopen.

**Root Cause:**
`TimelineViewModel` had no access to `ViewportSceneLoader.liveTitles`; the two systems were unconnected.

**Implementation:**
1. `SceneDot.title` and `SceneDot.chapterTitle` changed from `let` to `var` for in-place patching.
2. Added `TimelineViewModel.updateDotTitles(liveTitles:allScenes:)` applying the Navigator's three-way priority: explicit `info.title` → `liveTitles` first-line text → existing "Scene N" fallback.
3. `ManuscriptTextView.Coordinator.titleTask` debounce calls `updateDotTitles` immediately after `loader.updateLiveTitle(...)`, propagating first-line edits within the 300ms window.
4. All four `reloadSceneDots` call sites call `updateDotTitles` immediately after.

**Files Affected:**
- `Scrivi/Views/TimelineStripView.swift` — `SceneDot` (`title`/`chapterTitle` now `var`), `TimelineViewModel.updateDotTitles(liveTitles:allScenes:)`
- `Scrivi/Views/ManuscriptTextView.swift` — `titleTask` debounce, all four `reloadSceneDots` call sites

**Fix Date:** 2026-06-16 | **Verification Date:** 2026-06-16

**Related Issues:** I-0037, I-0038

---

## I-0043: Splitting a chapter creates a duplicate chapter number instead of renumbering; no confirmation dialog

**Status:** ✅ Resolved - Verified
**Platform:** macOS
**Component:** `ManuscriptTextView.swift` — chapter split handler; `ViewportSceneLoader.swift` — `splitChapter`
**Severity:** High
**Sprint:** SP-042

**Description:**
1. Splitting Chapter 8 produced two chapters both titled "Chapter 8" instead of renaming the second half "Chapter 9" and incrementing all subsequent chapters.
2. No confirmation dialog gated the irreversible renumbering operation — Shift-Cmd-Enter fired immediately.

**Root Cause:**
`splitChapter` re-assigned `chapterID`/`chapterMetadataPath` but copied `chapterTitle` verbatim. The engine wrote correct ordinal titles to disk but did not return them in `CreateChapterResult`, so Swift never learned them. No in-memory renumbering was attempted, and `handleCreateChapter` fired with no confirmation.

**Implementation:**
1. Added `ViewportSceneLoader.renumberChapterTitlesFrom(segmentIndex:)` — rewrites `chapterTitle` for every scene in every chapter from the affected ordinal onward to `"Chapter N"`.
2. `handleCreateChapter` calls `renumberChapterTitlesFrom` after both the append-at-end and split-in-middle paths.
3. If chapters follow the split point, an `NSAlert` explains the renumbering scope and the split proceeds only on confirmation.
4. `handleMergeChapter` (Shift-Cmd-Backspace) calls `renumberChapterTitlesFrom` after `mergeChapterIntoPredecessor` so subsequent chapters do not retain stale titles.

**Files Affected:**
- `Scrivi/Views/ViewportSceneLoader.swift` — `renumberChapterTitlesFrom(segmentIndex:)`
- `Scrivi/Views/ManuscriptTextView.swift` — `handleCreateChapter` (confirmation dialog + renumber), `handleMergeChapter` (renumber after merge)

**Fix Date:** 2026-06-16 | **Verification Date:** 2026-06-16

**Related Issues:** I-0038

---

## I-0044: Three cluster layout defects: wrong direction, anchor-only grouping, center dot on the line

**Status:** ✅ Resolved - Verified
**Platform:** macOS
**Component:** `TimelineStripView.swift` — `clusterOffset`, `buildClusters`, `buildImportedRowClusters`, main-row rendering loop
**Severity:** High
**Sprint:** SP-042

**Description:**
Defects in the cluster layout discovered incrementally during verification:
1. Ring grew downward instead of upward; clockwise direction inverted.
2. Grouping was non-transitive — candidates compared only against the first member's X, so proximity chains formed multiple small clusters instead of one.
3. Cluster members positioned relative to their own timeline X, not the cluster center, scattering dots at wrong angles.
4. Center dot (position 0) sat on the timeline line instead of being lifted off with the ring.

**Root Cause Analysis:**
- **RC-1:** `height = r * sin(angle)` with the rendering loop's `posY = lineY - offset.height` placed 12 o'clock below the line. Fixed by negating sin.
- **RC-2:** Inner loop compared each candidate against the first member's X. After sorting by X, a contiguous-window pass against the growing right edge fixes transitivity.
- **RC-3:** `posX = baseX + offset.width` used each dot's own `baseX`; must use the cluster center's X.
- **RC-4:** `clusterOffset(position: 0)` returned `.zero`; remapped to a flat 6-position ring with position 0 at the 90° slot.

**Implementation:**
1. `clusterOffset` rewritten: position `i` maps to angle `90° - i * 60°` (clockwise from 12 o'clock), all positions off the line, `spacing = radius * 2 + 3`, height positive-upward.
2. `buildClusters` grouping changed to a contiguous window comparing against `clusterMaxX`.
3. `buildImportedRowClusters` given the same contiguous-window fix.
4. Rendering loop extracts `centerX` from `cluster.members[0]` once; all members use `centerX + offset.width`. Each dot's own `baseX` is still passed as `startX` for drag/story-time math.

**Files Affected:**
- `Scrivi/Views/TimelineStripView.swift` — `clusterOffset`, `buildClusters`, `buildImportedRowClusters`, main-row rendering loop

**Fix Date:** 2026-06-16 | **Verification Date:** 2026-06-16

**Related Issues:** I-0039, I-0040, I-0041

---

## I-0045: Cluster geometry is display-only — tall/edge clusters clip and overlap; dots too large

**Status:** ✅ Resolved - Verified
**Platform:** macOS
**Component:** `TimelineStripView.swift`
**Severity:** Medium
**Sprint:** SP-043

**Found during:** Verification of T-0171 / T-0172 against `the-twisted-remains-of-myself`
(23 scenes; 22 in a 24-hour window forming one main cluster, one flashback two years prior).

**Description:**
The cluster ring layout was a pure display offset; the modified geometry was not accounted
for when positioning/sizing the panel. As a result: (1) two-ring clusters clipped their
rightmost dots when the centre was near the right edge and leftmost dots near the left edge;
(2) the ring stack overlapped the rows above and below (project row and every imported row);
(3) when the count badge appeared, the main-row cluster overlapped it because no vertical
space was reserved for the ring stack.

**Implementation:**
- `dotRadius` 7 → 3.5 (scene + main-row historical dots halved). New `importedDotRadius = 3.0`
  so imported-row dots keep ~their prior absolute size (slightly smaller than scene dots)
  instead of dropping to 2.45.
- New `requiredClusterHeight` state fed by `tallestClusterStack(usable:panelW:)`, which
  measures the tallest ring stack across the main row and every visible imported row at the
  current zoom. `minPanelHeight` grows to fit it (+ badge cap), and recomputes as zoom
  changes so the panel shrinks back when zoom resolves the cluster. Applies to both main-row
  and imported/historical-row clusters.

**Files Affected:**
- `Scrivi/Views/TimelineStripView.swift`

**Fix Date:** 2026-06-18 | **Verification Date:** 2026-06-18

**Related Tasks:** T-0166, T-0171, T-0172 | **Related:** T-0174 (cluster readability redesign)

---

## I-0046: Year-spanning timelines — max zoom too low to resolve a 24-hour cluster

**Status:** ✅ Resolved - Verified
**Platform:** macOS
**Component:** `TimelineStripView.swift`
**Severity:** Medium
**Sprint:** SP-043

**Found during:** Verification of T-0172 against `the-twisted-remains-of-myself`.

**Description:**
With a multi-year span (a 24-hour main cluster plus a flashback two years prior), the fixed
`maxZoom = 50` could only shrink the visible window to ~2 weeks, so the 24-hour cluster of 22
scenes never resolved into individual dots.

**Root Cause:**
`maxZoom` was a constant independent of the data span. The zoom needed to separate a cluster
scales with the ratio of total span to the smallest scene gap, which far exceeds 50 for
multi-year spans.

**Implementation:**
- `maxZoom` is now a computed, data-driven property: `spanMs / max(1h, smallestMainRowGapMs)`,
  clamped to a safety ceiling of 100,000. New `smallestMainRowGapMs()` finds the smallest
  non-zero gap across sorted scene + historical offsets. Zoom remains display-only.

**Files Affected:**
- `Scrivi/Views/TimelineStripView.swift`

**Fix Date:** 2026-06-18 | **Verification Date:** 2026-06-18

**Related Tasks:** T-0172

---

## I-0047: Two-finger pan scrolls too fast when zoomed in

**Status:** ✅ Resolved - Verified
**Platform:** macOS
**Component:** `TimelineStripView.swift` — scroll-pan branch of the scroll-capture handler
**Severity:** Low
**Sprint:** SP-043

**Description:**
When zoomed in, two-finger trackpad pan moved the timeline far too fast for the finger travel.

**Root Cause:**
`scrollOffsetFraction` spans the full timeline, but the visible window is only `1/effectiveZoom`
of it, so `panFraction = -dx / usable` moved on-screen content by `dx * effectiveZoom` pixels.

**Implementation:**
`panFraction = -dx / usable / effectiveZoom` — `dx` pixels of finger travel now pan `dx`
pixels of content at any zoom.

**Files Affected:**
- `Scrivi/Views/TimelineStripView.swift`

**Fix Date:** 2026-06-18 | **Verification Date:** 2026-06-18

**Related Tasks:** T-0172

---

## I-0048: Story Structure bands don't span scene 1→n or track timeline zoom/pan

**Status:** ✅ Resolved - Verified
**Platform:** macOS
**Component:** `TimelineStripView.swift` — `BandOverlayView`, band layout + border-drag math, `TimelineViewModel.structureRange`
**Severity:** Medium
**Sprint:** SP-043

**Description:**
Story Structure bands are intended to span the whole story — from the first scene to the last
scene **in manuscript order**. Previously the bands were laid out by `proportion × panelWidth`
across the entire panel, decoupled from story time and from the timeline's zoom/pan transform,
so they neither lined up with the scenes nor moved/scaled with the timeline.

**Resolution:**
- New `TimelineViewModel.structureRange` → `(startMs, endMs)` where `startMs = dots.first.offsetMs`
  and `endMs = dots.last.offsetMs + dots.last.durationMs` (first→last scene **in manuscript
  order**). Returns nil for < 2 scenes (bands hidden).
- **Key behaviour:** the range uses manuscript-order first/last, **not** min/max offset. A
  flashback scene in the middle of the order (e.g. Scene 13 at −2 years) therefore falls
  *outside* the band region rather than stretching it leftward. Historical events and other
  outliers never affect the band range either (it is scene-range-only by construction).
- `BandOverlayView` now takes `regionX` + `regionWidth` (computed by the parent via `eventX`
  from `structureRange`) and a `fractionForPanelX` closure. All band geometry —
  `drawBands`, `bandLabelRow`, `nearestBorderIndex`, `redistributeProportions` — works within
  `[regionX, regionX + regionWidth]`, so bands grow/shrink on zoom and scroll on pan with the
  timeline. The label row is offset to the region and clipped to it.
- Border-drag editing maps pointer-X → fraction within the region via `fractionForPanelX`
  (`redistributeProportions` rewritten to take a `draggedFraction`), keeping the 4% minimum
  per band.
- Drag-up-to-assign (`SceneDotView.bandAtLabelRow`) now maps within the same region
  (`bandRegionX` / `bandRegionWidth` passed from the parent) so a dot assigns to the band it
  visually sits under.

**Range decision (per user):** scene 1 start → last scene **end** (offset + duration).

**Files Affected:**
- `Scrivi/Views/TimelineStripView.swift` — `TimelineViewModel.structureRange`; `BandOverlayView`
  (props + `drawBands`, `bandLabelRow`, `bandBorderDragGesture`, `nearestBorderIndex`,
  `redistributeProportions`); `BandOverlayView` call site; `bandRegionX`/`bandRegionWidth`
  helpers; `SceneDotView` (`bandRegionX`/`bandRegionWidth`, `bandAtLabelRow`).

**Fix Date:** 2026-06-18 | **Verification Date:** 2026-06-23

---

## I-0049: Band border drag does nothing — gesture never fires

**Status:** ✅ Resolved - Verified
**Platform:** macOS
**Component:** `TimelineStripView.swift` — `BandOverlayView`
**Severity:** Medium
**Sprint:** SP-043

**Description:**
When zoomed in, dragging a Story Structure band border had no effect. The band would not
resize. Diagnostic logging confirmed the SwiftUI drag gesture never fired even once: the
passive AppKit pointer observer logged `RAW-DOWN`/`RAW-DRAG` events throughout the drag, but
no `BANDDIAG P2-gesture` line (emitted inside `bandBorderDragGesture.onChanged`) ever appeared,
so no border latched and nothing moved.

**Root Cause:**
In `BandOverlayView.body`, `.allowsHitTesting(false)` and `.simultaneousGesture(bandBorderDragGesture)`
were stacked on the same `Canvas` view. `allowsHitTesting(false)` makes the view
non-interactive, which also starves any gesture attached to that same view of events. The
`I-0033` intent ("canvas doesn't intercept, gesture layer does") was defeated because both
modifiers lived on one view. The border-resize hover (`.onContinuousHover`, I-0032) was on the
same non-hittable view and was likewise dead.

**Resolution:**
- Kept `.allowsHitTesting(false)` on the `Canvas` so it stays purely visual.
- Moved the drag gesture and the border-resize hover onto a separate hit-testable transparent
  overlay (`Color.clear.contentShape(Rectangle())`) layered over the canvas, in the same panel
  coordinate space the canvas draws in (so `nearestBorderIndex(at:)` and `fractionForPanelX`
  receive the X values they expect).
- Layering preserved: `BandOverlayView` is the bottom layer of the parent `ZStack`; scene/event
  dots are added after it and still win direct hit-testing. `.simultaneousGesture` does not
  consume their events.

**Files Affected:**
- `Scrivi/Views/TimelineStripView.swift` — `BandOverlayView.body`.

**Fix Date:** 2026-06-23 | **Verification Date:** 2026-06-23

**Related Issues:** I-0048 | **Related:** I-0032, I-0033

---
