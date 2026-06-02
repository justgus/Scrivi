---
name: T-0073
title: "ScriviCoreAdapter — createChapter binding (C++ shim + header)"
sprint: SP-022
epic: EP-009
status: ✅ Verified
verified_date: 2026-06-01
---

Added `createChapter(projectRootPath, appSupportRoot, projectID, identityID, personaID, displayName)` to `ScriviCoreAdapter.hpp` and implemented in `ScriviCoreAdapter.cpp`. Returns JSON envelope with `chapterID`, `chapterMetadataPath`, `firstSceneID`, `firstSceneMetadataPath`, `firstSceneContentPath`.
