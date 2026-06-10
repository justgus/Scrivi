# SP-038: Real-Time Timeline Panel

**Status:** ✅ Closed
**Epic:** EP-015: Real-Time Timeline Panel
**Goal:** Deliver a hideable graphical timeline strip docked below the manuscript surface, with stub placeholder event markers, a View menu toggle (⌘⌥T), and iPhone exclusion.
**Start Date:** 2026-06-10
**End Date:** 2026-06-10
**Capacity:** Single session

### Assigned Tasks

| ID | Title | Priority | Status |
| -- | ----- | -------- | ------ |
| T-0133 | TimelineStripView skeleton — panel chrome, horizontal scroll container | High | ✅ Verified |
| T-0134 | Placeholder graphical event markers — static stub layout | High | ✅ Verified |
| T-0135 | Timeline integration into ManuscriptEditorView — docking, height, collapse | High | ✅ Verified |
| T-0136 | iPhone exclusion — Timeline absent on phone idiom | Medium | ✅ Verified |
| T-0137 | EP-015 verification | High | ✅ Verified |

### Assigned Issues

*None.*

### Sprint Notes

- `timelineVisible` on `AppEnvironment` mirrors `inspectorVisible` from EP-014.
- Toggle: second item in `CommandMenu("View")` with `⌘⌥T`.
- `TimelineStripView` docks inside the manuscript `VStack` below `ManuscriptTextView`.
- Fixed height 80pt; `frame(maxWidth: .infinity).frame(height: 80)` — `frame(maxWidth:height:)` overload does not exist on macOS 26, chained calls required.
- `project.pbxproj` updated: `C035`/`C035_BF`, `D035_BF`, `E035_BF` for all three app targets.

### Retrospective

**Completed:**
- T-0133: `TimelineStripView.swift` created; 80pt fixed height; "Timeline" caption header; `ScrollView(.horizontal)` container.
- T-0134: `TimelineMarkerView` private struct; 7 alternating-color stub capsules ("Event 1"–"Event 7"); 80×40pt each.
- T-0135: `AppEnvironment.timelineVisible`; `TimelineStripView` docked in manuscript `VStack`; `CommandMenu("View")` second toggle `⌘⌥T`; `pbxproj` updated.
- T-0136: iPhone exclusion via `userInterfaceIdiom != .phone` guard — same pattern as T-0131.
- T-0137: All 8 EP-015 ACs verified by user.

**Returned to Backlog:**
- Nothing.

**What went well:**
- All five tasks delivered and verified in a single session.
- `AppEnvironment` pattern from EP-014 applied cleanly.

**What to improve:**
- `frame(maxWidth:height:)` is not a valid SwiftUI overload — caught at first build. Always use chained `.frame()` calls for multi-axis constraints.

**Carry-forward notes:**
- EP-015 complete and closed. No active epics or sprints remain. Backlog: T-0118 (EP-011), T-0123 (EP-012).
