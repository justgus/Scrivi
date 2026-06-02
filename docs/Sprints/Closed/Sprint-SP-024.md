---
id: SP-024
title: "Scene Navigator — Sidebar, Live Titles, and Click-to-Navigate"
epic: EP-009
status: 🟡 Active
start_date: 2026-06-01
end_date: TBD
---

## Goal

Add the Scene Navigator sidebar: an ordered list of all scenes in the manuscript, with each entry titled using the author's own first line of text (or "Scene X" if the scene has no text yet). Titles update live as the author types. Clicking a scene loads it as the current scene and scrolls the editor to its start.

## Tasks

| ID | Title | Status |
| -- | ----- | ------ |
| T-0083 | `SceneNavigatorView` — sidebar `List` of all scenes in manuscript order; each row shows the scene title | 🔵 Planned |
| T-0084 | Navigator title derivation — use first line of author's text if present; fall back to "Scene X" (1-based position); chapter breaks displayed as non-selectable section headers | 🔵 Planned |
| T-0085 | Live title updates — as the author types the first line of a scene segment, the corresponding Navigator row title updates in real time (debounced ~300ms to avoid per-keystroke rebuilds) | 🔵 Planned |
| T-0086 | Click-to-navigate — selecting a scene in the Navigator calls `ViewportSceneLoader` to load that scene as current (plus neighbors), then scrolls the `NSTextView` to the start of that scene's segment | 🔵 Planned |

## Acceptance Criteria

- [ ] The Scene Navigator sidebar lists all scenes in manuscript order
- [ ] Scene titles: if the scene has text, the title is the first line of the author's own text, trimmed and truncated to a reasonable display length; if the scene has no text, the title is "Scene X" where X is the 1-based position in the full manuscript scene list
- [ ] Chapters are shown as non-selectable section headers in the Navigator list (e.g. "Chapter 1", "Chapter 2") above their scenes; the header is derived from the chapter's stored title
- [ ] Navigator titles update live as the author types — specifically, the title for the currently active scene segment reflects the current text of the first line within approximately 300ms of the last keystroke
- [ ] Clicking a scene in the Navigator loads it as the current scene: `ViewportSceneLoader` re-centers on that scene, the editor scrolls to the top of that scene's segment, and the Navigator selection highlight follows
- [ ] The Navigator does not display any system-generated identifiers (IDs, slugs, UUIDs) visible to the author
- [ ] All 165 ctests remain green; the macOS app builds and runs without crash; Navigator renders correctly with 1-scene and 10-scene projects

## Notes

- Navigator title derivation is purely client-side: read the first line from the in-memory `SceneSegment` text, not from the stored `SceneMetaData.title` field. The stored title field is not updated live during typing — it remains the value from creation time.
- The "Scene X" ordinal is the 1-based index in the full `ViewportSceneLoader.allScenes` array, not the index within a chapter.
- Chapter headers in the Navigator are read from `CreateChapterResult.chapterTitle` or from `OpenProjectResult.scenes[i].chapterID` cross-referenced against the chapter list. The exact data path should be determined during implementation; the stored chapter title is used, not derived from content.
- Navigator is a sidebar panel in the main window split view, shown by default, togglable with a toolbar button (toggle implementation is optional stretch for this Sprint — Navigator may always be visible if toggle adds complexity).

---

*Last Updated: 2026-06-01 (created as Planning)*
