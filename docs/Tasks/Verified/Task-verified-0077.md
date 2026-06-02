---
name: T-0077
title: "ViewportSceneLoader — load current scene + neighbors until viewport fills + 1 buffer scene each direction"
sprint: SP-023
epic: EP-009
status: ✅ Verified
verified_date: 2026-06-01
---

Created `ViewportSceneLoader.swift` in `Sources/ScriviApp/`. `@Observable @MainActor` class holding an ordered array of `SceneSegment` values. Loads scenes via `engine.openScene` forward and backward from the current position until combined character count exceeds 3 000-char viewport budget plus one scene each direction. Short/empty scenes are absorbed transparently. Exposes `insertScene(_:after:)` and `insertChapterFirstScene(_:after:)` for `⌘↩`/`⌘⇧↩`. Auto-save helpers `saveCurrentIfDirty` and `saveAndRelease` call `engine.saveScene`.
