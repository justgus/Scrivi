# Active Epics

## EP-025: [Linux] Timeline Panel

**Codebase:** `[Linux]` (Qt/QML Ubuntu app, `platforms/linux/`) — calls `[ScriviCore]` only via the existing
plain-C ABI through `ScriviBridge`. **The full timeline C ABI already exists** (EP-016/SP-039:
`scrivi_get_timeline`, `scrivi_set_scene_story_time`, story-structure, historical events, imported timelines,
export — `scrivi.h` lines 248–291). No new endpoint is expected; any gap is a Task with a `[ScriviCore]` note.

**Status:** 🟡 Active (activated 2026-07-22; first sprint SP-079 active)
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
- [ ] AC3 — **Scene-dot drag + Time Delta Picker + chain propagation:** dragging a dot horizontally opens the
  Time Delta Picker (spinner pre-populated, duration row, "Immediately after"); committing writes
  `scrivi_set_scene_story_time` and recomputes subsequent default-positioned scenes' offsets. Context-menu
  "Set Time Delta…" opens it without a drag. **(SP-080)**
- [ ] AC4 — **Story-structure bands:** applying a structure writes via `scrivi_set_story_structure` and renders
  colored bands + labels; band borders drag (proportional, persist); a dot assigns to a band by drag-up or
  context menu ("Assign to Act…"); removing a structure keeps offsets + assignments. **(SP-081)**
- [ ] AC5 — **Historical events + imported timelines + export:** author/edit/delete historical events (own
  dot color) via the C ABI; import a `.scrivi-timeline.json` (distinct grey row, window-clipped, multiple
  rows/shades, hide/show); export the project timeline. **(SP-082)**
- [ ] AC6 — **Clustering + pan/zoom + full verify:** co-located dots form an aggregate dot with a members
  popover (Apple T-0174); zoom resolves clusters; two-finger/scroll pan + zoom; panel persists all state
  across close/reopen. No regression: Linux smokes + the app-launch smoke green; writing loop / navigator /
  inspector unaffected. **(SP-083)**

> **Apple ACs N/A on Linux:** the iPhone-exclusion AC from EP-016 does not apply (Linux is desktop-only).

### Sprints (sequence — mirrors EP-016; each activated one at a time)

| Sprint | Title | Status | Dates |
| ------ | ----- | ------ | ----- |
| SP-079 | `[Linux]` Timeline panel scaffold + scene dots (story-time layout) + show/hide + dot↔navigator selection | 🟡 Active — all tasks Verified (AC1/AC2 met); ready to close | 2026-07-22 – |
| SP-080 | `[Linux]` Scene-dot drag + Time Delta Picker + chain propagation (`set_scene_story_time`) | 🔵 Planned | — |
| SP-081 | `[Linux]` Story-structure bands — overlay, border drag, band assignment | 🔵 Planned | — |
| SP-082 | `[Linux]` Historical events + imported timelines + export | 🔵 Planned | — |
| SP-083 | `[Linux]` Clustering + pan/zoom + persistence + full EP-025 verify & Epic close | 🔵 Planned | — |

### Tasks

| ID     | Title | Sprint | Status |
| ------ | ----- | ------ | ------ |
| T-0321 | `[Linux]` `ScriviBridge::getTimeline` + `getSceneStoryTime` invokables — `scrivi_get_timeline` (epoch/meta) + `scrivi_get_scene_story_time` (per-scene gap/duration) | SP-079 | ✅ Verified (2026-07-22) |
| T-0322 | `[Linux]` `TimelinePanel` widget — horizontal strip, scene dots laid out by story-time, empty state, resizable top edge, active-dot highlight, hit-testing | SP-079 | ✅ Verified (2026-07-22) |
| T-0323 | `[Linux]` Dock the panel as the editor's bottom strip (outer vertical splitter) + **View ▸ Show Timeline** toggle (Ctrl+Alt+T, session-scoped, editor-only, check-state synced) | SP-079 | ✅ Verified (2026-07-22) |
| T-0324 | `[Linux]` Dot↔navigator bidirectional selection + tooltip + `reloadTimeline` chain math (closes AC1/AC2) | SP-079 | ✅ Verified (2026-07-22) |

### Scope Notes

- **No new backend expected.** The timeline C ABI is complete (EP-016). Each sprint reconfirms the exact
  envelope it consumes at planning; a genuinely missing endpoint becomes a `[ScriviCore]` Task, not a new Epic.
- **Re-create, don't port.** The Apple `TimelineStripView` is the behavioral reference (dot layout, guards,
  Time Delta Picker semantics, clustering model T-0174); the Linux panel is native Qt widgets/painting.
- **Panel placement = bottom strip** (user decision) — docks below the existing
  navigator+viewport+inspector splitter in `EditorShell`'s `QVBoxLayout`, resizable by its top edge.
- **Menu home:** the SP-077 `QMenuBar` — **View ▸ Show Timeline** joins **View ▸ Show Inspector** (EP-024)
  on the existing `editorOnlyActions_` + `updateMenuState()` machinery.

### Completion Summary

_(filled in when the Epic reaches 🟠 Complete)_

---

*Last Updated: 2026-07-22 (**EP-025 `[Linux]` Timeline Panel activated + SP-079 implemented** — the largest
Linux Epic since Apple EP-016 (full parity, ~5 sprints, rebuilt in Qt/C++; the timeline C ABI already exists
from EP-016/SP-039). **SP-079** (read + layout + select core) 🟢 Implemented, Not Verified: `ScriviBridge`
`getTimeline` + `getSceneStoryTime` invokables, a `TimelinePanel` bottom strip with one scene dot per scene in
story-time order (chain-computed shell-side), a View ▸ Show Timeline toggle (Ctrl+Alt+T) on the SP-077 menu
bar, and bidirectional dot↔navigator selection + tooltip (T-0321–T-0324). Container build green (185/185,
0 warnings) + all Linux smokes PASS; live VNC + AC1/AC2 sign-off pending. Panel docks as a bottom strip (user
decision). Epics Active 0→1, Draft 2→1. Prior note follows.)*

*2026-07-22 (**EP-024 `[Linux]` Scene Inspector Panel ✅ closed with user approval** — the Linux mirror of
Apple EP-014: a hideable right-side `QTabWidget` panel docked as `EditorShell`'s third splitter pane, a stub
"Scene Entities" tab, and a View ▸ Show Inspector toggle (Ctrl+Alt+I) on the SP-077 menu bar; session-scoped,
default shown, 120/200 width; window enlarged to 1020×760. Delivered & verified in a single sprint (SP-078)
like Apple EP-014/SP-037; AC1–AC6 all Verified live over VNC. Archived to `Closed/Epic-EP-024.md`. Active
Epics 1→0. Next in line: EP-025 `[Linux]` (Timeline Panel).)*
