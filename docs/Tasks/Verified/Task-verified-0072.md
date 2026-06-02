---
name: T-0072
title: "ScriviCoreAdapter — createScene binding (C++ shim + header)"
sprint: SP-022
epic: EP-009
status: ✅ Verified
verified_date: 2026-06-01
---

Added `createScene(projectRootPath, appSupportRoot, projectID, chapterID, afterSceneID, identityID, personaID, displayName)` to `ScriviCoreAdapter.hpp` and implemented in `ScriviCoreAdapter.cpp`. Returns JSON envelope with `sceneID`, `chapterID`, `metadataPath`, `contentPath`.
