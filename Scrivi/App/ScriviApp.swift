import SwiftUI
#if canImport(CoreSpotlight)
import CoreSpotlight
#endif

// App-level delegate for reliable URL delivery on macOS. SwiftUI's per-view
// `.onOpenURL` is delivered to at most one scene and is unreliable with a
// Window + WindowGroup split across multiple windows (evidence: deep links stopped
// arriving after the first window interaction). `application(_:open:)` receives every
// scrivi:// URL at the process level regardless of window state (EP-018 / T-0194).
#if os(macOS)
final class AppDelegate: NSObject, NSApplicationDelegate {
    // Set by ScriviApp at launch; routes incoming URLs to the environment. Main-actor
    // isolated — both the setter (in a MainActor task) and application(_:open:) (called
    // on the main thread by AppKit) touch it from the main actor.
    @MainActor static var onOpenURLs: (([URL]) -> Void)?

    // Set by ScriviApp at launch; called when the app is about to quit so the session
    // manifest can be frozen to the still-open set BEFORE windows tear down (R4 / T-0195).
    @MainActor static var onWillTerminate: (() -> Void)?

    func application(_ application: NSApplication, open urls: [URL]) {
        MainActor.assumeIsolated {
            AppDelegate.onOpenURLs?(urls)
        }
    }

    func applicationWillTerminate(_ notification: Notification) {
        MainActor.assumeIsolated {
            AppDelegate.onWillTerminate?()
        }
    }

    // Do NOT quit when the last project window closes — Scrivi returns to the Welcome
    // window instead. Without this, closing the last project terminated the app before
    // the Welcome-reopen could run (EP-018 / T-0194).
    func applicationShouldTerminateAfterLastWindowClosed(_ sender: NSApplication) -> Bool {
        false
    }
}
#endif

@main
struct ScriviApp: App {

    #if os(macOS)
    @NSApplicationDelegateAdaptor(AppDelegate.self) private var appDelegate
    #endif

    @State private var env = AppEnvironment()
    @State private var showAbout = false

    init() {
        // Populate the Scene Inspector card registry once (EP-030 SP-090). The layout
        // file stores typeIDs, so a card must be registered before its entries resolve;
        // unregistered typeIDs are reported, not dropped.
        InspectorCardRegistry.registerBuiltIns()
    }

    #if os(macOS)
    // The session of the frontmost project window — drives the project/view menus.
    // Tracked in AppEnvironment (set by each AppKit window when it becomes key), because
    // AppKit NSWindows don't feed SwiftUI's @FocusedValue.
    private var focusedSession: ProjectSession? { env.frontmostSession }
    #endif

    var body: some Scene {
        #if os(macOS)
        // macOS: the only SwiftUI scene is the Welcome/Landing window. Project editor windows
        // are AppKit NSWindows owned by ProjectWindowManager (deterministic lifecycle;
        // WindowGroup(for:) was abandoned because it cached dead windows — T-0194).
        Window("Welcome to Scrivi", id: "welcome") {
            WelcomeWindowRoot()
                .environment(env)
                .task { await launchSetup() }
                .modifier(AppEventsModifier(env: env))
                .sheet(isPresented: $showAbout) { AboutView() }
        }
        .defaultSize(width: 720, height: 480)
        .commands { appCommands }
        #else
        // iOS/iPadOS/visionOS: single-window model (the macOS AppKit multi-window model and
        // menu bar do not apply — the full per-platform window UX is deferred, EP-018 non-goal).
        // One WindowGroup hosts the editor for the active project, or Landing when none is open.
        WindowGroup {
            iOSRootView()
                .environment(env)
                .task { await launchSetup() }
                .modifier(AppEventsModifier(env: env))
                .sheet(isPresented: $showAbout) { AboutView() }
        }
        // iPad hardware-keyboard menu bar. Deconflicted against iOS's auto-synthesized menus
        // (no ⌘, no second "View" menu, ⇧⌘ view-toggle shortcuts) — see iosCommands. The
        // in-view nav-bar ••• menu remains the primary surface and the only one on iPhone.
        .commands { iosCommands }
        #endif
    }

    // Bootstrap + restore, run once per launch. On macOS the Welcome window's .task re-fires
    // whenever Welcome reopens, so the one-time guard prevents re-opening just-closed projects.
    @MainActor
    private func launchSetup() async {
        guard !env.didLaunchSetup else { return }
        env.didLaunchSetup = true
        await env.bootstrap()
        #if os(macOS)
        // Reliable app-level URL delivery (per-view .onOpenURL was unreliable).
        AppDelegate.onOpenURLs = { urls in
            Task { @MainActor in
                for url in urls { await env.handleDeepLink(url) }
            }
        }
        // Freeze the restore manifest at quit, before windows tear down.
        AppDelegate.onWillTerminate = { env.beginTermination() }
        #endif
        // Restore projects open at last quit (R4 / T-0195). On iOS this surfaces the
        // most-recent project into the single window via the active session.
        await env.restoreOpenProjects()
    }

    // MARK: — Menu bar commands (macOS only — iOS has no menu bar)

    #if os(macOS)
    @CommandsBuilder
    private var appCommands: some Commands {
        // Standard File menu: New / Open / Close, in place of the system new-item group.
        CommandGroup(replacing: .newItem) {
            Button("New Project…") {
                env.presentNewProject()
            }
            .keyboardShortcut("n", modifiers: .command)

            Button("Open Project…") {
                env.presentOpenProjectPanel()
            }
            .keyboardShortcut("o", modifiers: .command)

            Divider()

            Button("Close Project") {
                if let pid = focusedSession?.openProjectResult?.projectID {
                    // Close the AppKit window; its windowWillClose delegate tears down the
                    // session (and reopens Welcome if it was the last).
                    #if os(macOS)
                    env.windows.closeWindow(projectID: pid)
                    #endif
                }
            }
            .keyboardShortcut("w", modifiers: .command)
            .disabled(focusedSession == nil)
        }

        // Project menu: project-scoped settings (acts on the focused window).
        CommandMenu("Project") {
            Button("Project Settings…") {
                focusedSession?.showProjectSettings = true
            }
            .keyboardShortcut(",", modifiers: .command)
            .disabled(focusedSession == nil)
        }

        // Worlds menu (EP-031 SP-099 T-0408) — Doc 2 §7.3.
        //
        // ⚠️ This menu is where a writer SEES and CREATES worlds. Without it there
        // was no way to tell which world a project used, or to make one at all, so
        // the four world-scoped kinds (artifacts, chronicles, factions, rules) were
        // unreachable in the UI.
        //
        // "Remove All World References" is DESTRUCTIVE to pending links and belongs
        // here and nowhere else (§7.3) — never inline on a card, where it would read
        // as routine tidying. It arrives with the pending surface in SP-102.
        CommandMenu("Worlds") {
            Button("Manage Worlds…") {
                focusedSession?.showWorlds = true
            }
            .disabled(focusedSession == nil)
        }

        // Scene / Chapter menus (EP-019 SP-056, T-0214) — mouse-accessible equivalents of
        // the manuscript's structural keyboard commands, for writers who prefer the menu.
        // We do NOT attach the ⌘↩/⌘⌫ key equivalents here: those chords are owned by the
        // editor's proven keyDown path, and a SwiftUI menu key equivalent would intercept
        // the event before keyDown, risking a double-fire (two scenes) or a regression if
        // the menu ever fails to fire. The chord is shown in the title text instead. The
        // menu action routes to the same coordinator handler via the session bridge.
        // Merge acts on the caret's scene/chapter and no-ops when the caret isn't at a
        // valid merge point (start of a non-first scene/chapter), matching the keys.
        CommandMenu("Scene") {
            Button("New Scene  (⌘↩)") { focusedSession?.createSceneAction?() }
                .disabled(focusedSession?.createSceneAction == nil)
            Button("Merge Scene with Previous  (⌘⌫)") { focusedSession?.mergeSceneAction?() }
                .disabled(focusedSession?.mergeSceneAction == nil)

            // ⚠️ No key equivalents yet (2026-08-18, user-raised). ⌘↑/⌘↓ are document
            // start/end, ⌥↑/⌥↓ are paragraph start/end, and ⌃↑/⌃↓ belong to Mission
            // Control — all three with Shift-variants that extend the selection. Taking
            // any of them would cost an existing editing behaviour, so the functions ship
            // as menu items first and the binding stays an open question.
            Divider()
            Button("Go to Scene Start") { focusedSession?.sceneStartAction?() }
                .disabled(focusedSession?.sceneStartAction == nil)
            Button("Go to Scene End") { focusedSession?.sceneEndAction?() }
                .disabled(focusedSession?.sceneEndAction == nil)
        }

        CommandMenu("Chapter") {
            Button("New Chapter  (⇧⌘↩)") { focusedSession?.createChapterAction?() }
                .disabled(focusedSession?.createChapterAction == nil)
            Button("Merge Chapter with Previous  (⇧⌘⌫)") { focusedSession?.mergeChapterAction?() }
                .disabled(focusedSession?.mergeChapterAction == nil)

            Divider()
            Button("Go to Chapter Start") { focusedSession?.chapterStartAction?() }
                .disabled(focusedSession?.chapterStartAction == nil)
            Button("Go to Chapter End") { focusedSession?.chapterEndAction?() }
                .disabled(focusedSession?.chapterEndAction == nil)
        }

        // View menu — toggles act on the focused project window. Inspector/Timeline are
        // per-window (session); the buffers palette is app-global (one panel that follows
        // the frontmost project), so its toggle binds to AppEnvironment, not the session.
        CommandMenu("View") {
            if let session = focusedSession {
                Toggle("Show Scene Inspector", isOn: Bindable(session).inspectorVisible)
                    .keyboardShortcut("i", modifiers: [.command, .option])
                Toggle("Show Timeline", isOn: Bindable(session).timelineVisible)
                    .keyboardShortcut("t", modifiers: [.command, .option])
                // §7.3 (SP-102 R1): its own toggle, NOT tied to the Timeline. The
                // strip still only renders when a world is actually unavailable.
                Toggle("Show World Warnings", isOn: Bindable(session).worldWarningVisible)
                Toggle("Show Buffers", isOn: Bindable(env).buffersPaletteVisible)
                    .keyboardShortcut("b", modifiers: [.command, .option])
            } else {
                Toggle("Show Scene Inspector", isOn: .constant(false))
                    .keyboardShortcut("i", modifiers: [.command, .option])
                    .disabled(true)
                Toggle("Show Timeline", isOn: .constant(false))
                    .keyboardShortcut("t", modifiers: [.command, .option])
                    .disabled(true)
                Toggle("Show World Warnings", isOn: .constant(false))
                    .disabled(true)
                Toggle("Show Buffers", isOn: .constant(false))
                    .keyboardShortcut("b", modifiers: [.command, .option])
                    .disabled(true)
            }
        }

        // Edit menu — copy-buffer discoverability items (EP-019 SP-056, T-0214). The
        // three submenus are the discoverable face of the ⌘/⌃/⌥ + digit chords. We do
        // NOT attach the digit key equivalents here: the chords are owned by the editor's
        // keyDown (a menu key equivalent fires before the responder chain and would
        // shadow the editor path); the chord is named in each submenu's header instead.
        // Clicking a menu item performs that one explicit action on the front project.
        CommandGroup(after: .pasteboard) {
            Divider()
            Menu("Copy To Buffer") {
                Text("⌘1–9 — copy the selection")
                ForEach(BufferService.slotIDs, id: \.self) { slotID in
                    Button("Buffer \(slotID)") {
                        focusedSession?.bufferService?.loadSelectionHandler?(slotID)
                    }
                }
            }
            .disabled(focusedSession?.bufferService == nil)

            Menu("Paste From Buffer") {
                Text("⌃1–9 — paste at the cursor")
                ForEach(BufferService.slotIDs, id: \.self) { slotID in
                    Button("Buffer \(slotID)") {
                        focusedSession?.bufferService?.pasteFromBufferHandler?(slotID)
                    }
                }
            }
            .disabled(focusedSession?.bufferService == nil)

            Menu("Cut To Buffer") {
                Text("⌥1–9 — cut the selection")
                ForEach(BufferService.slotIDs, id: \.self) { slotID in
                    Button("Buffer \(slotID)") {
                        focusedSession?.bufferService?.cutIntoBufferHandler?(slotID)
                    }
                }
            }
            .disabled(focusedSession?.bufferService == nil)
        }

        // About menu.
        CommandGroup(replacing: .appInfo) {
            Button("About Scrivi") { showAbout = true }
        }
        CommandGroup(after: .appInfo) {
            Button("User Manual") { }
                .disabled(true)
        }
    }
    #endif

    // MARK: — iOS/iPadOS menu commands (hardware-keyboard menu bar on iPad)

    // iPadOS surfaces a menu bar when a hardware keyboard is attached. Without an explicit
    // .commands block iPad shows only the generic system menus (empty File, no app actions).
    //
    // Deconflicted against the iOS-synthesized menu bar (the earlier version collided — see the
    // note at the .commands call site):
    //   • No ⌘, on Project Settings — iOS reserves ⌘, for the system "Settings…" item.
    //   • No CommandMenu("View") — iOS already synthesizes a "View" menu; adding our own
    //     duplicated it. The view toggles live in the Project menu instead.
    //   • View toggles use ⇧⌘I / ⇧⌘T (the macOS app uses ⌥⌘I / ⌥⌘T, which collided on iOS).
    // Actions target the single active session.
    @CommandsBuilder
    private var iosCommands: some Commands {
        CommandGroup(replacing: .newItem) {
            Button("New Project…") {
                env.presentNewProject()
            }
            .keyboardShortcut("n", modifiers: .command)

            Button("Open Project…") {
                env.presentOpenImporter()
            }
            .keyboardShortcut("o", modifiers: .command)

            Divider()

            Button("Close Project") {
                if let pid = env.activeSession?.openProjectResult?.projectID {
                    env.closeProject(projectID: pid)
                }
            }
            .keyboardShortcut("w", modifiers: .command)
            .disabled(env.activeSession == nil)
        }

        CommandMenu("Project") {
            Button("Project Settings…") {
                env.activeSession?.showProjectSettings = true
            }
            .disabled(env.activeSession == nil)

            Divider()

            if let session = env.activeSession {
                Toggle("Show Scene Inspector", isOn: Bindable(session).inspectorVisible)
                    .keyboardShortcut("i", modifiers: [.command, .shift])
                Toggle("Show Timeline", isOn: Bindable(session).timelineVisible)
                    .keyboardShortcut("t", modifiers: [.command, .shift])
                Toggle("Show World Warnings", isOn: Bindable(session).worldWarningVisible)
            } else {
                Toggle("Show Scene Inspector", isOn: .constant(false))
                    .keyboardShortcut("i", modifiers: [.command, .shift])
                    .disabled(true)
                Toggle("Show Timeline", isOn: .constant(false))
                    .keyboardShortcut("t", modifiers: [.command, .shift])
                    .disabled(true)
                Toggle("Show World Warnings", isOn: .constant(false))
                    .disabled(true)
            }
        }

        CommandGroup(replacing: .appInfo) {
            Button("About Scrivi") { showAbout = true }
        }
    }
}

// MARK: — App-level event wiring shared by every scene

// Installs the openWindow bridge, deep-link handlers, and app-resign save on whichever
// window is showing. Applied to both the Welcome window and each project window so a
// deep link works regardless of which window has focus.
private struct AppEventsModifier: ViewModifier {
    let env: AppEnvironment
    @Environment(\.openWindow) private var openWindow

    func body(content: Content) -> some View {
        content
            .onAppear {
                // Bridge SwiftUI's openWindow(id:"welcome") into the environment so
                // orchestration can reopen the Welcome window when the last project closes.
                env.openWelcomeAction = { openWindow(id: "welcome") }
            }
            // Deep link via the scrivi:// URL scheme. On macOS this is handled at the
            // app level by AppDelegate.application(_:open:) (reliable across windows);
            // keep the per-view path for non-macOS platforms only.
            #if !os(macOS)
            .onOpenURL { url in
                Task { await env.handleDeepLink(url) }
            }
            #endif
            // Deep link via a tapped Core Spotlight result. The activity carries the item's
            // uniqueIdentifier ("<kind>:<id>") under CSSearchableItemActivityIdentifier. When
            // available we also recover the full scrivi:// deep link from the donated
            // contentAttributeSet.relatedUniqueIdentifier — it carries the projectID, so even a
            // scene tap can open a *closed* project. (This continuation path is known to be
            // unreliable on SwiftUI macOS; the scrivi:// URL scheme remains the primary route.)
            #if canImport(CoreSpotlight)
            .onContinueUserActivity(CSSearchableItemActionType) { activity in
                guard let uid = activity.userInfo?[CSSearchableItemActivityIdentifier] as? String
                else { return }
                let relatedURL = (activity.contentAttributeSet?.relatedUniqueIdentifier)
                    .flatMap(URL.init(string:))
                Task { await env.handleSpotlightItem(uniqueIdentifier: uid, relatedURL: relatedURL) }
            }
            #endif
            .onReceive(
                NotificationCenter.default.publisher(
                    for: {
                        #if os(macOS)
                        NSApplication.willResignActiveNotification
                        #else
                        UIApplication.willResignActiveNotification
                        #endif
                    }()
                )
            ) { _ in
                Task { await env.onAppResign() }
            }
            #if os(macOS)
            .background(WindowFrameAutosave())
            #endif
    }
}

// MARK: — Welcome window root (macOS)

#if os(macOS)
// Hosts the Landing view. Auto-dismisses itself once any project window is open, and is
// reopened by orchestration when the last project closes ("close on open, reopen when
// none left", EP-018 / T-0194).
private struct WelcomeWindowRoot: View {
    @Environment(AppEnvironment.self) private var env
    @Environment(\.dismissWindow) private var dismissWindow

    var body: some View {
        LandingView()
            // When a project opens, close the Welcome window.
            .onChange(of: env.openProjects.isEmpty) { _, isEmpty in
                if !isEmpty { dismissWindow(id: "welcome") }
            }
            .onAppear {
                // If a project is already open when this appears (e.g. reopened then a
                // project still lingers), close immediately.
                if !env.openProjects.isEmpty { dismissWindow(id: "welcome") }
            }
    }
}
#endif

// MARK: — iOS root (single window)

#if !os(macOS)
// iOS/iPadOS/visionOS single-window root: shows the editor for the active project session,
// or the Landing view when none is open. (The macOS AppKit multi-window model and menu bar
// do not apply here; the full per-platform window UX is deferred — EP-018 non-goal.)
private struct iOSRootView: View {
    @Environment(AppEnvironment.self) private var env

    var body: some View {
        Group {
            if let session = env.activeSession {
                EditorView()
                    .environment(session)
            } else {
                LandingView()
            }
        }
    }
}
#endif

