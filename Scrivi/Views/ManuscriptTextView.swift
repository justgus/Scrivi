import SwiftUI
#if os(macOS)
import AppKit

// Custom attribute key used to mark chapter title heading ranges as non-editable.
extension NSAttributedString.Key {
    static let scriviHeading = NSAttributedString.Key("scrivi.heading")
    /// ✅ EP-045 AC1 — THE ONE TEST FOR "is this a scene divider?". Its value is a
    /// `DividerRenderState`. ⛔ Never test `.attachment` for that question: any attachment a
    /// future feature inserts (EP-032 references, EP-046 rendering) would then split scenes,
    /// and `sceneBoundaries` is what the save path slices scene bytes with.
    static let scriviDivider = NSAttributedString.Key("scrivi.divider")
}

/// EP-045 AC1 — the rendering state of a `DividerTextAttachment`, carried as the value of
/// `.scriviDivider` on the divider's character (user ruling 2026-10-03: *"We can make the class
/// manage different rendering states without changing the type. The attribute key can be
/// managed via an enum and can represent the rendering state of the class."*).
/// ⚠️ View-only: ScriviCore has no dividers, so this never crosses the ABI or reaches disk.
enum DividerRenderState: Equatable {
    /// Between two scenes of the same chapter.
    case sceneBreak
    /// Closes a chapter — drawn tinted (T-0575).
    case chapterEnd
}

// ManuscriptTextView presents all loaded SceneSegments as a single continuous NSTextView.
//
// Scene boundaries are tracked as character ranges in `sceneBoundaries`.
// A thin 1pt horizontal rule NSTextAttachment is inserted between segments.
// Auto-save: 1-second debounce after each keystroke for the current segment;
//            immediate save on scene-exit (cursor crosses a boundary) and on demand.
// Key bindings:
//   ⌘↩  — createScene in current chapter after current scene
//   ⌘⇧↩ — createChapter (new chapter with auto first scene)

struct ManuscriptTextView: NSViewRepresentable {

    var loader: ViewportSceneLoader
    var env: AppEnvironment
    var session: ProjectSession
    @Binding var navigateToSceneID: String?
    var showChapterTitles: Bool
    /// ✅ EP-047 S2: the project's type (face + size). Applied to the presenter's ONE stored value; a change re-presents.
    var typography: ManuscriptTypography
    /// ✅ EP-047 S4: Markup Hints on/off — applied WITHOUT a rebuild (the presenter re-presents what changes).
    var markupHints: Bool = true
    func makeCoordinator() -> Coordinator { Coordinator(self) }

    func makeNSView(context: Context) -> NSScrollView {
        let textView = ManuscriptNSTextView()
        textView.isEditable = true
        textView.isRichText = false
        // EP-019: disable AppKit's native undo entirely. Undo/redo are driven by
        // our custom HistoryService via the undo(_:)/redo(_:) action methods on
        // ManuscriptNSTextView (T-0199 spike: a real allowsUndo=false + first-
        // responder action methods, NOT an UndoManager proxy).
        textView.allowsUndo = false
        // ✅ EP-047 S2: the project's type, from the ONE source (the presenter's `typography`).
        context.coordinator.presenter.typography = typography
        if let ts = textView.textStorage {
            context.coordinator.presenter.setHints(markupHints, selection: textView.selectedRange(), in: ts)
        }
        textView.font = typography.bodyFont
        textView.typingAttributes = typography.bodyAttributes
        // I-0112: an NSTextView whose textColor is nil renders runs that carry no
        // .foregroundColor as literal NSColor.black — NOT the adaptive default — so in
        // Dark Mode the manuscript was black text on a dark gray background. Body runs
        // now set .foregroundColor explicitly (see rebuildStorage / the history-apply
        // path); this assignment is the backstop so any run written without one still
        // inherits an appearance-adaptive color instead of black.
        textView.textColor = NSColor.textColor
        textView.autoresizingMask = [.width]
        textView.isVerticallyResizable = true
        textView.textContainer?.widthTracksTextView = true
        textView.textContainerInset = NSSize(width: 60, height: 40)
        textView.delegate = context.coordinator
        context.coordinator.textView = textView
        // ✅ EP-046 E2-S1 — the PRESENTER (route (a′)): TextKit 2 asks it for each paragraph and gets
        // the same characters with styled attributes; storage stays plain. It is also the storage
        // delegate, to re-present a whole block after a character edit. (Weak delegates — the
        // coordinator owns it.) It replaces EP-045's `EscapeHidingStyler`.
        textView.textContentStorage?.delegate = context.coordinator.presenter
        textView.textStorage?.delegate = context.coordinator.presenter
        if let storage = textView.textStorage { context.coordinator.boundaryTable.observe(storage) }

        // T-0531 DIAGNOSTIC — report which layout engine this view ACTUALLY uses.
        // ⚠️ Reading `.layoutManager` to check would itself cause the downgrade, so the
        // test is `textLayoutManager != nil` — present ⇒ TextKit 2, nil ⇒ TextKit 1.
        NSLog("[SCRIVI-TK] engine at construction: %@",
              textView.textLayoutManager != nil ? "TextKit 2" : "TextKit 1 (DOWNGRADED)")
        // Register takeFocus with both the coordinator and the loader so any
        // caller (Navigator, delete handler) can transfer first-responder directly.
        //
        // ⚠️ **Deferred one pass, and that is all it needs (I-0132).** Called from a
        // SwiftUI `onChange` on the selection, so it runs during a view update; hopping to
        // the next runloop pass lets that update — and any responder change AppKit makes
        // while completing the click — finish first.
        //
        // Earlier attempts fought a race here (synchronous call, then a claim/re-claim
        // retry) because navigation itself was unreliable: the *real* defect was the
        // one-shot `navigateToSceneID` trigger, not first-responder arbitration. With
        // selection as the source of truth the call site is deterministic, so this does
        // not need to defend itself.
        let takeFocus: () -> Void = { [weak textView] in
            DispatchQueue.main.async {
                guard let textView, let window = textView.window else { return }
                if window.firstResponder === textView { return }
                window.makeFirstResponder(textView)
            }
        }
        context.coordinator.onTakeFocus = takeFocus
        loader.takeFocusHandler = takeFocus

        // Bridge the buffers palette / Edit menu to the coordinator's text mutations
        // (EP-019 SP-056, T-0214). The palette calls these so paste/load run on this
        // text view through the history + auto-save path. Loading from the palette
        // takes focus first so the current selection is this view's selection.
        let coordinator = context.coordinator
        session.bufferService?.pasteFromBufferHandler = { [weak coordinator] bufferID in
            coordinator?.pasteFromBuffer(bufferID)
        }
        session.bufferService?.loadSelectionHandler = { [weak coordinator] bufferID in
            coordinator?.copyIntoBuffer(bufferID)
        }
        session.bufferService?.cutIntoBufferHandler = { [weak coordinator] bufferID in
            coordinator?.cutIntoBuffer(bufferID)
        }

        // Structural editing bridges (T-0214) — let the Scene/Chapter menu items invoke
        // the same handlers as the ⌘↩ / ⌘⇧↩ / ⌘⌫ / ⌘⇧⌫ keys. Take focus first so the
        // operation acts on the manuscript caret (menu clicks don't change first
        // responder), matching what the keyboard path already has.
        // ✅ EP-046 E2-S4: Edit ▸ Find acts on this manuscript; Replace All is one grouped step (Q2).
        textView.finderClient.replaceAllHandler = { [weak coordinator] edits in coordinator?.replaceAll(edits) }
        session.findAction = { [weak coordinator, weak textView] command in
            coordinator?.takeFocus(); textView?.performFind(command)
        }
        // ✅ EP-046 E2-S3: the Format menu acts on the manuscript's own selection.
        session.formatAction = { [weak coordinator, weak textView] format in
            coordinator?.takeFocus(); textView?.applyFormat(format)
        }
        session.createSceneAction   = { [weak coordinator] in coordinator?.takeFocus(); coordinator?.handleCreateScene() }
        session.createChapterAction = { [weak coordinator] in coordinator?.takeFocus(); coordinator?.handleCreateChapter() }
        session.mergeSceneAction    = { [weak coordinator] in coordinator?.takeFocus(); coordinator?.handleMergeScene() }
        session.mergeChapterAction  = { [weak coordinator] in coordinator?.takeFocus(); coordinator?.handleMergeChapter() }

        // Scene/Chapter boundary navigation — menu-only until a key equivalent is ruled.
        // Same take-focus-first discipline: a menu click leaves first responder alone, and
        // moving the caret in an unfocused text view would hide the very thing the command
        // exists to show.
        session.sceneStartAction = { [weak coordinator, weak textView] in
            guard let tv = textView else { return }
            coordinator?.takeFocus(); coordinator?.moveToSceneBoundary(.start, in: tv)
        }
        session.sceneEndAction = { [weak coordinator, weak textView] in
            guard let tv = textView else { return }
            coordinator?.takeFocus(); coordinator?.moveToSceneBoundary(.end, in: tv)
        }
        session.chapterStartAction = { [weak coordinator, weak textView] in
            guard let tv = textView else { return }
            coordinator?.takeFocus(); coordinator?.moveToChapterBoundary(.start, in: tv)
        }
        session.chapterEndAction = { [weak coordinator, weak textView] in
            guard let tv = textView else { return }
            coordinator?.takeFocus(); coordinator?.moveToChapterBoundary(.end, in: tv)
        }
        // ⚠️ [T-0568] — Manuscript Start / End. ✅ Same take-focus-first discipline; the
        // navigator follows the viewport on its own.
        session.manuscriptStartAction = { [weak coordinator, weak textView] in
            guard let tv = textView else { return }
            coordinator?.takeFocus(); coordinator?.moveToManuscriptBoundary(.start, in: tv)
        }
        session.manuscriptEndAction = { [weak coordinator, weak textView] in
            guard let tv = textView else { return }
            coordinator?.takeFocus(); coordinator?.moveToManuscriptBoundary(.end, in: tv)
        }

        let scroll = NSScrollView()
        scroll.documentView = textView
        scroll.hasVerticalScroller = true
        scroll.autohidesScrollers = true

        // Observe scroll position to update the viewport scene (Navigator highlight).
        // Does not trigger any load/release — purely for highlight tracking.
        scroll.contentView.postsBoundsChangedNotifications = true
        NotificationCenter.default.addObserver(
            context.coordinator,
            selector: #selector(Coordinator.scrollDidChange(_:)),
            name: NSView.boundsDidChangeNotification,
            object: scroll.contentView
        )
        // ⚠️ [I-0273] A RESIZE posts FRAME-changed, not bounds-changed (AppKit), so the
        // observer above never hears the window giving the manuscript its height. The
        // restore needs exactly that moment.
        scroll.contentView.postsFrameChangedNotifications = true
        NotificationCenter.default.addObserver(
            context.coordinator,
            selector: #selector(Coordinator.clipFrameDidChange(_:)),
            name: NSView.frameDidChangeNotification,
            object: scroll.contentView
        )
        // ⚠️ [I-0273] …and the TEXT's own height: TextKit 2 keeps revising its document-height
        // ESTIMATE as it lays out (logged 237,598 → 640,264 pt within a second), and a fixed
        // scroll offset then shows different text. That change arrives as the document view's
        // frame change.
        textView.postsFrameChangedNotifications = true
        NotificationCenter.default.addObserver(
            context.coordinator,
            selector: #selector(Coordinator.clipFrameDidChange(_:)),
            name: NSView.frameDidChangeNotification,
            object: textView
        )

        return scroll
    }

    func updateNSView(_ scrollView: NSScrollView, context: Context) {
        guard let tv = scrollView.documentView as? NSTextView else { return }
        let coordinator = context.coordinator

        // Keep coordinator current so delegate callbacks always see the latest env and loader.
        coordinator.parent = self

        // Rebuild text storage when the segment list, the chapter-title toggle, or any chapter
        // title changes. A rename leaves the segment IDs identical (same scenes, new heading text),
        // so the title fingerprint is what forces the heading to refresh after a rename (I-0095).
        // T-0531 DIAGNOSTIC — updateNSView runs on EVERY SwiftUI update, and the GUARD
        // below is O(N): a 1,156-element map plus a full allScenes walk.
        let __u0 = Date()
        let segIDs = loader.segments.map(\.id)
        let chapterTitleFingerprint = coordinator.chapterHeadingFingerprint(for: loader)
        let __uGuard = Date()
        defer {
            let total = Date().timeIntervalSince(__u0) * 1000
            if total > 0.5 {
                NSLog(String(format: "[SCRIVI-UPD] updateNSView total=%.1f  guard=%.1f ms",
                             total, __uGuard.timeIntervalSince(__u0) * 1000))
            }
        }
        // ✅ EP-047 S2: a new face or size re-presents through the rebuild — attributes only; no history event, and the
        // scene text (so the `.md`) is untouched.
        let typographyChanged = typography != coordinator.presenter.typography
        // ✅ EP-047 S4: hints on/off — attributes only, never a rebuild.
        if markupHints != coordinator.presenter.hintsEnabled, let ts = tv.textStorage {
            coordinator.presenter.setHints(markupHints, selection: tv.selectedRange(), in: ts)
        }
        if segIDs != coordinator.lastSegmentIDs
            || showChapterTitles != coordinator.lastShowChapterTitles
            || chapterTitleFingerprint != coordinator.lastChapterTitleFingerprint
            || typographyChanged {
            coordinator.lastSegmentIDs = segIDs
            coordinator.lastShowChapterTitles = showChapterTitles
            coordinator.lastChapterTitleFingerprint = chapterTitleFingerprint
            if typographyChanged {
                // ✅ [I-0282]: a new face or size must not move the writer's place — ONE method does the whole sequence.
                coordinator.applyTypography(typography, to: tv, segments: loader.segments)
            } else {
                coordinator.rebuildStorage(tv, segments: loader.segments)
            }
        }

        // Resume the last-session writing surface once, after storage is built (I-0058).
        // The loader was seeded with the restored scene (viewportSceneID) and scene-local
        // cursor offset by loadAll(); place the cursor there and scroll to it.
        if !coordinator.didRestoreSurface {
            coordinator.didRestoreSurface = true
            coordinator.restoreWritingSurface(in: tv)
        }

        if let targetID = navigateToSceneID {
            coordinator.navigateToScene(targetID, in: tv)
            // Reset the binding after the current update pass completes —
            // mutating bound state inside updateNSView is a view-update violation.
            DispatchQueue.main.async {
                navigateToSceneID = nil
            }
        }

    }

    // MARK: — Coordinator

    @MainActor
    final class Coordinator: NSObject, NSTextViewDelegate {
        var parent: ManuscriptTextView
        weak var textView: NSTextView?

        // ⚠️ I-0131: while set, scroll-driven viewport retargeting is suppressed.
        // An explicit navigation (navigator click, deep link) sets it so the scroll it
        // *causes* cannot overwrite the scene the writer actually asked for. Expires on
        // its own, so hand-scrolling is unaffected a moment later.
        var navigationLockUntil: Date?

        // Character range for each scene segment in the NSTextStorage.
        // Dividers occupy 1 character each between segments.
        // ✅ EP-045 AC11 (T-0583): MAINTAINED by `boundaryTable` from each edit; read here, it
        // is current or rescanned — never a stale copy (ONE authority, [I-0131]).
        let boundaryTable = SceneBoundaryTable()
        var sceneBoundaries: [NSRange] {
            guard let storage = textView?.textStorage else { return [] }
            return boundaryTable.ranges(in: storage)
        }

        var lastSegmentIDs: [String] = []
        var lastShowChapterTitles: Bool = false
        // Fingerprint of the resolved chapter headings (ordered chapterID→title pairs). A rename
        // changes a title without changing any segment ID, so this is what makes updateNSView
        // rebuild the storage — and thus refresh the heading — after a rename (I-0095).
        var lastChapterTitleFingerprint: String = ""

        // Ordered "chapterID=title" join across the loaded scenes — the raw stored titles the
        // heading builder reads (empty titles included, so a cleared title also shifts the print
        // and triggers a rebuild). Cheap: one pass over allScenes, deduped by chapter.
        func chapterHeadingFingerprint(for loader: ViewportSceneLoader) -> String {
            var seen = Set<String>()
            var parts: [String] = []
            for info in loader.allScenes where seen.insert(info.chapterID).inserted {
                parts.append("\(info.chapterID)=\(info.chapterTitle)")
            }
            return parts.joined(separator: "\u{1f}")
        }
        private var saveTask: Task<Void, Never>?
        private var titleTask: Task<Void, Never>?
        private var highlightTask: Task<Void, Never>?
        private var scrollTask: Task<Void, Never>?

        // Set true during rebuildStorage to suppress textDidChange callbacks
        // that fire from NSTextStorage delegate notifications mid-rebuild.
        private var isRebuilding: Bool = false

        // Tracks which segment index the cursor was in at last check.
        private var lastCursorSegmentIndex: Int = 0

        // Set true after the first updateNSView restores the last-session writing
        // surface (I-0058), so resume runs exactly once per editor lifetime.
        var didRestoreSurface: Bool = false

        // Set by textDidChange so the immediately-following selection-change
        // callback (fired on the same run loop turn) does not treat an edit as a
        // pure cursor move; cleared each turn. History trigger #2 (§4.a).
        private var editInFlight: Bool = false

        // Set by makeNSView; call to transfer first-responder directly in AppKit.
        var onTakeFocus: (() -> Void)?

        func takeFocus() { onTakeFocus?() }

        // Inline fork popover shown when an undo/redo step lands on a fork
        // (SP-055 / §10 T2, T-0211). Owned by the coordinator; anchored at the
        // caret in the text view.
        private let forkPopover = ForkPopoverController()

        init(_ parent: ManuscriptTextView) { self.parent = parent }
        /// EP-046 E2-S1 — presents headings and hides escape backslashes (see `ManuscriptPresenter`).
        let presenter = ManuscriptPresenter()

        // MARK: — History capture helpers (EP-019)

        // The loaded segment at `index`, or nil if out of range.
        func loader(for index: Int) -> SceneSegment? {
            parent.loader.segments.indices.contains(index) ? parent.loader.segments[index] : nil
        }

        // Scene-local UTF-8 byte offset of the insertion point. `sceneText` is
        // the scene's full text, `storageLoc` the storage char offset, `segRange`
        // the scene's storage char range. History offsets are UTF-8 bytes (§4.b).
        func sceneLocalByteOffset(in sceneText: String, storageLoc: Int, segRange: NSRange) -> Int {
            let charOffset = max(0, min(storageLoc - segRange.location, (sceneText as NSString).length))
            let prefix = (sceneText as NSString).substring(to: charOffset)
            return prefix.utf8.count
        }

        // True if the character just typed (at cursorCharOffset-1 within the
        // scene) is a sentence terminator or newline — history trigger #1 (§4.a).
        // Trailing whitespace after a terminator does NOT re-commit: it starts the
        // next pending edit and is deferred by the soft cursor-move trigger until
        // real text follows (whitespace-only-delta rule in HistoryCapture.flush).
        func isCommitBoundary(_ sceneText: String, cursorCharOffset: Int) -> Bool {
            let ns = sceneText as NSString
            let idx = cursorCharOffset - 1
            guard idx >= 0, idx < ns.length else { return false }
            let ch = ns.character(at: idx)
            // '.' 46, '!' 33, '?' 63, '\n' 10, '\r' 13
            return ch == 46 || ch == 33 || ch == 63 || ch == 10 || ch == 13
        }

        // MARK: — Undo / Redo apply path (T-0205, design §8)

        var canUndo: Bool { parent.session.historyCapture?.canUndoNow ?? false }
        var canRedo: Bool { parent.session.historyCapture?.canRedoNow ?? false }

        // Performs an undo: asks HistoryService for the prior state, applies the
        // returned full scene text into that scene's storage range, restores the
        // cursor, and saves immediately.
        func performUndo() {
            guard let capture = parent.session.historyCapture else { return }
            apply(step: capture.undo(), fallbackMessage: "Nothing to undo")
        }

        func performRedo() {
            guard let capture = parent.session.historyCapture else { return }
            apply(step: capture.redo(), fallbackMessage: nil)
        }

        // Applies a step result (undo or redo). A step may be blocked by a
        // structural barrier (§4.5) — in that case we surface the notice and do
        // not mutate text.
        private func apply(step: HistoryStepResult?, fallbackMessage: String?) {
            guard let step else { return }
            guard let capture = parent.session.historyCapture else { return }

            if let barrier = step.stoppedAtBarrier {
                presentBarrierNotice(barrier.note.isEmpty
                    ? "Can't undo past this point." : barrier.note)
                capture.refreshCanState()
                return
            }

            // Reversible structural step (T-0356 / AC6): the pointer crossed a structuredCut/
            // structuredPaste node. Run the inverse fragment op on disk + reload — no text change.
            if let inverse = step.structuralInverse {
                applyStructuralInverse(inverse)
                capture.refreshCanState()
                return
            }

            guard step.moved, !step.changes.isEmpty,
                  let tv = textView, let storage = tv.textStorage else {
                // A step that did not move (nothing left to undo/redo) still
                // means we are no longer on the previous fork — dismiss it.
                forkPopover.close()
                capture.refreshCanState()
                return
            }

            // ⚠️ [I-0270] A step can carry SEVERAL scene changes — an edit group (a delete or
            // cut across scene breaks) undoes/redoes as ONE step. It used to apply only
            // `changes.first`, which would have restored one scene of the group and silently
            // dropped the rest. ✅ Apply every change, then leave the caret in the EARLIEST
            // scene the step touched (where the cross-scene edit began).
            // ⚠️ [SP-164] live pass: undoing a Replace All across ~880 scenes moved the caret and SCROLLED to each scene in
            // turn. ✅ A multi-scene step places the caret once, below.
            let single = step.changes.count == 1
            var placed: [(segIdx: Int, caret: Int)] = []
            for change in step.changes {
                if let p = applySceneChange(change, in: tv, storage: storage, capture: capture, placesCaret: single) {
                    placed.append(p)
                }
            }
            if !single, let first = placed.min(by: { $0.segIdx < $1.segIdx }) {
                tv.setSelectedRange(NSRange(location: first.caret, length: 0))
                tv.scrollRangeToVisible(NSRange(location: first.caret, length: 0))
                if parent.loader.segments.indices.contains(first.segIdx) {
                    capture.syncCommittedText(parent.loader.segments[first.segIdx].text)
                }
            }
            capture.refreshCanState()

            // Fork popover (§10 T2, T-0211): if this step landed on a fork show
            // the branch chooser at the caret; otherwise the writer has moved off
            // any prior fork, so dismiss a lingering popover.
            if let fork = step.forkAhead {
                presentForkPopover(fork, in: tv)
            } else {
                forkPopover.close()
            }

            // Session-boundary warning (§5, T-0209): the engine flags the first
            // undo that steps into a previous session's work (once per crossing).
            if step.crossedSessionBoundary {
                presentSessionBoundaryNotice(boundaryTimestamp: step.boundaryTimestamp)
            }
        }

        // Applies ONE scene's undo/redo text into its storage range, places the caret,
        // syncs the loader and saves. Returns the segment and caret it used, or nil when
        // the scene is not loaded. (Body unchanged from when `apply` handled one change.)
        private func applySceneChange(_ change: HistorySceneChange, in tv: NSTextView,
                                      storage: NSTextStorage,
                                      capture: HistoryCapture,
                                      placesCaret: Bool = true) -> (segIdx: Int, caret: Int)? {
            // Map the changed scene to its loaded segment / storage range.
            guard let segIdx = parent.loader.segments.firstIndex(where: { $0.sceneID == change.sceneID }) else {
                return nil
            }

            ensureBoundaries(tv)
            guard sceneBoundaries.indices.contains(segIdx) else { return nil }
            let range = sceneBoundaries[segIdx]
            var caret = range.location

            // Apply the full scene text into the scene's boundary under the
            // rebuild/apply guard so textDidChange does not record it as an edit.
            // Dividers and scriviHeading runs sit outside [range] and are untouched.
            capture.withApplying {
                // Match rebuildStorage's body-text attributes exactly (regular
                // monospaced font, adaptive text color) so replaced text does not
                // pick up typing attributes (e.g. bold) or a mismatched color.
                // I-0112: .foregroundColor must be set explicitly here as well as in
                // rebuildStorage — omitting it renders literal black, so an undo/redo
                // would otherwise re-blacken a scene's text under Dark Mode even after
                // the initial build was fixed.
                // ✅ EP-047 AC4: the ONE source — the face survives an undo.
                let attrs = presenter.typography.bodyAttributes
                storage.replaceCharacters(in: range, with: NSAttributedString(string: change.newText, attributes: attrs))
                ensureBoundaries(tv)

                // Restore the cursor: scene-local UTF-8 byte offset → storage char.
                if sceneBoundaries.indices.contains(segIdx) {
                    let newRange = sceneBoundaries[segIdx]
                    let sceneText = (tv.string as NSString).substring(with: newRange)
                    let charOffset = charOffsetForByteOffset(Int(change.cursorAfter), in: sceneText)
                    let storageLoc = min(newRange.location + charOffset, (tv.string as NSString).length)
                    caret = storageLoc
                    if placesCaret {
                        tv.setSelectedRange(NSRange(location: storageLoc, length: 0))
                        tv.scrollRangeToVisible(NSRange(location: storageLoc, length: 0))
                    }
                }
            }

            // Keep the loader's in-memory text in sync and save immediately.
            parent.loader.updateText(change.newText, at: segIdx)
            // The restored text is now the whitespace-delta reference.
            capture.syncCommittedText(change.newText)
            let env = parent.env
            let loader = parent.loader
            if let ref = env.authorshipRef {
                Task { @MainActor in
                    await loader.saveScene(at: segIdx, engine: env.engine, ref: ref)
                }
            }
            return (segIdx, caret)
        }

        // Shows the inline fork popover for `fork` at the caret. Selecting a
        // branch re-primaries the fork (selectBranch) then redoes onto it;
        // dismissing without a choice leaves the primary child in place (§10 T2).
        private func presentForkPopover(_ fork: HistoryForkAhead, in tv: NSTextView) {
            guard let _ = parent.session.historyCapture else { return }
            forkPopover.show(
                fork: fork,
                in: tv,
                onSelect: { [weak self] childEventID in
                    guard let self, let capture = self.parent.session.historyCapture else { return }
                    // Re-primary the fork, then walk forward onto the chosen
                    // branch. redo() returns the step to apply just like a normal
                    // redo; apply() also handles a possible nested fork ahead.
                    guard capture.selectBranch(forkNodeID: fork.nodeID, childEventID: childEventID) else { return }
                    self.apply(step: capture.redo(), fallbackMessage: nil)
                },
                onCancel: { /* leave the existing primary child in place */ })
        }

        // Warns that undo has stepped into a previous session's changes, showing
        // the boundary's wall-clock time when available.
        private func presentSessionBoundaryNotice(boundaryTimestamp: String?) {
            let alert = NSAlert()
            alert.alertStyle = .informational
            if let ts = boundaryTimestamp, let when = HistoryTimestamp.friendly(ts) {
                alert.messageText = "Undoing changes from a previous session"
                alert.informativeText = "You are now undoing changes made \(when)."
            } else {
                alert.messageText = "Undoing changes from a previous session"
                alert.informativeText = "You are now undoing changes made before this session."
            }
            alert.addButton(withTitle: "OK")
            alert.runModal()
        }

        // Scene-local UTF-8 byte offset → character offset within `sceneText`.
        private func charOffsetForByteOffset(_ byteOffset: Int, in sceneText: String) -> Int {
            guard byteOffset > 0 else { return 0 }
            var bytes = 0
            var chars = 0
            for scalar in sceneText.unicodeScalars {
                if bytes >= byteOffset { break }
                bytes += String(scalar).utf8.count
                chars += scalar.utf16.count   // NSString char units
            }
            return chars
        }

        private func presentBarrierNotice(_ message: String) {
            let alert = NSAlert()
            alert.messageText = message
            alert.alertStyle = .informational
            alert.addButton(withTitle: "OK")
            alert.runModal()
        }

        // redo-of-a-cut needs the scene/chapter IDs that undo-of-that-cut re-pasted, so it can
        // remove them again. Keyed by the structural payload's stable identity (op + caret +
        // fragment), which is identical on the undo and the matching redo (same history node).
        // Session-local: a redo only ever follows an undo within the same run. (T-0356)
        private var structuralRedoCreated: [String: (scenes: [String], chapters: [String])] = [:]

        private func structuralCacheKey(_ p: HistoryStructuralPayload) -> String {
            "\(p.op)|\(p.caretSceneID)|\(p.caretByte)|\(p.fragmentJSON.hashValue)"
        }

        // Runs the inverse of a structuredCut/structuredPaste on disk, then reloads (T-0356 / AC6).
        // The four directions reduce to two disk ops — a paste-splice or its uncut inverse:
        //   undo-cut    → paste the extracted fragment back at the fold point
        //   redo-paste  → paste the same fragment at the target again
        //   undo-paste  → uncut the created scenes/chapters (strip pasted text, re-join target)
        //   redo-cut    → uncut the scenes the undo-cut just re-pasted
        private func applyStructuralInverse(_ inverse: HistoryStructuralInverse) {
            let p = inverse.payload
            guard let rootPath = parent.session.projectRootPath,
                  let ref = parent.env.authorshipRef,
                  let projectID = parent.session.openProjectResult?.projectID else { return }
            let key = structuralCacheKey(p)

            // Does this direction re-INSERT the fragment (a paste) or REMOVE it (an uncut)?
            let isPaste = (p.op == "structuredCut" && inverse.direction == "undo")
                       || (p.op == "structuredPaste" && inverse.direction == "redo")

            if isPaste {
                guard let result = try? parent.env.engine.fragmentPaste(
                    projectRootPath: rootPath,
                    appSupportRoot: parent.session.appSupportRoot,
                    projectID: projectID,
                    fragmentJSON: p.fragmentJSON,
                    caretSceneID: p.caretSceneID,
                    caretByteOffset: p.caretByte,
                    authorshipRef: ref) else {
                    print("[Scrivi] applyStructuralInverse: paste failed"); return
                }
                // Remember what this paste created so the matching uncut direction can remove it.
                structuralRedoCreated[key] = (result.createdSceneIDs, result.createdChapterIDs)
                reloadManuscriptFromDisk(caretSceneID: result.targetSceneID, caretByteOffset: p.caretByte)
            } else {
                // Uncut: remove the created scenes/chapters and re-join the target. For undo-paste
                // the created IDs are the paste's own (carried in the payload); for redo-cut they
                // are what the preceding undo-cut re-pasted (from the session cache).
                let created = structuralRedoCreated[key]
                let sceneIDs   = created?.scenes   ?? p.createdSceneIDs
                let chapterIDs = created?.chapters ?? p.createdChapterIDs
                guard let result = try? parent.env.engine.fragmentUncutPaste(
                    projectRootPath: rootPath,
                    fragmentJSON: p.fragmentJSON,
                    targetSceneID: p.caretSceneID,
                    createdSceneIDs: sceneIDs,
                    createdChapterIDs: chapterIDs) else {
                    print("[Scrivi] applyStructuralInverse: uncut failed"); return
                }
                structuralRedoCreated[key] = nil   // consumed; a later paste re-populates it
                reloadManuscriptFromDisk(caretSceneID: result.survivingSceneID, caretByteOffset: p.caretByte)
            }
        }

        // [I-0273] The restore's centring target, held until the scroll view has a height
        // and kept while the window and TextKit 2 settle.
        // ⛔ 2nd attempt stopped after a FIXED 1 s: the user's log showed it centred correctly
        // at viewH=493, then the view grew to 787 and the document-height estimate went
        // 237,598 → 640,264 pt — after the window closed — and the centre drifted ~145,000
        // characters back (to 1,710,569 against a target of 1,855,739).
        // ✅ Now: re-centre on ANY clip or document frame change, but ONLY when the target has
        // left the visible rect (never a no-op scroll, never a loop); END on the writer's first
        // key, click or scroll (`cancelRestoreCentre`), with a 5 s safety cap.
        private var pendingRestoreCentre: (target: Int, sceneID: String)?
        private var restoreCentreUntil: Date?
        private var isCentringRestore = false

        func cancelRestoreCentre() {
            pendingRestoreCentre = nil
            restoreCentreUntil = nil
        }

        @objc func clipFrameDidChange(_ notification: Notification) {
            guard let pending = pendingRestoreCentre, !isCentringRestore, let tv = textView,
                  let clip = tv.enclosingScrollView?.contentView, clip.bounds.height > 0 else { return }
            if let until = restoreCentreUntil, Date() > until { cancelRestoreCentre(); return }
            // Still on screen → leave the scroll alone.
            if let rect = boundingRect(forCharacterIndex: pending.target, in: tv),
               clip.bounds.intersects(rect) { return }
            applyPendingRestoreCentre(in: tv)
        }

        private func applyPendingRestoreCentre(in tv: NSTextView) {
            guard let pending = pendingRestoreCentre else { return }
            if restoreCentreUntil == nil { restoreCentreUntil = Date().addingTimeInterval(5.0) }
            isCentringRestore = true
            defer { isCentringRestore = false }
            navigationLockUntil = Date().addingTimeInterval(0.5)
            centerStorageOffset(pending.target, in: tv)
            parent.loader.setViewportScene(pending.sceneID)
        }

        // Called by NSScrollView bounds-change notification.
        // Updates the viewport scene (Navigator highlight) based on scroll position.
        // Does not load or release any scenes.
        @objc func scrollDidChange(_ notification: Notification) {
            guard let clipView = notification.object as? NSClipView,
                  let tv = textView else { return }

            // Record the document scroll fraction so the next save persists it (I-0058).
            if let docView = clipView.documentView {
                let scrollable = max(0, docView.bounds.height - clipView.bounds.height)
                let fraction = scrollable > 0 ? Double(clipView.bounds.minY / scrollable) : 0
                parent.loader.updateScrollFraction(fraction)
            }

            // Use the center of the visible area as the anchor so the scene only
            // changes when a boundary has clearly passed the midpoint of the viewport.
            let centerY = clipView.bounds.minY + clipView.bounds.height / 2
            scrollTask?.cancel()
            let loader = parent.loader
            scrollTask = Task { @MainActor in
                try? await Task.sleep(nanoseconds: 120_000_000) // 120ms debounce
                guard !Task.isCancelled else { return }
                // I-0131: an explicit navigation owns the viewport for its lock window.
                if let until = self.navigationLockUntil {
                    if Date() < until { return }
                    self.navigationLockUntil = nil
                }
                ensureBoundaries(tv)
                guard let charIdx = self.characterIndex(atPoint: NSPoint(x: 0, y: centerY), in: tv),
                      let segIdx = segmentIndex(for: charIdx),
                      loader.segments.indices.contains(segIdx) else { return }
                let sceneID = loader.segments[segIdx].sceneID
                loader.setViewportScene(sceneID)
            }
        }

        // Rebuild the entire NSTextStorage from the current segments.
        // Called when the segment list changes (new scene inserted, viewport shifted, toggle flipped).
        func rebuildStorage(_ tv: NSTextView, segments: [SceneSegment]) {
            // T-0573 — remember how far below the top of the viewport the caret sits, so
            // `revealCaretAfterRebuild` can put it back at the same height.
            caretViewportOffset = caretOffsetInViewport(tv)
            // Suppress delegate callbacks and undo registration during rebuild.
            // NSTextStorage fires textDidChange synchronously mid-rebuild, before
            // sceneBoundaries is valid — which would corrupt segment text extraction.
            isRebuilding = true
            let undoManager = tv.undoManager
            undoManager?.disableUndoRegistration()
            defer {
                undoManager?.enableUndoRegistration()
                isRebuilding = false
            }

            // ⚠️ [I-0196] instrumentation. This is the prime suspect for the hang
            // AFTER loadAll: it builds ONE NSTextStorage across every segment,
            // and the chapter-heading lookup below is O(N) PER SEGMENT (see the
            // tick inside the loop).
            NSLog("[SCRIVI-TK] engine at rebuildStorage: %@ (segments=%d)",
                  tv.textLayoutManager != nil ? "TextKit 2" : "TextKit 1 (DOWNGRADED)",
                  segments.count)
            NSLog("[SCRIVI-TIMING] >>> entering: rebuildStorage segments=\(segments.count)")
            let rebuildStart = Date()
            defer {
                NSLog(String(format: "[SCRIVI-TIMING] <<< rebuildStorage took %.1f s",
                             Date().timeIntervalSince(rebuildStart)))
            }
            let storage = tv.textStorage!
            storage.beginEditing()
            storage.setAttributedString(NSAttributedString(string: ""))

            // ✅ EP-047 AC4: the ONE source (font, line height). I-0112: it carries `.foregroundColor` — a run without
            // it renders as literal NSColor.black, invisible in Dark Mode.
            let attrs = presenter.typography.bodyAttributes
            let showTitles = parent.showChapterTitles

            var built: [NSRange] = []
            var offset = 0

            for (i, seg) in segments.enumerated() {
                ScriviDiag.tick("rebuildStorage", i, of: segments.count)
                let isChapterBoundary = i == 0 || segments[i - 1].chapterID != seg.chapterID

                if i > 0 {
                    // Insert divider between every pair of adjacent scenes.
                    // ✅ T-0575 — the divider that CLOSES a chapter is drawn at full strength.
                    let state: DividerRenderState = isChapterBoundary ? .chapterEnd : .sceneBreak
                    let divStr = SceneDivider.string(makeDividerAttachment(state), state: state,
                                                     newlineAttributes: attrs)
                    storage.append(divStr)
                    offset += divStr.length
                }

                // Insert chapter heading at every chapter boundary (including the first scene).
                if showTitles && isChapterBoundary {
                    let chapterTitle = parent.loader.allScenes
                        .first(where: { $0.sceneID == seg.sceneID })
                        .map { info -> String in
                            let t = info.chapterTitle.trimmingCharacters(in: .whitespaces)
                            if !t.isEmpty { return t }
                            let chapterIDs = parent.loader.allScenes.map(\.chapterID)
                            var seen: [String: Int] = [:]
                            var ordinal = 0
                            for cid in chapterIDs {
                                if seen[cid] == nil { ordinal += 1; seen[cid] = ordinal }
                            }
                            return "Chapter \(seen[info.chapterID] ?? ordinal)"
                        } ?? ""
                    // ✅ EP-047 P7 + P9: chapter titles in the manuscript face, at Heading 1 size, in the text colour.
                    var headingAttrs = presenter.typography.chapterTitleAttributes
                    headingAttrs[.scriviHeading] = true
                    let headingStr = NSAttributedString(
                        string: i == 0 ? "\(chapterTitle)\n" : "\n\(chapterTitle)\n",
                        attributes: headingAttrs
                    )
                    storage.append(headingStr)
                    offset += headingStr.length
                }

                let start = offset
                let segStr = NSAttributedString(string: seg.text, attributes: attrs)
                storage.append(segStr)
                offset += segStr.length

                built.append(NSRange(location: start, length: segStr.length))
            }

            storage.endEditing()
            // ✅ EP-045 AC11: the ranges were built exactly while appending — install them, so
            // the whole-storage edit above (which marked the table dirty) costs no rescan.
            boundaryTable.reset(built)

            // Sync the change-detection keys so the NEXT updateNSView pass — which the
            // @Observable segment mutation that prompted this rebuild will schedule — sees no
            // change and does NOT rebuild storage again. A redundant rebuild there re-runs
            // setAttributedString and drops the selection to 0, which stole the caret after a
            // scene/chapter merge (SP-075). Any handler that rebuilds storage then places the
            // caret can now trust that placement to survive.
            lastSegmentIDs = segments.map(\.id)
            lastShowChapterTitles = parent.showChapterTitles
            // ⚠️ [I-0272] ALL THREE keys `updateNSView` compares — the chapter-title
            // fingerprint (added by I-0095) was never synced here. A chapter create/merge
            // renumbers titles, so the next update pass saw a "changed" fingerprint and
            // rebuilt AGAIN — after the handler had placed the caret — undoing T-0573's
            // scroll and I-0271's viewport layout (the blank band came back).
            lastChapterTitleFingerprint = chapterHeadingFingerprint(for: parent.loader)
        }

        // MARK: — Copy buffers (EP-019 SP-056, T-0214)
        //
        // Ten buffers 0–9; buffer 0 is the system pasteboard (ordinary ⌘C/⌘V, never
        // touched here). Slots 1–9 are ScriviCore-persisted. ⌘N/⇧⌘N route here from
        // ManuscriptNSTextView.keyDown. These mutate text via the same paths the
        // native paste/cut overrides use, so history capture + auto-save happen
        // through the existing textDidChange flow (Trade T3).

        // ⌘N (or the palette's plain button): copy the selection into slot N. Copy-only
        // — the selection is left in place and NO text changes, so per Trade T3 there is
        // no history event. A no-op when nothing is selected (the palette button can be
        // clicked with an empty selection; the keyboard chord likewise just no-ops).
        func copyIntoBuffer(_ bufferID: String) {
            guard let tv = textView else { return }
            let sel = tv.selectedRange()
            guard sel.length > 0 else { return }
            // Cross-boundary selection → store a STRUCTURED fragment (flat text + fragment) so
            // ⌃N can reconstruct the scene/chapter boundaries (T-0355 / AC4). Single-scene keeps
            // the flat-text slot (AC5). Copy is not a text change → no history event (Trade T3).
            if selectionCrossesBoundary(sel),
               let spans = fragmentSpans(for: sel),
               let rootPath = parent.session.projectRootPath,
               let frag = try? parent.env.engine.fragmentExtract(projectRootPath: rootPath, spans: spans) {
                parent.session.bufferService?.load(frag.plainText, intoSlot: bufferID,
                                                   fragmentJSON: frag.toJSON())
                return
            }
            let text = (tv.string as NSString).substring(with: sel)
            parent.session.bufferService?.load(text, intoSlot: bufferID)
        }

        // ⌥N (or the palette's Option button): cut the selection into slot N. This DOES
        // mutate text, so it records a `cut` history event tagged with bufferID (Trade
        // T3). The store load happens first (so the slot holds the text even if the
        // delete is later undone), then the delete runs through AppKit and is captured
        // as the cut event by the surrounding beginCutIntoBuffer/flush bracket. A no-op
        // when nothing is selected.
        func cutIntoBuffer(_ bufferID: String) {
            guard let tv = textView, let mtv = tv as? ManuscriptNSTextView else { return }
            // The palette lives in a floating panel; a click there leaves the text view
            // not-first-responder, so its delete would be dropped. Restore focus first
            // (a no-op on the keyboard path, where the text view is already first
            // responder). The selection is preserved across the focus change.
            tv.window?.makeFirstResponder(tv)
            let sel = tv.selectedRange()
            guard sel.length > 0 else { return }

            // ✅ [I-0270] Cross-boundary cut into a slot = the cross-boundary COPY into the slot
            // (unchanged, structured fragment) + delete-keeping-scenes. ⛔ No longer the
            // collapsing `fragmentCut` (user ruling 2026-10-02).
            if selectionCrossesBoundary(sel) {
                copyIntoBuffer(bufferID)
                deleteAcrossScenes(sel, kind: "cut")
                return
            }

            let text = (tv.string as NSString).substring(with: sel)
            // Load into the slot first — a cut must not lose the text if the store
            // write and the delete are ever separated by a failure.
            guard parent.session.bufferService?.load(text, intoSlot: bufferID) == true else { return }
            // Delete the selection through the same history bracket the native cut
            // uses, but tagged with the buffer slot. deleteSelectionForBuffer performs
            // the AppKit removal that textDidChange then records as the cut event.
            parent.session.historyCapture?.beginCutIntoBuffer(bufferID: bufferID)
            mtv.deleteSelectionForBuffer()
            parent.session.historyCapture?.flush(trigger: "cut", kind: "cut")
        }

        // ⌃N (or the palette's Control button): paste slot N at the caret, replacing any
        // selection. An empty slot is a silent no-op. A non-empty paste inserts text and
        // records an ordinary `paste` history event (one undo step) — the system
        // pasteboard is untouched.
        func pasteFromBuffer(_ bufferID: String) {
            guard let tv = textView, let mtv = tv as? ManuscriptNSTextView else { return }
            // Restore first responder so a palette-driven paste inserts into the editor
            // (the panel click steals focus; insertText on a non-first-responder text
            // view is dropped — the root cause of the palette paste doing nothing).
            // No-op on the keyboard path. This is why the earlier row-click paste failed.
            tv.window?.makeFirstResponder(tv)

            // Structured slot → reconstruct the carried scene/chapter boundaries at the caret
            // (T-0355 / AC4), the same path as ⌘V of a cross-boundary copy. A heading caret is
            // refused (flash) inside pasteStructuredFragment.
            if let frag = parent.session.bufferService?.fragment(inSlot: bufferID) {
                _ = pasteStructuredFragment(frag)
                return
            }

            guard let text = parent.session.bufferService?.text(inSlot: bufferID),
                  !text.isEmpty else { return }   // empty slot → silent no-op
            parent.session.historyCapture?.beginPasteOrCut(kind: "paste")
            mtv.insertTextForBuffer(text)
            parent.session.historyCapture?.flush(trigger: "paste", kind: "paste")
        }

        // MARK: — NSTextViewDelegate

        func textDidChange(_ notification: Notification) {
            guard !isRebuilding else { return }
            guard let tv = notification.object as? NSTextView else { return }
            // While a history undo/redo apply mutates storage, the resulting
            // textDidChange is not a user edit — skip capture (still update the
            // loader below so in-memory text stays in sync).
            let isHistoryApply = parent.session.historyCapture?.isApplying ?? false
            // A genuine keystroke commits the writer to the current branch, so
            // any open fork popover must dismiss rather than obstruct (§10 T2).
            if !isHistoryApply { forkPopover.close() }
            let loc = tv.selectedRange().location

            // T-0531 DIAGNOSTIC — per-keystroke cost, by step.
            let __k0 = Date()
            // Recompute boundaries from live storage — they shift with every keystroke.
            ensureBoundaries(tv)
            let __kBounds = Date()

            guard let segIdx = segmentIndex(for: loc) else { return }
            let __kSeg = Date()

            // Extract this segment's text from storage.
            let range = sceneBoundaries[segIdx]
            let extracted = (tv.string as NSString).substring(with: range)
            let __kExtract = Date()

            // The scene's text *before* this edit (for first-edit baseline seeding).
            let preEditText = loader(for: segIdx)?.text ?? ""

            // Update loader in-memory; segment stays loaded.
            parent.loader.updateText(extracted, at: segIdx)
            let __kUpdate = Date()
            defer {
                let ms = { (a: Date, b: Date) in b.timeIntervalSince(a) * 1000 }
                let total = Date().timeIntervalSince(__k0) * 1000
                if total > 0.5 {
                    NSLog(String(format:
                        "[SCRIVI-KEY] total=%.1f  bounds=%.1f  segIdx=%.1f  extract=%.1f  update=%.1f  rest=%.1f (len=%d)",
                        total, ms(__k0,__kBounds), ms(__kBounds,__kSeg), ms(__kSeg,__kExtract),
                        ms(__kExtract,__kUpdate), Date().timeIntervalSince(__kUpdate) * 1000,
                        tv.textStorage?.length ?? -1))
                }
            }

            if segIdx != lastCursorSegmentIndex {
                lastCursorSegmentIndex = segIdx
                parent.loader.setCurrentIndex(segIdx)
            }

            // History capture (EP-019 §4.a): latch the edit, then commit on a
            // sentence terminator or Return. `editInFlight` lets the following
            // selection-change callback know this run came from an edit.
            if !isHistoryApply, let capture = parent.session.historyCapture,
               let sceneID = loader(for: segIdx)?.sceneID {
                let cursorCharOffset = loc - range.location
                let cursorByte = sceneLocalByteOffset(in: extracted, storageLoc: loc, segRange: range)
                parent.loader.setCursorByteOffset(cursorByte, sceneID: sceneID)
                capture.noteEdit(sceneID: sceneID, text: extracted, cursor: cursorByte,
                                 baselineIfFirst: preEditText)
                editInFlight = true
                // Commit on a sentence terminator or Return — but NOT on trailing
                // whitespace (e.g. the second space of ". "). Whitespace is carried
                // into the next event once real text follows (§4.a refinement).
                if isCommitBoundary(extracted, cursorCharOffset: cursorCharOffset) {
                    capture.flush(trigger: "sentence")
                }
            }

            // Debounce 1-second auto-save.
            saveTask?.cancel()
            let loader = parent.loader
            let env = parent.env
            let session = parent.session
            saveTask = Task { @MainActor in
                try? await Task.sleep(nanoseconds: 1_000_000_000)
                guard !Task.isCancelled else { return }
                if let ref = env.authorshipRef {
                    // T-0396: this no longer seals the typing session. Mid-session
                    // it deliberately records nothing (so a continuously-typed
                    // sentence stays ONE entry); it commits only if the writer has
                    // already gone idle past the threshold. The scene is written to
                    // disk either way — see the §4.d note on `flushThenSave`.
                    session.historyCapture?.flushThenSave()
                    await loader.saveCurrentIfDirty(engine: env.engine, ref: ref)
                }
            }

            // Debounce 300ms live-title update for the Navigator.
            titleTask?.cancel()
            let sceneID = loader.segments.indices.contains(segIdx)
                ? loader.segments[segIdx].sceneID : nil
            titleTask = Task { @MainActor in
                try? await Task.sleep(nanoseconds: 300_000_000)
                guard !Task.isCancelled, let sid = sceneID else { return }
                let firstLine = extracted
                    .components(separatedBy: .newlines)
                    .first { !$0.trimmingCharacters(in: .whitespaces).isEmpty } ?? ""
                loader.updateLiveTitle(firstLine, forSceneID: sid)
                session.timelineModel?.updateDotTitles(liveTitles: loader.liveTitles, allScenes: loader.allScenes)
            }
        }

        func textViewDidChangeSelection(_ notification: Notification) {
            guard !isRebuilding else { return }
            guard let tv = notification.object as? NSTextView else { return }
            let loc = tv.selectedRange().location
            guard let segIdx = segmentIndex(for: loc) else { return }

            // History triggers #2 and #5 (§4.a). A selection change that did NOT
            // come from an edit this turn is a pure cursor move: commit any
            // pending edit. A scene switch also commits (against the previous
            // scene, which the pending edit already targets). An undo/redo apply
            // moves the selection too — skip capture in that case.
            let isHistoryApply = parent.session.historyCapture?.isApplying ?? false
            if !isHistoryApply, let capture = parent.session.historyCapture {
                let sceneSwitched = segIdx != lastCursorSegmentIndex
                if sceneSwitched {
                    // Leaving a scene: commit its pending edit (hard).
                    capture.flush(trigger: "sceneSwitch")
                } else if !editInFlight && capture.hasPendingChanges {
                    // In-scene cursor move: soft — defers a whitespace-only pending
                    // change so double-spaces never become a standalone undo step.
                    capture.flush(trigger: "cursorMove", soft: true)
                }
            }
            editInFlight = false

            // Publish the caret's scene-local byte offset so the history card can bold
            // the entry whose change the caret sits inside (user request, 2026-08-06).
            // Done on every selection change, not just edits, so plain arrow-key moves
            // update the highlight too.
            if let seg = loader(for: segIdx), sceneBoundaries.indices.contains(segIdx) {
                let byte = sceneLocalByteOffset(in: seg.text, storageLoc: loc,
                                                segRange: sceneBoundaries[segIdx])
                parent.loader.setCursorByteOffset(byte, sceneID: seg.sceneID)
            }

            if segIdx != lastCursorSegmentIndex {
                lastCursorSegmentIndex = segIdx
                parent.loader.setCurrentIndex(segIdx)
                // Cursor moved to a new scene — update viewport scene immediately.
                // Cancel any pending scroll-based update; cursor movement takes priority.
                scrollTask?.cancel()
                highlightTask?.cancel()
                let loader = parent.loader
                let sceneID = loader.segments.indices.contains(segIdx)
                    ? loader.segments[segIdx].sceneID : nil
                highlightTask = Task { @MainActor in
                    try? await Task.sleep(nanoseconds: 80_000_000) // 80ms debounce
                    guard !Task.isCancelled else { return }
                    loader.setViewportScene(sceneID)
                }
            }

            let manuscriptPos = storageOffsetToManuscriptPosition(loc)
            parent.loader.updateCursorPosition(manuscriptPos)

            // Record the scene-local cursor offset so the next save persists it as the
            // restored selection (I-0058). Boundaries were recomputed above via segmentIndex.
            if sceneBoundaries.indices.contains(segIdx) {
                let sceneLocalOffset = max(0, loc - sceneBoundaries[segIdx].location)
                parent.loader.updateSceneCursorOffset(sceneLocalOffset)
            }
        }

        // MARK: — Scene/Chapter creation and split/merge (called from ManuscriptNSTextView)

        // Cmd-Enter: split scene at cursor, or append empty scene if at end.
        func handleCreateScene() {
            // ⚠️ I-0213 — TIME THE STRUCTURAL OP ITSELF.
            // The ⌘-chord paths RETURN EARLY from `keyDown`, so its `defer` timer never
            // sees them. A 2.7 s hang was logged against a PLAIN Return and could not be
            // attributed to any action; this makes each op report its own cost.
            let __t0 = Date()
            defer {
                let ms = Date().timeIntervalSince(__t0) * 1000
                if ms > 0.5 { NSLog("[SCRIVI-STRUCT] createScene=%.1f ms", ms) }
            }
            guard let tv = textView else { return }
            let loc = tv.selectedRange().location
            guard let segIdx = segmentIndex(for: loc) else { return }

            let loader = parent.loader
            let env = parent.env
            let session = parent.session

            // Determine split offset within this segment's text.
            let segRange = sceneBoundaries.indices.contains(segIdx) ? sceneBoundaries[segIdx] : NSRange(location: loc, length: 0)
            let splitOffsetInSeg = max(0, loc - segRange.location)
            let currentText = loader.segments.indices.contains(segIdx) ? loader.segments[segIdx].text : ""
            let isAtEnd = splitOffsetInSeg >= currentText.count

            Task { @MainActor in
                guard let ref = env.authorshipRef,
                      let rootPath = session.projectRootPath,
                      let proj = session.openProjectResult,
                      loader.segments.indices.contains(segIdx)
                else { return }

                let currentSeg = loader.segments[segIdx]
                await loader.saveCurrentIfDirty(engine: env.engine, ref: ref)
                // Structural op — record a barrier so undo stops here (§4.5).
                session.historyCapture?.recordBarrier(kind: "sceneSplit", note: "Can't undo past creating a scene")

                // Caret at the very start of a (non-empty) scene → insert a new empty scene
                // BEFORE this one, exactly as a newline at position 0 would in one big buffer.
                // The current scene keeps all its text + title; the new empty scene precedes it.
                // This is the ONLY correct behaviour at offset 0 for any scene, including a
                // chapter's first scene (handled by the engine's beforeSceneID prepend). I-0096.
                let insertBefore = splitOffsetInSeg == 0 && !isAtEnd

                do {
                    let result = try env.engine.createScene(
                        projectRootPath: rootPath,
                        appSupportRoot: env.appSupportRoot,
                        projectID: proj.projectID,
                        chapterID: currentSeg.chapterID,
                        afterSceneID: insertBefore ? "" : currentSeg.sceneID,
                        beforeSceneID: insertBefore ? currentSeg.sceneID : "",
                        authorshipRef: ref
                    )

                    if insertBefore {
                        // Empty scene inserted before the current one; current scene untouched.
                        let newIdx = loader.insertScene(result, before: segIdx)
                        loader.setCurrentIndex(newIdx)
                        // Rebuild and drop the caret at the start of the new empty scene.
                        rebuildStorageAndPlaceCursor(at: newIdx, textOffset: 0)
                    } else if isAtEnd {
                        // Append empty scene — original behaviour.
                        let newIdx = loader.insertScene(result, after: segIdx)
                        loader.setCurrentIndex(newIdx)
                        insertDividerAndMoveCursor(after: segIdx, placeCursorAtStart: true)
                    } else {
                        // Split: head stays in current scene, tail goes to the new scene.
                        let headText = splitHead(of: currentText, at: splitOffsetInSeg)
                        let tailText = splitTail(of: currentText, at: splitOffsetInSeg)

                        // Save head into current scene.
                        // ⛔ [I-0259] — success is CAPTURED: `splitScene` marks both halves
                        // clean ("saved by caller"), so a failed half is re-marked dirty below.
                        let headSaved = (try? env.engine.saveScene(
                            projectID: proj.projectID,
                            projectRootPath: rootPath,
                            appSupportRoot: env.appSupportRoot,
                            sceneID: currentSeg.sceneID,
                            sceneMetadataPath: currentSeg.metadataPath,
                            sceneContentPath: currentSeg.contentPath,
                            markdown: headText,
                            authorshipRef: ref
                        )) != nil
                        // Save tail into new scene.
                        let tailSaved = (try? env.engine.saveScene(
                            projectID: proj.projectID,
                            projectRootPath: rootPath,
                            appSupportRoot: env.appSupportRoot,
                            sceneID: result.sceneID,
                            sceneMetadataPath: result.metadataPath,
                            sceneContentPath: result.contentPath,
                            markdown: tailText,
                            authorshipRef: ref
                        )) != nil
                        let newIdx = loader.splitScene(result, at: segIdx, headText: headText, tailText: tailText)
                        if !headSaved { loader.markDirty(at: segIdx) }   // [I-0259]
                        if !tailSaved { loader.markDirty(at: newIdx) }
                        loader.setCurrentIndex(newIdx)
                        insertDividerAndMoveCursor(after: segIdx, placeCursorAtStart: true)
                    }

                    // ⚠️ I-0213 — the other untimed block on this path. `reloadSceneDots`
                    // does one `getSceneStoryTime` C ABI call PER SCENE (1,174 of them),
                    // so it is a candidate for the remaining 2.1 s. Measured, not assumed.
                    // ⚠️ I-0213 — TIMED SEPARATELY. One timer spanning BOTH was misleading:
                    // it was labelled `reloadSceneDots` while most of the cost was actually
                    // in `updateDotTitles`, which `load()` never calls.
                    let __r0 = Date()
                    session.timelineModel?.reloadSceneDots(
                        engine: env.engine, projectRootPath: rootPath, scenes: loader.allScenes)
                    let __rMs = Date().timeIntervalSince(__r0) * 1000
                    let __t0t = Date()
                    session.timelineModel?.updateDotTitles(liveTitles: loader.liveTitles, allScenes: loader.allScenes)
                    let __tMs = Date().timeIntervalSince(__t0t) * 1000
                    if __rMs > 0.5 { NSLog("[SCRIVI-STRUCT]   reloadSceneDots=%.1f ms", __rMs) }
                    if __tMs > 0.5 { NSLog("[SCRIVI-STRUCT]   updateDotTitles=%.1f ms", __tMs) }
                } catch {
                    print("[Scrivi] createScene failed: \(error)")
                }
            }
        }

        // Shift-Cmd-Enter: split at cursor creating a new chapter, or append empty chapter at end.
        func handleCreateChapter() {
            // ⚠️ I-0213 — DO **NOT** TIME THE WHOLE FUNCTION HERE.
            //
            // It did, and the number was MEANINGLESS: `handleCreateChapter` shows a
            // CONFIRMATION MODAL (`alert.runModal()`, below) that blocks until the user
            // clicks. The logged 2959.5 / 2092.5 / 3638.6 ms were dominated by HUMAN
            // REACTION TIME — which is also why they varied so wildly, and why the
            // apparent "29% improvement" after the batching fixes was noise, not signal.
            //
            // ✅ The work is timed from AFTER the modal instead (see `__w0`), so the
            // number reports computation only.
            guard let tv = textView else { return }
            let loc = tv.selectedRange().location
            guard let segIdx = segmentIndex(for: loc) else { return }

            let loader = parent.loader
            let env = parent.env
            let session = parent.session

            let segRange = sceneBoundaries.indices.contains(segIdx) ? sceneBoundaries[segIdx] : NSRange(location: loc, length: 0)
            let splitOffsetInSeg = max(0, loc - segRange.location)
            let currentText = loader.segments.indices.contains(segIdx) ? loader.segments[segIdx].text : ""
            let isAtEnd = splitOffsetInSeg >= currentText.count

            // Count chapters that follow the current one — these will be renumbered.
            // If splitting at end there are no following scenes in the current chapter so
            // renumbering only affects chapters after the current one (if any).
            let currentChapterID = loader.segments.indices.contains(segIdx)
                ? loader.segments[segIdx].chapterID : ""
            let orderedChapterIDs: [String] = {
                var seen = Set<String>()
                return loader.allScenes.compactMap {
                    seen.insert($0.chapterID).inserted ? $0.chapterID : nil
                }
            }()
            let currentChapterOrdinal = (orderedChapterIDs.firstIndex(of: currentChapterID) ?? 0) + 1
            let chaptersAfter = orderedChapterIDs.count - currentChapterOrdinal

            // Show confirmation dialog when the split will renumber subsequent chapters.
            if chaptersAfter > 0 {
                let alert = NSAlert()
                alert.messageText = "Split into New Chapter?"
                alert.informativeText = "Splitting here will create a new chapter and renumber \(chaptersAfter) subsequent chapter\(chaptersAfter == 1 ? "" : "s"). This cannot be undone."
                alert.addButton(withTitle: "Split")
                alert.addButton(withTitle: "Cancel")
                alert.alertStyle = .warning
                guard alert.runModal() == .alertFirstButtonReturn else { return }
            }

            // ✅ I-0213 — the timer starts AFTER the modal, so it measures WORK, not the
            // user's click latency. Reported at the end of the async block below.
            let __w0 = Date()

            Task { @MainActor in
                guard let ref = env.authorshipRef,
                      let rootPath = session.projectRootPath,
                      let proj = session.openProjectResult,
                      loader.segments.indices.contains(segIdx)
                else { return }

                let currentSeg = loader.segments[segIdx]
                await loader.saveCurrentIfDirty(engine: env.engine, ref: ref)
                // Structural op — record a barrier so undo stops here (§4.5).
                session.historyCapture?.recordBarrier(kind: "chapterSplit", note: "Can't undo past creating a chapter")

                do {
                    // ⚠️ I-0213 — the C ABI call is SYNCHRONOUS ON THE MAIN ACTOR and was
                    // untimed. After batching the loader mutations, createChapter fell
                    // 2959.5 → 2092.5 ms — a real 29% gain, but 2.1 s remains unexplained.
                    // This splits the engine call out from the Swift-side work so the next
                    // run says WHICH it is, instead of another inference.
                    let __e0 = Date()
                    let result = try env.engine.createChapter(
                        projectRootPath: rootPath,
                        appSupportRoot: env.appSupportRoot,
                        projectID: proj.projectID,
                        authorshipRef: ref
                    )
                    let __eMs = Date().timeIntervalSince(__e0) * 1000
                    if __eMs > 0.5 {
                        NSLog("[SCRIVI-STRUCT]   engine.createChapter (C ABI)=%.1f ms", __eMs)
                    }

                    if isAtEnd {
                        // Append empty chapter after current — original behaviour.
                        let newIdx = loader.insertChapterFirstScene(result, after: segIdx)
                        loader.renumberChapterTitlesFrom(segmentIndex: newIdx)
                        loader.setCurrentIndex(newIdx)
                        insertDividerAndMoveCursor(after: segIdx, placeCursorAtStart: true)
                    } else {
                        // Split scene at cursor: head stays in current scene (current chapter),
                        // tail becomes first scene of new chapter.
                        let headText = splitHead(of: currentText, at: splitOffsetInSeg)
                        let tailText = splitTail(of: currentText, at: splitOffsetInSeg)

                        // Save head into current scene.
                        // ⛔ [I-0259] — success is CAPTURED: `splitScene` marks both halves
                        // clean ("saved by caller"), so a failed half is re-marked dirty below.
                        let headSaved = (try? env.engine.saveScene(
                            projectID: proj.projectID,
                            projectRootPath: rootPath,
                            appSupportRoot: env.appSupportRoot,
                            sceneID: currentSeg.sceneID,
                            sceneMetadataPath: currentSeg.metadataPath,
                            sceneContentPath: currentSeg.contentPath,
                            markdown: headText,
                            authorshipRef: ref
                        )) != nil
                        // Save tail into new chapter's first scene.
                        let tailSaved = (try? env.engine.saveScene(
                            projectID: proj.projectID,
                            projectRootPath: rootPath,
                            appSupportRoot: env.appSupportRoot,
                            sceneID: result.firstSceneID,
                            sceneMetadataPath: result.firstSceneMetadataPath,
                            sceneContentPath: result.firstSceneContentPath,
                            markdown: tailText,
                            authorshipRef: ref
                        )) != nil

                        // Capture old chapter ID before splitScene changes the segment.
                        let oldChapterID = loader.segments.indices.contains(segIdx)
                            ? loader.segments[segIdx].chapterID : ""

                        // Update in-memory state for the new chapter's first scene.
                        let chapterFirstResult = CreateSceneResult(
                            sceneID: result.firstSceneID,
                            chapterID: result.chapterID,
                            metadataPath: result.firstSceneMetadataPath,
                            contentPath: result.firstSceneContentPath
                        )
                        let newIdx = loader.splitScene(chapterFirstResult, at: segIdx,
                                                       headText: headText, tailText: tailText)
                        if !headSaved { loader.markDirty(at: segIdx) }   // [I-0259]
                        if !tailSaved { loader.markDirty(at: newIdx) }
                        // Re-assign subsequent scenes in the old chapter to the new chapter.
                        loader.splitChapter(result, movingFrom: newIdx, oldChapterID: oldChapterID)
                        // Fix chapter titles in-memory — engine wrote correct ordinals to disk.
                        loader.renumberChapterTitlesFrom(segmentIndex: newIdx)
                        loader.setCurrentIndex(newIdx)
                        insertDividerAndMoveCursor(after: segIdx, placeCursorAtStart: true)
                    }

                    // ⚠️ I-0213 — the other untimed block on this path. `reloadSceneDots`
                    // does one `getSceneStoryTime` C ABI call PER SCENE (1,174 of them),
                    // so it is a candidate for the remaining 2.1 s. Measured, not assumed.
                    // ⚠️ I-0213 — TIMED SEPARATELY. One timer spanning BOTH was misleading:
                    // it was labelled `reloadSceneDots` while most of the cost was actually
                    // in `updateDotTitles`, which `load()` never calls.
                    let __r0 = Date()
                    session.timelineModel?.reloadSceneDots(
                        engine: env.engine, projectRootPath: rootPath, scenes: loader.allScenes)
                    let __rMs = Date().timeIntervalSince(__r0) * 1000
                    let __t0t = Date()
                    session.timelineModel?.updateDotTitles(liveTitles: loader.liveTitles, allScenes: loader.allScenes)
                    let __tMs = Date().timeIntervalSince(__t0t) * 1000
                    if __rMs > 0.5 { NSLog("[SCRIVI-STRUCT]   reloadSceneDots=%.1f ms", __rMs) }
                    if __tMs > 0.5 { NSLog("[SCRIVI-STRUCT]   updateDotTitles=%.1f ms", __tMs) }

                    // ✅ WORK ONLY — excludes the confirmation modal (see `__w0`).
                    let __wMs = Date().timeIntervalSince(__w0) * 1000
                    if __wMs > 0.5 {
                        NSLog("[SCRIVI-STRUCT] createChapter WORK=%.1f ms (modal excluded)", __wMs)
                    }
                } catch {
                    print("[Scrivi] createChapter failed: \(error)")
                }
            }
        }

        // Cmd-Backspace: merge scene with previous scene (only if cursor at position 0 of scene,
        // and the scene is not the first scene in its chapter).
        func handleMergeScene() {
            // ⚠️ I-0213 — TIME THE STRUCTURAL OP ITSELF.
            // The ⌘-chord paths RETURN EARLY from `keyDown`, so its `defer` timer never
            // sees them. A 2.7 s hang was logged against a PLAIN Return and could not be
            // attributed to any action; this makes each op report its own cost.
            let __t0 = Date()
            defer {
                let ms = Date().timeIntervalSince(__t0) * 1000
                if ms > 0.5 { NSLog("[SCRIVI-STRUCT] mergeScene=%.1f ms", ms) }
            }
            guard let tv = textView else { return }
            let loc = tv.selectedRange().location
            guard let segIdx = segmentIndex(for: loc) else { return }
            guard segIdx > 0 else { return }

            let loader = parent.loader
            let env = parent.env
            let session = parent.session

            // Only fire if cursor is at the very start of this segment's content.
            let segRange = sceneBoundaries.indices.contains(segIdx) ? sceneBoundaries[segIdx] : nil
            guard let range = segRange, loc == range.location else { return }

            // Do nothing if this is the first scene in its chapter.
            guard loader.segments.indices.contains(segIdx),
                  loader.segments.indices.contains(segIdx - 1),
                  loader.segments[segIdx].chapterID == loader.segments[segIdx - 1].chapterID
            else { return }

            Task { @MainActor in
                guard let ref = env.authorshipRef,
                      let rootPath = session.projectRootPath,
                      let _ = session.openProjectResult,
                      loader.segments.indices.contains(segIdx),
                      loader.segments.indices.contains(segIdx - 1)
                else { return }

                let currentSeg  = loader.segments[segIdx]
                let predecessorSeg = loader.segments[segIdx - 1]
                // Flush BOTH scenes to disk first — the endpoint joins the on-disk bodies, so
                // any unsaved edits in either scene must be persisted before the merge. (In the
                // normal flow only the current scene can be dirty, but the predecessor may hold
                // edits too; save it explicitly.)
                await loader.saveCurrentIfDirty(engine: env.engine, ref: ref)
                await loader.saveScene(at: segIdx - 1, engine: env.engine, ref: ref)
                // Structural op — record a barrier so undo stops here (§4.5).
                session.historyCapture?.recordBarrier(kind: "sceneMerge", note: "Can't undo past a scene merge")

                // Atomic same-chapter merge in ScriviCore: the current scene joins into the
                // predecessor (survivor), which keeps its own files; the current scene's files
                // are removed and the chapter cache is rebuilt (EP-028 SP-075). Replaces the
                // former saveScene(joinText)+deleteScene composition.
                let joinPoint = predecessorSeg.text.count  // cursor lands at the seam
                do {
                    _ = try env.engine.mergeScene(projectRootPath: rootPath, sceneID: currentSeg.sceneID)
                } catch {
                    print("[Scrivi] mergeScene failed: \(error)")
                    return
                }

                // Mirror the on-disk join in memory. The endpoint joins with a blank line
                // (SceneMerger.joinBodies), elided when either side is empty — match it so the
                // in-memory text equals disk.
                let joinText = Self.joinedMergeBody(predecessorSeg.text, currentSeg.text)
                let mergedIdx = loader.mergeSceneIntoPredecessor(at: segIdx, joinText: joinText)
                loader.setCurrentIndex(mergedIdx)
                rebuildStorageAndPlaceCursor(at: mergedIdx, textOffset: joinPoint)

                session.timelineModel?.reloadSceneDots(
                    engine: env.engine, projectRootPath: rootPath, scenes: loader.allScenes)
                session.timelineModel?.updateDotTitles(liveTitles: loader.liveTitles, allScenes: loader.allScenes)
            }
        }

        // Join two scene bodies the way ScriviCore's SceneMerger.joinBodies does: a blank-line
        // separator, elided when either side is empty (so no stray leading/trailing blanks).
        static func joinedMergeBody(_ base: String, _ addition: String) -> String {
            if base.isEmpty { return addition }
            if addition.isEmpty { return base }
            return base + "\n\n" + addition
        }

        // Shift-Cmd-Backspace: merge chapter with previous chapter (only if cursor at position 0
        // of the first scene of a chapter, and not in the first chapter).
        func handleMergeChapter() {
            // ⚠️ I-0213 — TIME THE STRUCTURAL OP ITSELF.
            // The ⌘-chord paths RETURN EARLY from `keyDown`, so its `defer` timer never
            // sees them. A 2.7 s hang was logged against a PLAIN Return and could not be
            // attributed to any action; this makes each op report its own cost.
            let __t0 = Date()
            defer {
                let ms = Date().timeIntervalSince(__t0) * 1000
                if ms > 0.5 { NSLog("[SCRIVI-STRUCT] mergeChapter=%.1f ms", ms) }
            }
            guard let tv = textView else { return }
            let loc = tv.selectedRange().location
            guard let segIdx = segmentIndex(for: loc) else { return }
            guard segIdx > 0 else { return }

            let loader = parent.loader
            let env = parent.env
            let session = parent.session

            // Only fire if cursor is at the very start of the segment.
            let segRange = sceneBoundaries.indices.contains(segIdx) ? sceneBoundaries[segIdx] : nil
            guard let range = segRange, loc == range.location else { return }

            guard loader.segments.indices.contains(segIdx),
                  loader.segments.indices.contains(segIdx - 1)
            else { return }

            let currentChapterID    = loader.segments[segIdx].chapterID
            let predecessorChapterID = loader.segments[segIdx - 1].chapterID

            // Must be a chapter boundary.
            guard currentChapterID != predecessorChapterID else { return }

            // Must be at the first scene of its chapter.
            guard segIdx == loader.segments.firstIndex(where: { $0.chapterID == currentChapterID }) else { return }

            Task { @MainActor in
                guard let ref = env.authorshipRef,
                      let rootPath = session.projectRootPath,
                      let _ = session.openProjectResult,
                      loader.segments.indices.contains(segIdx),
                      loader.segments.indices.contains(segIdx - 1)
                else { return }

                await loader.saveCurrentIfDirty(engine: env.engine, ref: ref)
                // Structural op — record a barrier so undo stops here (§4.5).
                session.historyCapture?.recordBarrier(kind: "chapterMerge", note: "Can't undo past a chapter merge")

                // Find predecessor chapter's metadata path/title from allScenes.
                let predecessorMeta = loader.allScenes.first(where: { $0.chapterID == predecessorChapterID })?.chapterMetadataPath ?? ""
                let predecessorTitle = loader.allScenes.first(where: { $0.chapterID == predecessorChapterID })?.chapterTitle ?? ""

                // Atomic whole-chapter merge in ScriviCore (EP-028 SP-075): RELOCATES every
                // scene file of the current chapter into the predecessor's folder, then removes
                // the emptied chapter. Replaces the former in-memory-reassign + deleteChapter
                // composition that deleted the scene files on disk (I-0083).
                do {
                    _ = try env.engine.mergeChapter(projectRootPath: rootPath, chapterID: currentChapterID)
                } catch {
                    print("[Scrivi] mergeChapter failed: \(error)")
                    return
                }

                // Mirror the move in memory: re-assign the moved scenes to the predecessor chapter.
                loader.mergeChapterIntoPredecessor(
                    at: segIdx,
                    predecessorChapterID: predecessorChapterID,
                    predecessorChapterMetadataPath: predecessorMeta,
                    predecessorChapterTitle: predecessorTitle
                )
                // The relocation minted NEW order-key filenames for every moved scene, so their
                // in-memory metadataPath/contentPath are now stale (the I-0081 stale-path class).
                // Refresh all scene paths from a fresh openProject before any later rename/save.
                if let reopened = try? env.engine.openProject(projectRootPath: rootPath, appSupportRoot: env.appSupportRoot) {
                    loader.refreshScenePaths(from: reopened.scenes)
                }
                // Renumber all chapters from the predecessor onward — the merged chapter
                // shifts every subsequent chapter's ordinal down by one.
                let renumberFrom = loader.segments.firstIndex(where: { $0.chapterID == predecessorChapterID }) ?? segIdx
                loader.renumberChapterTitlesFrom(segmentIndex: renumberFrom)

                // Rebuild storage; cursor stays at segIdx (now in predecessor chapter).
                if let tv = textView {
                    rebuildStorage(tv, segments: loader.segments)
                    if sceneBoundaries.indices.contains(segIdx) {
                        revealCaretAfterRebuild(sceneBoundaries[segIdx].location, in: tv)
                    }
                }

                session.timelineModel?.reloadSceneDots(
                    engine: env.engine, projectRootPath: rootPath, scenes: loader.allScenes)
                session.timelineModel?.updateDotTitles(liveTitles: loader.liveTitles, allScenes: loader.allScenes)
            }
        }

        // After inserting a new segment, rebuild storage and position cursor.
        private func insertDividerAndMoveCursor(after segIdx: Int, placeCursorAtStart: Bool) {
            guard let tv = textView else { return }
            rebuildStorage(tv, segments: parent.loader.segments)
            let newSegIdx = segIdx + 1
            if sceneBoundaries.indices.contains(newSegIdx) {
                revealCaretAfterRebuild(sceneBoundaries[newSegIdx].location, in: tv)
            }
        }

        // Rebuild storage and place the cursor at `textOffset` characters into segment `segIdx`.
        private func rebuildStorageAndPlaceCursor(at segIdx: Int, textOffset: Int) {
            guard let tv = textView else { return }
            rebuildStorage(tv, segments: parent.loader.segments)
            if sceneBoundaries.indices.contains(segIdx) {
                let loc = sceneBoundaries[segIdx].location + textOffset
                revealCaretAfterRebuild(min(loc, tv.string.count), in: tv)
            }
        }

        // ⚠️ [I-0271] Place the caret after a FULL storage rebuild and make the viewport show
        // it. ⛔ The user saw the top of the viewport stay BLANK after create/merge until the
        // next scroll or keystroke — with the caret dropped at the BOTTOM edge, because
        // `scrollRangeToVisible` scrolls the minimum distance. ✅ Present on `dbc158f`, so it
        // predates SP-151's work (A/B by the user, 2026-10-02). ✅ TextKit 2 lays out only the
        // viewport; after the rebuild + scroll, nothing asked it to lay that viewport out again
        // until the next interaction did. `layoutViewport()` asks now (AppKit,
        // `NSTextViewportLayoutController.layoutViewport()`, macOS — doc fetched 2026-10-02).
        // ⚠️ Every rebuild-then-place-caret path goes through here so the fix is not partial.
        //
        // ✅ T-0573 (user request 2026-10-02): the caret STAYS AT THE SAME HEIGHT in the
        // viewport across create/merge — *"I find it jarring when the cursor that was in the
        // middle of the viewport is suddenly at the bottom."* `rebuildStorage` measures the
        // caret's offset below the viewport top BEFORE it rebuilds.
        // ⛔ FIRST ATTEMPT FAILED (user, same day: it scrolled to "somewhere around Chapter
        // 44"): it scrolled to the caret's ABSOLUTE y right after the rebuild. ✅ MEASURED in a
        // standalone TextKit 2 harness (~1.8 M chars): after a full rebuild most of the document
        // is not laid out, so y is an ESTIMATE — and the whole coordinate space shifts (the same
        // text moved ~16,000 pt). ✅ So: let AppKit scroll to the caret (it resolves this
        // correctly), lay the viewport out, THEN nudge by the RELATIVE difference between where
        // the caret now sits and where it sat — measured only inside laid-out text. ✅ Harness:
        // one nudge lands exactly on the requested offset, and the text drawn there is the
        // caret's line. ⚠️ No measurement (caret was off screen) → `scrollRangeToVisible` only.
        private func revealCaretAfterRebuild(_ loc: Int, in tv: NSTextView) {
            let caret = NSRange(location: loc, length: 0)
            tv.setSelectedRange(caret)
            tv.scrollRangeToVisible(caret)
            tv.textLayoutManager?.textViewportLayoutController.layoutViewport()
            if let offset = caretViewportOffset,
               let scroll = tv.enclosingScrollView,
               let rect = boundingRect(forCharacterIndex: loc, in: tv) {
                let clip = scroll.contentView
                let delta = (rect.minY - clip.bounds.minY) - offset
                let maxY = max(0, tv.bounds.height - clip.bounds.height)
                clip.scroll(to: NSPoint(x: clip.bounds.minX, y: max(0, min(clip.bounds.minY + delta, maxY))))
                scroll.reflectScrolledClipView(clip)
                tv.textLayoutManager?.textViewportLayoutController.layoutViewport()
            }
            caretViewportOffset = nil
        }

        // ✅ [I-0282] (SP-168 live pass, user: *"when changing font or font size, the position of the caret is not maintained
        // … my manuscript position will shift from say, 53 to say, 35"*). A typeface or size change rebuilds the storage
        // with the SAME text but different line heights, so the scroll's PIXEL offset lands on different text. ✅ Anchor on a
        // CHARACTER instead: the caret's line when it is on screen, else the character at the top of the visible area —
        // and put that character's line back at the same height. The selection is kept.
        /// ✅ [I-0282] THE way a typeface or size change is applied: read the writer's place FIRST, then change the type,
        /// rebuild, and restore. ⛔ LIVE RE-CHECK 3 (log `caretLineY=nan`): `updateNSView` used to set `tv.font` BEFORE the
        /// place was read — an `NSTextView` font assignment re-fonts the WHOLE document and invalidates every line position,
        /// so the place was read from garbage. (It also overwrote the chapter titles' font, which the rebuild then repaired.)
        /// ⛔ No `tv.font` assignment here: the rebuild writes every attribute; typing uses `typingAttributes`.
        func applyTypography(_ t: ManuscriptTypography, to tv: NSTextView, segments: [SceneSegment]) {
            rebuildKeepingReadingPosition(tv, segments: segments) {
                self.presenter.typography = t
                tv.typingAttributes = t.bodyAttributes
            }
        }

        func rebuildKeepingReadingPosition(_ tv: NSTextView, segments: [SceneSegment], change: () -> Void = {}) {
            let selection = tv.selectedRanges
            let anchor = readingAnchor(tv)
            change()                       // only AFTER the place has been read
            rebuildStorage(tv, segments: segments)
            let length = tv.textStorage?.length ?? 0
            tv.selectedRanges = selection.map { v in
                let r = v.rangeValue
                let loc = min(r.location, length)
                return NSValue(range: NSRange(location: loc, length: min(r.length, length - loc)))
            }
            guard let anchor, let scroll = tv.enclosingScrollView else { return }
            let clip = scroll.contentView
            let target = min(anchor.index, length)
            // ⛔ LIVE RE-CHECK (user, 2026-10-07): computing a pixel target and clamping it to `tv.bounds.height` sent the view
            // to Chapter 25 from Chapter 51 — right after a rebuild the text view's FRAME still holds TextKit 2's provisional
            // height, so the clamp landed mid-document. ✅ The pattern that already works in the app (T-0573,
            // `revealCaretAfterRebuild`): bring the anchor on screen with `scrollRangeToVisible` FIRST — AppKit sizes the
            // document as it goes — then correct by the small remaining delta.
            tv.scrollRangeToVisible(NSRange(location: target, length: 0))
            // ⚠️ Measured: straight after `scrollRangeToVisible` the first `layoutViewport` has not yet brought the anchor into
            // the DRAWN range — one more layout does. So a not-yet-drawn anchor means "lay out again", not "give up".
            for _ in 0..<10 {
                tv.textLayoutManager?.textViewportLayoutController.layoutViewport()
                guard let line = lineRect(forCharacterIndex: target, in: tv) else { continue }   // not drawn yet: lay out again
                let delta = (line.minY - clip.bounds.minY) - anchor.offset
                if abs(delta) < 0.5 { break }
                let maxY = max(0, (scroll.documentView?.frame.height ?? tv.bounds.height) - clip.bounds.height)
                clip.scroll(to: NSPoint(x: clip.bounds.minX, y: max(0, min(clip.bounds.minY + delta, maxY))))
                scroll.reflectScrolledClipView(clip)
            }
            tv.textLayoutManager?.textViewportLayoutController.layoutViewport()
        }

        /// The character the writer is "at", and its line's distance below the top of the visible area.
        /// ⛔ [I-0282] live re-check 2 (log): `caret=1846568 caretLineY=624533.0 clipY=591857.6` — the caret WAS on screen, but
        /// `ensureLayout` put its line at ~1.2 M pt (past the document's end) while the viewport draws Chapter 51 at ~0.6 M:
        /// TextKit 2 places the VIEWPORT's paragraphs on ESTIMATED heights above them, and `ensureLayout` computes TRUE
        /// positions — two frames that disagree. `characterIndexForInsertion` mis-read the top of the view the same way
        /// (index 898922, Chapter 25). ✅ Read ONLY what the viewport has laid out: the caret is on screen iff it is in the
        /// viewport's range, and the top of the view is the first DRAWN line at or below the visible top.
        func readingAnchor(_ tv: NSTextView) -> (index: Int, offset: CGFloat)? {
            guard let clip = tv.enclosingScrollView?.contentView else { return nil }
            tv.textLayoutManager?.textViewportLayoutController.layoutViewport()
            let caret = tv.selectedRange().location
            if let r = lineRect(forCharacterIndex: caret, in: tv), (0...clip.bounds.height).contains(r.minY - clip.bounds.minY) {
                return (caret, r.minY - clip.bounds.minY)
            }
            guard let top = firstVisibleLine(in: tv) else { return nil }
            return (top.index, top.rect.minY - clip.bounds.minY)
        }

        /// The first line the viewport has DRAWN whose top is at or below the visible top — its character and rect.
        func firstVisibleLine(in tv: NSTextView) -> (index: Int, rect: NSRect)? {
            guard let lm = tv.textLayoutManager, let content = lm.textContentManager,
                  let clip = tv.enclosingScrollView?.contentView,
                  let viewport = lm.textViewportLayoutController.viewportRange else { return nil }
            let inset = tv.textContainerInset
            var found: (Int, NSRect)?
            lm.enumerateTextLayoutFragments(from: viewport.location, options: []) { fragment in
                let frame = fragment.layoutFragmentFrame
                for line in fragment.textLineFragments {
                    let b = line.typographicBounds
                    let rect = NSRect(x: frame.minX + b.minX + inset.width, y: frame.minY + b.minY + inset.height,
                                      width: b.width, height: b.height)
                    if rect.minY >= clip.bounds.minY - 0.5 {
                        let start = content.offset(from: content.documentRange.location, to: fragment.rangeInElement.location)
                        found = (start + line.characterRange.location, rect)
                        return false
                    }
                }
                return fragment.rangeInElement.endLocation.compare(viewport.endLocation) == .orderedAscending
            }
            return found
        }

        /// The LINE holding character `index`, in the text view's coordinates — ⚠️ ONLY if the viewport has laid it out
        /// (nil otherwise). Never `ensureLayout`: see `readingAnchor` for why its positions disagree with what is drawn.
        func lineRect(forCharacterIndex index: Int, in tv: NSTextView) -> NSRect? {
            guard let lm = tv.textLayoutManager, let content = lm.textContentManager,
                  let viewport = lm.textViewportLayoutController.viewportRange,
                  let loc = content.location(content.documentRange.location, offsetBy: index) else { return nil }
            let inViewport = viewport.contains(loc) || loc.compare(viewport.endLocation) == .orderedSame
            guard inViewport, let fragment = lm.textLayoutFragment(for: loc) else { return nil }
            let frame = fragment.layoutFragmentFrame
            let within = content.offset(from: fragment.rangeInElement.location, to: loc)
            let line = fragment.textLineFragments.first { NSLocationInRange(within, $0.characterRange) }
                ?? fragment.textLineFragments.last
            let bounds = line?.typographicBounds ?? CGRect(x: 0, y: 0, width: frame.width, height: frame.height)
            return NSRect(x: frame.minX + bounds.minX + tv.textContainerInset.width,
                          y: frame.minY + bounds.minY + tv.textContainerInset.height,
                          width: bounds.width, height: bounds.height)
        }

        // T-0573 — the caret's distance below the top of the visible area, or nil when the
        // caret is not on screen (then there is no position to preserve).
        private var caretViewportOffset: CGFloat?
        private func caretOffsetInViewport(_ tv: NSTextView) -> CGFloat? {
            guard let clip = tv.enclosingScrollView?.contentView,
                  let rect = boundingRect(forCharacterIndex: tv.selectedRange().location, in: tv)
            else { return nil }
            let offset = rect.minY - clip.bounds.minY
            return (0...clip.bounds.height).contains(offset) ? offset : nil
        }

        // Return the substring of `text` before `offset`.
        private func splitHead(of text: String, at offset: Int) -> String {
            guard offset > 0 else { return "" }
            let idx = text.index(text.startIndex, offsetBy: min(offset, text.count))
            return String(text[..<idx])
        }

        // Return the substring of `text` from `offset` onward.
        private func splitTail(of text: String, at offset: Int) -> String {
            guard offset < text.count else { return "" }
            let idx = text.index(text.startIndex, offsetBy: offset)
            return String(text[idx...])
        }

        // MARK: — Helpers

        // Recompute sceneBoundaries by scanning the live text storage.
        //
        // Layout of storage when chapter titles are shown:
        //   [heading\n] scene0text [attachment\n] [\nheading\n] scene1text [attachment\n] ...
        //
        // A segment's content starts AFTER any leading scriviHeading-attributed characters,
        // not immediately after the divider. recomputeBoundaries must skip heading runs
        // the same way rebuildStorage does, otherwise boundaries point into heading text
        // and textDidChange extracts heading characters into seg.text.
        // MARK: — TextKit 2 geometry (EP-039 SP-133, T-0525)
        //
        // ⚠️ **NEVER TOUCH `tv.layoutManager` FROM ANYWHERE IN THIS FILE.**
        //
        // ⚠️ `NSTextView` starts on TEXTKIT 2, which lays out only the VIEWPORT. Reading the
        // TextKit-1 `.layoutManager` property makes AppKit PERMANENTLY DOWNGRADE the view to
        // TextKit 1, which lays out the WHOLE DOCUMENT. ✅ VERIFIED EMPIRICALLY: a fresh
        // `NSTextView` reports `textLayoutManager != nil`; after one `_ = tv.layoutManager`
        // it reports `nil`.
        //
        // ✅ MEASURED ON THE REAL MANUSCRIPT (1,831,770 chars, 14,985 paragraphs):
        //   initial layout   TextKit 1 = 272.8 ms   →   TextKit 2 = 0.9 ms
        //   window resize    TextKit 1 = 222.5 ms   →   viewport-bounded
        //   rebuildStorage   TextKit 1 = 270.0 ms   →   viewport-bounded
        //
        // ⚠️ These two helpers are the ONLY places that convert between a character offset
        // and screen geometry. ⚠️ Two lines in this file were the entire downgrade; keeping
        // the conversion in one pair is what stops a third appearing.
        //
        // ⚠️ TEXTKIT 2'S LAZINESS IS PER PARAGRAPH: a manuscript that is ONE enormous
        // paragraph collapses to TextKit 1 numbers (measured 201 ms). ✅ Real prose is ~15k
        // paragraphs, so this is a shape risk, not a normal one (T-0530 measures it).

        /// Character index at `point` in `tv`'s coordinate space, or nil.
        /// ⚠️ Returns nil rather than guessing — the caller must not treat that as index 0.
        func characterIndex(atPoint point: NSPoint, in tv: NSTextView) -> Int? {
            guard let lm = tv.textLayoutManager,
                  let content = lm.textContentManager else { return nil }
            // ⚠️ The text view insets its container; fragment coordinates are container-relative.
            let p = NSPoint(x: point.x - tv.textContainerInset.width,
                            y: point.y - tv.textContainerInset.height)
            // ⛔ [I-0273] Lay out the point FIRST. It used to look up a fragment only among
            // those ALREADY laid out and, finding none, report END OF DOCUMENT — so a not-yet-
            // laid-out centre read as the last scene. That fed `viewportSceneID`, and the quit
            // stamp saves `viewportSceneID`: the log showed viewport = an END scene at
            // scroll 0.0 (the TOP).
            lm.ensureLayout(for: CGRect(x: p.x, y: p.y, width: 1, height: 1))
            guard let fragment = lm.textLayoutFragment(for: p) else {
                // ⚠️ Truly beyond the text — clamp to the nearer end, rather than failing, so
                // a scroll to the bottom still resolves a scene.
                return p.y <= 0 ? 0 : (tv.string as NSString).length
            }
            let loc = fragment.rangeInElement.location
            return content.offset(from: content.documentRange.location, to: loc)
        }

        /// Bounding rect (in `tv`'s coordinate space) of the line containing `charIndex`.
        func boundingRect(forCharacterIndex charIndex: Int, in tv: NSTextView) -> NSRect? {
            guard let lm = tv.textLayoutManager,
                  let content = lm.textContentManager,
                  let loc = content.location(content.documentRange.location, offsetBy: charIndex)
            else { return nil }
            // ⚠️ ensureLayout is REQUIRED: the target may be outside the laid-out viewport,
            // and TextKit 2 will not have positioned it yet. ✅ It lays out only up to the
            // requested range, not the document.
            lm.ensureLayout(for: NSTextRange(location: loc))
            guard let fragment = lm.textLayoutFragment(for: loc) else { return nil }
            let f = fragment.layoutFragmentFrame
            return NSRect(x: f.origin.x + tv.textContainerInset.width,
                          y: f.origin.y + tv.textContainerInset.height,
                          width: f.width, height: f.height)
        }

        // Recomputes each scene's range in the text storage, from the storage itself.
        //
        // ⚠️ **THIS WAS THE INTERACTION FREEZE.** [I-0200] / EP-039.
        //
        // ⚠️ The previous implementation walked the storage ONE CHARACTER AT A TIME —
        // `storage.attribute(.attachment, at: pos, …)` with `pos += 1` in the common
        // branch. ⚠️ On the measured 1,153-scene manuscript that is **1,823,706 attribute
        // lookups PER CALL**, and it is called from NINE sites including the navigator
        // click path and the scroll handler.
        // ✅ MEASURED SYMPTOM: AppKit logged
        // `NSTableView.doubleTapGestureRecognizer has been in possible phase for
        // 48.06 seconds` — the table blocked while this ran on the main thread.
        //
        // ✅ NOW: `enumerateAttribute` jumps between attribute RUNS, so the cost is
        // proportional to the number of SCENES (~1,153 dividers), not to the number of
        // CHARACTERS. ⚠️ Same output, same authority — the storage is still the single
        // source of truth for boundaries (I-0131), this only stops re-deriving it the
        // slowest possible way.
        /// ✅ EP-045 AC11: makes `sceneBoundaries` trustworthy — a no-op unless an edit the
        /// table could not prove local marked it dirty, in which case it rescans the storage.
        /// ⛔ It used to RESCAN the whole manuscript on every call, keystrokes included.
        func ensureBoundaries(_ tv: NSTextView) {
            guard let storage = tv.textStorage else { return }
            _ = boundaryTable.ranges(in: storage)
        }

        // Resume the writing surface restored from the last session (I-0058).
        // Places the cursor at the restored scene's storage offset + scene-local
        // selection offset, and scrolls it into view. No-op when nothing was restored
        // (fresh open at scene 0 — the existing scroll observer handles highlight).
        func restoreWritingSurface(in tv: NSTextView) {
            let loader = parent.loader
            guard let sceneID = loader.viewportSceneID else {
                NSLog("[SCRIVI-DIAG] restoreWritingSurface BAILED: viewportSceneID is nil")
                return
            }
            // ⚠️ **I-0131 root cause: two sources of truth for the same coordinate.**
            //
            // This used `loader.storageOffset(forSceneID:)`, which reads
            // `sceneStorageOffsetMap` — built by `rebuildSceneStartMap()` from scene
            // text plus 2 characters per separator, and **with no knowledge of chapter
            // headings**. `rebuildStorage` inserts heading text into the same storage
            // when "Show Chapter Titles" is on, and `recomputeBoundaries` skips over it
            // (see `skipHeading`). So the map is short by the total heading length of
            // every chapter before the target, and the caret landed roughly one scene
            // early — which then made the delegate report a different scene and the
            // scroll observer drift further still.
            //
            // Boundaries are computed from the REAL storage, so they are authoritative.
            // Recompute FIRST, then resolve the offset from them.
            ensureBoundaries(tv)

            guard let segIdx = loader.segments.firstIndex(where: { $0.sceneID == sceneID }) else {
                NSLog("[SCRIVI-DIAG] restoreWritingSurface BAILED: scene not loaded \(sceneID)")
                return
            }
            // ⚠️ Fall back to the map when boundaries are unavailable. `recomputeBoundaries`
            // returns early on EMPTY storage (`fullLen > 0` guard), and **a brand-new
            // project is exactly that** — ProjectCreator writes one empty scene .md. With
            // no text there is also no heading run, so the map and the boundaries agree
            // trivially; the fallback is correct, not a guess.
            //
            // The same holds with "Show Chapter Titles" OFF: no character carries
            // `.scriviHeading`, `skipHeading` is a no-op, and boundaries == the map's
            // arithmetic. The boundary path is preferred only because it is authoritative
            // when headings ARE present.
            let sceneStorageStart: Int
            if sceneBoundaries.indices.contains(segIdx) {
                sceneStorageStart = sceneBoundaries[segIdx].location
            } else if let mapped = loader.storageOffset(forSceneID: sceneID) {
                sceneStorageStart = mapped
            } else {
                NSLog("[SCRIVI-DIAG] restoreWritingSurface BAILED: no offset for \(sceneID)")
                return
            }
            NSLog("[SCRIVI-DIAG] restoreWritingSurface: scene=\(sceneID) idx=\(segIdx) boundaryStart=\(sceneStorageStart) mapSaid=\(loader.storageOffset(forSceneID: sceneID) ?? -1) tvLen=\(tv.string.count)")

            // Clamp the scene-local offset to the scene's text length so a shrunken
            // scene (edited externally) can't place the cursor past its bounds.
            let localOffset = loader.restoredSelectionOffset ?? 0
            let sceneLen = loader.segments.first(where: { $0.sceneID == sceneID })?.text.count ?? 0
            let clampedLocal = min(max(0, localOffset), sceneLen)
            let target = sceneStorageStart + clampedLocal

            // ⚠️ **The scroll this causes must not retarget the current scene (I-0131).**
            //
            // `placeCursorAt` calls `scrollRangeToVisible`, whose notification wakes the
            // debounced scroll handler; that handler recomputes the viewport from the
            // CENTRE of the visible rect. `scrollRangeToVisible` scrolls *minimally*, so
            // the restored scene sits at the viewport EDGE while the centre still shows
            // an earlier scene — and the navigator highlight follows `viewportSceneID`
            // (`SceneNavigatorView:200`), so the writer sees the wrong scene selected AND
            // the wrong text. Deterministic, which is why it landed on the same wrong
            // scene every run.
            //
            // Centre the restored scene rather than merely revealing it, so the viewport
            // the writer sees and the scene we restored are the same thing (§1 of the
            // Current Scene Model), then hold the scene against the resulting scroll.
            navigationLockUntil = Date().addingTimeInterval(0.5)
            placeCursorAt(target, in: tv)
            // ⛔ [I-0273] MEASURED on the user's relaunch 2026-10-02: `viewH=0` — the restore
            // runs BEFORE the window has given the scroll view any height, so centring here
            // scrolled nothing (`clipY=0`): caret at the end, viewport at the top. ✅ Centre
            // now if there is a height; otherwise hold the target for `clipFrameDidChange`.
            pendingRestoreCentre = (target, sceneID)
            restoreCentreUntil = nil
            if let clip = tv.enclosingScrollView?.contentView, clip.bounds.height > 0 {
                applyPendingRestoreCentre(in: tv)
            }
            loader.setViewportScene(sceneID)


            // Consume the one-shot restore state so it can't reapply on a later rebuild.
            //
            // ⚠️ There is deliberately no scroll-fraction restore here (I-0133, ruled
            // 2026-08-18). Restore *centres* the restored scene, and reapplying a
            // document-wide fraction from the previous session would fight that — the
            // exact edge-vs-centre disagreement I-0131 exists to eliminate. The Apple
            // layer therefore ignores `restored.scroll` by design, not by omission.
            // ⚠️ The schema field is NOT dead: `[Linux]` consumes it (T-0247,
            // `EditorShell.cpp`), so it must stay in `scrivi.h` and the open envelope.
            loader.restoredSelectionOffset = nil
        }

        // Navigate to a scene by ID — places the caret at the scene's first character,
        // scrolls it into view, and (via the navigator's `takeFocus`) hands over the
        // keyboard. §3 of the Current Scene Model.
        //
        // ⚠️ **I-0131: the scroll this triggers must not be allowed to overwrite the
        // destination.** `scrollRangeToVisible` emits scroll notifications, and the
        // handler for those recomputes the viewport scene from the CENTER of the
        // visible rect after a 120 ms debounce. Cancelling `scrollTask` here is not
        // enough — the cancelled task is the *old* one; the scroll we are about to
        // cause schedules a NEW one that lands 120 ms later and wins.
        //
        // It also lands on the wrong scene: `scrollRangeToVisible` scrolls
        // *minimally*, so the target sits at the edge of the viewport while its centre
        // still shows earlier scenes. That is why navigating to scene 15 resumed at
        // 10, and 22 resumed at 14 — the gap is however many scenes fit on screen.
        //
        // `navigationLockUntil` makes an explicit navigation authoritative for a short
        // window, so the scroll it causes cannot retarget it. Scrolling by hand after
        // that window behaves exactly as before.
        func navigateToScene(_ sceneID: String, in tv: NSTextView) {
            // ⚠️ Same defect as restoreWritingSurface (I-0131): `storageOffset(forSceneID:)`
            // ignores chapter-heading text, so it scrolls short by the heading length of
            // every chapter before the target. Use the boundaries computed from real
            // storage instead.
            ensureBoundaries(tv)
            guard let segIdx = parent.loader.segments.firstIndex(where: { $0.sceneID == sceneID }) else { return }
            // Same fallback as restoreWritingSurface: boundaries are authoritative when
            // they exist, but empty storage (a new project) leaves them unbuilt.
            let sceneStart: Int
            if sceneBoundaries.indices.contains(segIdx) {
                sceneStart = sceneBoundaries[segIdx].location
            } else if let mapped = parent.loader.storageOffset(forSceneID: sceneID) {
                sceneStart = mapped
            } else { return }
            let storageOffset = searchMatchOffset(sceneID: sceneID, segIdx: segIdx, in: tv) ?? sceneStart
            NSLog("[SCRIVI-DIAG] navigateToScene -> \(sceneID) boundaryStart=\(sceneStart) caret=\(storageOffset) mapSaid=\(parent.loader.storageOffset(forSceneID: sceneID) ?? -1)")
            scrollTask?.cancel()
            highlightTask?.cancel()
            navigationLockUntil = Date().addingTimeInterval(0.5)
            // §3 of the Current Scene Model (ruled 2026-08-17): a navigator click SELECTS
            // the scene — the caret moves to its first character, the manuscript scrolls
            // there, and focus transfers to the manuscript. All three, or the writer
            // cannot simply start typing.
            //
            // ⚠️ This deliberately **reverses SP-063's scroll-without-caret rule for
            // navigator clicks specifically**; SP-063 still governs click-to-place *within*
            // the manuscript, which is unchanged. An earlier comment here asserted the
            // opposite ("navigateToScene deliberately does not move the caret") and was
            // simply wrong against the ruling — the caret stayed wherever it had last
            // been, which was invisible only because focus used to stay in the navigator.
            // Once focus started transferring, the stale caret became visible immediately.
            let __n0 = Date()
            tv.setSelectedRange(NSRange(location: storageOffset, length: 0))
            let __nSel = Date()
            // Centre rather than merely reveal, for the same reason as restore: a scene
            // parked at the viewport edge makes the scroll handler read a DIFFERENT
            // scene at the centre, and the navigator highlight follows that.
            tv.scrollRangeToVisible(NSRange(location: storageOffset, length: 0))
            let __nScroll = Date()
            centerStorageOffset(storageOffset, in: tv)
            let __nCenter = Date()
            parent.loader.setViewportScene(sceneID)
            NSLog(String(format:
                "[SCRIVI-NAV] setSel=%.1f  scrollToVisible=%.1f  center=%.1f  setViewport=%.1f ms",
                __nSel.timeIntervalSince(__n0) * 1000,
                __nScroll.timeIntervalSince(__nSel) * 1000,
                __nCenter.timeIntervalSince(__nScroll) * 1000,
                Date().timeIntervalSince(__nCenter) * 1000))
            // Keep the caret-derived current scene in step with the click, so a later quit
            // resumes here (I-0131's stamp reads cursorSceneID). `setCurrentIndex` moves
            // `currentIndex` and `cursorSceneID` together — setting either alone is what
            // let the two disagree in the first place (§1: there is ONE current scene).
            parent.loader.setCurrentIndex(segIdx)
        }

        // T-0571 — where a navigator click made WHILE SEARCHING puts the caret: the query's
        // FIRST match inside the scene's own text (ruled 2026-10-01: caret only, no selection;
        // first match only). nil → the scene start, as for any other navigation — including a
        // scene listed only because its CHAPTER title matched (ruled the same day).
        //
        // ✅ Same semantics as the navigator's `localizedStandardContains`: case- and
        // diacritic-insensitive, current locale — so the scene the list showed is the scene
        // this finds a match in.
        // ⚠️ The hint is NOT consumed here: one click drives `navigateToScene` two or three
        // times (see the log of 2026-10-01), and consuming it on the first call would send
        // the later ones back to the scene start. It expires after the same window as
        // `navigationLockUntil`, so a LATER navigation to this scene from elsewhere (timeline,
        // Detail Sheet) lands at the scene start as before.
        private func searchMatchOffset(sceneID: String, segIdx: Int, in tv: NSTextView) -> Int? {
            guard let hint = parent.loader.searchCaretHint, hint.sceneID == sceneID,
                  sceneBoundaries.indices.contains(segIdx) else { return nil }
            let loader = parent.loader
            DispatchQueue.main.asyncAfter(deadline: .now() + 0.5) {
                if loader.searchCaretHint == hint { loader.searchCaretHint = nil }
            }
            let ns = tv.string as NSString
            let scene = NSIntersectionRange(sceneBoundaries[segIdx], NSRange(location: 0, length: ns.length))
            // ✅ [SP-164] Q5: match what the writer SEES — a query with punctuation (`Mr. Smith`, stored `Mr\. Smith`) or one
            // across a hidden marker (`bold here`, stored `**bold** here`) used to find nothing here.
            guard let storage = tv.textStorage else { return nil }
            return PresentedText.build(storage, presenter: presenter, range: scene).firstMatch(of: hint.query)?.location
        }

        // Scroll so `storageOffset` sits near the VERTICAL CENTRE of the viewport.
        //
        // `scrollRangeToVisible` only guarantees visibility, which puts a target at the
        // edge — and the scroll handler then reads the centre and concludes a different
        // scene is current. Centring makes the two agree.
        //
        // ⛔ [I-0273] It used to scroll to the target's ABSOLUTE y. After a full rebuild (a
        // project OPEN, i.e. the restore) TextKit 2 has laid out almost nothing, so that y is
        // an ESTIMATE — measured for T-0573 (2026-10-02) — and the restore landed ~12
        // chapters away (the log's first viewport scene after restoring index 1180 was in
        // chapter 46 of 58). ✅ Same cure as T-0573: let AppKit reveal the target (it resolves
        // the estimate), lay the viewport out, then nudge by the RELATIVE distance from the
        // viewport centre, measured inside laid-out text. ⚠️ Also: the clamp used
        // `tv.string.count` (Characters) against UTF-16 offsets.
        func centerStorageOffset(_ storageOffset: Int, in tv: NSTextView) {
            guard let scroll = tv.enclosingScrollView else { return }
            let clip = scroll.contentView
            let loc = min(storageOffset, (tv.string as NSString).length)
            tv.scrollRangeToVisible(NSRange(location: loc, length: 0))
            tv.textLayoutManager?.textViewportLayoutController.layoutViewport()
            guard let rect = self.boundingRect(forCharacterIndex: loc, in: tv) else { return }
            let delta = rect.midY - (clip.bounds.minY + clip.bounds.height / 2)
            let maxY = max(0, tv.bounds.height - clip.bounds.height)
            clip.scroll(to: NSPoint(x: clip.bounds.minX, y: max(0, min(clip.bounds.minY + delta, maxY))))
            scroll.reflectScrolledClipView(clip)
            tv.textLayoutManager?.textViewportLayoutController.layoutViewport()
        }

        // MARK: — Scene / Chapter boundary navigation
        //
        // Move the caret to the start or end of the scene or chapter the caret is
        // currently in. Unlike `navigateToScene` (a survey gesture that scrolls without
        // moving the caret), these are *editing* moves: they place the insertion point,
        // because that is what "go to the start of this scene" means while writing.
        //
        // ⚠️ **No key equivalents — deliberately, pending a ruling (2026-08-18).** Every
        // conventional candidate is already owned:
        //   • ⌘↑/⌘↓  — document start/end (NSTextView)
        //   • ⌥↑/⌥↓  — paragraph start/end (NSTextView)
        //   • ⌃↑/⌃↓  — Mission Control (macOS, not ours to take)
        // …and Shift with any of them extends the selection, so rebinding one would cost
        // an existing text-editing behaviour. The menu items exist so the *functions* can
        // be exercised and judged while the binding question stays open.

        // Storage range of the scene containing `storageOffset`, boundaries recomputed.
        private func sceneStorageRange(containing storageOffset: Int, in tv: NSTextView) -> NSRange? {
            ensureBoundaries(tv)
            return sceneBoundaries.first { NSLocationInRange(storageOffset, $0)
                || storageOffset == $0.location + $0.length }
                ?? sceneBoundaries.last
        }

        func moveToSceneBoundary(_ edge: ManuscriptEdge, in tv: NSTextView) {
            let caret = tv.selectedRange().location
            guard let range = sceneStorageRange(containing: caret, in: tv) else { return }
            placeCursorAt(edge == .start ? range.location : range.location + range.length, in: tv)
        }

        func moveToChapterBoundary(_ edge: ManuscriptEdge, in tv: NSTextView) {
            let caret = tv.selectedRange().location
            ensureBoundaries(tv)
            // Find the caret's scene, then widen to every contiguous scene sharing its
            // chapter. Scene order in `segments` is manuscript order, so the chapter is a
            // contiguous run — the same assumption the Linux chapter-reorder splice makes.
            guard let segIdx = sceneBoundaries.firstIndex(where: {
                NSLocationInRange(caret, $0) || caret == $0.location + $0.length
            }) ?? (sceneBoundaries.isEmpty ? nil : sceneBoundaries.count - 1) else { return }
            guard parent.loader.segments.indices.contains(segIdx) else { return }
            let chapterID = parent.loader.segments[segIdx].chapterID

            var first = segIdx, last = segIdx
            while first > 0, parent.loader.segments[first - 1].chapterID == chapterID { first -= 1 }
            while last < parent.loader.segments.count - 1,
                  parent.loader.segments[last + 1].chapterID == chapterID { last += 1 }

            guard sceneBoundaries.indices.contains(first),
                  sceneBoundaries.indices.contains(last) else { return }
            let target = edge == .start
                ? sceneBoundaries[first].location
                : sceneBoundaries[last].location + sceneBoundaries[last].length
            placeCursorAt(target, in: tv)
        }

        // ⚠️ [T-0568] — the manuscript's first or last character.
        // ✅ The end is measured in UTF-16 (`NSString.length`), the unit `NSRange` uses —
        // ⛔ not `String.count`, which counts Characters and falls SHORT of the true end
        // whenever the text holds an emoji or a combining mark.
        func moveToManuscriptBoundary(_ edge: ManuscriptEdge, in tv: NSTextView) {
            // ⚠️ [I-0266] The start is the FIRST SCENE's first character, not offset 0 —
            // with chapter titles shown, offset 0 is in front of Chapter 1's heading.
            let target: Int
            if edge == .start {
                ensureBoundaries(tv)
                target = sceneBoundaries.first?.location ?? 0
            } else {
                target = (tv.string as NSString).length
            }
            placeCursorAt(target, in: tv)
        }

        // Place cursor at a given NSTextStorage offset and take focus.
        func placeCursorAt(_ storageOffset: Int, in tv: NSTextView) {
            let loc = NSRange(location: min(storageOffset, tv.string.count), length: 0)
            tv.setSelectedRange(loc)
            tv.scrollRangeToVisible(loc)
            takeFocus()
        }

        // Translate an NSTextStorage offset to a manuscript position
        // by subtracting the 2-char separator pairs (attachment + \n) before it.
        func storageOffsetToManuscriptPosition(_ storageOffset: Int) -> Int {
            guard let tv = textView, let storage = tv.textStorage else { return storageOffset }
            var separatorCount = 0
            var pos = 0
            while pos < storageOffset && pos < storage.length {
                if SceneDivider.isDivider(in: storage, at: pos) {
                    separatorCount += 1
                    pos += 2
                } else {
                    pos += 1
                }
            }
            return storageOffset - (separatorCount * 2)
        }

        private func segmentIndex(for characterLocation: Int) -> Int? {
            for (i, range) in sceneBoundaries.enumerated() {
                let end = range.location + range.length
                if characterLocation >= range.location && characterLocation <= end {
                    return i
                }
            }
            // If after all segments, return last.
            return sceneBoundaries.isEmpty ? nil : sceneBoundaries.count - 1
        }

        private func makeDividerAttachment(_ state: DividerRenderState) -> NSTextAttachment {
            // ⚠️ EP-039 T-0526: was `attachment.attachmentCell = DividerAttachmentCell()`.
            // ✅ `NSTextAttachmentCell` is TEXTKIT 1 **and APPKIT-ONLY** — it has no UIKit
            // equivalent, so it blocked the iOS port independently of performance.
            // ✅ `DividerTextAttachment` overrides the TextKit 2 sizing/imaging API, which is
            // `macos(12.0), ios(15.0)` — the SAME type on both platforms.
            let attachment = DividerTextAttachment()
            attachment.renderState = state

            // ⛔ [I-0252] / T-0554 — THIS ONE LINE IS THE FIX, AND WITHOUT IT THE
            // DIVIDER DRAWS NOTHING AT ALL.
            //
            // ⚠️ MEASURED 2026-09-29 by rendering a real `NSTextView` to a bitmap
            // and sampling it: with no `image` PROPERTY set, TextKit 2 calls
            // `attachmentBounds` (so the 24 pt gap IS reserved) but NEVER calls
            // `image(for:)` — ⛔ `image(for:) calls = 0`, and an OPAQUE MAGENTA bar
            // did not put a single pixel on the canvas.
            //
            // ✅ Setting a placeholder `image` makes TK2 take the image path; the
            // `image(for:)` override then supplies the real, correctly-width-ed art
            // per layout pass (measured: `image(for:) calls = 2`, 48 rows drawn).
            // ⚠️ 1×1 is deliberate — the override always replaces it, so the
            // placeholder's own size is never used and must not be mistaken for the
            // divider's geometry (`attachmentBounds` owns that).
            //
            // ⚠️ WHY THIS TOOK THREE ATTEMPTS, recorded because the pattern is the
            // lesson: the first fix blamed a dropped appearance guard, the second
            // blamed `separatorColor`'s 1.34:1 contrast. ⛔ BOTH MEASUREMENTS WERE
            // CORRECT AND BOTH ANSWERED THE WRONG QUESTION — the line was never
            // being drawn, so no colour could have made it visible.
            attachment.image = NSImage(size: NSSize(width: 1, height: 1))

            return attachment
        }

        // MARK: — Cross-boundary Cut/Copy/Paste support (EP-029 SP-089, T-0354)
        //
        // The manuscript is one NSTextStorage; sceneBoundaries[i] is scene i's storage char
        // range (dividers/headings sit OUTSIDE those ranges). A selection is "cross-boundary"
        // when it overlaps more than one scene's range — then Cut/Copy/Paste route through the
        // ScriviCore fragment endpoints. A single-scene selection keeps the existing fast path
        // (AC5).

        // True if `sel` overlaps more than one scene boundary (i.e. spans a divider/heading).
        func selectionCrossesBoundary(_ sel: NSRange) -> Bool {
            guard sel.length > 0 else { return false }
            var touched = 0
            for range in sceneBoundaries where NSIntersectionRange(range, sel).length > 0 {
                touched += 1
                if touched > 1 { return true }
            }
            return false
        }

        // T-0572 — where a caret proposed at `loc` goes instead, when `loc` is in the GAP
        // between two scenes (nil = not in a gap, leave it).
        //
        // ✅ Layout: `[scene][divider][\n][heading][scene]` (heading only at a chapter break).
        // ✅ The DIVIDER'S OWN POSITION is the previous scene's END — a real caret stop, kept
        // by user ruling 2026-10-02 (typing there appends to that scene).
        // ⛔ The gap is everything AFTER it up to the next scene's first character: the
        // divider's `\n` and any heading. Typing after the divider prepended to the next
        // scene — or, at a chapter break, landed in FRONT of the heading and pulled it into
        // the scene text (user report 2026-10-02).
        // Moving BACKWARD (from a later position) → the divider position (previous scene's
        // end); otherwise → the next scene's start. Chapter 1's heading has no divider before
        // it, so it always resolves forward.
        // ✅ Read from the TEXT's own markers (attachment + heading attribute), not from
        // `sceneBoundaries`, which can be a beat stale mid-rebuild.
        func caretOutsideSceneGap(_ loc: Int, from previous: Int) -> Int? {
            guard let storage = textView?.textStorage else { return nil }
            func isDivider(_ i: Int) -> Bool {
                SceneDivider.isDivider(in: storage, at: i)
            }
            func headingRun(at i: Int) -> NSRange? {
                guard i < storage.length else { return nil }
                var run = NSRange()
                return storage.attribute(.scriviHeading, at: i, longestEffectiveRange: &run,
                                         in: NSRange(location: 0, length: storage.length)) != nil ? run : nil
            }
            let heading = headingRun(at: loc)
            let afterDivider = isDivider(loc - 1)
            guard heading != nil || afterDivider else { return nil }

            if loc < previous {
                if afterDivider { return loc - 1 }
                if let h = heading, isDivider(h.location - 2) { return h.location - 2 }
            }
            var p = loc
            if afterDivider { p += 1 }                       // past the divider's `\n`
            if let h = headingRun(at: p) { p = h.location + h.length }
            return min(p, storage.length)
        }

        // True if the caret at storage offset `loc` sits inside a non-editable scriviHeading run.
        func caretInHeading(_ loc: Int) -> Bool {
            guard let storage = textView?.textStorage, loc < storage.length else { return false }
            return storage.attribute(.scriviHeading, at: loc, effectiveRange: nil) != nil
        }

        // Map a storage selection to ordered per-scene byte spans (reading order). Each span is
        // (sceneID, startByte, endByte) with scene-local UTF-8 byte offsets. Only scenes the
        // selection actually overlaps produce a span. Returns nil if any overlapped boundary has
        // no backing segment (should not happen; guards against a rebuild race).
        func fragmentSpans(for sel: NSRange) -> [FragmentSpanArg]? {
            guard let tv = textView else { return nil }
            let ns = tv.string as NSString
            var spans: [FragmentSpanArg] = []
            for (i, range) in sceneBoundaries.enumerated() {
                let inter = NSIntersectionRange(range, sel)
                if inter.length == 0 {
                    // Include a zero-length touch only when the selection edge lands exactly at a
                    // scene start inside the selection (a boundary crossed with 0 chars selected).
                    let atStart = sel.location <= range.location && range.location < sel.location + sel.length
                    if !(atStart) { continue }
                }
                guard parent.loader.segments.indices.contains(i) else { return nil }
                let seg = parent.loader.segments[i]
                // Scene-local char offsets, then convert each to a UTF-8 byte offset in the body.
                let startChar = max(0, (inter.length == 0 ? range.location : inter.location) - range.location)
                let endChar   = max(startChar, (inter.length == 0 ? range.location : inter.location + inter.length) - range.location)
                let startByte = byteOffset(charOffset: startChar, in: seg.text)
                let endByte   = byteOffset(charOffset: endChar,   in: seg.text)
                spans.append(.init(sceneID: seg.sceneID, start: startByte, end: endByte))
                _ = ns  // (kept for clarity; spans derive from the loader's canonical scene text)
            }
            return spans.isEmpty ? nil : spans
        }

        // Scene-local UTF-16 char offset → scene-local UTF-8 byte offset (history's convention).
        private func byteOffset(charOffset: Int, in sceneText: String) -> Int {
            let nsText = sceneText as NSString
            let clamped = max(0, min(charOffset, nsText.length))
            return nsText.substring(to: clamped).utf8.count
        }

        // Reload the whole manuscript from disk after a structural fragment op (cut/paste create
        // or delete scenes+chapters). Re-opens the project → fresh scene list → rebuilds storage,
        // then places the caret at `caretSceneID` + scene-local byte offset. The robust path (vs
        // the in-memory patching the single merge/split handlers do — too fragile for N scenes).
        func reloadManuscriptFromDisk(caretSceneID: String, caretByteOffset: Int) {
            guard let tv = textView else { return }
            let loader = parent.loader
            let session = parent.session
            guard let rootPath = session.projectRootPath else { return }
            let appSupport = session.appSupportRoot

            // Re-open to get the authoritative post-op scene list.
            guard let reopened = try? parent.env.engine.openProject(
                projectRootPath: rootPath, appSupportRoot: appSupport) else {
                print("[Scrivi] reloadManuscriptFromDisk: openProject failed")
                return
            }
            loader.replaceScenes(reopened.scenes, activeSceneID: caretSceneID)
            rebuildStorage(tv, segments: loader.segments)
            ensureBoundaries(tv)

            // Place the caret at the target scene + scene-local byte offset.
            if let segIdx = loader.segments.firstIndex(where: { $0.sceneID == caretSceneID }),
               sceneBoundaries.indices.contains(segIdx) {
                let base = sceneBoundaries[segIdx].location
                let charOff = charOffsetForByteOffset(caretByteOffset, in: loader.segments[segIdx].text)
                revealCaretAfterRebuild(base + charOff, in: tv)   // [I-0271] + T-0573
                takeFocus()
                loader.setCurrentIndex(segIdx)
                loader.setViewportScene(caretSceneID)
            }

            // Keep the timeline in sync (scene set changed).
            session.timelineModel?.reloadSceneDots(
                engine: parent.env.engine, projectRootPath: rootPath, scenes: loader.allScenes)
            session.timelineModel?.updateDotTitles(liveTitles: loader.liveTitles, allScenes: loader.allScenes)
        }

        // The internal structured clipboard (T2=A): a fragment placed by a cross-boundary ⌘C/⌘X.
        // The system pasteboard carries only the flat plainText for external apps; the structured
        // fragment lives here so an internal ⌘V can reconstruct boundaries. Nil until a
        // cross-boundary copy/cut runs; a single-scene copy/cut clears it (so ⌘V falls back to the
        // plain system-pasteboard path).
        private var internalClipboardFragment: FragmentResult? {
            get { structuredClipboard.fragment(currentChangeCount: NSPasteboard.general.changeCount) }
            set { structuredClipboard.hold(newValue, changeCount: NSPasteboard.general.changeCount) }
        }
        /// ⛔ [I-0279]: the fragment used to be held FOREVER — every later ⌘V pasted it, whatever had been copied
        /// since. ✅ Now it is tied to the system pasteboard's change count, as `ownCopy` is.
        private var structuredClipboard = StructuredClipboard()

        // Flash the screen + beep and do NOT paste — the caret sits in a non-editable chapter
        // heading (§4.2, §10 Q2, user ruling 2026-07-27). Leaves the caret where it is.
        private func flashRefuse(_ tv: NSTextView) {
            NSSound.beep()
            if let layer = tv.enclosingScrollView?.layer ?? tv.layer {
                let flash = CABasicAnimation(keyPath: "backgroundColor")
                flash.fromValue = NSColor.systemRed.withAlphaComponent(0.18).cgColor
                flash.toValue   = NSColor.clear.cgColor
                flash.duration  = 0.18
                layer.add(flash, forKey: "scrivi.headingRefuseFlash")
            }
        }

        // Cross-boundary ⌘C: extract the fragment, put its flat plainText on the system pasteboard
        // (external apps get clean text), and hold the structured fragment internally for ⌘V.
        // Returns true if it handled the copy (selection crossed a boundary); false → caller uses
        // the normal single-scene copy.
        func structuredCopyIfCrossBoundary() -> Bool {
            guard let tv = textView else { return false }
            let sel = tv.selectedRange()
            guard selectionCrossesBoundary(sel), let spans = fragmentSpans(for: sel),
                  let rootPath = parent.session.projectRootPath else { return false }
            guard let raw = try? parent.env.engine.fragmentExtract(
                projectRootPath: rootPath, spans: spans) else { return false }
            // ✅ [T-0591] (ruled 2026-10-05: Apple-side): each scene's piece is BALANCED — a piece that starts or ends
            // inside a bold span carries its own markers (AC12) — and other apps get what the writer SEES ([SP-162] Q2:
            // no markers, no `## `, no escapes), pieces joined by a blank line as ScriviCore joins them.
            var frag = raw
            var presented = MarkdownEscapes.map(raw.plainText).presented
            if let storage = tv.textStorage, let parts = sceneParts(of: sel), parts.count == raw.pieces.count {
                let copies = parts.map { presenter.balancedCopy(in: storage, $0) }
                let pieces = zip(raw.pieces, copies).map { piece, copy in
                    FragmentPiece(opensWith: piece.opensWith, chapterTitle: piece.chapterTitle,
                                  sceneTitle: piece.sceneTitle, text: copy.source, partial: piece.partial)
                }
                frag = FragmentResult(schema: raw.schema, pieces: pieces,
                                      plainText: pieces.map(\.text).joined(separator: "\n\n"))
                presented = copies.map(\.presented).joined(separator: "\n\n")
            }
            let pb = NSPasteboard.general
            pb.clearContents()
            // EP-045 AC4: other apps get what the writer SEES, not escape backslashes. ⌘V in
            // Scrivi still reconstructs from `internalClipboardFragment`, which keeps the source.
            pb.setString(presented, forType: .string)
            // ⚠️ [I-0279]: held AFTER the pasteboard write, so it matches the change count that write produced.
            internalClipboardFragment = frag
            return true
        }

        // ✅ [I-0270] (user ruling 2026-10-02) — DELETE ACROSS SCENES, KEEP THE SCENES.
        // ⌫ / ⌦ / ⌘X / ⌥N on a selection that spans scene breaks removes the selected text
        // from EACH scene it overlaps and keeps every scene, divider and heading; a scene
        // wholly inside the selection becomes EMPTY, not deleted. User: *"when I copy text
        // across a scene boundary and paste it somewhere else the scene boundary does not
        // copy with the text. Therefore in this instance, and with cut, the scene boundary
        // also must not disappear."*
        // ⛔ This REPLACES the EP-029 cross-boundary cut (`fragmentCut` = delete + COLLAPSE
        // into a survivor). Old `structuredCut` history nodes still undo via
        // `applyStructuralInverse`.
        // ✅ Recorded as ONE edit group (one event per scene, shared groupID), so a single
        // ⌘Z restores every scene and a single redo re-applies them (user ruling, same day).
        // Returns false for a selection inside one scene — the ordinary path handles it.
        @discardableResult
        /// ✅ [SP-164] Q2: Replace All is ONE undoable step, even across scenes — the replacements are applied with history
        /// capture suppressed, then recorded as ONE grouped edit (as [I-0270]'s cross-scene delete is). `edits` run back to
        /// front (storage ranges, unescaped replacement text).
        func replaceAll(_ edits: [(NSRange, String)]) {
            guard let tv = textView as? ManuscriptNSTextView, !edits.isEmpty else { return }
            ensureBoundaries(tv)
            let touched = Set(edits.compactMap { segmentIndex(for: $0.0.location) })
            var before: [Int: String] = [:]
            for i in touched where sceneBoundaries.indices.contains(i) {
                before[i] = (tv.string as NSString).substring(with: sceneBoundaries[i])
            }
            let t0 = Date()
            guard let capture = parent.session.historyCapture else {
                tv.applyReplacements(edits)
                return
            }
            capture.flush(trigger: "replace")
            capture.withApplying { tv.applyReplacements(edits) }
            let tApplied = Date()
            ensureBoundaries(tv)
            let loader = parent.loader
            var grouped: [HistoryCapture.GroupedSceneEdit] = []
            for i in touched.sorted() where sceneBoundaries.indices.contains(i) && loader.segments.indices.contains(i) {
                let after = (tv.string as NSString).substring(with: sceneBoundaries[i])
                let seg = loader.segments[i]
                loader.updateText(after, at: i)
                grouped.append(.init(sceneID: seg.sceneID, textBefore: before[i] ?? after,
                                     textAfter: after, cursorByte: 0))
                let firstLine = after.components(separatedBy: .newlines)
                    .first { !$0.trimmingCharacters(in: .whitespaces).isEmpty } ?? ""
                loader.updateLiveTitle(firstLine, forSceneID: seg.sceneID)
            }
            capture.recordGroupedEdit(grouped, kind: "replace")
            let tRecorded = Date()
            if let ref = parent.env.authorshipRef {
                let env = parent.env
                for i in touched {
                    Task { @MainActor in await loader.saveScene(at: i, engine: env.engine, ref: ref) }
                }
            }
            parent.session.timelineModel?.updateDotTitles(liveTitles: loader.liveTitles, allScenes: loader.allScenes)
            NSLog(String(format: "[SCRIVI-FIND] replaceAll %d edits / %d scenes: apply=%.0f ms  history=%.0f ms",
                         edits.count, touched.count, tApplied.timeIntervalSince(t0) * 1000,
                         tRecorded.timeIntervalSince(tApplied) * 1000))
        }

        /// The selected part of each scene `sel` overlaps, in storage order — what `fragmentSpans` sends ScriviCore.
        func sceneParts(of sel: NSRange) -> [NSRange]? {
            var parts: [NSRange] = []
            for range in sceneBoundaries {
                let inter = NSIntersectionRange(range, sel)
                if inter.length > 0 { parts.append(inter) }
                else if sel.location <= range.location && range.location < NSMaxRange(sel) {
                    parts.append(NSRange(location: range.location, length: 0))
                }
            }
            return parts.isEmpty ? nil : parts
        }

        func deleteAcrossScenes(_ sel: NSRange, kind: String) -> Bool {
            guard let tv = textView, let storage = tv.textStorage, sel.length > 0 else { return false }
            ensureBoundaries(tv)
            let loader = parent.loader
            // Every scene the selection overlaps: its index, its range BEFORE the edit,
            // and the part of it that is selected.
            var parts: [(segIdx: Int, range: NSRange, cut: NSRange)] = []
            for (i, range) in sceneBoundaries.enumerated() where loader.segments.indices.contains(i) {
                let cut = NSIntersectionRange(range, sel)
                if cut.length > 0 { parts.append((i, range, cut)) }
            }
            guard parts.count > 1 else { return false }

            let ns = tv.string as NSString
            let before = parts.map { ns.substring(with: $0.range) }
            // Back to front, so the earlier ranges stay valid. Only scene text is removed;
            // dividers and headings sit outside every scene range.
            // ✅ [T-0591]: each scene's removal is BALANCED (E2-S2's `balancedEdit`) — a cut that starts inside bold
            // closes it in the first scene, one that ends inside bold re-opens it in the last.
            var caret = parts[0].cut.location
            storage.beginEditing()
            for part in parts.reversed() {
                if let edit = presenter.balancedEdit(in: storage, replacing: part.cut, with: "") {
                    storage.replaceCharacters(in: edit.range, with: edit.replacement)
                    if part.segIdx == parts[0].segIdx { caret = edit.caret }
                } else {
                    storage.deleteCharacters(in: part.cut)
                }
            }
            storage.endEditing()
            ensureBoundaries(tv)

            var edits: [HistoryCapture.GroupedSceneEdit] = []
            for (n, part) in parts.enumerated() where sceneBoundaries.indices.contains(part.segIdx) {
                let after = (tv.string as NSString).substring(with: sceneBoundaries[part.segIdx])
                let seg = loader.segments[part.segIdx]
                loader.updateText(after, at: part.segIdx)
                edits.append(.init(sceneID: seg.sceneID, textBefore: before[n], textAfter: after,
                                   cursorByte: byteOffset(charOffset: part.cut.location - part.range.location,
                                                          in: before[n])))
                let firstLine = after.components(separatedBy: .newlines)
                    .first { !$0.trimmingCharacters(in: .whitespaces).isEmpty } ?? ""
                loader.updateLiveTitle(firstLine, forSceneID: seg.sceneID)
            }
            parent.session.historyCapture?.recordGroupedEdit(edits, kind: kind)

            // The caret goes where the edit began, in the first scene.
            tv.setSelectedRange(NSRange(location: caret, length: 0))
            tv.scrollRangeToVisible(NSRange(location: caret, length: 0))
            lastCursorSegmentIndex = parts[0].segIdx
            loader.setCurrentIndex(parts[0].segIdx)

            // Save every touched scene — the 1 s autosave only writes the CURRENT one.
            if let ref = parent.env.authorshipRef {
                let env = parent.env
                for part in parts {
                    Task { @MainActor in await loader.saveScene(at: part.segIdx, engine: env.engine, ref: ref) }
                }
            }
            parent.session.timelineModel?.updateDotTitles(liveTitles: loader.liveTitles, allScenes: loader.allScenes)
            return true
        }

        // ⌘V when the internal clipboard holds a structured fragment: reconstruct it at the caret.
        // Refuses (flash) if the caret is in a heading. Returns true if handled; false → caller
        // uses the normal system-pasteboard paste (no structured fragment, or single-scene).
        func structuredPasteIfAvailable() -> Bool {
            guard let frag = internalClipboardFragment else { return false }
            return pasteStructuredFragment(frag)
        }

        // Reconstruct `frag` at the caret (shared by ⌘V of the internal clipboard and ⌃N of a
        // structured buffer slot — T-0355). Refuses (flash) if the caret is in a heading.
        // Returns true if handled (pasted or refused); false → caller should fall back.
        func pasteStructuredFragment(_ frag: FragmentResult) -> Bool {
            guard let tv = textView else { return false }
            let loc = tv.selectedRange().location
            if caretInHeading(loc) { flashRefuse(tv); return true }   // handled = refused, no super
            guard let segIdx = segmentIndex(for: loc),
                  parent.loader.segments.indices.contains(segIdx),
                  sceneBoundaries.indices.contains(segIdx),
                  let rootPath = parent.session.projectRootPath,
                  let ref = parent.env.authorshipRef,
                  let projectID = parent.session.openProjectResult?.projectID else { return false }

            let seg = parent.loader.segments[segIdx]
            let caretChar = max(0, loc - sceneBoundaries[segIdx].location)
            let caretByte = byteOffset(charOffset: caretChar, in: seg.text)

            parent.session.historyCapture?.flush(trigger: "flush")
            // ✅ [T-0591]: pasting INSIDE a bold span — the first piece closes it, the last re-opens it for the text after
            // the split. ✅ [SP-163] live pass (user): the pasted text KEEPS ITS OWN formatting (only what was bold is).
            var frag = frag
            if let storage = tv.textStorage {
                let style = presenter.styleAtCaret(in: storage, loc)
                let texts = ManuscriptPresenter.balancePastePieces(frag.pieces.map(\.text), caretStyle: style)
                if texts != frag.pieces.map(\.text) {
                    let pieces = zip(frag.pieces, texts).map { piece, text in
                        FragmentPiece(opensWith: piece.opensWith, chapterTitle: piece.chapterTitle,
                                      sceneTitle: piece.sceneTitle, text: text, partial: piece.partial)
                    }
                    frag = FragmentResult(schema: frag.schema, pieces: pieces, plainText: pieces.map(\.text).joined(separator: "\n\n"))
                }
            }
            guard let result = try? parent.env.engine.fragmentPaste(
                projectRootPath: rootPath,
                appSupportRoot: parent.session.appSupportRoot,
                projectID: projectID,
                fragmentJSON: frag.toJSON(),
                caretSceneID: seg.sceneID,
                caretByteOffset: caretByte,
                authorshipRef: ref) else { return false }

            // Reversible structural op (T-0356 / AC6): undo cuts away exactly the created
            // scenes/chapters and re-joins the split target (undo-paste == cut-merge, §5); redo
            // re-runs the paste.
            parent.session.historyCapture?.recordBarrier(
                kind: "structuredPaste", note: "Can't undo past a cross-boundary paste",
                structuralPayload: HistoryStructuralPayload(
                    op: "structuredPaste",
                    fragmentJSON: frag.toJSON(),
                    caretSceneID: result.targetSceneID,
                    caretByte: caretByte,
                    createdSceneIDs: result.createdSceneIDs,
                    createdChapterIDs: result.createdChapterIDs))
            // Place the caret at the start of the paste target (the split point); the reload
            // re-anchors to a valid scene.
            reloadManuscriptFromDisk(caretSceneID: result.targetSceneID, caretByteOffset: caretByte)
            return true
        }

        // True when a structured fragment is available for ⌘V (drives paste routing).
        var hasInternalStructuredFragment: Bool { internalClipboardFragment != nil }
    }
}

// MARK: — Custom NSTextView for key binding interception

// Subclass so we can intercept ⌘↩ and ⌘⇧↩ before the text system handles them.
final class ManuscriptNSTextView: NSTextView {

    // T-0531 DIAGNOSTIC — the OUTERMOST edit boundary AppKit gives us.
    // ⚠️ `[SCRIVI-KEY]` covers `textDidChange` only. If a keystroke is slow but KEY is
    // fast, the cost is BETWEEN these two — i.e. in AppKit's own edit/layout/display
    // work, not in Scrivi's delegate.
    override func didChangeText() {
        let t0 = Date()
        super.didChangeText()
        let ms = Date().timeIntervalSince(t0) * 1000
        if ms > 0.5 { NSLog(String(format: "[SCRIVI-EDIT] didChangeText=%.1f ms", ms)) }
    }

    override func shouldChangeText(in affectedCharRange: NSRange, replacementString: String?) -> Bool {
        let __t0 = Date()
        defer {
            let ms = Date().timeIntervalSince(__t0) * 1000
            if ms > 0.5 { NSLog(String(format: "[SCRIVI-EDIT] shouldChangeText=%.1f ms", ms)) }
        }
        guard let storage = textStorage, affectedCharRange.length > 0 || (replacementString?.isEmpty == false) else {
            return super.shouldChangeText(in: affectedCharRange, replacementString: replacementString)
        }
        // Block any edit that touches a character with the scriviHeading attribute.
        // For pure insertions (length == 0, non-empty replacement) check the character
        // AT the insertion point — do NOT look backward, as that would land inside
        // heading text when the cursor is at the start of a scene.
        // ⚠️ [I-0266] It used to check a ZERO-length range, which matches nothing, so
        // typing at offset 0 (before Chapter 1's heading) was let through. ✅ An insertion
        // whose next character is heading text is BEFORE or INSIDE a heading — the same
        // rule `caretInHeading` applies to structured paste.
        // For deletions (length > 0) or backspace-style (length == 0, empty replacement)
        // check the character being removed, falling back one position for backspace.
        let isInsertion = affectedCharRange.length == 0 && replacementString?.isEmpty == false
        let checkRange: NSRange
        if isInsertion {
            checkRange = NSRange(location: affectedCharRange.location, length: 1)
        } else {
            checkRange = affectedCharRange.length > 0
                ? affectedCharRange
                : NSRange(location: max(0, affectedCharRange.location - 1), length: 1)
        }
        var isHeading = false
        if checkRange.location < storage.length {
            let safeRange = NSRange(
                location: checkRange.location,
                length: min(checkRange.length, storage.length - checkRange.location)
            )
            if safeRange.length > 0 {
                storage.enumerateAttribute(.scriviHeading, in: safeRange, options: []) { value, _, stop in
                    if value != nil { isHeading = true; stop.pointee = true }
                }
            }
        }
        if isHeading { return false }
        // ⛔ [I-0270] NEVER let an ordinary edit remove a scene DIVIDER. Each divider is one
        // scene boundary; `recomputeBoundaries` maps the Nth text segment to the Nth loaded
        // scene, so deleting one made the merged text save into the first scene (the second
        // scene's tail then existed TWICE on disk) and shifted every later scene's edits into
        // the PREVIOUS scene's file. Joining scenes is a structural op (⌘⌫ merge, cross-scene
        // ⌘X) that goes through ScriviCore and rebuilds — never a text edit.
        if affectedCharRange.length > 0, affectedCharRange.location < storage.length {
            let safe = NSRange(location: affectedCharRange.location,
                               length: min(affectedCharRange.length, storage.length - affectedCharRange.location))
            var hasDivider = false
            storage.enumerateAttribute(.scriviDivider, in: safe, options: []) { value, _, stop in
                if value != nil { hasDivider = true; stop.pointee = true }
            }
            if hasDivider { return false }
        }
        // ✅ EP-045 AC4: an escape pair (`\*`) is ONE unit. ⚠️ Measured 2026-10-03: ⌫ after a `*`
        // removed only the `*` (leaving `a\b` — an orphaned backslash that becomes VISIBLE), and
        // ⌦ before the `\` removed only the backslash (leaving a LIVE `*`). Re-issue the edit
        // over the whole pair instead. (EP-046: the same for any stop run — a marker is never split.)
        // ⚠️ Never on an edit the presenter COMPUTED (balanced edits, Format commands): it is exact, and widening it
        // past a stop run it starts beside (a list's `1. `) replaced the prefix — [SP-163] live pass, step 12.
        if affectedCharRange.length > 0, !applyingBalancedEdit, let presenter,
           let pair = MarkdownEscapes.snapSelection(affectedCharRange, in: storage.string as NSString,
                                                    runAt: Self.runLookup(presenter, storage)) {
            if shouldChangeText(in: pair, replacementString: replacementString) {
                let replacement = NSAttributedString(
                    string: replacementString ?? "",
                    attributes: typography.bodyAttributes)
                storage.replaceCharacters(in: pair, with: replacement)
                didChangeText()
                setSelectedRange(NSRange(location: pair.location + replacement.length, length: 0))
            }
            return false
        }
        // ✅ EP-046 AC12 / [SP-162] Q3: an edit that would leave emphasis UNBALANCED — a cut or delete
        // across a span edge, typing or pasting over one, a Return inside bold, Scrivi's own copy pasted
        // into bold — is re-issued as the BALANCED edit of the stretch it touches (`balancedEdit`). ONE
        // replacement, so ONE history event. ✅ Plain typing inside or outside a span returns nil there.
        if !applyingBalancedEdit, let presenter, let replacementString,
           let edit = presenter.balancedEdit(in: storage, replacing: affectedCharRange, with: replacementString,
                                             keepsOwnFormatting: pastingOwnCopy) {
            applyingBalancedEdit = true
            defer { applyingBalancedEdit = false }
            if shouldChangeText(in: edit.range, replacementString: edit.replacement) {
                storage.replaceCharacters(in: edit.range, with: NSAttributedString(
                    string: edit.replacement,
                    attributes: typography.bodyAttributes))
                didChangeText()
                setSelectedRange(NSRange(location: edit.caret, length: 0))
            }
            return false
        }
        return super.shouldChangeText(in: affectedCharRange, replacementString: replacementString)
    }

    /// True while a balanced edit is being applied — it must not be balanced again.
    private var applyingBalancedEdit = false

    // MARK: — EP-046 E2-S4: Find and Replace over what the writer SEES ([SP-164])

    /// The client AppKit's find bar searches: the manuscript AS PRESENTED (Q1, Q-E2-5).
    let finderClient = ManuscriptFinderClient()
    private var finderObserver: NSObjectProtocol?
    lazy var textFinder: NSTextFinder = {
        let finder = NSTextFinder()
        finderClient.textView = self
        finder.client = finderClient
        finder.findBarContainer = enclosingScrollView
        finder.isIncrementalSearchingEnabled = true
        // Every character edit invalidates the presented text; AppKit must hear of it BEFORE the change.
        if let storage = textStorage {
            finderObserver = NotificationCenter.default.addObserver(
                forName: NSTextStorage.willProcessEditingNotification, object: storage, queue: nil) { [weak self] note in
                guard let ts = note.object as? NSTextStorage, ts.editedMask.contains(.editedCharacters) else { return }
                MainActor.assumeIsolated {
                    self?.textFinder.noteClientStringWillChange()
                    self?.finderClient.storageVersion += 1
                }
            }
        }
        return finder
    }()

    /// What Edit ▸ Find calls.
    func performFind(_ command: ManuscriptFindCommand) {
        guard let action = NSTextFinder.Action(rawValue: command.rawValue) else { return }
        textFinder.performAction(action)
    }

    /// AppKit's own Find items (if any reach this view) go to the same finder.
    override func performTextFinderAction(_ sender: Any?) {
        guard let tag = (sender as? NSValidatedUserInterfaceItem)?.tag,
              let action = NSTextFinder.Action(rawValue: tag) else { return }
        textFinder.performAction(action)
    }

    /// Replace one found range (STORAGE) with what the writer typed in the find bar: ✅ escaped like typing and balanced like
    /// any edit — it takes the style of the match's first character (Q3).
    func replaceFound(_ range: NSRange, with string: String) {
        insertText(string, replacementRange: range)
    }

    /// Replace All's replacements (STORAGE ranges, back to front, typed text) in ONE editing pass — each escaped, snapped
    /// and balanced exactly as `replaceFound` does it. ⚠️ [SP-164] live pass: 1,172 replacements through the typing path
    /// (`replaceFound` each) kept the app busy for about a minute on dumas.
    func applyReplacements(_ edits: [(NSRange, String)]) {
        guard let storage = textStorage else { return }
        let attrs: [NSAttributedString.Key: Any] = typography.bodyAttributes
        storage.beginEditing()
        for (r, typed) in edits {
            let escaped = MarkdownEscapes.escape(typed)
            var range = r
            if range.length > 0, let presenter,
               let pair = MarkdownEscapes.snapSelection(range, in: storage.string as NSString,
                                                        runAt: Self.runLookup(presenter, storage)) {
                range = pair
            }
            if let presenter, let edit = presenter.balancedEdit(in: storage, replacing: range, with: escaped) {
                storage.replaceCharacters(in: edit.range, with: NSAttributedString(string: edit.replacement, attributes: attrs))
            } else {
                storage.replaceCharacters(in: range, with: NSAttributedString(string: escaped, attributes: attrs))
            }
        }
        storage.endEditing()
        let len = storage.length
        let sel = selectedRange()
        setSelectedRange(NSRange(location: min(sel.location, len), length: 0))
    }
    /// True while Scrivi's OWN copy is pasted: it keeps its own formatting.
    private var pastingOwnCopy = false

    /// The presenter's stop runs in the form the snap takes.
    static func runLookup(_ presenter: ManuscriptPresenter, _ ts: NSAttributedString) -> (Int) -> MarkdownEscapes.StopRun? {
        let stops = presenter.stopTest(in: ts)
        return { i in stops(i).map { ($0.range, $0.homeAfter) } }
    }

    /// ✅ EP-046 E2-S1 — the view's presenter, found through TextKit 2's own delegate slot so a view
    /// built without a coordinator (the interop fixtures) works the same way.
    var presenter: ManuscriptPresenter? { textContentStorage?.delegate as? ManuscriptPresenter }
    /// ✅ EP-047 AC4: the project's type — read from the presenter's ONE stored value, never kept here.
    var typography: ManuscriptTypography { presenter?.typography ?? .default }

    // MARK: — EP-045 AC4: the escape layer

    /// True while inserting text that is ALREADY in stored form (a copy-buffer slot, or Scrivi's
    /// own copy coming back) — it must not be escaped a second time.
    private var insertsVerbatim = false
    /// What THIS view last put on a pasteboard, in STORED form, keyed to that pasteboard write.
    /// ⚠️ The pasteboard carries what the writer SEES; pasting it back must restore the exact
    /// source — re-escaping it would turn intended markup (R2: e.g. a `##` heading) literal.
    private var ownCopy: (pasteboard: NSPasteboard.Name, changeCount: Int, source: String)?

    /// ✅ Typing escapes: every keystroke and IME commit arrives here (measured 2026-10-03).
    override func insertText(_ string: Any, replacementRange: NSRange) {
        guard !insertsVerbatim else { return super.insertText(string, replacementRange: replacementRange) }
        let typed = (string as? String) ?? (string as? NSAttributedString)?.string ?? ""
        // ✅ [SP-163] Q1: whitespace typed into an EMPTY pending pair cannot start a span (`** x**` is not emphasis):
        // it goes BEFORE the pair, which stays pending with the caret inside it.
        if let p = presenter?.pending, selectedRange() == NSRange(location: p.caret, length: 0),
           !typed.isEmpty, typed.allSatisfy({ $0 == " " || $0 == "\t" }) {
            let markers = String(repeating: "*", count: p.markerLength)
            removePendingPair()
            super.insertText(MarkdownEscapes.escape(typed), replacementRange: NSRange(location: p.range.location, length: 0))
            insertPendingPair(at: selectedRange().location, markers: markers)
            return
        }
        super.insertText(MarkdownEscapes.escape(typed), replacementRange: replacementRange)
    }

    /// ✅ EP-045 AC5 + AC6 — Return starts a PARAGRAPH: it stores `\n\n` (study §3A.6), first
    /// reducing the trailing spaces before the caret to AT MOST ONE (§4B.6: two or more are a
    /// CommonMark hard break once a ⌫ merges the paragraphs) and collapsing a trailing typed
    /// backslash (`\\`) to `\`, so that a merge leaves the break the writer typed.
    /// ⚠️ Measured 2026-10-04: Return, keypad Enter AND Shift-Return all send `insertNewline:`;
    /// Option-Return sends `insertNewlineIgnoringFieldEditor:` and is NOT changed here.
    /// ⛔ ONE replacement, never several: Return is a history commit boundary
    /// (`isCommitBoundary`), so a split edit would commit half the gesture as its own undo step.
    override func insertNewline(_ sender: Any?) {
        guard let storage = textStorage, !hasMarkedText() else { return super.insertNewline(sender) }
        let sel = selectedRange()
        // ✅ [SP-163] Return in a LIST ITEM starts the next item (an empty item ends the list), renumbering what follows.
        if let presenter, let edit = presenter.listReturnEdit(in: storage, selection: sel) {
            applyCommandEdit(edit)
            return
        }
        // ⚠️ Read backward from the caret only — ⛔ never copy the document (EP-045 trap #2).
        let ns = storage.string as NSString
        var start = sel.location
        while start > 0, ns.character(at: start - 1) == 0x20 { start -= 1 }
        var tail = start < sel.location ? " " : ""
        if tail.isEmpty {
            // Only an escape PAIR collapses: an even run of backslashes is all `\\` pairs.
            var run = 0
            while run < start, ns.character(at: start - 1 - run) == 0x5C { run += 1 }
            if run >= 2, run % 2 == 0 { start -= 2; tail = "\\" }
        }
        let range = NSRange(location: start, length: NSMaxRange(sel) - start)
        insertVerbatim(tail + "\n\n", replacementRange: range)
    }

    /// Insert text that is already in stored form, unescaped.
    func insertVerbatim(_ source: String, replacementRange: NSRange) {
        insertsVerbatim = true
        defer { insertsVerbatim = false }
        insertText(source, replacementRange: replacementRange)
    }

    /// ✅ [T-0584] (Q-E2-4 = (b), ruled 2026-10-05): Option-Return is a deliberate HARD LINE BREAK — `\` + `\n`, the
    /// form EP-045 AC6 already writes. ⚠️ Measured 2026-10-04: Option-Return sends THIS action, not `insertNewline:`.
    /// ⚠️ [SP-163] Q6: at a paragraph's END the backslash shows until the next line is typed (CommonMark reads it as
    /// literal before a blank line) — accepted.
    override func insertNewlineIgnoringFieldEditor(_ sender: Any?) {
        guard !hasMarkedText() else { return super.insertNewlineIgnoringFieldEditor(sender) }
        insertVerbatim("\\\n", replacementRange: selectedRange())
    }

    // MARK: — EP-046 E2-S3: the Format commands ([SP-163], Q-E2-2)

    /// What the Format menu calls. ✅ ONE edit per command — one history event.
    func applyFormat(_ format: ManuscriptFormat) {
        guard let storage = textStorage, let presenter else { return }
        let sel = selectedRange()
        switch format {
        case .bold, .italic:
            let bit = format == .bold ? MarkdownBlocks.bold : MarkdownBlocks.italic
            let markers = format == .bold ? "**" : "*"
            if sel.length > 0 {
                if let edit = presenter.emphasisEdit(in: storage, selection: sel, bit: bit) { applyCommandEdit(edit) } else { NSSound.beep() }
                return
            }
            // ✅ Q1: the same command on a pending pair takes it away again.
            if let p = presenter.pending, sel.location == p.caret {
                removePendingPair()
                return
            }
            // ✅ Q1: IN a word → format the word, the caret stays where it was.
            if let word = presenter.wordRange(in: storage, at: sel.location) {
                if let edit = presenter.emphasisEdit(in: storage, selection: word, bit: bit, caret: sel.location) {
                    applyCommandEdit(edit)
                } else { NSSound.beep() }
                return
            }
            // ✅ Q1: BETWEEN words → a pending pair, the caret between its hints.
            insertPendingPair(at: sel.location, markers: markers)
        case .heading, .body, .bulletList, .numberedList:
            if let edit = presenter.paragraphEdit(in: storage, selection: sel, format: format) { applyCommandEdit(edit) } else { NSSound.beep() }
        }
    }

    /// Apply a presenter-computed edit as ONE replacement (one `textDidChange`, one history event), through the
    /// heading/divider guards but never re-balanced (it already is).
    private func applyCommandEdit(_ edit: ManuscriptPresenter.CommandEdit) {
        guard let storage = textStorage else { return }
        applyingBalancedEdit = true
        defer { applyingBalancedEdit = false }
        guard shouldChangeText(in: edit.range, replacementString: edit.replacement) else { return }
        storage.replaceCharacters(in: edit.range, with: NSAttributedString(
            string: edit.replacement,
            attributes: typography.bodyAttributes))
        didChangeText()
        setSelectedRange(edit.selection)
    }

    /// Insert the PENDING pair (Q1) — storage only, NO `didChangeText`: history, autosave and the scene text never see
    /// it (see `ManuscriptPresenter.PendingPair`).
    private func insertPendingPair(at loc: Int, markers: String) {
        guard let storage = textStorage, let presenter else { return }
        presenter.insertingPending = true
        storage.replaceCharacters(in: NSRange(location: loc, length: 0), with: NSAttributedString(
            string: markers + markers,
            attributes: typography.bodyAttributes))
        presenter.insertingPending = false
        presenter.pending = .init(range: NSRange(location: loc, length: markers.utf16.count * 2), markerLength: markers.utf16.count)
        keepingPending = true
        setSelectedRange(NSRange(location: loc + markers.utf16.count, length: 0))
        keepingPending = false
    }

    /// Take the pending pair out again — storage only, nothing recorded (user: *"it should be removed"*).
    func removePendingPair() {
        guard let storage = textStorage, let presenter, let p = presenter.pending else { return }
        presenter.pending = nil
        let text = (storage.string as NSString)
        guard NSMaxRange(p.range) <= text.length,
              text.substring(with: p.range) == String(repeating: "*", count: p.range.length) else { return }
        presenter.insertingPending = true
        storage.replaceCharacters(in: p.range, with: "")
        presenter.insertingPending = false
    }

    /// True while the view itself places the caret inside a pending pair.
    private var keepingPending = false

    /// ✅ Copy, cut and drag put what the writer SEES on the pasteboard (measured: all three
    /// route here), and remember the stored form for a paste back into Scrivi.
    override func writeSelection(to pboard: NSPasteboard, type: NSPasteboard.PasteboardType) -> Bool {
        let sel = selectedRange()
        guard Self.isPlainTextType(type), sel.length > 0, let storage = textStorage else {
            return super.writeSelection(to: pboard, type: type)
        }
        // ✅ EP-046 AC12 + [SP-162] Q2: other apps get what the writer SEES (no markers, no prefix, no
        // escapes); Scrivi's own paste gets the BALANCED source — a selection that starts or ends inside a
        // bold span carries its own markers, so the paste stays bold.
        let raw = (storage.string as NSString).substring(with: sel)
        let copy: (source: String, presented: String) = presenter?.balancedCopy(in: storage, sel)
            ?? (raw, MarkdownEscapes.map(raw).presented)
        let ok = pboard.setString(copy.presented, forType: .string)
        if ok { ownCopy = (pboard.name, pboard.changeCount, copy.source) }
        return ok
    }

    /// ⛔ USER-FOUND 2026-10-03: AppKit calls the two overrides below with the LEGACY type
    /// `NSStringPboardType` — ⚠️ which is NOT equal to `.string` (`public.utf8-plain-text`). The
    /// first version tested `type == .string`, so BOTH fell through to AppKit: copy wrote the
    /// stored backslashes and paste inserted unescaped text. Its tests passed only because they
    /// named `.string` themselves. ✅ Accept either spelling.
    private static let legacyStringType = NSPasteboard.PasteboardType("NSStringPboardType")
    static func isPlainTextType(_ type: NSPasteboard.PasteboardType) -> Bool {
        type == .string || type == legacyStringType
    }

    /// The plain text a paste of `type` would insert, or nil to leave it to AppKit.
    /// ⚠️ A plain view still READS rich text, RTFD and HTML (measured `readablePasteboardTypes`),
    /// so those are reduced to their plain text here and escaped like any other paste (R1).
    /// ⛔ Left to AppKit: colours, fonts, rulers — and URLs / filenames with no plain-text form.
    static func plainText(from pboard: NSPasteboard, type: NSPasteboard.PasteboardType) -> String? {
        if isPlainTextType(type) || pboard.string(forType: .string) != nil {
            return pboard.string(forType: .string)
        }
        let rich: [NSPasteboard.PasteboardType: NSAttributedString.DocumentType] = [
            .rtf: .rtf, .init("NeXT Rich Text Format v1.0 pasteboard type"): .rtf,
            .rtfd: .rtfd, .init("NeXT RTFD pasteboard type"): .rtfd,
            .html: .html, .init("Apple HTML pasteboard type"): .html,
        ]
        guard let docType = rich[type], let data = pboard.data(forType: type) else { return nil }
        return (try? NSAttributedString(data: data, options: [.documentType: docType],
                                        documentAttributes: nil))?.string
    }

    /// ✅ Paste and drop (measured: both route here, NOT through `insertText`). R1: text from
    /// anywhere else is escaped like typing; Scrivi's own copy goes back exactly as stored.
    override func readSelection(from pboard: NSPasteboard, type: NSPasteboard.PasteboardType) -> Bool {
        guard let text = Self.plainText(from: pboard, type: type) else {
            return super.readSelection(from: pboard, type: type)
        }
        let range = rangeForUserTextChange
        guard range.location != NSNotFound else { return false }
        if let own = ownCopy, own.pasteboard == pboard.name, own.changeCount == pboard.changeCount {
            // ✅ [SP-163] live pass (user, 2026-10-05): Scrivi's own copy keeps ITS formatting where it lands — plain
            // text pasted inside bold stays plain (the bold closes around it); bold into bold still merges.
            pastingOwnCopy = true
            defer { pastingOwnCopy = false }
            insertVerbatim(own.source, replacementRange: range)
        } else {
            insertText(text, replacementRange: range)
        }
        return true
    }

    // MARK: — Custom undo/redo routing (EP-019 T-0205, T-0199-validated mechanism)
    //
    // allowsUndo is false and there is NO undoManager override. The Edit ▸ Undo /
    // Redo menu items send undo:/redo: down the responder chain; as first
    // responder this text view implements them and delegates to the Coordinator's
    // custom HistoryService-backed apply path. validateUserInterfaceItem drives
    // the enable state (and could set the menu titles). ⌘Z/⇧⌘Z arrive as these
    // same actions via the SwiftUI menu item's key equivalents.

    // ✅ T-0572 — THE CARET NEVER RESTS BETWEEN SCENES: not on a chapter heading (user
    // request 2026-10-01: *"it would be better if the cursor just skipped the chapter lines
    // entirely"*), and not after a divider (2026-10-02: *"skip the scene dividers too"*).
    // Every caret placement — arrows, clicks, programmatic — arrives here, so this is the
    // one place to enforce it. A selection with LENGTH is left alone: selecting across
    // scenes (fragment cut/copy) must still span headings.
    // ⚠️ Not while `stillSelecting` (a mouse drag in progress) — only the settled caret.
    override func setSelectedRanges(_ ranges: [NSValue], affinity: NSSelectionAffinity,
                                    stillSelecting: Bool) {
        var ranges = ranges
        if !stillSelecting, ranges.count == 1, let r = ranges.first?.rangeValue, r.length == 0,
           let target = coordinator?.caretOutsideSceneGap(r.location, from: selectedRange().location) {
            ranges = [NSValue(range: NSRange(location: target, length: 0))]
        }
        // ✅ EP-045 AC3 / R3 = (c), generalised by EP-046 AC5: nor inside a STOP RUN — an escape
        // backslash, an emphasis marker or a heading prefix. ✅ [SP-162] Q1: at the start of a span or a
        // heading the caret goes AFTER the marker, so the revealed hint shows to its LEFT; at the end of
        // a span it stays before the closer. A selection's ends are snapped too, so cut/copy never
        // splits `\` from its mark or a marker in two.
        if !stillSelecting, ranges.count == 1, let r = ranges.first?.rangeValue, let storage = textStorage,
           let presenter {
            // ✅ EP-045 AC10a: the snap's own cost, separate from `[SCRIVI-NAV] setSel`.
            let t0 = Date()
            defer {
                let ms = Date().timeIntervalSince(t0) * 1000
                if ms > 0.5 { NSLog(String(format: "[SCRIVI-EDIT] caretSnap=%.1f ms", ms)) }
            }
            let runAt = Self.runLookup(presenter, storage)
            if r.length == 0,
               let target = MarkdownEscapes.snapCaret(r.location, from: selectedRange().location,
                                                      length: storage.length, runAt: runAt) {
                ranges = [NSValue(range: NSRange(location: target, length: 0))]
            } else if r.length > 0,
                      let snapped = MarkdownEscapes.snapSelection(r, previousEnd: NSMaxRange(selectedRange()),
                                                                  in: storage.string as NSString, runAt: runAt) {
                ranges = [NSValue(range: snapped)]
            }
        }
        super.setSelectedRanges(ranges, affinity: affinity, stillSelecting: stillSelecting)
        // ✅ EP-046 AC4 (Q-E2-1): the heading prefixes on the selection's lines, and the markers of the
        // span it is in, show.
        // ⚠️ Not mid-drag — a reveal reflows the line, which must not happen under the pointer.
        // ✅ [SP-163] Q1: the caret LEFT an empty pending pair — remove it (nothing was recorded, nothing to undo).
        if !stillSelecting, !keepingPending, let p = presenter?.pending, selectedRange() != NSRange(location: p.caret, length: 0) {
            let sel = selectedRange()
            removePendingPair()
            let shift = sel.location >= NSMaxRange(p.range) ? p.range.length : (sel.location > p.range.location ? sel.location - p.range.location : 0)
            setSelectedRange(NSRange(location: sel.location - shift, length: sel.length))
            return
        }
        if !stillSelecting, let storage = textStorage { presenter?.reveal(for: selectedRange(), in: storage) }
    }

    // ⚠️ [I-0266] ⌘↑ lands where `Go to Manuscript Start` does — the first scene's first
    // character — not at offset 0, in front of Chapter 1's heading, where typing is refused.
    override func moveToBeginningOfDocument(_ sender: Any?) {
        guard let coordinator else { return super.moveToBeginningOfDocument(sender) }
        coordinator.moveToManuscriptBoundary(.start, in: self)
    }

    @objc func undo(_ sender: Any?) {
        coordinator?.performUndo()
    }

    @objc func redo(_ sender: Any?) {
        coordinator?.performRedo()
    }

    override func validateUserInterfaceItem(_ item: any NSValidatedUserInterfaceItem) -> Bool {
        switch item.action {
        case #selector(undo(_:)): return coordinator?.canUndo ?? false
        case #selector(redo(_:)): return coordinator?.canRedo ?? false
        default:                  return super.validateUserInterfaceItem(item)
        }
    }

    // Paste/cut commit as their own history events (§4.a triggers #3/#4): flush
    // any pending typing first, tag the next commit, let AppKit mutate text
    // (captured by textDidChange), then flush that insertion/removal as the
    // paste/cut event.
    // ⌘C: a cross-boundary selection copies as a structured fragment (flat plainText goes to the
    // system pasteboard for external apps; the fragment is held internally for ⌘V). A single-scene
    // selection uses the normal copy (AC5). EP-029 SP-089.
    override func copy(_ sender: Any?) {
        if coordinator?.structuredCopyIfCrossBoundary() == true { return }
        super.copy(sender)
    }

    override func paste(_ sender: Any?) {
        // A held structured fragment reconstructs at the caret (or refuses in a heading). Otherwise
        // the normal system-pasteboard paste runs (AC5). EP-029 SP-089.
        if coordinator?.hasInternalStructuredFragment == true,
           coordinator?.structuredPasteIfAvailable() == true { return }
        coordinator?.parent.session.historyCapture?.beginPasteOrCut(kind: "paste")
        super.paste(sender)
        coordinator?.parent.session.historyCapture?.flush(trigger: "paste", kind: "paste")
    }

    override func cut(_ sender: Any?) {
        // ✅ [I-0270] A cross-boundary cut = the cross-boundary COPY (unchanged) + delete the
        // selected text from each scene, KEEPING the scenes (user ruling 2026-10-02). ⛔ Not the
        // EP-029 collapse. A single-scene selection uses the normal cut (AC5).
        if let c = coordinator, c.selectionCrossesBoundary(selectedRange()) {
            let sel = selectedRange()
            copy(sender)
            c.deleteAcrossScenes(sel, kind: "cut")
            return
        }
        coordinator?.parent.session.historyCapture?.beginPasteOrCut(kind: "cut")
        super.cut(sender)
        coordinator?.parent.session.historyCapture?.flush(trigger: "cut", kind: "cut")
    }

    // MARK: — Copy-buffer text mutation (EP-019 SP-056, T-0214)
    //
    // The buffer paste/cut go through insertText/deleteBackward like ordinary
    // editing so textDidChange captures them, but WITHOUT going through the system
    // pasteboard (buffers are a parallel mechanism). The coordinator brackets these
    // with the history capture begin/flush, exactly as the native paste/cut do.

    // Inserts `text` at the caret (replacing any selection), routed through
    // insertText so it commits as a normal editable change. Used by pasteFromBuffer.
    func insertTextForBuffer(_ text: String) {
        // EP-045 AC4: a slot holds STORED text (`copyIntoBuffer` slices storage), so it goes
        // back verbatim — escaping it again would turn `\*` into `\\\*`.
        insertVerbatim(text, replacementRange: selectedRange())
    }

    // Deletes the current selection, routed through AppKit's delete so the removal
    // is captured by textDidChange. Used by cutIntoBuffer (the text is already saved
    // into the slot before this runs). No-op when there is no selection.
    func deleteSelectionForBuffer() {
        guard selectedRange().length > 0 else { return }
        deleteBackward(nil)
    }

    // [I-0273] The writer's first key, click or scroll ends the restore's re-centring.
    override func mouseDown(with event: NSEvent) {
        coordinator?.cancelRestoreCentre()
        super.mouseDown(with: event)
    }

    override func scrollWheel(with event: NSEvent) {
        coordinator?.cancelRestoreCentre()
        super.scrollWheel(with: event)
    }

    override func keyDown(with event: NSEvent) {
        coordinator?.cancelRestoreCentre()
        // T-0531 DIAGNOSTIC — the OUTERMOST boundary for a keystroke.
        // ⚠️ If this is slow while `[SCRIVI-KEY]` (textDidChange) is fast, the cost is in
        // AppKit's own edit/layout/display work, NOT in Scrivi's delegate.
        let __t0 = Date()
        defer {
            let ms = Date().timeIntervalSince(__t0) * 1000
            if ms > 0.5 {
                NSLog(String(format: "[SCRIVI-EDIT] keyDown(%@ code=%d)=%.1f ms",
                             (event.charactersIgnoringModifiers ?? "?") as NSString,
                             Int(event.keyCode), ms))
            }
        }
        let isReturn    = event.keyCode == 36  // kVK_Return
        let isDelete    = event.keyCode == 51  // kVK_Delete (backspace)
        let cmd         = event.modifierFlags.contains(.command)
        let shift       = event.modifierFlags.contains(.shift)

        if isReturn && cmd && shift {
            coordinator?.handleCreateChapter()
            return
        }
        if isReturn && cmd && !shift {
            coordinator?.handleCreateScene()
            return
        }
        if isDelete && cmd && shift {
            coordinator?.handleMergeChapter()
            return
        }
        if isDelete && cmd && !shift {
            coordinator?.handleMergeScene()
            return
        }

        // Copy-buffer chords (EP-019 SP-056, T-0214). Three EXPLICIT chords over slots
        // 1–9 — one action each, so behaviour never depends on whether text happens to
        // be selected (the earlier context-sensitive ⌘N was ambiguous and couldn't
        // paste-over-selection). The palette's modifier-sensitive button mirrors these.
        //   ⌘1–9  → copy selection into slot N (copy-only, no history event, Trade T3)
        //   ⌃1–9  → paste slot N at the caret, replacing any selection (a `paste` event)
        //   ⌥1–9  → cut selection into slot N (a `cut` history event, bufferID-tagged)
        // ⇧⌘digit is deliberately NOT used (⇧⌘3/4/5 are the system screenshot chords).
        // ⌘C/⌘V (buffer 0 = system pasteboard) are untouched. Each branch requires its
        // one modifier and none of the other two, so the chords never overlap.
        let ctrl = event.modifierFlags.contains(.control)
        let opt  = event.modifierFlags.contains(.option)
        if let digit = event.charactersIgnoringModifiers,
           digit.count == 1, let scalar = digit.unicodeScalars.first,
           scalar >= "1", scalar <= "9" {
            let bufferID = String(scalar)
            if cmd && !ctrl && !opt && !shift {
                coordinator?.copyIntoBuffer(bufferID)      // ⌘N — copy
                return
            }
            if ctrl && !cmd && !opt && !shift {
                coordinator?.pasteFromBuffer(bufferID)     // ⌃N — paste
                return
            }
            if opt && !cmd && !ctrl && !shift {
                coordinator?.cutIntoBuffer(bufferID)       // ⌥N — cut
                return
            }
        }

        super.keyDown(with: event)
    }

    // ⚠️ [I-0269] Both delete overrides guard the ONE character a bare caret would remove.
    // With a SELECTION, the selection is what is deleted — so a selection starting at a
    // scene's first character was refused because the character BEFORE it is the divider's
    // `\n`. ✅ A selection across scenes → `deleteAcrossScenes` ([I-0270], text removed
    // from each scene, scenes kept); within one scene → super.
    override func deleteBackward(_ sender: Any?) {
        guard let storage = textStorage else { super.deleteBackward(sender); return }
        if selectedRange().length > 0 {
            if coordinator?.deleteAcrossScenes(selectedRange(), kind: "delete") == true { return }
            super.deleteBackward(sender); return
        }
        let loc = selectedRange().location
        // ✅ [SP-163] Q1: ⌫ inside an empty pending pair takes the pair away.
        if let p = presenter?.pending, loc == p.caret { removePendingPair(); setSelectedRange(NSRange(location: p.range.location, length: 0)); return }
        guard loc > 0 else { return }
        // ✅ EP-046: markers are ATOMIC — ⌫ never deletes part of one. Just after an opening marker (the
        // start of a bold word, [SP-162] Q1) ⌫ acts on what the writer SEES before the caret: the
        // character before the marker, or the paragraph join. ⚠️ Just after a heading's `## ` (the
        // visible start of the heading) ⌫ removes the WHOLE prefix — the line becomes body text.
        if let presenter, let stop = presenter.stopTest(in: storage)(loc - 1), NSMaxRange(stop.range) == loc {
            switch stop.kind {
            case .prefix, .listPrefix:
                // ✅ [SP-163]: a list item's `- ` / `1. ` goes the same way — the item becomes body text.
                insertVerbatim("", replacementRange: stop.range)
                return
            case .opener:
                let s = stop.range.location
                guard s > 0, !isSeparatorPosition(s - 1, in: storage) else { return }
                if let join = paragraphJoin(at: s, in: storage.string as NSString) {
                    insertVerbatim(" ", replacementRange: join)
                } else {
                    insertVerbatim("", replacementRange: NSRange(location: s - 1, length: 1))
                }
                return
            case .escape, .closer:
                break
            }
        }
        // The character that would be deleted is at loc-1.
        let target = loc - 1
        if isSeparatorPosition(target, in: storage) { return }
        if let join = paragraphJoin(at: loc, in: storage.string as NSString) {
            insertVerbatim(" ", replacementRange: join)
            return
        }
        super.deleteBackward(sender)
    }

    /// ✅ EP-045 Q3 (ruled 2026-10-04): ⌫ at the start of a paragraph JOINS it to the one above
    /// with ONE space — `a.⏎⏎b.` → `a. b.`. ⚠️ §3A.6 ruled "delete one `\n`" on the premise that
    /// a soft break RENDERS as a space; ⛔ E1 renders nothing, so it showed as a line break and
    /// the paragraphs never visibly joined (live check 2026-10-04). ✅ A space is what [EP-046]
    /// would render for that soft break anyway, so nothing needs reverting later.
    /// ✅ Q2 is subsumed: the trailing-space run becomes exactly ONE space and no newline is
    /// left, so no hard break can form. Returns the range to replace with `" "`, or nil to let
    /// ⌫ delete one character:
    /// - ⛔ the line above ends in a BARE `\` — the writer's deliberate hard break (§4B.6): one
    ///   `\n` goes, `end\⏎next` remains, and the styler hides the `\`;
    /// - the paragraph above or the one being joined is EMPTY — ⌫ just removes a blank line.
    private func paragraphJoin(at loc: Int, in ns: NSString) -> NSRange? {
        let nl = loc - 2   // the first newline of the `\n\n` before the caret
        guard nl > 0, ns.character(at: loc - 1) == 0x0A, ns.character(at: nl) == 0x0A else { return nil }
        guard loc < ns.length, ns.character(at: loc) != 0x0A else { return nil }
        var start = nl
        while start > 0, ns.character(at: start - 1) == 0x20 { start -= 1 }
        guard start > 0, ns.character(at: start - 1) != 0x0A else { return nil }
        if start == nl {
            var run = 0
            while run < start, ns.character(at: start - 1 - run) == 0x5C { run += 1 }
            if run % 2 == 1 { return nil }
        }
        return NSRange(location: start, length: loc - start)
    }

    override func deleteForward(_ sender: Any?) {
        guard let storage = textStorage else { super.deleteForward(sender); return }
        if selectedRange().length > 0 {
            if coordinator?.deleteAcrossScenes(selectedRange(), kind: "delete") == true { return }
            super.deleteForward(sender); return
        }
        let loc = selectedRange().location
        if let p = presenter?.pending, loc == p.caret { removePendingPair(); setSelectedRange(NSRange(location: p.range.location, length: 0)); return }
        guard loc < storage.length else { return }
        // ✅ EP-046: just before a CLOSING marker (the end of a bold word) ⌦ deletes what the writer SEES
        // after the caret — the character after the marker — never the marker itself.
        if let presenter, let stop = presenter.stopTest(in: storage)(loc), stop.kind == .closer,
           stop.range.location == loc {
            let after = NSMaxRange(stop.range)
            guard after < storage.length, !isSeparatorPosition(after, in: storage) else { return }
            insertVerbatim("", replacementRange: NSRange(location: after, length: 1))
            return
        }
        // The character that would be deleted is at loc.
        if isSeparatorPosition(loc, in: storage) { return }
        super.deleteForward(sender)
    }

    // Returns true if the character at `pos` is part of a separator (attachment or its \n).
    private func isSeparatorPosition(_ pos: Int, in storage: NSTextStorage) -> Bool {
        guard pos >= 0, pos < storage.length else { return false }
        // The divider character itself (EP-045 AC1: by `.scriviDivider`, not `.attachment`).
        if SceneDivider.isDivider(in: storage, at: pos) { return true }
        // The \n that immediately follows a divider.
        if (storage.string as NSString).character(at: pos) == 10,
           pos >= 1,
           SceneDivider.isDivider(in: storage, at: pos - 1) { return true }
        return false
    }

    private var coordinator: ManuscriptTextView.Coordinator? {
        delegate as? ManuscriptTextView.Coordinator
    }
}

// MARK: — Divider attachment cell

// Renders a 1pt horizontal rule across the full text column.
// No text, no label — purely visual separation.
private let dividerCellHeight: CGFloat = 24

// MARK: — EP-045 AC1: the ONE definition of "scene divider"

/// ✅ Every question "is this a scene divider?" is answered here, from the `.scriviDivider` key.
/// ⛔ Never from `.attachment` — that matches ANY attachment, and `sceneBoundaries` (computed
/// here) is what the save path slices scene bytes with.
enum SceneDivider {

    /// The divider as it goes into storage: the attachment character carrying the
    /// `.scriviDivider` KEY, then a "\n". ✅ The ONLY place a divider is built, so the key
    /// cannot be forgotten. ⚠️ The key is on the attachment character ALONE — the "\n" keeps
    /// the body attributes, so text typed after it never inherits the key.
    static func string(_ attachment: NSTextAttachment, state: DividerRenderState,
                       newlineAttributes: [NSAttributedString.Key: Any]) -> NSAttributedString {
        let s = NSMutableAttributedString(attachment: attachment)
        s.addAttribute(.scriviDivider, value: state, range: NSRange(location: 0, length: s.length))
        s.append(NSAttributedString(string: "\n", attributes: newlineAttributes))
        return s
    }

    /// True when the character at `i` is a scene divider.
    static func isDivider(in storage: NSAttributedString, at i: Int) -> Bool {
        i >= 0 && i < storage.length
            && storage.attribute(.scriviDivider, at: i, effectiveRange: nil) != nil
    }

    /// Scene ranges in storage: the text between dividers, skipping chapter-heading runs.
    /// Pure — reads only `storage` — so the save path's slicing can be tested directly.
    /// `nil` for empty storage (the caller keeps its previous boundaries).
    static func sceneBoundaries(in storage: NSAttributedString) -> [NSRange]? {
        let fullLen = storage.length
        guard fullLen > 0 else { return nil }
        let whole = NSRange(location: 0, length: fullLen)

        // Scene divider positions, found by RUN, not by character. ✅ EP-045 AC1: by the
        // `.scriviDivider` KEY — ⛔ not `.attachment`, which any attachment would match.
        var dividers: [Int] = []
        storage.enumerateAttribute(.scriviDivider, in: whole, options: []) { value, range, _ in
            if value != nil { dividers.append(range.location) }
        }

        // Ranges carrying chapter-heading text, so a segment can start AFTER one.
        // ⚠️ `skipHeading`'s job, expressed once up front instead of per position.
        var headings: [NSRange] = []
        storage.enumerateAttribute(.scriviHeading, in: whole, options: []) { value, range, _ in
            if value != nil { headings.append(range) }
        }

        // First position at or after `pos` that is not inside a heading run.
        func skipHeading(from pos: Int) -> Int {
            var p = pos
            var moved = true
            while moved {
                moved = false
                for h in headings where h.location <= p && p < h.location + h.length {
                    p = h.location + h.length
                    moved = true
                }
            }
            return min(p, fullLen)
        }

        var newBoundaries: [NSRange] = []
        newBoundaries.reserveCapacity(dividers.count + 1)
        var segStart = skipHeading(from: 0)

        for divider in dividers {
            // ⚠️ A divider inside the leading heading run cannot close a segment.
            guard divider >= segStart else { continue }
            newBoundaries.append(NSRange(location: segStart, length: divider - segStart))
            // Skip the attachment + its trailing newline, then any heading after it.
            segStart = skipHeading(from: divider + 2)
        }
        // Last (or only) segment runs to end of storage.
        newBoundaries.append(NSRange(location: segStart,
                                     length: max(0, fullLen - segStart)))

        return newBoundaries
    }
}


// MARK: — [I-0279]: the held cross-scene fragment

/// Scrivi's cross-scene copy (a structured fragment ScriviCore can paste back as scenes), valid only while the
/// system pasteboard still holds what that copy put there. ⛔ It was held forever (SP-089), so after one cross-scene
/// copy every ⌘V pasted it — whatever had been copied since ([I-0279], found in [SP-163]'s live pass).
struct StructuredClipboard {
    private var held: (fragment: FragmentResult, changeCount: Int)?

    mutating func hold(_ fragment: FragmentResult?, changeCount: Int) {
        held = fragment.map { ($0, changeCount) }
    }

    /// The fragment, or nil once anything else has been copied (the change count moved on).
    func fragment(currentChangeCount: Int) -> FragmentResult? {
        guard let held, held.changeCount == currentChangeCount else { return nil }
        return held.fragment
    }
}

// MARK: — EP-045 AC11: the scene-boundary table, MAINTAINED (T-0583)

/// `sceneBoundaries`, kept current from each edit instead of rescanned over the whole manuscript.
/// ⛔ It used to be REBUILT by `SceneDivider.sceneBoundaries` (two whole-storage attribute scans)
/// on every keystroke: measured 2.3–3.9 ms on 1.85 MB (SP-157 AC10).
/// ✅ The shape Linux already ships (`SceneDocument::applyContentsChange`): an edit inside scene
/// `i` changes that scene's length and shifts every LATER scene by the edit's `changeInLength`.
/// ⚠️ Anything it cannot prove local — an edit spanning scenes, or text that carries a divider or
/// heading — marks the table DIRTY, and the next reader rescans. ⛔ ONE authority ([I-0131] was a
/// second scene-offset table drifting from the real one): readers take `ranges(in:)`, nothing else.
/// ⚠️ It observes a NOTIFICATION, not the storage delegate (the presenter's slot), and it needs
/// `willProcessEditing` anyway — see below.
/// ⛔ MEASURED 2026-10-04: by `didProcessEditing`, `editedRange` has been WIDENED to whole lines
/// (cause not traced), and a scene's LAST line holds its divider character — so every edit in a
/// scene's last line looked cross-scene and forced a rescan. ✅ `willProcessEditing` carries the
/// raw edit (the AC11 tests fail with `did`, pass with `will`).
final class SceneBoundaryTable: NSObject {
    private var table: [NSRange] = []
    private(set) var isDirty = true
    private weak var storage: NSTextStorage?

    func observe(_ storage: NSTextStorage) {
        if let old = self.storage {
            NotificationCenter.default.removeObserver(self, name: NSTextStorage.willProcessEditingNotification, object: old)
        }
        self.storage = storage
        isDirty = true
        NotificationCenter.default.addObserver(self, selector: #selector(storageWillProcessEditing(_:)),
                                               name: NSTextStorage.willProcessEditingNotification, object: storage)
    }

    /// The scene ranges, rescanned only when the table cannot be trusted.
    func ranges(in storage: NSAttributedString) -> [NSRange] {
        if isDirty, let scanned = SceneDivider.sceneBoundaries(in: storage) {
            table = scanned
            isDirty = false
        }
        return table
    }

    /// Install ranges already known to be exact (`rebuildStorage` builds them as it appends).
    func reset(_ ranges: [NSRange]) { table = ranges; isDirty = false }

    func invalidate() { isDirty = true }

    @objc private func storageWillProcessEditing(_ note: Notification) {
        guard let ts = note.object as? NSTextStorage, ts.editedMask.contains(.editedCharacters) else { return }
        apply(editedRange: ts.editedRange, changeInLength: ts.changeInLength, in: ts)
    }

    /// One processed edit. `editedRange` is in POST-edit coordinates.
    func apply(editedRange: NSRange, changeInLength delta: Int, in storage: NSAttributedString) {
        guard !isDirty, !table.isEmpty, editedRange.location != NSNotFound else { return }
        let start = editedRange.location
        let oldEnd = start + editedRange.length - delta
        // The last scene starting at or before the edit (binary search; scenes are ordered).
        var lo = 0, hi = table.count - 1
        while lo < hi {
            let mid = (lo + hi + 1) / 2
            if table[mid].location <= start { lo = mid } else { hi = mid - 1 }
        }
        let scene = table[lo]
        guard scene.location <= start, oldEnd <= scene.location + scene.length, oldEnd >= start else {
            isDirty = true; return
        }
        // Inserted text that brings structure (a divider or heading) changes the scene list.
        if editedRange.length > 0 {
            var structural = false
            for key in [NSAttributedString.Key.scriviDivider, .scriviHeading] where !structural {
                storage.enumerateAttribute(key, in: editedRange, options: []) { value, _, stop in
                    if value != nil { structural = true; stop.pointee = true }
                }
            }
            if structural { isDirty = true; return }
        }
        table[lo].length += delta
        for j in (lo + 1)..<table.count { table[j].location += delta }
    }
}

// Renders a 1pt horizontal rule across the full text column — the scene divider.
// No text, no label; purely visual separation.
//
// ⚠️ EP-039 T-0526 — REPLACES `DividerAttachmentCell: NSTextAttachmentCell`.
// ⚠️ `NSTextAttachmentCell` is TextKit 1 AND APPKIT-ONLY: there is no UIKit counterpart, so
// it blocked the iOS port on its own, before any performance question.
// ✅ `attachmentBounds(for:location:textContainer:proposedLineFragment:position:)` and
// `image(forBounds:attributes:location:textContainer:)` are `macos(12.0), ios(15.0)` — the
// SAME API on both platforms, so this class ports with only its drawing primitives changed.
//
// ✅ EP-045 AC1: dividers are found by the `.scriviDivider` KEY, whose value is this class's
// `renderState` — never by `.attachment`. ONE class; its rendering states live in the enum.
private final class DividerTextAttachment: NSTextAttachment {

    // ✅ T-0575 (user request 2026-10-02): with chapter titles OFF nothing showed where a
    // chapter ended — *"slightly dim the current Scene separator and use the full color one
    // for the last scene in the Chapter."* ⚠️ Done by RAISING the chapter-end rule to
    // `labelColor`, NOT by dimming the scene rule: `secondaryLabelColor` (5.89:1) is the
    // measured visibility floor for a 1 px rule ([I-0252]; `tertiaryLabelColor` at 2.26:1
    // was rejected as too faint), so the relative difference is made above it.
    // ✅ Drawn the same with titles ON — what a divider means does not change with the toggle.
    var renderState: DividerRenderState = .sceneBreak

    override func attachmentBounds(
        for attributes: [NSAttributedString.Key: Any],
        location: any NSTextLocation,
        textContainer: NSTextContainer?,
        proposedLineFragment: CGRect,
        position: CGPoint
    ) -> CGRect {
        // ⚠️ Full column width, as the cell did. `proposedLineFragment` is the authority
        // under TextKit 2; the container may not be sized yet on first layout.
        let width = proposedLineFragment.width > 0
            ? proposedLineFragment.width
            : (textContainer?.size.width ?? 0)
        return CGRect(x: 0, y: 0, width: width, height: dividerCellHeight)
    }

    override func image(
        for bounds: CGRect,
        attributes: [NSAttributedString.Key: Any],
        location: any NSTextLocation,
        textContainer: NSTextContainer?
    ) -> NSImage? {
        // ⚠️ [I-0112]: `NSColor.separatorColor` is semantic and must be resolved against the
        // right appearance. ✅ An `NSImage` drawn with a handler resolves against the CURRENT
        // appearance at draw time, which the text view sets — so Light/Dark tracks correctly
        // without the cell's `controlView` dance.
        //
        // ✅ [I-0252] (T-0554, 2026-09-28): THAT COMMENT IS CORRECT AND WAS VERIFIED BY
        // MEASUREMENT — a harness rasterising this exact handler under each appearance got
        // `rgba(1,1,1,0.047)` under Dark and `rgba(0,0,0,0.047)` under Light. ⛔ The appearance
        // was NEVER the bug.
        //
        // ⛔ THE BUG WAS THE COLOUR. `separatorColor` is `white @ 9.8% alpha` in Dark, which
        // COMPOSITES over the editor background to a contrast ratio of **1.34 : 1** — against
        // `16.67 : 1` for body text. It is a chrome hairline meant to divide CONTROLS, and it
        // is invisible as a content mark inside a writing surface. ⚠️ It measures `1.25 : 1`
        // in Light too, so this was never a Dark Mode defect — Dark is only where it was
        // noticed.
        //
        // ✅ `secondaryLabelColor` measures `5.89 : 1` (Dark) and `3.95 : 1` (Light): visible
        // as a structural mark without competing with prose. ⚠️ `tertiaryLabelColor` was
        // measured too and rejected at `2.26 : 1` — still too faint for a 1 px rule.
        //
        // ⚠️ THIS IS A VISIBILITY FIX, NOT THE FINAL DESIGN. The scene break is to become a
        // typeset `* * *` mark (the writer must be able to SEE that the caret is at a scene
        // boundary, because operations depend on it) — see
        // `docs/Scrivi_Manuscript_Rendering_Trade_Study_v0_1.md` §1.5. ⛔ Do not elaborate
        // this drawing code in the meantime; it is scheduled to be replaced.
        let size = NSSize(width: max(bounds.width, 1), height: max(bounds.height, 1))
        return NSImage(size: size, flipped: false) { rect in
            // ⚠️ [I-0252] SECOND DEFECT, also measured: a 1 pt line centred on an INTEGRAL
            // y straddles the pixel grid, so antialiasing splits it across TWO rows at
            // HALF alpha each — measured `0.275 / 0.275` instead of one row at `0.549`.
            // ⛔ That halved the line's contrast on its own, independently of the colour.
            // ✅ Offsetting by half a point puts the 1 pt stroke inside ONE pixel row:
            // measured `0.549` on a single row.
            let lineY = rect.midY.rounded() + 0.5
            let path = NSBezierPath()
            path.lineWidth = 1.0
            path.move(to: NSPoint(x: rect.minX + 20, y: lineY))
            path.line(to: NSPoint(x: rect.maxX - 20, y: lineY))
            // T-0575 — first pass used `labelColor`; ⛔ user: *"The dividers are too subtle.
            // Perhaps a slight tint to the chapter divider?"* ✅ The ACCENT colour: a different
            // KIND of mark, not merely a darker line, and it follows the writer's own
            // System Settings accent.
            // ⚠️ 3rd pass — user: *"The tinted divider may be too much."* ✅ Softened to 60%.
            (self.renderState == .chapterEnd ? NSColor.controlAccentColor.withAlphaComponent(0.6)
                              : NSColor.secondaryLabelColor).setStroke()
            path.stroke()
            return true
        }
    }
}

#else

// iOS / visionOS stub — full UITextView implementation is a future task.
struct ManuscriptTextView: View {
    var loader: ViewportSceneLoader
    var env: AppEnvironment
    var session: ProjectSession
    @Binding var navigateToSceneID: String?
    var showChapterTitles: Bool
    var typography: ManuscriptTypography   // EP-047: the macOS surface draws with it; not used here yet
    var markupHints: Bool = true           // EP-047 S4: likewise

    var body: some View {
        Text("Manuscript editor not yet available on this platform.")
            .foregroundStyle(.secondary)
            .frame(maxWidth: .infinity, maxHeight: .infinity)
    }

    func takeFocus() {}
}

#endif
