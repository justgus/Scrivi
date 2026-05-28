# Task Backlog

Tasks listed here are documented and ready for Sprint assignment. All items are 🔵 Backlog.

---

## T-0046: Inbox — `listInbox` / `importFromInbox` Facade Methods

**Status:** 🔵 Backlog
**Component:** ScriviCore — Requests.hpp, Results.hpp, InboxStore, ScriviCore facade
**Priority:** Medium
**Date Requested:** 2026-05-28
**Sprint Assigned:** Not Assigned

**Rationale:**
EP-005 requires `listInbox` / `importFromInbox` facade methods. The inbox is a staging area for externally dropped files before they are committed to the project.

**Current Behavior:**
No inbox-related code exists. `inbox/dropped-files/` is not created at project creation.

**Desired Behavior:**
`listInbox` scans `inbox/dropped-files/` and returns discovered files. `importFromInbox` takes one file and an action (importAsAsset / ignore / deleteFile) and executes it. `ProjectCreator` creates `inbox/dropped-files/` at project creation time.

**Requirements:**
1. `ListInboxRequest` / `ListInboxResult` (vector of `InboxEntry`).
2. `ImportFromInboxRequest` (filename, action enum, author) / `ImportFromInboxResult` (actionTaken, resultPath).
3. `InboxStore` in `src/inbox/` with extension-based MIME detection.
4. `ProjectCreator` amended to create `inbox/dropped-files/`.
5. `ScriviCore.xcodeproj/project.pbxproj` updated.

**Design Approach:**
New `InboxStore.hpp` / `.cpp` in `src/inbox/`. `importAsAsset` action delegates to `AssetStore`. `createScene` action is a stretch goal — scope to `importAsAsset`, `ignore`, `deleteFile` for v1 if `createScene` is complex.

**Components Affected:**
- `ScriviCore/include/scrivi/Requests.hpp`
- `ScriviCore/include/scrivi/Results.hpp`
- `ScriviCore/src/inbox/InboxStore.hpp` / `.cpp` (new)
- `ScriviCore/src/project_package/ProjectCreator.cpp`
- `ScriviCore/include/scrivi/ScriviCore.hpp`
- `ScriviCore/src/public_api/ScriviCore.cpp`
- `ScriviCore.xcodeproj/project.pbxproj`

**Test Steps:**
1. `cmake --build build --parallel` compiles without errors.
2. `ctest --test-dir build --output-on-failure` passes.
3. Integration test: file in inbox → `listInbox` returns it → `importFromInbox` with `importAsAsset` moves to `assets/` and writes sidecar.

**Notes:**
Depends on T-0041 (`AssetStore` must exist for `importAsAsset` action).

---

## T-0047: `ScriviCoreAdapter` — Expose All EP-005 Facade Methods

**Status:** 🔵 Backlog
**Component:** ScriviCoreAdapter, ScriviEngine.swift, interop tests
**Priority:** High
**Date Requested:** 2026-05-28
**Sprint Assigned:** Not Assigned

**Rationale:**
EP-005 requires the adapter to expose all new facade methods and `swift test` to pass. This is the final task in EP-005.

**Current Behavior:**
Adapter exposes character-only object CRUD. No asset, comment, or inbox methods exist.

**Desired Behavior:**
Adapter exposes all EP-005 facade methods (all-type object CRUD, asset CRUD, comment CRUD, inbox). `ScriviEngine.swift` wraps each. At least one happy-path interop test per new method.

**Requirements:**
1. One adapter method per new facade method, `const char*` JSON in, `std::string` envelope out.
2. `ScriviEngine.swift` adds one Swift method per adapter method.
3. At least one interop test per new method.
4. `swift test` passes all interop tests.
5. `ScriviCore.xcodeproj/project.pbxproj` updated if new files added.

**Design Approach:**
Follow established adapter pattern from T-0024/T-0033.

**Components Affected:**
- `platforms/apple/Sources/ScriviCoreAdapter/ScriviCoreAdapter.hpp` / `.cpp`
- `platforms/apple/Sources/Scrivi/ScriviEngine.swift`
- `platforms/apple/Tests/ScriviInteropTests/`

**Test Steps:**
1. `swift build` in `platforms/apple/` passes.
2. `swift test` passes all interop tests.

**Notes:**
Depends on T-0037, T-0038, T-0041, T-0043, T-0044, T-0046. Final task for EP-005 completion gate.

---

*Last Updated: 2026-05-28 (T-0040–T-0045 moved to Active for SP-012)*
