---
id: T-0064
title: "clang-tidy — modernize-use-designated-initializers + readability-qualified-auto (51 warnings)"
status: ✅ Verified
sprint: SP-020
epic: EP-008
verified_date: 2026-06-01
---

## Description

Replaced all positional `Error{}`, `TextSelection{}`, and `SceneRef{}` struct initializations with designated initializer syntax (`.field = value`). Added `const` and `*` qualifiers to all `auto` loop variables and `std::get_if` results as directed by `readability-qualified-auto`. 51 warnings eliminated.

## Files Changed

- `ScriviCore/src/schemas/SchemaUtils.hpp`
- `ScriviCore/src/schemas/ChapterMetaJson.cpp`
- `ScriviCore/src/schemas/ManuscriptMetaJson.cpp`
- `ScriviCore/src/schemas/ObjectJson.cpp`
- `ScriviCore/src/schemas/ProjectJson.cpp`
- `ScriviCore/src/schemas/ProjectMembersJson.cpp`
- `ScriviCore/src/schemas/ProjectPersonasJson.cpp`
- `ScriviCore/src/schemas/RepairIssueJson.cpp`
- `ScriviCore/src/schemas/SceneMetaJson.cpp`
- `ScriviCore/src/schemas/SnapshotMetadataJson.cpp`
- `ScriviCore/src/schemas/WorkspaceStateJson.cpp`
- `ScriviCore/src/inbox/InboxStore.cpp`
- `ScriviCore/src/repair/RepairHandlers.cpp`
- `ScriviCore/src/util/AtomicWrite.cpp`
- `ScriviCore/src/util/Json.cpp`
- `ScriviCore/src/util/Process.cpp`
- `ScriviCore/src/workspace/WorkspaceStateService.cpp`

## Verification

- `cmake --build build/ninja --parallel` — clean
- `ctest --test-dir build/ninja --output-on-failure` — 165/165 passed
- `bash scripts/verify-scrivi-core-tidy.sh` — zero warnings in these categories
