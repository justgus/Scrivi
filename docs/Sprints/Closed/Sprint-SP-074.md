## SP-074: [ScriviCore] Merge Endpoints + Filesystem-Coherence Fix

**Status:** ✅ **Closed (Human-approved 2026-07-21).**
**Epic:** EP-028 `[Cross]` Scene & Chapter Merging — Linux Parity & Filesystem-Coherence Fix (first of 3 sprints)
**Goal:** Add two first-class, atomic ScriviCore C ABI endpoints — `scrivi_merge_scene` (join a scene into
the previous scene in the same chapter) and `scrivi_merge_chapter` (relocate a whole chapter's scenes into
the previous chapter, then remove the emptied chapter) — and in doing so fix I-0083, the chapter-merge
data-loss bug the current Swift-composed merge has under EP-027. Both platforms adopt these in SP-075/076.
**Start Date:** 2026-07-20 | **End Date:** 2026-07-21 | **Capacity:** ~6–8 hours (actual: ~1 day)

### Assigned Tasks

| ID     | Title | Priority | Status |
| ------ | ----- | -------- | ------ |
| T-0298 | Reproduce chapter-merge data-loss (`MergeSceneTests.cpp`) + confirm same-chapter scene-merge coherence | High | 🟢 Implemented, Not Verified |
| T-0299 | `scrivi_merge_scene` — `SceneMerger`, request/result, facade, C ABI, `scrivi.h`, CMake | High | 🟢 Implemented, Not Verified |
| T-0300 | `scrivi_merge_chapter` — atomic cross-folder relocation + emptied-chapter removal (fixes I-0083) | High | 🟢 Implemented, Not Verified |
| T-0301 | Merge integration coverage + register in `tests/CMakeLists.txt`; `ctest` green macOS + Linux | High | 🟢 Implemented, Not Verified |

### Assigned Issues

| ID     | Title | Severity | Status |
| ------ | ----- | -------- | ------ |
| I-0083 | Chapter-merge loses scenes on reopen (in-memory reassign + `deleteChapter` deletes on-disk scene files under EP-027) | High | 🟢 Resolved (core), Not Verified |

### Implementation summary (2026-07-20 – 2026-07-21)

- **T-0298 — red repro.** `ScriviCore/tests/integration/MergeSceneTests.cpp`: the `[I-0083]` test replays
  the OLD buggy Swift-composed path (in-memory reassign + `scrivi_delete_chapter`) on a 2-chapter project
  and asserts the loss — reopen shows `size==1`, the chapter-2 folder gone, and scene 2's body deleted on
  disk. Retained as a regression guard on the buggy composition (not inverted). A companion test confirms
  the same-chapter scene-merge path was already coherent.
- **T-0299 — `scrivi_merge_scene(projectRootPath, sceneID)`.** New `SceneMerger`
  (`src/manuscript/SceneMerger.{hpp,cpp}`): same-chapter join into the PREVIOUS scene, located by
  filesystem-authoritative sceneID scan (EP-027 §8.1). The survivor keeps its own order-key files; the
  merged body is appended (blank-line separator, elided if either side is empty); the merged files are
  removed; the chapter's `scenes[]` cache is rebuilt. First-scene / empty / unknown sceneID →
  `invalidArgument`. Wired through `ScriviCore::mergeScene`, the `scrivi_merge_scene` C dispatcher, `scrivi.h`,
  and CMake.
- **T-0300 — `scrivi_merge_chapter(projectRootPath, chapterID)`. THE FIX FOR I-0083.** New `ChapterMerger`
  (`src/manuscript/ChapterMerger.{hpp,cpp}`): RELOCATES every scene file of the merged chapter into the
  predecessor's folder (`util::keyAfter` the predecessor's last scene, `.meta.json`+`.md` renamed and the
  sidecar slug/contentPath rewritten — exactly SceneReorderer's cross-chapter move), rebuilds the survivor
  cache, THEN removes the emptied chapter via `ChapterDeleter` (folder + `manuscript.meta.json` entry) +
  `rebuildIndexIfInconsistent`. **Files move BEFORE the folder is deleted — that ordering is the whole fix.**
  First-chapter → `invalidArgument`.
- **T-0301 — coverage + cross-platform green.** 11 tests in `MergeSceneTests.cpp` (same-chapter merge +
  empty-scene body elision; 4-scene/2-chapter whole-chapter merge with order + bodies preserved;
  manuscript-start / first-in-chapter no-op; empty/unknown ID errors; reopen round-trips via
  `ManuscriptOrderResolver`). Registered in `tests/CMakeLists.txt`.

### Verification (developer-level; user verification pending)

- **ctest macOS: 317/317.** **ctest Linux (Ubuntu 24.04 / GCC 13.3, Docker container): 324/324** (Linux
  has +7 `EncryptedFileSecureStore` tests). All 10 merge tests confirmed passing under real GCC.
- Both C symbols `_scrivi_merge_scene` and `_scrivi_merge_chapter` confirmed exported in `libScriviCore.a`
  (`nm`), so the Apple app (which links the prebuilt archive) can call them in SP-075.
- Boundary stayed pure C ABI / JSON-over-`std::string`; no struct interop; no pbxproj change (the Xcode app
  links the prebuilt `libScriviCore.a`, it does not compile ScriviCore `.cpp` files).

### Retrospective

**Completed:**
- T-0298, T-0299, T-0300, T-0301 — all 🟢 Implemented, Not Verified.
- I-0083 — 🟢 Resolved at the CORE level (the endpoint proves no scene loss on reopen).

**Returned to Backlog:**
- None. All sprint items delivered.

**What went well:**
- Modeling the new mergers on the existing `SceneDeleter` (location scan) and `SceneReorderer` (cross-chapter
  file move + sidecar rewrite) kept the new code small and idiomatic, and reused already-Linux-proven
  primitives — no new portability surface.
- The red-first discipline (T-0298 before any endpoint) gave a concrete, self-documenting statement of the
  bug that now doubles as a regression guard on the buggy Swift composition.

**What to improve / carry-forward:**
- **I-0083 is fixed at the core only.** The SHIPPING apps still compose chapter-merge from `deleteChapter`
  and remain lossy until they adopt `scrivi_merge_chapter` — **SP-075 (Apple)** then **SP-076 (Linux)**.
  Do not close EP-028 or mark I-0083 fully Verified until app adoption lands and is user-verified.
- **Stale devops file fixed:** `devops/docker/linux/Dockerfile` was missing `libssl-dev` (OpenSSL 3,
  required since SP-059's `EncryptedFileSecureStore`), so the local Linux parity harness
  (`scripts/test-ubuntu.sh`) could not `find_package(OpenSSL 3 REQUIRED)`. Added it to match the GitHub CI
  (`.github/workflows/scrivi-core-ci.yml`). Approved by the user. GitHub CI was unaffected (it installs
  libssl-dev via a separate apt step).

**Carry-forward notes for SP-075:**
- `scrivi_merge_scene` result JSON: `survivorSceneID`, `mergedSceneID`, `chapterID`, `survivorMetadataPath`,
  `survivorContentPath`, `chapterMetadataPath`, `merged`.
- `scrivi_merge_chapter` result JSON: `survivorChapterID`, `mergedChapterID`, `survivorChapterMetadataPath`,
  `scenesRelocated`, `merged`.
- Both ops relocate/rename files → any app consumer that captured paths must **refresh from the result**
  (the I-0081 stale-path contract). AC4/AC6 require the `sceneMerge`/`chapterMerge` history barriers.
