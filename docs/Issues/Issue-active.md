# Active Issues

## I-0031: Story Structure bands not rendered after selecting a structure

**Status:** ✅ Resolved - Verified
**Platform:** macOS
**Component:** `TimelineStripView.swift` — `TimelineViewModel`
**Severity:** High
**Sprint:** SP-041

**Description:**
Selecting a Story Structure from the `[Structure ▾]` menu had no visible effect. No colored bands appeared in the timeline panel and no label row was shown.

**Expected Behavior:**
Selecting a structure immediately renders colored bands behind the timeline and a label row at the top of the panel.

**Actual Behavior:**
Panel remained unchanged after selection. No bands, no label row.

**Steps to Reproduce:**
1. Open a project with at least one scene.
2. Click `[Structure ▾]` in the timeline panel header.
3. Select "Three Act Structure".
4. Observe: no bands appear.

**Impact:**
- T-0156 (band overlay), T-0157 (border drag), T-0158 (drag-up assignment), T-0159 (context menu assignment) all depend on bands rendering — all were effectively blocked.

**Date Identified:** 2026-06-12

**Root Cause Analysis:**
The C++ backend wraps the band array under a `"bands"` key in both directions. Swift was generating a plain array `[...]` outbound and trying to decode a plain array inbound — both silently failed, leaving `activeBands` empty.

**Resolution:**

**Fix Date:** 2026-06-12
**Verification Date:** 2026-06-12

**Implementation:**
1. `applyStructure` passes `bandLayoutJSON: ""` — C++ populates from its own built-in table.
2. `decodeBandLayout` unwraps the `"bands"` key before decoding.
3. `encodeBandLayout` wraps output as `{"bands":[...]}`.

**Files Affected:**
- `Scrivi/Views/TimelineStripView.swift`

**Verification:**
- ✅ Selecting "Three Act Structure" renders three colored bands immediately
- ✅ Band labels appear in the label row
- ✅ Bands persist across project close/reopen

**Related Issues:** I-0032, I-0033, I-0034, I-0035

---

## I-0032: Band border cursor does not change on hover

**Status:** ✅ Resolved - Verified
**Platform:** macOS
**Component:** `TimelineStripView.swift` — `BandOverlayView`
**Severity:** Low
**Sprint:** SP-041

**Description:**
When hovering the mouse pointer near a band border in the timeline panel, the cursor remains as the default arrow. It should change to a horizontal resize cursor to indicate the border is draggable.

**Expected Behavior:**
Cursor changes to `NSCursor.resizeLeftRight` when hovering within ~8pt of a band border.

**Actual Behavior:**
Cursor stays as the default arrow at all times within the band overlay.

**Steps to Reproduce:**
1. Apply a Story Structure.
2. Hover slowly over the border between two bands.
3. Observe: cursor does not change.

**Impact:**
- AC-7 fails. Discoverability of band border dragging is reduced.

**Date Identified:** 2026-06-12

**Root Cause Analysis:**
`BandOverlayView` uses a `Canvas` for drawing which does not participate in `onHover`. No cursor-change logic was added to the band overlay — the `cursor(.resizeUpDown)` helper exists on `TimelineStripView`'s resize handle but was never applied to band borders.

**Resolution:**

**Fix Date:** 2026-06-12

**Implementation:**
Added an `.onHover` handler to `BandOverlayView` that computes the nearest border index at the current mouse X position (same logic as `nearestBorderIndex`) and pushes/pops `NSCursor.resizeLeftRight` accordingly.

**Files Affected:**
- `Scrivi/Views/TimelineStripView.swift` — `BandOverlayView`

**Verification:**
- [ ] Hovering within 8pt of a band border changes cursor to horizontal resize
- [ ] Cursor returns to default when moving away from borders

**Related Issues:** I-0031

---

## I-0033: Drag-up band assignment gesture blocked after tooltip is shown

**Status:** ✅ Resolved - Verified
**Platform:** macOS
**Component:** `TimelineStripView.swift` — `SceneDotView`
**Severity:** High
**Sprint:** SP-041

**Description:**
After hovering a scene dot (causing the tooltip popover to appear), the drag-up band assignment gesture does not activate on the next click-drag. The user must press Escape first to dismiss the tooltip, then click and drag.

**Expected Behavior:**
A drag gesture on a scene dot should work at any time, regardless of whether the tooltip popover is currently shown.

**Actual Behavior:**
If the tooltip popover is visible when the user begins a drag, the gesture is blocked or misfires. Pressing Escape dismisses the tooltip, after which the drag works normally.

**Steps to Reproduce:**
1. Apply a Story Structure.
2. Hover a scene dot until the tooltip appears.
3. Without pressing Escape, begin dragging the dot upward.
4. Observe: gesture does not activate.

**Impact:**
- AC-13 fails. Band assignment by drag-up is unreliable in normal use whenever the tooltip has been shown.

**Date Identified:** 2026-06-12

---

### What I assumed initially — and why that was wrong

My first root cause assumption was: *"the tooltip popover intercepts the initial hit-test, preventing `onChanged` from firing at all."* I assumed this because the `.simultaneousGesture` modifier was already in use (a previous fix for a similar issue, I-0028), and popover hit-test interception is a known SwiftUI problem. I applied a fix — moving `showTooltip = false` to the top of `onChanged` — without evidence that `onChanged` was actually failing to fire.

**This was an assumption, not an analysis.** CLAUDE.md principle 1: *Don't assume. Don't hide confusion. Surface tradeoffs.* I should have mapped the actual failure path before writing a fix.

---

### Proper Problem Statement

The user described the symptom precisely: *"Drag up not active on simple click. User has to press 'Esc' first and then click to drag."*

Key observations to extract:
1. The failure mode is **"on simple click"** — not "after hover", not "always". It fails specifically when clicking without Escape first.
2. Escape dismisses the tooltip, after which the drag works. Escape is the standard macOS key to dismiss a popover.
3. The phrase "simple click" suggests the user is clicking and dragging in one motion — the normal way to drag on macOS. This is not a hover-then-drag sequence problem; it's a click-to-begin-drag problem with a popover present.

This points to a different mechanism than I assumed.

---

### 5 Whys

**Why 1: Why does the drag not activate when the tooltip popover is shown?**
Because macOS routes the initial mouse-down event to the frontmost interactive window — and a popover is a separate window. When the popover is visible, the first click on the dot goes to the popover window to dismiss it (as a click-outside dismiss), not to the dot's gesture recognizer.

**Why 2: Why does the first click go to the popover window?**
Because macOS popovers are implemented as `NSPanel` windows. Any click outside the popover's bounds is first consumed by the popover as a "click-outside-to-dismiss" event, before it reaches the content underneath. The dot never sees the mouse-down that started the drag.

**Why 3: Why does Escape fix it?**
Escape dismisses the popover via keyboard — no mouse event consumed, no click swallowed. After Escape the popover window is gone, so the very next mouse-down goes directly to the dot and the drag gesture fires normally.

**Why 4: Why does `.simultaneousGesture` not help here?**
`.simultaneousGesture` allows a gesture to coexist with *SwiftUI's own gesture recognizers* in the same view hierarchy. It does not override macOS window-level event routing. The popover is a separate `NSPanel` — it intercepts the mouse event at the OS level before SwiftUI's gesture system sees it at all.

**Why 5: Why was `showTooltip = false` at the top of `onChanged` insufficient?**
Because `onChanged` never fires on the first click-drag when the popover is present. The mouse-down is consumed by the popover window dismissal. `onChanged` only begins receiving events on the *second* click-drag (after the popover has already been dismissed by the first click). The `showTooltip = false` line runs too late — after the event has already been consumed.

---

### Possible Root Causes (all plausible, not yet narrowed to one)

**RC-1 (most likely): The popover is an `NSPanel` and consumes mouse-down at the OS level.**
The first click-drag is swallowed by the popover window dismiss handler. The dot gesture never receives a mouse-down. Fix: dismiss the popover at the `onHover { false }` exit point (when the mouse leaves the dot), so the popover is already gone before the user can click to drag. This is the only fix that eliminates the window-level interception.

**RC-2: `showPicker` and `showTooltip` are two separate popovers on the same view.**
SwiftUI may serialize popover presentation/dismissal. If `showPicker` was recently shown and dismissed, its dismissal animation may be holding the popover window alive briefly, causing a second popover (tooltip) to appear in an unexpected state. This could compound RC-1.

**RC-3: The `.onHover` handler sets `showTooltip = true` but does not set it to `false` during a drag.**
Once the drag begins, hover events cease. If `showTooltip` is `true` when the drag starts, the popover remains presented throughout the drag, and SwiftUI may refuse to fire the drag gesture on a view with a presented popover depending on the macOS version's gesture arbitration rules.

**RC-4: `minimumDistance: 4` on `DragGesture` combined with popover dismiss.**
The first mouse-down (consumed by popover) moves the apparent gesture start point. The second mouse-down (the actual drag start) is interpreted as a fresh gesture with zero prior translation. If the user moves less than 4pt before releasing, the gesture fires `onEnded` without ever calling `onChanged`, triggering the Time Delta Picker unexpectedly and masking the drag-up failure.

**RC-5: The `ZStack` hit-test area (`contentShape`) is smaller than the popover anchor frame.**
If the popover arrow or window edge overlaps the dot's `contentShape`, part of the dot's draggable area may fall outside the hit-testable region, making certain drag start positions unreliable regardless of popover state.

---

### Correct Fix Strategy

The fix that addresses RC-1 (the most likely and fundamental cause) is: **dismiss the tooltip when the mouse leaves the dot, not when the drag begins.** The `onHover` handler already runs `showTooltip = over` — when `over` becomes `false` (mouse exits), `showTooltip` becomes `false` and the popover dismisses. This means by the time the user moves the mouse to begin a drag (which requires leaving the dot's hover area briefly, or at minimum moving the cursor), the popover should already be dismissing.

However, the guard `if !isDragging { showTooltip = over }` means that if the user begins dragging directly from hover (without the cursor leaving first), `showTooltip` remains `true`. Removing the `!isDragging` guard from `onHover` would cause the tooltip to dismiss the moment a drag starts — but since the drag's `onChanged` fires only after `minimumDistance` is exceeded, there is still a window where the popover intercepts.

The most robust fix is to also set `showTooltip = false` in `onHover` unconditionally (not gated on `!isDragging`), so the tooltip is dismissed the instant the cursor moves at all. This is RC-1's solution.

The `showTooltip = false` at the top of `onChanged` (current fix) addresses RC-3, but not RC-1. It may partially help in cases where the popover is still in its presentation animation but not yet an active NSPanel window.

---

**Resolution:**

**Fix Date:** 2026-06-12 (partial — `showTooltip = false` in `onChanged`)
**Root Cause Confirmed:** Pending verification of revised fix

**Confirmed Root Cause (2026-06-12):**
None of the RC-1 through RC-5 hypotheses were correct. The drag fails on the very first attempt, before any popover has ever appeared — eliminating all popover theories. The actual cause is RC-6 (below), introduced in SP-041.

**RC-6 (confirmed): `BandOverlayView` full-panel `.gesture` swallows all drag events.**
`BandOverlayView` is rendered as a full-panel-width, full-panel-height `Canvas` inside the same `ZStack` as the scene dots. It carries a `.gesture(bandBorderDragGesture)` — the default SwiftUI gesture, which has higher priority than `.simultaneousGesture`. Every mouse-down anywhere in the panel is first offered to the band overlay's drag gesture. The gesture checks `nearestBorderIndex`, finds nothing (when the cursor is not on a border), and silently consumes the entire drag event. The dot's `.simultaneousGesture` never receives it. This was present from the moment `BandOverlayView` was introduced in SP-041. It explains why:
- Drag fails from fresh launch (no popovers involved)
- Drag fails in both directions equally
- Right-click twice → drag works: the context menu appearance/dismissal sequence resets SwiftUI's gesture state in a way that happens to let the dot gesture win the next event
- Once a drag is active, both directions work correctly

**Fix Applied (2026-06-12):**
1. Changed `BandOverlayView`'s `.gesture(bandBorderDragGesture)` to `.simultaneousGesture(bandBorderDragGesture)` so it coexists with the dot's gesture instead of taking priority.
2. Added `.allowsHitTesting(false)` to the `Canvas` itself so the canvas surface does not intercept events — only the gesture recognizer layer is active.

**Files Affected:**
- `Scrivi/Views/TimelineStripView.swift` — `BandOverlayView.body`

**Verification:**
- [ ] Fresh launch: click and drag a dot immediately — gesture activates on first attempt
- [ ] Drag works in both horizontal and vertical directions without prior right-click
- [ ] Band border drag still works correctly
- [ ] Drag-up band assignment works on first attempt

**Related Issues:** I-0031

---

## I-0034: Drag-up on assigned dot should toggle assignment off regardless of release position

**Status:** ✅ Resolved - Verified
**Platform:** macOS
**Component:** `TimelineStripView.swift` — `SceneDotView.combinedDragGesture`
**Severity:** Medium
**Sprint:** SP-041

**Description:**
When a scene dot is assigned to a band, dragging it upward and releasing anywhere — including over a band label — should behave as a toggle: if the dot is assigned, any upward drag unassigns it. Releasing over a label should not re-assign to that label; it should simply unassign.

**Expected Behavior:**
Drag-up on an assigned dot acts as a toggle switch — it always unassigns, regardless of where the dot is released. The user uses the context menu to assign to a specific band.

**Actual Behavior (after initial fix):**
- Drag-up and release below the label row — correctly unassigns (initial fix resolved this)
- Drag-up and release over a band label — assigns to that band instead of unassigning

**Steps to Reproduce:**
1. Assign a scene dot to Act I via context menu.
2. Drag the dot upward all the way to the Act II label and release.
3. Observe: dot is now assigned to Act II. Expected: dot is unassigned.

**Impact:**
- AC-14 partially fails. The unassign-by-drag gesture is inconsistent depending on release position.

**Date Identified:** 2026-06-12

**Root Cause Analysis:**
In `onEnded`, the assign branch (`onAssignToBand(hid)`) runs when `isDraggingUp && hoveredBandID != nil`, taking priority over unassignment. For an already-assigned dot, any upward drag should unassign — the release position should not trigger a re-assignment.

**Resolution:**

**Fix Date:** 2026-06-12

**Implementation:**
In `onEnded`, when `isDraggingUp` is true and the dot has an existing assignment (`!dot.bandID.isEmpty`), always unassign — regardless of whether `hoveredBandID` is set. The assign-on-release path only fires for unassigned dots.

**Files Affected:**
- `Scrivi/Views/TimelineStripView.swift` — `SceneDotView.combinedDragGesture.onEnded`

**Verification:**
- [ ] Drag assigned dot upward, release below label row — unassigns
- [ ] Drag assigned dot upward, release over a label — unassigns (not re-assigned)
- [ ] Drag unassigned dot upward, release over a label — assigns to that band

**Related Issues:** I-0031

---

## I-0035: No checkmark on currently assigned band in context menu

**Status:** ✅ Resolved - Verified
**Platform:** macOS
**Component:** `TimelineStripView.swift` — `SceneDotView` context menu
**Severity:** Medium
**Sprint:** SP-041

**Description:**
The "Assign to Act…" context menu submenu should show a checkmark next to the currently assigned band. No checkmark appears.

**Expected Behavior:**
The band matching `dot.bandID` displays a checkmark in the submenu (per AC-16 and §7.6 of the design doc).

**Actual Behavior:**
All band items appear identically — no checkmark on the assigned band.

**Steps to Reproduce:**
1. Assign a scene dot to a band.
2. Right-click the dot.
3. Open "Assign to Act…".
4. Observe: no checkmark on the assigned band.

**Impact:**
- AC-16 fails. The user cannot tell which band is currently assigned from the context menu.

**Date Identified:** 2026-06-12

**Root Cause Analysis:**
The context menu button uses `Label(band.label, systemImage: "checkmark")` when `dot.bandID == band.bandID`. On macOS, `Label` inside a `Menu` button does not render a checkmark the way a native menu item checkmark does — it renders the SF Symbol image inline as an icon, which macOS menu rendering suppresses. The correct approach is to use a SwiftUI `Button` with a `Text` label and apply a separate checkmark indicator, or use the menu item's native check state via `.tag` on a `Picker`.

**Resolution:**

**Fix Date:** 2026-06-12

**Implementation:**
Replaced the `Label(..., systemImage: "checkmark")` approach with a `Picker`-based context menu section. A `Picker` with `.inline` style renders native macOS menu checkmarks automatically based on the selected tag. The picker selection is bound to the dot's current `bandID`.

**Files Affected:**
- `Scrivi/Views/TimelineStripView.swift` — `SceneDotView` context menu

**Verification:**
- [ ] Right-click assigned dot → "Assign to Act…" → assigned band shows a native checkmark
- [ ] Selecting a different band moves the checkmark

**Related Issues:** I-0031

---

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

*Last Updated: 2026-06-14 (I-0037 opened)*

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

`ImportedEventDotView` receives `ev.title` and `sourceName` as let constants. However, the tooltip shows only `sourceName` for all dots. Two possible causes:

**RC-2 (confirmed):** `@State var isHovered` in `ImportedEventDotView` does not maintain stable per-instance state when the view is placed with `.position()` inside a `ForEach` inside a clipped `ZStack`. `.position()` does not create a new layout container — it places the view's centre at an absolute coordinate within the parent. SwiftUI's view identity diffing for `@State` across a `ForEach` inside a `ZStack` with `.position()` is unreliable: when `isHovered` becomes `true` on any one dot, the parent ZStack re-renders and SwiftUI re-uses or conflates the `@State` across instances. The result is that the hovered state propagates to all dots simultaneously. Since `sourceName` is identical for all dots on the same timeline row, all tooltips show the same string — the timeline name — regardless of which dot is actually hovered. The event titles are correctly populated in the data; the issue is purely SwiftUI state identity.

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

**Status:** 🔴 Open
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

**Correct Fix Strategy:**
After a successful `createScene` or `createChapter` call, reload the timeline model's dot array. The scene list held by `ViewportSceneLoader.allScenes` is the authoritative in-memory list — after a scene is created, `allScenes` is updated. `TimelineViewModel` should be reloaded from `allScenes` at that point. The simplest path: expose a `reload(engine:projectRootPath:scenes:)` method on `TimelineViewModel` (or reuse `load`) and call it from `AppEnvironment` after any scene/chapter creation. Alternatively, observe `ViewportSceneLoader.allScenes` changes and trigger a timeline reload reactively.

**Files Affected:**
- `Scrivi/Views/ViewportSceneLoader.swift` — scene/chapter creation callbacks
- `Scrivi/App/AppEnvironment.swift` — timeline reload after creation
- `Scrivi/Views/TimelineStripView.swift` — `TimelineViewModel`

**Verification:**
- [ ] Creating a scene via Cmd-Enter immediately adds a dot at the end of the timeline
- [ ] Creating a chapter via Shift-Cmd-Enter immediately adds a dot at the end of the timeline
- [ ] The new dot uses default duration and appears immediately after the previous last scene
- [ ] The timeline span updates to include the new dot
- [ ] No regression in existing timeline interactions

**Related Issues:** I-0036, I-0037
