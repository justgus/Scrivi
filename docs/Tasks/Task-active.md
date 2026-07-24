# Active Tasks

**Epic:** EP-025 `[Linux]` (Timeline Panel) — **active sprint:** SP-082 (historical events + imported
timelines + export, **AC5**) (`Sprints/Sprint-active.md`).

**SP-082** `[Linux]` — historical events + imported timelines + export (AC5); C ABI complete (EP-016,
`scrivi.h` 270–291, untouched); clustering + Epic close carved out to SP-084:

| ID | Title | Status |
| -- | ----- | ------ |
| T-0340 | **`[Linux]` Timeline-events bridge invokables** — `ScriviBridge` wrappers over the EP-016 C ABI: `createHistoricalEvent`/`updateHistoricalEvent`/`deleteHistoricalEvent`/`listHistoricalEvents` + `importExternalTimeline`/`updateImportedTimelineOffset`/`setImportedTimelineVisible`/`listImportedTimelines`/`removeImportedTimeline`/`exportProjectTimeline`. Each `parseEnvelope`s, RAII `scrivi_free`, `errorOccurred` on failure, `ready_` guard, no identity (empty identity args). `scrivi.h` untouched. | ✅ Verified (2026-07-24, VNC) |
| T-0341 | **`[Linux]` Historical-event dots + author/edit/delete** — `TimelinePanel` renders historical events as `#C8A97A` filled dots (distinct from scene accent + imported grey, §7.2), draggable in story time (`HistHorizontal` drag → `historicalEventDragged(eventID, newOffsetMs)`) + hover tooltip. Empty-area menu **"New Historical Event Here"** (§7.9); historical-dot menu **Edit / Delete** (§7.7) → a `HistoricalEventDialog` (title, description, tags). Fed via `reloadTimeline` (`listHistoricalEvents`); tags read from disk on Edit (list drops them). | ✅ Verified (2026-07-24, VNC) |
| T-0342 | **`[Linux]` Imported-timeline rows + epoch-offset dialog + hide/show** — one **grey row below** the project row per import (source-name label, per-source `assignedGreyShade`, dots **window-clipped** §6.7, read-only, tooltip = title + source + computed time). **Import Timeline…** (§7.9) → `QFileDialog` → **`EpochOffsetDialog`** (source name + epoch label + signed offset + in/out-of-window preview, FR-067) → `importExternalTimeline`. Row menu (§7.8): **Edit Epoch Offset… / Hide This Timeline / Remove Imported Timeline**; **Show Hidden Timelines** submenu on the empty-area menu (un-hide path, FR-065). Events read from the stored files (list is metadata-only). Min-height grows one row per visible import (FR-064). | ✅ Verified (2026-07-24, VNC) |
| T-0343 | **`[Linux]` Export timeline** — **Export Timeline…** (§7.9) → `exportProjectTimeline` (scene + historical events, no prose/identity, §6.6/FR-069) → `QFileDialog` save-as writes `.scrivi-timeline.json`. An exported file re-imports (T-0342) into another project (smoke round-trips it). | ✅ Verified (2026-07-24, VNC) |
| T-0344 | **`[Linux]` Wire-up + `timeline_events_smoke` + verify** — `reloadTimeline` also loads historical events (`reloadImportedTimelines`) + imported rows; new headless **`timeline_events_smoke`** (create/update/delete a historical event round-trips; import a fixture → stored + metadata + on-disk file; hide/show + update-offset persist; export → valid `scrivi.externalTimeline.v1` carrying the surviving historical event; remove clears) + CMake + CI. **Container green + smoke PASS; live VNC walkthrough pending.** Closes AC5. | ✅ Verified (2026-07-24, VNC) |
| T-0345 | **`[Linux]` File ▸ Import / Export Timeline… menu items** (user request 2026-07-24) — import/export are **file** ops, so they join **File** (below Close Project, own separator group) on the SP-077 menu bar, not only the timeline right-click. Editor-only (`editorOnlyActions_`); each forwards to new public `EditorShell::importTimeline()`/`exportTimeline()` → the T-0342/T-0343 flow. Panel empty-area entries kept (both homes). Container green (211/211) + app-launch OK. | ✅ Verified (2026-07-24, VNC) |

**Verification (2026-07-24):** ✅ **build green** (Qt 6.4: **211/211** targets, `scrivi_linux` +
`scrivi_linux_timeline_events_smoke` linked); ✅ **new `timeline_events_smoke` PASS** (historical CRUD list
round-trip; import fixture → stored + metadata + on-disk file; hide/show + offset persist; export a valid
`scrivi.externalTimeline.v1` carrying the surviving historical event; remove → list empty); ✅ **all 11
regression smokes PASS**; ✅ **headless app-launch OK**. ✅ **Live VNC walkthrough COMPLETE** — Human verified
all six tasks on `the-twisted-remains-of-myself`: historical events author/drag/edit/delete (`#C8A97A` dots);
imported grey rows (two sources, per-source shade); export + import round-trip; File ▸ Import/Export Timeline…
menu items. **Two findings surfaced + fixed + re-verified same day: I-0090** (imported row hidden behind the
zoom scrollbar) + **I-0091** (file dialogs defaulted to `/root`). **AC5 met; SP-082 awaiting Human close
approval. Next available Task after SP-082: T-0346.** `scrivi.h` untouched (C ABI complete from EP-016); no
pbxproj (Linux-only).

**Design/impl notes (2026-07-24):**
- **The C ABI was already complete** (EP-016/SP-039): all 10 endpoints at `scrivi.h` 270–291. T-0340 added only
  Qt bridge wrappers; `scrivi.h` untouched.
- **Envelope-shape findings** (confirmed against `scrivi_c_api.cpp` at planning, guarded by the smoke): the two
  list endpoints return **metadata only** as a nested JSON string — `listHistoricalEvents` → `{count,
  eventsJSON}` where `eventsJSON` = `{"events":[…]}` **without tags/slug**; `listImportedTimelines` → `{count,
  timelinesJSON}` = `{"timelines":[…metadata + eventCount…]}` **without the per-event array**. So per-event data
  is read **directly from the stored files** in `objects/historical-events/` and `objects/imported-timelines/`
  — **Apple's own pattern** (its `loadImportedTimelines` comment: "the stored files include events;
  listImportedTimelines returns metadata only"). **No C ABI gap.**
- **Tags** are accepted/stored by create/update but dropped by the list projection → the Edit dialog reads tags
  from the on-disk event file to prefill accurately; `tagsToJson` sends the `{"tags":[…]}` wrapper.
- **New files** (Linux CMake, not pbxproj): `HistoricalEventDialog.{cpp,hpp}`, `EpochOffsetDialog.{cpp,hpp}`,
  `tests/timeline_events_smoke.{cpp,sh}`. New CI step wired.

---

**SP-081 + SP-083 (EP-025 `[Linux]`) ✅ both closed 2026-07-23 (Human-approved).** Archived to
`Closed/Sprint-SP-081.md` and `Closed/Sprint-SP-083.md`; between them they delivered **AC4** (story-structure
bands + border drag + scene→band assignment) and **AC6a** (timeline zoom + pan) and closed I-0087/I-0088/I-0089.

- **SP-081** (`[Linux]` story-structure bands) — T-0329 (bridge invokables + built-in band table, `{"bands":[…]}`
  wrapper), T-0330 (band overlay + View ▸ Story Structure… selector), T-0331 (band border-drag re-proportion),
  T-0332 (scene→band drag-up + "Assign to Act…" assignment + `story_structure_smoke`; unblocked once SP-083
  zoom spread the crowded dots; I-0089 mode-latch fixed). All ✅ Verified.
- **SP-083** (`[Linux]` timeline zoom + pan, brought forward to fix I-0087) — T-0333 (zoom model, Ctrl+wheel
  zoom-about-pointer, linear axis kept), T-0334 (`+`/`−` control + scrollbar), T-0335 (background-drag pan +
  re-verify T-0332). Follow-ons T-0336 (Time Delta Picker anchors to any scene's end), T-0337 (bands wrap the
  storyline `[0, last-scene-end]`, zoom with the timeline), T-0338 (persist zoom/pan per project under the
  app-support root — the `--rm` `~/.config` wipe fix), T-0339 (elide long titles in the picker anchor combo).
  All ✅ Verified live over VNC.

_The full T-0329–T-0339 detail lives in the two sprint archives; not re-listed here._

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

*Last Updated: 2026-07-24 (**SP-082 planned + activated** — EP-025 `[Linux]` Timeline Panel, 4th sprint,
delivering **AC5**: historical events (author/edit/delete + `#C8A97A` draggable dots + context menus), imported
timelines (grey rows below, per-source shade, window-clip, epoch-offset dialog, hide/show, edit-offset, remove),
and export (`.scrivi-timeline.json` → re-import round-trip). Tasks **T-0340–T-0344**. The full timeline-events C
ABI is exported (EP-016, `scrivi.h` 270–291) → `scrivi.h` untouched; T-0340 adds Qt bridge wrappers only; new
`timeline_events_smoke`; no pbxproj (Linux-only). Clustering + panel-persistence sweep + Epic close carved out
to **SP-084** (user decision). **SP-081 + SP-083 ✅ both closed 2026-07-23** (AC4 + AC6a; archived). Next
available Task after SP-082: **T-0345**. Prior notes below retained for reference.)*
