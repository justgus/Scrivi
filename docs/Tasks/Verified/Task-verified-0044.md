---
id: T-0044
title: "`addComment` / `listComments` / `resolveComment` Facade Methods"
status: ✅ Verified
epic: EP-005
sprint: SP-012
priority: High
date_requested: 2026-05-28
date_verified: 2026-05-28
---

# T-0044: `addComment` / `listComments` / `resolveComment` Facade Methods

**Status:** ✅ Verified
**Component:** ScriviCore — Requests.hpp, Results.hpp, CommentStore, ScriviCore facade
**Priority:** High
**Sprint Assigned:** SP-012

## Requirements

1. `AddCommentRequest` / `AddCommentResult`, `ListCommentsRequest` / `ListCommentsResult`, `ResolveCommentRequest` / `ResolveCommentResult`.
2. `CommentStore` in `src/comments/`: atomic read-modify-write of `comments/<scopeKind>/<targetID>.comments.json`.
3. Three new facade methods on `ScriviCore`.
4. `ScriviCore.xcodeproj/project.pbxproj` updated.

## Implementation

- `ScriviCore/include/scrivi/Requests.hpp` — added `AddCommentRequest`, `ListCommentsRequest`, `ResolveCommentRequest`; added `#include "scrivi/CommentTypes.hpp"`.
- `ScriviCore/include/scrivi/Results.hpp` — added `AddCommentResult`, `ListCommentsResult`, `ResolveCommentResult`; added `#include "scrivi/CommentTypes.hpp"`.
- `ScriviCore/src/comments/CommentStore.hpp` (new) — `CommentStore` class with `add` / `list` / `resolve`.
- `ScriviCore/src/comments/CommentStore.cpp` (new) — `add` loads or creates thread, appends comment, writes atomically; `list` returns thread comments; `resolve` finds comment by ID, sets `resolved=true` + `resolvedAt` + resolver fields, writes atomically. Thread path: `comments/<scopeKind>/<targetID>.comments.json`.
- `ScriviCore/include/scrivi/ScriviCore.hpp` — added `addComment`, `listComments`, `resolveComment` declarations.
- `ScriviCore/src/public_api/ScriviCore.cpp` — added three facade implementations delegating to `CommentStore`.
- `ScriviCore/CMakeLists.txt` — added `src/comments/CommentStore.cpp`.
- `ScriviCore.xcodeproj/project.pbxproj` — E018 (`CommentStore.hpp`), E019 (`CommentStore.cpp`) added; `GRP_COMMENTS` group created in `GRP_SRC`.

## Verification

- `cmake --build build --parallel` — passed, no errors.
- `ctest --test-dir build --output-on-failure` — 152/152 passed.
