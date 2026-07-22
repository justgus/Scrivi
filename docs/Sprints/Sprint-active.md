# Active Sprint

## SP-080: [Linux] Scene-dot drag + Time Delta Picker + chain propagation

**Status:** 🟡 Active
**Epic:** EP-025 `[Linux]` Timeline Panel (2nd of ~5 sprints; mirrors Apple EP-016 SP-040 T-0150/0151/0168)
**Goal:** Make the timeline scene dots **interactive**: a writer drags a dot horizontally and, on release, the
**Time Delta Picker** opens (spinner pre-populated to the dragged story-time, a scene-duration row, and an
"Immediately after / Reset to default" option). Committing writes the scene's story-time via
`scrivi_set_scene_story_time` (as a **manual** placement) and **chain-propagates** — every subsequent
default-positioned scene's `offsetMs` is recomputed and re-persisted. The picker is also reachable **without a
drag** via a dot context-menu **"Set Time Delta…"**. Delivers **EP-025 AC3**.
**Start Date:** 2026-07-22
**End Date:** —
**Capacity:** ~6–8 hours

### Assigned Tasks

| ID     | Title | Priority | Status |
| ------ | ----- | -------- | ------ |
| T-0325 | `[Linux]` **`ScriviBridge::setSceneStoryTime` invokable** — `setSceneStoryTime(projectRootPath, sceneID, offsetMs, source, gapMs, durationMs, durationSource)` → `scrivi_set_scene_story_time` (already exported, EP-016). Returns the updated story-time result. Structural, no author identity. `scrivi.h` untouched. | High | ✅ Verified (2026-07-22) |
| T-0326 | `[Linux]` **`TimelinePanel` dot drag** — press-on-dot begins a horizontal drag (past a 4px threshold) that live-moves the dot along the baseline (x clamped, drawn at `dragX_`); on release emits `dotDragged(sceneID, newOffsetMs)` (release x → offset via new `offsetForX`, clamped ≥ 0). A press-with-no-move stays a click (`sceneClicked`). Drag is **on the dot only**; empty-background left free for pan (SP-083). Also a right-click **"Set Time Delta…"** context menu → `setTimeDeltaRequested(sceneID)`. | High | ✅ Verified (2026-07-22) |
| T-0327 | `[Linux]` **`TimeDeltaPicker` dialog** — `QDialog` (`TimeDeltaPicker.cpp/.hpp`) mirroring Apple's picker: amount spinbox + unit combo (Minutes…Years) + direction (Later/Before) pre-populated (Apple `bestFit`) from the dragged offset relative to the previous scene's end; a **scene-duration** row; **"Immediately after previous" (reset)** + Set/Cancel. Returns an `Outcome` (SetOffset / ResetDefault / Cancelled) + `resultOffsetMs`/`resultDurationMs`. No backend calls in the dialog. | High | ✅ Verified (2026-07-22) |
| T-0328 | `[Linux]` **Wire drag/menu → picker → commit + chain propagation + verify** — `EditorShell::onDotDragged` / `onSetTimeDeltaRequested` → `showTimeDeltaPicker`: seeds from the reloadTimeline offset/duration cache (`timelineOffsets_`/`timelineDurations_`), computes `gap = newOffset − previousSceneEnd`, `setSceneStoryTime(…, "manual", gap, dur, "manual")` (or reset → `"default"`, gap 0), then **recompute + re-persist** every following scene from the gap chain (Apple `recomputeAndPersistFrom`), then `reloadTimeline`. New `timeline_story_time_smoke` (+ CMake + CI). Container build + smokes green; live VNC walkthrough confirmed. Closes **AC3**. | High | ✅ Verified (2026-07-22) |

### Assigned Issues

_(none yet — new issues found during SP-080 are logged here.)_

### Sprint Notes

- **Reference — Apple EP-016 (`Scrivi/Views/TimelineStripView.swift`).** The behavioral model:
  - **Canonical stored value is `gapMs`** (gap from the previous scene's END to this scene's START), not the
    absolute `offsetMs`. A manual placement stores `offsetSource="manual"` + the computed `gapMs`; `offsetMs`
    is always **derived** by the chain (`recomputeAllOffsets`: `offset[i] = prevEnd + gapMs[i]`).
  - **`TimeDeltaPicker`** (`TimelineStripView.swift:2624`) — amount/unit/direction spinner + a duration row +
    "Immediately after" (reset). Its result cases: `.spinner(ms)` / `.anchor(ms)` / `.keepPosition(ms)` →
    absolute offset; `.resetDefault`.
  - **`applyPickerResult`** (`:1549`) — `gap = ms − previousSceneEndMs(dot)`; `setStoryTime(sceneID, offsetMs:ms,
    source:"manual", gapMs:gap, durationMs:dur, durationSource:"manual")`; else reset.
  - **`recomputeAndPersistFrom`** (`:548`) — after a manual set, walk subsequent dots: recompute `offsetMs` from
    the chain and re-persist each with its own `gapMs` (chain propagation). This is what makes "move scene N →
    everything after it shifts" work.
- **The C ABI is already complete.** `scrivi_set_scene_story_time(projectRootPath, sceneID, offsetMs, source,
  gapMs, durationMs, durationSource)` is exported (EP-016/SP-039). `scrivi.h` is **untouched** — T-0325 only
  adds the Qt bridge wrapper (peer to SP-079's `getSceneStoryTime`).
- **Drag vs click vs pan (VNC-safe).** SP-079's dot hit-test gives press detection. A press that moves past a
  small threshold is a **drag** (→ picker on release); a press with no move stays a **click** (→ `sceneClicked`
  navigate). Empty-background drag is **left free for pan (SP-083)**. All of this is plain mouse press/move/
  release — fully VNC-deliverable (unlike trackpad gestures; see the pan/zoom note below).
- **Pan/zoom decision (recorded for SP-083, user 2026-07-22):** Qt/X11/VNC cannot carry trackpad pinch or
  high-res pixelDelta (same class as the SP-076/077 swallowed keys). So EP-025 pan/zoom will be **Ctrl+wheel
  zoom + toolbar +/−** and **click-drag-background + Shift+wheel pan**, with native pinch only as an optional
  bonus on real Ubuntu. **Not this sprint** — but SP-080's dot-drag must not claim background-drag, so pan stays
  available later. (Memory: `project_linux_vnc_input_constraints`.)
- **Menu/context.** The context-menu "Set Time Delta…" is added to the timeline dot's right-click (a small
  `QMenu` in `TimelinePanel`, emitting a `setTimeDeltaRequested(sceneID)` signal), separate from the navigator
  context menu.
- **pbxproj:** N/A — Linux/Qt only. New `TimeDeltaPicker.cpp/.hpp` (if a separate file) go in
  `platforms/linux/CMakeLists.txt`, not `Scrivi.xcodeproj/project.pbxproj`.
- **Verify:** container build + all existing Linux smokes green. A **new headless smoke** is warranted here
  (unlike SP-079's pure-UI panel): `timeline_story_time_smoke` — drive `setSceneStoryTime` for a manual
  placement + the chain re-persist through `ScriviBridge`, then reopen and assert the offsets/sources persisted
  (the endpoint + chain math are headless-testable; the drag/picker UI is verified live over VNC).
- **Out of scope:** story-structure bands (SP-081), historical/imported events (SP-082), clustering + pan/zoom
  (SP-083), and any `scrivi.h` change (none needed).

### Implementation notes (2026-07-22)

- **T-0325 — 🟢.** `ScriviBridge::setSceneStoryTime` (`ScriviBridge.hpp`/`.cpp`), peer to SP-079's
  `getSceneStoryTime`: ready-guard → `scrivi_set_scene_story_time(root, sceneID, offsetMs, source, gapMs,
  durationMs, durationSource)` → `parseEnvelope`. Structural, no author identity. `scrivi.h` untouched.
- **T-0326 — 🟢.** `TimelinePanel` gained drag: `mousePressEvent` records `pressedDot_`; `mouseMoveEvent` flips
  `dragging_` once the pointer moves past `kDragThreshold` (4px) and live-updates `dragX_` (repainting the
  dragged dot at the pointer, highlighted); `mouseReleaseEvent` emits `dotDragged(sceneID, offsetForX(dragX_))`
  on a real drag, else `sceneClicked` (preserving SP-079 click-navigate). New `offsetForX` inverts `xForOffset`
  (clamped ≥ 0). `contextMenuEvent` shows a one-item "Set Time Delta…" `QMenu` → `setTimeDeltaRequested`.
  Background press leaves `pressedDot_ = -1` (reserved for SP-083 pan).
- **T-0327 — 🟢.** `TimeDeltaPicker` (`platforms/linux/src/TimeDeltaPicker.{hpp,cpp}`): a `QDialog` — a prompt
  ("Time after \"prev\" ends:"), amount `QSpinBox` + unit `QComboBox` (Minutes…Years, Apple's fixed ms incl.
  30d/365d month/year) + direction `QComboBox` (Later/Before), a scene-duration row, an "Immediately after
  previous" reset button, and Set/Cancel. Seeded via `initialiseFromRaw` (Apple `bestFit` on the raw offset +
  current duration). Returns an `Outcome` enum + `resultOffsetMs`/`resultDurationMs`; **no backend calls** — the
  shell owns persistence. Result offset clamped ≥ 0 (never before the epoch).
- **T-0328 — 🟢 (build+smokes; live verify pending).** `EditorShell::showTimeDeltaPicker` seeds the picker from
  the per-scene offset/duration cache built in `reloadTimeline` (`timelineOffsets_`/`timelineDurations_`), so
  the anchor = previous scene's END with no extra backend round-trip. On SetOffset it writes the manual
  placement (`gap = offset − prevEnd`, source "manual"); on ResetDefault it writes source "default", gap 0. Then
  **chain propagation**: walk every scene from the changed index onward, recompute `offset[i]` from the running
  chain, and re-persist each with its own stored `gapMs`/source (Apple `recomputeAndPersistFrom`) — so a moved
  scene shifts everything after it while default scenes stay on the chain. Finally `reloadTimeline`. Signals
  wired where `timeline_` is built. Cache cleared/rebuilt each reload.

### Verification (2026-07-22, container)

- ✅ **Build green** — Qt 6.4 container build: **193/193** targets; new `TimeDeltaPicker.cpp` + edited
  `TimelinePanel.cpp`/`ScriviBridge.cpp`/`EditorShell.cpp` + the new smoke compiled with **0 errors, 0 warnings**;
  `scrivi_linux` + `scrivi_linux_timeline_story_time_smoke` linked.
- ✅ **New `timeline_story_time_smoke` PASS** — manual placement (a scene 2h after the previous scene's end,
  source "manual") + chain re-persist + reset-to-default all round-trip through reopen (the AC3 backend
  contract, headless).
- ✅ **Regression smokes green** — `scene_merge_smoke`, `scene_create_smoke`, `scene_reorder_smoke`,
  `chapter_reorder_smoke`, `scene_load_smoke`, `editor_map_smoke` all PASS.
- ✅ **App-launch smoke** — boots under Xvfb and stays up.
- ✅ **Live VNC walkthrough — VERIFIED (2026-07-22, user).** Dragging a scene dot follows the pointer; on
  release the Time Delta Picker opens seeded to the dragged position; committing shifts the dot + the scenes
  after it and survives quit→reopen; the dot right-click "Set Time Delta…" opens the same picker without a
  drag; "Immediately after previous" returns the scene to the chain; a plain click still navigates (SP-079).
  **T-0325/T-0326/T-0327/T-0328 all Verified; EP-025 AC3 met.**

### Retrospective

_(filled in at close)_

---

*Last Updated: 2026-07-22 (**SP-080 activated & implemented same day** — EP-025 `[Linux]` Timeline Panel's 2nd
sprint: interactive scene dots. Drag a dot → **Time Delta Picker** on release → commit via
`scrivi_set_scene_story_time` (manual placement) → **chain-propagate** subsequent scenes' offsets; also a dot
context-menu "Set Time Delta…". Tasks T-0325–T-0328 🟢 Implemented, Not Verified. Container build green
(193/193, 0 warnings); new `timeline_story_time_smoke` PASS (manual placement + chain re-persist + reset
round-trip) + all regression smokes + Xvfb app-launch PASS; **live VNC walkthrough (drag/picker UI) pending**.
`scrivi.h` untouched (endpoint from EP-016); canonical stored value is `gapMs` + derived `offsetMs` chain.
Pan/zoom decided for SP-083 (Ctrl+wheel + buttons; drag/Shift+wheel — no trackpad pinch, VNC can't carry it).
Next available Task T-0329; next Sprint SP-081 (story-structure bands).)*
