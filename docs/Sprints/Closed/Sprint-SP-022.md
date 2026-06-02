---
id: SP-022
title: "Adapter + Swift Engine — createScene and createChapter"
epic: EP-009
status: ✅ Closed
start_date: 2026-06-01
end_date: 2026-06-01
---

## Goal

Expose `createScene` and `createChapter` through the full interop stack: `ScriviCoreAdapter` C++ shim → `ScriviEngine.swift` wrapper → Swift result types. By the end of this Sprint, Swift code can call both operations and receive typed results. No UI in this Sprint.

## Tasks

| ID | Title | Status |
| -- | ----- | ------ |
| T-0072 | `ScriviCoreAdapter` — `createScene` binding (C++ shim + header) | 🔵 Planned |
| T-0073 | `ScriviCoreAdapter` — `createChapter` binding (C++ shim + header) | 🔵 Planned |
| T-0074 | `ScriviEngine.swift` — `createScene` wrapper + `CreateSceneResult` Swift type | 🔵 Planned |
| T-0075 | `ScriviEngine.swift` — `createChapter` wrapper + `CreateChapterResult` Swift type | 🔵 Planned |
| T-0076 | Swift interop tests — round-trip `createScene` and `createChapter` | 🔵 Planned |

## Acceptance Criteria

- [ ] `ScriviAdapter::createScene(projectRootPath, appSupportRoot, projectID, chapterID, afterSceneID, authorIdentityID, authorPersonaID, authorDisplayName)` returns a JSON envelope with the new scene's `sceneID`, `metadataPath`, `contentPath`, `chapterID`
- [ ] `ScriviAdapter::createChapter(projectRootPath, appSupportRoot, projectID, authorIdentityID, authorPersonaID, authorDisplayName)` returns a JSON envelope with the new chapter's `chapterID`, `metadataPath`, the auto-created first scene's `sceneID`, `metadataPath`, `contentPath`
- [ ] `ScriviEngine.createScene(...)` decodes the envelope and returns `CreateSceneResult`
- [ ] `ScriviEngine.createChapter(...)` decodes the envelope and returns `CreateChapterResult`
- [ ] Swift interop tests create a project, call `createScene` twice (verifying order), then call `createChapter` and verify the chapter count and first scene existence
- [ ] All 165 ctests and all Swift package tests remain green

## Notes

- Adapter parameter passing follows the existing `withCString` nesting pattern in `ScriviEngine.swift`.
- `afterSceneID` may be passed as empty string to mean "append to end of chapter".
- `CreateChapterResult` Swift type carries both chapter and first-scene fields; keep flat (no nested struct) to match the existing Swift result type pattern.

## Completion Summary

All 5 tasks implemented and verified. `ScriviCoreAdapter` gained `createScene` and `createChapter` bindings; `ScriviEngine.swift` gained both wrappers and `CreateSceneResult`/`CreateChapterResult` Swift types; Swift interop tests grew from 19 to 21 (all green); 171/171 ctests green.

---

*Last Updated: 2026-06-01 (closed; all tasks verified)*
