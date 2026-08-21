import Foundation

/// Explicit back/forward navigation history for the Detail Sheet
/// (EP-034 SP-117, T-0435; design trade **D2-B**).
///
/// ## Why explicit rather than `NavigationStack`
///
/// D2-A (`NavigationStack` push/pop) gives **back** natively but **not forward**,
/// and the user asked for *"standard NavigatorView buttons"* — back *and*
/// forward. Forward needs exactly this machinery regardless, so it is built
/// openly instead of bolted onto a stack that half-does it.
///
/// ## ⚠️ Deliberately host-independent (S8)
///
/// This type knows nothing about panes, windows, SwiftUI environments or the
/// editor. That is what keeps **D1-B (a real window)** available later without a
/// rewrite — the D1-E ruling requires the sheet be *"a self-contained, navigable
/// component that does not depend on its host."* The same history object serves a
/// pane today and a window tomorrow.
///
/// ## The model
///
/// A cursor into an array, exactly like a browser:
///
/// ```
///   entries: [Mara, Eskandar, Vance]
///   index:              ^ 1              back → Mara, forward → Vance
/// ```
///
/// ⚠️ **Visiting a NEW object truncates everything ahead of the cursor** — the
/// browser rule. Keeping the old forward entries would offer a "forward" that
/// leads somewhere the writer never went from here.
@Observable
final class ObjectDetailHistory {

    /// One visited object. Identity only — the sheet re-reads from disk on
    /// arrival, so history never serves stale field values.
    struct Entry: Equatable, Sendable {
        let objectID: String
        let kind: String
        let worldID: String
        /// For the back/forward tooltips, so a writer can see where she is going.
        let displayName: String
    }

    private(set) var entries: [Entry] = []
    private(set) var index: Int = -1

    init() {}

    /// The object currently on screen, or nil before the first visit.
    var current: Entry? {
        guard index >= 0, index < entries.count else { return nil }
        return entries[index]
    }

    var canGoBack: Bool { index > 0 }
    var canGoForward: Bool { index >= 0 && index + 1 < entries.count }

    /// Where "Back" would land — used for the control's tooltip.
    var backTarget: Entry? { canGoBack ? entries[index - 1] : nil }
    var forwardTarget: Entry? { canGoForward ? entries[index + 1] : nil }

    /// Navigates to an object, truncating any forward history.
    ///
    /// ⚠️ Re-visiting the object already on screen is a NO-OP rather than a new
    /// entry. Without this, opening the same sheet twice would stack duplicates
    /// and "back" would appear to do nothing — the shape of I-0132, where a
    /// re-selection wrote an unchanged value and the update was coalesced away.
    func visit(_ entry: Entry) {
        if let current, current.objectID == entry.objectID { return }

        if index < entries.count - 1 {
            entries.removeSubrange((index + 1)...)
        }
        entries.append(entry)
        index = entries.count - 1
    }

    @discardableResult
    func goBack() -> Entry? {
        guard canGoBack else { return nil }
        index -= 1
        return entries[index]
    }

    @discardableResult
    func goForward() -> Entry? {
        guard canGoForward else { return nil }
        index += 1
        return entries[index]
    }

    /// Clears history — used when the sheet closes, so reopening starts fresh
    /// rather than resuming a trail the writer has left behind.
    func reset() {
        entries.removeAll()
        index = -1
    }
}
