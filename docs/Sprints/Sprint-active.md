# Active Sprint

# SP-069: [ScriviCore] EP-027 P1+P2 — Rename primitive + order-key/disk-authority (chapters)

**Status:** 🟢 **Implemented — Not Verified (2026-07-16).** **P1** (rename primitive) and **P2** (order-key
slugs + disk-authoritative chapter identity/order + open-time self-heal) both landed this session. ScriviCore
`ctest` green on **macOS (288/288)** and the **Linux container** (all EP-027 tests, 3194 assertions). Verification
is ctest (core-first; no UI surface). **I-0072 root cause fixed + regression + self-heal for existing damage.**
**Activated:** 2026-07-16
**Epic:** EP-027 `[ScriviCore]` — Filesystem-Authoritative Chapter/Scene Identity & Ordering (**Phases 1–2 of 6**).
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

**P2 deferred (Human decision 2026-07-16):** drop `chapterID` from `ChapterRef` schema + migrate the 3 consumers
that read it — the index id is now a self-healing cache that can't diverge, so this is churn without functional
gain (trade study §7.6).

## Exit criteria (P1+P2)

- **AC5 (P1):** `renamePath` — atomic-within-fs, no-clobber, missing-source guard; unit-tested. ✅ met.
- **AC1 (P2):** order-key slugs; no collision after deletes (**I-0072 fixed**); regression test. ✅ met.
- **AC2 (P2):** reorder renames one folder via `renamePath`. ✅ met.
- **AC3 (P2):** order = disk folder-key sort; identity = sidecar (index id kept as self-healing cache). ✅ met.
- **AC4 (P2):** inconsistent index self-heals from disk on open (I-0072 damage repaired). ✅ met.
- `scrivi.h` unchanged; no regression — full suite **288/288** macOS + Linux.

## Next (EP-027)

- **P3 — Migration** (old-format `chapter-NNN` projects → order-key + disk-authority; lazy/idempotent/resumable;
  dual-scheme read). Next available task **T-0270**.
- Then P4 Linux verify, P5 Apple verify, P6 scenes.

---

*Activated 2026-07-16. EP-027 Phases 1–2 of 6 (`[ScriviCore]`). Delivers AC1–AC5 (rename primitive + order-key
slugs + disk-authoritative chapter order/identity + open-time self-heal; **I-0072 fixed**). Verification is ctest
(core-first; no UI surface). `scrivi.h` untouched. Full suite 288/288 macOS + Linux.*
