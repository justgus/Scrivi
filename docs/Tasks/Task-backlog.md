# Task Backlog

Tasks listed here are documented and ready for Sprint assignment.

New, unstarted tasks are listed as summary rows. Tasks that have been implemented but returned to the backlog before verification retain their full detail below the index table so no documentation is lost.

---

## Index

| Task | Title | Epic | Status |
| ---- | ----- | ---- | ------ |
| T-0118 | Scroll bar fidelity — per-scene character-ratio thumb position and size | EP-011 | 🔵 Backlog |
| T-0123 | iPhone conditional — restore toolbar buttons on compact/phone idiom | EP-012 | 🟡 Implemented - Not Verified |
| T-0139 | `TimelineMetaJson` schema — read/write `timeline.meta.json` | EP-016 | 🔵 Backlog |
| T-0140 | `SceneMetaJson` extension — `storyTime` block read/write | EP-016 | 🔵 Backlog |
| T-0141 | `StoryStructureJson` schema — read/write `story-structure.json` | EP-016 | 🔵 Backlog |
| T-0142 | `HistoricalEventJson` schema — read/write `historical-events/<id>.json` | EP-016 | 🔵 Backlog |
| T-0143 | `ExternalTimelineJson` schema — read/write imported timeline files | EP-016 | 🔵 Backlog |
| T-0144 | ScriviCore facade — timeline, story-time, and band-assignment operations | EP-016 | 🔵 Backlog |
| T-0145 | ScriviCore facade — story structure, historical events, imported timelines, export | EP-016 | 🔵 Backlog |
| T-0146 | C API + `ScriviEngine.swift` — expose all timeline operations | EP-016 | 🔵 Backlog |
| T-0147 | Integration tests — timeline persistence round-trip | EP-016 | 🔵 Backlog |
| T-0148 | `createProject` update — write `timeline.meta.json` on new project | EP-016 | 🔵 Backlog |
| T-0156 | Story Structure band overlay — colored bands, label row, Structure selector menu | EP-016 | 🔵 Backlog |
| T-0157 | Band border drag — proportional resize, persistence | EP-016 | 🔵 Backlog |
| T-0158 | Band assignment by drag-up to label — ring color on dot | EP-016 | 🔵 Backlog |
| T-0159 | Band assignment via context menu ("Assign to Act…" / "Unassign") | EP-016 | 🔵 Backlog |
| T-0160 | Scene dot context menu — full menu | EP-016 | 🔵 Backlog |
| T-0161 | Historical event CRUD — author in project, drag on timeline, `#C8A97A` dot | EP-016 | 🔵 Backlog |
| T-0162 | Imported timeline row — render events as grey row below project row | EP-016 | 🔵 Backlog |
| T-0163 | Epoch offset dialog — import flow with window intersection preview | EP-016 | 🔵 Backlog |
| T-0164 | Multiple imported timeline rows — distinct grey shades, hide/show toggle | EP-016 | 🔵 Backlog |
| T-0165 | Timeline export — produce `.scrivi-timeline.json` | EP-016 | 🔵 Backlog |
| T-0166 | Co-located dot clustering — hexagonal ring layout, count badge | EP-016 | 🔵 Backlog |
| T-0167 | EP-016 verification | EP-016 | 🔵 Backlog |

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

*Last Updated: 2026-06-10 (T-0128–T-0132 activated for SP-037)*
