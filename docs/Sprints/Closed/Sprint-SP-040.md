# SP-040: Timeline SwiftUI — Core Rendering, Drag, and Time Delta Picker

**Status:** ✅ Closed
**Epic:** EP-016 — Timeline Panel Full Implementation
**Goal:** `TimelineStripView` renders real scene dots, dots are draggable with story-time persistence, the Time Delta Picker is fully functional, and the panel resizes.
**Start Date:** 2026-06-11
**End Date:** 2026-06-11
**Capacity:** Single session

---

### Assigned Tasks

| ID | Title | Status |
| -- | ----- | ------ |
| T-0149 | `TimelineStripView` rebuild — horizontal line, scene dots, manuscript-order layout | ✅ Verified |
| T-0150 | Scene dot drag — horizontal gesture, story-time update | ✅ Verified |
| T-0151 | Time Delta Picker — spinner, named anchors, dismiss behavior | ✅ Verified |
| T-0152 | Time Delta Picker — pre-population and anchor base | ✅ Verified |
| T-0153 | Scene dot tooltip / popover — title, chapter, human-readable story-time | ✅ Verified |
| T-0154 | Timeline panel resize — drag top edge, dynamic minimum height | ✅ Verified |
| T-0155 | Expand Timeline Forward/Backward popover | ✅ Verified |
| T-0168 | Scene duration, chain propagation, project timeline defaults, "Immediately after" | ✅ Verified |

### Assigned Issues (all resolved and verified)

| ID | Title | Severity |
| -- | ----- | -------- |
| I-0025 | Timeline dot drag moves at half the expected distance | High |
| I-0026 | Time Delta Picker popover appears empty and unsized | High |
| I-0027 | Timeline positions and picker display relative to epoch instead of previous scene end | High |
| I-0028 | Timeline dot drag requires right-click before left-click drag will work | High |
| I-0029 | Right-click on scene dot shows no context menu | Medium |
| I-0030 | Manually-positioned scene does not maintain relative gap when predecessor moves | High |

### Sprint Notes

- `TimelineViewModel` is gap-centric: `gapMs` is canonical, `offsetMs` is derived. All chain recomputation walks full manuscript order from the changed scene forward.
- `TimeDeltaPicker` redesigned from tile grid to spinner row + named time-of-day anchors. Previous scene end (not start) is the arithmetic base throughout.
- `SceneDot` drag uses `.simultaneousGesture` to coexist with `.popover` hit-test interceptors.
- Both `.popover` modifiers moved to the outer `ZStack`; context menu added for "Set Time Delta…".
- `gapMs` added to C++ `SceneStoryTime`, propagated through C API and Swift engine. Old projects without `gapMs` parse cleanly (default 0).
- `project.json` gains `timelineDefaults.defaultSceneDurationMs` (3,600,000ms) and `dragPositioningMode` ("proportional"). Written by `createProject`.
- Default timeline for n scenes spans n hours, not n days.
- ctest: 205/205 passing throughout.
