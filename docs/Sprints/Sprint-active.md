# Active Sprint

## SP-082: [Linux] Historical events + imported timelines + export

**Status:** 🟢 All tasks Verified (2026-07-24, VNC) — **awaiting Human close approval**
**Epic:** EP-025 `[Linux]` Timeline Panel (4th of ~5 sprints; mirrors Apple EP-016 **SP-042**, minus the
clustering half which is carved out to **SP-084**)
**Goal:** Give the Linux timeline the last of its authored/imported content. A writer can **author, edit, and
delete historical events** in the project — worldbuilding moments (not scenes) that appear on the project row as
a **muted warm-toned (`#C8A97A`) dot**, draggable in story time. A writer can **import** an external
`.scrivi-timeline.json` — its events render in a **distinct grey row below** the project row, window-clipped to
the current timeline span, with a **per-source grey shade**, and each row can be **hidden/shown**; the import
flow shows an **epoch-offset dialog** so only intersecting events appear. And a writer can **export** the
project's scene + historical events as a `.scrivi-timeline.json`. Delivers **EP-025 AC5**.
**Start Date:** 2026-07-24
**Target Close Date:** TBD (est. 1–2 days, matching SP-079/SP-080/SP-081)
**Capacity:** ~8–10 hours

> **Scope carve-out (user decision, planning 2026-07-24):** Apple bundled clustering into its equivalent
> SP-042; the Linux EP-025 plan already split **co-located dot clustering + panel persistence + full EP-025
> verify + Epic close into SP-084**. SP-082 is **historical / imported / export only**. Aggregate dots are
> explicitly out of scope here.

---

### Design references

- `docs/Scrivi_Timeline_Panel_Design_v0_3.md` — **§4.9 Historical Events (FR-050–FR-056)**, **§4.10 Multiple
  Timeline Rows (FR-057–FR-065)**, **§4.11 Import/Export (FR-066–FR-070)**; persistence **§6.5** (historical
  event file), **§6.6** (`.scrivi-timeline.json` exchange format), **§6.7** (imported record: `epochOffsetMs`,
  `visible`, `assignedGreyShade`); context menus **§7.7 / §7.8 / §7.9**; dot states **§7.2**.
- **The timeline C ABI is complete (EP-016).** `scrivi.h` lines **270–291** already export everything this
  sprint consumes — confirmed at planning, `scrivi.h` **untouched** this sprint:
  - `scrivi_create_historical_event(root, title, offsetMs, description, tagsJSON, identityID, personaID,
    displayName)`, `scrivi_update_historical_event(root, eventID, title, offsetMs, description, tagsJSON)`,
    `scrivi_delete_historical_event(root, eventID)`, `scrivi_list_historical_events(root)`.
  - `scrivi_import_external_timeline(root, timelineJSON, epochOffsetMs, assignedGreyShade)`,
    `scrivi_update_imported_timeline_offset(root, timelineID, epochOffsetMs)`,
    `scrivi_set_imported_timeline_visible(root, timelineID, visible)`,
    `scrivi_list_imported_timelines(root)`, `scrivi_remove_imported_timeline(root, timelineID)`,
    `scrivi_export_project_timeline(root)`.
- **Apple analogue** (behavior to re-create in Qt, NOT port): EP-016 **SP-042** (`Closed/Sprint-SP-042.md`),
  tasks T-0161–T-0165/T-0169 and `TimelineStripView.swift` (`HistoricalEventDot`, imported-row rendering,
  `EpochOffsetDialog`, export). Segment/cluster pieces (T-0166) belong to SP-084, not here.

---

### Assigned Tasks

| ID     | Title | Priority | Status |
| ------ | ----- | -------- | ------ |
| T-0340 | `[Linux]` **Timeline-events bridge invokables** — `ScriviBridge` Qt wrappers over the complete EP-016 C ABI: `createHistoricalEvent`/`updateHistoricalEvent`/`deleteHistoricalEvent`/`listHistoricalEvents` + `importExternalTimeline`/`updateImportedTimelineOffset`/`setImportedTimelineVisible`/`listImportedTimelines`/`removeImportedTimeline`/`exportProjectTimeline`. Each `parseEnvelope`s, RAII `scrivi_free`, `errorOccurred` on failure, `ready_` guard, no identity (empty `identityID`/`personaID`/`displayName` — same as story-time). `scrivi.h` **untouched**. | High | ✅ Verified (2026-07-24, VNC) |
| T-0341 | `[Linux]` **Historical-event dots + author/edit/delete** — `TimelinePanel` renders historical events on the project row as `#C8A97A` filled dots (distinct from scene accent + imported grey, spec §7.2), draggable in story time (`HistHorizontal` drag → `historicalEventDragged(eventID, newOffsetMs)`), with a hover tooltip. `EditorShell` feeds them via `reloadTimeline` (`listHistoricalEvents`). **Empty-area context menu "New Historical Event Here"** (§7.9) + **historical-dot context menu Edit / Delete** (§7.7) → `HistoricalEventDialog` (title, description, tags; tags read from disk on Edit since the list drops them). | High | ✅ Verified (2026-07-24, VNC) |
| T-0342 | `[Linux]` **Imported-timeline rows + epoch-offset dialog + hide/show** — `TimelinePanel` paints one **grey row below** the project row per imported timeline (source name label at left, per-source `assignedGreyShade`, dots **window-clipped** per §6.7, read-only, tooltip = title + source + computed time). **Import Timeline…** (§7.9) → `QFileDialog` → **`EpochOffsetDialog`** (source name + epoch label + signed offset + in/out-of-window preview, FR-067) → `importExternalTimeline`. Row menu (§7.8): **Edit Epoch Offset… / Hide This Timeline / Remove Imported Timeline**; **Show Hidden Timelines** submenu (empty-area menu) un-hides (FR-065). Events read from the stored files (list is metadata-only, Apple's pattern). Min-height +1 row per visible import (FR-064). | High | ✅ Verified (2026-07-24, VNC) |
| T-0343 | `[Linux]` **Export timeline** — **Export Timeline…** (§7.9) → `exportProjectTimeline` (scene + historical events, no prose/identity, §6.6/FR-069) → `QFileDialog` save-as writes the file. Round-trips: an exported file re-imports (T-0342) into another project (smoke asserts it). | Medium | ✅ Verified (2026-07-24, VNC) |
| T-0344 | `[Linux]` **Wire-up + `timeline_events_smoke` + verify** — end-to-end wiring in `EditorShell` (`reloadTimeline`/`reloadImportedTimelines` load historical events + imported rows); new **`timeline_events_smoke`** (headless, offscreen): historical CRUD round-trip, import a fixture (stored + metadata + on-disk file), hide/show + update-offset persist, export a valid `scrivi.externalTimeline.v1` carrying the surviving historical event, remove clears + CMake target + CI step. **Container green + smoke PASS + VNC walkthrough complete.** Closes AC5. | High | ✅ Verified (2026-07-24, VNC) |
| T-0345 | `[Linux]` **File ▸ Import / Export Timeline… menu items** (user request 2026-07-24) — now that SP-077 gave the app a native menu bar, the import/export **file** operations also live in **File** (below Close Project, own separator group), not only in the timeline panel's right-click menu. Editor-only (`editorOnlyActions_`, disabled on landing); each forwards to a new public `EditorShell::importTimeline()`/`exportTimeline()` trigger → the same T-0342/T-0343 flow. The panel's empty-area entries are **kept** (both homes, like Cut/Copy). | Medium | ✅ Verified (2026-07-24, VNC) |

### Assigned Issues

Found in the 2026-07-24 VNC verify, both fixed same day and **re-verified live (2026-07-24)**:

| ID | Title | Severity | Status |
| -- | ----- | -------- | ------ |
| I-0090 | `[Linux]` **Lowest imported-timeline row obscured by the zoom scrollbar** — imported rows stacked from `height()` over the +/- controls + scrollbar band. Fixed: reserve a `kBottomControlsHeight` (24px) band; rows stack above it. | Medium | ✅ **Verified (2026-07-24, VNC)** — user confirmed the second imported row ("Bureau of Identity") is fully visible above the scrollbar. |
| I-0091 | `[Linux]` **Import/Export file dialogs defaulted to `/root`** not the project's folder. Fixed: seed both `QFileDialog`s with `QFileInfo(projectPath_).absolutePath()` (the project's parent folder — NOT hardcoded). | Low | ✅ **Verified (2026-07-24, VNC)** — user confirmed both dialogs open in the project's folder. |

### Verification (2026-07-24, container)

- ✅ **Build green** — Qt 6.4: **211/211** targets, 0 errors; `scrivi_linux` +
  `scrivi_linux_timeline_events_smoke` linked (new dialogs compiled in).
- ✅ **New `timeline_events_smoke` PASS** — historical create/update/delete list round-trip; import a fixture
  `.scrivi-timeline.json` (stored + metadata + on-disk file present); hide/show + update-offset persist; export
  a valid `scrivi.externalTimeline.v1` body carrying the surviving historical event; remove → list empty.
- ✅ **All 11 regression smokes PASS** — story-structure, timeline-story-time, merge, create, reorder,
  chapter-reorder, editor-map, save, rename, delete, load, lifecycle, persistence.
- ✅ **Headless app-launch OK** — the full window tree (incl. the SP-082 panel + dialogs) constructs offscreen.
- ✅ **Live VNC walkthrough complete (2026-07-24)** — Human verified all six tasks on
  `the-twisted-remains-of-myself`: historical events author/drag/edit/delete (`#C8A97A` dots); imported grey
  rows (two sources, per-source shade); export + import round-trip; and the File ▸ Import/Export Timeline…
  menu items. Two findings surfaced + fixed + re-verified same day: **I-0090** (imported row hidden behind the
  scrollbar) and **I-0091** (file dialogs defaulted to `/root`). **AC5 met.** Sprint awaiting Human close
  approval.

---

### Acceptance Criteria (EP-025 AC5)

- [x] **"New Historical Event Here"** from the empty-area context menu creates an event and shows it as a
  `#C8A97A` dot on the project row.
- [x] Historical events are **draggable** on the timeline (their `offsetMs` updates + persists across reopen).
- [x] Historical-dot context menu: **Edit… / Delete** work; edit/delete persist. (Re-timing is by drag —
  "Set Story Time" folded into the drag path, §7.7.)
- [x] **Import Timeline…** picks a `.scrivi-timeline.json` and shows the **epoch-offset dialog** (source name,
  epoch label, signed offset, in/out-of-window preview) before storing.
- [x] Imported events appear as a **grey row below** the project row; the source name is labelled at left.
- [x] Only events whose **computed project story-time falls within the current window** are rendered
  (window-clipped per §6.7).
- [x] **Two** imported timelines appear as two distinct rows with **distinguishable grey shades**.
- [x] A row can be **hidden/shown** via the panel menu without removing the imported file.
- [x] **Edit Epoch Offset…** on a row re-offsets its events (persists).
- [x] **Remove Imported Timeline** deletes the stored file; the row disappears.
- [x] **Export** produces a syntactically valid `.scrivi-timeline.json` containing all project scenes +
  historical events; it re-imports into another project.
- [x] Hover tooltips: historical dot (title + story-time); imported dot (title + source name + computed
  story-time).
- [x] **No regression:** scene dots (SP-079), dot drag + Time Delta Picker (SP-080), bands + assignment
  (SP-081), zoom/pan (SP-083) all still work; imported rows follow the same zoom/scroll as the project row.
- [x] **File ▸ Import/Export Timeline… menu items** (T-0345) present, editor-only, drive the same flows.

> **N/A this sprint (→ SP-084):** aggregate/cluster dots (FR-030–035), full-panel persistence sweep beyond the
> zoom/pan state SP-083 already persists, and the EP-025 Epic close.

### Scope & guardrails

- **No backend work.** The timeline events C ABI is complete (EP-016/SP-039). `scrivi.h` **untouched**; a
  genuinely missing endpoint would become a `[ScriviCore]` Task, not new sprint scope.
- **Re-create, don't port.** The Apple `TimelineStripView` historical/imported/export code is the behavioral
  reference; the Linux panel is native Qt widgets/painting fed by `EditorShell`.
- **VNC-safe interaction.** Import/export use `QFileDialog` (already proven over VNC — `ScriviBridge::chooseFolder`,
  EP-021). All context menus are right-click `QMenu`s. No new gesture-only path (memory:
  `project_linux_vnc_input_constraints`).
- **JSON-shape watch.** As with SP-081's `{"bands":[…]}` finding, confirm the exact envelope shapes the C ABI
  returns/accepts against the smoke — the `timeline_events_smoke` exists to catch a shape mismatch, not to trust
  the format sight-unseen.
- **`EditorShell::reloadTimeline` is the single feed.** The panel calls no `scrivi_*` itself; the shell loads
  scenes + bands (existing) **plus** historical events + imported rows (new) and owns all persistence.
- **No pbxproj** — Linux-only (Qt/C++). New `.cpp` (`HistoricalEventDialog`, `EpochOffsetDialog`, the smoke) go
  in the Linux CMake, never `Scrivi.xcodeproj` (memory: `project_pbxproj_scrivicore_scope`).

---

*Last Updated: 2026-07-24 (**SP-082 all tasks ✅ Verified (VNC) — awaiting Human close approval**. EP-025
`[Linux]` Timeline Panel, 4th sprint, delivering **AC5**: historical events (author/edit/delete + `#C8A97A`
draggable dots), imported timelines (grey rows below, per-source shade, window-clip, epoch-offset dialog,
hide/show, edit-offset, remove), export (`.scrivi-timeline.json`), + File ▸ Import/Export menu items. Tasks
**T-0340–T-0345** all Verified live on `the-twisted-remains-of-myself`. Two findings surfaced + fixed +
re-verified same day: **I-0090** (imported row hidden behind the zoom scrollbar) + **I-0091** (file dialogs
defaulted to `/root` not the project folder). Container green (211/211) + `timeline_events_smoke` + 11
regression smokes PASS; `scrivi.h` untouched; no pbxproj (Linux-only). Clustering + panel persistence + Epic
close remain in **SP-084**. Next available Task after this sprint: **T-0346**.)*
