import SwiftUI

// The project toolbar, shared by the Mac window and the iOS editor (SP-199 T-0638). iOS places it where the system does —
// the top on iPad and iPhone, the side on the Duo — with no placement code ([SP-198] R3).
struct ProjectToolbar: ToolbarContent {
    let session: ProjectSession
    let env: AppEnvironment

    /// iOS: the system's secondary slot. With the editor role the iPad shows these in the middle of the bar and overflows
    /// them into the system ⋯ menu when the width runs out; the iPhone keeps them in that menu (SP-199 live pass: as
    /// primary items they pushed Worlds, Settings and ⋯ off the bar). The Mac keeps its own placement.
    #if os(iOS)
    private let placement = ToolbarItemPlacement.secondaryAction
    #else
    private let placement = ToolbarItemPlacement.automatic
    #endif

    // MARK: — The window toolbar (EP-040 SP-134 / T-0543, app-shape study S1+S2)
    //
    // ⚠️ WHY IT IS DECLARED HERE, IN SWIFTUI, AND NOT AS AN AppKit `NSToolbar` (ruled 2026-09-22).
    // The project window IS an AppKit `NSWindow`, so an `NSToolbar` was the obvious-looking
    // choice. ⛔ It is the wrong one: `ScriviApp.swift` records that "AppKit NSWindows don't feed
    // SwiftUI's @FocusedValue", which is why the MENU BAR has to reach the session through
    // `env.frontmostSession`. An `NSToolbar` would need that same workaround for EVERY item — a
    // second hand-maintained path to state this view already holds directly.
    // ✅ Declared here, `session` is simply in scope.
    //
    // ⚠️ EVERY ITEM CALLS AN EXISTING CLOSURE. Nothing here re-implements a verb; the toolbar is a
    // SURFACE for what the menu bar already does (EP-040 AC2), and the two can never disagree
    // because there is one code path.
    /// Width compact on iOS: the Inspector is a page, so its item opens it instead of toggling a column (SP-199 G3).
    var showInspectorPage: (() -> Void)? = nil

    // ⚠️ ORDERED BY USE, ONE SHAPE ON EVERY APPLE PLATFORM (user ruling 2026-11-03, SP-199). As the width shrinks the
    // system moves items into its ⋯ overflow from the END, so the most-used come first and stay longest. ✅ It supersedes
    // the 2026-09-22 per-object Scene / Chapter `ControlGroup`s: verbs are grouped by what they do (Navigate, New, Merge),
    // two clicks each, and every one keeps its menu-bar item and shortcut. ✅ Manuscript Start/End return to the toolbar
    // inside the labelled Navigate menu, where [T-0568]'s misreading as Scene Start/End cannot happen.
    // ⚠️ EVERY ITEM CALLS AN EXISTING CLOSURE: the toolbar is a SURFACE for what the menu bar already does (EP-040 AC2).
    var body: some ToolbarContent {
        ToolbarItem(placement: placement) {
            if let showInspectorPage {
                Button(action: showInspectorPage) { Label("Inspector", systemImage: "sidebar.trailing") }
            } else {
                Toggle(isOn: Bindable(session).inspectorVisible) { Label("Inspector", systemImage: "sidebar.trailing") }
                    .help("Show or hide the Scene Inspector")
            }
        }

        ToolbarItem(placement: placement) {
            Toggle(isOn: Bindable(session).timelineVisible) {
                Label("Timeline", systemImage: "calendar.day.timeline.left")
            }
            .help("Show or hide the Timeline")
        }

        ToolbarItem(placement: placement) {
            Button { session.showProjectSettings = true } label: {
                Label("Project Settings", systemImage: "gearshape")
            }
            .help("Project Settings… (⌘,)")
        }

        ToolbarItem(placement: placement) {
            Button { session.showWorlds = true } label: {
                Label("Worlds", systemImage: "globe")
            }
            .help("Manage Worlds…")
        }

        ToolbarItem(placement: placement) {
            Menu {
                item("Scene Start", "arrow.up.to.line.compact", session.sceneStartAction)
                item("Scene End", "arrow.down.to.line.compact", session.sceneEndAction)
                // ✅ `.circle` is Chapter's "enclosed" variant (user, live pass 2026-09-22).
                item("Chapter Start", "arrow.up.to.line.circle", session.chapterStartAction)
                item("Chapter End", "arrow.down.to.line.circle", session.chapterEndAction)
                item("Manuscript Start", "arrow.up.to.line", session.manuscriptStartAction)
                item("Manuscript End", "arrow.down.to.line", session.manuscriptEndAction)
            } label: {
                Label("Navigate", systemImage: "arrow.up.arrow.down")
            }
            .help("Go to a scene, chapter or manuscript start or end")
        }

        ToolbarItem(placement: placement) {
            Menu {
                item("New Scene", "plus.rectangle", session.createSceneAction)
                item("New Chapter", "plus.rectangle.on.folder", session.createChapterAction)
            } label: {
                Label("New", systemImage: "plus")
            }
            .help("New Scene (⌘↩) or New Chapter (⇧⌘↩)")
        }

        ToolbarItem(placement: placement) {
            Menu {
                item("Merge Scene with Previous", "arrow.triangle.merge", session.mergeSceneAction)
                // ✅ Its own glyph, so Merge Chapter never reads as Merge Scene (user, live pass 2026-09-22).
                item("Merge Chapter with Previous", "rectangle.compress.vertical", session.mergeChapterAction)
            } label: {
                Label("Merge", systemImage: "arrow.triangle.merge")
            }
            .help("Merge Scene (⌘⌫) or Merge Chapter (⇧⌘⌫) with the previous one")
        }

        ToolbarItem(placement: placement) {
            Menu {
                Toggle(isOn: Bindable(session).worldWarningVisible) {
                    Label("World Warnings", systemImage: "exclamationmark.triangle")
                }
                #if os(macOS)
                // ⚠️ Buffers visibility is APP-GLOBAL: one palette follows the frontmost project.
                Toggle(isOn: Bindable(env).buffersPaletteVisible) {
                    Label("Buffers", systemImage: "doc.on.clipboard")
                }
                #endif
            } label: {
                Label("View", systemImage: "eye")
            }
            .help("View options")
        }

        ToolbarItem(placement: placement) {
            Button(role: .destructive) { closeProject() } label: {
                Label("Close Project", systemImage: "xmark")
            }
            .help("Close Project (⌘W)")
        }

        // ⚠️ DECLARED SLOTS — EP-040 AC7 / AC5: Export and the text-size lens; ⛔ not built in this Epic.
    }

    /// A menu entry for an existing closure, disabled where the platform has not wired it yet (SP-199 AC3).
    private func item(_ title: String, _ symbol: String, _ action: (() -> Void)?) -> some View {
        Button { action?() } label: { Label(title, systemImage: symbol) }
            .disabled(action == nil)
    }

    private func closeProject() {
        guard let pid = session.openProjectResult?.projectID else { return }
        #if os(macOS)
        // As the menu bar's Close Project: close the window; its windowWillClose tears the session down.
        env.windows.closeWindow(projectID: pid)
        #else
        env.closeProject(projectID: pid)
        #endif
    }
}
