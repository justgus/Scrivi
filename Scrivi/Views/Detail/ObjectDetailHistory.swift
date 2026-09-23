import Foundation

/// The object the host hands to the Detail Sheet when it opens it.
///
/// ## ⚠️ THIS IS NOT A NAVIGATION MODEL. It used to be.
///
/// ⛔ Until [T-0547] (2026-09-23) this was a browser-style CURSOR — `entries` plus an
/// `index`, with `canGoBack`/`canGoForward`, `backTarget`/`forwardTarget` and
/// `goBack()`/`goForward()`. The Detail Sheet drove it from hand-built chrome.
///
/// ✅ **`NavigationStack` OWNS NAVIGATION NOW.** It draws the bar, the title and the
/// back chevron, holds the real stack in a `NavigationPath`, and manages the
/// transitions. ⛔ Nothing here participates in that.
///
/// ## ⚠️ Two requirements retired, in order — ⛔ do not reinstate either casually
///
/// ⛔ **FORWARD (user ruling, 2026-09-23):** *"Forward buttons are not required. We have
/// navigate forward by double clicking the object reference. That will also be much
/// easier to implement on iOS and visionOS as it is a standard UI metaphor."*
/// ⚠️ Forward was the ONLY reason this was ever a cursor rather than a stack:
/// `NavigationPath`'s entire API is `append` · `removeLast` · `count` · `isEmpty`, and
/// no SwiftUI, AppKit or UIKit component offers a forward affordance. ✅ That is why
/// browsers hand-roll one — and why, with forward gone, push/pop is the whole job.
///
/// ⛔ **ARRESTING A POP (user ruling, 2026-09-23):** an earlier design tried to INTERCEPT
/// the back chevron so the unsaved-changes prompt could offer Cancel. ✅ No such hook
/// exists — verified against the macOS 27 SDK's own `SwiftUI.swiftinterface`: no
/// navigation or dismissal function takes a closure, and `navigationTransition` takes a
/// STYLE, not an action. ✅ The user's ruling dissolved the need: *"This confirmation
/// dialog shouldn't attempt to 'Cancel' the navigation … rather it should require either
/// a save or a revert."* ⚠️ A settled pop needs no interception — only a decision about
/// the drafts left behind, and those live on the SHEET, above the stack.
///
/// ## ✅ What survives, and why
///
/// ⚠️ The sheet is presented by `EditorView`, so the first object has to be recorded
/// somewhere BEFORE the sheet exists to hold it. ✅ That is this type's whole remaining
/// job: the host calls `visit(_:)` before presenting, the sheet seeds its `trail` from
/// `current` on appear, and the host calls `reset()` on close so the next open starts
/// clean ([I-0168]'s guarded re-entry depends on that).
///
/// ⚠️ `Entry` is also the shared vocabulary for "which object" across
/// `ObjectRelationsSection`, `ObjectSourcesSection` and the inspector cards — ⛔ which is
/// the main reason this file is not simply deleted.
@Observable
final class ObjectDetailHistory {

    /// One object, by identity.
    ///
    /// ⚠️ `Hashable` since [T-0547]: `NavigationPath.append` requires it, and the sheet's
    /// `navigationDestination(for:)` keys on this type.
    struct Entry: Equatable, Hashable, Sendable {
        let objectID: String
        let kind: String
        let worldID: String
        /// ⚠️ Carried so a writer is never shown an ID — the Back tooltip, the sheet's
        /// title and the unsaved-changes prompt all name the object (the AC-A7 rule).
        let displayName: String
    }

    /// ⚠️ The handoff. ⛔ NOT a trail: the sheet keeps its own, in step with the
    /// `NavigationPath`, because a type-erased path cannot be read back.
    private(set) var current: Entry?

    init() {}

    /// Records the object the sheet should open on.
    ///
    /// ⚠️ Called by the host BEFORE presenting. ⛔ Re-visiting the same object is a
    /// no-op, which is what stops a double-click stacking duplicates (the shape of
    /// [I-0132], where a re-selection wrote an unchanged value and the update was
    /// coalesced away).
    func visit(_ entry: Entry) {
        guard current?.objectID != entry.objectID else { return }
        current = entry
    }

    /// Clears the handoff when the sheet closes, so reopening starts fresh rather than
    /// resuming a trail the writer has left behind.
    func reset() {
        current = nil
    }
}
