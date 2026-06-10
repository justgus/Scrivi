# EP-015: Real-Time Timeline Panel

**Status:** ✅ Closed
**Goal:** Add a hideable, docked graphical timeline strip below the manuscript viewport. EP-015 delivers the panel structure and UI chrome only; the timeline data model and event schema are deferred to a future Epic.
**Date Created:** 2026-06-09
**Date Closed:** 2026-06-10
**Actual Close Date:** 2026-06-10

### Acceptance Criteria

- [x] A Timeline panel is docked at the bottom of the manuscript surface (below `ManuscriptTextView`, above the window chrome)
- [x] The Timeline panel is independently hideable via a View menu item (⌘⌥T); its shown/hidden state persists within the session
- [x] When hidden, the manuscript surface expands to fill the reclaimed height (no dead space)
- [x] The panel renders a horizontal graphical track with placeholder event markers — visually distinct from the Scene Navigator list
- [x] The panel has a fixed height (80pt) with horizontal scroll if content overflows
- [x] On iPhone, the Timeline panel is not present (phone layout unchanged)
- [x] On iPadOS, the Timeline panel is present and functions identically to macOS
- [x] All content is clearly stub/placeholder — no real event data, no scene linkage, no interaction beyond show/hide

*All 8 acceptance criteria verified by user on 2026-06-10.*

### Tasks

| ID | Title | Status |
| -- | ----- | ------ |
| T-0133 | TimelineStripView skeleton — panel chrome, horizontal scroll container | ✅ Verified |
| T-0134 | Placeholder graphical event markers — static stub layout | ✅ Verified |
| T-0135 | Timeline integration into ManuscriptEditorView — docking, height, collapse | ✅ Verified |
| T-0136 | iPhone exclusion — Timeline absent on phone idiom | ✅ Verified |
| T-0137 | EP-015 verification | ✅ Verified |

### Sprints

| Sprint | Title | Status |
| ------ | ----- | ------ |
| SP-038 | Real-Time Timeline Panel | ✅ Closed |

### Completion Summary

EP-015 delivered in full in a single sprint (SP-038). `TimelineStripView.swift` introduced as a new standalone view: 80pt fixed height, "Timeline" caption header, horizontal `ScrollView` containing 7 stub `TimelineMarkerView` capsules in alternating accent/secondary tint. `AppEnvironment.timelineVisible` is the session-scoped source of truth toggled via a second `Toggle` in the existing `CommandMenu("View")` at `⌘⌥T`. iPhone excluded via the `userInterfaceIdiom` pattern from T-0131. `project.pbxproj` updated for all three app targets.
