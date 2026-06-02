---
name: T-0081
title: "⌘↩ key binding — save current scene, call createScene, insert divider, move cursor to new scene segment"
sprint: SP-023
epic: EP-009
status: ✅ Verified
verified_date: 2026-06-01
---

`ManuscriptNSTextView.keyDown` intercepts `⌘↩` (keyCode 36 + `.command`, no `.shift`). Calls `coordinator.handleCreateScene()`: saves current scene via `saveCurrentIfDirty`, calls `engine.createScene` with current `chapterID` and `afterSceneID`, calls `loader.insertScene`, calls `rebuildStorage`, positions cursor at start of new empty segment via `setSelectedRange`.
