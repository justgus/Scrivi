# Closed Epic: EP-023

## EP-023: [Linux] Manuscript Structure Editing

**Codebase:** `[Linux]` (Qt/QML Ubuntu app, `platforms/linux/`) — calls `[ScriviCore]` only via the
existing plain-C ABI through `ScriviBridge`. **No ScriviCore work:** all eight structure endpoints already
exist in `scrivi.h` (create scene/chapter already wrapped by `ScriviBridge` in SP-062; delete/reorder/rename
need bridge wrappers only). Any genuinely missing endpoint would be a Task with a `[ScriviCore]` note — none
expected.

**Status:** ✅ **CLOSED (Human-approved 2026-07-19).** All ACs AC1–AC8 Verified.
All four sprints delivered: SP-065 (delete), SP-066 (rename, I-0062), SP-067 (scene drag-reorder,
I-0063/I-0067/I-0068), and **SP-073** (chapter drag-reorder + full verify; renumbered from the skipped
SP-068). The chapter-split defects surfaced in SP-067 (I-0064/I-0069/I-0070) and the root slug corruption
(I-0072) were re-homed to EP-027 (closed 2026-07-18) and verified there. SP-073's verification surfaced and
same-day-fixed three final defects — **I-0080** (`[ScriviCore]` open-time migration undid chapter reorders),
**I-0081** (`[ScriviCore]`+`[Linux]` stale scene paths after drag broke rename/save), **I-0082** (`[Linux]`
non-selectable chapter rows blocked the drag) — all ✅ **Verified over VNC 2026-07-19** in the re-run
walkthrough (full AC7 loop, no errors).
**Goal:** Give the Linux writer full control over manuscript structure **from the scene navigator** —
**create**, **delete** (with confirmation), **rename**, and **reorder** scenes and chapters. Scenes reorder
within and across chapters by drag; chapters reorder as containers carrying their scenes. Rename writes the
sidecar `title` field. This is the Ubuntu analogue of Apple **EP-010** (Manuscript Structure Editing —
Delete, Reorder, and Title), building directly on EP-022's navigator + continuous viewport.

**Strategy:** Fourth of the `[Linux]` family (EP-020–EP-026). EP-020 gave the spine, EP-021 the project
lifecycle, EP-022 the writing loop; EP-023 makes the manuscript **editable as a structure** (not just as
text). Verified in Docker+VNC (developer) and, per milestone, on real Ubuntu (alpha tester). Sprint count
decided at planning (4 — see Sprints).

**Design references:**
- `ScriviCore/include/scrivi/scrivi.h:173–219` — the eight endpoints, already present and stable:
  `scrivi_create_scene`, `scrivi_create_chapter`, `scrivi_delete_scene`, `scrivi_delete_chapter`,
  `scrivi_reorder_scene(sceneID, sourceChapterID, targetChapterID, afterSceneID)`,
  `scrivi_reorder_chapter(chapterID, afterChapterID)`, `scrivi_rename_scene(metadataPath, newTitle)`,
  `scrivi_rename_chapter(metadataPath, newTitle)`.
- `platforms/linux/src/ScriviBridge.{hpp,cpp}` — the envelope-parsing bridge; `createScene`/`createChapter`
  already wrapped (SP-062, `ScriviBridge.cpp:153–193`). Delete/reorder/rename follow the same pattern.
- `platforms/linux/src/EditorShell.*`, `ManuscriptEditor.*`, `SceneDocument.*` — the `QTreeView` navigator
  (chapter parents → scene children) + continuous `QPlainTextEdit`/`QTextDocument` viewport with the
  `SceneDocument` offset map; the surgical splice path SP-062 built for in-editor creation is the model for
  delete/reorder edits to the document + map + navigator.
- Apple analogue (behavior to re-create in C++/QML, **not** port wholesale): EP-010's navigator context menu
  (Rename/Delete), rename edit sheet, delete confirmation (chapter delete warns "chapter + all its scenes"),
  drag reorder with insertion-line highlight, delete-of-open-scene → nearest remaining scene + focus
  transfer, blank-title fallback chain (custom → first prose line → "Scene N"/"Chapter N").

**Date Created:** 2026-07-15
**Target Close Date:** TBD (4 sprints)
**Actual Close Date:** 2026-07-19

### Acceptance Criteria (draft — refine at each sprint's planning)

- [x] AC1 — **Delete:** right-click (context menu) on a scene row offers **Delete** with a confirmation
  dialog before removal; on a chapter row, **Delete** with a confirmation that warns the chapter **and all
  its scenes** will be removed. Deletes call `scrivi_delete_scene`/`scrivi_delete_chapter`; the navigator +
  continuous viewport + `SceneDocument` map update to match on-disk state. ✅ **Verified over VNC 2026-07-15**
  (SP-065 — context menu → confirm → removal; chapter delete warns "and all N scene(s)"; map splice +
  boundary reflow).
- [x] AC2 — **Delete-of-open/active scene edge case:** deleting the currently active scene loads the nearest
  remaining scene (next, else previous); keyboard focus transfers to the editor; caret lands sensibly.
  Deleting the last scene of a chapter, or the last scene in the manuscript, is handled without corruption.
  ✅ **Verified over VNC 2026-07-15** (SP-065 — nearest promoted + editor keeps focus; last-scene/last-chapter
  guards refuse with a notice; no corruption).
- [x] AC3 — **Rename:** the context menu offers **Rename**, opening a focused edit field/dialog
  pre-populated with the current title; saving writes the sidecar `title` via
  `scrivi_rename_scene`/`scrivi_rename_chapter`. A blank/whitespace-only title is treated as "no custom
  title" (sidecar `title` saved empty); the navigator falls back to first prose line (scene) or "Chapter N"
  (chapter). Also closes **I-0062** — the live chapter-heading label now reflects the real title without a
  reload. ✅ **Verified over VNC 2026-07-15** (SP-066 — `QInputDialog` rename; live label + in-document
  heading update; blank chapter → app-**derived** ordinal "Chapter N" from segment order, macOS parity; blank
  scene → first-line/"Untitled"; I-0062 closed). *Note: chapter numbering is derived from order (the app
  layer owns it); renumbering **created** chapters on delete is I-0063, and Ctrl+Shift+Return chapter-split is
  I-0064 — both → SP-067, not AC3.*
- [x] AC4 — **Scene reorder (drag):** scene rows are draggable in the `QTreeView`; dragging a scene within
  its chapter reorders it; dragging a scene across a chapter boundary moves it to the target chapter at the
  indicated position, calling `scrivi_reorder_scene(sceneID, sourceChapterID, targetChapterID,
  afterSceneID)`. The continuous viewport re-splices to the new manuscript order. ✅ **Verified over VNC
  2026-07-16** (SP-067 — within/cross-chapter drag, insertion-line highlight, persists across quit→reopen;
  I-0067/I-0068 fixed by forcing `Qt::CopyAction` so Qt never auto-removes the source row).
- [x] AC5 — **Chapter reorder (drag):** chapter rows are draggable and move as a **unit** (chapter + all its
  scenes) to the new position via `scrivi_reorder_chapter(chapterID, afterChapterID)`. A clear
  insertion-line drop highlight makes the landing position unambiguous, including chapter-boundary positions
  (last-of-previous vs. first-of-next). ✅ **Verified over VNC 2026-07-19** (SP-073 — chapter-as-container
  drag with boundary-only landings persists across quit→reopen; round 1 found I-0082 (non-selectable rows
  blocked the drag) and I-0080 (open-time migration undid the reorder), both fixed + re-verified).
- [x] AC6 — **Create parity retained:** in-editor Ctrl+Return (scene) / Ctrl+Shift+Return (chapter) from
  EP-022 still work and are consistent with the new navigator structure ops (shared splice path, no
  regression). ✅ **Verified over VNC 2026-07-19** (SP-073 walkthrough — "create works ok").
- [x] AC7 — **Persistence + verify:** every create/delete/rename/reorder updates the manuscript/chapter
  index JSON on disk atomically (ScriviCore's job — confirmed via reload); the full loop
  create→rename→reorder→delete→quit→reopen is VNC-verified with real on-disk state; headless smokes green
  in Docker/CI. ✅ **Verified over VNC 2026-07-19** (SP-073 — full loop passed, no errors; round 1 found
  I-0081 (post-drag scene rename hit stale paths), fixed + re-verified; 11/11 headless smokes green).
- [x] AC8 — **No regression:** `scrivi.h` unchanged (additive-only if ever touched — not expected); ScriviCore
  `ctest` green; EP-020/EP-021/EP-022 flows (landing, create, open, close, recents, navigator, editing,
  auto-save, scroll, restore) still build and work; the macOS app unaffected. ✅ **Verified 2026-07-19** —
  `scrivi.h` untouched (all SP-073 core changes payload/behavior-only); ctest **306/306 macOS + 313/313
  Linux**; macOS `ScriviApp` BUILD SUCCEEDED; prior-Epic flows exercised throughout the VNC loop.

### Sprints

**Planned as 4 sprints** (decided 2026-07-15). No core work — pure bridge + `QTreeView`/viewport UI. Split
by risk: the two low-risk, high-value ops (delete, rename) first; then the two drag-reorder ops (the
ambitious `QTreeView` internal drag-drop, EP-010's biggest UI piece) split scene-reorder then
chapter-reorder; then verify + close. IDs/tasks assigned at each sprint's activation; titles/scope may
refine at planning.

| Sprint | Title | Status | Dates |
| ------ | ----- | ------ | ----- |
| SP-065 | **Delete** scene/chapter — bridge wrappers (`delete_scene`/`delete_chapter`) + navigator context menu (Delete) + confirmation dialogs (chapter warns "+ all scenes") + document/map/navigator removal splice + delete-of-active-scene → nearest + focus. (AC1, AC2) | ✅ Closed | 2026-07-15 |
| SP-066 | **Rename** scene/chapter — bridge wrappers (`rename_scene`/`rename_chapter`) + context-menu Rename + focused edit field + blank-title fallback chain + **live chapter-heading label (closes I-0062)**. (AC3) | ✅ Closed | 2026-07-15 |
| SP-067 | **Scene drag-reorder** in `QTreeView` (`reorder_scene`) + viewport re-splice + insertion-line highlight (**AC4**) + **I-0063 renumber**. *Chapter-split I-0064/I-0069/I-0070 + slug corruption I-0072 re-homed to EP-027.* | ✅ Closed | 2026-07-15 → 2026-07-16 |
| SP-073 *(renumbered from SP-068 — ID skipped by EP-027's SP-069–SP-072)* | **Chapter drag-reorder** (chapter-as-container, `reorder_chapter`) + boundary-unambiguous drop highlight + full EP-023 verify (create/rename/reorder/delete/quit/reopen) + **Epic close**. (AC5, AC6, AC7, AC8) — delivered + I-0080/I-0081/I-0082 fixed & Verified | ✅ Closed | 2026-07-19 |

**Split rationale:** SP-065/066 are the low-risk menu-driven ops (delete, rename) — they exercise the bridge
+ navigator + splice path without drag machinery, and land two immediately useful capabilities. SP-067/068
carry the drag-and-drop reorder (EP-010's largest UI piece; `QTreeView` internal move + insertion-line
highlight is the real work), split scene-then-chapter so chapter-as-container (moving N scenes as a unit)
doesn't destabilize the simpler scene case. SP-068 also runs the parity/verification pass and closes EP-023.

### Design decisions (locked 2026-07-15, pre-SP-065)

- **No ScriviCore work.** All eight endpoints exist and are stable (`scrivi.h:173–219`). EP-023 is pure
  `platforms/linux/` bridge + UI, exactly like EP-022. `scrivi.h` stays untouched (AC8).
  *(Softened 2026-07-19, post-EP-027: SP-071 already made an additive `scrivi.h` change
  (`createChapter(afterChapterID)`), and SP-073 may add the moved chapter's new folder path to the
  `reorder_chapter` envelope — a payload-only ScriviCore change, `scrivi.h` untouched — because EP-027's
  reorder reslugs the chapter folder and the app must refresh its cached scene paths. Decided at SP-073
  activation; see the sprint plan's Note 2.)*
- **Chapter-title-in-manuscript toggle → deferred to EP-026.** EP-010 bundled a global "Show chapter titles
  in manuscript" toggle in a Project Settings sheet. Linux has no settings sheet yet — that surface is
  EP-026's job. EP-023 is scoped to **create/delete/rename/reorder only**; the toggle waits for EP-026.
  *(User decision 2026-07-15.)*
- **Reorder UX = drag-and-drop in `QTreeView`** (not context-menu Move Up/Down) — full EP-010 parity: drag
  scene rows within/across chapters, drag chapter rows as containers, insertion-line drop highlight. The
  ambitious option; it drives the 2-sprint reorder split (SP-067 scene, SP-068 chapter). *(User decision
  2026-07-15.)*
- **Rename UX = focused inline edit** (context-menu Rename → editable field pre-filled with current title),
  the Qt-idiomatic analogue of EP-010's rename sheet. Blank/whitespace saves an empty `title`; navigator
  falls back (custom → first prose line → "Scene N"/"Chapter N"). Fixing the live heading label here retires
  **I-0062**.
- **Splice discipline (inherited from SP-062).** Delete/reorder edit the continuous `QTextDocument`, the
  `SceneDocument` offset map, and the `QTreeView` model **surgically** (mirroring SP-062's create splice) so
  the viewport and navigator stay in lock-step with on-disk index JSON — never a full-document rebuild that
  would lose caret/scroll.

### Tasks

Assigned at each sprint's activation. Next available: **T-0298** (T-0294–T-0297 claimed by SP-073's
planning, 2026-07-19).

| Sprint | Tasks | Delivers |
| ------ | ----- | -------- |
| SP-065 ✅ | T-0250 bridge wrappers · T-0251 context menu + confirmations · T-0252 removal splice + delete-of-active · T-0253 smoke + VNC verify — **all ✅ Verified 2026-07-15** | AC1, AC2 ✅ |
| SP-066 ✅ | T-0254 rename bridge wrappers + `chapterMetadataPath` · T-0255 context-menu Rename (`QInputDialog`) + live label/heading · T-0256 close I-0062 (app-derived ordinal) · T-0257 smoke + VNC verify — **all ✅ Verified 2026-07-15** | AC3 ✅ |
| SP-067 ✅ | T-0258 reorder bridge wrappers · T-0259 `SceneDocument::moveScene` re-splice · T-0260 `QTreeView` scene drag-drop (**AC4 verified**; I-0067/I-0068 CopyAction fix) · T-0262 I-0063 renumber (**verified**) · T-0263 smoke — **all done**. T-0261 (I-0064 split) 🔵 superseded → EP-027. | AC4 ✅ + I-0063 ✅ |
| SP-073 | T-0294 `NavigatorTree` chapter drag (CopyAction latch + boundary drop resolution/highlight) · T-0295 `onChapterDropped` + `SceneDocument::moveChapter` re-splice + post-reslug path refresh · T-0296 headless chapter-reorder smoke · T-0297 full EP-023 verify + close prep | AC5, AC6, AC7, AC8 |

### Issues

| ID | Title | Status |
| -- | ----- | ------ |
| I-0062 | Live new-chapter heading label reads "Chapter" until reload | ✅ Resolved-Verified (SP-066 / T-0256 — app derives "Chapter N" ordinal from segment order, macOS parity) |
| I-0064 | Ctrl+Shift+Return appends a chapter at the manuscript end instead of splitting/inserting at the caret | 🔵 **Moved to EP-027** (2026-07-16) — the chapter-split path is rebuilt on EP-027's new filesystem-authoritative on-disk model rather than fixed twice; SP-067 VNC found the mid-scene case worked but end-of-scene failed (→ I-0069/I-0070), all re-homed to EP-027 |
| I-0063 | Deleting/inserting a chapter doesn't renumber later **created** (stored-"Chapter N") chapters | ✅ **Resolved-Verified (2026-07-16, VNC)** — Option A app-side renumber (SP-067/T-0262); untitled already renumber via the derived ordinal |
| I-0072 | `chapter-<count+1>` slug collision corrupts the manuscript index (found during SP-067 drag diagnosis) | 🔵 **Root defect of EP-027** — the reason chapter structure ops corrupt the index; EP-027's A4b+B3 rework fixes the class |
| I-0080 | `[ScriviCore]` Open-time chapter migration undoes a legitimate chapter reorder (stale index array hijacked as order authority; masked by the no-clobber guard) | ✅ Resolved - **Verified (2026-07-19, VNC)** — legacy gate (digits-only folder set) + eager cache rebuild in `ChapterCreator`/`ChapterReorderer`; chapter reorder persists across quit→reopen |
| I-0081 | `[ScriviCore]`+`[Linux]` Scene rename/save fails after a drag-reorder (§8 reorder renames/relocates the files; segment paths stale) | ✅ Resolved - **Verified (2026-07-19, VNC)** — reorder envelope reports post-move paths; `refreshScenePaths` applies them; rename-after-drag works with no restart |
| I-0082 | `[Linux]` Chapter drag never starts (non-selectable heading rows can't satisfy Qt's selected-draggable gate; rubber-band selected the nearest scene) | ✅ Resolved - **Verified (2026-07-19, VNC)** — chapter rows selectable; container drag works |

### Open Questions

1. ✅ **Chapter-title-in-manuscript toggle** — **Resolved (2026-07-15): deferred to EP-026** (Linux settings
   sheet lives there). EP-023 = create/delete/rename/reorder only.
2. ✅ **Reorder UX** — **Resolved (2026-07-15): drag-and-drop in `QTreeView`** (full EP-010 parity), split
   scene (SP-067) then chapter (SP-068).
3. **Delete-of-active-scene target** *(confirm at SP-065 planning).* Apple EP-010: next scene, else
   previous. Confirm the Linux mapping against the `SceneDocument` active-scene model + how the departing
   scene's dirty state is flushed before removal.
4. **Rename edit surface** *(confirm at SP-066 planning).* Inline `QTreeView` item edit vs. a small modal
   `QInputDialog`/dialog. Inline is more native; a dialog is simpler and closer to the Apple sheet. Decide at
   planning.

### Scope Notes

- **In scope:** scene/chapter **delete** (with confirmation; chapter carries its scenes), **rename**
  (sidecar `title` + fallback chain + live heading), **reorder** (drag: scene within/across chapters,
  chapter as container, insertion-line highlight); delete-of-active-scene navigation + focus; retained
  in-editor create (Ctrl+Return / Ctrl+Shift+Return from EP-022).
- **Out of scope (later `[Linux]` Epics or unscheduled):** chapter-title-in-manuscript toggle + Project
  Settings sheet (EP-026); inspector panel (EP-024); timeline (EP-025); undo/redo of structural ops
  (structural undo is a documented future extension — EP-019 records barriers only); "Move to Chapter…"
  picker (EP-010 left it in backlog — same here unless drag covers the need).
- **Bridge discipline (inherited):** every new `ScriviBridge` method parses the `{"ok":…}`/`{"error":…}`
  envelope, `scrivi_free`s every pointer (RAII `ScriviString`), holds **no** backend logic. New
  `.cpp/.hpp/.qml` are CMake-tracked (not in the Apple pbxproj), browsable via the `platforms/linux` Xcode
  folder reference.
- **Verification model (inherited):** CI = build + headless smoke; developer = GUI over Docker+VNC (host
  port 5901, password `scrivi`; persistent app-support + shared `/projects` mounts); alpha tester = real
  Ubuntu.

### Completion Summary

**EP-023 delivered full manuscript-structure editing from the Linux scene navigator** — the Ubuntu analogue
of Apple EP-010 — across four sprints (SP-065/066/067/073; the SP-068 ID was skipped) plus the EP-027
interlude that rebuilt the on-disk model underneath it. All ACs AC1–AC8 Verified over Docker+VNC.

- **Delete** (SP-065): context-menu delete with confirmation (chapter warns "+ all N scenes"), surgical
  removal splice, delete-of-active → nearest scene + focus, last-scene/last-chapter guards. AC1/AC2.
- **Rename** (SP-066): `QInputDialog` rename writing the sidecar `title`, blank-title fallback chain
  (custom → first prose line → derived "Chapter N" ordinal, macOS parity), live heading label (I-0062).
  AC3.
- **Scene drag-reorder** (SP-067): `NavigatorTree` drag with insertion-line highlight, within/cross-chapter,
  `SceneDocument::moveScene` re-splice, CopyAction-only discipline (I-0067/I-0068), I-0063 renumber. AC4.
- **Chapter drag-reorder + verify** (SP-073): chapter rows drag as containers (selectable rows + manual
  `QDrag`, boundary-only drop resolution), `SceneDocument::moveChapter` block splice, post-reslug path
  refresh driven by new reorder-envelope paths (scene AND chapter — closing the I-0074/I-0079 stale-path
  class end-to-end), full create→rename→reorder→delete→quit→reopen loop VNC-verified. AC5–AC8.

**Defects found & fixed during EP-023 verification, all Verified:** I-0062 (live heading), I-0063
(renumber), I-0067/I-0068 (Qt MoveAction auto-remove), I-0080 (`[ScriviCore]` open-time migration undid
chapter reorders — legacy gate + eager index-cache coherence in `ChapterCreator`/`ChapterReorderer`),
I-0081 (stale scene paths after drag — envelope now reports post-move paths), I-0082 (non-selectable
chapter rows blocked the drag). The chapter-split family (I-0064/I-0069/I-0070/I-0071/I-0072) was re-homed
to EP-027 and verified there.

**Core impact:** `scrivi.h` untouched for the whole Epic. SP-073 made payload/behavior-only ScriviCore
changes: `ReorderSceneResult`/`ReorderChapterResult` report post-move paths, and I-0080's fix hardened the
EP-027 open path. Final test state: ctest **306/306 macOS + 313/313 Linux**; **11/11** Linux headless
smokes (new `chapter_reorder_smoke` + extended `scene_reorder_smoke` in CI); macOS app unaffected (BUILD
SUCCEEDED, real-app open confirmed during EP-027 P5).

**Deferred/out-of-scope (unchanged):** chapter-title-in-manuscript toggle + settings sheet → EP-026;
inspector → EP-024; timeline → EP-025; structural undo → EP-019 future; "Move to Chapter…" picker —
drag covers the need.

---

*Last Updated: 2026-07-19 (EP-023 `[Linux]` — **🟠 COMPLETE, all ACs AC1–AC8 Verified; awaiting user close
approval.** The user's VNC re-verification passed the full checklist + AC7 loop with no errors: chapter
drag-as-container (AC5, incl. persist across quit→reopen — exercising the I-0080 fix), post-drag scene
rename (I-0081 fix), create parity (AC6), full structure loop (AC7); AC8 regression sweep green (ctest
306/306 macOS + 313/313 Linux, `scrivi.h` untouched, macOS app builds). I-0080/I-0081/I-0082 all marked
✅ Verified. SP-073 → 🟠 Review, awaiting sprint-close approval alongside the Epic. Completion Summary
filled in. Prior note follows.)*

*2026-07-19 earlier (EP-023 `[Linux]` — **SP-073 activated + implemented same day** (final sprint,
renumbered from the skipped SP-068): **AC5 chapter drag-reorder implemented** — `NavigatorTree` chapter-row
drag (manual `QDrag`, CopyAction-only, boundary-only drop resolution), `SceneDocument::moveChapter` block
splice + `refreshChapterPaths`, `EditorShell::onChapterDropped`; the `reorder_chapter` envelope now reports
the post-reslug `metadataPath` (payload-only core change, `scrivi.h` untouched). SP-073's new
`chapter_reorder_smoke` surfaced **I-0080** (`[ScriviCore]` open-time migration undid chapter reorders on
the next open — stale index array hijacked as order authority, masked by the no-clobber guard) — fixed with
a digits-only legacy gate + eager cache rebuild in `ChapterCreator`/`ChapterReorderer`, core regression
proven RED pre-fix. ctest **305/305 macOS + 312/312 Linux**; **11/11** container smokes; macOS app builds.
Remaining: **user VNC walkthrough (AC5/AC6/AC7) + Epic close approval**. Prior note follows.)*

*2026-07-19 earlier (SP-073 planning: chapter drag-reorder AC5 + full EP-023 verify AC6–AC8 + Epic close
prep; tasks T-0294–T-0297. EP-027 dependency satisfied — EP-027 ✅ closed 2026-07-18. Key design risk
flagged: `reorder_chapter` reslugs the moved chapter's folder but its envelope returned no new path —
resolved at activation via the additive envelope field (I-0074/I-0079 stale-path lesson).)*

*2026-07-16 (EP-023 `[Linux]` — **SP-067 ✅ closed** (third of 4 sprints): delivered **AC4 scene
drag-reorder** (I-0067/I-0068 fixed via `Qt::CopyAction`, VNC-verified on a fresh project) + **I-0063** renumber
(Verified). The chapter-split defects **I-0064/I-0069/I-0070** + root slug corruption **I-0072** + **I-0071** were
**re-homed to the new EP-027** `[ScriviCore]` (Filesystem-Authoritative Chapter/Scene Identity & Ordering);
follow-on **I-0073** (VNC drag lag) flagged. **3 of 4 sprints closed**; remaining SP-068 (chapter drag-reorder +
close) 🔵 Planning, sequenced against EP-027. Prior note follows.*

*2026-07-15 (**SP-067 🟡 activated** (third of 4
sprints): scene drag-reorder (AC4) + I-0064 Ctrl+Shift+Return chapter-split (T-0261) + I-0063 renumber created
chapters (T-0262); tasks T-0258–T-0263; no ScriviCore work, `scrivi.h` untouched; I-0064/I-0063 moved 🔵 Open →
🟡 In SP-067. Earlier: **SP-065 (delete) ✅ closed** and **SP-066 (rename) ✅ closed** (AC1/AC2/AC3 verified; I-0062 Resolved-Verified via app-derived "Chapter N"
ordinal, macOS parity; T-0250–T-0257 archived). **2 of 4 sprints closed.** Next: **SP-067** (scene drag-reorder
+ **I-0064** Ctrl+Shift+Return chapter-split + **I-0063** renumber). Two `[Linux]` defects surfaced during
SP-066 verify: I-0064 (chapter-split → SP-067) and I-0063 (renumber created chapters on delete → backlog/SP-067).
**No ScriviCore work** — all eight structure endpoints already exist (`scrivi.h:173–219`); pure
`platforms/linux/` bridge + `QTreeView`/viewport UI. Scoping locked pre-SP-065: chapter-title toggle
**deferred to EP-026**; reorder = **`QTreeView` drag-and-drop**. **Sized at 4 sprints** — SP-065 delete ✅ ·
SP-066 rename (closes I-0062) · SP-067 scene drag-reorder · SP-068 chapter drag-reorder + verify/close.
EP-024–EP-026 remain 🔵 Draft.)*
