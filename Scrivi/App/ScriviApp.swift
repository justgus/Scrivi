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

    // The session of the frontmost project window — drives the project/view menus.
    // Tracked in AppEnvironment (set by each AppKit window when it becomes key), because
    // AppKit NSWindows don't feed SwiftUI's @FocusedValue.
    private var focusedSession: ProjectSession? { env.frontmostSession }

    var body: some Scene {
        // The only SwiftUI scene is the Welcome/Landing window. Project editor windows
        // are AppKit NSWindows owned by ProjectWindowManager (deterministic lifecycle;
        // WindowGroup(for:) was abandoned because it cached dead windows — T-0194).
        Window("Welcome to Scrivi", id: "welcome") {
            WelcomeWindowRoot()
                .environment(env)
                .task {
                    // Bootstrap + restore run once per launch. The Welcome window's task
                    // also fires when it reopens after the last project closes, so guard
                    // restore behind a one-time flag (otherwise it would re-open projects
                    // the user just closed).
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
                    // Restore all project windows open at last quit (R4 / T-0195).
                    await env.restoreOpenProjects()
                }
                .modifier(AppEventsModifier(env: env))
                .sheet(isPresented: $showAbout) { AboutView() }
        }
        .defaultSize(width: 720, height: 480)
        .commands { appCommands }
    }

    // MARK: — Menu bar commands

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

        // View menu — toggles act on the focused project window.
        CommandMenu("View") {
            if let session = focusedSession {
                Toggle("Show Scene Inspector", isOn: Bindable(session).inspectorVisible)
                    .keyboardShortcut("i", modifiers: [.command, .option])
                Toggle("Show Timeline", isOn: Bindable(session).timelineVisible)
                    .keyboardShortcut("t", modifiers: [.command, .option])
            } else {
                Toggle("Show Scene Inspector", isOn: .constant(false))
                    .keyboardShortcut("i", modifiers: [.command, .option])
                    .disabled(true)
                Toggle("Show Timeline", isOn: .constant(false))
                    .keyboardShortcut("t", modifiers: [.command, .option])
                    .disabled(true)
            }
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

// MARK: — Welcome window root

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

