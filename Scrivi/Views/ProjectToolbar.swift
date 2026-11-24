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
    /// Width compact on iOS: the Navigator is a page too (SP-242, View▾ ▸ Scene Navigator).
    var showNavigatorPage: (() -> Void)? = nil

    // ⚠️ ORDERED BY USE, ONE SHAPE ON EVERY APPLE PLATFORM (user ruling 2026-11-03, SP-199). As the width shrinks the
    // system moves items into its ⋯ overflow from the END, so the most-used come first and stay longest. ✅ It supersedes
    // the 2026-09-22 per-object Scene / Chapter `ControlGroup`s: verbs are grouped by what they do (Navigate, New, Merge),
    // two clicks each, and every one keeps its menu-bar item and shortcut. ✅ Manuscript Start/End return to the toolbar
    // inside the labelled Navigate menu, where [T-0568]'s misreading as Scene Start/End cannot happen.
    // ⚠️ EVERY ITEM CALLS AN EXISTING CLOSURE: the toolbar is a SURFACE for what the menu bar already does (EP-040 AC2).
    // ✅ EP-060 SP-242 (rulings (e), (f), Q3; user, 2026-11-24: "making it a bit smaller and better organized. same for all
    // platforms"): 14 items → 10. Import▾ + Export▾ → ONE Import/Export▾ with an Import and an Export submenu; Project Settings,
    // Inkwyn Settings, Purge and About → ONE Tools▾. A glyph with diamond pips marks a button that opens a menu.
    // ⚠️ Toolbar labels go through `toolbarLabel` (Q2): the 2× glyphs are too big for a toolbar button as symbols.
    // ⚠️ Menu CONTENT is `.titleAndIcon` (Q1): a menu may give its items another default label style and drop the icon (I-0035).
    // The style goes on the content, never on the Menu: there it would put titles on the toolbar buttons.
    var body: some ToolbarContent {
        // ✅ View▾ is LEADING and holds the panels (user, 2026-11-24): Scene Navigator, Scene Inspector, Timeline, then the overlays.
        // Width compact (iPhone) has pages, not panels, so the first two go to the page instead of toggling a column.
        ToolbarItem(placement: placement) {
            Menu {
                Group {
                    menuHeader("View", "inkwyn.menu.view")
                    #if os(macOS)
                    // The SAME actions as the menu bar's View items: Show/Hide Sidebar is the system's `toggleSidebar:`
                    // (NavigationSplitView's own item), and the other two flip the value as View ▸ Show or Hide … does.
                    Button { NSApp.sendAction(#selector(NSSplitViewController.toggleSidebar(_:)), to: nil, from: nil) } label: {
                        Label("Show or Hide Scene Navigator", image: "inkwyn.left.sidebar")
                    }
                    Button { session.inspectorVisible.toggle() } label: {
                        Label("Show or Hide Scene Inspector", image: "inkwyn.right.sidebar")
                    }
                    Button { session.timelineVisible.toggle() } label: {
                        Label("Show or Hide Timeline", image: "inkwyn.timeline")
                    }
                    #else
                    if let showNavigatorPage, let showInspectorPage {
                        Button(action: showNavigatorPage) { Label("Scene Navigator", image: "inkwyn.left.sidebar") }
                        Button(action: showInspectorPage) { Label("Scene Inspector", image: "inkwyn.right.sidebar") }
                    } else {
                        Toggle(isOn: Bindable(session).navigatorVisible) {
                            Label("Show Scene Navigator", image: "inkwyn.left.sidebar")
                        }
                        Toggle(isOn: Bindable(session).inspectorVisible) {
                            Label("Show Scene Inspector", image: "inkwyn.right.sidebar")
                        }
                    }
                    Toggle(isOn: Bindable(session).timelineVisible) {
                        Label("Show Timeline", image: "inkwyn.timeline")
                    }
                    #endif
                    Divider()
                    Toggle(isOn: Bindable(session).worldWarningVisible) {
                        Label("World Warnings", image: "inkwyn.warnings")
                    }
                    // SP-227 Q2: the iPhone has no menu bar for ⇧⌘H.
                    if let prefs = session.projectPreferences {
                        Toggle(isOn: Bindable(prefs).markupHints) {
                            Label("Markup Hints", image: "inkwyn.hints")
                        }
                    }
                    #if os(macOS)
                    // ⚠️ Buffers visibility is APP-GLOBAL: one palette follows the frontmost project.
                    Toggle(isOn: Bindable(env).buffersPaletteVisible) {
                        Label("Buffers", image: "inkwyn.buffers")
                    }
                    #endif
                }
                .labelStyle(.titleAndIcon)
            } label: {
                toolbarLabel("View", "inkwyn.menu.view")
            }
            .menuIndicator(.hidden)
            .help("Show or hide the Navigator, Inspector and Timeline; view options")
        }

        ToolbarItem(placement: placement) {
            // Its glyph has no pips: it opens a sheet, not a menu (user, 2026-11-24).
            Button { session.showWorlds = true } label: {
                toolbarLabel("Worlds", "inkwyn.menu.worlds")
            }
            .help("Manage Worlds…")
        }

        ToolbarItem(placement: placement) {
            Menu {
                Group {
                    menuHeader("Navigate", "inkwyn.menu.navigate")
                    item("Scene Start", "inkwyn.scene.start", session.sceneStartAction)
                    item("Scene End", "inkwyn.scene.end", session.sceneEndAction)
                    item("Chapter Start", "inkwyn.chapter.start", session.chapterStartAction)
                    item("Chapter End", "inkwyn.chapter.end", session.chapterEndAction)
                    item("Manuscript Start", "inkwyn.manuscript.start", session.manuscriptStartAction)
                    item("Manuscript End", "inkwyn.manuscript.end", session.manuscriptEndAction)
                }
                .labelStyle(.titleAndIcon)
            } label: {
                toolbarLabel("Navigate", "inkwyn.menu.navigate")
            }
            .menuIndicator(.hidden)   // the glyph's pips already say "menu"
            .help("Go to a scene, chapter or manuscript start or end")
        }

        ToolbarItem(placement: placement) {
            Menu {
                Group {
                    menuHeader("New", "inkwyn.menu.create")
                    item("New Scene", "inkwyn.new.scene", session.createSceneAction)
                    item("New Chapter", "inkwyn.new.chapter", session.createChapterAction)
                }
                .labelStyle(.titleAndIcon)
            } label: {
                toolbarLabel("New", "inkwyn.menu.create")
            }
            .menuIndicator(.hidden)   // the glyph's pips already say "menu"
            .help("New Scene (⌘↩) or New Chapter (⇧⌘↩)")
        }

        ToolbarItem(placement: placement) {
            Menu {
                Group {
                    menuHeader("Merge", "inkwyn.menu.merge")
                    item("Merge Scene with Previous", "inkwyn.merge.scene.with.previous", session.mergeSceneAction)
                    item("Merge Chapter with Previous", "inkwyn.merge.chapter.with.previous", session.mergeChapterAction)
                }
                .labelStyle(.titleAndIcon)
            } label: {
                toolbarLabel("Merge", "inkwyn.menu.merge")
            }
            .menuIndicator(.hidden)   // the glyph's pips already say "menu"
            .help("Merge Scene (⌘⌫) or Merge Chapter (⇧⌘⌫) with the previous one")
        }

        // [SP-206] EP-040's Export slot, [SP-215] File ▸ Export plus Copy as HTML, SP-227 Q2 Import beside it — now one button
        // (ruling (e)): the iPhone still reaches both without a menu bar.
        ToolbarItem(placement: placement) {
            Menu {
                Group {
                    menuHeader("Import/Export", "inkwyn.menu.import.export")
                    Menu {
                        ImportMenuItems(env: env, session: session)
                    } label: {
                        Label("Import", image: "inkwyn.import")
                    }
                    Menu {
                        ExportMenuItems(env: env, session: session)
                        Divider()
                        Button("Copy as HTML") { Task { await env.copyManuscriptAsHTML(session) } }
                    } label: {
                        Label("Export", image: "inkwyn.export")
                    }
                }
                .labelStyle(.titleAndIcon)
            } label: {
                toolbarLabel("Import/Export", "inkwyn.menu.import.export")
            }
            .menuIndicator(.hidden)   // the glyph's pips already say "menu"
            .help("Import or Export")
        }

        // Ruling (f): Project Settings (⌥⌘,), Inkwyn Settings, Purge and About in one menu. Items without an Inkwyn glyph keep
        // their system symbol (Q5).
        ToolbarItem(placement: placement) {
            Menu {
                Group {
                    menuHeader("Tools", "inkwyn.menu.tools")
                    Button { session.showProjectSettings = true } label: {
                        Label("Project Settings…", systemImage: "gearshape")
                    }
                    #if os(macOS)
                    // The Mac's Inkwyn Settings is the app's Settings scene (Inkwyn ▸ Settings…, ⌘,).
                    SettingsLink { Label("Inkwyn Settings…", systemImage: "gearshape.2") }
                    #else
                    Button { env.showAppSettings = true } label: {
                        Label("Inkwyn Settings…", systemImage: "gearshape.2")
                    }
                    #endif
                    Button { session.showStaleBranches = true } label: {
                        Label("Purge Stale History Branches…", systemImage: "arrow.triangle.branch")
                    }
                    Divider()
                    Button {
                        #if os(macOS)
                        AboutPanel.show()
                        #else
                        env.showAbout = true
                        #endif
                    } label: {
                        Label("About Inkwyn", image: "inkwyn.info")
                    }
                }
                .labelStyle(.titleAndIcon)
            } label: {
                toolbarLabel("Tools", "inkwyn.menu.tools")
            }
            .menuIndicator(.hidden)   // the glyph's pips already say "menu"
            .help("Project Settings, Inkwyn Settings, Purge Stale History Branches, About Inkwyn")
        }

        ToolbarItem(placement: placement) {
            Button(role: .destructive) { closeProject() } label: {
                toolbarLabel("Close Project", "inkwyn.menu.close")
            }
            .help("Close Project (⌘W)")
        }

        // ⚠️ DECLARED SLOTS — EP-040 AC7 / AC5: Export and the text-size lens; ⛔ not built in this Epic.
    }

    /// A toolbar button's label. Mac: the LARGE drawing aspect-fit in a box tied to the text size (user, 2026-11-24). iOS: the
    /// Small drawing as a symbol — the iPhone's items live in the ⋯ overflow MENU, which takes the image alone, not a fitted view.
    @ViewBuilder private func toolbarLabel(_ title: String, _ glyph: String) -> some View {
        #if os(macOS)
        Label { Text(title) } icon: { FittedGlyph(name: glyph) }
        #else
        Label(title, image: glyph).imageScale(.small)
        #endif
    }

    /// The top of a toolbar menu: the button it opened from, then a divider. Choosing it does nothing, and choosing any item
    /// closes a menu, so it dismisses the menu as a tap outside would (user, 2026-11-24 — as iOS shows an overflowed item's
    /// submenu). Width compact (iPhone) leaves it out: its items are always in the ⋯ overflow, where the system already shows
    /// this header. ⚠️ An iPad item the system overflows into ⋯ shows both.
    @ViewBuilder private func menuHeader(_ title: String, _ glyph: String) -> some View {
        if showInspectorPage == nil {
            Button {} label: { Label(title, image: glyph) }
            Divider()
        }
    }

    /// A menu entry for an existing closure, disabled where the platform has not wired it yet (SP-199 AC3).
    private func item(_ title: String, _ glyph: String, _ action: (() -> Void)?) -> some View {
        Button { action?() } label: { Label(title, image: glyph) }
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
