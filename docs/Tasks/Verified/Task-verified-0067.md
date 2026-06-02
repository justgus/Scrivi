---
name: T-0067
title: "`CreateSceneRequest` / `CreateSceneResult` types + `ScriviCore::createScene()` facade"
sprint: SP-021
epic: EP-009
status: ✅ Verified
verified_date: 2026-06-01
---

Added `CreateSceneRequest`, `CreateChapterRequest` to `Requests.hpp`; `CreateSceneResult`, `CreateChapterResult` to `Results.hpp`; declared `createScene` and `createChapter` on `ScriviCore` in `ScriviCore.hpp`. Stubs in `ScriviCore.cpp` (immediately superseded by T-0068/T-0070).
