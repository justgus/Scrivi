---
id: T-0042
title: Integration Tests for Asset Operations
status: ✅ Verified
epic: EP-005
sprint: SP-012
priority: High
date_requested: 2026-05-28
date_verified: 2026-05-28
---

# T-0042: Integration Tests for Asset Operations

**Status:** ✅ Verified
**Component:** ScriviCore — tests/integration
**Priority:** High
**Sprint Assigned:** SP-012

## Requirements

1. `importAsset` test: file + sidecar written to `assets/<category>/`.
2. `listAssets` test: import two assets, list returns both.
3. `removeAsset` test: both files deleted.
4. `tests/CMakeLists.txt` updated.

## Implementation

- `ScriviCore/tests/integration/AssetTests.cpp` (new) — `AssetFixture` struct with temp project dir; 4 test cases:
  - `importAsset writes binary file and sidecar to assets/<category>/`
  - `listAssets returns all imported assets`
  - `listAssets with category filter returns only matching assets`
  - `removeAsset deletes binary file and sidecar`
- `ScriviCore/tests/CMakeLists.txt` — added `integration/AssetTests.cpp`.
- `ScriviCore.xcodeproj/project.pbxproj` — E020 (`AssetTests.cpp`) added to `GRP_INTEGRATION`.

## Verification

- `cmake --build build --parallel` — passed, no errors.
- `ctest --test-dir build --output-on-failure` — 152/152 passed.
