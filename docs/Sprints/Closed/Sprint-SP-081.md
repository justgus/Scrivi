# SP-081: [Linux] Story-structure bands — overlay, border drag, band assignment

**Status:** ✅ Closed (2026-07-23)
**Epic:** EP-025 `[Linux]` Timeline Panel (3rd of ~5 sprints; mirrors Apple EP-016 SP-041 T-0156–T-0160)
**Goal:** Add **story-structure bands** to the Linux timeline. A writer picks a built-in structure (Three Act,
Five Act, Hero's Journey, …) from a menu; the timeline paints **colored, proportional bands** behind the scene
dots with labels. Band **borders drag** to re-proportion (persisted). A scene **assigns to a band** — by
dragging its dot up onto the band's label row and via a dot context-menu **"Assign to Act…"** — and shows a
colored ring; removing the structure keeps every scene's offsets + band assignments. Delivers **EP-025 AC4**.
**Start Date:** 2026-07-22
**End Date:** 2026-07-23
**Capacity:** ~7–9 hours

### Assigned Tasks

| ID     | Title | Priority | Status |
| ------ | ----- | -------- | ------ |
| T-0329 | `[Linux]` **Story-structure bridge invokables** — `getStoryStructure`/`setStoryStructure`/`updateBandLayout`/`removeStoryStructure` + `assignSceneToBand`/`unassignSceneFromBand` (all exported since EP-016; `scrivi.h` untouched). Plus the **built-in band table** ported from Apple `BuiltInStructure.bandTable` into `StoryStructures.cpp/.hpp` + `bandLayoutJSON`/`parseBandLayout` helpers. | High | ✅ Verified (2026-07-22) |
| T-0330 | `[Linux]` **Band overlay painting + Structure selector** — `TimelinePanel::setBands` + `paintEvent` paints translucent colored proportional bands + labels behind the dots + a label row; **View ▸ Story Structure…** pops built-ins + "None (Remove)"; loaded via `getStoryStructure` in `reloadTimeline`. | High | ✅ Verified (2026-07-22) |
| T-0331 | `[Linux]` **Band border drag (re-proportion + persist)** — border zone hit-tested first (`borderIndexNearX`); drags proportion between adjacent bands (0.05 floor, pair-sum constant); release → `bandProportionsChanged` → `updateBandLayout`. | High | ✅ Verified (2026-07-22) |
| T-0332 | `[Linux]` **Scene→band assignment (drag-up + context menu) + verify** — a dot dragged up → `sceneAssignedToBand` (`bandIndexAtX` at release) + a colored ring; context-menu "Assign to Act…" + "Unassign"; assignment survives structure-remove (AC4). New `story_structure_smoke`. **Two blockers found + fixed:** I-0087 (dot-crowding — fixed by SP-083 zoom/pan) and I-0089 (drag-up mode latched to horizontal on the first 4px move — fixed by classifying the drag by dominant direction + an assignment cue). | High | ✅ **Verified (2026-07-23, VNC)** |

### Assigned Issues

| ID     | Title | Severity | Status |
| ------ | ----- | -------- | ------ |
| I-0089 | `[Linux]` Scene→band drag-up assignment (T-0332) never triggers — drag mode latched to horizontal on the first 4px move (dots at strip centre, label row at top). Fixed: classify by dominant direction, re-evaluated while ambiguous, + an assignment cue (target-band outline + leader line). | High | ✅ **Verified (2026-07-23, VNC)** |

### Key decisions & findings

- **The C ABI was already complete** (EP-016/SP-039): `scrivi_get/set/update/remove_story_structure` +
  `scrivi_assign/unassign_scene_to_band`. `scrivi.h` **untouched** this sprint — T-0329 added Qt bridge wrappers
  + ported Apple's `bandTable` (Three Act / Five Act / Hero's Journey / Save the Cat / Freytag / Kishōtenketsu /
  In Medias Res) into `StoryStructures.{hpp,cpp}`.
- **JSON-shape finding (caught by the smoke).** ScriviCore expects `bandLayoutJSON` to be an object wrapping a
  `"bands"` array — `{"bands":[{bandID,label,color,proportion},…]}` — NOT a bare array. The first
  `story_structure_smoke` run FAILED (nothing persisted) with a bare array; fixed to the `{"bands":[…]}` wrapper.
  Exactly why the sprint added a headless smoke rather than trusting the format.
- **Drag disambiguation** (three drags share the strip): border zone → border drag; dot moved **up** → band
  assignment; dot moved **sideways** → story-time (SP-080); empty background → pan (SP-083). All plain mouse
  events (VNC-deliverable).
- **App owns the presets, backend stores the layout.** `set_story_structure` persists whatever `bandLayoutJSON`
  it's handed; border-drag edits go through `update_band_layout`. Custom proportions = an edited layout, no new
  schema. `remove_story_structure` keeps each scene's `offsetMs` + `bandID` (AC4).

### Verification

- ✅ **Container build green** — Qt 6.4: **202/202** targets, 0 warnings; `scrivi_linux` +
  `scrivi_linux_story_structure_smoke` linked.
- ✅ **`story_structure_smoke` PASS** — set structure (persists), assign a scene (bandID persists), re-proportion
  (persists), remove structure → **bandID assignment PRESERVED (AC4 contract)**.
- ✅ **Regression smokes green** — timeline_story_time, scene_merge/create/reorder, chapter_reorder, scene_load,
  editor_map.
- ✅ **Live VNC — T-0329/0330/0331 (2026-07-22, user)** — View ▸ Story Structure… applies a structure → colored
  proportional bands + labels behind the dots; border drag re-proportions + persists.
- ✅ **Live VNC — T-0332 (2026-07-23, user)** — after the I-0087 (SP-083 zoom) and **I-0089** fixes, drag-up
  assignment works (target-band cue + colored ring, survives quit→reopen); context-menu Assign/Unassign too.

### Retrospective

**What shipped.** Full story-structure band support on the Linux timeline: a built-in structure selector,
colored proportional bands painted behind the dots, draggable borders that re-proportion + persist, and
scene→band assignment by drag-up **and** context menu, with a colored ring; removing a structure preserves
offsets + assignments. **Delivers EP-025 AC4.**

**The T-0332 saga (two blockers, one real feature + one latent bug).** T-0332's drag-up assignment was blocked
twice. First by **I-0087** — a far-outlier flashback scene crowded every other dot to one strip edge under the
linear time-axis, so a dot couldn't be grabbed at all. The user kept the linear axis (Apple parity) and brought
**SP-083 (zoom/pan) forward** to fix it; that succeeded. But T-0332 *still* failed once the dots were spread —
revealing **I-0089**, a latent mode-latch bug: the dot-drag mode was chosen once on the first 4px move and
required the pointer to already be in the top label row, which is impossible from a dot at the strip's centre,
so it always latched to a horizontal story-time drag. Fixed by classifying the drag by **dominant direction,
re-evaluated while ambiguous**, plus a visible assignment cue. Only then did T-0332 verify.

**Lesson.** "Blocked by crowding" masked a second, independent defect. Bringing SP-083 forward was the right
call for I-0087, but the assignment gesture had never actually worked — the crowding just prevented us from
reaching the code path that would have exposed it. Worth remembering when one blocker is assumed to be the whole
story.

**Process.** Confirmed the Docker/Qt local build+smoke loop end-to-end this sprint (memory
`project_linux_docker_build`) — every fix was compiled + smoke-tested locally before VNC hand-off.

**Follow-ons captured elsewhere.** SP-083 also grew T-0336–0339 (scene-anchor picker, bands-wrap-storyline,
zoom/pan persistence, title truncation) + I-0088 (flashback negative-offset) — all closed under SP-083.

**Out of scope (unchanged):** historical/imported events → SP-082; clustering/aggregate dots + EP-025 Epic
close → SP-084.
