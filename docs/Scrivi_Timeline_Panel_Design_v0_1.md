# Scrivi Timeline Panel Design v0.1

**Project:** Scrivi
**Document:** Timeline Panel — Requirements and Design
**Version:** 0.1
**Status:** Draft — under review
**Date:** 2026-06-11

---

## 1. Purpose

This document defines the requirements, interaction model, persistence schema, and data model for the fully functional Scrivi Timeline Panel. It supersedes the placeholder `TimelineStripView.swift` and governs all Timeline implementation work.

---

## 2. Design Principles

1. **Non-intrusive by default.** The timeline must never interrupt writing. A seat-of-pants writer can open Scrivi, write scenes for months, and never consciously interact with the timeline. It should simply reflect what she has written.
2. **Rich when engaged.** A multi-world, multi-timeline author must be able to manage intersecting histories across projects without structural constraint.
3. **Story time is not narrative order.** The position of a scene on the timeline expresses *when in the story world the scene occurs*, not where it appears in the manuscript. Manuscript order is always controlled through the Scene Navigator. These are independent axes.
4. **Story Structure is cosmetic first, annotative optionally.** Structure bands paint a background interpretation over the timeline. They do not own or constrain scenes.
5. **The timeline is the duration of the story.** There is no fixed calendar unit. The timeline spans exactly the story's events — no more, no less.

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
- **UC-08:** Import an external timeline JSON file and view its events as grey dots on the timeline.
- **UC-09:** Set an epoch offset for an imported timeline so only intersecting events are visible.
- **UC-10:** Export the project's scene timeline as a JSON file for use in another project.
- **UC-11:** View a tooltip or popover for any dot (scene or imported event) showing its title and story-time position.

---

## 4. Functional Requirements

### 4.1 Timeline Strip — Core Display

**FR-001** The Timeline Panel shall render a horizontal line spanning the full panel width. The leftmost point represents the earliest scene in story time; the rightmost point represents the latest.

**FR-002** Each scene in the project shall be represented as a filled circle (dot) on the line. The dot is positioned at the scene's story-time offset relative to the project epoch.

**FR-003** When a project has only one scene, the dot shall appear on the far left of the panel with the line extending to the right.

**FR-004** When a project has two or more scenes and no story-time offsets have been manually set, scenes shall be evenly spaced along the timeline in manuscript order (left = first scene, right = last scene).

**FR-005** Scene dots shall be visually distinct from imported-event dots. Scene dots use the accent color; imported event dots use a neutral grey.

**FR-006** A scene dot whose story-time position was explicitly set (manually or inferred) shall display a small indicator distinguishing it from a default-positioned dot.

**FR-007** When a scene is assigned to a Story Structure band, the scene dot shall display a colored ring matching the band color.

**FR-008** The timeline shall be horizontally scrollable when the number of events exceeds the visible panel width.

**FR-009** The timeline shall support pinch-to-zoom (trackpad/touch) and scroll-to-zoom to adjust the visible time window without changing underlying offsets.

### 4.2 Scene Dot Interaction

**FR-010** Scene dots shall be draggable horizontally along the timeline line.

**FR-011** Dragging a scene dot shall update its story-time offset. It shall not change the scene's position in the manuscript (chapter/scene order is unaffected).

**FR-012** Dragging a scene dot past the rightmost dot shall trigger an "Expand Timeline Forward" popover. The popover shall allow the writer to specify a duration by which the timeline extends (e.g., "1 day", "2 weeks", "3 years"). The dragged scene shall be placed at the new end of the timeline.

**FR-013** Dragging a scene dot past the leftmost dot shall trigger an "Expand Timeline Backward" popover with equivalent behavior.

**FR-014** The timeline shall be contractable by dragging the first or last scene dot inward toward the opposite end, compressing the visible time span.

**FR-015** Scene dots shall be reorderable in story time by dragging them to any position between two existing dots, or to the first or last position. This is a story-time reorder only; manuscript order is unaffected.

**FR-016** A scene dot shall display a tooltip or popover on hover/tap showing: scene title, chapter, current story-time offset (human-readable), and manuscript position.

### 4.3 Story-Time Inference (Stub)

**FR-017** Each scene's story-time offset shall have a source field: `default`, `manual`, or `inferred`.

**FR-018** A stub interface shall exist in `scene.meta.json` for a future AI inference result: an extracted phrase (`inferenceHint`) and a confidence score (`inferenceConfidence`). These fields shall be null until the inference feature is implemented.

**FR-019** When `offsetSource` is `inferred`, the dot shall carry a visual indicator (e.g., a subtle dashed ring) distinguishing it from manually set offsets.

**FR-020** When `offsetSource` is `default`, the system shall position the scene immediately after the preceding scene with an offset of +1 millisecond (logically "right after"). The UI shall display this as "immediately after [scene name]."

### 4.4 Story Structure Bands

**FR-021** The project may have zero or one active Story Structure. Applying a Story Structure is a project setting stored in `project.json`.

**FR-022** The following Story Structures shall be available in v1:

| ID | Name | Bands |
|---|---|---|
| `three-act` | Three Act Structure | Act I, Act II, Act III |
| `five-act` | Five Act Structure | Exposition, Rising Action, Climax, Falling Action, Denouement |
| `heroes-journey` | Hero's Journey | Ordinary World, Call to Adventure, Refusal, Meeting the Mentor, Crossing the Threshold, Tests/Allies/Enemies, Approach, Ordeal, Reward, Road Back, Resurrection, Return |
| `save-the-cat` | Save the Cat | Opening Image, Theme Stated, Set-Up, Catalyst, Debate, Break Into Two, B Story, Fun and Games, Midpoint, Bad Guys Close In, All Is Lost, Dark Night of the Soul, Break Into Three, Finale, Final Image |
| `freytag` | Freytag's Pyramid | Introduction, Rising Action, Climax, Falling Action, Catastrophe |
| `kishōtenketsu` | Kishōtenketsu | Ki (Introduction), Shō (Development), Ten (Twist), Ketsu (Conclusion) |
| `in-medias-res` | In Medias Res | Action Open, Backstory, Rising Action, Resolution |
| `custom` | Custom | Writer-defined bands |

**FR-023** When a Story Structure is applied, the timeline background shall be divided into colored bands, one per structure element. Bands shall auto-divide the current timeline span equally on first application.

**FR-024** Band borders shall be draggable horizontally to reposition the act boundaries. Bands resize proportionally as their borders are moved.

**FR-025** Each band shall have a distinct color drawn from a predefined palette. Colors shall be consistent for a given structure (e.g., Act I is always a specific blue across all projects using Three Act Structure).

**FR-026** Band labels shall appear inside each band, positioned along the top edge. Labels shall truncate gracefully when a band is narrow.

**FR-027** Removing the active Story Structure shall remove all band display. It shall not affect scene story-time offsets or band assignments already recorded in scene metadata.

**FR-028** The Custom story structure shall allow the writer to add, remove, rename, and recolor bands. Custom band definitions are stored in the project.

### 4.5 Band Assignment

**FR-029** A scene may be explicitly assigned to a Story Structure band. Assignment is optional and purely annotative.

**FR-030** Assignment shall be possible via two gestures:
  - **Drag up:** Drag a scene dot vertically upward to the band label. Releasing over a label assigns the scene to that band. Dragging back down to the timeline line or releasing without a target unassigns it.
  - **Context menu:** Right-click (or long-press on touch) a scene dot to show a "Assign to Act…" menu listing all bands. Selecting a band assigns. Selecting "Unassign" removes assignment.

**FR-031** When a scene is assigned to a band, the dot displays a colored ring in the band's color.

**FR-032** Band assignment shall be stored in `scene.meta.json` under `storyStructure.bandID`.

**FR-033** If the active Story Structure is changed, existing band assignments referencing bands not present in the new structure shall be cleared and logged in the repair log.

### 4.6 External Timeline Import

**FR-034** The project may have zero or more imported external timelines. Each imported timeline is an independent JSON file stored in `objects/imported-timelines/`.

**FR-035** An imported timeline shall carry an epoch offset (in milliseconds) that maps its internal zero to a position on the project's story-time axis. This offset is set at import time and editable afterward.

**FR-036** Only events from an imported timeline whose story-time position (after epoch offset is applied) falls within the current project's timeline window shall be rendered.

**FR-037** Imported events shall be rendered as grey dots on the timeline. Grey intensity may vary per source timeline to distinguish multiple imports visually.

**FR-038** Imported event dots shall display a tooltip/popover showing: event title, source timeline name, original timestamp, and computed project story-time.

**FR-039** Imported events are read-only within the importing project. They cannot be dragged or assigned to bands.

**FR-040** The "Import Timeline" action shall accept a `.scrivi-timeline.json` file (see Section 6.3) and present an epoch offset dialog before importing.

### 4.7 Timeline Export

**FR-041** The "Export Timeline" action shall produce a `.scrivi-timeline.json` file containing the project's scene events with their story-time offsets and titles. This file is suitable for import into another Scrivi project.

**FR-042** Exported timelines shall not contain manuscript prose, identities, or private metadata — only the event list sufficient for timeline rendering.

---

## 5. Non-Functional Requirements

**NFR-001** The timeline panel shall render and be interactive within 100ms of project open on a project with up to 500 scenes.

**NFR-002** Drag operations shall update the visual position of dots at 60fps with no perceptible lag on supported hardware.

**NFR-003** Timeline data shall be stored as plain JSON inside the project package, readable and editable externally.

**NFR-004** The timeline panel shall function correctly with 0 scenes (empty state), 1 scene, 2 scenes, and up to 500 scenes.

**NFR-005** A project with no Story Structure applied and no imported timelines shall behave identically to the current placeholder: the panel shows scenes only, no background bands.

**NFR-006** All timeline persistence files must survive round-trip through external editors without data loss (standard JSON encoding, no binary fields).

---

## 6. Persistence Design

### 6.1 Project Package Layout (additions)

```text
MyNovel.scrivi/
  objects/
    timelines/
      timeline.meta.json          ← project's own timeline definition and epoch

    story-structures/
      story-structure.json        ← active structure selection and band layout

    imported-timelines/
      <id>-<slug>.scrivi-timeline.json   ← each imported external timeline
```

No new top-level folder is needed. All additions fit within the existing `objects/` folder established in `Scrivi_Project_Package_Structure_v0_1.md`.

### 6.2 `objects/timelines/timeline.meta.json`

Defines the project's own timeline.

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

The project epoch is defined as story-time offset 0. All scene offsets are relative to this epoch.

**Required fields:** `schema`, `timelineID`, `projectID`, `createdAt`

### 6.3 Story-Time in `scene.meta.json`

The `storyTime` block is added to each scene's existing `scene.meta.json`:

```json
"storyTime": {
  "offsetMs": 0,
  "offsetSource": "default",
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
| `offsetMs` | `int64` | Milliseconds from project epoch (signed; negative = before epoch) |
| `offsetSource` | `string` | `"default"` \| `"manual"` \| `"inferred"` |
| `inferenceHint` | `string?` | Extracted phrase that drove inference (e.g., `"later that day"`) |
| `inferenceConfidence` | `float?` | 0.0–1.0 confidence score from inference engine |
| `storyStructure.bandID` | `string?` | ID of assigned Story Structure band; null if unassigned |
| `storyStructure.assignedAt` | `string?` | ISO 8601 timestamp of assignment |

The existing `classification.timelineEventIDs` field in `scene.meta.json` is superseded by `storyTime.offsetMs` for primary story-time positioning. `timelineEventIDs` is retained as a hook for future cross-reference to imported event objects.

### 6.4 `objects/story-structures/story-structure.json`

Stores the active Story Structure and the current band layout for this project.

```json
{
  "schema": "scrivi.storyStructure.v1",
  "activeStructureID": "heroes-journey",
  "customBands": [],
  "bandLayout": [
    {
      "bandID": "ordinary-world",
      "label": "Ordinary World",
      "color": "#5B8DD9",
      "startOffsetMs": 0,
      "endOffsetMs": 604800000
    },
    {
      "bandID": "call-to-adventure",
      "label": "Call to Adventure",
      "color": "#D97B5B",
      "startOffsetMs": 604800000,
      "endOffsetMs": 1209600000
    }
  ]
}
```

`startOffsetMs` and `endOffsetMs` are in the same coordinate system as scene story-time offsets. On first application of a structure, bands auto-divide the current timeline span equally. After that, band borders are set by the writer's drag actions.

`customBands` holds band definitions when `activeStructureID` is `"custom"`. For built-in structures it is empty (the band definitions are compiled into the app).

**Required fields:** `schema`, `activeStructureID`, `bandLayout`

### 6.5 External Timeline Import Format: `.scrivi-timeline.json`

This is the exchange format for timeline export/import between Scrivi projects.

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

`kind` is `"scene"` for events sourced from a Scrivi scene, and `"historical"` for manually authored historical events.

### 6.6 Imported Timeline Record (stored in project)

When an external timeline is imported, Scrivi stores it with an added `epochOffsetMs` field representing the shift applied to align it with the importing project's story time:

```text
objects/imported-timelines/
  timeline_01J-the-shattered-veil.scrivi-timeline.json
```

The stored file is the original import JSON with one additional top-level field:

```json
{
  "schema": "scrivi.externalTimeline.v1",
  "epochOffsetMs": -94608000000,
  ...
}
```

`epochOffsetMs` is added by Scrivi at import time. It is the number of milliseconds to add to every event's `offsetMs` to produce a project-relative story-time position.

An imported event is rendered if and only if:

```
projectEpoch ≤ (event.offsetMs + epochOffsetMs) ≤ projectEndMs
```

---

## 7. UI / Interaction Model

### 7.1 Panel Layout

```
┌─────────────────────────────────────────────────────────────────────┐
│ Timeline                                              [Structure ▾]  │
│                                                                      │
│  ░░░░ Act I ░░░░░░│░░░░░░░░░ Act II ░░░░░░░░░│░░░░░░ Act III ░░░░  │ ← bands (background)
│  ─────●──────●───●────────●──────●──────●────────●──────────●───── │ ← timeline line + dots
│       S1     S2  S3       S4     S5     S6        S7         S8    │
│                                  ○     ○                            │ ← grey: imported events
└─────────────────────────────────────────────────────────────────────┘
```

The timeline panel is a fixed-height strip (initially ~100pt, resizable in a future sprint) docked at the bottom of the Editor view, below the manuscript surface.

### 7.2 Dot States

| State | Visual |
|---|---|
| Default position (offset = manuscript order) | Filled accent circle |
| Manual story-time set | Filled accent circle + solid ring |
| Inferred story-time | Filled accent circle + dashed ring |
| Assigned to band | Filled accent circle + solid colored ring (band color) |
| Imported event | Filled grey circle |
| Selected / hover | Enlarged with popover |

### 7.3 Expand Timeline Popover

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

Duration units: Minutes, Hours, Days, Weeks, Months, Years.

### 7.4 Band Assignment by Drag

When the writer begins dragging a scene dot, band labels appear at the top of the panel as drop targets. Dragging the dot upward reveals the labels. Releasing the dot over a label assigns it; releasing anywhere else (including back on the line) cancels the gesture without changing assignment.

### 7.5 Context Menu (scene dot)

- Assign to Act…  ▸ [list of bands]
- Unassign from Act
- Set Story Time…
- View Scene
- — (separator)
- Copy Story-Time Position
- Paste Story-Time Position

### 7.6 Empty State

When a project has no scenes, the panel shows the line with a subtle label:
`"Scenes will appear here as you write."`

### 7.7 Story Structure Selector

A `[Structure ▾]` button in the panel header opens a menu:

- None (remove structure)
- — (separator)
- Three Act Structure
- Five Act Structure
- Hero's Journey
- Save the Cat
- Freytag's Pyramid
- Kishōtenketsu
- In Medias Res
- — (separator)
- Custom…

Selecting a structure applies it immediately with auto-divided bands. Switching structures does not affect story-time offsets but clears band assignments for bands not present in the new structure.

---

## 8. Story Structure Band Definitions (v1)

### Three Act Structure

| Band ID | Label | Default Color |
|---|---|---|
| `act-i` | Act I | `#5B8DD9` (blue) |
| `act-ii` | Act II | `#D9A05B` (amber) |
| `act-iii` | Act III | `#5BD98D` (green) |

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

Save the Cat (15 beats), Freytag's Pyramid (5 acts), Kishōtenketsu (4 acts), and In Medias Res (4 acts) follow the same pattern and are defined identically in the app's compiled structure registry. Full color tables are omitted here for brevity.

---

## 9. ScriviCore Backend Responsibilities

The C++ backend (`ScriviCore`) owns all timeline persistence. The Swift layer owns all rendering and interaction.

### 9.1 ScriviCore API (new operations)

The following operations shall be added to the ScriviCore public API. All return JSON-over-string envelopes per the established boundary protocol.

```cpp
// Timeline meta
std::string getTimeline(const char* projectID);

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
std::string removeStoryStructure(const char* projectID);

// External timelines
std::string importExternalTimeline(const char* projectID, const char* timelineJSON,
                                    int64_t epochOffsetMs);
std::string listImportedTimelines(const char* projectID);
std::string removeImportedTimeline(const char* projectID, const char* timelineID);
std::string exportProjectTimeline(const char* projectID);
```

### 9.2 ScriviCore Storage Rules

- `timeline.meta.json` is created alongside `project.json` when a project is created.
- `story-structure.json` is created on first Story Structure application; it does not exist for projects with no structure applied.
- `imported-timelines/` folder is created on first import.
- All files are written atomically (existing `AtomicWrite` utility).
- Scene `storyTime` blocks are written into the scene's existing `scene.meta.json` file — no separate file per scene.

---

## 10. Open Questions (Deferred Decisions)

The following questions are identified but intentionally deferred to a later design revision:

| ID | Question |
|---|---|
| OQ-01 | Should the Timeline Panel height be user-resizable? If so, what is the minimum and maximum height? |
| OQ-02 | Should imported events be groupable by source timeline into separate visual rows (swimlanes), or always rendered on the same line as project scenes? |
| OQ-03 | Should scene dots carry a tooltip showing story-time as a formatted human-readable duration ("3 days, 4 hours after story opens") or as a relative phrase ("immediately after Chapter 1, Scene 2")? |
| OQ-04 | When a Story Structure is applied mid-project and bands auto-divide, should the writer be asked to confirm the initial band layout before it is saved? |
| OQ-05 | Should the Custom story structure allow per-band width ratios (unequal band widths by design), or always equal widths before manual adjustment? |
| OQ-06 | Should there be a "historical event" object type that a writer can author directly in the project (not just by import)? This would allow worldbuilding events that are not scenes (battles, discoveries, births) to appear on the timeline. |
| OQ-07 | What is the behavior when two scenes have identical `offsetMs` values? Stack vertically, overlap, or prevent the collision? |

---

## 11. Out of Scope for v1

- AI/NLP inference of story-time from prose text (stubbed only; FR-017 through FR-019)
- Character-timeline intersection views (which characters are present at each story-time position)
- Location-timeline intersection views
- CloudKit sync of timeline data
- Collaborative timeline editing (two writers on the same timeline simultaneously)
- Per-timeline Story Structure application (one structure applies to the whole project in v1)
- Historical event authoring directly in the project (OQ-06 above)

---

## 12. Relationship to Existing Documents

| Document | Relationship |
|---|---|
| `Scrivi_Project_Package_Structure_v0_1.md` | This document adds `objects/timelines/`, `objects/story-structures/`, `objects/imported-timelines/` to the approved package layout |
| `Scrivi_Minimum_Schema_Set_v0_1.md` | Timeline schemas are in the deferred list (Section 13). This document defines those schemas. |
| `Scrivi_Architecture_v0_3.md` | JSON-over-string boundary protocol governs all ScriviCore API calls. |
| `Scrivi_Cpp24_Core_API_Sketch_v0_3.md` | New timeline operations follow the same API shape and return envelope as existing operations. |

---

## 13. Success Criteria

This design is considered successfully implemented when:

1. A writer with no scenes sees an empty timeline panel with a helpful empty-state message.
2. A writer with scenes sees dots on the timeline in manuscript order without any configuration.
3. A writer can drag a dot to a new story-time position and the position persists across project close/reopen.
4. A writer can apply Three Act Structure and see three colored bands appear behind the timeline.
5. A writer can drag a band border to reposition act boundaries.
6. A writer can assign a scene to a band by dragging its dot to the band label, and the ring color updates.
7. A writer can assign a scene to a band via the context menu with the same result.
8. A writer can import an external `.scrivi-timeline.json` file and see its events as grey dots.
9. Only events within the project's timeline window are visible from imported timelines.
10. A writer can export the project timeline and import it into another project.
11. Removing a Story Structure removes bands but preserves all story-time offsets.
12. All timeline data survives an external round-trip (files opened and saved by a text editor).

---

*Document Status: Draft — all design decisions reflect the conversation of 2026-06-11. Review before implementation begins.*
