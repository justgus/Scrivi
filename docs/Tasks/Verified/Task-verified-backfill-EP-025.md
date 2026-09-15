# Verified Tasks — BACKFILL — EP-025 [Linux] Timeline Panel

**Created:** 2026-09-15 · **Audit ruling [R-11]** · ✅ **Epic attributions VERIFIED 2026-09-15**
**Tasks:** 29 (T-0321 … T-0349) · **All ✅ Verified**

⚠️ **WHY THIS FILE EXISTS.** ✅ **These Tasks were complete and Verified, but their ONLY record was
the closed Sprint archive.** ⚠️ **A prior audit (2026-08-19) recorded the gap and scoped it OUT;
✅ the user ruled 2026-09-15: *"This audit will not defer it. Backfill them."***

✅ **EVERY EPIC ATTRIBUTION IN THIS FILE IS VERIFIED AGAINST ITS SPRINT ARCHIVE.**
⚠️ **The first pass INFERRED 24 attributions from the first `EP-0xx` mention in the text;**
✅ **all 24 were then checked by opening the archives** — ⚠️ **16 were right, ⛔ 8 were WRONG.**
⚠️ **The method that failed: my checker looked for a `**Epic:**` markdown line and missed that five
archives declare the Epic in YAML FRONTMATTER (`epic: EP-0xx`).**

⚠️ **THE SPRINT ARCHIVE REMAINS THE RECORD OF TRUTH.**

| Task | Title | Sprint | Epic evidence | Record of truth |
| ---- | ----- | ------ | ------------- | --------------- |
| T-0321 | [Linux] ScriviBridge::getTimeline (+ getSceneStoryTime) invokables — getTimeline → scrivi_get_timeline (epoch label + meta); getSceneSt… | SP-079 | ✅ declared in archive | [`SP-079`](../../Sprints/Closed/Sprint-SP-079.md) |
| T-0322 | [Linux] TimelinePanel widget — QWidget (TimelinePanel.cpp/.hpp): horizontal strip painting a baseline + one dot per scene by story-tim… | SP-079 | ✅ declared in archive | [`SP-079`](../../Sprints/Closed/Sprint-SP-079.md) |
| T-0323 | [Linux] Dock as the editor's bottom strip + View ▸ Show Timeline toggle — wrapped the 3-pane horizontal splitter_ and timeline_ in an outer… | SP-079 | ✅ declared in archive | [`SP-079`](../../Sprints/Closed/Sprint-SP-079.md) |
| T-0324 | [Linux] Dot↔navigator bidirectional selection + tooltip + verify — dot click → sceneClicked(sceneID) → moveCaretToSegment + focus; active-sc… | SP-079 | ✅ declared in archive | [`SP-079`](../../Sprints/Closed/Sprint-SP-079.md) |
| T-0325 | [Linux] ScriviBridge::setSceneStoryTime invokable — → scrivi_set_scene_story_time(root, sceneID, offsetMs, source, gapMs, durationMs, duratio… | SP-080 | ✅ declared in archive | [`SP-080`](../../Sprints/Closed/Sprint-SP-080.md) |
| T-0326 | [Linux] TimelinePanel dot drag — press-on-dot → horizontal drag (4px threshold) live-moves the dot (dragX_) → dotDragged(sceneID, offsetFor… | SP-080 | ✅ declared in archive | [`SP-080`](../../Sprints/Closed/Sprint-SP-080.md) |
| T-0327 | [Linux] TimeDeltaPicker dialog (TimeDeltaPicker.cpp/.hpp) — amount spinbox + unit combo (Minutes…Years) + direction (Later/Before) seeded vi… | SP-080 | ✅ declared in archive | [`SP-080`](../../Sprints/Closed/Sprint-SP-080.md) |
| T-0328 | [Linux] Wire drag/menu → picker → commit + chain propagation — EditorShell::showTimeDeltaPicker seeds from the reloadTimeline offset/duratio… | SP-080 | ✅ declared in archive | [`SP-080`](../../Sprints/Closed/Sprint-SP-080.md) |
| T-0329 | [Linux] Story-structure bridge invokables — getStoryStructure/setStoryStructure/updateBandLayout/removeStoryStructure + assignSceneToBa… | SP-081 | ✅ declared in archive | [`SP-081`](../../Sprints/Closed/Sprint-SP-081.md) |
| T-0330 | [Linux] Band overlay painting + Structure selector — TimelinePanel::setBands + paintEvent paints translucent colored proportional bands + la… | SP-081 | ✅ declared in archive | [`SP-081`](../../Sprints/Closed/Sprint-SP-081.md) |
| T-0331 | [Linux] Band border drag (re-proportion + persist) — border zone hit-tested first (borderIndexNearX); drags proportion between adjacent bands… | SP-081 | ✅ declared in archive | [`SP-081`](../../Sprints/Closed/Sprint-SP-081.md) |
| T-0332 | [Linux] Scene→band assignment (drag-up + context menu) + verify — a dot dragged up → sceneAssignedToBand (bandIndexAtX at release) + a color… | SP-081 | ✅ declared in archive | [`SP-081`](../../Sprints/Closed/Sprint-SP-081.md) |
| T-0340 | [Linux] Timeline-events bridge invokables — ScriviBridge Qt wrappers over the complete EP-016 C ABI (historical CRUD + import/update-offset/vi… | SP-082 | ✅ declared in archive | [`SP-082`](../../Sprints/Closed/Sprint-SP-082.md) |
| T-0341 | [Linux] Historical-event dots + author/edit/delete — #C8A97A dots on the project row (distinct from scene accent + imported grey, §7.2), dragg… | SP-082 | ✅ declared in archive | [`SP-082`](../../Sprints/Closed/Sprint-SP-082.md) |
| T-0342 | [Linux] Imported-timeline rows + epoch-offset dialog + hide/show — one grey row below the project row per import (source label, per-source assi… | SP-082 | ✅ declared in archive | [`SP-082`](../../Sprints/Closed/Sprint-SP-082.md) |
| T-0343 | [Linux] Export timeline — Export Timeline… (§7.9) → exportProjectTimeline → QFileDialog save-as writes .scrivi-timeline.json (scene + hist… | SP-082 | ✅ declared in archive | [`SP-082`](../../Sprints/Closed/Sprint-SP-082.md) |
| T-0344 | [Linux] Wire-up + timeline_events_smoke + verify — reloadTimeline/reloadImportedTimelines load historical events + imported rows; new head… | SP-082 | ✅ declared in archive | [`SP-082`](../../Sprints/Closed/Sprint-SP-082.md) |
| T-0345 | [Linux] File ▸ Import / Export Timeline… menu items (user request 2026-07-24) — import/export are file ops, so they join File (below Close Proje… | SP-082 | ✅ declared in archive | [`SP-082`](../../Sprints/Closed/Sprint-SP-082.md) |
| T-0333 | [Linux] TimelinePanel zoom model — zoom_ (≥1) + panFraction_ threaded through xForOffset/offsetForX (band geometry follows for free);… | SP-083 | ✅ declared in archive | [`SP-083`](../../Sprints/Closed/Sprint-SP-083.md) |
| T-0334 | [Linux] +/− zoom control + horizontal scrollbar — two QToolButtons at the strip's bottom-right (+ left, − right) → zoomInStep/z… | SP-083 | ✅ declared in archive | [`SP-083`](../../Sprints/Closed/Sprint-SP-083.md) |
| T-0335 | [Linux] Pan by background drag — a press on the empty area (no dot/border) with zoom_ > 1 → DragMode::Pan (closed-hand cursor), drag adjusts… | SP-083 | ✅ declared in archive | [`SP-083`](../../Sprints/Closed/Sprint-SP-083.md) |
| T-0336 | [Linux] Time Delta Picker — anchor to ANY scene (resolved once to a manual offset; no schema change) | SP-083 | ✅ declared in archive | [`SP-083`](../../Sprints/Closed/Sprint-SP-083.md) |
| T-0337 | [Linux] Story bands wrap the main storyline [0, last-end], not the whole strip; zoom with the timeline; flashback stays assignable | SP-083 | ✅ declared in archive | [`SP-083`](../../Sprints/Closed/Sprint-SP-083.md) |
| T-0338 | [Linux] Persist timeline zoom + pan per project (INI under app-support so it survives the --rm container) | SP-083 | ✅ declared in archive | [`SP-083`](../../Sprints/Closed/Sprint-SP-083.md) |
| T-0339 | [Linux] Truncate long scene titles in the picker's anchor combo (cap width + elide, full title in tooltip) | SP-083 | ✅ declared in archive | [`SP-083`](../../Sprints/Closed/Sprint-SP-083.md) |
| T-0346 | [Linux] Clustering pass + aggregate-dot model — computeClusters() groups project-row members (scene + historical dots) whose current-zoom pixe… | SP-084 | ✅ declared in archive | [`SP-084`](../../Sprints/Closed/Sprint-SP-084.md) |
| T-0347 | [Linux] Aggregate-dot paint — paintAggregate draws a slightly-larger core (FR-031) + centred member count + a segmented arc ring (360… | SP-084 | ✅ declared in archive | [`SP-084`](../../Sprints/Closed/Sprint-SP-084.md) |
| T-0348 | [Linux] Hover fan-out + zoom-resolve + member interaction — updateHoverFan fans an aggregate's members into the hexagonal ring (fanOutMemberP… | SP-084 | ✅ declared in archive | [`SP-084`](../../Sprints/Closed/Sprint-SP-084.md) |
| T-0349 | [Linux] Persistence sweep + full EP-025 verify + timeline_cluster_smoke + Epic close — new headless timeline_cluster_smoke (N co-located → o… | SP-084 | ✅ declared in archive | [`SP-084`](../../Sprints/Closed/Sprint-SP-084.md) |
