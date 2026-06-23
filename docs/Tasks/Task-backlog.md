# Task Backlog

Tasks listed here are documented and ready for Sprint assignment.

New, unstarted tasks are listed as summary rows. Tasks that have been implemented but returned to the backlog before verification retain their full detail below the index table so no documentation is lost.

---

## Index

| Task | Title | Epic | Status |
| ---- | ----- | ---- | ------ |
| T-0118 | Scroll bar fidelity — per-scene character-ratio thumb position and size | EP-011 | 🔵 Backlog |
| T-0123 | iPhone conditional — restore toolbar buttons on compact/phone idiom | EP-012 | 🟡 Implemented - Not Verified |
| T-0175 | Spotlight integration — Core Spotlight donations + on-disk `.scrivi` importer extension | TBD | 🔵 Backlog (design-gated) |

## Full Detail — Implemented Tasks Returned to Backlog

---

## T-0175: Spotlight Integration — Core Spotlight Donations + On-Disk Importer Extension

**Status:** 🔵 Backlog (design-gated)
**Component:** ScriviCore (read-only indexing facade), `Scrivi/App` (donations + deep-link),
new Spotlight importer app-extension target
**Priority:** Medium-High (core search capability for a writing app)
**Date Requested:** 2026-06-23
**Epic:** TBD (candidate: search/discoverability epic)
**Design Reference:** `docs/Scrivi_Spotlight_Integration_Design_v0_1.md`

**Rationale:**
Scrivi is an authoring/worldbuilding app; finding the project and scene where something was
written, from system Spotlight, with the app closed, is a first-class capability. Surfaced
while investigating benign `CSInlineDonation … SetStoreUpdateService was invalidated` console
errors (OS auto-donation rejected by the indexing daemon). Decision (per user): implement
Spotlight support properly rather than suppress the donation path.

**Scope (approved): both layers**
1. **Layer 1 — in-app Core Spotlight donations** (`CSSearchableItem`) for project, scenes, and
   worldbuilding objects; deep-link results back into Scrivi at the item.
2. **Layer 2 — on-disk Spotlight importer** (`CSImportExtension`) so the OS indexes `.scrivi`
   package contents at the filesystem level even when Scrivi is not running.

**Design gates (MUST resolve before implementation — see design doc §3, §3a):**
- **Boundary:** the importer extension is a separate OS-launched process that must read project
  content. Recommended **Option A** — extension links ScriviCore via a new read-only
  `extractSearchableText(projectPath) -> JSON` facade, preserving "ScriviCore owns all project
  I/O; Swift = UI only." (Option B — parse JSON in Swift — violates the architecture; rejected.)
- **Structural discrepancy:** CLAUDE.md says the adapter is at `Scrivi/Adapter/` with no SPM
  package, but it currently builds under `platforms/apple/` (SPM-style). Confirm the real
  adapter/ScriviCore build graph before deciding how the extension links ScriviCore.

**Milestones (from design doc §7):**
1. Design sign-off (resolve gates) — no code.
2. ScriviCore indexing facade + unit tests against a fixture project.
3. Layer 1: donations on open/save, delete-by-domain on close, deep-link continuation.
4. Layer 2: importer extension target linking ScriviCore.
5. Verification: Spotlight finds content with app closed; result opens Scrivi at the item; the
   original `CSInlineDonation` errors no longer appear (donations now succeed).

**pbxproj note:** a new app-extension target plus its source files must be added to
`Scrivi.xcodeproj/project.pbxproj` in the same step as creating them (CLAUDE.md rule).

**Notes:**
Routed as a standalone task with a design pass; explicitly **not** folded into T-0167 (EP-016
verification). Implementation blocked until the two design gates above are signed off.

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

*Last Updated: 2026-06-23 (T-0175 Spotlight integration added — design-gated; design doc Scrivi_Spotlight_Integration_Design_v0_1.md created)*
