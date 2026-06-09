import SwiftUI
import AppKit

private let frameKey  = "scrivi.mainWindow.frame"
private let zoomedKey = "scrivi.mainWindow.zoomed"

// Installs an NSWindowDelegate on the host window to persist frame and
// zoomed state across launches.
//
// Frame restore happens in the first deferred pass (after SwiftUI gets the window).
// Zoom restore is deferred until NSApplication.didFinishLaunching fires, which is
// the earliest point where SwiftUI has fully completed all initial layout passes
// and a zoom call will not be overridden.
struct WindowFrameAutosave: NSViewRepresentable {

    func makeNSView(context: Context) -> NSView {
        let view = NSView()
        DispatchQueue.main.async {
            guard let window = view.window else { return }
            window.delegate = context.coordinator
            context.coordinator.window = window

            // Restore the saved frame immediately.
            if let frameString = UserDefaults.standard.string(forKey: frameKey) {
                let frame = NSRectFromString(frameString)
                if frame != .zero {
                    window.setFrame(frame, display: false)
                }
            }

            // Zoom restore is registered as a one-shot observer for didFinishLaunching.
            // If the app has already finished launching (e.g., view re-appears), apply
            // the zoom directly on the next run loop turn instead.
            let shouldZoom = UserDefaults.standard.bool(forKey: zoomedKey)
            guard shouldZoom else { return }

            if NSApp.isRunning {
                DispatchQueue.main.async { window.zoom(nil) }
            } else {
                NotificationCenter.default.addObserver(
                    forName: NSApplication.didFinishLaunchingNotification,
                    object: nil,
                    queue: .main
                ) { [weak window] _ in
                    window?.zoom(nil)
                }
            }
        }
        return view
    }

    func updateNSView(_ nsView: NSView, context: Context) {}

    func makeCoordinator() -> Coordinator { Coordinator() }

    final class Coordinator: NSObject, NSWindowDelegate {
        weak var window: NSWindow?

        @MainActor func windowDidEndLiveResize(_ notification: Notification) { saveState(notification) }
        @MainActor func windowDidMove(_ notification: Notification)          { saveState(notification) }
        @MainActor func windowWillClose(_ notification: Notification)        { saveState(notification) }

        @MainActor
        func windowDidChangeOcclusionState(_ notification: Notification) {
            guard let window = notification.object as? NSWindow else { return }
            UserDefaults.standard.set(window.isZoomed, forKey: zoomedKey)
        }

        @MainActor
        private func saveState(_ notification: Notification) {
            guard let window = notification.object as? NSWindow else { return }
            UserDefaults.standard.set(window.isZoomed, forKey: zoomedKey)
            // Only update the saved frame when un-zoomed, preserving the last
            // known un-zoomed size across zoomed sessions.
            if !window.isZoomed {
                UserDefaults.standard.set(
                    NSStringFromRect(window.frame),
                    forKey: frameKey
                )
            }
        }
    }
}
