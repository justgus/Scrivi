# SP-035: Menu Bar, Project Commands, and About Panel

**Status:** ✅ Closed
**Epic:** EP-012: Menu Bar and Toolbar Replacement
**Goal:** Remove the custom toolbar strip from `ManuscriptEditorView` and replace it with a proper macOS/iPadOS menu bar — a `Project` menu (Close Project, Project Settings), a standard `Edit` menu wired to the responder chain, and a Help/About menu with an About panel. iPhone retains the existing toolbar buttons via a platform conditional.
**Start Date:** 2026-06-09
**End Date:** 2026-06-09
**Capacity:** Single session

### Assigned Tasks

| ID | Title | Priority | Status |
| -- | ----- | -------- | ------ |
| T-0120 | Remove toolbar strip and add Project menu (Close Project, Project Settings) | High | ✅ Verified |
| T-0121 | Add Edit menu with standard responder-chain commands and keyboard shortcuts | High | ✅ Verified |
| T-0122 | Add About menu with About panel (name, version, build) and User Manual placeholder | Medium | ✅ Verified |
| T-0123 | iPhone conditional — restore toolbar buttons on compact/phone idiom | Medium | ⚪ Returned to Backlog |
| T-0124 | EP-012 verification | High | ✅ Verified |

### Assigned Issues

| ID | Title | Severity | Status |
| -- | ----- | -------- | ------ |
| I-0017 | Window maximized state not restored on app relaunch | Medium | ⚪ Returned to Backlog |
| I-0019 | Undo and Redo have no effect in the manuscript editor | High | ⚪ Returned to Backlog |
| I-0020 | Project name and subtitle should appear in the window title bar | Low | ✅ Verified |
| I-0021 | Chapter headers in Scene Navigator are visually indistinct from scenes | Low | ✅ Verified |
| I-0022 | No way to open a second project in a new window | Medium | ✅ Verified |
| I-0023 | About panel uses SF Symbol placeholder instead of app icon | Low | ✅ Verified |
| I-0024 | Chapter title text written into scene content on app relaunch | High | ✅ Verified |

### Sprint Notes

- Menu commands added via `commands { }` on `WindowGroup` in `ScriviApp.swift`
- `CommandMenu("Project")` replaces the system `File` menu
- Edit menu delegates entirely to the first-responder chain — no custom action handlers
- About panel uses `Bundle.main` for version and build number
- iPhone conditional: `#if os(iOS)` with `UIDevice.current.userInterfaceIdiom == .phone` guard
- `AboutView.swift` added; `pbxproj` updated for all three targets (macOS, iOS, visionOS)

### Retrospective

**Completed:**
- T-0120: Toolbar strip removed; Project menu implemented with Open Project, Project Settings, Close Project
- T-0121: Edit menu — system commands wired to responder chain; empty undo/redo CommandGroup removed
- T-0122: About panel with app name, version, build, and app icon via `NSApp.applicationIconImage`
- T-0124: EP-012 verification — 7 of 8 ACs confirmed on macOS
- I-0020: Project title and subtitle now appear in window title bar
- I-0021: Chapter headers now bold/semibold, scenes indented in Navigator
- I-0022: Open Project added to Project menu, accessible at all times
- I-0023: About panel uses real app icon, not SF Symbol placeholder
- I-0024: Chapter title corruption on relaunch fixed — `recomputeBoundaries` now skips `.scriviHeading` runs; `shouldChangeText` insertion check corrected

**Returned to Backlog:**
- T-0123 — iPhone toolbar conditional implemented but not verified; iPhone build target not in active test workflow
- I-0017 — Window zoom restore not resolved; requires deeper investigation
- I-0019 — Undo/redo still not working; requires deeper investigation

**What went well:**
- Menu bar replacement was clean and straightforward
- Issue triage during the sprint caught and fixed several data-integrity bugs (I-0024)

**What to improve:**
- Issues should be formally activated before implementation begins, not during
- Task template detail should be written at activation time, not retrospectively

**Carry-forward notes:**
- EP-012 moved to Epic-backlog pending AC 5 (iPhone, T-0123)
- I-0017 and I-0019 are open in Issue-backlog — require dedicated investigation sprints
