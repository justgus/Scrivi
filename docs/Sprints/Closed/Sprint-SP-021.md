---
id: SP-021
title: "C++ Core — createScene and createChapter"
epic: EP-009
status: ✅ Closed
start_date: 2026-06-01
end_date: 2026-06-01
---

## Goal

Add `createScene` and `createChapter` as first-class facade methods on `ScriviCore`. These are the only two author-initiated structural actions: `⌘↩` inserts a new scene after the current one within the current chapter; `⌘⇧↩` creates a new chapter and automatically creates its first scene. Both operations are pure C++ — no Swift, no UI in this Sprint.

## Tasks

| ID | Title | Status |
| -- | ----- | ------ |
| T-0067 | `CreateSceneRequest` / `CreateSceneResult` types + `ScriviCore::createScene()` facade | ✅ Verified |
| T-0068 | `SceneCreator` implementation — slug generation, file writes, chapter index update | ✅ Verified |
| T-0069 | `CreateChapterRequest` / `CreateChapterResult` types + `ScriviCore::createChapter()` facade | ✅ Verified |
| T-0070 | `ChapterCreator` implementation — slug generation, file writes, manuscript index update, auto first scene | ✅ Verified |
| T-0071 | Integration tests — `createScene` ordering, insert-after-current, `createChapter` with auto-scene | ✅ Verified |

## Acceptance Criteria

- [x] `ScriviCore::createScene(CreateSceneRequest)` inserts a new empty scene immediately after the scene identified by `afterSceneID` in the specified chapter; returns `CreateSceneResult` with the new scene's ID, metadata path, and content path
- [x] `ScriviCore::createChapter(CreateChapterRequest)` appends a new chapter to `manuscript.meta.json`, creates the chapter directory and `chapter.meta.json`, and automatically creates and inserts one empty scene; returns `CreateChapterResult` carrying both the new chapter and its first scene
- [x] Slug generation for both scene and chapter is deterministic: derived from a running ordinal (e.g. `002-scene`, `chapter-002`) — collision-safe via UUID fallback
- [x] All file writes are atomic (`atomicWriteTextFile`)
- [x] The chapter `.meta.json` `scenes` list reflects correct insertion order after `createScene`
- [x] The `manuscript.meta.json` `chapters` list reflects the new chapter after `createChapter`
- [x] Integration tests cover: insert first scene into a single-scene project; insert after the last scene; `createChapter` produces correct manuscript order with two chapters; `ManuscriptOrderResolver::resolve()` returns the correct full ordered list after each operation
- [x] All 165 existing ctests remain green (171/171 at close including 6 new tests)

## Notes

- `afterSceneID` in `CreateSceneRequest` identifies the insertion point. If omitted or not found, append to end of chapter.
- `chapterID` in `CreateSceneRequest` identifies which chapter receives the new scene. The Swift layer passes the currently active chapter's ID.
- `ChapterCreator` internally delegates scene creation to the same logic as `SceneCreator` — no duplication.
- No adapter or Swift work in this Sprint.

---

## Completion Summary

`SceneCreator` and `ChapterCreator` added to `src/manuscript/`. Both facade methods wired. 6 integration tests added (`CreateSceneTests.cpp`). Test suite grew from 165 to 171 tests, all green. Xcode project updated with file references for all 5 new files.

---

*Last Updated: 2026-06-01 (SP-021 closed; T-0067–T-0071 verified)*
