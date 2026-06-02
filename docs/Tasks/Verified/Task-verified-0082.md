---
name: T-0082
title: "⌘⇧↩ key binding — save current scene, call createChapter, insert divider, move cursor to new chapter's first scene"
sprint: SP-023
epic: EP-009
status: ✅ Verified
verified_date: 2026-06-01
---

`ManuscriptNSTextView.keyDown` intercepts `⌘⇧↩` (keyCode 36 + `.command` + `.shift`). Calls `coordinator.handleCreateChapter()`: saves current scene via `saveCurrentIfDirty`, calls `engine.createChapter`, calls `loader.insertChapterFirstScene`, calls `rebuildStorage`, positions cursor at start of new chapter's first scene.

**Modified files (SP-023 as a whole):**
- `platforms/apple/Sources/ScriviApp/ViewportSceneLoader.swift` — new file
- `platforms/apple/Sources/ScriviApp/ManuscriptTextView.swift` — new file
- `platforms/apple/Sources/ScriviApp/EditorView.swift` — replaced TrackingTextEditor; routes through viewportLoader
- `platforms/apple/Sources/ScriviApp/AppEnvironment.swift` — added viewportLoader; added onAppResign() stub; removed old saveScene method
- `platforms/apple/Sources/ScriviApp/ScriviApp.swift` — added willResignActiveNotification subscription
- `ScriviCore.xcodeproj/project.pbxproj` — PBXFileReference, PBXBuildFile, PBXGroup, PBXSourcesBuildPhase entries for both new files
