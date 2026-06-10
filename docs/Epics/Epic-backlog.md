# Epic Backlog

Epics listed here are fully defined but have no active sprint. Epics that were partially delivered and returned here retain their verified AC state. Work resumes when a sprint is assigned.

---

## EP-012: Menu Bar and Toolbar Replacement

**Status:** 🔴 Deferred (7/8 ACs verified — AC 5 iPhone pending T-0123)
**Goal:** Replace the toolbar buttons in `EditorView` with a proper macOS/iPadOS menu bar, eliminating the custom toolbar strip and reclaiming that vertical space for the manuscript surface.
**Date Created:** 2026-06-09
**Target Close Date:** TBD
**Actual Close Date:** —

### Acceptance Criteria

- [x] A **Project** menu replaces the legacy "File" menu label and contains: **Close Project** and **Project Settings…**
- [x] An **Edit** menu provides standard system commands: **Undo**, **Redo**, **Cut**, **Copy**, **Paste**, **Select All** — wired to the first-responder chain (no custom handlers needed)
- [x] A **Help** menu (or top-level **Scrivi** / **About** menu) contains: **About Scrivi** (shows an About panel) and a placeholder **User Manual** item (disabled, no-op for now)
- [x] The custom toolbar strip (`private var toolbar` in `ManuscriptEditorView`) is removed entirely; no vertical space is reserved for it
- [ ] On **iPhone** (`os(iOS)` with compact horizontal size class), the Close Project and Project Settings buttons remain in the view — the toolbar is conditionally compiled out only on macOS and iPadOS *(T-0123 implemented, not yet verified — iPhone testing deferred)*
- [x] The About panel shows at minimum: app name, version, and build number pulled from the bundle
- [x] All menu items carry correct keyboard shortcuts: Close Project `⌘W`, Project Settings `⌘,`, Undo `⌘Z`, Redo `⇧⌘Z`, Cut `⌘X`, Copy `⌘C`, Paste `⌘V`
- [x] No regression in existing Close Project and Project Settings behavior

### Scope Notes

- macOS menu bar implemented via SwiftUI `commands { }` on `WindowGroup` in `ScriviApp.swift`
- iPadOS 26 surfaces the same `commands { }` menu bar — no separate iPadOS implementation needed
- iPhone keeps the existing toolbar buttons; conditional is `#if os(iOS)` with `UIDevice.current.userInterfaceIdiom == .phone` check
- Standard Edit commands delegate entirely to the responder chain — no `NSMenuItem` action handlers needed

### Tasks

| ID | Title | Status |
| -- | ----- | ------ |
| T-0120 | Remove toolbar strip and add Project menu (Close Project, Project Settings) | ✅ Verified |
| T-0121 | Add Edit menu with standard responder-chain commands and keyboard shortcuts | ✅ Verified |
| T-0122 | Add About menu with About panel (name, version, build) and User Manual placeholder | ✅ Verified |
| T-0123 | iPhone conditional — restore toolbar buttons on compact/phone idiom | 🔵 Backlog (deferred) |
| T-0124 | EP-012 verification | ✅ Verified |

### Sprints

| Sprint | Title | Status |
| ------ | ----- | ------ |
| SP-035 | Menu Bar, Project Commands, and About Panel | ✅ Closed |

### Deferred Work

- **T-0123** — iPhone toolbar conditional implemented but not verified. Returned to Task-backlog. Requires iPhone build target testing.
- **AC 5** — blocked on T-0123 verification.

---



*Last Updated: 2026-06-10 (EP-015 activated; moved to Epic-active.md)*
