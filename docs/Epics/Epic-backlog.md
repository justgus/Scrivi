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

## EP-013: Scene Navigator Swipe Gesture

**Status:** 🔵 Draft
**Goal:** Let the writer show and hide the Scene Navigator sidebar using a two-finger horizontal swipe on the manuscript surface, as a faster alternative to the NavigationSplitView collapse control.
**Date Created:** 2026-06-09
**Target Close Date:** TBD
**Actual Close Date:** —

### Acceptance Criteria

- [ ] A two-finger swipe **left** on the manuscript surface hides the Scene Navigator (collapses the `NavigationSplitView` sidebar)
- [ ] A two-finger swipe **right** on the manuscript surface shows the Scene Navigator (expands the sidebar)
- [ ] The gesture does not interfere with text selection, scrolling, or cursor placement in `ManuscriptTextView`
- [ ] The gesture works on both macOS (trackpad) and iPadOS (two-finger touch)
- [ ] The swipe threshold is generous enough to avoid accidental triggers during normal typing/scrolling
- [ ] Sidebar state (shown/hidden) persists across scene navigation within the same project session (not required to persist across app launches)
- [ ] No regression in `NavigationSplitView` collapse/expand behavior via the standard sidebar toggle button

### Scope Notes

- On macOS the gesture target is the `NSScrollView` hosting `NSTextView` — a gesture recognizer added to `ManuscriptTextView`'s `NSViewRepresentable` coordinator
- On iPadOS the same gesture is added via SwiftUI's `.gesture()` modifier or a `UIGestureRecognizer` in the representable
- iPhone is out of scope — iPhone does not use `NavigationSplitView` in this layout
- The two-finger swipe is distinct from a one-finger scroll: `NSPanGestureRecognizer` with touch-count filtering on macOS, `UIPanGestureRecognizer` with `minimumNumberOfTouches = 2` on iPadOS

### Tasks

| ID | Title | Status |
| -- | ----- | ------ |
| T-0125 | Two-finger swipe gesture on ManuscriptTextView — macOS (trackpad) | 🔵 Backlog |
| T-0126 | Two-finger swipe gesture on ManuscriptTextView — iPadOS (touch) | 🔵 Backlog |
| T-0127 | EP-013 verification | 🔵 Backlog |

### Sprints

| Sprint | Title | Status |
| ------ | ----- | ------ |
| — | Not yet assigned | — |

---

## EP-014: Scene Inspector Panel

**Status:** 🔵 Draft
**Goal:** Add a hideable Scene Inspector panel on the right side of the editor window, housing a tabbed container whose first tab displays context-aware entity cards for the current scene. The panel and its tabs are structural stubs — no entity data yet — but the full UI skeleton is in place.
**Date Created:** 2026-06-09
**Target Close Date:** TBD
**Actual Close Date:** —

### Background

The Scene Inspector is the right-side panel visible in Xcode's inspector area. As the writer types in a scene, the Inspector will eventually surface entity cards (characters, locations, objects) that are participating in that scene, and allow tagging new ones. EP-014 delivers the panel structure and the first stub tab only — no entity inference, no backend entity schema, no data persistence.

### Acceptance Criteria

- [ ] A Scene Inspector panel is present on the right side of `ManuscriptEditorView`, between the manuscript surface and the window edge
- [ ] The panel is independently hideable via a toolbar icon or View menu item; its shown/hidden state persists within the session
- [ ] The panel contains a tab bar with at least one tab: **"Scene Entities"** (stub content — a placeholder message and a disabled "Add Entity" button)
- [ ] The tab bar is architecturally capable of hosting additional tabs in future epics without structural refactor
- [ ] When hidden, the manuscript surface expands to fill the reclaimed width (no dead space)
- [ ] The panel has a configurable minimum and default width (suggested: 240 pt min, 280 pt default)
- [ ] On iPhone, the Inspector panel is not present (the phone layout is unchanged)
- [ ] On iPadOS, the Inspector panel is present and functions identically to macOS
- [ ] The Scene Entities tab stub contains: a title label, a "No entities yet" empty-state message, and a disabled/placeholder "Add Entity" button — all clearly marked as placeholder UI

### Scope Notes

- Implemented as a conditional `HSplitView` or manual `HStack` with a `@State var inspectorVisible: Bool` in `ManuscriptEditorView`
- The Inspector panel is a new `SceneInspectorView.swift` in `Scrivi/Views/`
- No backend calls, no `ScriviEngine` interaction, no entity schema in this Epic
- Future tabs (e.g., Notes, References, Outline) are added in later Epics by extending the tab model in `SceneInspectorView`
- The hide/show toggle should be a button in the main window toolbar or a `View` menu command — consistent with macOS inspector conventions

### Tasks

| ID | Title | Status |
| -- | ----- | ------ |
| T-0128 | SceneInspectorView skeleton — panel, hide/show toggle, tab bar structure | 🔵 Backlog |
| T-0129 | Scene Entities stub tab — empty state, placeholder Add Entity button | 🔵 Backlog |
| T-0130 | Inspector panel integration into ManuscriptEditorView — width, collapse, expand | 🔵 Backlog |
| T-0131 | iPhone exclusion — Inspector absent on phone idiom | 🔵 Backlog |
| T-0132 | EP-014 verification | 🔵 Backlog |

### Sprints

| Sprint | Title | Status |
| ------ | ----- | ------ |
| — | Not yet assigned | — |

---

## EP-015: Real-Time Timeline Panel

**Status:** 🔵 Draft
**Goal:** Add a hideable, docked graphical timeline strip below the manuscript viewport. The timeline is a visual representation of the story's events — scenes from the manuscript plus non-scene events (battles, off-page deaths, political events, etc.) — laid out in narrative or chronological order. EP-015 delivers the panel structure and UI chrome only; the timeline data model and event schema are deferred to a future Epic.
**Date Created:** 2026-06-09
**Target Close Date:** TBD
**Actual Close Date:** —

### Background

The Real-Time Timeline gives the writer a live view of where the current scene sits within the broader story timeline. It is a graphical track — not a list — architecturally separate from `SceneNavigatorView`. It will eventually show both manuscript scenes and non-scene story events, but EP-015 delivers the panel shell and stub markers only.

### Acceptance Criteria

- [ ] A Timeline panel is docked at the bottom of the manuscript surface (below `ManuscriptTextView`, above the window chrome)
- [ ] The Timeline panel is independently hideable via a toolbar icon or View menu item; its shown/hidden state persists within the session
- [ ] When hidden, the manuscript surface expands to fill the reclaimed height (no dead space)
- [ ] The panel renders a horizontal graphical track with placeholder event markers — visually distinct from the Scene Navigator list
- [ ] The panel has a fixed height with horizontal scroll if content overflows
- [ ] On iPhone, the Timeline panel is not present (phone layout unchanged)
- [ ] On iPadOS, the Timeline panel is present and functions identically to macOS
- [ ] All content is clearly stub/placeholder — no real event data, no scene linkage, no interaction beyond show/hide

### Scope Notes

- The Timeline is a graphical view, not a list. Architecturally separate from `SceneNavigatorView`.
- Will eventually show both manuscript scenes and non-scene timeline events. Data model for both is undefined — EP-015 builds the panel shell only.
- No `ViewportSceneLoader` integration, no navigation-on-tap, no current-scene highlight in this Epic.
- New `TimelineStripView.swift` in `Scrivi/Views/`

### Tasks

| ID | Title | Status |
| -- | ----- | ------ |
| T-0133 | TimelineStripView skeleton — panel chrome, hide/show toggle, horizontal scroll container | 🔵 Backlog |
| T-0134 | Placeholder graphical event markers — static stub layout | 🔵 Backlog |
| T-0135 | Timeline integration into ManuscriptEditorView — docking, height, collapse | 🔵 Backlog |
| T-0136 | iPhone exclusion — Timeline absent on phone idiom | 🔵 Backlog |
| T-0137 | EP-015 verification | 🔵 Backlog |

### Sprints

| Sprint | Title | Status |
| ------ | ----- | ------ |
| — | Not yet assigned | — |

---

*Last Updated: 2026-06-09 (EP-013, EP-014, EP-015 moved here from Epic-active — Draft, no sprint assigned)*
