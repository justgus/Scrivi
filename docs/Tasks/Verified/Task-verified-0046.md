# T-0046: Inbox — `listInbox` / `importFromInbox` Facade Methods

**Status:** ✅ Implemented - Verified
**Component:** ScriviCore — InboxStore, Requests.hpp, Results.hpp, ScriviCore facade, ProjectCreator
**Priority:** Medium
**Date Requested:** 2026-05-28
**Date Implemented:** 2026-05-28
**Date Verified:** 2026-05-28
**Sprint Assigned:** SP-013

**Rationale:**
EP-005 requires `listInbox` / `importFromInbox` facade methods. The inbox is a staging area for externally dropped files before they are committed to the project.

**Current Behavior:**
No inbox-related code existed. `inbox/dropped-files/` was not created at project creation.

**Desired Behavior:**
`listInbox` scans `inbox/dropped-files/` and returns discovered files. `importFromInbox` takes one file and an action (importAsAsset / ignore / deleteFile) and executes it. `ProjectCreator` creates `inbox/dropped-files/` at project creation time.

**Requirements:**
1. `ListInboxRequest` / `ListInboxResult` (vector of `InboxEntry`). ✅
2. `ImportFromInboxRequest` (filename, action enum, author) / `ImportFromInboxResult` (actionTaken, resultPath). ✅
3. `InboxStore` in `src/inbox/` with `importAsAsset` delegating to `AssetStore`. ✅
4. `ProjectCreator` amended to create `inbox/dropped-files/`. ✅
5. `ScriviCore.xcodeproj/project.pbxproj` updated (E022–E024, GRP_INBOX). ✅

**Design Approach:**
`InboxStore.hpp` / `.cpp` in `src/inbox/`. `importAsAsset` action delegates to `AssetStore` and removes the source file after successful import. `ignore` is a no-op. `deleteFile` removes the file via `fs.removeFile`.

**Components Affected:**
- `ScriviCore/include/scrivi/Requests.hpp` — `InboxAction` enum, `ListInboxRequest`, `ImportFromInboxRequest`
- `ScriviCore/include/scrivi/Results.hpp` — `InboxEntry`, `ListInboxResult`, `ImportFromInboxResult`
- `ScriviCore/include/scrivi/ScriviCore.hpp` — `listInbox`, `importFromInbox` declarations
- `ScriviCore/src/inbox/InboxStore.hpp` / `.cpp` (new)
- `ScriviCore/src/public_api/ScriviCore.cpp` — two new facade dispatch methods
- `ScriviCore/src/project_package/ProjectCreator.cpp` — step 8: create `inbox/dropped-files/`
- `ScriviCore/CMakeLists.txt` — `src/inbox/InboxStore.cpp` added
- `ScriviCore/tests/CMakeLists.txt` — `integration/InboxTests.cpp` added
- `ScriviCore/tests/integration/InboxTests.cpp` (new) — 6 test cases
- `ScriviCore.xcodeproj/project.pbxproj` — E022–E024, GRP_INBOX

**Test Result:** 158/158 CTest passing at verification.
