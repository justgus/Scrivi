# SP-066: [Linux] Rename scene/chapter — context menu, inline edit, live heading + I-0062

**Status:** ✅ Closed (user-approved 2026-07-15)
**Epic:** EP-023 `[Linux]` — Manuscript Structure Editing (second of 4 sprints)
**Codebase:** `[Linux]` (`platforms/linux/`) only — **no ScriviCore source change.** `scrivi.h` unchanged
(AC8). Endpoint + payload signatures confirmed against source at activation (2026-07-15):

- `scrivi_rename_scene(projectRootPath, metadataPath, newTitle)` (`scrivi.h:211`) → `{metadataPath,
  newTitle, renamed}`. Writes the sidecar `title` field.
- `scrivi_rename_chapter(projectRootPath, metadataPath, newTitle)` (`scrivi.h:216`) → same shape. Takes the
  **chapter's** metadata path.
- `scrivi_open_project` scene entries already carry **`chapterMetadataPath`** (`scrivi_c_api.cpp:424`) — so
  the Linux `SceneDocument` captures it and renames a chapter without any new endpoint.

**Activated:** 2026-07-15
**Closed:** 2026-07-15 (user-approved)

**Goal:** Add **rename** to the Linux scene navigator — the second EP-023 structure op. Right-clicking a
scene or chapter row offers **Rename**, opening a focused edit pre-filled with the current title; saving
writes the sidecar `title` via `scrivi_rename_scene`/`scrivi_rename_chapter`, updates the navigator label
**and** (for chapters) the live in-document heading in place. A blank/whitespace-only title clears the custom
title, with the navigator/heading falling back (scene → first prose line/"Untitled"; chapter → **derived
ordinal "Chapter N"**). Also **closed I-0062** — a newly-created chapter's heading now shows its ordinal
immediately. Delivered **EP-023 AC3**.

## Delivered Tasks

| ID | Title | Status |
| -- | ----- | ------ |
| T-0254 | **Rename bridge wrappers + `chapterMetadataPath` capture** — `ScriviBridge::renameScene`/`renameChapter` (`scrivi_rename_scene`/`_chapter`, `parseEnvelope`, RAII `scrivi_free`, `errorOccurred` on error); added `chapterMetadataPath` to `SceneDocument::Input`/`SceneSegment` (from the `open_project` payload). No identity args. | ✅ Verified |
| T-0255 | **Context-menu Rename + inline edit + live label/heading** — `EditorShell::onNavigatorContextMenu` gained **Rename Scene…/Rename Chapter…** → `QInputDialog` pre-filled with the current title; on commit the bridge writes the sidecar title, `applyDerivedLabels` re-derives labels, and the navigator + live in-document chapter heading update in place via `reflowBoundaryAt`. Blank/whitespace → cleared sidecar + fallback (scene: first prose line/"Untitled"; chapter: derived ordinal). | ✅ Verified |
| T-0256 | **Close I-0062 — live new-chapter heading** — `SceneDocument::chapterHeadingText` derives the chapter heading ordinal "Chapter N" from segment order (custom title wins), matching macOS `ManuscriptTextView`; `insertSceneAfter` reflows a new untitled chapter's heading to its derived ordinal on splice, so it shows "Chapter N" immediately. (Superseded the planned Option-B `openProject` fetch — order-based derivation is the macOS mechanism, and it's what the app layer owns.) **I-0062 ✅ Resolved-Verified.** | ✅ Verified |
| T-0257 | **Verify AC3 + rename headless smoke** — new `scene_rename_smoke` (`.cpp`+`.sh`, CMake target `scrivi_linux_scene_rename_smoke`, CI step): Cases A–C (scene rename no-doc-change, chapter rename live-heading reflow + offset delta, blank→derived ordinal); `scene_create_smoke` gained the I-0062 "Chapter 2" derived-heading assertion. Docker build + **all 9 headless smokes green**; VNC walk-through passed. `scrivi.h` untouched. | ✅ Verified |

## Exit criteria — met

Right-click on a scene/chapter navigator row → **Rename** opens a `QInputDialog` pre-filled with the current
title; committing writes the sidecar `title` and updates the navigator label live; a chapter rename also
rewrites the live in-document heading (no reload). Blank/whitespace clears the custom title and the
navigator/heading fall back (scene → first prose line/"Untitled"; chapter → derived ordinal "Chapter N").
**I-0062 resolved** — a newly-created chapter shows its ordinal immediately. New `scene_rename_smoke` green
alongside the existing eight; `scrivi.h` unchanged; macOS/EP-020–022 + SP-065 flows unaffected. **AC3 +
I-0062 user-verified over Docker+VNC 2026-07-15.**

## Files touched

- `platforms/linux/src/ScriviBridge.{hpp,cpp}` — `renameScene`/`renameChapter` wrappers (T-0254).
- `platforms/linux/src/SceneDocument.{hpp,cpp}` — `chapterMetadataPath` field; `setSceneTitle`,
  `setChapterTitle`, `firstSegmentOfChapter`, `chapterHeadingText` (ordinal derivation),
  `reflowAllChapterHeadings` (added for the I-0063/I-0064 follow-up); `build`/`insertSceneAfter`/
  `reflowBoundaryAt` route the heading through the derived ordinal (T-0254/T-0255/T-0256).
- `platforms/linux/src/EditorShell.{hpp,cpp}` — `onNavigatorContextMenu` Rename actions, `renameSceneByID`,
  `renameChapterByID`, `applyDerivedLabels`, `refocusEditor`; `rebuildNavigator` uses `chapterHeadingText`;
  `onCreateChapterRequested` relies on the derived ordinal (I-0062) (T-0255/T-0256).
- `platforms/linux/tests/scene_rename_smoke.{cpp,sh}` — new headless smoke (T-0257).
- `platforms/linux/tests/scene_create_smoke.cpp` — I-0062 "Chapter 2" derived-heading assertion;
  `insertSceneAfter` call sites + `assemble` updated for the new `chapterMetadataPath` field.
- `platforms/linux/tests/scene_delete_smoke.cpp` / `editor_map_smoke.cpp` — updated for the new `Input` field
  (fixed a positional-aggregate-init regression in `editor_map_smoke`).
- `platforms/linux/CMakeLists.txt` — `scrivi_linux_scene_rename_smoke` target (T-0257).
- `.github/workflows/scrivi-linux-ci.yml` — Scene-rename smoke CI step (T-0257).

## Verification

- **AC3 + I-0062 user-verified over VNC 2026-07-15.** Right-click → Rename → `QInputDialog` → commit updates
  the navigator label live; chapter rename also updates the live in-document heading; blank chapter → derived
  ordinal "Chapter N"; blank scene → first-line/"Untitled"; cancel is a no-op; renames persist across
  quit→reopen; a newly-created chapter shows its ordinal immediately (I-0062).
- CI: ScriviCore `ctest` unaffected; Linux build + **nine** headless smokes green (adds `scene_rename`;
  `scene_create` now also asserts the I-0062 derived heading); macOS/EP-020–022 untouched (`scrivi.h`
  unchanged).

## Design note — chapter numbering is the app layer's job (user-directed)

Mid-sprint the user corrected the numbering model: **the app owns chapter numbering, derived from order**, so
deleting/inserting a chapter renumbers untitled chapters for free with no disk rewrite. This matches macOS
`ManuscriptTextView` (derives "Chapter N" from the ordered chapter list) and reshaped T-0256 —
`SceneDocument::chapterHeadingText` derives the ordinal from segment order (custom title wins). This closed
I-0062 more cleanly than the originally-planned Option-B `openProject` re-fetch, which was dropped.

## Issues

- **I-0062** (`[Linux]` new-chapter heading reads "Chapter" until reload) — ✅ **Resolved-Verified** this
  sprint (T-0256).

Surfaced during VNC verification, **logged and deferred (not SP-066 scope):**
- **I-0064** (`[Linux]`, Medium) — **Ctrl+Shift+Return appends a chapter at the manuscript end instead of
  splitting/inserting at the caret** (no scene reassignment, no renumber). Pre-existing since EP-022/SP-062;
  `scrivi_create_chapter` is append-only, and macOS orchestrates `create` + `reorder_chapter` + `reorder_scene`
  + `save_scene` + renumber on top of it. **Targeted SP-067** (reorder), whose primitives the fix needs.
- **I-0063** (`[Linux]`, Low) — created (stored-"Chapter N") chapters don't renumber when an earlier chapter
  is deleted. Backlog; pairs with I-0064's renumber logic in SP-067. (Untitled chapters already renumber via
  the derived ordinal.)

## Retrospective

**Completed:** T-0254 (bridge wrappers + `chapterMetadataPath`) · T-0255 (context-menu Rename via
`QInputDialog` + live label/heading + fallback) · T-0256 (I-0062 close via app-derived ordinal) · T-0257
(`scene_rename_smoke` + create-smoke I-0062 assertion; 9 smokes green).

**Returned to Backlog:** none.

**What went well:**
- The user's correction on chapter numbering reshaped the design for the better — deriving "Chapter N" from
  order (the macOS mechanism) closed I-0062 more cleanly than the planned re-fetch.
- `QInputDialog` was the right rename surface — simple, native, no `QTreeView` edit-trigger fight.
- Docker build + 9 headless smokes on the macOS host caught a regression (positional aggregate-init in
  `editor_map_smoke` broken by the new `Input` field) before VNC.

**What to improve:**
- Over-reached mid-sprint with ordinal renumber-on-delete + a smoke Case D that depend on ScriviCore behavior
  (stored "Chapter N") it doesn't provide. Descoped cleanly, but checking `ChapterCreator`/`ChapterDeleter`
  behavior should have preceded writing those assertions.

**Carry-forward notes:**
- **I-0064** (chapter-split on Ctrl+Shift+Return) is the priority follow-on — writer-facing, fully blueprinted
  for **SP-067**, which delivers the `reorder_*` orchestration it needs.
- **I-0063** (renumber created chapters on delete) pairs with I-0064's renumber logic in SP-067.
- The unused `SceneDocument::reflowAllChapterHeadings` helper was left in for the I-0063/I-0064 fix to call.

---

*Closed: 2026-07-15 (user-approved). Second of EP-023's 4 sprints. Delivered AC3; closed I-0062. Tasks
T-0254–T-0257 ✅ Verified and archived to `Tasks/Verified/Task-verified-0254-0257.md`. Surfaced I-0064
(→ SP-067) and I-0063 (→ backlog/SP-067).*
