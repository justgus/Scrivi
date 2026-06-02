---
name: T-0071
title: "Integration tests — `createScene` ordering, insert-after-current, `createChapter` with auto-scene"
sprint: SP-021
epic: EP-009
status: ✅ Verified
verified_date: 2026-06-01
---

Created `tests/integration/CreateSceneTests.cpp` with 6 tests: append scene to single-scene project; 3-scene ordering via `ManuscriptOrderResolver`; empty `afterSceneID` appends to end; error on unknown `chapterID`; `createChapter` auto first scene files exist on disk; two-chapter manuscript order. Test count grew from 165 to 171, all green.
