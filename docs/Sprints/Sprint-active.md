# Active Sprint

> **Two active sprints** (permitted across concerns), both EP-025 `[Linux]` Timeline: **SP-081** (story-structure
> bands) is implementation-complete but its last task **T-0332** (scene→band drag-up assignment) can't be
> live-verified — a far-outlier scene collapses all dots to one strip edge under the linear time-axis
> (**I-0087**), so you can't spread/grab a dot to drag it onto a band. **SP-083** (timeline zoom/pan) is
> **brought forward** to fix I-0087 and is the current focus; SP-081 closes once SP-083's zoom lets T-0332 be
> verified. (SP-082 historical/imported events stays next-after; clustering + Epic close move to a new SP-084.)

---

## SP-083: [Linux] Timeline zoom + pan (brought forward — fixes I-0087, unblocks SP-081 T-0332)

**Status:** 🟡 Active
**Epic:** EP-025 `[Linux]` Timeline Panel (brought forward from its planned position to fix I-0087; mirrors
Apple EP-016 SP-043 pan/zoom — `TimelineScrollCaptureView` scroll-wheel + magnify)
**Goal:** Let the writer **zoom and pan** the linear timeline so a far-outlier scene (e.g. a 2-year flashback)
no longer collapses every other dot to one edge (**I-0087**). Keep the **linear time-axis** (Apple parity — NOT
even-spacing). **Zoom = `Ctrl`+mouse-wheel** (the universal X11/Linux zoom idiom) **zoom-about-the-pointer**,
plus an **always-works `+`/`−` control** at the strip's bottom-right (a plain click — guaranteed over VNC and for
non-power-users; zoom about the pointer, or the strip center if the pointer is away). **Pan = click-drag on the
empty area above/below the dots** (the background was reserved for pan in SP-080). A horizontal scrollbar appears
when zoomed in. Delivers **EP-025 AC6a** and unblocks **SP-081 T-0332**.
**Start Date:** 2026-07-22
**End Date:** —
**Capacity:** ~5–7 hours

### Assigned Tasks

| ID     | Title | Priority | Status |
| ------ | ----- | -------- | ------ |
| T-0333 | `[Linux]` **`TimelinePanel` zoom model** — a `zoom_` factor (≥1) + `panFraction_` offset threaded through `xForOffset`/`offsetForX` (and band geometry) so the strip renders a zoomable/pannable window over `[minMs_, maxMs_]`. **`Ctrl`+mouse-wheel** (`wheelEvent`) zooms **about the pointer** (the story-time under the cursor stays put); clamp to [1, ~big]; zoom 1 = full-fit (current behavior). No axis-model change (still linear time). | High | 🔵 Backlog |
| T-0334 | `[Linux]` **`+`/`−` zoom control + horizontal scrollbar** — a small horizontal control at the strip's **bottom-right** (`+` left, `−` right) that zooms about the current pointer (or center if outside), stepping the same `zoom_` model; a `QScrollBar` (or painted equivalent) appears along the bottom when `zoom_ > 1` to pan. VNC-safe (plain clicks). | High | 🔵 Backlog |
| T-0335 | `[Linux]` **Pan by background drag + verify (closes I-0087 / unblocks T-0332)** — click-drag on the empty area **above/below** the dots pans (adjust `panFraction_`); the dot-drag (SP-080) + border-drag (SP-081) still win when the press is on a dot/border. VNC walkthrough: zoom into a crowded cluster (Ctrl+wheel **and** the `+` button), dots separate, pan across; then **re-verify SP-081 T-0332** — drag a spread-out dot up onto a band label to assign it. Closes **I-0087** + **T-0332/AC4** + **AC6a**. | High | 🔵 Backlog |

### Assigned Issues

| ID     | Title | Severity | Status |
| ------ | ----- | -------- | ------ |
| I-0087 | `[Linux]` Timeline dots crowd to one edge when a scene is a far time-outlier; no zoom to spread them (blocks SP-081 T-0332) | High | 🔴 Open — fixed by this sprint |

### Sprint Notes

- **Reference — Apple EP-016 (`Scrivi/Views/TimelineStripView.swift` `TimelineScrollCaptureView`, `:2810`).**
  Apple captures `scrollWheel` (trackpad pan/momentum) + magnify to pan/zoom the same linear axis. Linux
  re-creates the *behavior* with VNC-safe inputs (Ctrl+wheel + buttons + background-drag) since trackpad
  pinch/pixelDelta don't survive VNC (memory: `project_linux_vnc_input_constraints`).
- **Keep the linear time-axis (user decision 2026-07-22).** Do NOT switch to even/ordinal spacing. The fix for
  crowding is zoom + pan, exactly as on Apple.
- **Zoom-about-the-pointer:** on a zoom step, hold the story-time offset under the cursor fixed (adjust
  `panFraction_` so `xForOffset(offsetUnderCursor)` stays at the cursor x). Center-anchored when the pointer is
  outside the strip (the `+`/`−` buttons with no hover).
- **Gesture priority (mouse):** press on a **dot** → SP-080 story-time drag / SP-079 click; press on a **band
  border** → SP-081 re-proportion; press on the **empty area above/below the dots** → **pan** (this sprint). So
  T-0335 slots into the existing `mousePressEvent` zone check without disturbing SP-080/081.
- **VNC caveat (why the buttons matter):** `Ctrl`+wheel carries the modifier over VNC, but a Mac Magic
  Mouse/trackpad may not emit a discrete wheel x11vnc forwards — so the **`+`/`−` buttons are the guaranteed
  path** for the developer VNC loop and for writers without a wheel. Ship both.
- **pbxproj:** N/A — Linux/Qt only. All changes in `TimelinePanel.cpp/.hpp` (+ maybe a small control widget).
- **Verify:** container build + all existing Linux smokes green (zoom/pan is pure UI over the existing model —
  no new headless smoke; verified live over VNC). The headline check is **I-0087 gone + T-0332 verifiable**.
- **Out of scope:** clustering/aggregate dots (SP-084), historical/imported events (SP-082).

### Implementation notes

_(filled in as tasks land)_

### Retrospective

_(filled in at close)_

---

## SP-081: [Linux] Story-structure bands — overlay, border drag, band assignment

**Status:** 🟡 Active
**Epic:** EP-025 `[Linux]` Timeline Panel (3rd of ~5 sprints; mirrors Apple EP-016 SP-041 T-0156–T-0160)
**Goal:** Add **story-structure bands** to the Linux timeline. A writer picks a built-in structure (Three Act,
Five Act, Hero's Journey, …) from a menu; the timeline paints **colored, proportional bands** behind the scene
dots with labels. Band **borders drag** to re-proportion (persisted). A scene **assigns to a band** — by
dragging its dot up onto the band's label row and via a dot context-menu **"Assign to Act…"** — and shows a
colored ring; removing the structure keeps every scene's offsets + band assignments. Delivers **EP-025 AC4**.
**Start Date:** 2026-07-22
**End Date:** —
**Capacity:** ~7–9 hours

### Assigned Tasks

| ID     | Title | Priority | Status |
| ------ | ----- | -------- | ------ |
| T-0329 | `[Linux]` **Story-structure bridge invokables** — `getStoryStructure`/`setStoryStructure`/`updateBandLayout`/`removeStoryStructure` + `assignSceneToBand`/`unassignSceneFromBand` (all exported since EP-016; `scrivi.h` untouched). Plus the **built-in band table** (structureID → bands) ported from Apple `BuiltInStructure.bandTable` into `StoryStructures.cpp/.hpp` + `bandLayoutJSON`/`parseBandLayout` helpers. | High | ✅ Verified (2026-07-22) |
| T-0330 | `[Linux]` **Band overlay painting + Structure selector** — `TimelinePanel::setBands` + `paintEvent` paints translucent colored proportional bands + labels **behind** the dots + a label row; **View ▸ Story Structure…** on the SP-077 menu bar pops the built-ins + "None (Remove)" (`EditorShell::pickStoryStructure` → `chooseStoryStructure`). Loaded via `getStoryStructure` in `reloadTimeline`; bands render only when a structure is set. | High | ✅ Verified (2026-07-22) |
| T-0331 | `[Linux]` **Band border drag (re-proportion + persist)** — pressing a border zone (hit-tested first, `borderIndexNearX`) drags proportion between adjacent bands (each > 0.05 floor, pair-sum constant); live preview via `dragProportions_`; on release `bandProportionsChanged` → `EditorShell::onBandProportionsChanged` edits the cached layout + `updateBandLayout`. | High | ✅ Verified (2026-07-22) |
| T-0332 | `[Linux]` **Scene→band assignment (drag-up + context menu) + verify** — a dot dragged **up into the band label row** → `sceneAssignedToBand(sceneID, bandID)` (`bandIndexAtX` at release) + a colored ring in the band's color; dot context-menu **"Assign to Act…"** (submenu of current bands) + **"Unassign"**. Assignment survives structure-remove (bandID kept — AC4). New `story_structure_smoke` (+ CMake + CI). Container build + smokes green. **BLOCKED on live verify** by **I-0087** (dot-crowding: a far-outlier scene collapses all others to one edge; without zoom/spread you can't grab a dot to drag it up onto a band). | High | 🟡 Implemented — build+smokes green; live verify **blocked by I-0087** |

### Assigned Issues

_(none yet — new issues found during SP-081 are logged here.)_

### Sprint Notes

- **Reference — Apple EP-016 (`Scrivi/Views/TimelineStripView.swift`).** The pieces to mirror:
  - **`BuiltInStructure` + `bandTable`** (`:19`–) — the preset structures and their `StoryBand`
    `{bandID, label, color (hex), proportion}` lists (Three Act = three 1/3 bands, Five Act = five 0.2, Hero's
    Journey = twelve 1/12, etc.). Port the table to a Linux C++ constant (T-0329).
  - **`BandOverlayView`** (`:1588`) — paints colored proportional bands + labels behind the dots, draggable
    borders, and the label row as the drag-up assignment target.
  - **`SceneDotView` band ring** — an assigned dot shows a ring in its band's color.
  - Assignment writes via `assign_scene_to_band` / `unassign_scene_from_band`; the structure layout via
    `set_story_structure(structureID, bandLayoutJSON)` / `update_band_layout(bandLayoutJSON)`; removal via
    `remove_story_structure` **keeps** each scene's `offsetMs` + `bandID` (AC "removing a structure preserves
    offsets + assignments").
- **The C ABI is already complete.** `scrivi_get/set/update/remove_story_structure` +
  `scrivi_assign/unassign_scene_to_band` are all exported (EP-016/SP-039 — `scrivi.h` lines 260–268). `scrivi.h`
  is **untouched** this sprint — T-0329 adds Qt bridge wrappers only. Reconfirm the `bandLayoutJSON` shape
  (array of `{bandID,label,color,proportion}`) + the `StoryStructureResult` envelope
  (`{hasStructure, structureID, bandLayoutJSON}`) at task start.
- **Drag disambiguation (VNC-safe).** Three drags now share the strip: **dot horizontal** (SP-080, story-time),
  **dot up-onto-a-band** (assignment, T-0332), and **band border** (re-proportion, T-0331). Resolve by START
  ZONE + direction: a press on a border zone → border drag; a press on a dot that moves mostly **up** into the
  label row → assignment; a press on a dot that moves mostly **horizontally** → story-time (SP-080). Empty
  background still reserved for pan (SP-083). All plain mouse events — VNC-deliverable.
- **Bands paint BEHIND the dots.** `paintEvent` order: bands (translucent fills + labels) → baseline → dots →
  epoch label. The dots + their SP-079/080 hit-testing/geometry are unchanged; bands are a backdrop layer keyed
  off the loaded structure. No structure set → no bands (exact SP-080 appearance).
- **App owns the presets, backend stores the layout.** The built-in band tables live in the Linux app (ported
  from Apple); `set_story_structure` just persists whatever `bandLayoutJSON` it's handed, and border-drag edits
  go through `update_band_layout`. So "custom" proportions are just an edited layout — no new schema.
- **pbxproj:** N/A — Linux/Qt only. Any new `.cpp/.hpp` (e.g. a `StoryStructures` presets unit) go in
  `platforms/linux/CMakeLists.txt`.
- **Verify:** container build + all existing Linux smokes green + a **new `story_structure_smoke`** (set a
  structure, assign a scene, reopen → layout + assignment persisted; remove → assignment kept). The band
  painting + border/assignment drags are verified **live over VNC** (the persistence + layout are headless).
- **Out of scope:** historical/imported events (SP-082), clustering + pan/zoom (SP-083), a full custom-band
  editor (built-ins + border-drag re-proportion only this sprint), and any `scrivi.h` change (none needed).

### Implementation notes (2026-07-22)

- **T-0329 — 🟢.** Six `ScriviBridge` invokables (`getStoryStructure`/`setStoryStructure`/`updateBandLayout`/
  `removeStoryStructure`/`assignSceneToBand`/`unassignSceneFromBand`), each ready-guard → C ABI →
  `parseEnvelope`. New `StoryStructures.{hpp,cpp}` (`scrivi::linux_app::story`) ports Apple's full
  `bandTable` (Three Act / Five Act / Hero's Journey / Save the Cat / Freytag / Kishōtenketsu / In Medias
  Res) + `bandLayoutJSON`/`parseBandLayout`. **`scrivi.h` untouched.**
- **JSON-shape finding (caught by the smoke).** ScriviCore's `set/get/update_story_structure` expect the
  `bandLayoutJSON` to be an **object wrapping a `"bands"` array** — `{"bands":[{bandID,label,color,proportion},…]}`
  — NOT a bare array (`ScriviCore.cpp` reads `doc.arraySize("bands")`; `getStoryStructure` builds it via
  `appendToArray("bands", …)`). The first `story_structure_smoke` run FAILED (nothing persisted) with a bare
  array; fixed `bandLayoutJSON`/`parseBandLayout` to use the `{"bands":[…]}` wrapper — smoke green. This is
  exactly why the sprint added a headless smoke rather than trusting the format.
- **T-0330 — 🟢.** `TimelinePanel::setBands` + `paintEvent` paints translucent (`alpha 48`) colored
  proportional band rects + elided labels + border separators **behind** the baseline/dots; empty bands → no
  paint (exact SP-080 look). `getStoryStructure` loaded in `reloadTimeline` (structureID + layout cached in
  `currentStructureID_`/`currentBandLayoutJSON_`); per-scene `bandID` read from `getSceneStoryTime`. **View ▸
  Story Structure…** added to `ScriviWindow::buildMenuBar` → `EditorShell::pickStoryStructure` →
  `chooseStoryStructure` (a `QMenu` of the built-ins, current checked, + "None (Remove)").
- **T-0331 — 🟢.** Drag disambiguation by START ZONE: `mousePressEvent` checks `borderIndexNearX` (5px grab)
  FIRST → `DragMode::Border`; else a dot → mode decided on first move by direction (up-into-label-row =
  `DotToBand`, else `DotHorizontal`). Border drag moves proportion between the two adjacent bands (0.05 floor,
  pair-sum constant) live via `dragProportions_`; release emits `bandProportionsChanged` →
  `onBandProportionsChanged` (edit cached layout + `updateBandLayout`).
- **T-0332 — 🟢 (build+smokes; live verify pending).** A `DotToBand` release resolves the band under the
  pointer (`bandIndexAtX`) → `sceneAssignedToBand` → `assignSceneToBand` + reload; the dot paints a ring in the
  band's color (`sceneBands_`). Dot context menu gained "Assign to Act…" (`assignBandRequested` → a shell
  submenu of the current bands) + "Unassign" (`unassignBandRequested`). Removal preserves the bandID (endpoint
  contract, asserted by the smoke).

### Verification (2026-07-22, container)

- ✅ **Build green** — Qt 6.4 container build: **202/202** targets; new `StoryStructures.cpp` + edited
  `TimelinePanel.cpp`/`ScriviBridge.cpp`/`EditorShell.cpp`/`ScriviWindow.cpp` + the new smoke compiled with
  **0 errors, 0 warnings**; `scrivi_linux` + `scrivi_linux_story_structure_smoke` linked.
- ✅ **New `story_structure_smoke` PASS** — set the three-act structure (persists on reopen), assign a scene to
  a band (bandID persists), re-proportion the bands via `updateBandLayout` (new proportions persist), remove the
  structure → **the scene's bandID assignment is PRESERVED (the AC4 contract)**. (First run FAILED on the JSON
  shape; fixed → green.)
- ✅ **Regression smokes green** — `timeline_story_time_smoke`, `scene_merge_smoke`, `scene_create_smoke`,
  `scene_reorder_smoke`, `chapter_reorder_smoke`, `scene_load_smoke`, `editor_map_smoke` all PASS.
- ✅ **App-launch smoke** — boots under Xvfb and stays up.
- ✅ **Live VNC — T-0329/T-0330/T-0331 VERIFIED (2026-07-22, user).** **View ▸ Story Structure…** applies a
  structure → colored proportional bands + labels paint behind the dots (T-0330); dragging a band border
  re-proportions and persists (T-0331); the bridge invokables + presets underpin both (T-0329).
- 🔴 **T-0332 (scene→band drag-up assignment) BLOCKED by I-0087.** In a real project a single flashback scene
  (~2 years before the rest) stretches the linear story-time window so every other dot collapses to one edge,
  overlapping and un-grabbable — a small drag just opens the Time Delta Picker, so a dot can't be dragged up
  onto a band. This is the **linear time-axis with no zoom** (I-0087), not a T-0332 defect. **User decision
  (2026-07-22):** keep the linear axis (Apple parity), fix via zoom/pan — **SP-083 brought forward** (activated
  2026-07-22). Once zoom lands, re-run the T-0332 assignment verify zoomed into the cluster. The context-menu
  "Assign to Act…"/"Unassign" path and the assignment persistence are proven headlessly by `story_structure_smoke`.

### Retrospective

_(filled in at close)_

---

*Last Updated: 2026-07-22 (**SP-081 T-0329/0330/0331 ✅ Verified; T-0332 blocked by I-0087 → SP-083 zoom/pan
brought forward & activated** (two parallel EP-025 sprints). SP-081's bridge/presets + band overlay + View ▸
Story Structure… selector + band border-drag are Verified live; scene→band drag-up assignment (T-0332) is
blocked because a far-outlier flashback scene crowds all dots to one edge under the linear axis (I-0087). User
kept the linear axis (Apple parity) and brought SP-083 forward to add zoom (`Ctrl`+wheel zoom-about-pointer + an
always-works `+`/`−` control) + pan (drag empty area above/below the dots); tasks T-0333–T-0335 re-verify T-0332
zoomed in. Clustering + Epic close → new SP-084. The SP-081 note below is retained.)*

*2026-07-22 (**SP-081 activated & implemented same day** — EP-025 `[Linux]` Timeline Panel's 3rd
sprint: story-structure bands. A **View ▸ Story Structure…** selector paints colored proportional bands + labels
behind the dots; band borders drag to re-proportion (`update_band_layout`); a scene assigns to a band by
drag-up-onto-a-label or an "Assign to Act…" context menu (`assign_scene_to_band`) showing a colored ring;
removing a structure keeps offsets + assignments. Tasks T-0329–T-0332 🟢 Implemented, Not Verified. Container
build green (202/202, 0 warnings); new `story_structure_smoke` PASS (a JSON-shape bug — the layout is
`{"bands":[…]}` not a bare array — was caught by the smoke and fixed) + all regression smokes + Xvfb app-launch
PASS; **live VNC walkthrough (band paint + drags) pending**. `scrivi.h` untouched (story-structure C ABI from
EP-016); built-in band tables ported from Apple. Next available Task T-0333; next Sprint SP-082 (historical +
imported events).)*
