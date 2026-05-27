## T-0030: Repair Handlers — Manuscript File Operations

**Status:** ✅ Implemented - Verified
**Component:** ScriviCore (C++ backend) — repair
**Priority:** High
**Epic:** EP-004: Repair and Recovery
**Date Requested:** 2026-05-26
**Date Implemented:** 2026-05-27
**Date Verified:** 2026-05-27
**Sprint Assigned:** SP-009

**Rationale:**
The actual repair logic for each action kind. Eight actions operate on manuscript content and metadata files. Each must be implemented as a named handler in `src/repair/`, called by the `applyRepair` dispatcher.

**Current Behavior:**
No repair handlers exist. `applyRepair` dispatcher (T-0029) has stubs.

**Desired Behavior:**
All 8 handlers implemented and wired into the dispatcher:
1. `relinkToFile` — updates `contentPath` in scene metadata to point to `targetPath`
2. `createEmptyContentFile` — writes an empty `.md` file at the expected content path
3. `markMissing` — sets `status = "missing"` in scene metadata
4. `removeFromProject` — removes the scene entry from chapter metadata
5. `moveToInbox` — moves the file to `projectRoot/inbox/dropped-files/`
6. `reloadExternalVersion` — returns current on-disk content (no write)
7. `regenerateMetadata (scene)` — reconstructs `.meta.json` from content file name
8. `regenerateMetadata (chapter)` — reconstructs `chapter.meta.json` from folder contents

**Requirements:**
1. `src/repair/` contains named handler functions for each of the 8 actions
2. Handlers use `FileSystem&` — no direct `std::filesystem` calls (testable via mock)
3. Backup-required actions create a `.bak` copy before modifying
4. Each handler returns `Result<ApplyRepairResult>`
5. Build passes

**Components Affected:**
- New: `ScriviCore/src/repair/RepairHandlers.hpp/.cpp`
- Modified: `ScriviCore/src/repair/RepairDispatcher.cpp` (stubs replaced)
- Modified: `ScriviCore.xcodeproj/project.pbxproj`

**Implementation Details:**
- `HandlerContext` struct carries `request`, `issue`, and `services` to each handler.
- Private helpers: `okResult`, `repairError`, `readSceneMeta`, `writeBackup`, `findChapterForScene`.
- All 8 handlers implemented; remaining action kinds return `internalError / "not yet implemented"`.
- `CMakeLists.txt` and `project.pbxproj` (IDs `A011`/`A012` in repair group) updated.
- Build clean, 102/102 tests pass.

**Test Steps:**
1. `cmake --build build --parallel` — zero errors ✅
2. `ctest --test-dir build --output-on-failure` — 102/102 pass ✅
