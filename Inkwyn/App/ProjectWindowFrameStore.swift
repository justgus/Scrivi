#if os(macOS)
import AppKit

// ProjectWindowFrameStore — persists each project window's frame + full-screen state, keyed by
// projectID, so a restored or reopened project window returns to the size/position (and
// full-screen state) it had when last quit or closed (I-0051 / I-0055).
//
// The app's pre-EP-018 single-window autosave (WindowFrameAutosave) only ever stored ONE
// frame and was attached only to the SwiftUI Welcome window; the AppKit project windows
// (ProjectWindowController) used a hardcoded rect + center(), so multiple restored windows
// all stacked at the default. This store fixes that for the multi-window model.
//
// State lives in UserDefaults under per-project keys. The windowed frame is only saved while
// NOT full screen, preserving the pre-full-screen size/position across full-screen sessions.
//
// @MainActor because the save/clamp paths touch AppKit (`NSWindow`, `NSScreen`), which are
// main-actor isolated under Swift 6; all callers (ProjectWindowController) are already on the
// main actor.
@MainActor
enum ProjectWindowFrameStore {

    private static func frameKey(_ projectID: String) -> String {
        "scrivi.projectWindow.\(projectID).frame"
    }
    private static func fullScreenKey(_ projectID: String) -> String {
        "scrivi.projectWindow.\(projectID).fullScreen"
    }

    // The saved un-zoomed frame for a project, or nil if none recorded yet.
    static func savedFrame(projectID: String) -> NSRect? {
        guard !projectID.isEmpty,
              let s = UserDefaults.standard.string(forKey: frameKey(projectID)) else { return nil }
        let rect = NSRectFromString(s)
        return rect == .zero ? nil : rect
    }

    // Whether the project was last quit/closed in macOS Full Screen.
    static func savedFullScreen(projectID: String) -> Bool {
        guard !projectID.isEmpty else { return false }
        return UserDefaults.standard.bool(forKey: fullScreenKey(projectID))
    }

    // Persist the window's current state for a project.
    //
    // The green button puts the window into macOS **Full Screen** (menu bar hides, own Space) — a
    // distinct state from a window the user merely resized to fill the screen. We persist Full
    // Screen as a dimensionless binary via NSWindow.styleMask.contains(.fullScreen) — a
    // deterministic flag, unlike NSWindow.isZoomed (which proved unreliable) or a geometry check
    // (which would wrongly flag a manually screen-sized window — I-0055).
    //
    // While full screen, the *windowed* frame is NOT overwritten, so the stored frame stays at the
    // size/position the window had before going full screen. On restore that frame is applied and
    // the window is then toggled back into full screen (I-0051 / I-0055).
    static func save(window: NSWindow, projectID: String) {
        guard !projectID.isEmpty else { return }
        let fullScreen = window.styleMask.contains(.fullScreen)
        UserDefaults.standard.set(fullScreen, forKey: fullScreenKey(projectID))
        if !fullScreen {
            UserDefaults.standard.set(NSStringFromRect(window.frame), forKey: frameKey(projectID))
        }
    }

    // Clamp a saved frame back onto a visible screen. Guards against a frame saved on a
    // display that is no longer attached (or a since-resized arrangement) leaving the window
    // off every screen and thus unreachable.
    static func clampedOnscreen(_ frame: NSRect) -> NSRect {
        // If the frame already intersects any screen meaningfully, keep it.
        let visible = NSScreen.screens.map(\.visibleFrame)
        let intersectsEnough = visible.contains { screen in
            let i = screen.intersection(frame)
            return i.width >= 80 && i.height >= 80
        }
        if intersectsEnough { return frame }

        // Otherwise re-anchor onto the main screen, preserving size where it fits.
        guard let main = NSScreen.main?.visibleFrame else { return frame }
        var f = frame
        f.size.width  = min(f.size.width,  main.size.width)
        f.size.height = min(f.size.height, main.size.height)
        f.origin.x = main.midX - f.size.width / 2
        f.origin.y = main.midY - f.size.height / 2
        return f
    }
}
#endif
