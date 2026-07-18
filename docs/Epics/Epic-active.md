# Active Epics

## EP-027: [ScriviCore] Filesystem-Authoritative Chapter/Scene Identity & Ordering

**Codebase:** `[ScriviCore]` (shared C++ backend) + per-platform verification (`[Linux]`, `[Apple]`).
**Cross-platform:** changes the on-disk `.scrivi` package layout, so it affects **every** platform that reads a
project. `scrivi.h` shape kept stable where possible (the change is on-disk behavior, not API).

**Status:** 🟡 Active (activated 2026-07-16). **Design decided** — see
`docs/Scrivi_Chapter_Folder_and_Identity_Trade_Study_v0_1.md` (Human-approved 2026-07-16).

**Goal:** Make the **filesystem the source of truth** for chapter (then scene) **identity and order**, and stop
deriving folder names from positional counts. Concretely:
- **A4b — fractional order-key slugs:** `chapter-<orderKey>` where `orderKey` is a lexicographically-sortable
  rank string (`a0`, `a0m`, `a1`, …); a plain sorted directory listing equals manuscript reading order. Folder
  names are **order-key only** (no human suffix; the human title lives in the sidecar). Inserting/moving a
  chapter renames **at most one folder** (a new key between neighbours); a rare global **rebalance** is the only
  multi-rename event.
- **B3 — disk-authoritative:** chapter **identity** lives solely in the sidecar (`chapter.meta.json.chapterID`);
  chapter **order** is the folders' sort position. `chapterID` is **removed from `manuscript.meta.json`**, which
  becomes a **rebuildable cache/manifest**, never authoritative.
- **Order Authority = filesystem.**
- **New FileSystem-port primitive:** a crash-safe `renamePath`/move (the port has none today).
- **Migration:** detect old-format projects (numeric `chapter-NNN` names and/or per-entry `chapterID` in the
  index) and migrate them **lazily, idempotently, resumably** (repair-on-open), reading BOTH schemes during the
  transition and writing only the new one.
- **Load-time self-heal:** rebuild the index from disk when it's missing/inconsistent — this also repairs the
  I-0072 phantom/duplicate damage.

**Why now:** I-0072 root-caused the `chapter-<count+1>` slug collision that corrupts `manuscript.meta.json`
(phantom + duplicate entries, sidecar clobbering) after any delete — and the Human's review surfaced the deeper
architecture issues (positional names on an identity model; `chapterID` duplicated across index + sidecar with
no owner). This Epic fixes the class of bug, not just the instance, and delivers the self-describing, easily
repairable on-disk layout the Human wants.

**Design references:**
- `docs/Scrivi_Chapter_Folder_and_Identity_Trade_Study_v0_1.md` — **the decided design** (options, criteria,
  decision record §7).
- `ScriviCore/src/manuscript/ChapterCreator.cpp` (slug-collision origin), `ChapterReorderer.cpp` (order writer,
  clean), `ChapterDeleter.cpp` (no renumber), `SceneCreator.cpp` (same pattern → phase 2).
- `ScriviCore/include/scrivi/Services.hpp` (FileSystem port — needs `renamePath`), `LocalFileSystem.*`.
- `ScriviCore/src/schemas/ManuscriptMetaJson.*` (`ChapterRef{chapterID,path}` → `{path}`),
  `ChapterMetaJson.*` (sidecar owns id; has a `slug` field already).
- `ScriviCore/src/project_package/ProjectValidator.cpp`, `repair/ExternalChangeScanner.cpp` — existing
  integrity/repair machinery to extend for rebuild-from-disk.
- `docs/Scrivi_Project_Package_Structure_v0_1.md`, `Scrivi_Minimum_Schema_Set_v0_1.md`,
  `Scrivi_External_Change_Repair_Matrix_v0_2.md` — on-disk/schema/repair docs this Epic revises.

**Date Created:** 2026-07-16
**Target Close Date:** TBD
**Actual Close Date:** —

### Acceptance Criteria (draft — refine at each sprint's planning)

- [x] AC1 — **Order-key slugs (chapters):** new chapters get a fractional order-key folder name
  (`chapter-<orderKey>`); `listDirectory` sorted = reading order; **no name ever collides**, including after
  deletes that leave gaps. `chapter-<count+1>` is gone. Closes the I-0072 collision class. ✅ **P2 2026-07-16**
  (`ChapterCreator` + regression test; initial `chapter-001` kept — created once, never collides). Not verified.
- [x] AC2 — **Insert/reorder renames one folder:** moving a chapter picks a new order-key between its
  neighbours and renames **only that chapter's folder** via `renamePath` (rewriting the sidecar slug + embedded
  scene paths); the sorted listing reflects the new order. ✅ **P2 2026-07-16** (`ChapterReorderer`). Rare
  key-exhaustion rebalance path: not yet needed (deferred). Not verified.
- [x] AC3 — **Disk-authoritative identity + order (B3):** order is the on-disk folder-key sort
  (`ManuscriptOrderResolver` via `listChaptersByOrder`), identity is the sidecar `chapterID`. ✅ **P2 2026-07-16.**
  *Refinement:* the index `chapterID` is **kept as a self-healing cache** (not dropped from the schema) — it
  can't diverge (self-heals on open), so the literal schema drop is deferred as churn-without-gain (trade study
  §7.6). Not verified.
- [x] AC4 — **Rebuild-on-load self-heal:** an inconsistent index is rebuilt from the `chapter-*` folders on open
  (`rebuildIndexIfInconsistent`, wired into `ProjectOpener`). A project whose index disagrees with disk (incl.
  I-0072's phantom/duplicate damage) self-heals — tested. ✅ **P2 2026-07-16.** Not verified.
- [x] AC5 — **FileSystem rename/move primitive:** `renamePath` added to the port; atomic within a filesystem
  (`std::filesystem::rename` — a crash mid-rename never leaves a half-moved dir), **refuses to clobber** an
  existing destination, and refuses a missing source; `LocalFileSystem` impl; unit-tested (file move, directory
  move w/ contents, no-clobber file+dir, missing-source). ✅ **Implemented 2026-07-16 (SP-069/T-0264)** — ctest
  green macOS (273/273) + Linux container; not yet user-verified.
- [x] AC6 — **Migration of old-format projects:** a legacy project whose folder-key sort doesn't reproduce its
  index-array reading order is migrated at open (`migrateChapterOrderKeys`, wired into `ProjectOpener`) to
  order-key slugs, lazily/idempotently/resumably, without data loss (scene bodies preserved via path rewrite).
  Both schemes coexist: legacy numeric `chapter-NNN` are valid keys that keep working until migrated, and
  generated keys are letter-prefixed so they never collide with numeric folders. ✅ **P3 2026-07-16** (tests:
  reorder-legacy → correct order + bodies intact + idempotent; no-op when already in order). Not verified. *(A
  never-opened project isn't touched until opened — lazy, as specified.)*
- [x] AC7 — **Scenes:** the same order-key slug + disk-authority treatment applies to scenes (trade study **§8**,
  Human-approved 2026-07-17). Scene filenames are `<orderKey>-<slug>.{meta.json,md}`; `SceneRef` is **filename-only**
  (identity in the sidecar, derived by sceneID scan); cross-chapter reorder **relocates the files**; a chapter-folder
  rename touches **zero** scene fields; scene migration + orphan-repair runs on open (before validation) and dissolves
  the C6 "Missing scene.meta.json". ✅ **SP-070 2026-07-17** (T-0271–T-0277; ctest 298/298 macOS, 5 new §8 tests). Not
  verified. *(Pulled forward before P4 — P4 could not be verified without it.)*
- [ ] AC8 — **Cross-platform verify + no regression:** ScriviCore `ctest` green (new order-key, rename, migration,
  rebuild tests); the **Linux** app opens/creates/reorders/migrates correctly over VNC; the **Apple** app opens
  and migrates a project. Existing flows (create/open/close/delete/rename/reorder, history) unbroken on both
  platforms.

### Phases / Sprints (sketch — IDs assigned at activation)

**Core-first, then per-platform verification** (Human decision 2026-07-16). Chapters before scenes.

| Phase | Scope | Codebase | Sprint / Status |
| ----- | ----- | -------- | --------------- |
| P1 — Rename primitive | `renamePath`/move in the FileSystem port + `LocalFileSystem` + crash-safe tests (AC5) | `[ScriviCore]` | **SP-069** 🟢 Implemented (2026-07-16) — `FileSystem::renamePath` (atomic-within-fs, no-clobber, missing-source guard); T-0264; ctest green macOS + Linux (`[renamePath]` 5/20). Not verified. |
| P2 — Order-key + disk-authority (chapters) | Order-key generator; `ChapterCreator`/`Reorderer` on order-key slugs; disk-authoritative order; open-time index self-heal (AC1–AC4) | `[ScriviCore]` | ✅ **Functionally complete — Not Verified (2026-07-16, ctest 288/288 macOS + Linux).** `util/OrderKey` (fractional keys, 3119 property assertions); `manuscript/ChapterIndex` disk-authoritative helpers + `rebuildIndexIfInconsistent`; `ChapterCreator` order-key slugs (**I-0072 collision FIXED + regression**); `ManuscriptOrderResolver` orders by **folder-key sort (B3)**; `ChapterReorderer` = `keyBetween` + **`renamePath` one folder** (paths rewritten); **open-time self-heal repairs an I-0072-corrupt index** (phantom/duplicate → rebuilt from disk, idempotent, tested). **Deferred (Human decision — churn without functional gain):** dropping `chapterID` from `ChapterRef` schema + migrating the 3 consumers that read it — the index `chapterID` is now a self-healing cache that can't diverge (trade study §7.6). |
| P3 — Migration | Detect + lazy/idempotent/resumable old→new migration; dual-scheme read (AC6) | `[ScriviCore]` | ✅ **Functionally complete — Not Verified (2026-07-16, ctest 290/290 macOS + Linux).** `migrateChapterOrderKeys` (ChapterIndex): a legacy project whose folder-key sort ≠ its index-array reading order (e.g. after a legacy reorder that shuffled the array, not the folders) has its folders reslugged to order-keys **in index-array order** via the shared `renameChapterFolder` primitive (paths rewritten, bodies intact). No-op for new-scheme/already-in-order projects; idempotent + resumable; generated keys are letter-prefixed so they can't collide with legacy numeric `chapter-NNN`. Wired into `ProjectOpener` (runs before self-heal + resolve). Also refactored: `ChapterReorderer` now shares `renameChapterFolder` (dedup). Tests `[migration]` (2 cases: reorder-legacy + no-op). |
| P6 — Scenes | Apply order-key + disk-authority to scenes; cross-chapter reorder relocates files; migration + orphan-repair on open (AC7) | `[ScriviCore]` | ✅ **SP-070** — Implemented, Not Verified (2026-07-17). Trade study §8. `SceneRef` filename-only; `SceneIndex`; `SceneReorderer` relocates files; `renameChapterFolder` simplified; repair-on-open moved ahead of validation (C6 fix). ctest 298/298 macOS. **Pulled forward before P4.** |
| P4 — Linux verify | Rebuild/verify the Linux app on the new model; VNC create/reorder/migrate/**split**; re-home the paused SP-067 structure Issues here | `[Linux]` | 🔵 Planned — **now unblocked by P6**; re-run the VNC walkthrough (B3/B4/C6 should pass) |
| P5 — Apple verify | Confirm Apple opens + migrates; no regression (AC8) | `[Apple]` | 🔵 Planned |

### Issues rolled in (from SP-067 / EP-023, per the trade-study decision 2026-07-16)

| ID | Title | Disposition |
| -- | ----- | ----------- |
| I-0072 | `chapter-<count+1>` slug collision corrupts the index | **Root defect this Epic fixes** (AC1/AC4/AC6) |
| I-0064 | Ctrl+Shift+Return appends instead of splitting at the caret | Chapter-structure op — rebuilt on the new model here (was SP-067/T-0261) |
| I-0069 | End-of-scene-with-followers split shows no new chapter | Same split path — rebuilt here (was SP-067) |
| I-0070 | End-of-scene-no-followers split appends at manuscript end | Same split path — rebuilt here (was SP-067) |
| I-0071 | Last-scene drag orphans an empty chapter (backfill blank scene) | Scene-structure behavior — folds into P6/scenes |

**Not rolled in:** I-0067/I-0068 (scene drag vanish/no-persist) — these are an **app-layer** Qt MoveAction bug
independent of the on-disk model; they stay in SP-067 and are fixed there (verify on a fresh project). I-0063
(renumber created chapters) was **already Verified** and stays closed.

### Open items to confirm at P1/P2 planning

- Exact order-key encoding (base-N alphabet, rebalance threshold) — pick a proven scheme; document it.
- Whether the index (cache) is written eagerly by each op or purely rebuilt on load (leaning: written eagerly
  for speed, but authoritative-only as a rebuildable cache — never trusted over disk).
- `scrivi.h` impact — confirm the change stays on-disk-only (no C ABI shape change); if a new
  `scrivi_migrate`/`scrivi_rebuild_index` entrypoint is wanted, it's additive.

---

## EP-023: [Linux] Manuscript Structure Editing

**Codebase:** `[Linux]` (Qt/QML Ubuntu app, `platforms/linux/`) — calls `[ScriviCore]` only via the
existing plain-C ABI through `ScriviBridge`. **No ScriviCore work:** all eight structure endpoints already
exist in `scrivi.h` (create scene/chapter already wrapped by `ScriviBridge` in SP-062; delete/reorder/rename
need bridge wrappers only). Any genuinely missing endpoint would be a Task with a `[ScriviCore]` note — none
expected.

**Status:** 🟡 Active (activated 2026-07-15). **3 of 4 sprints closed** (SP-065 delete, SP-066 rename, SP-067
scene drag-reorder). **AC1/AC2/AC3 verified** (delete, rename; I-0062 closed) and **AC4 verified** (scene
drag-reorder, SP-067 — I-0067/I-0068 fixed via `Qt::CopyAction`; I-0063 renumber Verified). The chapter-split
defects surfaced in SP-067 (**I-0064/I-0069/I-0070**) and their root **`[ScriviCore]` slug corruption (I-0072)**
were **re-homed to EP-027** (`[ScriviCore]` Filesystem-Authoritative Chapter/Scene Identity & Ordering). Remaining:
**SP-068** (chapter drag-reorder AC5 + EP-023 verify/close) — 🔵 Planning; its chapter-structure verification
depends on EP-027's new on-disk model, so sequence SP-068 against EP-027.
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
**Actual Close Date:** —

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
- [ ] AC5 — **Chapter reorder (drag):** chapter rows are draggable and move as a **unit** (chapter + all its
  scenes) to the new position via `scrivi_reorder_chapter(chapterID, afterChapterID)`. A clear
  insertion-line drop highlight makes the landing position unambiguous, including chapter-boundary positions
  (last-of-previous vs. first-of-next).
- [ ] AC6 — **Create parity retained:** in-editor Ctrl+Return (scene) / Ctrl+Shift+Return (chapter) from
  EP-022 still work and are consistent with the new navigator structure ops (shared splice path, no
  regression). *(Create was delivered in EP-022 SP-062; EP-023 must not break it.)*
- [ ] AC7 — **Persistence + verify:** every create/delete/rename/reorder updates the manuscript/chapter
  index JSON on disk atomically (ScriviCore's job — confirmed via reload); the full loop
  create→rename→reorder→delete→quit→reopen is VNC-verified with real on-disk state; headless smokes green
  in Docker/CI.
- [ ] AC8 — **No regression:** `scrivi.h` unchanged (additive-only if ever touched — not expected); ScriviCore
  `ctest` green; EP-020/EP-021/EP-022 flows (landing, create, open, close, recents, navigator, editing,
  auto-save, scroll, restore) still build and work; the macOS app unaffected.

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
| SP-068 | **Chapter drag-reorder** (chapter-as-container, `reorder_chapter`) + boundary-unambiguous drop highlight + full EP-023 verify (create/rename/reorder/delete/quit/reopen) + **Epic close**. (AC5, AC6, AC7, AC8) — *note: chapter-structure ops depend on EP-027's new on-disk model; sequence against EP-027.* | 🔵 Planning | — |

**Split rationale:** SP-065/066 are the low-risk menu-driven ops (delete, rename) — they exercise the bridge
+ navigator + splice path without drag machinery, and land two immediately useful capabilities. SP-067/068
carry the drag-and-drop reorder (EP-010's largest UI piece; `QTreeView` internal move + insertion-line
highlight is the real work), split scene-then-chapter so chapter-as-container (moving N scenes as a unit)
doesn't destabilize the simpler scene case. SP-068 also runs the parity/verification pass and closes EP-023.

### Design decisions (locked 2026-07-15, pre-SP-065)

- **No ScriviCore work.** All eight endpoints exist and are stable (`scrivi.h:173–219`). EP-023 is pure
  `platforms/linux/` bridge + UI, exactly like EP-022. `scrivi.h` stays untouched (AC8).
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

Assigned at each sprint's activation. SP-065's tasks are defined at its planning (below). Next available:
**T-0250**.

| Sprint | Tasks | Delivers |
| ------ | ----- | -------- |
| SP-065 ✅ | T-0250 bridge wrappers · T-0251 context menu + confirmations · T-0252 removal splice + delete-of-active · T-0253 smoke + VNC verify — **all ✅ Verified 2026-07-15** | AC1, AC2 ✅ |
| SP-066 ✅ | T-0254 rename bridge wrappers + `chapterMetadataPath` · T-0255 context-menu Rename (`QInputDialog`) + live label/heading · T-0256 close I-0062 (app-derived ordinal) · T-0257 smoke + VNC verify — **all ✅ Verified 2026-07-15** | AC3 ✅ |
| SP-067 ✅ | T-0258 reorder bridge wrappers · T-0259 `SceneDocument::moveScene` re-splice · T-0260 `QTreeView` scene drag-drop (**AC4 verified**; I-0067/I-0068 CopyAction fix) · T-0262 I-0063 renumber (**verified**) · T-0263 smoke — **all done**. T-0261 (I-0064 split) 🔵 superseded → EP-027. | AC4 ✅ + I-0063 ✅ |
| SP-068 | chapter drag-reorder (container) · boundary drop highlight · full EP-023 verify + close prep | AC5, AC6, AC7, AC8 |

### Issues

| ID | Title | Status |
| -- | ----- | ------ |
| I-0062 | Live new-chapter heading label reads "Chapter" until reload | ✅ Resolved-Verified (SP-066 / T-0256 — app derives "Chapter N" ordinal from segment order, macOS parity) |
| I-0064 | Ctrl+Shift+Return appends a chapter at the manuscript end instead of splitting/inserting at the caret | 🔵 **Moved to EP-027** (2026-07-16) — the chapter-split path is rebuilt on EP-027's new filesystem-authoritative on-disk model rather than fixed twice; SP-067 VNC found the mid-scene case worked but end-of-scene failed (→ I-0069/I-0070), all re-homed to EP-027 |
| I-0063 | Deleting/inserting a chapter doesn't renumber later **created** (stored-"Chapter N") chapters | ✅ **Resolved-Verified (2026-07-16, VNC)** — Option A app-side renumber (SP-067/T-0262); untitled already renumber via the derived ordinal |
| I-0072 | `chapter-<count+1>` slug collision corrupts the manuscript index (found during SP-067 drag diagnosis) | 🔵 **Root defect of EP-027** — the reason chapter structure ops corrupt the index; EP-027's A4b+B3 rework fixes the class |

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

_(filled in when the Epic reaches 🟠 Complete)_

---

*Last Updated: 2026-07-16 (EP-023 `[Linux]` — **SP-067 ✅ closed** (third of 4 sprints): delivered **AC4 scene
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
