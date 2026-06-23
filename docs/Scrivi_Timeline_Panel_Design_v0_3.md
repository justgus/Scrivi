# Scrivi Timeline Panel Design v0.3

**Project:** Scrivi
**Document:** Timeline Panel — Requirements and Design
**Version:** 0.4
**Status:** Approved baseline
**Date:** 2026-06-11
**Supersedes:** v0.3 (adds scene duration, project timeline defaults, chain propagation; §2, §4.3b, §6.3, §6.2, §7.12)

---

## 1. Purpose

This document defines the requirements, interaction model, persistence schema, and data model for the fully functional Scrivi Timeline Panel. It supersedes `Scrivi_Timeline_Panel_Design_v0_1.md` and governs all Timeline implementation work.

---

## 2. Design Principles

1. **Non-intrusive by default.** The timeline must never interrupt writing. A seat-of-pants writer can open Scrivi, write scenes for months, and never consciously interact with the timeline. It should simply reflect what she has written.
2. **Rich when engaged.** A multi-world, multi-timeline author must be able to manage intersecting histories across projects without structural constraint.
3. **Story time is not narrative order.** The position of a scene on the timeline expresses *when in the story world the scene occurs*, not where it appears in the manuscript. Manuscript order is always controlled through the Scene Navigator. These are independent axes.
4. **Story Structure is cosmetic first, annotative optionally.** Structure bands paint a background interpretation over the timeline. They do not own or constrain scenes.
5. **The timeline is the duration of the story.** There is no fixed calendar unit. The timeline spans exactly the story's events — no more, no less.
6. **Scenes have duration, not just position.** A scene occupies a span of story time, not a point. "Immediately after" means this scene begins when the previous scene ends. Duration defaults silently and is never required of the writer.
7. **All positions are relative and chain-propagating.** When a scene's position or duration changes, every subsequent scene whose position is `offsetSource = "default"` recomputes silently on commit. The writer never has to manually reposition a chain.
8. **Co-location is information.** When many scenes share the same story-time position, that density is meaningful. The rendering must make it visible and navigable, not hide it.

---

## 3. Actors and Use Cases

### 3.1 Actors

| Actor | Profile | Primary Need |
|---|---|---|
| Seat-of-pants writer | Writes scenes in manuscript order as they occur in the story | Timeline reflects her work passively; she may never adjust it |
| Structural author | Plans acts and beats before writing; uses story structure actively | Assigns scenes to acts; uses structure to track story shape |
| Multi-world author | Writes stories that span worlds, eras, intersecting histories | Imports external timelines; sets epoch offsets; manages multiple overlapping timelines |

### 3.2 Core Use Cases

- **UC-01:** Open a project and see existing scenes on the timeline in manuscript order.
- **UC-02:** Add a new scene and see it appear on the timeline immediately after the previous scene.
- **UC-03:** Drag a scene dot to reposition it in story time (before or after other scenes).
- **UC-04:** Drag a scene past the first or last dot and expand the timeline.
- **UC-05:** Apply a Story Structure to the project and see colored bands appear behind the timeline.
- **UC-06:** Drag band borders to reposition act boundaries on the timeline.
- **UC-07:** Assign a scene to a Story Structure band by dragging its dot vertically to the band label, or via context menu.
- **UC-08:** Import an external timeline JSON file and view its events as a distinct row on the timeline panel.
- **UC-09:** Set an epoch offset for an imported timeline so only intersecting events are visible.
- **UC-10:** Export the project's scene timeline as a JSON file for use in another project.
- **UC-11:** View a tooltip or popover for any dot (scene or imported event) showing its title and human-readable story-time offset.
- **UC-12:** Resize the timeline panel by dragging its top edge.
- **UC-13:** Author a historical event directly in the project and see it appear on the timeline.

---

## 4. Functional Requirements

### 4.1 Timeline Strip — Core Display

**FR-001** The Timeline Panel shall render a horizontal line spanning the full panel width. The leftmost point represents the earliest event (scene or historical) in story time; the rightmost point represents the latest.

**FR-002** Each scene in the project shall be represented as a filled circle (dot) on the project timeline row. The dot is positioned at the scene's story-time offset relative to the project epoch.

**FR-003** When a project has only one scene, the dot shall appear on the far left of the panel with the line extending to the right.

**FR-004** When a project has two or more scenes and no story-time offsets have been manually set, scenes shall be evenly spaced along the timeline in manuscript order (left = first scene, right = last scene).

**FR-005** Scene dots shall be visually distinct from imported-event dots and historical event dots. Scene dots use the accent color; imported event dots use a neutral grey; historical event dots use a muted warm tone (distinct from both).

**FR-006** A scene dot whose story-time position was explicitly set (manually or inferred) shall display a small indicator distinguishing it from a default-positioned dot.

**FR-007** When a scene is assigned to a Story Structure band, the scene dot shall display a colored ring matching the band color.

**FR-008** The timeline shall be horizontally scrollable when the number of events exceeds the visible panel width.

**FR-009** The timeline shall support pinch-to-zoom (trackpad/touch) and scroll-to-zoom to adjust the visible time window without changing underlying offsets.

**FR-010** The timeline line and all its dots shall remain vertically centered within the panel at all panel heights.

### 4.2 Panel Resizing

**FR-011** The timeline panel shall be vertically resizable. The user drags the top edge of the panel to change its height.

**FR-012** The minimum panel height is the sum of:
- The height of the Story Structure band labels row (if a structure is active)
- The height of the timeline row (line + dots, accounting for clustered stacks — see FR-040)
- The height of one imported timeline row (if any are present)
- Standard vertical padding between rows

**FR-013** The maximum panel height is unconstrained by the design; practical limits are governed by the app window size.

**FR-014** When the panel height increases beyond the minimum, additional space shall be distributed evenly above and below the timeline row, keeping the content vertically centered.

**FR-015** When a Story Structure is active, the band label row shall always appear at the top of the panel, immediately below the panel header.

### 4.3 Scene Dot Interaction

**FR-016** Scene dots shall be draggable horizontally along the timeline line.

**FR-017** Dragging a scene dot shall update its story-time position. When the drag completes (finger/mouse released), a Time Delta Picker popover shall appear anchored to the dot, allowing the writer to confirm or refine the elapsed interval between the preceding scene and the dragged scene (see §4.3a and §7.12). It shall not change the scene's position in the manuscript (chapter/scene order is unaffected).

**FR-018** Dragging a scene dot past the rightmost dot shall trigger an "Expand Timeline Forward" popover. The popover shall allow the writer to specify a duration by which the timeline extends (e.g., "1 day", "2 weeks", "3 years"). The dragged scene shall be placed at the new end of the timeline.

**FR-019** Dragging a scene dot past the leftmost dot shall trigger an "Expand Timeline Backward" popover with equivalent behavior.

**FR-020** The timeline shall be contractable by dragging the first or last scene dot inward toward the opposite end, compressing the visible time span.

**FR-021** Scene dots shall be reorderable in story time by dragging them to any position between two existing dots, or to the first or last position. This is a story-time reorder only; manuscript order is unaffected.

**FR-022** A scene dot shall display a tooltip or popover on hover/tap showing: scene title, chapter, human-readable story-time offset from epoch (see FR-024), and manuscript position (e.g., "Chapter 2, Scene 4").

### 4.3a Time Delta Picker

The Time Delta Picker is a popover that appears after the writer completes a horizontal drag of a scene dot. It allows the writer to specify the elapsed interval between the preceding scene and the dragged scene. The picker does not appear after vertical drags (band assignment) or after the Expand Timeline popover resolves.

**Design rationale (supersedes tile-grid approach):** A fixed grid of twelve labelled tiles ("An hour later," "Later that day," …) proved too coarse for real writing — it cannot express "2 hours later," "that evening," or "around 3am" without forcing the writer into a separate compound-entry mode. The approved design uses a single spinner row (amount + unit + direction) that covers any interval, supplemented by six named time-of-day anchors for poetic precision. This eliminates the two-mode tile/precise split.

**FR-022a** When a horizontal drag of a scene dot completes, the Time Delta Picker shall appear as a popover anchored to the dot circle.

**FR-022b** The picker header shall display:
`"Time since: [previous scene title]"`

**FR-022c** The picker shall present a single spinner row:
```
[ amount ] [ Unit ▾ ] [ Later/Before ▾ ] [Set]
```
- **amount** — integer text field, defaults to the nearest natural fit of the drag delta (see FR-022d).
- **Unit** — menu: Minutes · Hours · Days · Weeks · Months · Years.
- **Later/Before** — menu: Later · Before. Controls the sign of the delta.
- **Set** — applies `previousScene.offsetMs ± (amount × unit.ms)` and closes the picker.

**FR-022d** When the picker opens, the spinner shall be pre-populated to the nearest natural fit of the drag-computed delta from the previous scene:

| Drag delta range | Initialised unit |
|---|---|
| ≥ 1 year | Years, rounded down |
| ≥ 1 month | Months, rounded down |
| ≥ 1 week | Weeks, rounded down |
| ≥ 1 day | Days, rounded down |
| ≥ 1 hour | Hours, rounded down |
| < 1 hour | Minutes, rounded down (minimum 1) |

Direction initialises to Later if the drag moved the dot forward in time, Before if backward.

**FR-022e** All spinner-based results are computed relative to `previousScene.offsetMs`, not the raw drag position:
```
result = previousScene.offsetMs + (amount × unit.ms)   [Later]
result = previousScene.offsetMs − (amount × unit.ms)   [Before]
```

**FR-022f** The picker shall present a named time-of-day anchor section below the spinner, labelled "Or jump to:". Anchors are computed as a fixed delta from `previousScene.offsetMs` and represent named moments in the story day. The approved anchor set is:

| Anchor label | Delta from previous scene | Story-day interpretation |
|---|---|---|
| that morning | +6 hours | Same story-day, 6am |
| that evening | +18 hours | Same story-day, 6pm |
| that night | +22 hours | Same story-day, 10pm |
| before dawn | +28 hours | Next story-day, 4am |
| around 3am | +27 hours | Next story-day, 3am |
| the next morning | +30 hours | Next story-day, 6am |

Selecting an anchor applies `previousScene.offsetMs + anchor.deltaMs` and closes the picker. Anchors are always relative to the previous scene, never to the drag position.

**FR-022g** The picker shall include a "Keep position" option, which accepts the raw drag position without modification. This sets `offsetSource = "manual"` and closes the picker.

**FR-022h** The picker shall include a "Reset" option, which discards the drag and returns the scene to `offsetSource = "default"` (immediately after the preceding scene in manuscript order).

**FR-022i** If the writer dismisses the picker by clicking or tapping outside it, the behavior is identical to "Keep position."

**FR-022j** The `offsetSource` field shall be set as follows after picker interaction:

| Action | `offsetSource` |
|---|---|
| Spinner Set | `"manual"` |
| Anchor selected | `"manual"` |
| Keep position | `"manual"` |
| Reset | `"default"` |

**FR-022k** The Time Delta Picker shall also be accessible from the scene dot context menu as "Set Time Delta…" for writers who prefer not to drag.

### 4.3b Scene Duration and Chain Propagation

**FR-022l** Each scene shall have a `durationMs` field in its `storyTime` block representing how long the scene occupies in story time. The default value is the project's `defaultSceneDurationMs` setting (see §6.2a).

**FR-022m** "Immediately after the previous scene" means:
```
thisScene.offsetMs = previousScene.offsetMs + previousScene.durationMs
```
This is the semantics of `offsetSource = "default"`. No scene with `offsetSource = "default"` stores a meaningful `offsetMs` on disk — it is always derived at load time.

**FR-022n** The Time Delta Picker shall include a scene duration row below the position row:
```
Scene duration: [ 1 ] [ Hours ▾ ]
```
This row is always visible but visually secondary. Changing the duration and pressing Set updates both the position and the duration atomically.

**FR-022o** The picker shall include an "Immediately after" option that sets `offsetSource = "default"` and `durationMs` to the project default. This is the canonical way to return a scene to the chain. It replaces "Reset" as the primary action for "put this back where it belongs."

**FR-022p** "Reset" (now secondary, renamed "Reset to default") sets `offsetSource = "default"` and `durationMs` to the project default. Functionally identical to "Immediately after" — both labels are kept because writers use both phrases.

**FR-022q** When a writer commits a position or duration change to scene N, the model shall silently recompute the `offsetMs` of every subsequent scene in the chain whose `offsetSource = "default"`, propagating forward until a manually-positioned scene is encountered or the end of the scene list is reached. This propagation happens on commit, not during drag.

**FR-022r** Chain propagation order: scenes are processed in manuscript order (not story-time order), because "immediately after the previous scene in the manuscript" is the semantic meaning of `offsetSource = "default"`.

### 4.3c Project Timeline Defaults

**FR-022s** The project shall store two timeline default settings in `project.json` under a `timelineDefaults` key:

- `defaultSceneDurationMs` — the duration assigned to new scenes and used when `offsetSource = "default"`. Default: `3_600_000` (1 hour).
- `dragPositioningMode` — controls how a drag maps to a time delta. Values:
  - `"proportional"` — the drag distance maps proportionally to the current timeline span.
  - `"fixed"` — each drag pixel maps to a fixed number of milliseconds (user-configurable). Default: `"proportional"`.

**FR-022t** A project settings UI shall allow the writer to change `defaultSceneDurationMs` and `dragPositioningMode`. This is in the Project Settings sheet, not the timeline panel itself.

### 4.4 Story-Time Display Format

**FR-023** Story-time offsets shall be displayed as human-readable durations relative to the project epoch. Example formats:

| `offsetMs` value | Displayed as |
|---|---|
| 0 | "Story opens" |
| 3 600 000 | "1 hour after story opens" |
| 90 000 000 | "1 day, 1 hour after story opens" |
| -86 400 000 | "1 day before story opens" |
| 31 536 000 000 | "1 year after story opens" |

**FR-024** The display format shall use the largest meaningful unit and remainder, rounding sub-minute durations to the nearest minute. The epoch label (default: "story opens") is drawn from `timeline.meta.json`.

**FR-025** For scenes with `offsetSource = "default"`, the tooltip shall display "immediately after [previous scene title]" rather than a computed duration.

### 4.5 Story-Time Inference (Stub)

**FR-026** Each scene's story-time offset shall have a source field: `default`, `manual`, or `inferred`.

**FR-027** A stub interface shall exist in `scene.meta.json` for a future AI inference result: an extracted phrase (`inferenceHint`) and a confidence score (`inferenceConfidence`). These fields shall be null until the inference feature is implemented.

**FR-028** When `offsetSource` is `inferred`, the dot shall carry a visual indicator (e.g., a subtle dashed ring) distinguishing it from manually set offsets.

**FR-029** When `offsetSource` is `default`, the system shall position the scene immediately after the preceding scene with an offset of +1 millisecond. The UI shall display "immediately after [previous scene title]."

### 4.6 Co-located Event Clustering

These requirements address the case where two or more events share the same (or visually indistinguishable) story-time position. This is common in stories that span years when multiple scenes occur within the same short window.

> **Revision (T-0174, supersedes prior FR-031, FR-033, FR-035):** The original spec rendered
> clusters as concentric hexagonal rings of dots. In practice a single ring (≤7 dots) was
> readable, but a second ring produced an illegible "blob" — made worse by per-dot adornments
> (band rings, manual/inferred offset rings, selection highlight) appearing inconsistently
> across members. The cluster is now rendered as a single **aggregate dot** that is compact
> (one timeline X position) and scales to any member count. The ring-of-dots layout is retained
> only as the transient **fan-out** state (FR-035b) and as the permanent result of zoom-resolve
> (FR-032). Co-location detection (FR-030, FR-032) and the no-line-shift rule (FR-033) are
> unchanged in intent.

**FR-030** When two or more events occupy the same story-time position on the timeline, they shall be rendered as a single aggregate representation rather than overlapping dots.

**FR-031 (revised)** A co-located group of two or more members shall be rendered as a single **aggregate dot** at the shared X position:

- The aggregate dot is compact — it occupies one timeline position regardless of member count.
- The aggregate dot's core shall be rendered **slightly larger** than a regular (single-scene) timeline dot. The larger core signals "this is a group of scenes, not one scene," and keeps the aggregate visually distinct from the per-dot decoration rings used on individual dots (the placement ring for `manual`/`inferred` offsets, FR-028; the band-membership ring, FR-007/FR-047) — so the aggregate's own arc ring is not confused with those decorations.
- A **count number** is shown on/at the aggregate dot indicating the total number of members. (Numbers are the only text permitted on the timeline line; titles are not.)
- A **segmented arc ring** surrounds the aggregate dot: one equal-width segment per member, ordered by story order (the same order used by the clustering pass). For *N* members each segment spans `360° / N`.
- All members receive a segment — both scene dots and historical-event dots. Segment base tint may hint at member type (scene vs. historical).
- The arc ring is a **display surface only — it is not interactive.**

**FR-031a (selection arc)** When a member of an aggregate is the currently selected scene (`viewportSceneID`), that member's arc segment shall be highlighted in the selection color. Because segments are strictly ordered, the *position* of the lit segment indicates *which* member is selected, not merely that one is. Selecting a member from elsewhere (Scene Navigator, manuscript) lights the corresponding segment on the collapsed aggregate; it does **not** auto-expand the aggregate.

**FR-032** For the purposes of clustering, two events are considered co-located if their pixel positions on the current zoom level are within one dot-diameter of each other. As the writer zooms in, aggregates may resolve into individual dots.

**FR-033** The aggregate shall be centred on the timeline line; the timeline line position shall not shift when aggregates form. Because the aggregate is compact (arc ring only, no stacked rings of dots), it does not require the panel to grow to accommodate it.

**FR-034 (revised — drill-in)** Members of an aggregate remain reachable through interaction rather than by always rendering every dot:

- **Hover the aggregate → fan-out.** Hovering an aggregate dot fans its members out into the ring/cluster layout (the retained hexagonal positions) as a transient overlay.
- Once fanned out, each member dot behaves like a normal individual dot: **hover it for its tooltip**, **click it to select/navigate** to that scene (drives `viewportSceneID`, manuscript scroll, and Navigator highlight per the bidirectional-selection model), drag to reposition, context menu.
- The fan-out collapses back to the aggregate dot when the pointer leaves.

**FR-035 (revised)** The count number (FR-031) replaces the prior conditional count badge — it is always present on an aggregate. The selection arc (FR-031a) replaces the prior need to expand to see selection state. Individual dots remain accessible via fan-out (FR-034) and via zoom-resolve (FR-032).

**FR-035b (fan-out layout)** The fan-out reuses the concentric hexagonal ring positions previously specified for static clusters: position 1 at the centre, positions 2–7 in the first ring (clockwise from 12 o'clock), positions 8–19 in the second ring, ring *n* holding 6*n* dots. This layout now appears only transiently during fan-out, not as the resting representation.

### 4.7 Story Structure Bands

**FR-036** The project may have zero or one active Story Structure. Applying a Story Structure is a project setting stored in `story-structure.json`.

**FR-037** The following Story Structures shall be available in v1:

| ID | Name | Band count |
|---|---|---|
| `three-act` | Three Act Structure | 3 |
| `five-act` | Five Act Structure | 5 |
| `heroes-journey` | Hero's Journey | 12 |
| `save-the-cat` | Save the Cat | 15 |
| `freytag` | Freytag's Pyramid | 5 |
| `kishotenketsu` | Kishōtenketsu | 4 |
| `in-medias-res` | In Medias Res | 4 |
| `custom` | Custom | Writer-defined |

**FR-038** When a Story Structure is applied, the timeline background shall be divided into colored bands, one per structure element. Bands shall auto-divide the current timeline span equally on first application. The layout saves immediately without confirmation.

**FR-039** Band widths may be unequal. Each band stores its width as a proportion (percentage) of the total timeline span. The sum of all band proportions is always 1.0. On first application, each band receives 1/N of the span.

**FR-040** Band borders shall be draggable horizontally to reposition act boundaries. When a border is dragged, the adjacent bands' proportions update accordingly. All other bands are unaffected.

**FR-041** Each band shall have a distinct color drawn from a predefined palette. Colors shall be consistent for a given structure across all projects (e.g., Act I is always a specific blue).

**FR-042** Band labels shall appear in a dedicated label row at the top of the panel (above the timeline line). Labels are left-aligned within their band. Labels shall truncate with an ellipsis when the band is too narrow.

**FR-043** Removing the active Story Structure shall remove all band display. Scene story-time offsets and band assignments already recorded in scene metadata are preserved.

**FR-044** The Custom story structure shall allow the writer to add, remove, rename, and recolor bands. Each band stores an explicit proportion. Custom band definitions are stored in `story-structure.json`.

### 4.8 Band Assignment

**FR-045** A scene may be explicitly assigned to a Story Structure band. Assignment is optional and purely annotative.

**FR-046** Assignment shall be possible via two gestures, which are additive (both are always available):
  - **Drag up:** Drag a scene dot vertically upward toward the band label row. As the dot enters the label row, the hovered band label highlights. Releasing over a highlighted label assigns the scene to that band and returns the dot to the timeline line with its new colored ring. Releasing without a highlighted target cancels the gesture.
  - **Context menu:** Right-click (or long-press on touch) a scene dot to show an "Assign to Act…" submenu listing all bands. Selecting a band assigns. Selecting "Unassign" removes assignment.

**FR-047** When a scene is assigned to a band, the dot displays a colored ring in the band's color.

**FR-048** Band assignment shall be stored in `scene.meta.json` under `storyTime.storyStructure.bandID`.

**FR-049** If the active Story Structure is changed, existing band assignments referencing bands not present in the new structure shall be cleared and logged in the project repair log.

### 4.9 Historical Events

**FR-050** A writer may author historical events directly in the project. Historical events are worldbuilding objects — not scenes — that appear on the timeline.

**FR-051** Historical events are stored as JSON files in `objects/historical-events/`. Each event has a stable ID, title, story-time offset, optional description, and optional tags.

**FR-052** Historical events shall appear on the project timeline row as dots visually distinct from scene dots and imported event dots. The approved visual is a muted warm-toned filled circle (e.g., `#C8A97A`).

**FR-053** Historical events are draggable on the timeline (their story-time offset can be changed). They are not scenes and have no manuscript association.

**FR-054** Historical events shall be included in the exported `.scrivi-timeline.json` file with `kind = "historical"`.

**FR-055** A "New Historical Event" action shall be available from the timeline panel context menu (right-click on empty timeline area) or from the panel header menu.

**FR-056** Editing or deleting a historical event shall be available via its context menu.

### 4.10 Multiple Timeline Rows (Imported Timelines)

**FR-057** Each imported external timeline shall be rendered as a distinct horizontal row below the project timeline row. The project's own events are always in the topmost row.

**FR-058** Each imported timeline row shall display the source timeline's name as a label at the left edge of its row.

**FR-059** Imported event dots within a row are rendered in grey. The grey shade is unique per source timeline to distinguish multiple imports at a glance. Shades shall be visually distinct and accessible (sufficient contrast).

**FR-060** Imported event rows follow the same horizontal scale (zoom and scroll position) as the project timeline row.

**FR-061** Only events whose computed project story-time falls within the current timeline window are rendered in the imported row. Events outside the window are clipped.

**FR-062** Imported event dots are read-only. They cannot be dragged or assigned to bands.

**FR-063** Imported event dots display a tooltip/popover on hover/tap: event title, source timeline name, source `offsetMs`, and computed project story-time (human-readable, using FR-023 format).

**FR-064** The panel minimum height (FR-012) grows by one row height for each active imported timeline.

**FR-065** An imported timeline row may be individually hidden/shown via a toggle in the panel header menu. Hidden rows are not removed; they are suppressed from rendering only.

### 4.11 External Timeline Import and Export

**FR-066** The "Import Timeline" action accepts a `.scrivi-timeline.json` file. After selection, an epoch offset dialog appears before the file is stored in the project.

**FR-067** The epoch offset dialog shows:
- Source timeline name
- Source timeline epoch label
- A signed integer field for the offset (in the unit selected: years, months, days, hours, minutes, milliseconds)
- A preview showing which events will fall within and outside the current project window at the chosen offset

**FR-068** The epoch offset is editable after import via the imported timeline's context menu ("Edit Epoch Offset…").

**FR-069** The "Export Timeline" action produces a `.scrivi-timeline.json` file containing the project's scene events and historical events with their story-time offsets, titles, and notes. The export does not include manuscript prose, identity data, or private metadata.

**FR-070** An imported timeline may be removed from the project via its row's context menu ("Remove Imported Timeline"). Removal deletes the stored JSON file from `objects/imported-timelines/`.

---

## 5. Non-Functional Requirements

**NFR-001** The timeline panel shall render and be interactive within 100ms of project open on a project with up to 500 scenes.

**NFR-002** Drag operations shall update the visual position of dots at 60fps with no perceptible lag on supported hardware.

**NFR-003** Timeline data shall be stored as plain JSON inside the project package, readable and editable externally.

**NFR-004** The timeline panel shall function correctly with 0 scenes (empty state), 1 scene, 2 scenes, and up to 500 scenes.

**NFR-005** A project with no Story Structure applied and no imported timelines shall function with a single timeline row and no background bands.

**NFR-006** All timeline persistence files must survive round-trip through external editors without data loss (standard JSON encoding, no binary fields).

**NFR-007** The clustering algorithm (FR-030 through FR-035) shall compute cluster positions in O(n) time relative to the number of co-located events and shall not cause frame drops during zoom transitions.

---

## 6. Persistence Design

### 6.1 Project Package Layout (additions)

```text
MyNovel.scrivi/
  objects/
    timelines/
      timeline.meta.json                   ← project's own timeline definition and epoch

    story-structures/
      story-structure.json                 ← active structure selection and band layout

    historical-events/
      <id>-<slug>.json                     ← each authored historical event

    imported-timelines/
      <id>-<slug>.scrivi-timeline.json     ← each imported external timeline (with epochOffsetMs)
```

No new top-level folder is needed. All additions fit within the existing `objects/` folder.

### 6.2 `objects/timelines/timeline.meta.json`

Defines the project's own timeline. Created alongside `project.json` when a new project is created.

```json
{
  "schema": "scrivi.timeline.v1",
  "timelineID": "timeline_01J...",
  "projectID": "project_01J...",
  "createdAt": "2026-06-11T00:00:00Z",
  "epochLabel": "Story Open",
  "notes": null
}
```

The project epoch is story-time offset 0 (ms). All scene and historical event offsets are relative to this epoch. `epochLabel` appears in human-readable duration display (e.g., "3 days after Story Open").

**Required fields:** `schema`, `timelineID`, `projectID`, `createdAt`, `epochLabel`

### 6.2a Timeline Defaults in `project.json`

Project-level timeline defaults are stored in `project.json` under a `timelineDefaults` key. They are written at project creation and editable via Project Settings.

```json
"timelineDefaults": {
  "defaultSceneDurationMs": 3600000,
  "dragPositioningMode": "proportional"
}
```

| Field | Type | Description |
|---|---|---|
| `defaultSceneDurationMs` | `int64` | Duration assigned to new scenes and used for `durationSource = "default"` scenes. Default: `3_600_000` (1 hour). |
| `dragPositioningMode` | `string` | `"proportional"` — drag distance maps proportionally to timeline span. `"fixed"` — each pixel maps to a fixed ms value (future). Default: `"proportional"`. |

These fields are optional in `project.json`; if absent, the runtime uses the defaults listed above. This preserves backward compatibility with projects created before this feature.

### 6.3 Story-Time in `scene.meta.json`

The `storyTime` block is added to each scene's existing `scene.meta.json`:

```json
"storyTime": {
  "offsetMs": 0,
  "offsetSource": "default",
  "durationMs": 3600000,
  "durationSource": "default",
  "inferenceHint": null,
  "inferenceConfidence": null,
  "storyStructure": {
    "bandID": null,
    "assignedAt": null
  }
}
```

| Field | Type | Description |
|---|---|---|
| `offsetMs` | `int64` | Milliseconds from project epoch (signed). For `offsetSource = "default"` scenes, this field is derived at load time and not authoritative on disk — it will be recomputed from the previous scene's `offsetMs + durationMs`. |
| `offsetSource` | `string` | `"default"` \| `"manual"` \| `"inferred"` |
| `durationMs` | `int64` | How long this scene occupies in story time (ms). Default: project's `defaultSceneDurationMs` (1 hour). Always positive. |
| `durationSource` | `string` | `"default"` \| `"manual"`. `"default"` means the project default is used; `"manual"` means the writer has explicitly set it. |
| `inferenceHint` | `string?` | Extracted phrase that drove inference — null until inference feature ships |
| `inferenceConfidence` | `float?` | 0.0–1.0 confidence score — null until inference feature ships |
| `storyStructure.bandID` | `string?` | ID of assigned Story Structure band; null if unassigned |
| `storyStructure.assignedAt` | `string?` | ISO 8601 timestamp of band assignment |

The existing `classification.timelineEventIDs` field is retained as a future cross-reference hook. `storyTime.offsetMs` is the authoritative story-time position for manually-positioned scenes. For default-positioned scenes it is derived.

### 6.4 `objects/story-structures/story-structure.json`

Stores the active Story Structure and band layout for this project. Created on first structure application; absent if no structure has been applied.

```json
{
  "schema": "scrivi.storyStructure.v1",
  "activeStructureID": "three-act",
  "customBands": [],
  "bandLayout": [
    {
      "bandID": "act-i",
      "label": "Act I",
      "color": "#5B8DD9",
      "proportion": 0.25
    },
    {
      "bandID": "act-ii",
      "label": "Act II",
      "color": "#D9A05B",
      "proportion": 0.50
    },
    {
      "bandID": "act-iii",
      "label": "Act III",
      "color": "#5BD98D",
      "proportion": 0.25
    }
  ]
}
```

`proportion` is a float in (0, 1). The sum of all `proportion` values must equal 1.0. The absolute story-time extent of each band is derived at render time by multiplying the total timeline span by each band's proportion.

`customBands` holds band definitions when `activeStructureID` is `"custom"`. For built-in structures it is empty.

**Required fields:** `schema`, `activeStructureID`, `bandLayout`

### 6.5 `objects/historical-events/<id>-<slug>.json`

One file per historical event. Created when the writer authors a historical event in the project.

```json
{
  "schema": "scrivi.historicalEvent.v1",
  "eventID": "hevent_01J...",
  "title": "The Battle of the Iron Gate",
  "slug": "battle-of-the-iron-gate",
  "offsetMs": -31536000000,
  "offsetSource": "manual",
  "description": "The siege that broke the western alliance.",
  "tags": ["war", "turning-point"],
  "createdAt": "2026-06-11T00:00:00Z",
  "modifiedAt": "2026-06-11T00:00:00Z"
}
```

**Required fields:** `schema`, `eventID`, `title`, `offsetMs`, `offsetSource`

### 6.6 External Timeline Exchange Format: `.scrivi-timeline.json`

The format for exporting a project timeline and for importing external timelines.

```json
{
  "schema": "scrivi.externalTimeline.v1",
  "timelineID": "timeline_01J...",
  "sourceProjectTitle": "The Shattered Veil",
  "sourceProjectID": "project_01J...",
  "exportedAt": "2026-06-11T00:00:00Z",
  "epochLabel": "The First Sundering",
  "events": [
    {
      "eventID": "event_01J...",
      "title": "Battle of the Iron Gate",
      "offsetMs": 0,
      "kind": "scene",
      "notes": null
    },
    {
      "eventID": "event_01J...",
      "title": "The Sundering of the Twin Moons",
      "offsetMs": -31536000000,
      "kind": "historical",
      "notes": "Precedes the main story by 1000 years"
    }
  ]
}
```

`kind` is `"scene"` for events sourced from a Scrivi scene and `"historical"` for authored historical events.

### 6.7 Imported Timeline Record (stored in project)

When imported, the file is stored in `objects/imported-timelines/` with one added top-level field:

```json
{
  "schema": "scrivi.externalTimeline.v1",
  "epochOffsetMs": -94608000000,
  "visible": true,
  "assignedGreyShade": "#8A8A8A",
  ...original fields...
}
```

| Added field | Description |
|---|---|
| `epochOffsetMs` | Milliseconds added to each event's `offsetMs` to produce a project-relative position |
| `visible` | Whether the imported row is currently shown in the panel |
| `assignedGreyShade` | Hex color assigned at import time to distinguish this source from other imports |

An imported event is rendered if and only if:
```
projectEpoch ≤ (event.offsetMs + epochOffsetMs) ≤ projectEndMs
```

---

## 7. UI / Interaction Model

### 7.1 Panel Layout

```
┌──────────────────────────────────────────────────────────────────────────┐
│ Timeline                                                  [Structure ▾]  │  ← panel header
├──────────────────────────────────────────────────────────────────────────┤
│  ░░░░░░ Act I ░░░░░░│░░░░░░░░░░░ Act II ░░░░░░░░░░░│░░░░ Act III ░░░░  │  ← label row (top, if structure active)
│                                                                          │
│         ●      ●  ●●          ●     ●      ●             ●       ●      │  ← project timeline row (centered)
│  ───────┼──────┼──┼┼──────────┼─────┼──────┼─────────────┼───────┼──── │    (dots may cluster vertically)
│         S1     S2 S3S4        S5    S6     S7            S8      S9     │
│                                                                          │
│  [The Shattered Veil]  ──────○─────────○─────────○──────────────────── │  ← imported row 1
│  [The Iron Chronicle]  ──────────────○───────────────────○────────────  │  ← imported row 2
└──────────────────────────────────────────────────────────────────────────┘
  ↕ drag top edge to resize
```

Key layout rules:
- The project timeline row is always vertically centered in the available space between the label row (or header) and the bottom of the panel.
- Imported rows are below the project row, each occupying a fixed row height.
- The label row is always at the top, always visible when a structure is active.
- Dragging the panel's top edge resizes it; the minimum height adjusts dynamically based on active content.

### 7.2 Dot States

| State | Visual |
|---|---|
| Default position (`offsetSource = "default"`) | Filled accent circle, no ring |
| Manual story-time set (`offsetSource = "manual"`) | Filled accent circle + solid accent ring |
| Inferred story-time (`offsetSource = "inferred"`) | Filled accent circle + dashed accent ring |
| Assigned to band | Filled accent circle + solid colored ring (band color) |
| Assigned to band + manual set | Filled accent circle + solid band-colored ring + inner accent ring |
| Historical event (project-authored) | Filled `#C8A97A` circle |
| Imported event | Filled grey circle (shade unique per source) |
| Selected / hover | Enlarged with popover |

### 7.3 Co-located Cluster Layout

When N events share the same pixel position, they form a cluster expanding vertically upward from the timeline line:

```
Ring 2 (positions 8–19):    ○ ○ ○ ○ ○ ○
                           ○           ○
Ring 1 (positions 2–7):    ○     ○     ○    (clockwise from 12 o'clock)
                           ○           ○
Center (position 1):         ●          ← sits on the timeline line
─────────────────────────────┼──────────
                          timeline line
```

- Ring 1: 6 positions at 60° increments, starting at 12 o'clock, clockwise.
- Ring 2: 12 positions at 30° increments, starting at 12 o'clock, clockwise.
- Ring n: 6n positions, starting at 12 o'clock, clockwise.

When the cluster is taller than the available panel height above the line, a count badge appears at the center dot. Individual dots remain accessible by zooming in until the cluster resolves.

### 7.4 Expand Timeline Popover

Triggered when a scene dot is dragged past the first or last dot.

```
┌──────────────────────────────────┐
│  Expand timeline forward by:     │
│                                  │
│  [  1  ] [Days ▾]                │
│                                  │
│  [Cancel]              [Expand]  │
└──────────────────────────────────┘
```

Duration units available: Minutes, Hours, Days, Weeks, Months, Years.

### 7.5 Band Assignment by Drag

When the writer begins dragging a scene dot upward, the band label row highlights to indicate it is a drop target. Individual band labels highlight as the dot passes over them. On release over a highlighted label, the scene is assigned to that band and the dot returns to the timeline line with its band ring. On release anywhere else, the gesture cancels.

When the writer drags a scene dot that is already assigned to a band, dragging it back to the timeline line (without releasing over a band label) is treated as unassignment.

### 7.6 Context Menu — Scene Dot

```
Assign to Act…  ▸  Act I
                   Act II ✓   (checkmark = currently assigned)
                   Act III
─────────────────────
Unassign from Act
Set Time Delta…
Set Story Time…
View Scene
─────────────────────
Copy Story-Time Position
Paste Story-Time Position
```

"Set Time Delta…" opens the Time Delta Picker (§7.12) without requiring a drag gesture.

### 7.7 Context Menu — Historical Event Dot

```
Edit Historical Event…
Set Story Time…
─────────────────────
Copy Story-Time Position
Delete Historical Event
```

### 7.8 Context Menu — Imported Event Dot

```
[Event title — read only]
View Source Timeline Info…
─────────────────────
Edit Epoch Offset…
Hide This Timeline
Remove Imported Timeline
```

### 7.9 Context Menu — Empty Timeline Area (right-click)

```
New Historical Event Here
Import Timeline…
Export Timeline…
─────────────────────
Set Epoch Label…
```

### 7.10 Empty State

When a project has no scenes and no historical events, the panel shows the line with:
`"Scenes will appear here as you write."`

### 7.11 Story Structure Selector

A `[Structure ▾]` button in the panel header opens a menu:

```
None (remove structure)
─────────────────────
Three Act Structure
Five Act Structure
Hero's Journey
Save the Cat
Freytag's Pyramid
Kishōtenketsu
In Medias Res
─────────────────────
Custom…
```

Selecting a structure applies it immediately with auto-divided equal bands. The layout saves without confirmation. Switching structures preserves story-time offsets but clears band assignments for bands not present in the new structure (see FR-049).

### 7.12 Time Delta Picker

The Time Delta Picker appears anchored to a scene dot after the writer completes a horizontal drag, or when "Set Time Delta…" is chosen from the dot's context menu.

```
┌──────────────────────────────────────────────┐
│  Time since: "The Burning Tavern"            │
│                                              │
│  [ 3 ] [ Hours ▾ ] [ Later ▾ ]  [Set]        │
│  Scene duration:  [ 1 ] [ Hours ▾ ]          │
│                                              │
│  ────────────────────────────────────────    │
│  Or jump to:                                 │
│                                              │
│  ┌─────────────┐ ┌─────────────┐ ┌────────┐  │
│  │that morning │ │that evening │ │that    │  │
│  │             │ │             │ │night   │  │
│  └─────────────┘ └─────────────┘ └────────┘  │
│  ┌─────────────┐ ┌─────────────┐ ┌────────┐  │
│  │before dawn  │ │around 3am   │ │the next│  │
│  │             │ │             │ │morning │  │
│  └─────────────┘ └─────────────┘ └────────┘  │
│                                              │
│  ────────────────────────────────────────    │
│  Immediately after    Keep position   Reset  │
└──────────────────────────────────────────────┘
```

The spinner row initialises to the nearest natural fit of the drag delta. The duration row shows the scene's current duration (default: 1 hour) and can be adjusted independently. Named anchors are always relative to the previous scene's `offsetMs + durationMs`. "Immediately after" sets `offsetSource = "default"` and restores the scene to the chain. "Keep position" accepts the raw drag. "Reset" is equivalent to "Immediately after."

---

## 8. Story Structure Band Definitions (v1)

Default band proportions are equal on first application (1/N each). Colors are fixed per structure.

### Three Act Structure

| Band ID | Label | Default Color |
|---|---|---|
| `act-i` | Act I | `#5B8DD9` |
| `act-ii` | Act II | `#D9A05B` |
| `act-iii` | Act III | `#5BD98D` |

### Five Act Structure

| Band ID | Label | Default Color |
|---|---|---|
| `exposition` | Exposition | `#5B8DD9` |
| `rising-action` | Rising Action | `#8D5BD9` |
| `climax` | Climax | `#D95B5B` |
| `falling-action` | Falling Action | `#D9A05B` |
| `denouement` | Denouement | `#5BD98D` |

### Hero's Journey (12 stages)

| Band ID | Label | Default Color |
|---|---|---|
| `ordinary-world` | Ordinary World | `#5B8DD9` |
| `call-to-adventure` | Call to Adventure | `#7B8DD9` |
| `refusal` | Refusal of the Call | `#9B8DD9` |
| `meeting-mentor` | Meeting the Mentor | `#BD8DD9` |
| `crossing-threshold` | Crossing the Threshold | `#D98DBD` |
| `tests-allies-enemies` | Tests, Allies, Enemies | `#D98D9B` |
| `approach` | Approach | `#D9A05B` |
| `ordeal` | Ordeal | `#D9735B` |
| `reward` | Reward | `#D95B5B` |
| `road-back` | The Road Back | `#A05BD9` |
| `resurrection` | Resurrection | `#7B5BD9` |
| `return` | Return with the Elixir | `#5B8DD9` |

### Save the Cat (15 beats)

| Band ID | Label | Default Color |
|---|---|---|
| `opening-image` | Opening Image | `#5B8DD9` |
| `theme-stated` | Theme Stated | `#6B8DD9` |
| `set-up` | Set-Up | `#7B8DD9` |
| `catalyst` | Catalyst | `#8B8DD9` |
| `debate` | Debate | `#9B8DD9` |
| `break-into-two` | Break Into Two | `#AB8DD9` |
| `b-story` | B Story | `#BD8DD9` |
| `fun-and-games` | Fun and Games | `#D98DBD` |
| `midpoint` | Midpoint | `#D98D9B` |
| `bad-guys-close-in` | Bad Guys Close In | `#D9735B` |
| `all-is-lost` | All Is Lost | `#D95B5B` |
| `dark-night` | Dark Night of the Soul | `#C05B5B` |
| `break-into-three` | Break Into Three | `#A05BD9` |
| `finale` | Finale | `#7B5BD9` |
| `final-image` | Final Image | `#5B8DD9` |

### Freytag's Pyramid

| Band ID | Label | Default Color |
|---|---|---|
| `introduction` | Introduction | `#5B8DD9` |
| `rising-action` | Rising Action | `#8D5BD9` |
| `climax` | Climax | `#D95B5B` |
| `falling-action` | Falling Action | `#D9A05B` |
| `catastrophe` | Catastrophe | `#5BD98D` |

### Kishōtenketsu

| Band ID | Label | Default Color |
|---|---|---|
| `ki` | Ki — Introduction | `#5B8DD9` |
| `sho` | Shō — Development | `#8D5BD9` |
| `ten` | Ten — Twist | `#D95B5B` |
| `ketsu` | Ketsu — Conclusion | `#5BD98D` |

### In Medias Res

| Band ID | Label | Default Color |
|---|---|---|
| `action-open` | Action Open | `#D95B5B` |
| `backstory` | Backstory | `#5B8DD9` |
| `rising-action` | Rising Action | `#8D5BD9` |
| `resolution` | Resolution | `#5BD98D` |

---

## 9. ScriviCore Backend Responsibilities

The C++ backend (`ScriviCore`) owns all timeline persistence. The Swift layer owns all rendering and interaction.

### 9.1 ScriviCore API (new operations)

All operations return JSON-over-string envelopes per the established boundary protocol (`Scrivi_Architecture_v0_3.md`).

```cpp
// Timeline meta
std::string getTimeline(const char* projectID);
std::string setTimelineEpochLabel(const char* projectID, const char* label);

// Scene story-time
std::string setSceneStoryTime(const char* projectID, const char* sceneID,
                               int64_t offsetMs, const char* source);
std::string getSceneStoryTime(const char* projectID, const char* sceneID);
std::string clearSceneStoryTime(const char* projectID, const char* sceneID);

// Band assignment
std::string assignSceneToBand(const char* projectID, const char* sceneID,
                               const char* bandID);
std::string unassignSceneFromBand(const char* projectID, const char* sceneID);

// Story Structure
std::string getStoryStructure(const char* projectID);
std::string setStoryStructure(const char* projectID, const char* structureID,
                               const char* bandLayoutJSON);
std::string updateBandLayout(const char* projectID, const char* bandLayoutJSON);
std::string removeStoryStructure(const char* projectID);

// Historical events
std::string createHistoricalEvent(const char* projectID, const char* eventJSON);
std::string updateHistoricalEvent(const char* projectID, const char* eventID,
                                   const char* eventJSON);
std::string deleteHistoricalEvent(const char* projectID, const char* eventID);
std::string listHistoricalEvents(const char* projectID);

// External timelines
std::string importExternalTimeline(const char* projectID, const char* timelineJSON,
                                    int64_t epochOffsetMs);
std::string updateImportedTimelineOffset(const char* projectID, const char* timelineID,
                                          int64_t epochOffsetMs);
std::string setImportedTimelineVisible(const char* projectID, const char* timelineID,
                                        bool visible);
std::string listImportedTimelines(const char* projectID);
std::string removeImportedTimeline(const char* projectID, const char* timelineID);
std::string exportProjectTimeline(const char* projectID);
```

### 9.2 ScriviCore Storage Rules

- `timeline.meta.json` is created alongside `project.json` when a project is created.
- `story-structure.json` is created on first Story Structure application; absent until then.
- `objects/historical-events/` is created on first historical event; absent until then.
- `objects/imported-timelines/` is created on first import; absent until then.
- All files are written atomically using the existing `AtomicWrite` utility.
- Scene `storyTime` blocks are written into the scene's existing `scene.meta.json` — no separate file per scene.

---

## 10. Open Questions

All open questions from v0.1 have been resolved. No open questions remain in v0.3.

---

## 11. Out of Scope for v1

- AI/NLP inference of story-time from prose text (stubbed via `offsetSource`, `inferenceHint`, `inferenceConfidence`)
- Character-timeline intersection views
- Location-timeline intersection views
- CloudKit sync of timeline data
- Collaborative timeline editing
- Per-timeline Story Structure application (one structure per project in v1)

---

## 12. Relationship to Existing Documents

| Document | Relationship |
|---|---|
| `Scrivi_Project_Package_Structure_v0_1.md` | This document adds `objects/timelines/`, `objects/story-structures/`, `objects/historical-events/`, and `objects/imported-timelines/` to the approved package layout. |
| `Scrivi_Minimum_Schema_Set_v0_1.md` | Timeline schemas were deferred (Section 13 of that doc). This document defines them. |
| `Scrivi_Architecture_v0_3.md` | JSON-over-string boundary protocol governs all ScriviCore API calls defined here. |
| `Scrivi_Cpp24_Core_API_Sketch_v0_3.md` | New timeline operations follow the same API shape and return-envelope pattern. |

---

## 13. Success Criteria

This design is considered successfully implemented when:

1. A writer with no scenes sees an empty timeline panel with a helpful empty-state message.
2. A writer with scenes sees dots on the timeline in manuscript order without any configuration.
3. A writer can drag a dot to a new story-time position and the position persists across project close/reopen.
4. After completing a drag, the Time Delta Picker appears. The spinner is pre-populated to the nearest natural fit of the drag delta. Adjusting the amount, unit, or direction and pressing Set moves the dot to the correctly computed position relative to the previous scene. Selecting a named anchor (e.g., "that evening") moves the dot to the previous scene's offset + the anchor delta. "Keep position" and "Reset" work correctly. Dismissing without selecting preserves the drag position.
5. "Set Time Delta…" from the context menu opens the Time Delta Picker without a drag gesture.
6. Dot tooltips display human-readable story-time durations (e.g., "3 days, 4 hours after Story Open").
5. A writer can apply Three Act Structure and see three colored bands appear in the label row.
6. A writer can drag a band border to reposition act boundaries; proportions update accordingly.
7. A writer can assign a scene to a band by dragging its dot to the band label, and the ring color updates.
8. A writer can assign a scene to a band via the context menu with the same result.
9. Multiple scenes at the same story-time position form a hexagonal cluster expanding upward from the line.
10. A writer can author a historical event and see it appear as a `#C8A97A` dot on the timeline.
11. A writer can import an external `.scrivi-timeline.json` file; its events appear in a separate row below the project row.
12. Only events within the project's timeline window are visible from imported timelines.
13. Multiple imported timelines appear as distinct rows with distinct grey shades.
14. An imported row can be hidden/shown without removing it from the project.
15. A writer can export the project timeline and import it into another project.
16. Removing a Story Structure removes bands but preserves all story-time offsets and band assignments in scene metadata.
17. The panel resizes when the user drags its top edge; minimum height adjusts based on active content.
18. All timeline data survives an external round-trip (files opened and saved in a text editor).

---

*Document Status: Approved baseline — v0.3 finalized as of 2026-06-11.*
