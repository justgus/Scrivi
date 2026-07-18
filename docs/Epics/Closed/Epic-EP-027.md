# Closed Epic: EP-027

## EP-027: [ScriviCore] Filesystem-Authoritative Chapter/Scene Identity & Ordering

**Codebase:** `[ScriviCore]` (shared C++ backend) + per-platform verification (`[Linux]`, `[Apple]`).
**Cross-platform:** changes the on-disk `.scrivi` package layout, so it affects **every** platform that reads a
project. `scrivi.h` shape kept stable where possible (the change is on-disk behavior, not API).

**Status:** ✅ **CLOSED (Human-approved 2026-07-18).** All acceptance criteria AC1–AC8 Verified across 6 phases /
5 sprints (SP-069 P1–P3, SP-070 P6, P4 Linux VNC verify, SP-072 P5 Apple verify + migration fixes). ctest green on
both platforms (**304/304 macOS, 311/311 Linux**); the class of I-0072 corruption is fixed; real legacy projects
migrate losslessly on open (verified on macOS + Linux with `the-twisted-remains-of-myself.scrivi`). **Design decided**
— see `docs/Scrivi_Chapter_Folder_and_Identity_Trade_Study_v0_1.md` (Human-approved 2026-07-16).

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
**Actual Close Date:** 2026-07-18

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
  the C6 "Missing scene.meta.json". ✅ **SP-070 2026-07-17** (T-0271–T-0277; ctest 298/298 macOS, 5 new §8 tests).
  ✅ **Verified (2026-07-18, VNC)** — B3 (split at caret / I-0064), B4 (end-of-scene split / I-0069+I-0070), C6
  (orphan scene opens cleanly), and I-0071 (last-scene drag leaves no empty chapter) all pass on the P6 model.
  *(Pulled forward before P4 — P4 could not be verified without it.)*
- [x] AC8 — **Cross-platform verify + no regression:** ✅ **Verified (2026-07-18).** ScriviCore `ctest` green on
  both platforms (**macOS 304/304, Linux 311/311**, incl. the new order-key/rename/migration/rebuild + I-0076/I-0077
  regression tests). **Linux:** VNC walkthrough (B3/B4/C6 + I-0071) passed; **Apple:** the rebuilt macOS app opened +
  migrated a real legacy project (`the-twisted-remains-of-myself.scrivi`, numeric folders + gaps + a chapter id
  mismatch) cleanly — all chapters + content intact (user-observed). P5 surfaced 3 defects (I-0076/I-0077/I-0078),
  all fixed + Verified. **Linux VNC open of the same real legacy project also confirmed (2026-07-18):** opened clean;
  on-disk result is a correct migration — all 14 chapters reslugged to order-keys (no stranded numeric folder →
  I-0077), **zero** stale scene `content.path` (was 18 pre-fix → I-0076), and **21/21 scene bodies preserved**
  (the single empty `.md` was already empty in the pristine backup). Existing flows unbroken (full suite green both
  platforms).

### Phases / Sprints (sketch — IDs assigned at activation)

**Core-first, then per-platform verification** (Human decision 2026-07-16). Chapters before scenes.

| Phase | Scope | Codebase | Sprint / Status |
| ----- | ----- | -------- | --------------- |
| P1 — Rename primitive | `renamePath`/move in the FileSystem port + `LocalFileSystem` + crash-safe tests (AC5) | `[ScriviCore]` | **SP-069** 🟢 Implemented (2026-07-16) — `FileSystem::renamePath` (atomic-within-fs, no-clobber, missing-source guard); T-0264; ctest green macOS + Linux (`[renamePath]` 5/20). Not verified. |
| P2 — Order-key + disk-authority (chapters) | Order-key generator; `ChapterCreator`/`Reorderer` on order-key slugs; disk-authoritative order; open-time index self-heal (AC1–AC4) | `[ScriviCore]` | ✅ **Functionally complete — Not Verified (2026-07-16, ctest 288/288 macOS + Linux).** `util/OrderKey` (fractional keys, 3119 property assertions); `manuscript/ChapterIndex` disk-authoritative helpers + `rebuildIndexIfInconsistent`; `ChapterCreator` order-key slugs (**I-0072 collision FIXED + regression**); `ManuscriptOrderResolver` orders by **folder-key sort (B3)**; `ChapterReorderer` = `keyBetween` + **`renamePath` one folder** (paths rewritten); **open-time self-heal repairs an I-0072-corrupt index** (phantom/duplicate → rebuilt from disk, idempotent, tested). **Deferred (Human decision — churn without functional gain):** dropping `chapterID` from `ChapterRef` schema + migrating the 3 consumers that read it — the index `chapterID` is now a self-healing cache that can't diverge (trade study §7.6). |
| P3 — Migration | Detect + lazy/idempotent/resumable old→new migration; dual-scheme read (AC6) | `[ScriviCore]` | ✅ **Functionally complete — Not Verified (2026-07-16, ctest 290/290 macOS + Linux).** `migrateChapterOrderKeys` (ChapterIndex): a legacy project whose folder-key sort ≠ its index-array reading order (e.g. after a legacy reorder that shuffled the array, not the folders) has its folders reslugged to order-keys **in index-array order** via the shared `renameChapterFolder` primitive (paths rewritten, bodies intact). No-op for new-scheme/already-in-order projects; idempotent + resumable; generated keys are letter-prefixed so they can't collide with legacy numeric `chapter-NNN`. Wired into `ProjectOpener` (runs before self-heal + resolve). Also refactored: `ChapterReorderer` now shares `renameChapterFolder` (dedup). Tests `[migration]` (2 cases: reorder-legacy + no-op). |
| P6 — Scenes | Apply order-key + disk-authority to scenes; cross-chapter reorder relocates files; migration + orphan-repair on open (AC7) | `[ScriviCore]` | ✅ **SP-070** — **Verified (2026-07-18, VNC + Linux ctest 309/309).** Trade study §8. `SceneRef` filename-only; `SceneIndex`; `SceneReorderer` relocates files; `renameChapterFolder` simplified; repair-on-open moved ahead of validation (C6 fix). B3/B4/C6 + I-0071 pass. **Pulled forward before P4.** |
| P4 — Linux verify | Rebuild/verify the Linux app on the new model; VNC create/reorder/migrate/**split**; re-home the paused SP-067 structure Issues here | `[Linux]` | 🟢 Walkthrough passed — **user-observed (2026-07-18, VNC)**: **B3** (split at caret / I-0064), **B4** (end-of-scene split / I-0069+I-0070), **C6** (orphan scene opens cleanly), and **I-0071** (last-scene drag leaves no empty chapter) all pass. Gate: Linux-container ctest **309/309**. Awaiting user Verified stamp for AC8. |
| P5 — Apple verify | Confirm Apple opens + migrates; no regression (AC8) | `[Apple]` | ✅ **Verified (2026-07-18, SP-072).** Rebuilt macOS app opened + migrated a real legacy project cleanly (user-observed); 3 defects found & fixed (I-0076/I-0077/I-0078). Details below. 🔴 **(history — in progress — blocked by migration defects found 2026-07-18.)** macOS build regression fixed first (**I-0078** — `createChapter` wrapper drifted behind the C ABI's `afterChapterID`; app hadn't compiled since SP-071). Real legacy project (`the-twisted-remains-of-myself.scrivi`, 14 chapters, numeric folders + gaps, legacy scene refs, one index/sidecar id mismatch) opened → app ran migration → **refused to open: "Repair required: Missing scene content file."** Two EP-027 migration defects diagnosed: **I-0076** (legacy scene `content.path` not normalised to bare filename — 13/14 chapters left pointing at vanished `chapter-NNN` folders; `isOrderKey` accepts `001`/`002`/… so `migrateScenes` skips them) and **I-0077** (chapter with index/sidecar id mismatch dropped as "phantom" → half-migrated `chapter-004`). **Both FIXED 2026-07-18** (Resolved-Not-Verified): `parseSceneMeta` bares `content.path` on read + `migrateScenes` rewrites stale sidecars idempotently (I-0076); `migrateChapterOrderKeys` falls back to path-folder match on id mismatch (I-0077). 2 regression tests added, each verified RED-without-fix; **ctest 304/304 macOS**; macOS app BUILD SUCCEEDED. **Re-verify (2026-07-18):** user opened a FRESH copy of `the-twisted-remains-of-myself.scrivi` (from the pristine backup) in the rebuilt macOS app → **opened cleanly, no "Repair required", all chapters + content present** (user-observed passing). P5 macOS open+migrate now demonstrated on real legacy data. Linux-container ctest parity ✅ **311/311** (incl. both new I-0076/I-0077 regression tests). **Remaining:** user Verified stamp for AC8 (macOS open+migrate was user-observed; a Linux VNC open of a migrated project is optional extra coverage). |

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
