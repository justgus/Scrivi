# Sprint Backlog

Sprints listed here are in 🔵 Planning status — defined and ready to activate, but not yet started.

---

| Sprint | Title | Epic | Status |
| ------ | ----- | ---- | ------ |
| SP-041 | Story Structure bands — overlay, drag borders, band assignment | EP-016 | 🔵 Planning |
| SP-042 | Historical events, imported timelines, export, and clustering | EP-016 | 🔵 Planning |
| SP-043 | EP-016 verification and polish | EP-016 | 🔵 Planning |

---

## SP-041: Story Structure Bands — Overlay, Drag Borders, Band Assignment

**Status:** 🔵 Planning
**Epic:** EP-016
**Goal:** Story Structure bands are fully interactive. The writer can select a structure, see colored bands rendered behind the timeline, drag band borders to resize, and assign scenes to bands by drag-up or context menu.

### Tasks

| ID | Title | Status |
| -- | ----- | ------ |
| T-0156 | Story Structure band overlay — colored bands, label row, Structure selector menu | 🔵 Backlog |
| T-0157 | Band border drag — proportional resize, persistence | 🔵 Backlog |
| T-0158 | Band assignment by drag-up to label — ring color on dot | 🔵 Backlog |
| T-0159 | Band assignment via context menu ("Assign to Act…" / "Unassign") | 🔵 Backlog |
| T-0160 | Scene dot context menu — full menu (assign, set time delta, set story time, view scene, copy/paste) | 🔵 Backlog |

### Acceptance Criteria

- [ ] The Structure selector menu lists all 7 built-in structures plus Custom. Selecting one writes `story-structure.json` and renders bands immediately.
- [ ] Band labels appear at the top of the panel in a dedicated label row when a structure is active.
- [ ] Band proportions auto-divide equally on first application.
- [ ] Dragging a band border updates adjacent band proportions and persists to disk.
- [ ] Dragging a scene dot upward to a band label assigns the scene; the dot displays a colored ring.
- [ ] Dragging the dot back to the timeline line removes the assignment.
- [ ] "Assign to Act…" context menu shows all bands with a checkmark on the current assignment.
- [ ] "Unassign" context menu item removes the band assignment and ring.
- [ ] Removing the active structure removes the band overlay; scene `gapMs`, `offsetMs`, and `bandID` values in metadata are unchanged.
- [ ] Full scene dot context menu is present with all items from §7.6 of the design doc.
- [ ] "Set Time Delta…" in the context menu opens the Time Delta Picker correctly.

---

## SP-042: Historical Events, Imported Timelines, Export, and Clustering

**Status:** 🔵 Planning
**Epic:** EP-016
**Goal:** Historical events can be authored in the project. External timelines can be imported with epoch offsets and rendered as distinct rows. The project timeline can be exported. Co-located dots cluster correctly.

### Tasks

| ID | Title | Status |
| -- | ----- | ------ |
| T-0161 | Historical event CRUD — author in project, drag on timeline, `#C8A97A` dot | 🔵 Backlog |
| T-0162 | Imported timeline row — render events as grey row below project row | 🔵 Backlog |
| T-0163 | Epoch offset dialog — import flow with window intersection preview | 🔵 Backlog |
| T-0164 | Multiple imported timeline rows — distinct grey shades, hide/show toggle | 🔵 Backlog |
| T-0165 | Timeline export — produce `.scrivi-timeline.json` | 🔵 Backlog |
| T-0166 | Co-located dot clustering — hexagonal ring layout, count badge | 🔵 Backlog |

### Acceptance Criteria

- [ ] "New Historical Event Here" from the empty-area context menu creates an event and shows it as a `#C8A97A` dot.
- [ ] Historical events are draggable on the timeline (their `gapMs` and `offsetMs` update).
- [ ] Historical event context menu: Edit, Set Story Time, Delete.
- [ ] Importing a `.scrivi-timeline.json` file triggers the epoch offset dialog showing source name, epoch label, and a window-intersection preview.
- [ ] Imported events appear as a grey row below the project row; source name is labelled at left.
- [ ] Only events whose computed project story-time falls within the current window are rendered.
- [ ] Two imported timelines appear as two distinct rows with distinguishably different grey shades.
- [ ] A row can be hidden/shown via the panel header menu without removing the imported file.
- [ ] Export produces a syntactically valid `.scrivi-timeline.json` containing all project scenes and historical events.
- [ ] Co-located dots form a hexagonal cluster: center dot on line, ring 1 has 6 clockwise from 12 o'clock, ring 2 has 12.
- [ ] When cluster height exceeds panel height, a count badge appears on the center dot.
- [ ] Zooming in resolves clusters into individual dots.

---

## SP-043: EP-016 Verification and Polish

**Status:** 🔵 Planning
**Epic:** EP-016
**Goal:** Full end-to-end verification against all EP-016 acceptance criteria. Fix any gaps discovered during verification. Close the Epic.

### Tasks

| ID | Title | Status |
| -- | ----- | ------ |
| T-0167 | EP-016 verification | 🔵 Backlog |

### Acceptance Criteria

- [ ] All EP-016 acceptance criteria verified by the user.
- [ ] `ctest` fully green.
- [ ] Xcode build clean on macOS target.
- [ ] No regressions in existing EP-001 through EP-015 functionality.

---

*Last Updated: 2026-06-11 (SP-039 and SP-040 closed; SP-041–SP-043 remain)*
