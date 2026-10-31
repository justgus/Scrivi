#if os(iOS)
import SwiftUI
import UIKit

// EP-055 S2 (SP-199 T-0637) — the iOS panel swipes and the Timeline grabber. Design: iOS Manuscript Design §8.
// ⚠️ Never the only route (G4): the ⋯ menu, the Project menu's ⇧⌘I / ⇧⌘T and VoiceOver actions reach the same panels.

/// What a finished horizontal swipe does.
enum PanelSwipe: Equatable {
    case showNavigator, hideNavigator, showInspector, hideInspector, nextPage

    /// A swipe STARTS at an edge within this many points of it. ⚠️ 24 pt (the system's) missed often in the live pass.
    static let edgeZone: CGFloat = 44
    /// A swipe ENDS at an edge when it finishes in this fraction of the width nearest it.
    static let endZone: CGFloat = 0.15

    /// Width regular: from the left edge shows the Navigator, to the right edge hides the Inspector, and the mirror image
    /// for a leftward swipe. A full-width swipe does both (G1). Width compact: a leftward swipe from the right edge goes to
    /// the next page. A rightward swipe there is the system's own back swipe, so it does nothing here.
    static func actions(startX: CGFloat, endX: CGFloat, width: CGFloat, compact: Bool) -> [PanelSwipe] {
        let fromLeft = startX <= edgeZone, fromRight = startX >= width - edgeZone
        let toLeft = endX <= width * endZone, toRight = endX >= width * (1 - endZone)
        if endX < startX {
            if compact { return fromRight ? [.nextPage] : [] }
            return (fromRight ? [.showInspector] : []) + (toLeft ? [.hideNavigator] : [])
        }
        if compact { return [] }
        return (fromLeft ? [.showNavigator] : []) + (toRight ? [.hideInspector] : [])
    }
}

/// Puts one pan recognizer on the WINDOW, and lets it see only touches that begin on the MANUSCRIPT (user, SP-199 live
/// pass: the Timeline's own drag-to-scroll was being taken). It recognises alongside the text view's scrolling and
/// selection and cancels no touches: it only reads a swipe after it has finished.
struct PanelSwipeInstaller: UIViewRepresentable {
    var onSwipe: ([PanelSwipe]) -> Void

    func makeUIView(context: Context) -> InstallerView {
        let v = InstallerView()
        v.isUserInteractionEnabled = false
        v.handler = context.coordinator
        return v
    }
    func updateUIView(_ v: InstallerView, context: Context) { context.coordinator.parent = self }
    func makeCoordinator() -> Coordinator { Coordinator(self) }

    final class InstallerView: UIView {
        weak var handler: Coordinator?
        override func didMoveToWindow() {
            super.didMoveToWindow()
            handler?.install(on: window)
        }
    }

    final class Coordinator: NSObject, UIGestureRecognizerDelegate {
        var parent: PanelSwipeInstaller
        private let pan = UIPanGestureRecognizer()
        init(_ parent: PanelSwipeInstaller) {
            self.parent = parent
            super.init()
            pan.addTarget(self, action: #selector(panned(_:)))
            pan.cancelsTouchesInView = false
            pan.delegate = self
        }
        func install(on window: UIWindow?) {
            pan.view?.removeGestureRecognizer(pan)
            window?.addGestureRecognizer(pan)
        }
        func gestureRecognizer(_ g: UIGestureRecognizer, shouldRecognizeSimultaneouslyWith other: UIGestureRecognizer) -> Bool { true }
        func gestureRecognizer(_ g: UIGestureRecognizer, shouldReceive touch: UITouch) -> Bool {
            var view = touch.view
            while let v = view, !(v is ManuscriptUITextView) { view = v.superview }
            return view != nil
        }
        @objc private func panned(_ g: UIPanGestureRecognizer) {
            guard g.state == .ended, let window = g.view else { return }
            let t = g.translation(in: window), v = g.velocity(in: window)
            let end = g.location(in: window).x
            // [SCRIVI-SWIPE] temporary: SP-199 live pass, swipes still miss. Remove once the misses are explained.
            NSLog("[SCRIVI-SWIPE] start=%.0f end=%.0f tx=%.0f ty=%.0f vx=%.0f width=%.0f", end - t.x, end, t.x, t.y, v.x,
                  window.bounds.width)
            // A swipe, not a selection drag or a scroll: mostly sideways, and quick or long.
            guard abs(t.x) > 1.5 * abs(t.y), abs(t.x) > 60, abs(v.x) > 200 || abs(t.x) > 160 else { return }
            let actions = PanelSwipe.actions(startX: end - t.x, endX: end, width: window.bounds.width,
                                             compact: window.traitCollection.horizontalSizeClass == .compact)
            NSLog("[SCRIVI-SWIPE] actions=%@", "\(actions)")
            if !actions.isEmpty { parent.onSwipe(actions) }
        }
    }
}

/// The strip above the bottom edge whose TAP shows and hides the Timeline (G2, revised by the user after the live pass: a
/// swipe up also raised the Dock and the app switcher).
struct TimelineGrabber: View {
    @Binding var isShown: Bool

    var body: some View {
        Capsule()
            .fill(.secondary)
            .frame(width: 36, height: 5)
            .frame(maxWidth: .infinity, minHeight: 20)  // layout-ok: the grabber's touch strip inside the bottom bar, not a column
            .contentShape(Rectangle())
            .onTapGesture { isShown.toggle() }
            .accessibilityElement()
            .accessibilityLabel(isShown ? "Hide Timeline" : "Show Timeline")
            .accessibilityAddTraits(.isButton)
            .accessibilityAction { isShown.toggle() }
    }
}
#endif
