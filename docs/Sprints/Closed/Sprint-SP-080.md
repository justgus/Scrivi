# SP-080 (Closed)

## SP-080: [Linux] Scene-dot drag + Time Delta Picker + chain propagation

**Status:** ✅ Closed (Human-approved 2026-07-22)
**Epic:** EP-025 `[Linux]` Timeline Panel (2nd of ~5 sprints; mirrors Apple EP-016 SP-040 T-0150/0151/0168)
**Goal:** Make the timeline scene dots **interactive**: a writer drags a dot horizontally and, on release, the
**Time Delta Picker** opens (spinner pre-populated to the dragged story-time, a scene-duration row, and an
"Immediately after / Reset to default" option). Committing writes the scene's story-time via
`scrivi_set_scene_story_time` (as a **manual** placement) and **chain-propagates** — every subsequent
default-positioned scene's `offsetMs` is recomputed and re-persisted. The picker is also reachable **without a
drag** via a dot context-menu **"Set Time Delta…"**. Delivered **EP-025 AC3**.
**Start Date:** 2026-07-22
**End Date:** 2026-07-22
**Capacity:** ~6–8 hours

### Assigned Tasks

| ID     | Title | Priority | Status |
| ------ | ----- | -------- | ------ |
| T-0325 | `[Linux]` **`ScriviBridge::setSceneStoryTime` invokable** — → `scrivi_set_scene_story_time(root, sceneID, offsetMs, source, gapMs, durationMs, durationSource)` (exported since EP-016). Peer to SP-079's `getSceneStoryTime`. Structural, no author identity. `scrivi.h` untouched. | High | ✅ Verified (2026-07-22) |
| T-0326 | `[Linux]` **`TimelinePanel` dot drag** — press-on-dot → horizontal drag (4px threshold) live-moves the dot (`dragX_`) → `dotDragged(sceneID, offsetForX())` on release; press-with-no-move stays a click (`sceneClicked`). Right-click **"Set Time Delta…"** → `setTimeDeltaRequested`. Drag is **on the dot only**; empty-background left free for pan (SP-083). | High | ✅ Verified (2026-07-22) |
| T-0327 | `[Linux]` **`TimeDeltaPicker` dialog** (`TimeDeltaPicker.cpp/.hpp`) — amount spinbox + unit combo (Minutes…Years) + direction (Later/Before) seeded via Apple `bestFit`; a scene-duration row; **"Immediately after previous" (reset)** + Set/Cancel. Returns an `Outcome` (SetOffset/ResetDefault/Cancelled) + `resultOffsetMs`/`resultDurationMs`. No backend calls in the dialog. | High | ✅ Verified (2026-07-22) |
| T-0328 | `[Linux]` **Wire drag/menu → picker → commit + chain propagation** — `EditorShell::showTimeDeltaPicker` seeds from the `reloadTimeline` offset/duration cache, computes `gap = newOffset − previousSceneEnd`, `setSceneStoryTime(…, "manual", gap, …)` (or reset → `"default"`, gap 0), then **recompute + re-persist** every following scene from the gap chain (Apple `recomputeAndPersistFrom`), then `reloadTimeline`. New `timeline_story_time_smoke` (+ CMake + CI). Closes **AC3**. | High | ✅ Verified (2026-07-22) |

### Assigned Issues

_(none — no new issues logged during SP-080.)_

### Sprint Notes

- **Reference — Apple EP-016 (`Scrivi/Views/TimelineStripView.swift`).** The behavioral model:
  - **Canonical stored value is `gapMs`** (gap from the previous scene's END to this scene's START), not the
    absolute `offsetMs`. A manual placement stores `offsetSource="manual"` + the computed `gapMs`; `offsetMs`
    is always **derived** by the chain (`recomputeAllOffsets`: `offset[i] = prevEnd + gapMs[i]`).
  - **`TimeDeltaPicker`** (`:2624`) — amount/unit/direction spinner + a duration row + "Immediately after"
    (reset). Result cases: `.spinner`/`.anchor`/`.keepPosition` → absolute offset; `.resetDefault`.
  - **`applyPickerResult`** (`:1549`) — `gap = ms − previousSceneEndMs(dot)`; `setStoryTime(source:"manual",
    gapMs:gap, durationMs:dur, …)`; else reset.
  - **`recomputeAndPersistFrom`** (`:548`) — after a manual set, walk subsequent dots: recompute `offsetMs`
    from the chain and re-persist each with its own `gapMs` (chain propagation).
- **The C ABI was already complete.** `scrivi_set_scene_story_time` is exported (EP-016/SP-039). `scrivi.h`
  stayed **untouched** — T-0325 only added the Qt bridge wrapper.
- **Drag vs click vs pan (VNC-safe).** A press that moves past a 4px threshold is a drag (→ picker on release);
  a press with no move stays a click (→ navigate). Empty-background drag is left free for pan (SP-083). All
  plain mouse press/move/release — fully VNC-deliverable (unlike trackpad gestures).
- **pbxproj:** N/A — Linux/Qt only. `TimeDeltaPicker.cpp/.hpp` in `platforms/linux/CMakeLists.txt`.

### Implementation summary (2026-07-22)

- **T-0325** — `ScriviBridge::setSceneStoryTime`, peer to `getSceneStoryTime` (ready-guard → C ABI →
  `parseEnvelope`). Structural, no author identity.
- **T-0326** — `TimelinePanel` drag: `mousePressEvent` records `pressedDot_`; `mouseMoveEvent` flips
  `dragging_` past `kDragThreshold` (4px) and live-updates `dragX_` (repainting the dragged dot, highlighted);
  `mouseReleaseEvent` emits `dotDragged(sceneID, offsetForX(dragX_))` on a real drag, else `sceneClicked`. New
  `offsetForX` inverts `xForOffset` (clamped ≥ 0). `contextMenuEvent` → one-item "Set Time Delta…" `QMenu` →
  `setTimeDeltaRequested`. Background press leaves `pressedDot_ = -1`.
- **T-0327** — `TimeDeltaPicker` `QDialog`: prompt + amount `QSpinBox` + unit `QComboBox` (Minutes…Years,
  Apple's fixed ms incl. 30d/365d) + direction `QComboBox` + a scene-duration row + "Immediately after
  previous" reset + Set/Cancel. Seeded via `initialiseFromRaw` (`bestFit`). Returns `Outcome` +
  `resultOffsetMs`/`resultDurationMs`; no backend calls (the shell owns persistence); offset clamped ≥ 0.
- **T-0328** — `EditorShell::showTimeDeltaPicker` seeds from the per-scene offset/duration cache built in
  `reloadTimeline` (`timelineOffsets_`/`timelineDurations_`; anchor = previous scene's END, no extra
  round-trip). SetOffset → manual placement (`gap = offset − prevEnd`); ResetDefault → source "default", gap 0.
  Then chain propagation from the changed index onward (recompute + re-persist each with its own stored
  `gapMs`/source), then `reloadTimeline`. Signals wired where `timeline_` is built.

### Verification (2026-07-22)

- ✅ **Build green** — Qt 6.4 container build: **193/193** targets; new `TimeDeltaPicker.cpp` + edited
  `TimelinePanel.cpp`/`ScriviBridge.cpp`/`EditorShell.cpp` + the new smoke compiled **0 errors, 0 warnings**;
  `scrivi_linux` + `scrivi_linux_timeline_story_time_smoke` linked.
- ✅ **New `timeline_story_time_smoke` PASS** — manual placement (a scene 2h after the previous scene's end,
  source "manual") + chain re-persist + reset-to-default all round-trip through reopen (the AC3 backend
  contract, headless).
- ✅ **Regression smokes green** — `scene_merge_smoke`, `scene_create_smoke`, `scene_reorder_smoke`,
  `chapter_reorder_smoke`, `scene_load_smoke`, `editor_map_smoke` all PASS.
- ✅ **App-launch smoke** — boots under Xvfb and stays up.
- ✅ **Live VNC walkthrough (user-verified 2026-07-22):** dragging a dot follows the pointer; on release the
  Time Delta Picker opens seeded to the dragged position; committing shifts the dot + the scenes after it and
  survives quit→reopen; the dot right-click "Set Time Delta…" opens the same picker without a drag; "Immediately
  after previous" returns the scene to the chain; a plain click still navigates. **T-0325/T-0326/T-0327/T-0328
  all Verified; EP-025 AC3 met.**

### Retrospective

- **What went well:** The SP-079 offset/duration cache (built for the tooltip) fed the picker's previous-scene
  anchor with no extra backend round-trips, and the SP-079 dot hit-test made click-vs-drag a small threshold
  change. The `gapMs`-canonical + client-side chain model ported cleanly from Apple; the headless smoke locked
  down the endpoint + chain math while the drag/dialog went to VNC.
- **What surfaced:** Nothing broke; the drag needed only a click-vs-drag threshold to coexist with SP-079's
  click-navigate, and deliberately not claiming the background keeps SP-083 pan available. The pan/zoom input
  decision (Ctrl+wheel + buttons, not trackpad pinch) was settled this sprint and recorded for SP-083.
- **Outcome:** EP-025 AC3 Verified. Scene dots are now positionable in story-time; SP-081 adds the
  story-structure band overlay on top.

---

*Closed 2026-07-22 (Human-approved). SP-080 delivered interactive timeline scene dots: drag → Time Delta
Picker → `scrivi_set_scene_story_time` (manual placement) + chain propagation; + a dot context-menu "Set Time
Delta…". EP-025 AC3 Verified. `scrivi.h` untouched (endpoint from EP-016). EP-025 stays 🟡 Active → SP-081
(story-structure bands, AC4).*
