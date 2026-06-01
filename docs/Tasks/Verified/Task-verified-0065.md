---
id: T-0065
title: "clang-tidy — [[nodiscard]], const, and static member functions (21 warnings)"
status: ✅ Verified
sprint: SP-020
epic: EP-008
verified_date: 2026-06-01
---

## Description

Added `[[nodiscard]]` to 12 functions across 7 headers. Marked `import`, `list`, `remove`, `add`, `resolve`, `create`, `load`, and `save` as `const` on `AssetStore`, `CommentStore`, `ObjectStore`, `InboxStore`, and `WorkspaceStateService`. Promoted `droppedFilesDir` and `stateFilePath` from `const` instance methods to `static`. 21 warnings eliminated.

## Files Changed

- `ScriviCore/src/assets/AssetStore.hpp`
- `ScriviCore/src/assets/AssetStore.cpp`
- `ScriviCore/src/comments/CommentStore.hpp`
- `ScriviCore/src/comments/CommentStore.cpp`
- `ScriviCore/src/git/SnapshotService.hpp`
- `ScriviCore/src/identity/IdentityService.hpp`
- `ScriviCore/src/inbox/InboxStore.hpp`
- `ScriviCore/src/inbox/InboxStore.cpp`
- `ScriviCore/src/objects/ObjectStore.hpp`
- `ScriviCore/src/objects/ObjectStore.cpp`
- `ScriviCore/src/project_package/ProjectCreator.hpp`
- `ScriviCore/src/project_package/ProjectValidator.hpp`
- `ScriviCore/src/workspace/WorkspaceStateService.hpp`
- `ScriviCore/src/workspace/WorkspaceStateService.cpp`

## Verification

- `cmake --build build/ninja --parallel` — clean
- `ctest --test-dir build/ninja --output-on-failure` — 165/165 passed
- `bash scripts/verify-scrivi-core-tidy.sh` — zero warnings in these categories
