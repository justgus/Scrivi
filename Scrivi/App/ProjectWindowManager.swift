#if os(macOS)
@preconcurrency import AppKit
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

// Holds a NotificationCenter observer token so a one-shot @Sendable observer closure can
// remove itself. @unchecked Sendable is sound: the token is only written after addObserver
// returns and read back on the main queue where the notification is delivered.
private final class ObserverTokenBox: @unchecked Sendable {
    var token: NSObjectProtocol?
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

        // Restore this project's last size/position instead of defaulting (I-0051). A saved
        // frame is clamped back on-screen in case its display is gone. With no saved frame
        // (first-ever open), center then cascade so concurrently-opened windows don't stack
        // exactly on top of one another.
        if let saved = ProjectWindowFrameStore.savedFrame(projectID: projectID) {
            window.setFrame(ProjectWindowFrameStore.clampedOnscreen(saved), display: false)
        } else {
            window.center()
            // cascadeTopLeft(from:) places the window and returns the next anchor point.
            ProjectWindowController.lastCascadePoint =
                window.cascadeTopLeft(from: ProjectWindowController.lastCascadePoint)
        }

        let title = session.projectPreferences?.projectTitle ?? "Scrivi"
        window.title = title.trimmingCharacters(in: .whitespaces).isEmpty ? "Untitled" : title

        super.init()
        window.delegate = self

        // The saved windowed frame is applied above (setFrame) and is the size the window returns to
        // when it leaves full screen. If the project was last quit in macOS Full Screen, restore
        // that state after the window is shown (showAndFocus) via the proper toggleFullScreen API —
        // NOT a geometry hack (a manually screen-sized window is NOT full screen) and NOT
        // window.zoom(nil) (unreliable). I-0055.
        restoreFullScreenOnShow = ProjectWindowFrameStore.savedFullScreen(projectID: projectID)
    }

    // Set in init from the persisted per-project full-screen flag; consumed once in showAndFocus().
    private var restoreFullScreenOnShow = false

    // Shared cascade anchor so successive first-open windows step down/right instead of
    // landing on the same centered point. AppKit advances and returns the next point.
    private static var lastCascadePoint = NSPoint.zero

    func showAndFocus() {
        window.makeKeyAndOrderFront(nil)
        NSApp.activate(ignoringOtherApps: true)

        // Restore macOS Full Screen. toggleFullScreen drives the real full-screen transition
        // (menu-bar hide, own Space); the windowDidEnterFullScreen delegate persists the flag and
        // the windowed frame (applied in init) is never overwritten while full screen (I-0055).
        if restoreFullScreenOnShow {
            restoreFullScreenOnShow = false
            // A programmatic toggleFullScreen fired DURING app launch stalls mid-flight —
            // windowWillEnterFullScreen arrives but windowDidEnterFullScreen never does, so the
            // window is left windowed (I-0097). Manual (green-button) full screen works because
            // the app is already fully active. So defer the restore until the app is active AND
            // the launch storm has settled: wait for NSApplication.didBecomeActive (or run now if
            // already active), then a short delay before toggling.
            if NSApp.isActive {
                scheduleFullScreenAfterSettle()
            } else {
                // The block-observer closure is @Sendable, so it can capture only Sendable state:
                // self (weak, hopped back onto the main actor) and a boxed token so the one-shot
                // observer can remove itself. Delivery is already on .main, so assumeIsolated is
                // sound.
                let box = ObserverTokenBox()
                box.token = NotificationCenter.default.addObserver(
                    forName: NSApplication.didBecomeActiveNotification,
                    object: nil, queue: .main
                ) { [weak self] _ in
                    MainActor.assumeIsolated {
                        if let token = box.token { NotificationCenter.default.removeObserver(token) }
                        self?.scheduleFullScreenAfterSettle()
                    }
                }
            }
        }
    }

    // Wait out the launch storm, then toggle into full screen if not already there (I-0097).
    private func scheduleFullScreenAfterSettle() {
        DispatchQueue.main.asyncAfter(deadline: .now() + 0.35) { [weak self] in
            guard let self, !self.window.styleMask.contains(.fullScreen) else { return }
            self.window.toggleFullScreen(nil)
        }
    }

    func close() {
        window.close()
    }

    // NSWindowDelegate: fires for every close path (red button, ⌘W, window.close()).
    func windowWillClose(_ notification: Notification) {
        // Persist final size/position before teardown so reopening this project restores it
        // (I-0051). Quit-time also routes through here as each window closes.
        ProjectWindowFrameStore.save(window: window, projectID: projectID)
        onClose()
    }

    // Persist frame on user resize/move so a crash or force-quit still restores recent layout.
    func windowDidEndLiveResize(_ notification: Notification) {
        // A full-screen transition emits live-resize end callbacks with the styleMask flipping
        // mid-flight; saving here would persist the transient state (fs=1 then fs=0) and could
        // clobber the real value. Suppress during the transition, exactly like windowDidResize
        // (the ent/ exit did-callbacks persist the settled state). I-0097.
        guard !isTransitioningFullScreen else { return }
        ProjectWindowFrameStore.save(window: window, projectID: projectID)
    }

    func windowDidMove(_ notification: Notification) {
        ProjectWindowFrameStore.save(window: window, projectID: projectID)
    }

    // Live drag-resize and tiling (quadrant/half/third) end here; persist the new windowed frame.
    // Suppressed during a full-screen transition so the transient full-screen resize does not
    // overwrite the saved windowed frame (save() also guards via styleMask, this avoids the churn).
    func windowDidResize(_ notification: Notification) {
        guard !isTransitioningFullScreen else { return }
        ProjectWindowFrameStore.save(window: window, projectID: projectID)
    }

    // MARK: Full-screen transitions (the green-button "maximize" path)

    // True from willEnter/willExit until the matching did-callback, so windowDidResize does not
    // persist the transient full-screen geometry as the windowed frame.
    private var isTransitioningFullScreen = false

    func windowWillEnterFullScreen(_ notification: Notification) {
        isTransitioningFullScreen = true
    }

    func windowDidEnterFullScreen(_ notification: Notification) {
        isTransitioningFullScreen = false
        // Persist the full-screen flag; save() leaves the windowed frame untouched while full screen.
        ProjectWindowFrameStore.save(window: window, projectID: projectID)
    }

    func windowWillExitFullScreen(_ notification: Notification) {
        isTransitioningFullScreen = true
    }

    func windowDidExitFullScreen(_ notification: Notification) {
        isTransitioningFullScreen = false
        // Back to windowed: record fullScreen=false and the restored windowed frame.
        ProjectWindowFrameStore.save(window: window, projectID: projectID)
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
