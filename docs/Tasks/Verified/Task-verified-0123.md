# T-0123 — Verified

## T-0123: iPhone Conditional — Restore Toolbar Buttons on Phone Idiom

**Status:** ✅ Resolved - Verified (2026-06-29, user-confirmed on iPhone 17 Pro / iOS 27.0)
**Component:** `EditorView.swift`, `SceneNavigatorView.swift`, `ScriviApp.swift`, `AppEnvironment.swift`, `LandingView.swift`
**Priority:** Medium
**Date Requested:** 2026-06-09
**Date Implemented:** 2026-06-09 (original phoneToolbar); reworked 2026-06-28/29
**Date Verified:** 2026-06-29
**Sprint Assigned:** SP-035 (returned to backlog before verification)
**Epic:** EP-012 (satisfies AC5)

> **Verified solution differs from the original design below.** When verified live on an
> iPhone 17 Pro (iOS 27.0), the original `phoneToolbar` HStack-in-detail-column approach was found
> not to work: the iPhone landed in a chrome-less detail with no navigation and no toolbar (a leaked
> macOS `minWidth: 700` frame pushed the split-view sidebar off the ~393pt screen), and the Project
> Settings button was never wired to a sheet on iOS. The phoneToolbar was replaced with a proper
> **iOS Master/Detail** solution — see the **2026-06-29 Resolution** addendum at the end of this entry.

**Rationale:**
EP-012 removed the custom toolbar strip from `ManuscriptEditorView` and replaced it with macOS/iPadOS menu bar commands. On iPhone, no menu bar is available, so the Close Project and Project Settings buttons must remain reachable in-view.

**Original Design (superseded):**
Conditional compilation via `#if os(iOS)` wrapped a `phoneToolbar` computed property in
`ManuscriptEditorView`, gated at runtime to `UIDevice.current.userInterfaceIdiom == .phone`, placed
above `ManuscriptTextView`. This did not survive live verification (see Status note).

**Environment note — where the iOS simulators live (2026-06-28):**
The iPhone/iPad simulators now live inside the **Xcode DeviceHub**, not the legacy standalone
Simulator-runtimes path. Boot the iPhone/iPad 27.0 simulator from the DeviceHub for this verification.

---

### Resolution (2026-06-29 — iOS Master/Detail rework; Verified)

The original phone-toolbar design did not survive live verification (see the note under Status).
The shipped solution unifies iPhone + iPad on one Master/Detail layout and adds the iPad menu bar:

1. **`EditorView.swift` — one `NavigationSplitView` for all iOS idioms.** It adapts on its own:
   side-by-side Master/Detail on iPad and iPhone landscape (regular width), and a
   navigator→manuscript push with a system back chevron on iPhone portrait (compact width). The
   leaked macOS `minWidth: 700` constraint is now `#if os(macOS)`-only (it was forcing the sidebar
   off the iPhone screen). A selection binding (`selectedSceneID`) makes the detail track the
   master and drives the compact-width push; on open it seeds from the restored viewport scene,
   else the first scene of chapter one.
2. **iOS nav-bar `•••` menu** (`editorMenuToolbar`, `ToolbarItem(.primaryAction)`) on the detail's
   navigation bar — the modern equivalent of the removed toolbar buttons — with Project Settings…,
   Show Scene Inspector, Show Timeline, and Close Project. Replaces the in-view `phoneToolbar`.
3. **Project Settings sheet wired on iOS** (`EditorView` `.sheet(isPresented: showProjectSettings)`)
   — previously the button set the flag but nothing presented the sheet on iOS.
4. **`SceneNavigatorView.swift`** — gains an optional `selection` binding so the List selection is
   the navigation source of truth on iOS (macOS keeps tap-to-navigate); the highlight/selection
   syncs two-way with manuscript scroll (loop-guarded).
5. **iPad menu bar** (`ScriviApp.iosCommands`, `.commands` on the iOS `WindowGroup`) — File
   (New/Open/Close), Project (Settings, Show Inspector ⇧⌘I, Show Timeline ⇧⌘T). Deconflicted
   against the iOS-synthesized menu bar: no ⌘, (reserved), no second "View" menu, ⇧⌘ toggles
   (⌥⌘I/⌥⌘T collided). Closes **I-0054**.
6. **New/Open on iOS close the open project first** (`AppEnvironment.presentNewProject` /
   `presentOpenImporter` → `closeActiveProjectForLanding`) so the single-scene Landing host can
   present the sheet/importer — fixes "Unable to open a window when the app does not support
   multiple scenes". New `requestOpenImporter` flag bridges the menu's Open to `LandingView`'s
   `.fileImporter`.

**Verified (user, 2026-06-29):** iPhone 17 Pro / iOS 27.0 — navigation handles + ••• button bar
appear, Project Settings opens. iPad Pro / iOS 27.0 — File/Project menus populated and functional,
menu-conflict console spam gone, New/Open close-then-present works. "Save as you go" — no explicit
save UI by design.

**Related:** I-0054 (iPad reachability) — Resolved - Verified 2026-06-29 in the same work; EP-012 closed.
