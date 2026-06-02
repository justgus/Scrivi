---
name: T-0068
title: "`SceneCreator` implementation — slug generation, file writes, chapter index update"
sprint: SP-021
epic: EP-009
status: ✅ Verified
verified_date: 2026-06-01
---

Created `src/manuscript/SceneCreator.hpp` and `SceneCreator.cpp`. Reads manuscript and chapter metadata to locate insertion point, generates ordinal slug (`NNN-scene`), writes empty `.md` and `.meta.json` atomically, splices `SceneRef` after `afterSceneID` (or appends), rewrites `chapter.meta.json`. Wired into `ScriviCore::createScene()`.
