---
name: T-0075
title: "ScriviEngine.swift — createChapter wrapper + CreateChapterResult Swift type"
sprint: SP-022
epic: EP-009
status: ✅ Verified
verified_date: 2026-06-01
---

Added `createChapter(projectRootPath:appSupportRoot:projectID:authorshipRef:)` to `ScriviEngine.swift` using the nested `withCString` pattern. Added `CreateChapterResult: Decodable, Sendable` struct with all five fields: `chapterID`, `chapterMetadataPath`, `firstSceneID`, `firstSceneMetadataPath`, `firstSceneContentPath`.
