# EP-009: Writing Surface and Scene Navigator

**Status:** ✅ Closed
**Goal:** Give the author a real typing surface. When a project is opened, the first scene is immediately ready to receive text. The author types continuously through the manuscript — scene boundaries are invisible except for a thin horizontal divider the author did not write. The Scene Navigator lets the author jump to any scene by title. Saving is automatic and requires no author attention.
**Date Created:** 2026-06-01
**Start Date:** 2026-06-01
**Target Close Date:** TBD
**Actual Close Date:** 2026-06-03

### Acceptance Criteria

- [x] Opening a project on macOS immediately presents a full-height plain-text editor with the first scene's content loaded and the cursor ready to receive input
- [x] The editor displays up to three scenes (or more if scenes are short) as continuous text, separated only by a thin horizontal rule — no labels, no numbers, no system-generated text on the writing surface
- [x] The viewport loading rule: scenes are loaded forward and backward from the current scene until the combined content fills the visible area plus one scene of buffer in each direction; short/empty scenes are absorbed transparently
- [x] `⌘↩` inserts a scene break within the current chapter: saves the current scene, creates a new scene immediately after the current one in the backend, inserts a thin divider, and moves the cursor to the start of the new scene
- [x] `⌘⇧↩` creates a new chapter: saves the current scene, creates a new chapter in the backend (which automatically creates that chapter's first scene), inserts a thin divider, and moves the cursor to the start of the new chapter's first scene
- [x] The Scene Navigator displays chapters as non-selectable section headers above their scenes
- [x] Auto-save fires 1 second after the last keystroke in the current scene
- [x] When a scene scrolls out of the viewport window (scene-exit), it is saved immediately before being released from memory
- [x] On app resign/quit, the current scene is saved immediately; a placeholder hook exists for a future backup operation (stubbed, not implemented)
- [x] The Scene Navigator sidebar lists all scenes in manuscript order
- [x] Scene Navigator titles: if the scene has text, use the first line of the author's own text; if the scene has no text yet, display "Scene X" (X = 1-based position in manuscript order)
- [x] Scene Navigator titles update live as the author types the first line of a scene
- [x] Clicking a scene in the Navigator loads it as the current scene (plus its neighbors) and scrolls the editor to its start
- [x] Scrolling up past a divider makes the previous scene the current scene (the scene that fell off the bottom is saved and released)
- [x] Scrolling down past a divider makes the next scene the current scene (the scene that fell off the top is saved and released)
- [x] The editor is plain text / raw Markdown — no rendering, no styling applied to the author's text
- [x] All ctests remain green (171/171 at close)
- [x] macOS only — iOS/iPadOS deferred to a later Epic

*(18/18 acceptance criteria verified)*

### Sprints

| Sprint | Title | Status | Dates |
| ------ | ----- | ------ | ----- |
| SP-021 | C++ Core — `createScene` and `createChapter` | ✅ Closed | 2026-06-01 → 2026-06-01 |
| SP-022 | Adapter + Swift Engine — `createScene` and `createChapter` | ✅ Closed | 2026-06-01 → 2026-06-01 |
| SP-023 | Writing Surface — `NSTextView`, Viewport Loader, and Auto-Save | ✅ Closed | 2026-06-01 → 2026-06-01 |
| SP-024 | Scene Navigator — Sidebar, Live Titles, and Click-to-Navigate | ✅ Closed | 2026-06-01 → 2026-06-03 |
| SP-025 | Scroll-Driven Scene Switching and EP-009 Close | ✅ Closed | 2026-06-03 → 2026-06-03 |
| SP-026 | Apple Platform C API Boundary — Retire C++ Adapter, Wire scrivi.h | ✅ Closed | 2026-06-02 → 2026-06-02 |

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
| T-0083 | `SceneNavigatorView` — sidebar list in manuscript order | ✅ Verified |
| T-0084 | Navigator title derivation — first line or "Scene X"; chapter headers | ✅ Verified |
| T-0085 | Live title updates — ~300ms debounce | ✅ Verified |
| T-0086 | Click-to-navigate — load scene, scroll editor | ✅ Verified |
| T-0087 | Restructure Apple platform — Xcode Workspace + proper xcodeproj, retire SPM | ✅ Verified |
| T-0088 | Scroll-down past divider — promote next scene, release top scene | ✅ Verified |
| T-0089 | Scroll-up past divider — promote previous scene, release bottom scene | ✅ Verified |
| T-0090 | Remove `ScriviCoreAdapter` target from Xcode — update `project.pbxproj` | ✅ Verified |
| T-0091 | Replace adapter module map with `ScriviCore` plain-C module map (`scrivi.h`) | ✅ Verified |
| T-0092 | Rewrite `ScriviEngine.swift` — call `scrivi_*` C functions; `scrivi_free()` after each call | ✅ Verified |
| T-0093 | Update `ScriviInteropTests.swift` — fix `@testable import` to correct module name | ✅ Verified |
| T-0094 | Verify: Xcode build clean, `ScriviInteropTests` green, `ctest` count unchanged | ✅ Verified |
| T-0095 | EP-009 verification — all ACs green; ctest green; macOS smoke test | ✅ Verified |

### Issues Resolved

| ID | Title | Status |
| -- | ----- | ------ |
| I-0002 | Stale Coordinator Parent Causes Silent Save Failure and Dead Live Titles | ✅ Verified |

### Scope Notes

**Deferred out of EP-009:**
- iOS/iPadOS typing surface — future Epic
- Markdown rendering / rich text — future Epic
- Backup operation on scene-exit/app-resign — placeholder hook only; implementation is a future Epic
- Scene title editing (rename via Navigator) — future Epic
- Chapter title editing (rename via Navigator) — future Epic
- Word count display — future Epic
- Navigator toggle (show/hide sidebar) — deferred if it adds complexity
- Scroll bar manuscript fidelity — T-0096 backlog

### Completion Summary

EP-009 delivered the full writing surface for Scrivi on macOS. Starting from no typing surface, six sprints built and verified:

- **C++ backend:** `createScene` and `createChapter` facade methods with full integration test coverage (171 ctests green)
- **C API boundary:** Retired the C++ adapter in favour of a plain-C `scrivi.h` boundary; `ScriviEngine.swift` rewritten to call `scrivi_*` functions directly
- **Writing surface:** `NSTextView`-based continuous editor with viewport-based scene loading, thin divider rendering, auto-save (1s debounce + scene-exit + app-resign), and `⌘↩` / `⌘⇧↩` key bindings
- **Scene Navigator:** Sidebar listing all scenes in manuscript order; chapter section headers; first-line titles (live ~300ms update, seeded from disk on launch); click-to-navigate
- **Scroll-driven scene switching:** Scrolling past a divider promotes the adjacent scene to current and saves the departing scene; viewport buffer always loads at least one neighbor in each direction regardless of scene size

Two critical bugs discovered and resolved during verification (I-0002): stale `Coordinator.parent` causing silent save failure, and frozen `sceneBoundaries` discarding every keystroke.

**Final state:** 18/18 ACs green · 171/171 ctests green · macOS verified 2026-06-03

---

*Closed: 2026-06-03*
