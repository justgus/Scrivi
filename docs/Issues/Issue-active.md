# Active Issues

## I-0036: Color.clear overlay blocks all timeline interaction; historical event placed at midpoint, not click location; no title-entry popup on creation

**Status:** 🔴 Resolved - Not Verified
**Platform:** macOS
**Component:** `TimelineStripView.swift` — `TimelineStripView.body`, `TimelineViewModel.createHistoricalEvent`
**Severity:** Critical
**Sprint:** SP-042

**Description:**
Three related defects introduced in SP-042 that together make the timeline panel non-interactive:

1. A `Color.clear.contentShape(Rectangle())` view was placed as the last (topmost) child of the ZStack. On macOS, the topmost ZStack child wins all hit-testing. This single view consumed every mouse event — right-clicks, hovers, drags — across the entire timeline panel. No scene dot, historical event dot, or band border ever received an event.

2. "New Historical Event Here" placed the new event at the timeline midpoint (`minOffsetMs + spanMs/2`) regardless of where the user right-clicked.

3. No title-entry popup appeared after creating a historical event. The event was silently created with the name "New Historical Event" and the user had no immediate way to rename it.

**Expected Behavior:**
- Scene dots, historical event dots, and band borders receive mouse events normally.
- "New Historical Event Here" places the event at the right-click X position on the timeline.
- The Historical Event editor sheet opens immediately after creation so the user can set the title.

**Actual Behavior:**
- No dots hovered, scaled, dragged, or showed context menus. The band border cursor never changed. Only the `Color.clear` overlay's context menu appeared regardless of click location, and it always showed "New Historical Event Here / Import / Export".
- Historical event was placed at the fixed midpoint.
- No title-entry popup appeared; event was named "New Historical Event" silently.

**Side effect:** Two (later three) stale `hevent_*.json` files were written to the project during AC testing before the issue was identified. These were deleted manually from `objects/historical-events/`.

**Secondary defect (identified during verification, 2026-06-13):** After the initial fix, the empty-area context menu no longer appeared on right-click in blank timeline space. Moving the context menu from `Color.clear` to the ZStack itself left empty space with no hit-testable surface, so right-clicks in blank areas found nothing and the menu never fired.

**Steps to Reproduce:**
1. Open a project, show the timeline.
2. Hover over any scene dot — no tooltip appears, dot does not scale up.
3. Right-click a scene dot — shows "New Historical Event Here" instead of the dot's own context menu.
4. Right-click empty space → "New Historical Event Here" → event created at midpoint with no title popup.

**Date Identified:** 2026-06-13

**Root Cause Analysis:**
`Color.clear.contentShape(Rectangle())` was the last child in the ZStack, making it the topmost hit-test target for the entire panel. SwiftUI's ZStack renders children in order, last on top. A `contentShape(Rectangle())` on a full-panel `Color.clear` makes the entire panel area hit-testable by that view only. All child views (dots, band overlay) were unreachable by any mouse event. The midpoint placement was a placeholder that was never updated to use the actual click location. The title-entry popup was never implemented.

**Resolution:**

**Fix Date:** 2026-06-13

**Implementation:**
1. Removed `Color.clear.contentShape(Rectangle())` from the ZStack entirely.
2. Moved the empty-area context menu to the ZStack itself via `.contextMenu`. SwiftUI gives priority to child views' own context menus when the right-click lands on them; the ZStack-level menu fires only on empty space.
3. Added `.onContinuousHover` to the ZStack to track the last known mouse X position into `contextMenuClickX`.
4. "New Historical Event Here" now computes `offsetMs` from `contextMenuClickX` via `offsetMs(fromPanelX:usable:)`.
5. After `createHistoricalEvent` returns, the `HistoricalEventEditorSheet` is immediately presented so the user can set the title before dismissing.
6. Deleted three stale `hevent_*.json` files from the test project's `objects/historical-events/` directory.
7. (Secondary fix, 2026-06-13) Added `Color.clear.contentShape(Rectangle())` as the **first** child of the ZStack (bottom layer), giving empty space a hit-testable surface so the ZStack-level context menu fires there. Because it is the bottom layer, all child views rendered above it win hit-testing when clicked directly — restoring dot and band interaction while also making blank space right-clickable.

**Files Affected:**
- `Scrivi/Views/TimelineStripView.swift` — `TimelineStripView.body`

**Verification:**
- [ ] Hovering a scene dot shows tooltip and scales the dot
- [ ] Right-clicking a scene dot shows the dot's own context menu (Set Time Delta…, Assign to Act…, etc.)
- [ ] Right-clicking empty space shows "New Historical Event Here / Import / Export"
- [ ] "New Historical Event Here" places the dot at the right-click X position, not the midpoint
- [ ] The Historical Event editor sheet opens immediately after creation
- [ ] Band border cursor changes to horizontal resize on hover
- [ ] Dragging scene dots and historical event dots works correctly

**Related Issues:** I-0033 (same ZStack hit-testing class of problem)

---

## I-0037: Hover tooltips for historical event and imported event dots display in wrong position and show wrong content

**Status:** 🔴 Open
**Platform:** macOS
**Component:** `TimelineStripView.swift` — `HistoricalEventDotView`, `ImportedEventDotView`, `SimpleTooltipView`
**Severity:** High
**Sprint:** SP-042

**Description:**
Two tooltip defects introduced when implementing hover tooltips for historical event and imported event dots (T-0169):

1. **Historical event dot tooltip is clipped above the panel.** Only the drop shadow at the bottom edge of the tooltip is visible. The tooltip body is entirely above the top of the timeline panel.

2. **All imported event dots show the same tooltip text** — the source timeline name ("Mr. Omicron — The Shadow Thread") regardless of which dot is hovered. The individual event title is not shown.

**Expected Behavior:**
- Hovering a historical event dot shows a tooltip above the dot, within the panel bounds, with the event title and its story-time offset.
- Hovering an imported event dot shows a tooltip with that specific event's title and the source timeline name.
- Tooltips appear and dismiss immediately as the cursor moves between dots.

**Actual Behavior:**
- Historical event tooltip appears above the panel boundary — only the drop shadow is visible inside the panel.
- All imported event dots show identical tooltip content regardless of which dot is hovered.

**Date Identified:** 2026-06-14

**Root Cause Analysis:**

**RC-1 — Historical event tooltip clipped above panel:**

`HistoricalEventDotView` renders its tooltip as a local `.overlay(alignment: .bottom)` with `.offset(y: -(radius * 2 + 52))` pushing upward. The dot is placed via `.position(x: x, y: lineY)` inside the ZStack, which has `.frame(maxWidth: .infinity, maxHeight: .infinity).clipped()` applied at the ZStack level. The tooltip, offset upward from the dot, extends outside the ZStack's frame and is clipped by it — only the drop shadow bleeds through.

This is the identical class of problem that required scene dot tooltips to use the panel-level `.overlay` (outside `.clipped()`). A local overlay on a view inside a clipped ZStack can never escape the clip boundary. Historical event tooltips must follow the same pattern as scene dot tooltips: render via `model.hoveredHistoricalEventID` in the panel-level overlay.

**RC-2 — All imported event dots show the same tooltip:**

`@State var isHovered` in `ImportedEventDotView` does not maintain stable per-instance state when the view is placed with `.position()` inside a `ForEach` inside a clipped `ZStack`. `.position()` does not create a new layout container — it places the view's centre at an absolute coordinate within the parent. SwiftUI's view identity diffing for `@State` across a `ForEach` inside a `ZStack` with `.position()` is unreliable: when `isHovered` becomes `true` on any one dot, the parent ZStack re-renders and SwiftUI re-uses or conflates the `@State` across instances. The result is that the hovered state propagates to all dots simultaneously. Since `sourceName` is identical for all dots on the same timeline row, all tooltips show the same string — the timeline name — regardless of which dot is actually hovered. The event titles are correctly populated in the data; the issue is purely SwiftUI state identity.

**Correct Fix Strategy:**

1. Restore `hoveredHistoricalEventID: String?` to `TimelineViewModel`. Wire `HistoricalEventDotView.onHover` to set it. Render the historical event tooltip in the panel-level `.overlay` outside `.clipped()`, using `lineY` as the Y anchor (always above).

2. Imported event dots: remove `@State var isHovered` from `ImportedEventDotView` entirely. Instead, track the hovered imported event by key (`"timelineID:eventID"`) on `TimelineViewModel`, and render the tooltip in the panel-level `.overlay` outside `.clipped()` — the same pattern used for scene dots and historical event dots. This eliminates the SwiftUI identity/state sharing problem completely because the tooltip is no longer owned by the individual dot view.

**Files Affected:**
- `Scrivi/Views/TimelineStripView.swift`
- `scripts/mr-omicron-mystery-timeline.scrivi-timeline.json` (verify title fields)

**Verification:**
- [ ] Hovering a historical event dot shows tooltip fully within panel bounds, above the dot
- [ ] Tooltip shows the event's title and story-time offset
- [ ] Hovering different imported event dots shows different titles per dot
- [ ] Tooltips appear and dismiss as cursor moves between dots
- [ ] No other interactions affected

**Related Issues:** I-0036

---

## I-0038: New scenes created via Cmd-Enter do not appear as dots on the Timeline

**Status:** 🟡 Resolved - Not Verified
**Platform:** macOS
**Component:** `ViewportSceneLoader.swift`, `AppEnvironment.swift`, `TimelineStripView.swift` — `TimelineViewModel`
**Severity:** High
**Sprint:** SP-042
**Epic:** EP-016

**Description:**
When a writer creates a new scene using Cmd-Enter (create scene in current chapter after current scene) or Shift-Cmd-Enter (create chapter with first scene), the new scene is created in the manuscript but no corresponding dot appears on the Timeline panel. The Timeline only reflects the scene list that existed when the project was opened.

**Expected Behavior:**
- Creating a new scene adds a dot at the right end of the Timeline immediately, using the default scene duration and `offsetSource = "default"` (immediately after the previous scene).
- Creating a new chapter with a first scene similarly adds a dot.
- The timeline span compresses/expands to accommodate the new dot without manual intervention.

**Actual Behavior:**
The timeline panel does not update. The new scene dot does not appear until the project is closed and reopened.

**Steps to Reproduce:**
1. Open a project with at least one scene visible on the timeline.
2. Press Cmd-Enter to create a new scene.
3. Observe: the scene appears in the Scene Navigator but no dot appears on the Timeline.

**Date Identified:** 2026-06-14

**Root Cause Analysis:**
`TimelineViewModel` is populated once in `AppEnvironment.openProject()` via `tlModel.load(engine:projectRootPath:scenes:)` using the scene list from `OpenProjectResult`. When `createScene` or `createChapter` is called from `ManuscriptTextView` (via the Coordinator), the new scene is written to disk by ScriviCore and added to the Scene Navigator's model, but `TimelineViewModel.dots` is never updated. There is no notification or callback path from scene creation back to the timeline model.

**Resolution:**

**Fix Date:** 2026-06-14

**Implementation:**
1. Added `reloadSceneDots(engine:projectRootPath:scenes:)` to `TimelineViewModel` (`TimelineStripView.swift:265`). Rebuilds only the dot array from the updated scene list, preserving existing band assignments, historical events, imported timelines, and story structure.
2. After every successful `createScene`, `createChapter`, scene split, and chapter split in `ManuscriptTextView.Coordinator`, `env.timelineModel?.reloadSceneDots(...)` is called with `loader.allScenes` (which is already updated by the loader's insert methods).

**Files Affected:**
- `Scrivi/Views/TimelineStripView.swift` — `TimelineViewModel.reloadSceneDots`
- `Scrivi/Views/ManuscriptTextView.swift` — Coordinator create/split handlers

**Verification:**
- [ ] Creating a scene via Cmd-Enter immediately adds a dot at the end of the timeline
- [ ] Creating a chapter via Shift-Cmd-Enter immediately adds a dot at the end of the timeline
- [ ] The new dot uses default duration and appears immediately after the previous last scene
- [ ] The timeline span updates to include the new dot
- [ ] No regression in existing timeline interactions

**Related Issues:** I-0036, I-0037

---

## I-0039: Clustering only applies to unanchored dots; anchored scene dots with overlapping positions are not grouped

**Status:** 🔴 Open
**Platform:** macOS
**Component:** `TimelineStripView.swift` — `buildClusters`, `TimelineStripView.body`
**Severity:** High
**Sprint:** SP-042

**Description:**
`buildClusters` iterates over `model.dots` and groups any two scene dots whose rendered X positions are within one diameter of each other. However, the clustering groups that are produced are only rendered for dots where the cluster position logic fires — in practice, only unanchored ("default" `offsetSource`) dots that land at the same default position get grouped. Anchored dots (those with a manually or inferred `offsetMs`) that happen to occupy the same or very close screen positions are not being grouped and therefore overlap as stacked dots on the timeline line.

**Expected Behavior:**
Any two scene dots — anchored or unanchored — whose rendered X positions are within one dot diameter should be grouped into a cluster and rendered using the hexagonal ring layout. No dot should visually overlap another on the main timeline row.

**Actual Behavior:**
Only unanchored dots that happen to land at the same default position are grouped. Anchored dots that are close together or co-located overlap without grouping, creating a visual pile-up on the timeline line.

**Steps to Reproduce:**
1. Open a project where multiple scenes have manually-set story times that land within one dot diameter of each other on screen.
2. Observe the timeline: the overlapping anchored dots appear stacked on the line with no cluster ring or count badge.

**Root Cause (suspected):**
`buildClusters` iterates `model.dots` in order and computes `dotX` for each. The comparison `abs(cx - ox) <= diameter` should group all nearby dots regardless of `offsetSource`. However, the ring-layout and badge rendering path (`clusterOffset`) requires the ZStack to place each member with a computed offset from the center. If `dotX` returns the same X for many unanchored dots (because they all share `offsetMs = 0` and `offsetSource = "default"`), they coincide trivially. For anchored dots the X values differ by tiny screen fractions — the `<= diameter` threshold may not catch them if `dotX` uses a floating-point comparison that is too tight, or `buildClusters` uses sorted order assumptions that cause it to skip comparisons between non-adjacent members.

**Date Identified:** 2026-06-15

**Files Affected:**
- `Scrivi/Views/TimelineStripView.swift` — `buildClusters` (line 1029), `TimelineStripView.body` cluster rendering (line 626)

**Verification:**
- [ ] Two anchored scene dots at the same story time are rendered as a cluster with a ring layout
- [ ] Two anchored scene dots within one diameter on screen are rendered as a cluster
- [ ] Unanchored dots continue to cluster as before
- [ ] No dot overlaps another on the main timeline row

**Related Issues:** I-0037

---

## I-0040: Historical event dots on the main timeline are not clustered when co-located

**Status:** 🔴 Open
**Platform:** macOS
**Component:** `TimelineStripView.swift` — `TimelineStripView.body`, historical event dot rendering
**Severity:** High
**Sprint:** SP-042

**Description:**
Historical event dots placed on the main (project) timeline row are rendered individually via a `ForEach` over `model.historicalEvents` without any clustering pass. When two or more historical event dots — or a historical event dot and a scene dot — land at the same or very close X position, they overlap visually. Unlike scene dots, historical event dots are not fed through `buildClusters` and are not subject to the hexagonal ring layout.

**Expected Behavior:**
Historical event dots that are co-located with other historical event dots, or co-located with scene dots, on the main timeline row should be grouped and rendered in the cluster ring layout (or at minimum offset so they do not visually overlap).

**Actual Behavior:**
Historical event dots are rendered at their computed X positions without any overlap detection. Multiple historical events at the same offset stack on top of each other and the overlapping dots are invisible beneath the top-most dot.

**Steps to Reproduce:**
1. Create two or more historical events at the same story-time offset (or close enough to overlap on screen).
2. Observe the timeline main row: only the topmost dot is visible; the others are hidden underneath it.

**Date Identified:** 2026-06-15

**Files Affected:**
- `Scrivi/Views/TimelineStripView.swift` — historical event `ForEach` in `TimelineStripView.body` (approximately line 690–730)

**Verification:**
- [ ] Two historical event dots at the same offset are rendered as a cluster (ring or offset)
- [ ] A historical event dot co-located with a scene dot does not visually overlap it
- [ ] No historical event dot is hidden beneath another

**Related Issues:** I-0039, I-0037

---

## I-0041: Imported timeline dots on secondary and tertiary rows are not clustered when co-located

**Status:** 🔴 Open
**Platform:** macOS
**Component:** `TimelineStripView.swift` — `ImportedEventDotView`, imported timeline row rendering
**Severity:** Medium
**Sprint:** SP-042

**Description:**
Imported timeline rows (secondary and tertiary, rendered below the main project row) display their event dots via a `ForEach` over each `ImportedTimelineRow`'s events without any clustering or overlap detection. When two events from the same imported timeline land at the same or very close X position, they overlap visually. There is no grouping, ring layout, or count badge for imported timeline rows.

**Expected Behavior:**
Imported timeline event dots that are co-located on the same row should be grouped and rendered using the same ring layout as scene dots, or at minimum offset so they do not overlap. A count badge should appear when the cluster exceeds the panel height budget.

**Actual Behavior:**
Imported event dots are rendered at their raw X positions with no clustering. Co-located events stack invisibly.

**Steps to Reproduce:**
1. Import a `.scrivi-timeline.json` that contains two or more events at the same story-time offset (after epoch adjustment).
2. Observe the imported timeline row: the dots are stacked with only the topmost visible.

**Date Identified:** 2026-06-15

**Files Affected:**
- `Scrivi/Views/TimelineStripView.swift` — imported timeline `ForEach` body (approximately line 730–790)

**Verification:**
- [ ] Two events from the same imported timeline at the same offset render as a cluster or are offset
- [ ] Count badge appears when cluster height exceeds panel row budget
- [ ] Events from different imported timeline rows do not interfere with each other's clustering

**Related Issues:** I-0039, I-0040, I-0037

---

## I-0042: Timeline tooltip shows "Scene N" fallback title instead of first-line text; scene rename in Navigator is not reflected in Timeline

**Status:** 🔴 Open
**Platform:** macOS
**Component:** `TimelineStripView.swift` — `SceneDotTooltipView`, `TimelineViewModel`; `ViewportSceneLoader.swift` — `liveTitles`
**Severity:** Medium
**Sprint:** SP-042

**Description:**
Two related defects in how scene titles are displayed in the Timeline panel:

1. **Tooltip shows "Scene N" instead of first-line text.** When a scene has no explicit title set (i.e., `SceneInfo.title` is empty) the Scene Navigator correctly falls back to the first line of the scene's text content via `loader.liveTitles`. The Timeline tooltip (`SceneDotTooltipView`) uses `dot.title`, which is set from `info.title.isEmpty ? "Scene \(idx + 1)" : info.title` — it never consults `liveTitles`. The result is that the tooltip displays the ordinal fallback ("Scene 7") while the Navigator shows the actual first line ("Ekta lives in an apartment near the campus").

2. **Renaming a scene in the Scene Navigator does not update the Timeline tooltip.** When the user edits a scene's title in the Navigator, `liveTitles` is updated via `updateLiveTitle(_:forSceneID:)`. The `SceneDot.title` field in `TimelineViewModel.dots` is only populated at load time — there is no reactive path from `liveTitles` back to `TimelineViewModel`. Consequently, `dot.title` remains stale until the project is closed and reopened.

**Expected Behavior:**
- When a scene has no explicit title, the Timeline tooltip shows the same first-line text that the Scene Navigator shows.
- When the user renames a scene in the Navigator (or the first line of text changes), the Timeline tooltip updates to match within the same session.

**Actual Behavior:**
- Timeline tooltip shows "Scene N" for untitled scenes regardless of what the Navigator shows.
- Changing the scene name in the Navigator has no effect on the Timeline tooltip for the rest of the session.

**Steps to Reproduce:**
1. Open a project where at least one scene has no explicit title (the first line of text is the de-facto name).
2. Hover the corresponding dot in the Timeline panel — tooltip shows "Scene N".
3. The Scene Navigator for the same scene shows the first line of text.
4. Rename the scene in the Navigator.
5. Hover the dot again — tooltip still shows the old "Scene N" fallback.

**Root Cause:**
`TimelineViewModel` is populated from `SceneInfo.title` at project open and on `reloadSceneDots`. It has no access to `ViewportSceneLoader.liveTitles`, which is the live first-line-title dictionary updated by `ManuscriptTextView.Coordinator`. The two systems are not connected.

**Date Identified:** 2026-06-15

**Files Affected:**
- `Scrivi/Views/TimelineStripView.swift` — `TimelineViewModel` dot construction (line 244, 275), `SceneDotTooltipView.tooltipContent` (line 1769)
- `Scrivi/Views/ViewportSceneLoader.swift` — `liveTitles` (line 38), `updateLiveTitle` (line 123)

**Verification:**
- [ ] Hovering a dot for an untitled scene shows the same first-line text that the Scene Navigator shows
- [ ] Renaming a scene in the Navigator updates the Timeline tooltip in the same session without reopening the project
- [ ] Scenes with explicit titles continue to show their titles unchanged

**Related Issues:** I-0037, I-0038

---

## I-0043: Splitting a chapter creates a duplicate chapter number instead of renumbering; no confirmation dialog

**Status:** 🔴 Open
**Platform:** macOS
**Component:** `ManuscriptTextView.swift` — chapter split handler; `ViewportSceneLoader.swift` — `splitChapter`
**Severity:** High
**Sprint:** SP-042

**Description:**
Two related defects in the chapter split operation (Shift-Cmd-Enter):

1. **Duplicate chapter number produced.** Splitting Chapter 8 (for example) produces two chapters both titled "Chapter 8" instead of renaming the second half "Chapter 9" and incrementing all subsequent chapters (Chapter 9 → 10, etc.). The `splitChapter` implementation reassigns segments to the new chapter ID but does not update chapter ordinal labels anywhere — it preserves `chapterTitle` from the original chapter verbatim.

2. **No confirmation dialog.** Renumbering all chapters following the split point is a large, irreversible operation that affects the entire manuscript. Currently, Shift-Cmd-Enter fires immediately with no warning. A confirmation dialog explaining the scope (e.g., "Splitting here will renumber Chapter 9 through Chapter N") should gate the operation.

**Expected Behavior:**
- Splitting a chapter at the cursor produces the original chapter (retaining its number) and a new chapter numbered one higher than the original, with all subsequent chapters incremented by one.
- Before executing the split, a confirmation dialog appears explaining that all subsequent chapters will be renumbered and giving the user the opportunity to cancel.

**Actual Behavior:**
- Both halves of the split retain the original chapter's title, producing a duplicate chapter number in the Navigator.
- The split fires immediately with no confirmation.

**Steps to Reproduce:**
1. Position the cursor in the middle of Chapter 8 (not the first or last scene).
2. Press Shift-Cmd-Enter.
3. Observe: two chapters labelled "Chapter 8" appear in the Scene Navigator. Chapters 9 and beyond are not renumbered.
4. Observe: no confirmation dialog appeared before the split.

**Root Cause (suspected):**
`ViewportSceneLoader.splitChapter` copies `allScenes[j].chapterTitle` unchanged into the new chapter's `SceneInfo` entries (line 447). No renaming of the new chapter or renumbering of subsequent chapters is attempted — either in-memory or on disk (via the engine). The chapter-split engine call creates the chapter with a new ID but the title assignment is the caller's responsibility; neither the caller (`ManuscriptTextView.Coordinator`) nor the loader updates any titles.

**Date Identified:** 2026-06-15

**Files Affected:**
- `Scrivi/Views/ManuscriptTextView.swift` — chapter split handler (line 390–480)
- `Scrivi/Views/ViewportSceneLoader.swift` — `splitChapter` (line 423), `chapterTitle` propagation (line 447)

**Verification:**
- [ ] Splitting Chapter 8 produces "Chapter 8" and "Chapter 9" (or equivalent ordinal scheme)
- [ ] All chapters after the split point are renumbered correctly
- [ ] A confirmation dialog appears before the split, naming the scope of the renumber
- [ ] The user can cancel from the confirmation dialog with no changes made
- [ ] The Timeline panel reflects the updated chapter titles after the split

**Related Issues:** I-0038

---

*Last Updated: 2026-06-15 (archived I-0031–I-0035 to verified; I-0036–I-0043 active)*
