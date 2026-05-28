---
id: T-0043
title: Comments Schema (`CommentJson`) and Comment Types
status: ✅ Verified
epic: EP-005
sprint: SP-012
priority: High
date_requested: 2026-05-28
date_verified: 2026-05-28
---

# T-0043: Comments Schema (`CommentJson`) and Comment Types

**Status:** ✅ Verified
**Component:** ScriviCore — schemas, Types
**Priority:** High
**Sprint Assigned:** SP-012

## Requirements

1. `Comment` struct: `commentID`, `body`, `resolved`, `createdAt`, authorship, `resolvedAt` (optional), `resolvedByIdentityID` / `resolvedByPersonaID` / `resolvedByDisplayName` (optional).
2. `CommentThread` struct: `schema`, `scopeKind`, `targetID`, `comments` vector.
3. `CommentJson.hpp` / `.cpp` with `serializeCommentThread` / `parseCommentThread`. Schema key `"scrivi.comments.v1"`.
4. Unit tests cover round-trip including resolved comments.
5. `ScriviCore.xcodeproj/project.pbxproj` updated.

## Implementation

- `ScriviCore/include/scrivi/CommentTypes.hpp` (new) — `Comment` struct (with `std::optional` resolved fields) and `CommentThread` struct.
- `ScriviCore/src/schemas/CommentJson.hpp` (new) — `serializeCommentThread` / `parseCommentThread` declarations.
- `ScriviCore/src/schemas/CommentJson.cpp` (new) — implementation; schema key `"scrivi.comments.v1"`; optional fields stored as empty strings on disk and restored as `std::nullopt` when empty.
- `ScriviCore/tests/unit/JsonSchemaTests.cpp` — added 5 unit tests: empty thread, unresolved comment, resolved comment with all optional fields, wrong schema tag, corrupt JSON.
- `ScriviCore.xcodeproj/project.pbxproj` — E011 (`CommentTypes.hpp`), E014 (`CommentJson.hpp`), E015 (`CommentJson.cpp`) added.

## Verification

- `cmake --build build --parallel` — passed, no errors.
- `ctest --test-dir build --output-on-failure` — 152/152 passed.
