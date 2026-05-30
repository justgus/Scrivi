# T-0047: `ScriviCoreAdapter` — Expose All EP-005 Facade Methods

**Status:** ✅ Implemented - Verified
**Component:** ScriviCoreAdapter, ScriviEngine.swift, ScriviInteropTests
**Priority:** High
**Date Requested:** 2026-05-28
**Date Implemented:** 2026-05-28
**Date Verified:** 2026-05-28
**Sprint Assigned:** SP-013

**Rationale:**
EP-005 requires the adapter to expose all new facade methods and `swift test` to pass. This is the final task in EP-005.

**Current Behavior:**
Adapter exposed character-only object CRUD. No asset, comment, or inbox methods existed.

**Desired Behavior:**
Adapter exposes all EP-005 facade methods (all-type object CRUD, asset CRUD, comment CRUD, inbox). `ScriviEngine.swift` wraps each. At least one happy-path interop test per new method.

**Requirements:**
1. One adapter method per new facade method, `const char*` JSON in, `std::string` envelope out. ✅
2. `ScriviEngine.swift` adds one Swift method per adapter method. ✅
3. At least one interop test per new method. ✅
4. `swift test` passes all interop tests. ✅
5. `ScriviCore.xcodeproj/project.pbxproj` updated if new files added. ✅ (no new files needed)

**Design Approach:**
Followed established adapter pattern from T-0024/T-0033. `saveObject` takes an `objectKind` parameter alongside the JSON string so the adapter can call `parseWorldObject(json, kind)`. Asset listing uses a compact JSON array string for the asset list to avoid building a nested JSON object array manually.

**Components Affected:**
- `platforms/apple/Sources/ScriviCoreAdapter/ScriviCoreAdapter.hpp` — 13 new method declarations
- `platforms/apple/Sources/ScriviCoreAdapter/ScriviCoreAdapter.cpp` — 13 new implementations
- `platforms/apple/Sources/Scrivi/ScriviEngine.swift` — 13 new engine methods + 12 new result types
- `platforms/apple/Tests/ScriviInteropTests/ScriviInteropTests.swift` — Tests 10–13 (8 new tests)

**New adapter methods:** `createObject`, `openObject`, `saveObject`, `deleteObject`, `importAsset`, `listAssets`, `removeAsset`, `addComment`, `listComments`, `resolveComment`, `listInbox`, `importFromInbox`

**New Swift result types:** `CreateObjectResult`, `OpenObjectResult`, `SaveObjectResult`, `DeleteObjectResult`, `ImportAssetResult`, `ListAssetsResult`, `RemoveAssetResult`, `AddCommentResult`, `ListCommentsResult`, `ResolveCommentResult`, `ListInboxResult`, `ImportFromInboxResult`

**Test Result:** 17/17 `swift test` passing at verification. EP-005 completion gate passed.
