import SwiftUI
import AppKit

private let frameKey  = "scrivi.mainWindow.frame"
private let zoomedKey = "scrivi.mainWindow.zoomed"

// Installs an NSWindowDelegate on the host window to persist frame and
// zoomed state. Restoration is deferred until after SwiftUI finishes its
// own layout pass so the saved frame is not overwritten.
struct WindowFrameAutosave: NSViewRepresentable {

    func makeNSView(context: Context) -> NSView {
        let view = NSView()
        // Use two deferred passes: first to get the window reference,
        // second (after SwiftUI layout) to apply the saved state.
        DispatchQueue.main.async {
            guard let window = view.window else { return }
            window.delegate = context.coordinator
            context.coordinator.window = window

            // Restore frame first.
            if let frameString = UserDefaults.standard.string(forKey: frameKey) {
                let frame = NSRectFromString(frameString)
                if frame != .zero {
                    window.setFrame(frame, display: false)
                }
            }

            // Defer zoom restoration one more run loop turn so SwiftUI
            // does not override it with its own initial sizing.
            let shouldZoom = UserDefaults.standard.bool(forKey: zoomedKey)
            if shouldZoom {
                DispatchQueue.main.async {
                    window.zoom(nil)
                }
            }
        }
        return view
    }

    func updateNSView(_ nsView: NSView, context: Context) {}

    func makeCoordinator() -> Coordinator { Coordinator() }

    final class Coordinator: NSObject, NSWindowDelegate {
        weak var window: NSWindow?

        func windowDidEndLiveResize(_ notification: Notification) { save(notification) }
        func windowDidMove(_ notification: Notification)          { save(notification) }

        func windowWillClose(_ notification: Notification) {
            guard let window = notification.object as? NSWindow else { return }
            saveState(window)
        }

        func windowDidChangeOcclusionState(_ notification: Notification) {
            // Catch zoom/unzoom which doesn't always fire a resize notification.
            guard let window = notification.object as? NSWindow else { return }
            UserDefaults.standard.set(window.isZoomed, forKey: zoomedKey)
        }

        private func save(_ notification: Notification) {
            guard let window = notification.object as? NSWindow else { return }
            saveState(window)
        }

        private func saveState(_ window: NSWindow) {
            UserDefaults.standard.set(window.isZoomed, forKey: zoomedKey)
            // Save the un-zoomed frame so restore gives a sensible size
            // when the user un-maximizes on the next launch.
            if !window.isZoomed {
                UserDefaults.standard.set(
                    NSStringFromRect(window.frame),
                    forKey: frameKey
                )
            }
        }
    }
}
