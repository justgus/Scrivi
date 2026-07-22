# SP-079 (Closed)

## SP-079: [Linux] Timeline panel scaffold + scene dots + show/hide + dot↔navigator selection

**Status:** ✅ Closed (Human-approved 2026-07-22)
**Epic:** EP-025 `[Linux]` Timeline Panel (opened the Epic; mirrors Apple EP-015/EP-016, SP-039/SP-040 core)
**Goal:** Stand up the **Timeline Panel** on Linux — a hideable horizontal strip across the **bottom** of the
editor showing **one dot per scene in story-time order** (chain-computed from the backend story-time; default
chain when a scene has no manual position), vertically centered, with an empty-state and a top-edge resize.
Add a **View ▸ Show Timeline** toggle (session-scoped, editor-only) and **bidirectional dot↔navigator
selection** (click a dot → scroll/select the scene; active scene highlights its dot) plus a dot tooltip. This
was the read-and-select core; **drag / Time Delta Picker / bands / events are later sprints (SP-080+).**
**Start Date:** 2026-07-22
**End Date:** 2026-07-22
**Capacity:** ~5–7 hours

### Assigned Tasks

| ID     | Title | Priority | Status |
| ------ | ----- | -------- | ------ |
| T-0321 | `[Linux]` **`ScriviBridge::getTimeline` (+ `getSceneStoryTime`) invokables** — `getTimeline` → `scrivi_get_timeline` (epoch label + meta); `getSceneStoryTime` → `scrivi_get_scene_story_time` (gapMs/durationMs/offsetSource/bandID). **Two** invokables — `get_timeline` returns only timeline meta; per-scene story-time is a separate endpoint (matches Apple `TimelineViewModel.load`). `scrivi.h` untouched (both exported since EP-016). | High | ✅ Verified (2026-07-22) |
| T-0322 | `[Linux]` **`TimelinePanel` widget** — `QWidget` (`TimelinePanel.cpp/.hpp`): horizontal strip painting a baseline + **one dot per scene** by story-time `offsetMs`, vertically centered; **empty-state** when no scenes; **min height** (80px) + top-edge resize; active-dot highlight (filled + ring); hit-testing for clicks + hover tooltips. Chain math done shell-side — the panel receives resolved `Dot{offsetMs,durationMs}`. Registered in the Linux CMake. | High | ✅ Verified (2026-07-22) |
| T-0323 | `[Linux]` **Dock as the editor's bottom strip + View ▸ Show Timeline toggle** — wrapped the 3-pane horizontal `splitter_` and `timeline_` in an outer **vertical `QSplitter`** (`outerSplitter_`: panels above, timeline below, sizes `{620,120}`, non-collapsible); `setTimelineVisible`/`isTimelineVisible` (session-scoped, default **shown**). **View ▸ Show Timeline** on the SP-077 `QMenuBar` (checkable, **Ctrl+Alt+T**, editor-only, check-state synced under a `QSignalBlocker`). Panel (re)loads on open + after every structural change (via `rebuildNavigator`). | High | ✅ Verified (2026-07-22) |
| T-0324 | `[Linux]` **Dot↔navigator bidirectional selection + tooltip + verify** — dot click → `sceneClicked(sceneID)` → `moveCaretToSegment` + focus; active-scene change highlights its dot via `selectNavigatorScene` (the single caret+scroll hook). Hover tooltip = title + chapter + human-readable story-time (`humanStoryTime`). Closes **AC1 + AC2**. | High | ✅ Verified (2026-07-22) |

### Assigned Issues

_(none — no new issues logged during SP-079.)_

### Sprint Notes

- **Reference — Apple EP-016 (`Epics/Closed/Epic-EP-016.md`, `Scrivi/Views/TimelineStripView.swift`).** The
  Linux mirror of the EP-016 **read + layout + select** core: SP-040 **T-0149** (horizontal line + scene dots,
  story-time order), **T-0153** (dot tooltip), **T-0154** (panel resize, min height), and SP-043 **T-0173**
  (bidirectional Timeline↔Navigator selection). Apple's `TimelineViewModel.load` (line ~246) is the layout
  reference.
- **The C ABI was already complete.** `scrivi_get_timeline` + the whole timeline surface were exported in
  `libScriviCore.a` from EP-016. `scrivi.h` stayed **untouched** — T-0321 only added Qt bridge wrappers.
- **Panel placement — bottom strip (user decision 2026-07-22).** The existing 3-pane `QSplitter`
  (navigator|viewport|inspector) + the `TimelinePanel` are wrapped in an outer **vertical `QSplitter`** so the
  timeline is a resizable bottom strip under all three panes — Apple's EP-016 layout. Non-collapsible via drag;
  the menu is the hide path (as with the inspector).
- **Menu home.** **View ▸ Show Timeline** joins **View ▸ Show Inspector** (EP-024) under the **View** menu on
  the SP-077 `QMenuBar`, using the same `editorOnlyActions_` + `updateMenuState()` machinery. **Ctrl+Alt+T**
  (the timeline analogue of the inspector's Ctrl+Alt+I; not macOS→VNC-intercepted).
- **pbxproj:** N/A — Linux/Qt only. `TimelinePanel.cpp/.hpp` in `platforms/linux/CMakeLists.txt`, not
  `Scrivi.xcodeproj/project.pbxproj`.

### Implementation summary (2026-07-22)

- **Design correction found at implementation.** `scrivi_get_timeline` returns **only timeline meta**
  (`timelineID`, `epochLabel`, `projectID`, `createdAt`) — **not** per-scene story-time. Apple's
  `TimelineViewModel.load` gets the epoch from `getTimeline` and each scene's story-time from a **separate**
  `scrivi_get_scene_story_time` call, then computes `offsetMs` **client-side** from the gap chain
  (`recomputeAllOffsets`). So T-0321 added **two** invokables and the chain math lives shell-side in
  `EditorShell::reloadTimeline` — the panel only lays out resolved dots. The sprint plan's "getTimeline returns
  per-scene story-time" wording was wrong; corrected at implementation.
- **T-0321** — `ScriviBridge::getTimeline` / `getSceneStoryTime`, cloned from the read invokables (ready-guard
  → `ScriviString` → `parseEnvelope`). Read-only, no author identity.
- **T-0322** — `TimelinePanel`: paints a theme-aware baseline + one dot per `Dot{sceneID,title,chapterTitle,
  offsetMs,durationMs}`, x-positioned over the `[minMs,maxMs]` window (guarded against a zero-width window),
  vertically centred; the active dot is filled `QPalette::Highlight` + a ring. Empty-state when no dots.
  `mousePressEvent` hit-tests → `sceneClicked`; `QEvent::ToolTip` → title + chapter + `humanStoryTime`.
- **T-0323** — `EditorShell` builds `timeline_` + `outerSplitter_` (vertical) holding the 3-pane `splitter_`
  above and `timeline_` below (`{620,120}`, non-collapsible); `root` hosts `outerSplitter_`.
  `setTimelineVisible`/`isTimelineVisible` (session-scoped, default shown). **View ▸ Show Timeline** in
  `ScriviWindow::buildMenuBar` (checkable, Ctrl+Alt+T, `editorOnlyActions_`, `QSignalBlocker`-guarded check-state
  sync); `showTimelineAction_` member.
- **T-0324** — `reloadTimeline()` chains `gapMs`/`durationMs` from `getSceneStoryTime` into per-dot offsets
  (`offset[i] = prevEnd + gapMs[i]`; unset → gap 0 + 1h default), called at the end of `load()` and
  `rebuildNavigator()` (the single post-structural-change funnel), guarded by `loading_`. Dot click →
  `moveCaretToSegment` + focus; active-dot highlight driven from `selectNavigatorScene` (the one caret+scroll
  active-scene hook), so the timeline tracks the navigator with no extra wiring.

### Verification (2026-07-22)

- ✅ **Build green** — Qt 6.4 container build: **185/185** targets; `TimelinePanel.cpp` + edited
  `ScriviBridge.cpp`/`EditorShell.cpp`/`ScriviWindow.cpp` compiled with **0 errors, 0 warnings**; `scrivi_linux`
  linked.
- ✅ **Regression smokes green** — `scene_merge_smoke`, `scene_create_smoke`, `scene_reorder_smoke`,
  `chapter_reorder_smoke`, `scene_load_smoke`, `editor_map_smoke` all PASS (no `SceneDocument`/`ScriviBridge`
  behavioral change beyond the two additive read invokables).
- ✅ **App-launch smoke** — boots under Xvfb and stays up.
- ✅ **Live VNC walkthrough (user-verified 2026-07-22):** dots render in story-time order on the bottom strip;
  **View ▸ Show Timeline** (menu + Ctrl+Alt+T) hides/shows it; the strip resizes by its top-edge divider;
  clicking a dot scrolls/selects that scene; the active scene highlights its dot (both directions); the tooltip
  reads title + story-time; a scene-less project shows the empty-state; the View item is inert on the landing
  page. **T-0321/T-0322/T-0323/T-0324 all Verified; EP-025 AC1 + AC2 met.**
- **No new headless smoke** — the panel is pure UI painting/interaction, verified live over VNC.

### Retrospective

- **What went well:** The existing infrastructure paid off — the SP-061 `QSplitter` layout took the timeline as
  a clean outer vertical splitter, and the SP-077 `QMenuBar`/`editorOnlyActions_`/`updateMenuState` machinery
  hosted the Show Timeline toggle with zero new plumbing (the inspector's Ctrl+Alt+I → the timeline's Ctrl+Alt+T).
  Routing the active-dot highlight through the single `selectNavigatorScene` hook meant the timeline tracked the
  navigator for free. Clean first-pass container build, 0 warnings.
- **What surfaced:** The plan's assumption that `scrivi_get_timeline` returns per-scene story-time was wrong —
  caught at implementation by reading Apple's `TimelineViewModel.load`, which uses a separate
  `get_scene_story_time` per scene + client-side gap-chain math. Split into two invokables + shell-side chain;
  the correction is recorded on EP-025 AC1 and its design references so SP-080+ start from the right model.
- **Outcome:** EP-025 AC1 + AC2 Verified. The timeline strip reads, lays out, shows/hides, and selects
  bidirectionally — the foundation the drag/picker/bands/events sprints (SP-080–SP-083) build on.

---

*Closed 2026-07-22 (Human-approved). SP-079 delivered the Linux Timeline panel's read + layout + select core:
a hideable bottom strip with scene dots in story-time order (chain-computed from `getTimeline` +
`getSceneStoryTime`), a View ▸ Show Timeline toggle (Ctrl+Alt+T) on the SP-077 menu bar, and bidirectional
dot↔navigator selection + tooltip. EP-025 AC1 + AC2 Verified. `scrivi.h` untouched (timeline C ABI complete
from EP-016). EP-025 stays 🟡 Active → SP-080 (scene-dot drag + Time Delta Picker + chain propagation, AC3).*
