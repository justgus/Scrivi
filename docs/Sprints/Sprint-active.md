# Active Sprint

## SP-042: Historical Events, Imported Timelines, Export, and Clustering

**Status:** 🟡 Active
**Epic:** EP-016
**Start Date:** 2026-06-12
**End Date:** —
**Goal:** Historical events can be authored in the project. External timelines can be imported with epoch offsets and rendered as distinct rows. The project timeline can be exported. Co-located dots cluster correctly.

### Tasks

| ID | Title | Status |
| -- | ----- | ------ |
| T-0161 | Historical event CRUD — author in project, drag on timeline, `#C8A97A` dot | ✅ Verified |
| T-0162 | Imported timeline row — render events as grey row below project row | ✅ Verified |
| T-0163 | Epoch offset dialog — import flow with window intersection preview | ✅ Verified |
| T-0164 | Multiple imported timeline rows — distinct grey shades, hide/show toggle | ✅ Verified |
| T-0165 | Timeline export — produce `.scrivi-timeline.json` | ✅ Verified |
| T-0166 | Co-located dot clustering — hexagonal ring layout, count badge | 🟡 Implemented - Not Verified |
| T-0169 | Hover tooltips for historical event and imported event dots | 🔴 Open |
| T-0170 | Scene/Chapter split and merge — Cmd-Enter splits at cursor, Cmd-Backspace merges | 🔴 Open |

### Issues

| ID | Title | Status |
| -- | ----- | ------ |
| I-0036 | Color.clear overlay blocks all timeline interaction; historical event at midpoint; no title popup | ✅ Verified |
| I-0037 | Hover tooltips for historical event and imported event dots display in wrong position and show wrong content | 🔴 Open |
| I-0038 | New scenes created via Cmd-Enter do not appear as dots on the Timeline | 🔴 Open |

### Acceptance Criteria

- [x] "New Historical Event Here" from the empty-area context menu creates an event and shows it as a `#C8A97A` dot.
- [x] Historical events are draggable on the timeline (their `gapMs` and `offsetMs` update).
- [x] Historical event context menu: Edit, Set Story Time, Delete.
- [x] Importing a `.scrivi-timeline.json` file triggers the epoch offset dialog showing source name, epoch label, and a window-intersection preview.
- [x] Imported events appear as a grey row below the project row; source name is labelled at left.
- [x] Only events whose computed project story-time falls within the current window are rendered.
- [x] Two imported timelines appear as two distinct rows with distinguishably different grey shades.
- [x] A row can be hidden/shown via the panel header menu without removing the imported file.
- [x] Export produces a syntactically valid `.scrivi-timeline.json` containing all project scenes and historical events.
- [ ] Co-located dots form a hexagonal cluster: center dot on line, ring 1 has 6 clockwise from 12 o'clock, ring 2 has 12.
- [ ] When cluster height exceeds panel height, a count badge appears on the center dot.
- [ ] Zooming in resolves clusters into individual dots.
- [ ] Creating a scene via Cmd-Enter immediately adds a dot at the end of the timeline.
- [ ] Creating a chapter via Shift-Cmd-Enter immediately adds a dot at the end of the timeline.
- [ ] Cmd-Enter in middle of scene splits text at cursor into two scenes in the same chapter.
- [ ] Shift-Cmd-Enter in middle of scene splits text; subsequent scenes move to a new chapter.
- [ ] Cmd-Backspace at beginning of a scene (not first in chapter) merges it with the previous scene.
- [ ] Cmd-Backspace at beginning of first scene in chapter does nothing.
- [ ] Shift-Cmd-Backspace at beginning of first scene in a chapter merges the chapter with the previous chapter.
- [ ] Shift-Cmd-Backspace at beginning of first scene in Chapter 1 does nothing.
- [ ] Hover tooltip appears above historical event dots showing title and story-time offset.
- [ ] Hover tooltip appears above imported event dots showing the event title and source timeline name.

---

*Last Updated: 2026-06-14 (AC-1–AC-9 verified; I-0038 and T-0170 added)*
