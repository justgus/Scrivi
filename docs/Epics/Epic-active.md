# Active Epics

## EP-025: [Linux] Timeline Panel

**Codebase:** `[Linux]` (Qt/QML Ubuntu app, `platforms/linux/`) — calls `[ScriviCore]` only via the existing
plain-C ABI through `ScriviBridge`. **The full timeline C ABI already exists** (EP-016/SP-039:
`scrivi_get_timeline`, `scrivi_set_scene_story_time`, story-structure, historical events, imported timelines,
export — `scrivi.h` lines 248–291). No new endpoint is expected; any gap is a Task with a `[ScriviCore]` note.

**Status:** 🟡 Active (2026-07-22: SP-079 ✅ [AC1/AC2] + SP-080 ✅ [AC3] closed; SP-081 active [AC4], T-0332
blocked by I-0087; **SP-083 zoom/pan brought forward** [AC6a] to unblock it — two parallel active sprints)
**Goal:** The **Timeline Panel** on Linux — a hideable horizontal timeline strip across the **bottom** of the
editor showing one dot per scene in **story-time** order, with drag-to-reposition + the Time Delta Picker,
story-structure bands, historical events, imported timelines, export, and co-located-dot clustering. Full
parity with Apple **EP-015/EP-016** (`Scrivi_Timeline_Panel_Design_v0_3.md`), rebuilt in Qt/C++ (not ported).
**Date Created:** 2026-07-22
**Target Close Date:** TBD (estimated 4–5 sprints, mirroring EP-016's five)
**Actual Close Date:** —

> **Strategy:** Fifth delivered Epic of the `[Linux]` family (EP-020–EP-026). The largest since EP-016 was
> the largest Apple Epic. Verified in Docker+VNC (developer) per sprint, then on real Ubuntu (alpha tester).
> Sprints sequenced below; each sprint's exact tasks are fixed at its planning, one at a time, matching how
> EP-022/EP-023 ran. **User decisions (2026-07-22):** full EP-016 parity; the panel docks as a **bottom
> strip** (below navigator+viewport+inspector, resizable by its top edge — Apple's layout).

### Design references

- `docs/Scrivi_Timeline_Panel_Design_v0_3.md` — the authoritative Timeline design (requirements, interaction
  model, persistence schema, story-structure integration, historical/imported timelines, clustering, Time
  Delta Picker). The Linux UI re-creates this behavior.
- `ScriviCore/include/scrivi/scrivi.h` lines 248–291 — the complete timeline C ABI (envelopes reconfirmed at
  each sprint's planning). **`scrivi_get_timeline` returns only the timeline META** (`timelineID`, `epochLabel`,
  `projectID`, `createdAt`); **per-scene story-time is a separate `scrivi_get_scene_story_time`** call
  (`offsetSource`, `gapMs`, `durationMs`, `durationSource`, `bandID`), and dot `offsetMs` is computed
  client-side from the gap chain (Apple `TimelineViewModel.recomputeAllOffsets`). Confirmed at SP-079
  implementation (the initial plan wording conflated the two).
- **Apple analogue (behavior to re-create in C++/QML, NOT port wholesale):** `Scrivi/Views/TimelineStripView.swift`
  (~3000 lines) — the `TimelineViewModel` (dot layout from story-time + gap chain), scene/historical/imported
  dots, `BandOverlayView`, `AggregateDotView` clustering (T-0174), `TimeDeltaPicker`, `EpochOffsetDialog`,
  pan/zoom. This is the reference for guards/interaction, not a source to translate line-by-line.

### Acceptance Criteria (draft — each refined at the owning sprint's planning)

- [x] AC1 — **Bottom timeline strip + scene dots:** a hideable horizontal panel across the editor bottom shows
  one dot per scene, positioned by **story-time** (chain-computed from `get_scene_story_time`'s gap/duration,
  default chain when unset), vertically centered; empty-state message when no scenes. Panel resizable by its
  top edge (min height). Absent/inert on the landing page. **(SP-079)** ✅ **Verified (2026-07-22).**
- [x] AC2 — **Panel show/hide + dot↔navigator selection:** a **View ▸ Show Timeline** toggle (Ctrl+Alt+T,
  session-scoped, editor-only, check-state synced) shows/hides the strip; clicking a dot selects/scrolls to
  that scene in the editor, and the active scene highlights its dot (bidirectional, Apple T-0173). Dot tooltip
  shows title + human-readable story-time. **(SP-079)** ✅ **Verified (2026-07-22).**
- [x] AC3 — **Scene-dot drag + Time Delta Picker + chain propagation:** dragging a dot horizontally opens the
  Time Delta Picker (spinner pre-populated, duration row, "Immediately after"); committing writes
  `scrivi_set_scene_story_time` and recomputes subsequent default-positioned scenes' offsets. Context-menu
  "Set Time Delta…" opens it without a drag. **(SP-080)** ✅ **Verified (2026-07-22).**
- [ ] AC4 — **Story-structure bands:** applying a structure writes via `scrivi_set_story_structure` and renders
  colored bands + labels; band borders drag (proportional, persist); a dot assigns to a band by drag-up or
  context menu ("Assign to Act…"); removing a structure keeps offsets + assignments. **(SP-081)**
- [ ] AC5 — **Historical events + imported timelines + export:** author/edit/delete historical events (own
  dot color) via the C ABI; import a `.scrivi-timeline.json` (distinct grey row, window-clipped, multiple
  rows/shades, hide/show); export the project timeline. **(SP-082)**
- [ ] AC6a — **Zoom + pan (SP-083, brought forward to fix I-0087):** the writer can zoom the linear time-axis
  to spread crowded dots (an outlier scene no longer makes the rest un-interactable) and pan across it. **Zoom
  = `Ctrl`+scroll-wheel (zoom-about-pointer) + an always-works `+`/`−` control at the strip's bottom-right**
  (a plain click, VNC-safe); **pan = click-drag on the empty area above/below the dots**. Linear axis kept
  (Apple parity — NOT even-spacing); native pinch an optional bonus on real Ubuntu. Unblocks T-0332.
- [ ] AC6b — **Clustering + full verify (SP-084):** co-located dots form an aggregate dot with a members
  popover (Apple T-0174); zoom resolves clusters; panel persists all state across close/reopen. No regression:
  Linux smokes + the app-launch smoke green; writing loop / navigator / inspector unaffected.

> **Apple ACs N/A on Linux:** the iPhone-exclusion AC from EP-016 does not apply (Linux is desktop-only).

### Sprints (sequence — mirrors EP-016; each activated one at a time)

| Sprint | Title | Status | Dates |
| ------ | ----- | ------ | ----- |
| SP-079 | `[Linux]` Timeline panel scaffold + scene dots (story-time layout) + show/hide + dot↔navigator selection | ✅ Closed | 2026-07-22 – 2026-07-22 |
| SP-080 | `[Linux]` Scene-dot drag + Time Delta Picker + chain propagation (`set_scene_story_time`) | ✅ Closed | 2026-07-22 – 2026-07-22 |
| SP-081 | `[Linux]` Story-structure bands — overlay, border drag, band assignment | 🟡 Active (T-0332 blocked by I-0087 → SP-083) | 2026-07-22 – |
| SP-083 | `[Linux]` Timeline zoom + pan (Ctrl+wheel + `+`/`−` control, drag-to-pan) — **brought forward** to fix I-0087 / unblock T-0332 | 🟡 Active | 2026-07-22 – |
| SP-082 | `[Linux]` Historical events + imported timelines + export | 🔵 Planned | — |
| SP-084 | `[Linux]` Co-located dot clustering + persistence + full EP-025 verify & Epic close (was SP-083's tail) | 🔵 Planned | — |

### Tasks

| ID     | Title | Sprint | Status |
| ------ | ----- | ------ | ------ |
| T-0321 | `[Linux]` `ScriviBridge::getTimeline` + `getSceneStoryTime` invokables — `scrivi_get_timeline` (epoch/meta) + `scrivi_get_scene_story_time` (per-scene gap/duration) | SP-079 | ✅ Verified (2026-07-22) |
| T-0322 | `[Linux]` `TimelinePanel` widget — horizontal strip, scene dots laid out by story-time, empty state, resizable top edge, active-dot highlight, hit-testing | SP-079 | ✅ Verified (2026-07-22) |
| T-0323 | `[Linux]` Dock the panel as the editor's bottom strip (outer vertical splitter) + **View ▸ Show Timeline** toggle (Ctrl+Alt+T, session-scoped, editor-only, check-state synced) | SP-079 | ✅ Verified (2026-07-22) |
| T-0324 | `[Linux]` Dot↔navigator bidirectional selection + tooltip + `reloadTimeline` chain math (closes AC1/AC2) | SP-079 | ✅ Verified (2026-07-22) |
| T-0325 | `[Linux]` `ScriviBridge::setSceneStoryTime` invokable → `scrivi_set_scene_story_time` (peer to `getSceneStoryTime`) | SP-080 | ✅ Verified (2026-07-22) |
| T-0326 | `[Linux]` `TimelinePanel` dot drag — press-on-dot → horizontal drag → `dotDragged(sceneID, newOffsetMs)` on release; click vs drag by threshold; context-menu "Set Time Delta…"; background reserved for pan | SP-080 | ✅ Verified (2026-07-22) |
| T-0327 | `[Linux]` `TimeDeltaPicker` `QDialog` — amount/unit/direction spinner + duration row + "Immediately after" (reset); returns Outcome + offset + duration | SP-080 | ✅ Verified (2026-07-22) |
| T-0328 | `[Linux]` Wire drag/context-menu → picker → commit (`setSceneStoryTime` manual) + chain propagation + `timeline_story_time_smoke` (closes AC3) | SP-080 | ✅ Verified (2026-07-22) |
| T-0329 | `[Linux]` Story-structure bridge invokables (`get/set/update/remove_story_structure`, `assign/unassign_scene_to_band`) + built-in band table ported from Apple (`StoryStructures.cpp/.hpp`) | SP-081 | ✅ Verified (2026-07-22) |
| T-0330 | `[Linux]` Band overlay painting (behind dots) + View ▸ Story Structure… selector (built-ins + Remove); loaded via `getStoryStructure` | SP-081 | ✅ Verified (2026-07-22) |
| T-0331 | `[Linux]` Band border drag — re-proportion adjacent bands (sum 1.0) + persist via `updateBandLayout` | SP-081 | ✅ Verified (2026-07-22) |
| T-0332 | `[Linux]` Scene→band assignment (drag-up-onto-label + "Assign to Act…" context menu) + colored ring + `story_structure_smoke` (closes AC4) | SP-081 | 🟡 Implemented — build+smokes green; live verify **blocked by I-0087** (dot-crowding) |
| T-0333 | `[Linux]` `TimelinePanel` zoom model — zoom factor + pan offset over `xForOffset`/`offsetForX`; `Ctrl`+wheel zoom-about-pointer; clamp/reset | SP-083 | 🟢 Implemented, Not Verified |
| T-0334 | `[Linux]` `+`/`−` zoom control (bottom-right, zoom-about-pointer-or-center) + a horizontal scrollbar when zoomed | SP-083 | 🟢 Implemented, Not Verified |
| T-0335 | `[Linux]` Pan by click-drag on the empty area above/below the dots + verify; **re-verify T-0332 assignment zoomed in** (closes I-0087) | SP-083 | 🟡 Implemented — build+smokes green; live VNC verify pending |

### Scope Notes

- **No new backend expected.** The timeline C ABI is complete (EP-016). Each sprint reconfirms the exact
  envelope it consumes at planning; a genuinely missing endpoint becomes a `[ScriviCore]` Task, not a new Epic.
- **Re-create, don't port.** The Apple `TimelineStripView` is the behavioral reference (dot layout, guards,
  Time Delta Picker semantics, clustering model T-0174); the Linux panel is native Qt widgets/painting.
- **Panel placement = bottom strip** (user decision) — docks below the existing
  navigator+viewport+inspector splitter in `EditorShell`'s `QVBoxLayout`, resizable by its top edge.
- **Menu home:** the SP-077 `QMenuBar` — **View ▸ Show Timeline** joins **View ▸ Show Inspector** (EP-024)
  on the existing `editorOnlyActions_` + `updateMenuState()` machinery.
- **Pan/zoom input = keyboard/wheel/buttons, NOT trackpad pinch (user decision 2026-07-22, AC6/SP-083).** The
  macOS→VNC→x11vnc→Xvfb→Qt(X11) test path cannot carry trackpad pinch/magnify or high-res pixelDelta scroll
  (same class as the SP-076/077 swallowed keys; and X11 delivers no native pinch to Qt regardless). So EP-025
  pan/zoom is **zoom = Ctrl+scroll-wheel + toolbar +/−**, **pan = click-drag on empty timeline background +
  Shift+wheel**, with native `QNativeGestureEvent`/pixelDelta accepted only as an optional bonus on real
  Ubuntu. Consequence for **SP-080**: the scene-dot drag binds to the **dot only** — empty-background drag stays
  reserved for pan. (Memory: `project_linux_vnc_input_constraints`.)

### Completion Summary

_(filled in when the Epic reaches 🟠 Complete)_

---

*Last Updated: 2026-07-22 (**EP-025 SP-081 — T-0329/0330/0331 ✅ Verified; T-0332 blocked by I-0087; SP-083
zoom/pan brought forward** (two parallel active sprints). SP-081's story-structure bridge/presets (T-0329),
band overlay + View ▸ Story Structure… selector (T-0330), and band border-drag re-proportion (T-0331) are all
Verified live. **T-0332** (scene→band drag-up assignment) is blocked by **I-0087**: a single far-outlier
flashback scene (~2yr) collapses all other dots to one strip edge under the linear time-axis, so a dot can't
be spread/grabbed to drag onto a band. **User decision:** keep the linear axis (Apple parity — NOT
even-spacing); fix via zoom/pan and **bring SP-083 forward** now. **SP-083** (activated 2026-07-22): timeline
zoom (`Ctrl`+wheel zoom-about-pointer + an always-works `+`/`−` control, bottom-right, for VNC/non-power-users)
+ pan (drag the empty area above/below the dots). Tasks T-0333–T-0335; re-verify T-0332 zoomed in. Clustering +
full verify + Epic close move to a new **SP-084**. Prior note follows.)*

*2026-07-22 (**SP-079 ✅ closed (Human-approved)** — the first
of ~5 sprints delivered the timeline read + layout + select core: `ScriviBridge` `getTimeline` +
`getSceneStoryTime` invokables, a `TimelinePanel` bottom strip with one scene dot per scene in story-time order
(chain-computed shell-side), a View ▸ Show Timeline toggle (Ctrl+Alt+T) on the SP-077 menu bar, and
bidirectional dot↔navigator selection + tooltip (T-0321–T-0324, all Verified). **AC1 + AC2 met.** Container
build green (185/185, 0 warnings) + all Linux smokes PASS; `scrivi.h` untouched (timeline C ABI complete from
EP-016). **EP-025 stays 🟡 Active** → SP-080 (scene-dot drag + Time Delta Picker + chain propagation, AC3).
Design note: `scrivi_get_timeline` returns only timeline meta; per-scene story-time is a separate
`get_scene_story_time` call + client-side gap chain (corrected on AC1/design refs at SP-079). Prior note
follows.)*

*2026-07-22 (**EP-024 `[Linux]` Scene Inspector Panel ✅ closed with user approval** — the Linux mirror of
Apple EP-014: a hideable right-side `QTabWidget` panel docked as `EditorShell`'s third splitter pane, a stub
"Scene Entities" tab, and a View ▸ Show Inspector toggle (Ctrl+Alt+I) on the SP-077 menu bar; session-scoped,
default shown, 120/200 width; window enlarged to 1020×760. Delivered & verified in a single sprint (SP-078)
like Apple EP-014/SP-037; AC1–AC6 all Verified live over VNC. Archived to `Closed/Epic-EP-024.md`. Active
Epics 1→0. Next in line: EP-025 `[Linux]` (Timeline Panel).)*
