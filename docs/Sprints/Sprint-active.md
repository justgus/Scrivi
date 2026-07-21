# Active Sprint

## SP-074: [ScriviCore] Merge Endpoints + Filesystem-Coherence Fix

**Status:** 🟡 Active
**Epic:** EP-028: [Cross] Scene & Chapter Merging — Linux Parity & Filesystem-Coherence Fix
**Goal:** Add two first-class, atomic ScriviCore C ABI endpoints — `scrivi_merge_scene` (join a scene into
the previous scene in the same chapter) and `scrivi_merge_chapter` (relocate a whole chapter's scenes into
the previous chapter, then remove the emptied chapter) — and in doing so fix I-0083, the chapter-merge
data-loss bug the current Swift-composed merge has under EP-027. Both platforms will call these in SP-075/076.
**Start Date:** 2026-07-20
**End Date:** —
**Capacity:** ~6–8 hours

### Assigned Tasks

| ID     | Title | Priority | Status |
| ------ | ----- | -------- | ------ |
| T-0298 | Reproduce chapter-merge data-loss (`MergeSceneTests.cpp`) + confirm same-chapter scene-merge coherence | High | 🟢 Implemented, Not Verified |
| T-0299 | `scrivi_merge_scene` — `SceneMerger`, request/result, facade, C ABI, `scrivi.h`, CMake + pbxproj | High | 🟢 Implemented, Not Verified |
| T-0300 | `scrivi_merge_chapter` — atomic cross-folder relocation + emptied-chapter removal (fixes I-0083) | High | 🟢 Implemented, Not Verified |
| T-0301 | Merge integration coverage + register in `tests/CMakeLists.txt`; `ctest` green macOS + Linux | High | 🟢 Implemented, Not Verified |

### Assigned Issues

| ID     | Title | Severity | Status |
| ------ | ----- | -------- | ------ |
| I-0083 | Chapter-merge loses scenes on reopen (in-memory reassign + `deleteChapter` deletes on-disk scene files under EP-027) | High | 🟢 Resolved, Not Verified |

### Sprint Notes

- **T-0298 comes first** — reproduce the bug as a red integration test before writing any endpoint, so we
  can prove the fix. Model on `ScriviCore/tests/integration/DeleteSceneTests.cpp` / `ReorderTests.cpp`:
  build a project in a `TempDir`, mutate, assert with `ManuscriptOrderResolver::resolve`.
- `scrivi_merge_scene` reuses the survivor's order key (no new key minted); same-chapter only. Model the
  file plumbing on `SceneDeleter` (`src/manuscript/SceneDeleter.cpp`).
- `scrivi_merge_chapter` mints order keys after the predecessor's last scene and renames/relocates files
  like `SceneReorderer` (`src/manuscript/SceneReorderer.cpp:35`), rewriting each moved sidecar's
  slug/contentPath, then removes the emptied chapter folder + its `manuscript.meta.json` entry.
- **pbxproj:** new `SceneMerger.cpp/.hpp` (and any new files) MUST be added to
  `Scrivi.xcodeproj/project.pbxproj` in the same step (CLAUDE.md — user commits from the command line).
- Boundary stays pure C ABI / JSON-over-`std::string`. Reuse the reserved `sceneMerge` history-barrier
  vocabulary (`src/history/HistoryService.hpp:100`).

### Retrospective

_(filled in at close)_

---

*Last Updated: 2026-07-21 (SP-074 all four tasks 🟢 Implemented, Not Verified — T-0298 red repro,
T-0299 `scrivi_merge_scene`, T-0300 `scrivi_merge_chapter` (fixes I-0083 at core), T-0301 coverage +
cross-platform green: **macOS 317/317, Linux 324/324**. Both merge C symbols exported in `libScriviCore.a`.
Awaiting user verification, then sprint close. App adoption of `scrivi_merge_chapter` = SP-075 (Apple) /
SP-076 (Linux). Next available sprint **SP-075**; next task **T-0308**; next issue **I-0084**.)*
