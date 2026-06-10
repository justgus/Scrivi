# EP-014: Scene Inspector Panel

**Status:** ✅ Closed
**Goal:** Add a hideable Scene Inspector panel on the right side of the editor window, housing a tabbed container whose first tab displays context-aware entity cards for the current scene. The panel and its tabs are structural stubs — no entity data yet — but the full UI skeleton is in place.
**Date Created:** 2026-06-09
**Date Closed:** 2026-06-10
**Actual Close Date:** 2026-06-10

### Acceptance Criteria

- [x] A Scene Inspector panel is present on the right side of `ManuscriptEditorView`, between the manuscript surface and the window edge
- [x] The panel is independently hideable via a View menu item; its shown/hidden state persists within the session
- [x] The panel contains a tab bar with at least one tab: **"Scene Entities"** (stub content — a placeholder message and a disabled "Add Entity" button)
- [x] The tab bar is architecturally capable of hosting additional tabs in future epics without structural refactor
- [x] When hidden, the manuscript surface expands to fill the reclaimed width (no dead space)
- [x] The panel has a configurable minimum and default width (240 pt min, 280 pt default)
- [x] On iPhone, the Inspector panel is not present (the phone layout is unchanged)
- [x] On iPadOS, the Inspector panel is present and functions identically to macOS
- [x] The Scene Entities tab stub contains: a title label, a "No entities yet" empty-state message, and a disabled/placeholder "Add Entity" button — all clearly marked as placeholder UI

*All 9 acceptance criteria verified by user on 2026-06-10.*

### Tasks

| ID | Title | Status |
| -- | ----- | ------ |
| T-0128 | SceneInspectorView skeleton — panel, tab bar structure | ✅ Verified |
| T-0129 | Scene Entities stub tab — empty state, placeholder Add Entity button | ✅ Verified |
| T-0130 | Inspector panel integration into ManuscriptEditorView — width, collapse, expand | ✅ Verified |
| T-0131 | iPhone exclusion — Inspector absent on phone idiom | ✅ Verified |
| T-0132 | EP-014 verification | ✅ Verified |

### Sprints

| Sprint | Title | Status |
| ------ | ----- | ------ |
| SP-037 | Scene Inspector Panel | ✅ Closed |

### Completion Summary

EP-014 delivered in full in a single sprint (SP-037). `SceneInspectorView.swift` introduced as a new standalone view with an extensible `InspectorTab` enum, a segmented `Picker` tab bar, and a `SceneEntitiesTabView` stub. `AppEnvironment.inspectorVisible` is the session-scoped source of truth toggled via `CommandMenu("View")` at `⌘⌥I`. iPhone excluded via the `userInterfaceIdiom` runtime check pattern established in T-0123. `project.pbxproj` updated for all three app targets.
