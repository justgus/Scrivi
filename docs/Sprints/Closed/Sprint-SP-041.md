# SP-041: Story Structure Bands — Overlay, Drag Borders, Band Assignment

**Status:** ✅ Closed
**Epic:** EP-016 — Timeline Panel Full Implementation
**Goal:** Story Structure bands are fully interactive. The writer can select a structure, see colored bands rendered behind the timeline, drag band borders to resize, and assign scenes to bands by drag-up or context menu. Full scene dot context menu is also wired in this sprint.
**Start Date:** 2026-06-12
**End Date:** 2026-06-12
**Capacity:** Single session

---

### Assigned Tasks

| ID | Title | Status |
| -- | ----- | ------ |
| T-0156 | Story Structure band overlay — colored bands, label row, Structure selector menu | ✅ Verified |
| T-0157 | Band border drag — proportional resize, persistence | ✅ Verified |
| T-0158 | Band assignment by drag-up to label — ring color on dot | ✅ Verified |
| T-0159 | Band assignment via context menu ("Assign to Act…" / "Unassign") | ✅ Verified |
| T-0160 | Scene dot context menu — full menu | ✅ Verified |

### Assigned Issues

| ID | Title | Severity | Status |
| -- | ----- | -------- | ------ |
| I-0031 | Story Structure bands not rendered after selecting a structure | High | ✅ Verified |
| I-0032 | Band border cursor does not change on hover | Low | ✅ Verified |
| I-0033 | Drag-up band assignment gesture blocked after tooltip is shown | High | ✅ Verified |
| I-0034 | Drag-up on assigned dot should toggle assignment off regardless of release position | Medium | ✅ Verified |
| I-0035 | No checkmark on currently assigned band in context menu | Medium | ✅ Verified |

### Acceptance Criteria

- [x] The Structure selector menu lists all 7 built-in structures plus Custom. Selecting one writes `story-structure.json` and renders bands immediately.
- [x] Band labels appear at the top of the panel in a dedicated label row when a structure is active.
- [x] Band proportions auto-divide equally on first application.
- [x] Dragging a band border updates adjacent band proportions and persists to disk.
- [x] Dragging a scene dot upward to a band label assigns the scene; the dot displays a colored ring.
- [x] Dragging the dot back to the timeline line removes the assignment.
- [x] "Assign to Act…" context menu shows all bands with a checkmark on the current assignment.
- [x] "Unassign" context menu item removes the band assignment and ring.
- [x] Removing the active structure removes the band overlay; scene `gapMs`, `offsetMs`, and `bandID` values in metadata are unchanged.
- [x] Full scene dot context menu is present with all items from §7.6 of the design doc.
- [x] "Set Time Delta…" in the context menu opens the Time Delta Picker correctly.

### Retrospective

**Completed:**
- T-0156: Band overlay with colored bands, label row, Structure selector menu in panel header
- T-0157: Band border drag with proportional resize and persistence
- T-0158: Band assignment by drag-up with ring color on dot; drag-up toggles unassignment for already-assigned dots
- T-0159: Band assignment via inline Picker context menu with native macOS checkmarks
- T-0160: Full scene dot context menu per §7.6

**Returned to Backlog:** None

**Issues Resolved This Sprint:** 5 (I-0031–I-0035)

**What went well:**
- Band rendering, label row, and Structure selector worked correctly once the JSON boundary format mismatch (I-0031) was identified
- Inline Picker approach for context menu checkmarks (I-0035) was the right call
- User-provided diagnostic approach (hover highlight probe) was effective in localising the drag gesture root cause

**What to improve:**
- I-0033 took multiple fix attempts and two incorrect root cause analyses before the actual cause was found (tooltip popover NSPanel consuming the mouse-down that dismisses it). The correct fix — moving the tooltip to a non-interactive overlay outside the clipped ZStack — was only reached after systematic user-guided diagnosis.
- I-0031 (JSON boundary format) should have been caught by reading the C++ source before writing the Swift encoder/decoder. The backend's `{"bands":[...]}` wrapper was documented in the existing code.
- Several fixes were applied before the root cause was confirmed. CLAUDE.md principle 1 (don't assume) was violated multiple times. Future issue work should read all relevant code and gather user data before proposing a fix.

**Carry-forward notes:**
- The `DragGesture(minimumDistance: 0)` probe on `SceneDotView` was added as a diagnostic tool during I-0033 investigation. It is now doing useful work (clearing hover state on click) and should be retained.
- The tooltip is now a `DotTooltipView` overlay rendered outside the clipped ZStack on the GeometryReader — this is the correct architecture for any future tooltip additions.
- SP-042 begins historical events, imported timelines, export, and clustering. All depend on the timeline panel being stable — SP-041 establishes that baseline.

---

*Closed: 2026-06-12*
