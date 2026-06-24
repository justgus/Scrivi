#if os(macOS)
import AppKit
import SwiftUI
import os

// ProjectWindowManager — owns the AppKit NSWindow for each open project (EP-018 / T-0194).
//
// Why AppKit instead of SwiftUI WindowGroup(for:): WindowGroup(for:) caches dismissed
// windows and reopening a closed project rebinds to a dead view (no .task/.onAppear), so
// reopen hung on "Loading…". Managing NSWindows directly gives deterministic lifecycle:
// create on open, focus on R3, fully close+release on close. One window per ProjectSession.
//
// Keyed by projectID. The Welcome/Landing surface stays a SwiftUI Window; only project
// editor windows are AppKit-managed.
@MainActor
final class ProjectWindowManager {

    private let log = Logger(subsystem: "com.caposoft.scrivi", category: "Windows")

    // projectID → its window controller.
    private var controllers: [String: ProjectWindowController] = [:]

    var openProjectIDs: [String] { Array(controllers.keys) }
    var isEmpty: Bool { controllers.isEmpty }

    // Opens a new window for `session`, or focuses the existing one for its project (R3).
    func openOrFocus(session: ProjectSession, env: AppEnvironment) {
        guard let projectID = session.openProjectResult?.projectID else {
            log.error("openOrFocus: session has no projectID")
            return
        }
        if let existing = controllers[projectID] {
            log.notice("focus existing window for \(projectID, privacy: .public)")
            existing.showAndFocus()
            return
        }
        log.notice("create window for \(projectID, privacy: .public)")
        let controller = ProjectWindowController(session: session, env: env) { [weak self] in
            // onClose: window closed by the user (red button or ⌘W) — tear down.
            self?.handleWindowClosed(projectID: projectID, env: env)
        }
        controllers[projectID] = controller
        controller.showAndFocus()
    }

    // Closes the window for a project (called by the Close Project menu command). The
    // window's close handler does the session teardown.
    func closeWindow(projectID: String) {
        controllers[projectID]?.close()
    }

    private func handleWindowClosed(projectID: String, env: AppEnvironment) {
        log.notice("window closed for \(projectID, privacy: .public)")
        controllers.removeValue(forKey: projectID)
        env.didCloseProjectWindow(projectID: projectID)
    }
}

// One NSWindow hosting a project's SwiftUI EditorView.
@MainActor
final class ProjectWindowController: NSObject, NSWindowDelegate {

    private let window: NSWindow
    private let projectID: String
    private let onClose: () -> Void

    private let session: ProjectSession
    private weak var env: AppEnvironment?

    init(session: ProjectSession, env: AppEnvironment, onClose: @escaping () -> Void) {
        self.session = session
        self.env = env
        self.projectID = session.openProjectResult?.projectID ?? ""
        self.onClose = onClose

        let root = ProjectWindowContent(session: session)
            .environment(env)
            .environment(session)

        window = NSWindow(
            contentRect: NSRect(x: 0, y: 0, width: 1100, height: 700),
            styleMask: [.titled, .closable, .miniaturizable, .resizable],
            backing: .buffered,
            defer: false
        )
        window.contentView = NSHostingView(rootView: root)
        window.isReleasedWhenClosed = false
        window.center()

        let title = session.projectPreferences?.projectTitle ?? "Scrivi"
        window.title = title.trimmingCharacters(in: .whitespaces).isEmpty ? "Untitled" : title

        super.init()
        window.delegate = self
    }

    func showAndFocus() {
        window.makeKeyAndOrderFront(nil)
        NSApp.activate(ignoringOtherApps: true)
    }

    func close() {
        window.close()
    }

    // NSWindowDelegate: fires for every close path (red button, ⌘W, window.close()).
    func windowWillClose(_ notification: Notification) {
        onClose()
    }

    // Frontmost tracking — drives the menu bar's enabled state and per-window toggles
    // (AppKit windows don't feed SwiftUI's @FocusedValue, so we track focus explicitly).
    func windowDidBecomeKey(_ notification: Notification) {
        env?.frontmostSession = session
    }
}

// SwiftUI content hosted in the AppKit project window. Mirrors what ContentRoot did:
// the editor plus the Project Settings sheet, with the session as a FocusedValue so the
// menu bar acts on the frontmost project window.
private struct ProjectWindowContent: View {
    let session: ProjectSession

    var body: some View {
        EditorView()
            .sheet(isPresented: Bindable(session).showProjectSettings) {
                if let prefs = session.projectPreferences {
                    ProjectSettingsSheet(prefs: prefs)
                }
            }
            .frame(minWidth: 700, minHeight: 400)
    }
}
#endif
