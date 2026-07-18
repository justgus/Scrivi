# Active Sprint

# SP-070: [ScriviCore] EP-027 P6 — Filesystem-authoritative scene identity & ordering (scenes)

**Status:** ✅ **Verified (2026-07-18, VNC + Linux ctest 309/309).** B3/B4 split + C6 orphan-open + I-0071
last-scene drag all pass on the P6 model (Human-driven VNC walkthrough); Linux-container ctest 309/309. AC7 +
AC8-Linux Verified; I-0064/I-0069/I-0070/I-0071 Verified. Sprint close awaits Human approval. Applies the §7
chapter model one level down to
**scenes** (trade study **§8**, Human-approved 2026-07-17). This is **P6 pulled forward before P4**, because the
P4 Linux VNC walkthrough (2026-07-16) failed B3/B4 (chapter-split) and C6 ("Missing scene.meta.json") — all root-
caused to the pre-EP-027 `SceneReorderer` moving a `SceneRef` between chapter sidecars **without relocating the
scene's files**, which chapter-folder renames then orphaned. ScriviCore `ctest` **298/298 macOS** (293 pre-existing,
all still green + 5 new §8 tests). Linux-container ctest parity: ✅ **CONFIRMED 2026-07-18** — fresh Ubuntu-24.04
container (image `scrivi-linux`), `-DSCRIVI_BUILD_TESTS=ON`, clean configure + build, **`ctest` 309/309, 0 failed**
(the 309 vs 298 delta is the Linux-only `EncryptedFileSecureStore` + C-ABI history tests; all EP-027 scene tests —
`SceneIdentityTests` §8 cases + `SceneSplitRepro` — compiled and passed). Portable C++23, no platform code. **P4
gate cleared.**
**Activated:** 2026-07-17
**Epic:** EP-027 `[ScriviCore]` — Filesystem-Authoritative Chapter/Scene Identity & Ordering (**Phase 6 of 6**).
**Codebase:** `[ScriviCore]` only. `scrivi.h` untouched (on-disk behavior change, not a C ABI change).

## Design (trade study §8)

- **`SceneRef` → filename-only.** The chapter sidecar's scene reference is the **bare filename** of the scene
  `.meta.json` (resolved against the chapter's own folder). It drops the embedded chapter-folder prefix **and**
  the duplicated `sceneID`. `scenes[]` stays an ordered, rebuildable cache. Identity (`sceneID`) lives solely in
  the scene sidecar and is **derived by scanning sidecars** when a lookup needs it (Human decision 2026-07-17 —
  no duplicated identity in the ref that could drift; a moved/renamed scene is always recoverable by sceneID scan).
- **Order-key scene filenames.** Scenes are `<orderKey>-<slug>.{meta.json,md}` (reusing `util/OrderKey`); the
  order-key filename sort within a chapter folder is scene reading order (B3). `contentPath` is a **bare filename**.
- **Cross-chapter reorder RELOCATES the files** (`renamePath` both files into the destination folder, assigns a
  between-neighbours order key, rebuilds both caches). No cross-child path rewriting.
- **A chapter-folder rename now touches ZERO scene fields** — filename-only refs + bare contentPaths need no
  rewrite. `renameChapterFolder` simplified accordingly (removed the per-scene path-rewrite loop).
- **Repair-on-open reordered ahead of validation** (`ProjectOpener` step 0): chapter migration → chapter index
  self-heal → **scene migration + orphan repair** run **before** `ProjectValidator`, so a repairable project
  opens instead of erroring (the C6 fix). The scene self-heal preserves a genuinely-missing ref so the validator
  still surfaces a real loss.

## Tasks

| ID | Title | Status |
| -- | ----- | ------ |
| T-0271 | **`SceneRef` filename-only schema + serde** — `{metadataFilename}` (drops `sceneID`+path); dual-scheme read keeps a legacy `metadataPath`/`sceneID` ref parsing (filename kept) so un-migrated projects still open. Unit tests `[schemas][EP-027]`. | ✅ Done (not verified). |
| T-0272 | **`util/OrderKey` scene filenames in `SceneCreator`/`ChapterCreator`/`ProjectCreator`** — new scenes get `<orderKey>-<slug>` filenames, filename-only refs, bare `contentPath`; empty `afterSceneID` still appends (contract preserved). | ✅ Done (not verified) — AC7. |
| T-0273 | **`manuscript/SceneIndex`** — `listScenesByOrder` (folder-scan, order-key sort, id from sidecar), `sceneMetaRelPath`/`chapterDirOf`, `rebuildChapterScenesIfInconsistent` (preserves missing refs), `migrateScenes` (orphan relocate + legacy reslug + cache rebuild). | ✅ Done (not verified) — AC7. |
| T-0274 | **`SceneReorderer` relocates files** — cross-chapter move = `renamePath` both files into the dest folder + between-neighbours order key + rebuild both caches; same-chapter = reslug/no-op fast path. | ✅ Done (not verified) — AC7. |
| T-0275 | **Simplify `renameChapterFolder`** — remove per-scene `metadataPath`/`contentPath` rewriting (filename-only refs need none); only the chapter `slug` still updates. Chapter rename → zero scene fields touched (B3/B4 fix), regression-tested. | ✅ Done (not verified). |
| T-0276 | **Scene consumers updated** — `SceneDeleter`, `SceneRenamer.findSceneMetadataPath`, `ManuscriptOrderResolver`, `ProjectValidator`, `ExternalChangeScanner`, `RepairHandlers` resolve identity from the sidecar + filename-only refs. RepairHandlers: scene-vs-chapter regen now keyed on filename; external-rename detection re-derived on orphan-in-same-dir with sceneID read from the orphan's sidecar. | ✅ Done (not verified) — §8.3. |
| T-0277 | **Scene migration + orphan repair wired into `ProjectOpener` (before validation)** + 5 integration tests `[EP-027][scenes]` (order-key filenames + filename-only refs; cross-chapter reorder relocates files; chapter rename leaves scene refs byte-identical; **C6 orphan relocated on open**; legacy numeric→order-key migration + idempotent). Full suite 298/298 macOS. | ✅ Done (not verified) — AC7 + AC8 (ctest). |

## Exit criteria (P6)

- **AC7:** order-key scene filenames + disk-authoritative scene identity/order; cross-chapter reorder relocates
  files; scene migration/orphan-repair on open. ✅ met (ctest).
- **C6 dissolved:** a project with a scene stranded in a foreign folder (ref moved, files not) **opens** — the
  orphan is relocated on open. ✅ met (test `open - relocates an orphaned scene … (C6)`).
- **B3/B4 mechanism removed:** a chapter-folder rename touches no scene field. ✅ met (test `chapter reorder -
  renaming a chapter folder leaves scene refs untouched`).
- `scrivi.h` unchanged; no regression — 298/298 macOS. **Linux-container ctest parity: pending** (run before P4).

## Next (EP-027)

- **P4 — Linux verify** (gate cleared 2026-07-18: Linux-container ctest 309/309). Remaining: re-run the **human VNC
  walkthrough** on the scene model (split B3/B4 + C6 orphan-repair-on-open should now pass). Then **P5 Apple verify**.
  Next available task **T-0278**.

---

# SP-069: [ScriviCore] EP-027 P1–P3 — Rename primitive + order-key/disk-authority + migration (chapters)

**Status:** 🟢 **Implemented — Not Verified (2026-07-16).** **P1** (rename primitive), **P2** (order-key
slugs + disk-authoritative chapter identity/order + open-time self-heal), and **P3** (old-format migration) all
landed this session. ScriviCore `ctest` green on **macOS (290/290)** and the **Linux container** (all EP-027
tests, 3225 assertions). Verification is ctest (core-first; no UI surface). **I-0072 root cause fixed +
regression + self-heal for existing damage + lazy migration of legacy projects.**
**Activated:** 2026-07-16
**Epic:** EP-027 `[ScriviCore]` — Filesystem-Authoritative Chapter/Scene Identity & Ordering (**Phases 1–3 of 6**).
**Codebase:** `[ScriviCore]` — the shared FileSystem port. Cross-platform (macOS + Linux verified at the
ctest level). `scrivi.h` untouched (this is an internal port capability, not a C ABI change).

**Goal (EP-027 AC5):** Add a **crash-safe `renamePath`/move primitive** to the FileSystem port — the capability
the order-key folder reslugging (P2) and the old→new migration (P3) both need, and which the port did not have.

## Design

- **Contract** (`Services.hpp` `FileSystem::renamePath(from, to)`):
  - **Atomic within a filesystem** — delegates to `std::filesystem::rename`, which the OS performs as a single
    operation that either fully succeeds or fully fails; a crash mid-rename never leaves a half-moved directory.
  - **Never clobbers** — refuses (`invalidArgument`) if `to` already exists, guaranteeing a new destination can
    never destroy existing content. This is the direct antidote to the I-0072 class (a slug collision
    overwriting a live chapter sidecar).
  - **Source must exist** — a missing `from` is a caller error (`invalidArgument`), not I/O noise.
  - **Cross-filesystem move** (`rename`'s `cross_device_link`) is reported as `ioError`, not silently turned
    into a non-atomic copy+delete. In-package chapter/scene moves are always same-filesystem, so this does not
    arise for the EP-027 use; a copy+delete fallback can be added later if a real cross-device case appears.
- **Only one implementer:** `LocalFileSystem` (verified — no test mock implements the port; tests use
  `LocalFileSystem` against temp dirs). So adding a pure-virtual method needed no mock updates.

## Tasks

| ID | Title | Status |
| -- | ----- | ------ |
| T-0264 | **P1: `FileSystem::renamePath` port method + `LocalFileSystem` impl** — pure-virtual on the port; no-clobber + missing-source guards + `std::filesystem::rename` (atomic-within-fs; `cross_device_link` → `ioError`). Unit tests `[renamePath][EP-027]` (5 cases). | ✅ Done (not verified) — AC5. |
| T-0265 | **P2: `util/OrderKey`** — fractional (LexoRank-style) base-62 order keys: `keyBetween/keyBefore/keyAfter/isOrderKey`. 8 property tests (3119 assertions) incl. 500 repeated same-gap inserts + 300 interleaved inserts stay strictly ordered. | ✅ Done (not verified). |
| T-0266 | **P2: `manuscript/ChapterIndex`** — disk-authoritative helpers: `listChaptersByOrder` (order by folder key, id from sidecar), `findChapterByID`, `resolveChapterID`, `rebuildIndexIfInconsistent` (open-time self-heal). Unit tests `[ChapterIndex]` (5 cases). | ✅ Done (not verified) — AC4. |
| T-0267 | **P2: `ChapterCreator` order-key slug** — new chapters named `chapter-<keyAfter(lastKey)>`, collision-free (**fixes I-0072**); `ProjectCreator` initial `chapter-001` kept (never collides). Integration regression `[I-0072]` (create→create→delete→create → distinct/ordered/no-clobber). | ✅ Done (not verified) — AC1. |
| T-0268 | **P2: disk-authoritative order + reorder** — `ManuscriptOrderResolver` iterates `listChaptersByOrder` (folder-key sort, B3); `ChapterReorderer` reworked to `keyBetween` + `renamePath` the one moved folder, rewriting the sidecar slug + all embedded scene `metadataPath`/`contentPath`. Existing reorder tests pass on the new mechanics. | ✅ Done (not verified) — AC2/AC3. |
| T-0269 | **P2: open-time index self-heal** — `rebuildIndexIfInconsistent` wired into `ProjectOpener`; an I-0072-corrupt index (phantom + duplicate) is rebuilt from disk on open, idempotent. Integration test `[I-0072]`. | ✅ Done (not verified) — AC4. |
| T-0270 | **P3: legacy-project migration** — `migrateChapterOrderKeys` (ChapterIndex): walks the `manuscript.meta.json` `chapters[]` array (legacy intended order), assigns fresh ascending order-keys via `keyAfter`, renames each out-of-position folder via the shared `renameChapterFolder` primitive (rewriting sidecar slug + embedded scene paths), then self-heals the index. No-op when folder-key sort already == index-array order. Wired into `ProjectOpener` (step 2a, before self-heal). Letter-prefixed generated keys sort after numeric legacy folders → collision-free dual-scheme read. Integration tests `[EP-027][migration]` (2 cases: reorder-legacy → correct order + scene bodies intact + idempotent; no-op when already ordered). | ✅ Done (not verified) — AC6. |

**P2 deferred (Human decision 2026-07-16):** drop `chapterID` from `ChapterRef` schema + migrate the 3 consumers
that read it — the index id is now a self-healing cache that can't diverge, so this is churn without functional
gain (trade study §7.6).

## Exit criteria (P1–P3)

- **AC5 (P1):** `renamePath` — atomic-within-fs, no-clobber, missing-source guard; unit-tested. ✅ met.
- **AC1 (P2):** order-key slugs; no collision after deletes (**I-0072 fixed**); regression test. ✅ met.
- **AC2 (P2):** reorder renames one folder via `renamePath`. ✅ met.
- **AC3 (P2):** order = disk folder-key sort; identity = sidecar (index id kept as self-healing cache). ✅ met.
- **AC4 (P2):** inconsistent index self-heals from disk on open (I-0072 damage repaired). ✅ met.
- **AC6 (P3):** legacy `chapter-NNN` projects migrate to order-key slugs at open (lazy/idempotent/resumable,
  dual-scheme read, no data loss); no-op when already ordered. ✅ met.
- `scrivi.h` unchanged; no regression — full suite **290/290** macOS + Linux.

## Next (EP-027)

- **P4 — Linux verify** (drive the migrated/self-healed behavior through the Qt UI on the Linux container; VNC
  walkthrough). Next available task **T-0271**.
- Then P5 Apple verify, P6 scenes.

---

*Activated 2026-07-16. EP-027 Phases 1–3 of 6 (`[ScriviCore]`). Delivers AC1–AC6 (rename primitive + order-key
slugs + disk-authoritative chapter order/identity + open-time self-heal + legacy-project migration; **I-0072
fixed**). Verification is ctest (core-first; no UI surface). `scrivi.h` untouched. Full suite 290/290 macOS +
Linux.*
