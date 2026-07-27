# SP-082: [Linux] Historical events + imported timelines + export

**Status:** ✅ Closed (2026-07-24)
**Epic:** EP-025 `[Linux]` Timeline Panel (4th of ~5 sprints; mirrors Apple EP-016 SP-042, minus the clustering
half carved out to SP-084)
**Goal:** Give the Linux timeline the last of its authored/imported content. A writer can **author, edit, and
delete historical events** — worldbuilding moments (not scenes) that appear on the project row as a muted
warm-toned (`#C8A97A`) dot, draggable in story time. A writer can **import** an external `.scrivi-timeline.json`
— its events render in a distinct **grey row below** the project row, window-clipped to the current span, with a
per-source grey shade, and each row can be hidden/shown; import shows an **epoch-offset dialog**. And a writer
can **export** the project's scene + historical events as a `.scrivi-timeline.json`. Delivers **EP-025 AC5**.
**Start Date:** 2026-07-24
**End Date:** 2026-07-24
**Capacity:** ~8–10 hours

---

### Assigned Tasks

| ID     | Title | Status |
| ------ | ----- | ------ |
| T-0340 | `[Linux]` **Timeline-events bridge invokables** — `ScriviBridge` Qt wrappers over the complete EP-016 C ABI (historical CRUD + import/update-offset/visible/list/remove + export). `parseEnvelope` + RAII `scrivi_free` + `errorOccurred` + `ready_` guard, no identity. `scrivi.h` untouched. | ✅ Verified (2026-07-24, VNC) |
| T-0341 | `[Linux]` **Historical-event dots + author/edit/delete** — `#C8A97A` dots on the project row (distinct from scene accent + imported grey, §7.2), draggable (`HistHorizontal` → `historicalEventDragged`), hover tooltip; empty-area "New Historical Event Here" (§7.9) + dot menu Edit/Delete (§7.7) → `HistoricalEventDialog` (title/description/tags; tags read from disk on Edit since the list drops them). | ✅ Verified (2026-07-24, VNC) |
| T-0342 | `[Linux]` **Imported-timeline rows + epoch-offset dialog + hide/show** — one grey row below the project row per import (source label, per-source `assignedGreyShade`, dots window-clipped §6.7, read-only, tooltip = title + source + computed time); Import Timeline… (§7.9) → `QFileDialog` → `EpochOffsetDialog` (source/epoch/signed offset + in/out-of-window preview, FR-067) → `importExternalTimeline`; row menu Edit-Offset/Hide/Remove (§7.8) + "Show Hidden Timelines" un-hide submenu (FR-065); events read from the stored files (list is metadata-only, Apple's pattern); min-height +1 row per visible import (FR-064). | ✅ Verified (2026-07-24, VNC) |
| T-0343 | `[Linux]` **Export timeline** — Export Timeline… (§7.9) → `exportProjectTimeline` → `QFileDialog` save-as writes `.scrivi-timeline.json` (scene + historical events, no prose/identity, §6.6/FR-069); re-imports into another project (smoke round-trips it). | ✅ Verified (2026-07-24, VNC) |
| T-0344 | `[Linux]` **Wire-up + `timeline_events_smoke` + verify** — `reloadTimeline`/`reloadImportedTimelines` load historical events + imported rows; new headless `timeline_events_smoke` (historical CRUD round-trip; import fixture → stored + metadata + on-disk file; hide/show + update-offset persist; export a valid `scrivi.externalTimeline.v1` carrying the surviving historical event; remove clears) + CMake + CI. Closes AC5. | ✅ Verified (2026-07-24, VNC) |
| T-0345 | `[Linux]` **File ▸ Import / Export Timeline… menu items** (user request 2026-07-24) — import/export are file ops, so they join File (below Close Project, own separator group) on the SP-077 menu bar, not only the timeline right-click. Editor-only (`editorOnlyActions_`); each forwards to a new public `EditorShell::importTimeline()`/`exportTimeline()` → the T-0342/T-0343 flow. Panel empty-area entries kept (both homes). | ✅ Verified (2026-07-24, VNC) |

### Assigned Issues

Both found in the 2026-07-24 VNC verify, fixed + re-verified same day:

| ID | Title | Severity | Status |
| -- | ----- | -------- | ------ |
| I-0090 | `[Linux]` **Lowest imported-timeline row obscured by the zoom scrollbar** — imported rows stacked from `height()` over the +/- controls + scrollbar band. Fixed: reserve a `kBottomControlsHeight` (24px) band; `importedRowY` stacks up from `height() − 24`, `projectRowY` includes it. | Medium | ✅ Verified (2026-07-24, VNC) |
| I-0091 | `[Linux]` **Import/Export file dialogs defaulted to `/root`** not the project's folder. Fixed: seed both `QFileDialog`s with `QFileInfo(projectPath_).absolutePath()` (the project's parent folder — NOT hardcoded). | Low | ✅ Verified (2026-07-24, VNC) |

### Key decisions & findings

- **The C ABI was already complete** (EP-016/SP-039): all 10 endpoints at `scrivi.h` 270–291. T-0340 added only
  Qt bridge wrappers; **`scrivi.h` untouched** this sprint.
- **List endpoints are metadata-only (no C ABI gap).** `listHistoricalEvents` → `{count, eventsJSON}` where
  `eventsJSON` = `{"events":[…]}` **without tags/slug**; `listImportedTimelines` → `{count, timelinesJSON}` =
  `{"timelines":[…metadata + eventCount…]}` **without the per-event array**. So per-event data is read **directly
  from the stored files** in `objects/historical-events/` and `objects/imported-timelines/` — exactly **Apple's
  own pattern** (its `loadImportedTimelines` comment: "the stored files include events; listImportedTimelines
  returns metadata only"). Confirmed against `scrivi_c_api.cpp` at planning; guarded by the smoke.
- **Tags** are accepted/stored by create/update but dropped by the list projection → the Edit dialog reads tags
  from the on-disk event file to prefill accurately; `tagsToJson` sends the `{"tags":[…]}` wrapper (the SP-081
  object-wrap lesson).
- **`EditorShell::reloadTimeline` is the single feed** — the panel calls no `scrivi_*` itself.
- **File-menu items are additive** (T-0345) — the panel's right-click Import/Export entries were kept (both
  homes, like Cut/Copy in Edit + right-click).

### Verification

- ✅ **Container build green** — Qt 6.4: **211/211** targets, 0 warnings; `scrivi_linux` +
  `scrivi_linux_timeline_events_smoke` linked.
- ✅ **New `timeline_events_smoke` PASS** — historical CRUD list round-trip; import fixture → stored + metadata
  + on-disk file; hide/show + update-offset persist; export a valid `scrivi.externalTimeline.v1` carrying the
  surviving historical event; remove → list empty.
- ✅ **All 11 regression smokes PASS** — story-structure, timeline-story-time, merge, create, reorder,
  chapter-reorder, editor-map, save, rename, delete, load, lifecycle, persistence.
- ✅ **Headless app-launch OK** — full window tree (incl. the SP-082 panel + dialogs) constructs offscreen.
- ✅ **Live VNC walkthrough complete (2026-07-24)** — Human verified all six tasks on
  `the-twisted-remains-of-myself` + the two findings (I-0090/I-0091) re-verified. **AC5 met.**

### New files

Linux CMake only (not pbxproj — Linux-only Qt/C++): `src/HistoricalEventDialog.{cpp,hpp}`,
`src/EpochOffsetDialog.{cpp,hpp}`, `tests/timeline_events_smoke.{cpp,sh}`. New CI step wired in
`.github/workflows/scrivi-linux-ci.yml`. `scrivi.h`/ScriviCore untouched.

### Outcome

Delivered **EP-025 AC5**. EP-025 now stands at AC1–AC5 + AC6a done; only **SP-084** (AC6b — co-located dot
clustering + panel-persistence verify + full EP-025 verify + Epic close) remains. Closed with Human approval
2026-07-24.
