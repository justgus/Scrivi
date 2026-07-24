# Active Tasks

**Epic:** EP-025 `[Linux]` (Timeline Panel) — **two active sprints:** SP-083 (zoom/pan, focus) + SP-081
(story-structure bands) (`Sprints/Sprint-active.md`).

**SP-083** `[Linux]` — timeline zoom + pan (brought forward to fix **I-0087** / unblock SP-081 T-0332):

| ID | Title | Status |
| -- | ----- | ------ |
| T-0333 | **`[Linux]` `TimelinePanel` zoom model** — `zoom_` factor + `panFraction_` over `xForOffset`/`offsetForX`; **`Ctrl`+wheel** zoom-about-pointer (`zoomAbout`/`wheelEvent`); clamp [1,500]; zoom 1 = full-fit. Linear axis kept. | ✅ **Verified (2026-07-23, VNC)** — zoom works; exercised throughout the I-0088/T-0336–0339 session. |
| T-0334 | **`[Linux]` `+`/`−` zoom control + scrollbar** — bottom-right `QToolButton` `+`/`−` (`zoomInStep`/`zoomOutStep`, about pointer/center) + a horizontal `QScrollBar` when `zoom_ > 1` (`syncScrollBar`/`resizeEvent`). VNC-safe (plain clicks). | ✅ **Verified (2026-07-23, VNC)** — user confirmed "the zoom buttons work". |
| T-0335 | **`[Linux]` Pan (background drag) + verify** — empty-area press with `zoom_>1` → `DragMode::Pan` (closed-hand) adjusts `panFraction_`; dot/border drags still win. VNC: zoom a crowded cluster (`+` button and/or wheel), dots separate, pan; **re-verify SP-081 T-0332** (drag a dot up onto a band). Closes **I-0087 + T-0332/AC4 + AC6a**. | ✅ **Verified (2026-07-23, VNC)** — zoom + pan spread the crowded cluster and let the flashback/scene work proceed. |
| T-0336 | **`[Linux]` Time Delta Picker — anchor to ANY scene** — the anchor combo lists the previous scene's end, Story Open (0), **and the END of every other scene** (`TimeDeltaPicker::AnchorScene` list from `EditorShell::showTimeDeltaPicker`, built off the `timelineOffsets_`/`timelineDurations_` caches). Choosing a scene resolves to an absolute `manual` offset **once** — no persisted anchor link, no backend/schema change (user decision 2026-07-23). Lets "Scene 14 immediately after 'We go to the Lab'" be one click. Re-open seeds the anchor to the nearest match. | ✅ **Verified (2026-07-23, VNC)** — user set Scene 14 to immediately-after "We go to the Lab" from the picker. |
| T-0337 | **`[Linux]` Story bands wrap the main storyline, not the whole strip** — `bandRegionLeftX/RightX` = `xForOffset(0)`…`xForOffset(storyEndMs_)`, so bands cover story-time **[0, last-scene-end]** and **expand/contract with zoom/pan** (`bandLeftX/RightX` + the border-drag proportion math divide that region, not `usable`). A flashback left of the epoch has **no band behind it** but is still assignable — `bandIndexAtX` snaps an out-of-region drop to the nearer band (first if left of the region), and the context-menu "Assign to Act…" is position-independent. Fixes bands being dragged into the negative/flashback region. | ✅ **Verified (2026-07-23, VNC)** — user confirmed bands wrap the storyline, resize with zoom, and border moves persist. |
| T-0338 | **`[Linux]` Persist timeline zoom + pan per project** — `TimelinePanel::viewStateChanged(zoom, pan)` fires when zoom/pan settles (`zoomAbout`, scrollbar, pan-drag release); `EditorShell::onTimelineViewStateChanged` writes it to an INI **under the app-support root** (`timelineViewStatePath()` = `<appSupport>/timeline-view.ini`, group `timeline/<projectID>`) + `sync()`. `reloadTimeline` restores via `setViewState` (clamps + repaints, does NOT re-emit → no save echo). **Fix (2026-07-23):** first cut used default `QSettings` → wrote to `~/.config`, which the Docker `--rm` container wiped on exit ("not saved"); moved into the bind-mounted app-support dir so it survives restarts. | ✅ **Verified (2026-07-23, VNC)** — zoom/pan persists across container restart. |
| T-0339 | **`[Linux]` Truncate long scene titles in the picker's anchor combo** — a sentence-length title (e.g. "There were a total of twenty four men…") no longer stretches the dialog: the anchor combo caps width (`AdjustToMinimumContentsLengthWithIcon` + `setMinimumContentsLength(24)` + `setMaximumWidth(260)`) and elides each entry to 40 chars with an ellipsis, keeping the full title in the item's tooltip (Scene-Navigator feel). | ✅ **Verified (2026-07-23, VNC)** — user confirmed the truncation renders correctly. |

**Verification (2026-07-22, container):** ✅ build green (0 warnings, `scrivi_linux` linked); ✅ regression smokes
PASS (story-structure/timeline-story-time/merge/create/reorder/chapter-reorder/editor-map); ✅ Xvfb app-launch
PASS. ⏳ **Live VNC pending** — `+`/`−` + Ctrl+wheel zoom, scrollbar + background-drag pan, and the headline
**I-0087** check: zoom into the crowded cluster (the `+` button is the VNC-safe path if the Mac wheel doesn't
reach x11vnc), dots separate, then **re-verify T-0332** band-assignment. **Next available T-0340.** Zoom/pan is
pure UI (no new headless smoke); `scrivi.h` untouched; no pbxproj (Linux-only).

**T-0336 / T-0337 (2026-07-23, follow-on to I-0088) — ✅ Verified (VNC).** The flashback fix surfaced two
follow-ups — (1) a scene chains off its manuscript predecessor, so an untitled Scene 14 followed the Flashback
back to −2y; the picker now anchors to **any scene's end** (resolved once to a manual offset — no schema
change); (2) story bands filled the whole strip and got dragged into the flashback region — they now wrap
**[0, last-scene-end]** and zoom with the timeline, while a flashback stays assignable. User verified both over
VNC: Scene 14 set immediately-after "We go to the Lab"; bands wrap the storyline, resize with zoom, border
moves persist.

**T-0338 / T-0339 (2026-07-23, same VNC session) — 🟢 Implemented, Not Verified.** Two polish items the user
asked for after verifying T-0336/0337: (T-0338) **persist the timeline zoom/pan** per project so it doesn't
reset to full-fit on every open (Linux-only `QSettings`, keyed by projectID — not the backend); (T-0339)
**truncate long scene titles** in the picker's anchor combo so a sentence-length title no longer stretches the
dialog (cap width + elide to 40 chars, full title in tooltip — Scene-Navigator feel). All in
`TimeDeltaPicker.cpp/.hpp`, `TimelinePanel.cpp/.hpp`, `EditorShell.cpp/.hpp`. `scrivi.h`/backend untouched; no
pbxproj (Linux-only).

**Container build + smokes GREEN (2026-07-23):** `docker build --no-cache` compiled all 202 targets incl.
`scrivi_linux` with 0 errors; all 9 relevant smoke wrappers PASS (timeline_story_time, story_structure,
scene_merge/create/reorder, chapter_reorder, editor_map, scene_save/rename) with `QT_QPA_PLATFORM=offscreen`.
T-0339 (title truncation) **user-verified over VNC (2026-07-23)**.

**T-0338 persistence fix (2026-07-23):** the user reported zoom/pan not surviving. Root cause **found +
proven**: default `QSettings` wrote to `/root/.config/Caposoft/Scrivi.conf`, which the `docker run --rm`
container is NOT bind-mounting (only `/root/.local/share/Scrivi` and `/projects`), so the file was destroyed on
container exit. Fixed by writing the INI into the app-support root (`<appSupport>/timeline-view.ini`) + `sync()`.
Verified in-container that the INI now lands at `/root/.local/share/Scrivi/timeline-view.ini` and **appears on
the host bind-mount** (survives `--rm`), round-tripping zoom/pan. ✅ **Verified (2026-07-23, VNC)** — user
confirmed zoom/pan persists across quit → re-run `build-and-run.sh` → reopen.

---

**SP-081** `[Linux]` — story-structure bands (T-0332 blocked by I-0087, verifies after SP-083):

| ID | Title | Status |
| -- | ----- | ------ |
| T-0329 | **`[Linux]` Story-structure bridge invokables** — `getStoryStructure`/`setStoryStructure`/`updateBandLayout`/`removeStoryStructure` + `assignSceneToBand`/`unassignSceneFromBand` (exported since EP-016; `scrivi.h` untouched). Plus the **built-in band table** ported from Apple into `StoryStructures.cpp/.hpp` + `bandLayoutJSON`/`parseBandLayout` (`{"bands":[…]}` wrapper — the ScriviCore contract). | ✅ Verified (2026-07-22) |
| T-0330 | **`[Linux]` Band overlay painting + Structure selector** — `TimelinePanel::setBands`/`paintEvent` paints translucent colored proportional bands + labels **behind** the dots + a label row; **View ▸ Story Structure…** (`pickStoryStructure` → `chooseStoryStructure`) pops built-ins + Remove; loaded via `getStoryStructure` in `reloadTimeline`. Bands render only when a structure is set. | ✅ Verified (2026-07-22) |
| T-0331 | **`[Linux]` Band border drag** — press a border zone (`borderIndexNearX`, hit-tested first) → drag proportion between adjacent bands (0.05 floor, pair-sum constant); live `dragProportions_`; release → `bandProportionsChanged` → `updateBandLayout`. | ✅ Verified (2026-07-22) |
| T-0332 | **`[Linux]` Scene→band assignment + verify** — dot dragged up into the label row → `sceneAssignedToBand` (`bandIndexAtX`) + colored ring; context-menu **"Assign to Act…"** / **"Unassign"**; assignment survives structure-remove. New `story_structure_smoke` (+ CMake + CI). Closes **AC4**. | 🟡 Implemented — build+smokes green; **live verify blocked by I-0087** (needs SP-083 zoom to spread crowded dots) |

**Verification (2026-07-22, container):** ✅ build green (202/202, 0 warnings, both binaries linked);
✅ **new `story_structure_smoke` PASS** (set structure → assign scene → re-proportion → remove-keeps-assignment
= AC4; a JSON-shape bug — layout is `{"bands":[…]}` not a bare array — was caught by the smoke and fixed);
✅ regression smokes PASS (timeline-story-time/merge/create/reorder/chapter-reorder/scene-load/editor-map);
✅ Xvfb app-launch PASS. ⏳ **Live VNC walkthrough pending** (View ▸ Story Structure… paints bands; border drag
re-proportions; dot drag-up assigns + ring; "Assign to Act…"/"Unassign"; remove keeps assignment; SP-080 dot
drag + SP-079 click still work). **Next available T-0333.** `scrivi.h` untouched (C ABI from EP-016); no pbxproj
(Linux-only).

---

**SP-080 (EP-025 `[Linux]`) ✅ closed 2026-07-22 (Human-approved).** T-0325/T-0326/T-0327/T-0328 all ✅
**Verified** live over VNC and archived with the sprint (`Closed/Sprint-SP-080.md`): `setSceneStoryTime`
invokable, `TimelinePanel` dot drag + "Set Time Delta…" context menu, the `TimeDeltaPicker` `QDialog`, and
drag/menu → picker → commit (manual placement) + chain propagation. **EP-025 AC3 met.** New
`timeline_story_time_smoke`. `scrivi.h` untouched (endpoint from EP-016).

---

**SP-079 (EP-025 `[Linux]`) ✅ closed 2026-07-22 (Human-approved).** T-0321/T-0322/T-0323/T-0324 all ✅
**Verified** live over VNC and archived with the sprint (`Closed/Sprint-SP-079.md`):

- **T-0321** — `ScriviBridge::getTimeline` + `getSceneStoryTime` invokables (epoch/meta + per-scene gap/duration).
- **T-0322** — `TimelinePanel` widget (bottom strip, scene dots in story-time order, empty-state, resize,
  active-dot highlight, hit-testing).
- **T-0323** — docked as the editor's bottom strip (outer vertical splitter) + View ▸ Show Timeline toggle
  (Ctrl+Alt+T) on the SP-077 menu bar.
- **T-0324** — bidirectional dot↔navigator selection + tooltip + `reloadTimeline` gap chain.

**EP-025 AC1 + AC2 Verified.** `scrivi.h` untouched (timeline C ABI complete from EP-016); no pbxproj
(Linux-only); no new headless smoke (pure UI, verified live). **EP-025 continues → SP-080** (scene-dot drag +
Time Delta Picker + chain propagation, AC3). **Next available Task: T-0325.**

---

**SP-078 (EP-024 `[Linux]`) ✅ closed 2026-07-22 (Human-approved) — this closed EP-024.** T-0318/T-0319/T-0320
all ✅ **Verified** live over VNC and archived with the sprint (`Closed/Sprint-SP-078.md`) and the closed Epic
(`../Epics/Closed/Epic-EP-024.md`):

- **T-0318** — `SceneInspector` widget + Scene Entities stub tab (`QTabWidget`; bold title + "No entities yet."
  + disabled Add Entity; **120/200 width**).
- **T-0319** — docked as `EditorShell`'s third `QSplitter` pane + `setInspectorVisible`/`isInspectorVisible`
  (session-scoped, default shown).
- **T-0320** — View ▸ Show Inspector menu toggle (Ctrl+Alt+I, checkable, editor-only, check-state synced) on
  the SP-077 menu bar.

Post-verify user tweaks: inspector widths 240/280 → 120/200, window default 820×560 → 1020×760; rebuilt green.
Qt/C++ UI only — no `scrivi_*`/`scrivi.h`/Apple change → no pbxproj. **All EP-024 ACs met; Epic closed in one
sprint (mirroring Apple EP-014/SP-037). Next available Task: T-0321.**

---

**SP-076 + SP-077 (EP-028 `[Linux]`) ✅ both closed 2026-07-22 (Human-approved) — this closed EP-028.**
All of T-0304–T-0317 are ✅ **Verified** and archived with their sprints:

- **SP-076** (`[Linux]` merge parity) — `Closed/Sprint-SP-076.md`. T-0304 (`ManuscriptEditor` merge signals),
  T-0305 (`ScriviBridge` merge invokables), T-0306 (`EditorShell` merge slots + `scene_merge_smoke`), T-0307
  (spec update §8.1), T-0308 (between-scene separator rule), T-0309 (`Ctrl+Return` mid-scene split). Scene-merge
  verified live over VNC; chapter-merge verified live via the SP-077 menu.
- **SP-077** (`[Linux]` native menu bar) — `Closed/Sprint-SP-077.md`. T-0310 (menu bar scaffold + File/Edit),
  T-0311 (Scene/Chapter menus + public `EditorShell` triggers — made chapter-merge testable over VNC), T-0312
  (Project ▸ Settings… stub), T-0313 (chapter-merge live-verified + TEMP diagnostics stripped — closed AC5/AC6
  Linux), T-0314 (File ▸ New opens the New Project panel), T-0315 (File ▸ Open shows the folder picker), T-0316
  (flush-safe editor-leaving paths — closed a data-loss hole; removed the raw ‹ Close button), T-0317 (Ctrl+W
  closes the project).

**EP-028 AC1–AC7 all Verified. Archived to `../Epics/Closed/Epic-EP-028.md`. No active Epic; next in line
EP-024–EP-026 `[Linux]` (Draft). Next available Task: T-0318.**

_The full T-0304–T-0317 detail lives in the two sprint archives above and the closed Epic; not re-listed here._

---

**SP-074 ✅ closed 2026-07-21** (EP-028 `[ScriviCore]` — **Merge endpoints + filesystem-coherence fix**).
Delivered `scrivi_merge_scene` (`SceneMerger`) + `scrivi_merge_chapter` (`ChapterMerger` — the atomic I-0083
fix: relocates scene files into the predecessor BEFORE removing the emptied chapter) + `MergeSceneTests.cpp`
(11 tests, incl. the I-0083 loss guard on the old `deleteChapter`-composed path). ctest **macOS 317/317 +
Linux 324/324**; both C symbols exported in `libScriviCore.a`; `scrivi.h` boundary stayed pure C ABI. Fixed a
stale `devops/docker/linux/Dockerfile` (missing `libssl-dev`). **I-0083 Resolved at core, Not Verified** — app
adoption is SP-075 (this sprint) / SP-076. T-0298–T-0301 record: `Sprints/Closed/Sprint-SP-074.md`.
**Next available T-0302.**

---

**SP-073 ✅ closed 2026-07-19** (EP-023 `[Linux]` — chapter drag-reorder AC5 + full EP-023 verify; **this
closed EP-023**). T-0294–T-0297 all ✅ Verified & archived to `Verified/Task-verified-0294-0297.md`; fixed
& Verified en route: I-0080 (`[ScriviCore]` open-time migration undid chapter reorders), I-0081 (stale
scene paths after drag), I-0082 (undraggable chapter rows). Sprint record:
`Sprints/Closed/Sprint-SP-073.md`. **Next available T-0298.**

---

**SP-072 ✅ closed 2026-07-18** (EP-027 `[Apple]` — **P5 Apple-verify migration fixes**). Fixed I-0078 (macOS
`createChapter` wrapper drift), I-0076 (legacy scene `content.path` bared on read + stale sidecars normalised on
migration), I-0077 (id-mismatched chapter path-folder fallback). 2 regression tests (RED-without-fix). ctest
**304/304 macOS + 311/311 Linux**; real legacy project opens clean. Full task table in `Closed/Sprint-SP-072.md`
(T-0290–T-0293). **Next available T-0294.**

---

**SP-070** (EP-027 `[ScriviCore]` — **P6 filesystem-authoritative scene identity & ordering**) is
🟢 Implemented, Not Verified (2026-07-17). Trade study **§8** (Human-approved 2026-07-17). `SceneRef` filename-only
(identity derived by sceneID sidecar scan); order-key scene filenames; cross-chapter reorder **relocates the files**;
chapter-folder rename touches **zero** scene fields; scene migration + orphan-repair on open (**before** validation)
dissolves the **C6** "Missing scene.meta.json". `scrivi.h` untouched; ctest **298/298 macOS** (Linux-container parity
pending). Full task table in `Sprints/Sprint-active.md` (T-0271–T-0277). **Next available T-0278** (P4 Linux verify,
now unblocked).

---

**SP-069** (EP-027 `[ScriviCore]` — **P1 rename primitive + P2 order-key/disk-authority + P3 migration**) is
🟢 Implemented, Not Verified (2026-07-16). `scrivi.h` untouched; ctest green **290/290 macOS + Linux**. **I-0072
root cause fixed + regression + open-time self-heal for existing damage + lazy migration of legacy `chapter-NNN`
projects.** Full task table in `Sprints/Sprint-active.md` (T-0264 P1; T-0265–T-0269 P2; T-0270 P3). **Deferred:**
drop `chapterID` from `ChapterRef` schema (self-healing cache — churn without gain; trade study §7.6).

**Epic:** EP-027 `[ScriviCore]` — **Sprint:** SP-069 (`Sprints/Sprint-active.md`). (P6/SP-070 consumed
T-0271–T-0277; next available **T-0278**.)

---

_Prior:_ **SP-067 ✅ closed 2026-07-16** — delivered **AC4 scene drag-reorder** (T-0260, verified;
I-0067/I-0068 fixed via `Qt::CopyAction`) + **I-0063 renumber** (T-0262, verified). Its chapter-split task
**T-0261 (I-0064)** was **superseded → EP-027** (the split path is rebuilt on EP-027's new on-disk model), along
with I-0069/I-0070/I-0072/I-0071. SP-067's verified tasks (T-0258/259/260/262/263) archive with the sprint
(`Sprints/Closed/Sprint-SP-067.md`). **No ScriviCore work in SP-067**; `scrivi.h` untouched.

**Next:** two Epics Active — **EP-027** `[ScriviCore]` (recommended next: P1 FileSystem rename/move primitive)
and **EP-023** `[Linux]` (remaining SP-068, sequenced against EP-027). Tasks assigned at the next sprint's
activation; next available **T-0264**.

The SP-067 task table below is retained for reference until archived with the sprint.

| ID | Title | Status |
| -- | ----- | ------ |
| T-0258 | **Reorder bridge wrappers** — `ScriviBridge::reorderScene(...)` / `reorderChapter(...)` (`scrivi_reorder_scene`/`_chapter`, `parseEnvelope`, RAII `scrivi_free`, `errorOccurred`, `ready_` guard; no identity). | ✅ Done (not verified) |
| T-0259 | **`SceneDocument::moveScene` re-splice** — lift a scene body span (+ one boundary) and re-insert at the destination with the correct heading/separator; re-promote/demote chapter-first scenes; fix all `bodyStart`s; `reflowAllChapterHeadings()`. Programmatic-window guarded. | ✅ Done (not verified) |
| T-0260 | **`NavigatorTree` scene drag-drop + insertion-line highlight** — new `QTreeView` subclass: drag scene rows; drop indicator; `dragMoveEvent` legality gate; on drop resolve `(draggedSceneID, targetChapterID, afterSceneID)` → `sceneDropRequested`; `EditorShell::onSceneDropped` does `saveDirtyScenes()` + `bridge_->reorderScene` + `moveScene` + `rebuildNavigator()` + re-anchor caret. Chapter-row drag disabled (SP-068). | ✅ Done (not verified) |
| T-0261 | **I-0064 — Ctrl+Shift+Return splits the chapter at the caret** — orchestrate `create_chapter` → `reorder_chapter(K, afterC)` → per-following-scene `reorder_scene` into K → (mid-scene) `save_scene` head/tail (end-of-scene w/ followers drops blank K0; no followers keeps it) → reload → `renumberCreatedChapters` → renumber confirmation. | 🔵 **Superseded → EP-027** (2026-07-16) — VNC: mid-scene worked, end-of-scene failed (I-0069/I-0070); the split path is rebuilt on EP-027's new on-disk model. Not verified on the old model. |
| T-0262 | **I-0063 — renumber created chapters on structural change** — `renumberCreatedChapters()`: `renameChapter` each later chapter whose stored title matches `^Chapter \d+$` to its new ordinal (Option A); custom untouched; untitled already handled. Wired into `deleteChapterByID` + T-0261; also fixes `load()` to copy `chapterMetadataPath`. **Closes I-0063.** | ✅ Done (not verified) |
| T-0263 | **Verify AC4 + I-0063/I-0064 + `scene_reorder_smoke`** — new headless smoke (within/cross-chapter move, chapter-first re-promotion, offset map well-formed, cross-chapter persistence, I-0064 split end/mid-scene, I-0063 renumber) + CMake target + CI step. **Docker build + `scene_reorder_smoke` + all prior smokes green (2026-07-15).** VNC walk-through user-pending; `scrivi.h` untouched; macOS/EP-020–022 + SP-065/066 unaffected. | ✅ Done (smokes green; VNC user-pending) |

**Epic:** EP-023 `[Linux]` (Manuscript Structure Editing) — Active. **Sprint:** SP-067 (`Sprints/Sprint-active.md`).
Previous sprint SP-066 (rename) ✅ closed; T-0254–T-0257 Verified & archived to
`Verified/Task-verified-0254-0257.md`.

---

*Last Updated: 2026-07-22 (**SP-081 T-0329/0330/0331 ✅ Verified; T-0332 blocked by I-0087 → SP-083 zoom/pan
brought forward & implemented** (two parallel EP-025 sprints). SP-081's bridge/presets + band overlay + View ▸
Story Structure… selector + band border-drag are Verified live; scene→band drag-up assignment (T-0332) is
blocked because a far-outlier flashback scene crowds all dots to one edge under the linear axis (I-0087). Per
user decision (keep linear axis, Apple parity) **SP-083** now delivers zoom (`Ctrl`+wheel zoom-about-pointer +
an always-works `+`/`−` control, bottom-right) + pan (background drag) — T-0333–T-0335 🟢 Implemented, build +
all smokes green; live VNC pending (then re-verify T-0332 zoomed in). Clustering + Epic close → new SP-084.
Next available **T-0336**. Prior notes below retained for reference.)*
