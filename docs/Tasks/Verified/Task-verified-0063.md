---
id: T-0063
title: "clang-tidy — readability-braces-around-statements (142 warnings)"
status: ✅ Verified
sprint: SP-020
epic: EP-008
verified_date: 2026-06-01
---

## Description

Added `{ }` braces around all single-statement `if`, `else`, `for`, and `while` bodies across all 42 ScriviCore source files. 142 `readability-braces-around-statements` warnings eliminated.

## Files Changed

- `ScriviCore/src/schemas/SchemaUtils.hpp`
- `ScriviCore/src/schemas/RepairIssueJson.cpp`
- `ScriviCore/src/schemas/ObjectJson.cpp`
- `ScriviCore/src/schemas/CommentJson.cpp`
- `ScriviCore/src/schemas/AssetMetaJson.cpp`
- `ScriviCore/src/schemas/ChapterMetaJson.cpp`
- `ScriviCore/src/schemas/ManuscriptMetaJson.cpp`
- `ScriviCore/src/schemas/ProjectJson.cpp`
- `ScriviCore/src/schemas/ProjectMembersJson.cpp`
- `ScriviCore/src/schemas/ProjectPersonasJson.cpp`
- `ScriviCore/src/schemas/SceneMetaJson.cpp`
- `ScriviCore/src/schemas/SnapshotMetadataJson.cpp`
- `ScriviCore/src/schemas/WorkspaceStateJson.cpp`
- `ScriviCore/src/inbox/InboxStore.cpp`
- `ScriviCore/src/repair/RepairHandlers.cpp`
- `ScriviCore/src/util/AtomicWrite.cpp`
- `ScriviCore/src/util/Hash.cpp`
- `ScriviCore/src/util/Json.cpp`
- `ScriviCore/src/workspace/WorkspaceStateService.cpp`

## Verification

- `cmake --build build/ninja --parallel` — clean
- `ctest --test-dir build/ninja --output-on-failure` — 165/165 passed
- `bash scripts/verify-scrivi-core-tidy.sh` — zero `readability-braces-around-statements` warnings
