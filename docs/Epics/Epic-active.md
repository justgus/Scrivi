# Active Epics

## EP-009: Writing Surface and Scene Navigator

**Status:** 🔵 Draft
**Goal:** Give the author a real typing surface. When a project is opened, the first scene is immediately ready to receive text. The author types continuously through the manuscript — scene boundaries are invisible except for a thin horizontal divider the author did not write. The Scene Navigator lets the author jump to any scene by title. Saving is automatic and requires no author attention.
**Date Created:** 2026-06-01
**Start Date:** —
**Target Close Date:** TBD
**Actual Close Date:** —

### Acceptance Criteria

- [ ] Opening a project on macOS immediately presents a full-height plain-text editor with the first scene's content loaded and the cursor ready to receive input
- [ ] The editor displays up to three scenes (or more if scenes are short) as continuous text, separated only by a thin horizontal rule — no labels, no numbers, no system-generated text on the writing surface
- [ ] The viewport loading rule: scenes are loaded forward and backward from the current scene until the combined content fills the visible area plus one scene of buffer in each direction; short/empty scenes are absorbed transparently
- [ ] `⌘↩` inserts a scene break within the current chapter: saves the current scene, creates a new scene immediately after the current one in the backend, inserts a thin divider, and moves the cursor to the start of the new scene
- [ ] `⌘⇧↩` creates a new chapter: saves the current scene, creates a new chapter in the backend (which automatically creates that chapter's first scene), inserts a thin divider, and moves the cursor to the start of the new chapter's first scene
- [ ] The Scene Navigator displays chapters as non-selectable section headers above their scenes
- [ ] Auto-save fires 1 second after the last keystroke in the current scene
- [ ] When a scene scrolls out of the viewport window (scene-exit), it is saved immediately before being released from memory
- [ ] On app resign/quit, the current scene is saved immediately; a placeholder hook exists for a future backup operation (stubbed, not implemented)
- [ ] The Scene Navigator sidebar lists all scenes in manuscript order
- [ ] Scene Navigator titles: if the scene has text, use the first line of the author's own text; if the scene has no text yet, display "Scene X" (X = 1-based position in manuscript order)
- [ ] Scene Navigator titles update live as the author types the first line of a scene
- [ ] Clicking a scene in the Navigator loads it as the current scene (plus its neighbors) and scrolls the editor to its start
- [ ] Scrolling up past a divider makes the previous scene the current scene (the scene that fell off the bottom is saved and released)
- [ ] Scrolling down past a divider makes the next scene the current scene (the scene that fell off the top is saved and released)
- [ ] The editor is plain text / raw Markdown — no rendering, no styling applied to the author's text
- [ ] All existing ctests (165/165) remain green throughout
- [ ] macOS only — iOS/iPadOS deferred to a later Epic

*(18 acceptance criteria total)*

### Sprints

| Sprint | Title | Status | Dates |
| ------ | ----- | ------ | ----- |
| SP-021 | C++ Core — `createScene` and `createChapter` | ✅ Closed | 2026-06-01 → 2026-06-01 |
| SP-022 | Adapter + Swift Engine — `createScene` and `createChapter` | ✅ Closed | 2026-06-01 → 2026-06-01 |
| SP-023 | Writing Surface — `NSTextView`, Viewport Loader, and Auto-Save | ✅ Closed | 2026-06-01 → 2026-06-01 |
| SP-024 | Scene Navigator — Sidebar, Live Titles, and Click-to-Navigate | 🟡 Active | 2026-06-01 → TBD |
| SP-025 | Scroll-Driven Scene Switching and EP-009 Close | 🔵 Planning | TBD |

### Tasks

| ID | Title | Status |
| -- | ----- | ------ |
| T-0067 | `CreateSceneRequest` / `CreateSceneResult` types + `ScriviCore::createScene()` facade | ✅ Verified |
| T-0068 | `SceneCreator` implementation — slug generation, file writes, chapter index update | ✅ Verified |
| T-0069 | `CreateChapterRequest` / `CreateChapterResult` types + `ScriviCore::createChapter()` facade | ✅ Verified |
| T-0070 | `ChapterCreator` implementation — slug generation, file writes, manuscript index update, auto first scene | ✅ Verified |
| T-0071 | Integration tests — `createScene` ordering, insert-after-current, `createChapter` with auto-scene | ✅ Verified |
| T-0072 | `ScriviCoreAdapter` — `createScene` binding | ✅ Verified |
| T-0073 | `ScriviCoreAdapter` — `createChapter` binding | ✅ Verified |
| T-0074 | `ScriviEngine.swift` — `createScene` wrapper + Swift type | ✅ Verified |
| T-0075 | `ScriviEngine.swift` — `createChapter` wrapper + Swift type | ✅ Verified |
| T-0076 | Swift interop tests — round-trip `createScene` and `createChapter` | ✅ Verified |
| T-0077 | `ViewportSceneLoader` — viewport-based scene loading | ✅ Verified |
| T-0078 | `ManuscriptTextView` — `NSViewRepresentable` writing surface | ✅ Verified |
| T-0079 | Divider rendering — thin horizontal rule, no text | ✅ Verified |
| T-0080 | Auto-save — debounce, scene-exit, app-resign + backup placeholder | ✅ Verified |
| T-0081 | `⌘↩` — create scene key binding | ✅ Verified |
| T-0082 | `⌘⇧↩` — create chapter key binding | ✅ Verified |
| T-0083 | `SceneNavigatorView` — sidebar list in manuscript order | 🟡 Active |
| T-0084 | Navigator title derivation — first line or "Scene X"; chapter headers | 🔵 Backlog |
| T-0085 | Live title updates — ~300ms debounce | 🔵 Backlog |
| T-0086 | Click-to-navigate — load scene, scroll editor | 🔵 Backlog |
| T-0087 | Scroll-down past divider — promote next scene, release top scene | 🔵 Backlog |
| T-0088 | Scroll-up past divider — promote previous scene, release bottom scene | 🔵 Backlog |
| T-0089 | EP-009 verification — 16 ACs green; 165/165 ctests; macOS smoke test | 🔵 Backlog |

### Issues

| ID | Title | Status |
| -- | ----- | ------ |
| —  | None  | —      |

### Scope Notes

**Deferred out of EP-009:**
- iOS/iPadOS typing surface — future Epic
- Markdown rendering / rich text — future Epic
- Backup operation on scene-exit/app-resign — placeholder hook only; implementation is a future Epic
- Scene title editing (rename via Navigator) — future Epic
- Chapter title editing (rename via Navigator) — future Epic
- Word count display — future Epic
- Navigator toggle (show/hide sidebar) — stretch goal for SP-024; deferred if it adds complexity

**Dependency check (completed 2026-06-01):**
- Gap confirmed: `createScene` and `createChapter` do not exist on the facade. Scene creation exists only inside `ProjectCreator` at project-creation time. SP-021 adds both methods. `ChapterCreator` will internally reuse `SceneCreator` logic for the auto-first-scene. `ManuscriptOrderResolver` is already in place and will correctly resolve the updated chapter/scene lists after each operation.

### Completion Summary

*(Filled in when EP-009 reaches 🟠 Complete)*

---

*Last Updated: 2026-06-01 (SP-023 closed; T-0077–T-0082 verified; SP-024 activated)*
