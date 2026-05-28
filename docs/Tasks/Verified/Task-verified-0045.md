---
id: T-0045
title: Integration Tests for Comment Operations
status: ✅ Verified
epic: EP-005
sprint: SP-012
priority: High
date_requested: 2026-05-28
date_verified: 2026-05-28
---

# T-0045: Integration Tests for Comment Operations

**Status:** ✅ Verified
**Component:** ScriviCore — tests/integration
**Priority:** High
**Sprint Assigned:** SP-012

## Requirements

1. `addComment` test: thread file created with comment.
2. `listComments` test: multiple comments returned.
3. `resolveComment` test: `resolved == true`, `resolvedAt` populated.
4. Tests cover both scene and object scopes.
5. `tests/CMakeLists.txt` updated.

## Implementation

- `ScriviCore/tests/integration/CommentTests.cpp` (new) — `CommentFixture` struct with temp project dir; 7 test cases:
  - `addComment creates thread file for scene scope`
  - `listComments returns all comments for scene scope`
  - `resolveComment sets resolved flag and resolvedAt for scene scope`
  - `addComment creates thread file for object scope`
  - `listComments returns all comments for object scope`
  - `resolveComment sets resolved flag for object scope`
  - `scene and object scopes write to separate thread files`
- `ScriviCore/tests/CMakeLists.txt` — added `integration/CommentTests.cpp`.
- `ScriviCore.xcodeproj/project.pbxproj` — E021 (`CommentTests.cpp`) added to `GRP_INTEGRATION`.

## Verification

- `cmake --build build --parallel` — passed, no errors.
- `ctest --test-dir build --output-on-failure` — 152/152 passed.
