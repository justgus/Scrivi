---
name: T-0070
title: "`ChapterCreator` implementation — slug generation, file writes, manuscript index update, auto first scene"
sprint: SP-021
epic: EP-009
status: ✅ Verified
verified_date: 2026-06-01
---

Created `src/manuscript/ChapterCreator.hpp` and `ChapterCreator.cpp`. Reads manuscript for ordinal, generates chapter slug (`chapter-NNN`), creates chapter directory, writes auto first scene (`.md` + `.meta.json`), writes `chapter.meta.json`, appends `ChapterRef` to `manuscript.meta.json`. Wired into `ScriviCore::createChapter()`.
