# Task Backlog

Tasks listed here are documented and ready for Sprint assignment.

New, unstarted tasks are listed as summary rows. Tasks that have been implemented but returned to the backlog before verification retain their full detail below the index table so no documentation is lost.

---

## Index

| Task | Title | Epic | Status |
| ---- | ----- | ---- | ------ |
| T-0118 | Scroll bar fidelity — per-scene character-ratio thumb position and size | EP-011 | 🔵 Backlog |
| T-0123 | iPhone conditional — restore toolbar buttons on compact/phone idiom | EP-012 | 🟡 Implemented - Not Verified |
| T-0125 | Two-finger swipe gesture on ManuscriptTextView — macOS (trackpad) | EP-013 | 🔵 Backlog |
| T-0126 | Two-finger swipe gesture on ManuscriptTextView — iPadOS (touch) | EP-013 | 🔵 Backlog |
| T-0127 | EP-013 verification | EP-013 | 🔵 Backlog |
| T-0128 | SceneInspectorView skeleton — panel, hide/show toggle, tab bar structure | EP-014 | 🔵 Backlog |
| T-0129 | Scene Entities stub tab — empty state, placeholder Add Entity button | EP-014 | 🔵 Backlog |
| T-0130 | Inspector panel integration into ManuscriptEditorView — width, collapse, expand | EP-014 | 🔵 Backlog |
| T-0131 | iPhone exclusion — Inspector absent on phone idiom | EP-014 | 🔵 Backlog |
| T-0132 | EP-014 verification | EP-014 | 🔵 Backlog |
| T-0133 | TimelineStripView skeleton — panel chrome, hide/show toggle, horizontal scroll container | EP-015 | 🔵 Backlog |
| T-0134 | Placeholder graphical event markers — static stub layout | EP-015 | 🔵 Backlog |
| T-0135 | Timeline integration into ManuscriptEditorView — docking, height, collapse | EP-015 | 🔵 Backlog |
| T-0136 | iPhone exclusion — Timeline absent on phone idiom | EP-015 | 🔵 Backlog |
| T-0137 | EP-015 verification | EP-015 | 🔵 Backlog |

---

## Full Detail — Implemented Tasks Returned to Backlog

---

## T-0123: iPhone Conditional — Restore Toolbar Buttons on Phone Idiom

**Status:** 🟡 Implemented - Not Verified
**Component:** `EditorView.swift`
**Priority:** Medium
**Date Requested:** 2026-06-09
**Date Implemented:** 2026-06-09
**Date Verified:** —
**Sprint Assigned:** SP-035 (returned to backlog before verification)
**Epic:** EP-012

**Rationale:**
EP-012 removed the custom toolbar strip from `ManuscriptEditorView` and replaced it with macOS/iPadOS menu bar commands. On iPhone, no menu bar is available, so the Close Project and Project Settings buttons must remain as a visible toolbar in the view. The implementation uses a platform conditional so the toolbar is only compiled and shown on the phone idiom.

**Current Behavior (pre-implementation):**
The toolbar strip was removed entirely for all platforms, leaving iPhone with no way to access Project Settings or Close Project.

**Desired Behavior:**
On iPhone (`UIDevice.current.userInterfaceIdiom == .phone`), a toolbar with Project Settings and Close Project buttons appears above the manuscript surface. On macOS and iPadOS, no toolbar is shown — those platforms use the menu bar.

**Requirements:**
1. `#if os(iOS)` conditional compiles the phone toolbar only on iOS builds
2. `UIDevice.current.userInterfaceIdiom == .phone` runtime check gates the toolbar to phone only — iPadOS does not show it
3. Toolbar contains: Project Settings button (triggers `env.showProjectSettings = true`) and Close Project button (calls `env.closeProject()`)
4. Toolbar matches the visual style of the removed toolbar (`.borderless`, `.callout` font, trailing-aligned, standard padding)
5. A `Divider()` separates the toolbar from the manuscript surface

**Design Approach:**
Conditional compilation via `#if os(iOS)` wraps a `phoneToolbar` computed property in `ManuscriptEditorView`. The property uses a runtime `UIDevice` check so it does not appear on iPadOS. Placed at the top of the detail `VStack`, above `ManuscriptTextView`.

**Components Affected:**
- `Scrivi/Views/EditorView.swift` — `phoneToolbar` computed property added under `#if os(iOS)`

**Implementation Details:**
Added `#if os(iOS)` block inside the detail `VStack` in `ManuscriptEditorView.body`:

```swift
#if os(iOS)
if UIDevice.current.userInterfaceIdiom == .phone {
    phoneToolbar
    Divider()
}
#endif
```

Added `phoneToolbar` computed property under `#if os(iOS)`:

```swift
#if os(iOS)
private var phoneToolbar: some View {
    HStack {
        Spacer()
        Button("Project Settings") { env.showProjectSettings = true }
            .buttonStyle(.borderless)
            .font(.callout)
        Button("Close Project") { env.closeProject() }
            .buttonStyle(.borderless)
            .font(.callout)
    }
    .padding(.horizontal, 12)
    .padding(.vertical, 8)
}
#endif
```

**Test Steps:**
1. Build for iPhone simulator
2. Open a project — confirm Project Settings and Close Project buttons appear above the manuscript
3. Tap Project Settings — confirm settings sheet opens
4. Tap Close Project — confirm returns to Landing View
5. Build for iPad simulator — confirm no toolbar appears (menu bar only)
6. Build for macOS — confirm no toolbar appears

**Notes:**
Returned to backlog before iPhone testing was available. macOS and iPadOS targets verified under T-0120. iPhone verification requires an iPhone build target, which is not yet part of the active test workflow.

---

*Last Updated: 2026-06-09 (T-0123 returned to backlog with full detail; index restructured)*
