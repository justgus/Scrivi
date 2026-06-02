# Active Sprint

## SP-024: Scene Navigator — Sidebar, Live Titles, and Click-to-Navigate

**Epic:** EP-009 — Writing Surface and Scene Navigator
**Status:** 🟡 Active
**Start Date:** 2026-06-01
**Target End Date:** TBD

### Goal

Add the Scene Navigator sidebar: an ordered list of all scenes in the manuscript, with each entry titled using the author's own first line of text (or "Scene X" if the scene has no text yet). Titles update live as the author types. Clicking a scene loads it as the current scene and scrolls the editor to its start.

### Tasks

| ID | Title | Status |
| -- | ----- | ------ |
| T-0083 | `SceneNavigatorView` — sidebar `List` of all scenes in manuscript order; each row shows the scene title | 🟡 Active |
| T-0084 | Navigator title derivation — use first line of author's text if present; fall back to "Scene X" (1-based position); chapter breaks as non-selectable section headers | 🔵 Planned |
| T-0085 | Live title updates — ~300ms debounce on keystroke for currently active scene segment | 🔵 Planned |
| T-0086 | Click-to-navigate — selecting a scene loads it as current, scrolls editor to that segment's start | 🔵 Planned |
| T-0087 | Restructure Apple platform — Xcode Workspace + proper xcodeproj, retire SPM package | 🟠 Implemented — Not Verified |

### Acceptance Criteria

- [ ] The Scene Navigator sidebar lists all scenes in manuscript order
- [ ] Scene titles: first line of author's own text (trimmed), or "Scene X" (1-based position in full manuscript) if no text yet
- [ ] Chapters shown as non-selectable section headers above their scenes
- [ ] Navigator titles update live — active scene title reflects current first-line text within ~300ms of last keystroke
- [ ] Clicking a scene in the Navigator loads it as current: loader re-centers, editor scrolls to segment start, Navigator highlight follows
- [ ] No system-generated identifiers (IDs, slugs, UUIDs) visible to the author in the Navigator
- [ ] All 171 ctests remain green; macOS app builds and runs without crash; Navigator renders correctly with 1-scene and 10-scene projects

---

*Last Updated: 2026-06-01 (SP-024 activated)*
