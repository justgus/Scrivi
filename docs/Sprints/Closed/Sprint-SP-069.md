# SP-069: [ScriviCore] EP-027 P1–P3 — Rename primitive + order-key/disk-authority + migration (chapters)

**Status:** ✅ **VERIFIED — closed (Human-approved 2026-07-18).** **P1** (rename primitive), **P2** (order-key
slugs + disk-authoritative chapter identity/order + open-time self-heal), and **P3** (old-format migration) all
landed. ScriviCore `ctest` green on **macOS (290/290)** and the **Linux container** (all EP-027 tests, 3225
assertions). **I-0072 root cause fixed + regression + self-heal for existing damage + lazy migration of legacy
projects.** The chapter-level model was subsequently exercised end-to-end via P4 (VNC) and P5 (real legacy project
open on macOS + Linux, SP-072) — Verified 2026-07-18.
**Activated:** 2026-07-16
**Closed:** 2026-07-18
**Epic:** EP-027 `[ScriviCore]` — Filesystem-Authoritative Chapter/Scene Identity & Ordering (**Phases 1–3 of 6**).
**Codebase:** `[ScriviCore]` — the shared FileSystem port. `scrivi.h` untouched (internal port capability).

**Goal (EP-027 AC5):** Add a **crash-safe `renamePath`/move primitive** to the FileSystem port — the capability
the order-key folder reslugging (P2) and the old→new migration (P3) both need, and which the port did not have.

## Design

- **Contract** (`Services.hpp` `FileSystem::renamePath(from, to)`):
  - **Atomic within a filesystem** — delegates to `std::filesystem::rename`; a crash mid-rename never leaves a
    half-moved directory.
  - **Never clobbers** — refuses (`invalidArgument`) if `to` already exists (direct antidote to the I-0072 class —
    a slug collision overwriting a live chapter sidecar).
  - **Source must exist** — a missing `from` is a caller error (`invalidArgument`).
  - **Cross-filesystem move** is reported as `ioError`, not silently turned into a non-atomic copy+delete.
- **Only one implementer:** `LocalFileSystem` (tests use it against temp dirs) — adding a pure-virtual needed no
  mock updates.

## Tasks

| ID | Title | Status |
| -- | ----- | ------ |
| T-0264 | **P1: `FileSystem::renamePath` port method + `LocalFileSystem` impl** — no-clobber + missing-source guards + `std::filesystem::rename` (atomic-within-fs; `cross_device_link` → `ioError`). Unit tests `[renamePath][EP-027]` (5 cases). | ✅ Verified — AC5. |
| T-0265 | **P2: `util/OrderKey`** — fractional (LexoRank-style) base-62 order keys: `keyBetween/keyBefore/keyAfter/isOrderKey`. 8 property tests (3119 assertions). | ✅ Verified. |
| T-0266 | **P2: `manuscript/ChapterIndex`** — disk-authoritative helpers + `rebuildIndexIfInconsistent` (open-time self-heal). Unit tests `[ChapterIndex]` (5 cases). | ✅ Verified — AC4. |
| T-0267 | **P2: `ChapterCreator` order-key slug** — collision-free (**fixes I-0072**); `ProjectCreator` initial `chapter-001` kept. Integration regression `[I-0072]`. | ✅ Verified — AC1. |
| T-0268 | **P2: disk-authoritative order + reorder** — `ManuscriptOrderResolver` folder-key sort (B3); `ChapterReorderer` = `keyBetween` + `renamePath` the one moved folder. | ✅ Verified — AC2/AC3. |
| T-0269 | **P2: open-time index self-heal** — `rebuildIndexIfInconsistent` wired into `ProjectOpener`; I-0072-corrupt index rebuilt from disk, idempotent. Integration test `[I-0072]`. | ✅ Verified — AC4. |
| T-0270 | **P3: legacy-project migration** — `migrateChapterOrderKeys` assigns fresh ascending order-keys in index-array order via `renameChapterFolder`, then self-heals. No-op when already ordered. Integration tests `[EP-027][migration]`. | ✅ Verified — AC6. |

**P2 deferred (Human decision 2026-07-16):** drop `chapterID` from `ChapterRef` schema — the index id is now a
self-healing cache that can't diverge, so this is churn without functional gain (trade study §7.6).

## Exit criteria (P1–P3) — all met

- **AC5 (P1):** `renamePath` — atomic-within-fs, no-clobber, missing-source guard; unit-tested. ✅.
- **AC1 (P2):** order-key slugs; no collision after deletes (**I-0072 fixed**); regression test. ✅.
- **AC2 (P2):** reorder renames one folder via `renamePath`. ✅.
- **AC3 (P2):** order = disk folder-key sort; identity = sidecar. ✅.
- **AC4 (P2):** inconsistent index self-heals from disk on open. ✅.
- **AC6 (P3):** legacy `chapter-NNN` projects migrate at open (lazy/idempotent/resumable, dual-scheme, no loss). ✅.
- `scrivi.h` unchanged; no regression — full suite **290/290** macOS + Linux.
