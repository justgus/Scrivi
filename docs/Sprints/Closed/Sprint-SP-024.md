---
id: SP-024
title: "Scene Navigator — Sidebar, Live Titles, and Click-to-Navigate"
epic: EP-009
status: ✅ Closed
start_date: 2026-06-01
end_date: 2026-06-03
---

## Goal

Add the Scene Navigator sidebar: an ordered list of all scenes in the manuscript, with each entry titled using the author's own first line of text (or "Scene X" if the scene has no text yet). Titles update live as the author types. Clicking a scene loads it as the current scene and scrolls the editor to its start.

## Tasks

| ID | Title | Status |
| -- | ----- | ------ |
| T-0083 | `SceneNavigatorView` — sidebar `List` of all scenes in manuscript order; each row shows the scene title | ✅ Verified |
| T-0084 | Navigator title derivation — use first line of author's text if present; fall back to "Scene X" (1-based position); chapter breaks displayed as non-selectable section headers | ✅ Verified |
| T-0085 | Live title updates — as the author types the first line of a scene segment, the corresponding Navigator row title updates in real time (debounced ~300ms to avoid per-keystroke rebuilds) | ✅ Verified |
| T-0086 | Click-to-navigate — selecting a scene in the Navigator calls `ViewportSceneLoader` to load that scene as current (plus neighbors), then scrolls the `NSTextView` to the start of that scene's segment | ✅ Verified |
| T-0087 | Restructure Apple platform — Xcode Workspace + proper xcodeproj, retire SPM package (implemented in SP-024, compiler errors resolved in SP-026) | ✅ Verified |

## Acceptance Criteria

- [x] The Scene Navigator sidebar lists all scenes in manuscript order
- [x] Scene titles: if the scene has text, the title is the first line of the author's own text; if the scene has no text, the title is "Scene X" (1-based position in full manuscript list)
- [x] Chapters are shown as non-selectable section headers ("Chapter 1", "Chapter 2") above their scenes
- [x] Navigator titles update live as the author types — within approximately 300ms of the last keystroke
- [x] Navigator titles reflect persisted content immediately on launch (seeded from loaded scene content)
- [x] Clicking a scene in the Navigator loads it as the current scene, scrolls the editor to its start, and updates the Navigator selection highlight
- [x] The Navigator does not display any system-generated identifiers visible to the author
- [x] macOS app builds and runs without crash; Navigator renders correctly

## Issues Resolved During This Sprint

| ID | Title | Status |
| -- | ----- | ------ |
| I-0002 | Stale Coordinator Parent Causes Silent Save Failure and Dead Live Titles | ✅ Verified |

## Notes

- SP-024 was paused mid-sprint to allow SP-026 (C API boundary migration) to resolve compiler errors that blocked verification. SP-024 resumed after SP-026 closed.
- I-0002 was discovered during verification: two bugs (`Coordinator.parent` staleness and frozen `sceneBoundaries`) caused all text saves and live titles to silently fail. Both fixed and verified as part of this sprint.
- Title seeding on load (`liveTitles` populated in `loadScene`) added to address Navigator showing "Scene X" on every relaunch.

---

*Closed: 2026-06-03*
