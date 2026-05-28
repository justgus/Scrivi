# Unverified Tasks

Tasks listed here are implemented and awaiting user verification.

---

## T-0046: Inbox — `listInbox` / `importFromInbox` Facade Methods

**Status:** 🟠 Implemented - Not Verified
**Sprint:** SP-013
**Component:** ScriviCore — InboxStore, Requests.hpp, Results.hpp, ScriviCore facade, ProjectCreator
**Priority:** Medium

**What was implemented:**
- `InboxAction` enum (`importAsAsset`, `ignore`, `deleteFile`) added to `Requests.hpp`
- `ListInboxRequest` / `ImportFromInboxRequest` added to `Requests.hpp`
- `InboxEntry` / `ListInboxResult` / `ImportFromInboxResult` added to `Results.hpp`
- `InboxStore` in `src/inbox/InboxStore.hpp` / `.cpp`
  - `list`: scans `inbox/dropped-files/` and returns all files as `InboxEntry`
  - `importFromInbox`: `importAsAsset` delegates to `AssetStore` and removes source; `ignore` no-ops; `deleteFile` removes the file
- `ProjectCreator` amended to create `inbox/dropped-files/` at project creation time
- `ScriviCore::listInbox` / `ScriviCore::importFromInbox` facade methods
- `ScriviCore.xcodeproj/project.pbxproj` updated (E022–E024, GRP_INBOX)
- `ScriviCore/CMakeLists.txt` and `tests/CMakeLists.txt` updated
- Integration tests in `tests/integration/InboxTests.cpp`: 6 test cases covering directory creation, empty list, file listing, importAsAsset, ignore, deleteFile

**Files changed:**
- `ScriviCore/include/scrivi/Requests.hpp` (modified)
- `ScriviCore/include/scrivi/Results.hpp` (modified)
- `ScriviCore/include/scrivi/ScriviCore.hpp` (modified)
- `ScriviCore/src/inbox/InboxStore.hpp` (new)
- `ScriviCore/src/inbox/InboxStore.cpp` (new)
- `ScriviCore/src/public_api/ScriviCore.cpp` (modified)
- `ScriviCore/src/project_package/ProjectCreator.cpp` (modified)
- `ScriviCore/CMakeLists.txt` (modified)
- `ScriviCore/tests/CMakeLists.txt` (modified)
- `ScriviCore/tests/integration/InboxTests.cpp` (new)
- `ScriviCore.xcodeproj/project.pbxproj` (modified)

**Test result:** 158/158 CTest passing

---

## T-0047: `ScriviCoreAdapter` — Expose All EP-005 Facade Methods

**Status:** 🟠 Implemented - Not Verified
**Sprint:** SP-013
**Component:** ScriviCoreAdapter, ScriviEngine.swift, ScriviInteropTests
**Priority:** High

**What was implemented:**
- `ScriviCoreAdapter.hpp` / `.cpp` — adapter methods for all EP-005 facade operations:
  - Object CRUD: `createObject`, `openObject`, `saveObject`, `deleteObject`
  - Assets: `importAsset`, `listAssets`, `removeAsset`
  - Comments: `addComment`, `listComments`, `resolveComment`
  - Inbox: `listInbox`, `importFromInbox`
- `ScriviEngine.swift` — Swift wrapper methods for all new adapter methods
- Swift result types: `CreateObjectResult`, `OpenObjectResult`, `SaveObjectResult`, `DeleteObjectResult`, `ImportAssetResult`, `ListAssetsResult`, `RemoveAssetResult`, `AddCommentResult`, `ListCommentsResult`, `ResolveCommentResult`, `ListInboxResult`, `ImportFromInboxResult`
- `ScriviInteropTests.swift` — 8 new interop tests (Tests 10–13 covering objects, assets, comments, inbox)

**Files changed:**
- `platforms/apple/Sources/ScriviCoreAdapter/ScriviCoreAdapter.hpp` (modified)
- `platforms/apple/Sources/ScriviCoreAdapter/ScriviCoreAdapter.cpp` (modified)
- `platforms/apple/Sources/Scrivi/ScriviEngine.swift` (modified)
- `platforms/apple/Tests/ScriviInteropTests/ScriviInteropTests.swift` (modified)

**Test result:** 17/17 `swift test` passing

---

*Last Updated: 2026-05-28 (T-0046, T-0047 implemented — awaiting verification)*
