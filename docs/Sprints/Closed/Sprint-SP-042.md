# SP-042: Historical Events, Imported Timelines, Export, and Clustering

**Status:** ✅ Closed
**Epic:** EP-016
**Start Date:** 2026-06-12
**End Date:** 2026-06-16
**Goal:** Historical events can be authored in the project. External timelines can be imported with epoch offsets and rendered as distinct rows. The project timeline can be exported. Co-located dots cluster correctly.

---

### Tasks

| ID | Title | Status |
| -- | ----- | ------ |
| T-0161 | Historical event CRUD — author in project, drag on timeline, `#C8A97A` dot | ✅ Verified |
| T-0162 | Imported timeline row — render events as grey row below project row | ✅ Verified |
| T-0163 | Epoch offset dialog — import flow with window intersection preview | ✅ Verified |
| T-0164 | Multiple imported timeline rows — distinct grey shades, hide/show toggle | ✅ Verified |
| T-0165 | Timeline export — produce `.scrivi-timeline.json` | ✅ Verified |
| T-0166 | Co-located dot clustering — hexagonal ring layout, count badge | ✅ Verified |
| T-0169 | Hover tooltips for historical event and imported event dots | ✅ Verified |
| T-0170 | Scene/Chapter split and merge — Cmd-Enter splits at cursor, Cmd-Backspace merges | ✅ Verified |

### Issues

| ID | Title | Status |
| -- | ----- | ------ |
| I-0036 | Color.clear overlay blocks all timeline interaction; historical event at midpoint; no title popup | ✅ Verified |
| I-0037 | Hover tooltips for historical event and imported event dots display in wrong position and show wrong content | ✅ Verified |
| I-0038 | New scenes created via Cmd-Enter do not appear as dots on the Timeline | ✅ Verified |
| I-0039 | Clustering only applies to unanchored dots; anchored scene dots with overlapping positions are not grouped | ✅ Verified |
| I-0040 | Historical event dots on the main timeline are not clustered when co-located | ✅ Verified |
| I-0041 | Imported timeline dots on secondary and tertiary rows are not clustered when co-located | ✅ Verified |
| I-0042 | Timeline tooltip shows "Scene N" fallback title instead of first-line text; rename not reflected | ✅ Verified |
| I-0043 | Splitting a chapter creates a duplicate chapter number instead of renumbering; no confirmation dialog | ✅ Verified |
| I-0044 | Three cluster layout defects: wrong direction, anchor-only grouping, center dot on the line | ✅ Verified |

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
- [x] Co-located dots form a hexagonal cluster: center dot on line, ring 1 has 6 clockwise from 12 o'clock, ring 2 has 12.
- [x] Creating a scene via Cmd-Enter immediately adds a dot at the end of the timeline.
- [x] Creating a chapter via Shift-Cmd-Enter immediately adds a dot at the end of the timeline.
- [x] Cmd-Enter in middle of scene splits text at cursor into two scenes in the same chapter.
- [x] Shift-Cmd-Enter in middle of scene splits text; subsequent scenes move to a new chapter.
- [x] Cmd-Backspace at beginning of a scene (not first in chapter) merges it with the previous scene.
- [x] Cmd-Backspace at beginning of first scene in chapter does nothing.
- [x] Shift-Cmd-Backspace at beginning of first scene in a chapter merges the chapter with the previous chapter.
- [x] Shift-Cmd-Backspace at beginning of first scene in Chapter 1 does nothing.
- [x] Hover tooltip appears above historical event dots showing title and story-time offset.
- [x] Hover tooltip appears above imported event dots showing the event title and source timeline name.

**Not completed (deferred to SP-043):**
- [ ] When cluster height exceeds panel height, a count badge appears on the center dot.
- [ ] Zooming in resolves clusters into individual dots.

---

### Retrospective

**Completed:**
- All 8 tasks implemented and verified (T-0161–T-0166, T-0169, T-0170)
- All 9 issues resolved and verified (I-0036–I-0044)
- 20 of 22 acceptance criteria met

**Returned to Backlog:**
- Count badge when cluster height exceeds panel height — not reached; deferred to SP-043
- Zoom resolves clusters into individual dots — not reached; deferred to SP-043

**What went well:**
- Clustering was implemented cleanly with a unified `MainRowItem` enum covering both scene and historical event dots, keeping the rendering loop simple.
- The transitive grouping fix (contiguous-window pass) correctly handled chains of nearby dots that the original anchor-only algorithm missed.
- The JSON-over-string boundary held cleanly — no backend changes were needed for any of the issues fixed.
- Issue discovery was thorough: I-0039 through I-0044 were all identified and resolved in the same session.

**What to improve:**
- The cluster ring layout had four separate bugs (direction, grouping, base X, position-0 placement) that were found incrementally rather than caught by a single careful design review before implementation. A more deliberate coordinate-system audit before writing layout math would have caught these together.
- Several issue statuses lagged behind actual implementation state at the start of the session; more discipline in updating docs immediately after a fix would reduce confusion.

**Carry-forward notes:**
- SP-043 should address the two deferred ACs (count badge, zoom resolution) and complete EP-016 close.
- `renumberChapterTitlesFrom` is purely in-memory — if the engine ever returns chapter titles in `CreateChapterResult`, that method can be simplified or removed.

---

*Closed: 2026-06-16*
