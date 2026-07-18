# SP-072: [ScriviCore]+[Apple] EP-027 P5 Apple-verify — legacy migration fixes (I-0076/I-0077/I-0078)

**Status:** ✅ **VERIFIED — closed (Human-approved 2026-07-18).** Surfaced during **P5 (Apple verify)** when the
rebuilt macOS app opened a **real legacy project** (`the-twisted-remains-of-myself.scrivi`: 14 chapters, numeric
`chapter-NNN` folders with gaps, legacy scene refs, one index/sidecar `chapterID` mismatch on `chapter-004`). The
first open ran migration and then **refused to open — "Repair required: Missing scene content file."** Three defects
diagnosed and fixed; the fixes were then confirmed by the Human opening a **fresh copy** in the rebuilt app — it
**opened cleanly, all chapters + content intact.**
**Activated:** 2026-07-18
**Closed:** 2026-07-18
**Epic:** EP-027 `[ScriviCore]` — Filesystem-Authoritative Chapter/Scene Identity & Ordering (**Phase 5 of 6, Apple
verify**). `scrivi.h` **unchanged** (the migration is on-disk behavior; the Swift change is a caller catch-up).

## Defects fixed

| ID | Sev | Root cause | Fix |
| -- | --- | ---------- | --- |
| **I-0078** `[Apple]` | High | `ScriviEngine.createChapter` still called `scrivi_create_chapter` with 6 args after SP-071 added a 7th (`afterChapterID`); macOS hadn't compiled since c949d0b. | Thread `afterChapterID: String = ""` (empty ⇒ append, preserving prior macOS behavior) through the wrapper via `withCString` (`Scrivi/Engine/ScriviEngine.swift`). Both macOS callers only append. |
| **I-0076** `[ScriviCore]` | High | A legacy scene sidecar's `content.path = "manuscript/chapter-NNN/…md"` was never normalised to a bare filename on migration, so after the chapter folder reslug it dangled → validator "Missing scene content file". `util::isOrderKey("001"/…)` returns true (base-62 accepts digits; only trailing `'0'` rejected), so `migrateScenes` skipped those stems; `renameChapterFolder` by design touches no scene fields (assumes bare paths). | (a) `parseSceneMeta` bares `content.path` on read (`SceneMetaJson.cpp`) → every consumer resolves it in the scene's own folder → **project opens**. (b) `migrateScenes` gains `normalizeSceneContentPathIfStale` — rewrites a canonical-key sidecar whose stored path is still full, idempotently (`SceneIndex.cpp`). `isOrderKey` left unchanged (digit-acceptance is correct for the key space). |
| **I-0077** `[ScriviCore]` | Med | A chapter whose index `chapterID` disagrees with its sidecar (`chapter-004`: index `…942e…` vs sidecar `…9cdd…`) had `migrateChapterOrderKeys::keyForID` fail to map it → dropped as a "phantom" → left half-migrated (numeric slug, old-scheme refs, mis-sorted). | `keyForRef` resolves each index ref by chapterID **then falls back to the ref's `path` folder** when the id doesn't match a sidecar (`ChapterIndex.cpp`); a mismatched-but-present chapter now participates in the reslug. Only a ref with neither an id match nor an existing folder is skipped. |

## Tasks

| ID | Title | Status |
| -- | ----- | ------ |
| T-0290 | I-0078 — macOS `createChapter` wrapper gains `afterChapterID` (append default) | ✅ Verified |
| T-0291 | I-0076 — `parseSceneMeta` bares `content.path`; `migrateScenes` normalises stale sidecars | ✅ Verified |
| T-0292 | I-0077 — `migrateChapterOrderKeys` path-folder fallback on id mismatch | ✅ Verified |
| T-0293 | 2 regression tests in `SceneIdentityTests.cpp` (I-0076 full-path normalise; I-0077 mismatch migrates), each verified RED-without-fix | ✅ Verified |

## Verification

- **macOS ctest 304/304**, **Linux-container ctest 311/311** (both incl. the 2 new regression tests).
- Each new test **proven to catch its bug** by reverting the corresponding fix and confirming it goes RED (the first
  I-0077 test was too weak — it passed without the fix because `isOrderKey("001")` is true — strengthened to assert
  the `chapter-001` folder is gone, then re-proved RED).
- **macOS app BUILD SUCCEEDED**; Human opened a **fresh copy** of the real legacy project → opened cleanly, all 14
  chapters + content present (user-observed passing → Verified).
- Linux VNC open of the same real legacy project (`/projects/twisted-legacy-vnc.scrivi`) — **confirmed clean
  (2026-07-18)**: opened without error; migration produced 14 order-key chapter folders (no stranded numeric folder),
  **0** stale scene `content.path` (was 18 pre-fix), **21/21** scene bodies preserved (the one empty `.md` was empty
  in the pristine backup too). Cross-platform AC8 fully covered.
- `scrivi.h` unchanged; no regression (full suite green both platforms).

## Notes

- The **original** on-disk `the-twisted-remains-of-myself.scrivi` was left half-migrated by the FIRST (pre-fix) open;
  the pristine **backup copy** was used for the re-verify. Not a code issue.
- Closes the P5 gap. **EP-027 AC7 + AC8 now Verified**; only the Human's Epic-close approval remains.
