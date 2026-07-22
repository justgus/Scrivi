# Active Sprint

## SP-079: [Linux] Timeline panel scaffold + scene dots + show/hide + dot↔navigator selection

**Status:** 🟡 Active
**Epic:** EP-025 `[Linux]` Timeline Panel (opens the Epic; mirrors Apple EP-015/EP-016, SP-039/SP-040 core)
**Goal:** Stand up the **Timeline Panel** on Linux — a hideable horizontal strip across the **bottom** of the
editor showing **one dot per scene in story-time order** (from `scrivi_get_timeline`; default chain when a
scene has no manual position), vertically centered, with an empty-state and a top-edge resize. Add a **View ▸
Show Timeline** toggle (session-scoped, editor-only) and **bidirectional dot↔navigator selection** (click a
dot → scroll/select the scene; active scene highlights its dot) plus a dot tooltip. This is the read-and-select
core (Apple EP-016 SP-039 read + SP-040 T-0149/0153/0173 + T-0173 selection); **drag / Time Delta Picker /
bands / events are later sprints (SP-080+).**
**Start Date:** 2026-07-22
**End Date:** —
**Capacity:** ~5–7 hours

### Assigned Tasks

| ID     | Title | Priority | Status |
| ------ | ----- | -------- | ------ |
| T-0321 | `[Linux]` **`ScriviBridge::getTimeline` (+ `getSceneStoryTime`) invokables** — `getTimeline(projectRootPath)` → `scrivi_get_timeline` (epoch label + meta); `getSceneStoryTime(projectRootPath, sceneID)` → `scrivi_get_scene_story_time` (gapMs/durationMs/offsetSource/bandID). Both cloned from the read invokables (ready-guard → `ScriviString` → `parseEnvelope`). **Two** invokables, not one — `get_timeline` returns only timeline meta; per-scene story-time is a separate endpoint (matches Apple `TimelineViewModel.load`). `scrivi.h` untouched (both exported since EP-016). | High | ✅ Verified (2026-07-22) |
| T-0322 | `[Linux]` **`TimelinePanel` widget** — new `QWidget` (`TimelinePanel.cpp/.hpp`): a horizontal strip that paints a baseline + **one dot per scene** positioned by story-time `offsetMs`, vertically centered; **empty-state** text when the project has no scenes; **min height** (80px) + top-edge resize (via the outer splitter, T-0323); active-dot highlight (filled + ring); hit-testing for clicks + hover tooltips. Chain math done shell-side (T-0324) — the panel receives resolved `Dot{offsetMs,durationMs}`. Registered in `platforms/linux/CMakeLists.txt`. | High | ✅ Verified (2026-07-22) |
| T-0323 | `[Linux]` **Dock as the editor's bottom strip + View ▸ Show Timeline toggle** — wrapped `EditorShell`'s 3-pane horizontal `splitter_` and the new `timeline_` in an outer **vertical `QSplitter`** (`outerSplitter_`: panels above, timeline below, sizes `{620,120}`, non-collapsible); `setTimelineVisible`/`isTimelineVisible` (session-scoped, default **shown**). Added **View ▸ Show Timeline** to the SP-077 `QMenuBar` (checkable, **Ctrl+Alt+T**, editor-only, check-state synced in `updateMenuState()` under a `QSignalBlocker`). Panel (re)loads from `bridge_->getTimeline` + `getSceneStoryTime` on open and after every structural change (via `rebuildNavigator`). | High | ✅ Verified (2026-07-22) |
| T-0324 | `[Linux]` **Dot↔navigator bidirectional selection + tooltip + verify** — dot click → `sceneClicked(sceneID)` → `moveCaretToSegment` (same navigate as the navigator click) + focus; active-scene change highlights its dot via `selectNavigatorScene` (the single hook both the caret + scroll paths route through). Hover tooltip = title + chapter + human-readable story-time (`humanStoryTime`, e.g. "3 days, 4 hours after Story Open"). Container build + smokes green; live VNC walkthrough confirmed. Closes **AC1 + AC2**. | High | ✅ Verified (2026-07-22) |

### Assigned Issues

_(none yet — new issues found during SP-079 are logged here.)_

### Sprint Notes

- **Reference — Apple EP-016 (`Epics/Closed/Epic-EP-016.md`, `Scrivi/Views/TimelineStripView.swift`).** This
  sprint is the Linux mirror of the EP-016 **read + layout + select** core: SP-039's `scrivi_get_timeline`
  consumption, SP-040 **T-0149** (horizontal line + scene dots, manuscript/story-time order), **T-0153** (dot
  tooltip: title + human-readable story-time), **T-0154** (panel resize, dynamic min height), and SP-043
  **T-0173** (bidirectional Timeline↔Navigator selection). The Apple `TimelineViewModel.load` (line ~246) is
  the layout reference — dots derive from the `openProject` scene list joined with `get_timeline` story-time;
  a scene with `offsetSource == "default"` sits on the gap chain (`offsetMs` computed, not stored).
- **The C ABI is already complete.** `scrivi_get_timeline` (+ the whole timeline surface) is exported in
  `libScriviCore.a` from EP-016. `scrivi.h` is **untouched** this sprint — T-0321 only adds the Qt bridge
  wrapper. Reconfirm the `get_timeline` envelope field names against the header/an existing decode at task
  start.
- **Panel placement — bottom strip (user decision 2026-07-22).** `EditorShell`'s root is a `QVBoxLayout`
  (toolbar / horizontal-splitter / errorLabel). Wrap the existing `splitter_` (navigator|viewport|inspector)
  and the `TimelinePanel` in a **vertical `QSplitter`** so the timeline is a resizable bottom strip under all
  three panes — Apple's EP-016 layout. Non-collapsible via drag; the menu is the hide path (as with the
  inspector).
- **Menu home.** **View ▸ Show Timeline** joins **View ▸ Show Inspector** (EP-024) under the existing **View**
  menu on the SP-077 `QMenuBar`, using the same `editorOnlyActions_` + `updateMenuState()` machinery.
  **Ctrl+Alt+T** (the timeline analogue of the inspector's Ctrl+Alt+I; not macOS→VNC-intercepted).
- **Scope this sprint = read + layout + select only.** No drag, no Time Delta Picker, no bands, no historical/
  imported events, no clustering, no pan/zoom — those are SP-080–SP-083. The dots are display + click-to-select
  in SP-079.
- **Default duration / chain.** When scenes have no manual story-time, positions come from the default gap
  chain (`project.json` `timelineDefaults.defaultSceneDurationMs`, 1 hour). `get_timeline` already returns the
  resolved per-scene `offsetMs`/`durationMs`; the panel positions from those (no client-side chain math needed
  for read — the endpoint owns it).
- **pbxproj:** N/A — Linux/Qt only. New `TimelinePanel.cpp/.hpp` go in `platforms/linux/CMakeLists.txt`, not
  `Scrivi.xcodeproj/project.pbxproj` (pbxproj is Apple-only).
- **Verify:** container build + all existing Linux smokes green (no new headless smoke — the panel is pure UI
  painting/interaction, verified live over VNC), then the VNC walkthrough per T-0324.
- **Out of scope:** everything from SP-080+ (drag, picker, chain-write, bands, events, import/export,
  clustering, pan/zoom), and any `scrivi.h` change (none needed).

### Implementation notes (2026-07-22)

- **Design correction found at implementation.** `scrivi_get_timeline` returns **only timeline meta**
  (`timelineID`, `epochLabel`, `projectID`, `createdAt`) — **not** per-scene story-time. Apple's
  `TimelineViewModel.load` gets the epoch from `getTimeline` and each scene's story-time from a **separate**
  `scrivi_get_scene_story_time` call, then computes `offsetMs` **client-side** from the gap chain
  (`recomputeAllOffsets`). So T-0321 added **two** invokables (`getTimeline` + `getSceneStoryTime`), and the
  chain math lives shell-side in `EditorShell::reloadTimeline` — the panel only lays out resolved dots. The
  sprint plan's "getTimeline returns per-scene story-time" wording was wrong; corrected here.
- **T-0321 — 🟢.** `ScriviBridge::getTimeline` / `getSceneStoryTime` (`ScriviBridge.hpp`/`.cpp`), cloned from
  the read invokables (ready-guard → `ScriviString` → `parseEnvelope`). Read-only, no author identity. `scrivi.h`
  untouched (both endpoints exported since EP-016/SP-039).
- **T-0322 — 🟢.** `TimelinePanel` (`platforms/linux/src/TimelinePanel.{hpp,cpp}`): a `QWidget` that paints a
  theme-aware baseline + one dot per `Dot{sceneID,title,chapterTitle,offsetMs,durationMs}`, x-positioned by
  `offsetMs` over the `[minMs,maxMs]` window (guarded against a zero-width window), vertically centred; the
  active dot is filled with `QPalette::Highlight` + a ring. Empty-state text when no dots. `setMinimumHeight(80)`.
  `mousePressEvent` hit-tests dots → `sceneClicked`; `QEvent::ToolTip` shows title + chapter + `humanStoryTime`.
  Registered in the Linux CMake app target.
- **T-0323 — 🟢.** `EditorShell` builds `timeline_` and an outer **vertical** `QSplitter` (`outerSplitter_`)
  holding the existing 3-pane `splitter_` above and `timeline_` below (sizes `{620,120}`, timeline
  non-collapsible); `root` now hosts `outerSplitter_`. `setTimelineVisible`/`isTimelineVisible` (session-scoped,
  default shown). **View ▸ Show Timeline** added to `ScriviWindow::buildMenuBar` (checkable, Ctrl+Alt+T,
  `editorOnlyActions_`, check-state synced in `updateMenuState` under a `QSignalBlocker` — same pattern as Show
  Inspector). `showTimelineAction_` member.
- **T-0324 — 🟢 (build+smokes; live verify pending).** `reloadTimeline()` builds the dots from `segments()` +
  the backend: epoch from `getTimeline`, per-scene `gapMs`/`durationMs` from `getSceneStoryTime`, chained
  (`offset[i] = prevEnd + gapMs[i]`, `prevEnd = offset[i]+duration[i]`; unset → gap 0 + 1h default). Called at
  the end of `load()` and at the end of `rebuildNavigator()` (the single post-structural-change funnel), guarded
  by `loading_` so the mid-assembly navigator rebuilds don't fire it. Dot click → `moveCaretToSegment` + focus;
  active-dot highlight is driven from `selectNavigatorScene` (the one hook the caret + scroll active-scene paths
  both call), so the timeline tracks the navigator with no extra wiring.

### Verification (2026-07-22, container)

- ✅ **Build green** — Qt 6.4 container build: **185/185** targets (the new `TimelinePanel` target added);
  `TimelinePanel.cpp` + edited `ScriviBridge.cpp`/`EditorShell.cpp`/`ScriviWindow.cpp` compiled with **0 errors,
  0 warnings**; `scrivi_linux` linked.
- ✅ **Regression smokes green** — `scene_merge_smoke`, `scene_create_smoke`, `scene_reorder_smoke`,
  `chapter_reorder_smoke`, `scene_load_smoke`, `editor_map_smoke` all PASS (no `SceneDocument`/`ScriviBridge`
  behavioral change beyond the two additive read invokables).
- ✅ **App-launch smoke** — boots under Xvfb and stays up (the timeline wiring loads without crashing).
- ✅ **Live VNC walkthrough + AC1/AC2 sign-off — VERIFIED (2026-07-22, user).** Dots render in story-time
  order on the bottom strip; **View ▸ Show Timeline** (menu + Ctrl+Alt+T) hides/shows it; the strip resizes by
  its top-edge divider; clicking a dot scrolls/selects that scene; the active scene highlights its dot (both
  directions); the tooltip reads title + story-time; a scene-less project shows the empty-state; the View item
  is inert on the landing page. **T-0321/T-0322/T-0323/T-0324 all Verified; EP-025 AC1 + AC2 met.** No new
  headless smoke — pure UI painting/interaction, verified live.

### Retrospective

_(filled in at close)_

---

*Last Updated: 2026-07-22 (**SP-079 implemented & ✅ Verified live over VNC** — EP-025 `[Linux]` Timeline
Panel's read + layout + select core. T-0321–T-0324 all ✅ Verified (user, VNC): a hideable bottom timeline
strip (`TimelinePanel`) with one scene dot per scene in story-time order (chain-computed from `getTimeline` +
`getSceneStoryTime`), a View ▸ Show Timeline toggle (Ctrl+Alt+T) on the SP-077 menu bar, and bidirectional
dot↔navigator selection + tooltip. **EP-025 AC1 + AC2 met.** Container build green (185/185, 0 warnings) + all
Linux smokes PASS. `scrivi.h` untouched. **SP-079 ready to close** (awaiting user approval) — EP-025 continues
to SP-080 (drag + Time Delta Picker, AC3). Next available Task T-0325; next Sprint SP-080.)*
