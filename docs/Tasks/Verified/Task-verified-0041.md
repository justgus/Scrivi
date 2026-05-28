---
id: T-0041
title: "`importAsset` / `listAssets` / `removeAsset` Facade Methods"
status: ✅ Verified
epic: EP-005
sprint: SP-012
priority: High
date_requested: 2026-05-28
date_verified: 2026-05-28
---

# T-0041: `importAsset` / `listAssets` / `removeAsset` Facade Methods

**Status:** ✅ Verified
**Component:** ScriviCore — Requests.hpp, Results.hpp, AssetStore, ScriviCore facade
**Priority:** High
**Sprint Assigned:** SP-012

## Requirements

1. `ImportAssetRequest` / `ImportAssetResult`, `ListAssetsRequest` / `ListAssetsResult`, `RemoveAssetRequest` / `RemoveAssetResult`.
2. `AssetStore` in `src/assets/`: copies source file to `assets/<category>/`, writes sidecar, enumerates via `.meta.json` files, deletes asset + sidecar.
3. Three new facade methods on `ScriviCore`.
4. `ScriviCore.xcodeproj/project.pbxproj` updated.

## Implementation

- `ScriviCore/include/scrivi/Requests.hpp` — added `ImportAssetRequest`, `ListAssetsRequest`, `RemoveAssetRequest`; added `#include "scrivi/AssetTypes.hpp"`.
- `ScriviCore/include/scrivi/Results.hpp` — added `ImportAssetResult`, `ListAssetsResult`, `RemoveAssetResult`; added `#include "scrivi/AssetTypes.hpp"`.
- `ScriviCore/src/assets/AssetStore.hpp` (new) — `AssetStore` class with `import` / `list` / `remove`.
- `ScriviCore/src/assets/AssetStore.cpp` (new) — `import` copies source bytes and writes sidecar; `list` scans `.meta.json` files in category directories; `remove` finds sidecar by assetID and deletes both files.
- `ScriviCore/include/scrivi/ScriviCore.hpp` — added `importAsset`, `listAssets`, `removeAsset` declarations.
- `ScriviCore/src/public_api/ScriviCore.cpp` — added three facade implementations delegating to `AssetStore`.
- `ScriviCore/CMakeLists.txt` — added `src/assets/AssetStore.cpp`.
- `ScriviCore.xcodeproj/project.pbxproj` — E016 (`AssetStore.hpp`), E017 (`AssetStore.cpp`) added; `GRP_ASSETS` group created in `GRP_SRC`.

## Verification

- `cmake --build build --parallel` — passed, no errors.
- `ctest --test-dir build --output-on-failure` — 152/152 passed.
