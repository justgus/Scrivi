# EP-016: Timeline Panel — Full Implementation

**Status:** ✅ Closed
**Goal:** Deliver the fully functional Timeline Panel as specified in `Scrivi_Timeline_Panel_Design_v0_3.md`. This includes the ScriviCore persistence layer (timeline meta, scene story-time, story structure bands, historical events, imported timelines), all ScriviCore API operations, the rebuilt SwiftUI `TimelineStripView`, the Time Delta Picker, and the Story Structure band overlay.
**Date Created:** 2026-06-11
**Actual Close Date:** 2026-06-23

### Acceptance Criteria

- [x] A project with no scenes shows an empty timeline panel with the correct empty-state message.
- [x] A project with scenes shows one dot per scene on a horizontal line, positioned in manuscript order by default, vertically centered in the panel.
- [x] `timeline.meta.json` is created in `objects/timelines/` when a new project is created.
- [x] Each scene's `scene.meta.json` contains a `storyTime` block with `offsetMs`, `offsetSource`, `durationMs`, `durationSource`, `inferenceHint`, `inferenceConfidence`, and `storyStructure` fields.
- [x] `project.json` contains a `timelineDefaults` block with `defaultSceneDurationMs` (default 1 hour) and `dragPositioningMode` (default "proportional").
- [x] New scenes default to `durationMs = defaultSceneDurationMs` and `offsetSource = "default"` (immediately after the previous scene).
- [x] The default timeline spans `n × defaultSceneDurationMs` for `n` scenes with no manual positioning.
- [x] A writer can drag a scene dot horizontally; on release the Time Delta Picker appears with the spinner pre-populated, a scene duration row, and "Immediately after" option.
- [x] Committing a position or duration change to scene N silently recomputes the `offsetMs` of all subsequent default-positioned scenes (chain propagation).
- [x] "Immediately after" in the picker sets `offsetSource = "default"`, returning the scene to the chain.
- [x] "Keep position" and "Reset" options in the Time Delta Picker behave correctly.
- [x] "Set Time Delta…" in the scene dot context menu opens the picker without a drag gesture.
- [x] Dot tooltip shows human-readable story-time duration relative to epoch (e.g., "3 days, 4 hours after Story Open").
- [x] Applying a Story Structure writes `objects/story-structures/story-structure.json` and renders colored bands with labels in the panel.
- [x] Band borders are draggable; proportions update correctly and persist. (Final drag-gesture fix in I-0049.)
- [x] A scene dot can be assigned to a band by dragging it vertically to the band label row; the dot displays a colored ring.
- [x] A scene dot can be assigned to a band via the context menu "Assign to Act…" with the same result.
- [x] Removing a Story Structure removes band rendering but preserves all `offsetMs` values and `bandID` assignments in scene metadata.
- [x] A writer can author a historical event; it persists in `objects/historical-events/` and appears as a `#C8A97A` dot on the timeline.
- [x] A writer can import a `.scrivi-timeline.json` file; its events appear as a distinct grey row below the project row.
- [x] Only events within the current project timeline window are shown from imported timelines.
- [x] Multiple imported timelines appear as distinct rows with distinct grey shades.
- [x] A writer can export the project timeline as a `.scrivi-timeline.json` file.
- [x] The panel resizes by dragging its top edge; minimum height is maintained dynamically.
- [x] Multiple co-located scene dots form a cluster. **Note:** the original hexagonal ring (T-0166) was superseded by the aggregate-dot + circle-of-dots popover model (T-0174) for readability/scalability; zoom-resolve into individual dots retained (T-0172).
- [x] Timeline panel is absent on iPhone idiom.
- [x] All timeline data persists correctly across project close/reopen.

### Sprints

| Sprint | Title | Status |
| ------ | ----- | ------ |
| SP-039 | Timeline Persistence — ScriviCore schema and API | ✅ Closed |
| SP-040 | Timeline SwiftUI — core rendering, drag, and Time Delta Picker | ✅ Closed |
| SP-041 | Story Structure bands — overlay, drag borders, band assignment | ✅ Closed |
| SP-042 | Historical events, imported timelines, export, and clustering | ✅ Closed |
| SP-043 | EP-016 verification and polish | ✅ Closed |

### Tasks

| ID | Title | Sprint | Status |
| -- | ----- | ------ | ------ |
| T-0139 | `TimelineMetaJson` schema — read/write `timeline.meta.json` | SP-039 | ✅ Verified |
| T-0140 | `SceneMetaJson` extension — `storyTime` block read/write | SP-039 | ✅ Verified |
| T-0141 | `StoryStructureJson` schema — read/write `story-structure.json` | SP-039 | ✅ Verified |
| T-0142 | `HistoricalEventJson` schema — read/write `historical-events/<id>.json` | SP-039 | ✅ Verified |
| T-0143 | `ExternalTimelineJson` schema — read/write imported timeline files | SP-039 | ✅ Verified |
| T-0144 | ScriviCore facade — timeline, story-time, and band-assignment operations | SP-039 | ✅ Verified |
| T-0145 | ScriviCore facade — story structure, historical events, imported timelines, export | SP-039 | ✅ Verified |
| T-0146 | C API + `ScriviEngine.swift` — expose all timeline operations | SP-039 | ✅ Verified |
| T-0147 | Integration tests — timeline persistence round-trip | SP-039 | ✅ Verified |
| T-0148 | `createProject` update — write `timeline.meta.json` on new project | SP-039 | ✅ Verified |
| T-0149 | `TimelineStripView` rebuild — horizontal line, scene dots, manuscript-order layout | SP-040 | ✅ Verified |
| T-0150 | Scene dot drag — horizontal gesture, story-time update | SP-040 | ✅ Verified |
| T-0151 | Time Delta Picker — spinner, named anchors, dismiss behavior | SP-040 | ✅ Verified |
| T-0152 | Time Delta Picker — pre-population and anchor base | SP-040 | ✅ Verified |
| T-0153 | Scene dot tooltip / popover — title, chapter, human-readable story-time | SP-040 | ✅ Verified |
| T-0154 | Timeline panel resize — drag top edge, dynamic minimum height | SP-040 | ✅ Verified |
| T-0155 | Expand Timeline Forward/Backward popover | SP-040 | ✅ Verified |
| T-0156 | Story Structure band overlay — colored bands, label row, Structure selector menu | SP-041 | ✅ Verified |
| T-0157 | Band border drag — proportional resize, persistence | SP-041 | ✅ Verified |
| T-0158 | Band assignment by drag-up to label — ring color on dot | SP-041 | ✅ Verified |
| T-0159 | Band assignment via context menu ("Assign to Act…" / "Unassign") | SP-041 | ✅ Verified |
| T-0160 | Scene dot context menu — full menu (assign, set time delta, set story time, view scene, copy/paste) | SP-041 | ✅ Verified |
| T-0161 | Historical event CRUD — author in project, drag on timeline, `#C8A97A` dot | SP-042 | ✅ Verified |
| T-0162 | Imported timeline row — render events as grey row below project row | SP-042 | ✅ Verified |
| T-0163 | Epoch offset dialog — import flow with window intersection preview | SP-042 | ✅ Verified |
| T-0164 | Multiple imported timeline rows — distinct grey shades, hide/show toggle | SP-042 | ✅ Verified |
| T-0165 | Timeline export — produce `.scrivi-timeline.json` | SP-042 | ✅ Verified |
| T-0166 | Co-located dot clustering — hexagonal ring layout, count badge (superseded by T-0174) | SP-042 | ✅ Verified |
| T-0167 | EP-016 verification | SP-043 | ✅ Verified |
| T-0168 | Scene duration, chain propagation, project timeline defaults, "Immediately after" | SP-040 | ✅ Verified |
| T-0169 | Hover tooltips for historical event and imported event dots | SP-042 | ✅ Verified |
| T-0170 | Scene/Chapter split and merge — Cmd-Enter splits at cursor, Cmd-Backspace merges | SP-042 | ✅ Verified |
| T-0171 | Cluster count badge when ring height exceeds panel height | SP-043 | ✅ Verified |
| T-0172 | Zoom in resolves clusters into individual dots | SP-043 | ✅ Verified |
| T-0173 | Bidirectional Timeline ↔ Scene Navigator selection and highlight | SP-043 | ✅ Verified |
| T-0174 | Cluster aggregate-dot representation redesign (readability) | SP-043 | ✅ Verified |

### Issues Resolved During EP-016

| ID | Title | Sprint | Status |
| -- | ----- | ------ | ------ |
| I-0045 | Cluster geometry display-only — clipping/overlap; halve dots + auto-grow panel | SP-043 | ✅ Verified |
| I-0046 | Year-spanning timelines: max zoom too low to resolve a 24-hour cluster | SP-043 | ✅ Verified |
| I-0047 | Two-finger pan scrolls too fast when zoomed in | SP-043 | ✅ Verified |
| I-0048 | Story Structure bands don't span scene 1→n or track timeline zoom/pan | SP-043 | ✅ Verified |
| I-0049 | Band border drag does nothing — gesture never fires | SP-043 | ✅ Verified |

### Verification Summary (2026-06-23)

- All EP-016 acceptance criteria verified by the user (T-0167).
- `ctest`: 205/205 passed.
- Xcode build clean on macOS target.
- No regressions in EP-001 through EP-015 functionality.

### Notes

- Co-located clustering evolved within the Epic: hexagonal ring (T-0166) → readability fixes
  (I-0045) → aggregate-dot + circle-of-dots popover redesign (T-0174). The Timeline Panel
  Design was revised to v0.3 §4.6 to reflect this.

---

*Closed 2026-06-23 with user approval.*
