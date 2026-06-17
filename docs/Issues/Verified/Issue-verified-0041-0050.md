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
