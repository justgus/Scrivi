# SP-070: [ScriviCore] EP-027 P6 — Filesystem-authoritative scene identity & ordering (scenes)

**Status:** ✅ **VERIFIED — closed (Human-approved 2026-07-18).** B3/B4 split + C6 orphan-open + I-0071 last-scene
drag all pass on the P6 model (Human-driven VNC walkthrough 2026-07-18); Linux-container ctest 309/309. AC7 +
AC8-Linux Verified; I-0064/I-0069/I-0070/I-0071 Verified. Applies the §7 chapter model one level down to **scenes**
(trade study **§8**, Human-approved 2026-07-17). This is **P6 pulled forward before P4**, because the P4 Linux VNC
walkthrough (2026-07-16) failed B3/B4 (chapter-split) and C6 ("Missing scene.meta.json") — all root-caused to the
pre-EP-027 `SceneReorderer` moving a `SceneRef` between chapter sidecars **without relocating the scene's files**,
which chapter-folder renames then orphaned. ScriviCore `ctest` **298/298 macOS** (293 pre-existing, all still green +
5 new §8 tests). Linux-container ctest parity: ✅ **CONFIRMED 2026-07-18** — fresh Ubuntu-24.04 container
(`scrivi-linux`), `-DSCRIVI_BUILD_TESTS=ON`, clean configure + build, **`ctest` 309/309, 0 failed**.
**Activated:** 2026-07-17
**Closed:** 2026-07-18
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
| T-0271 | **`SceneRef` filename-only schema + serde** — `{metadataFilename}` (drops `sceneID`+path); dual-scheme read keeps a legacy `metadataPath`/`sceneID` ref parsing (filename kept) so un-migrated projects still open. Unit tests `[schemas][EP-027]`. | ✅ Verified. |
| T-0272 | **`util/OrderKey` scene filenames in `SceneCreator`/`ChapterCreator`/`ProjectCreator`** — new scenes get `<orderKey>-<slug>` filenames, filename-only refs, bare `contentPath`; empty `afterSceneID` still appends (contract preserved). | ✅ Verified — AC7. |
| T-0273 | **`manuscript/SceneIndex`** — `listScenesByOrder` (folder-scan, order-key sort, id from sidecar), `sceneMetaRelPath`/`chapterDirOf`, `rebuildChapterScenesIfInconsistent` (preserves missing refs), `migrateScenes` (orphan relocate + legacy reslug + cache rebuild). | ✅ Verified — AC7. |
| T-0274 | **`SceneReorderer` relocates files** — cross-chapter move = `renamePath` both files into the dest folder + between-neighbours order key + rebuild both caches; same-chapter = reslug/no-op fast path. | ✅ Verified — AC7. |
| T-0275 | **Simplify `renameChapterFolder`** — remove per-scene `metadataPath`/`contentPath` rewriting (filename-only refs need none); only the chapter `slug` still updates. Chapter rename → zero scene fields touched (B3/B4 fix), regression-tested. | ✅ Verified. |
| T-0276 | **Scene consumers updated** — `SceneDeleter`, `SceneRenamer.findSceneMetadataPath`, `ManuscriptOrderResolver`, `ProjectValidator`, `ExternalChangeScanner`, `RepairHandlers` resolve identity from the sidecar + filename-only refs. | ✅ Verified — §8.3. |
| T-0277 | **Scene migration + orphan repair wired into `ProjectOpener` (before validation)** + 5 integration tests `[EP-027][scenes]`. Full suite 298/298 macOS. | ✅ Verified — AC7 + AC8 (ctest). |

## Exit criteria (P6) — all met

- **AC7:** order-key scene filenames + disk-authoritative scene identity/order; cross-chapter reorder relocates
  files; scene migration/orphan-repair on open. ✅ Verified (ctest + VNC).
- **C6 dissolved:** a project with a scene stranded in a foreign folder **opens** — orphan relocated on open. ✅.
- **B3/B4 mechanism removed:** a chapter-folder rename touches no scene field. ✅.
- `scrivi.h` unchanged; no regression — 298/298 macOS, 309/309 Linux container.

*P4 followed (VNC walkthrough Verified 2026-07-18); P5 Apple-verify surfaced + fixed I-0076/I-0077/I-0078 (SP-072).*
