---
id: T-0040
title: Asset Metadata Sidecar Schema (`AssetMetaJson`)
status: ✅ Verified
epic: EP-005
sprint: SP-012
priority: High
date_requested: 2026-05-28
date_verified: 2026-05-28
---

# T-0040: Asset Metadata Sidecar Schema (`AssetMetaJson`)

**Status:** ✅ Verified
**Component:** ScriviCore — schemas, Types
**Priority:** High
**Sprint Assigned:** SP-012

## Requirements

1. `AssetMeta` struct: `assetID`, `slug`, `filename`, `category`, `mimeType`, `importedAt`, authorship fields, `title`, `notes`, `tags`.
2. `AssetMetaJson.hpp` / `.cpp` with `serializeAssetMeta` / `parseAssetMeta`. Schema key `"scrivi.asset.meta.v1"`.
3. Unit tests cover round-trip.
4. `ScriviCore.xcodeproj/project.pbxproj` and `tests/CMakeLists.txt` updated for all new files.

## Implementation

- `ScriviCore/include/scrivi/AssetTypes.hpp` (new) — `AssetCategory` enum + `assetCategorySubdir` / `assetCategoryString` / `assetCategoryFromString` helpers; `AssetMeta` struct.
- `ScriviCore/src/schemas/AssetMetaJson.hpp` (new) — `serializeAssetMeta` / `parseAssetMeta` declarations.
- `ScriviCore/src/schemas/AssetMetaJson.cpp` (new) — implementation; schema key `"scrivi.asset.meta.v1"`.
- `ScriviCore/tests/unit/JsonSchemaTests.cpp` — added 4 unit tests: round-trip minimal, round-trip all categories with tags, wrong schema tag rejection, corrupt JSON rejection.
- `ScriviCore.xcodeproj/project.pbxproj` — E010 (`AssetTypes.hpp`), E012 (`AssetMetaJson.hpp`), E013 (`AssetMetaJson.cpp`) added.

## Verification

- `cmake --build build --parallel` — passed, no errors.
- `ctest --test-dir build --output-on-failure` — 152/152 passed.
