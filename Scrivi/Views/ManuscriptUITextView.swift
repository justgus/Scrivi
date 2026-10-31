#if !os(macOS)
import SwiftUI
import UIKit

// EP-055 S1 (T-0634) — THE MANUSCRIPT ON iOS: a TextKit 2 `UITextView` host.
// Design: `docs/Scrivi_iOS_Manuscript_Design_v0_1.md` (Q1 = (c), ruled 2026-10-30).
//
// ✅ It owns NOTHING the Mac decides: the render, the model sync and autosave are `ManuscriptSceneSync`; what an edit stores
// is `ManuscriptEditRules`; the look is `ManuscriptPresenter` + `ManuscriptTypography`. This file only applies their answers
// to a `UITextView` (design §3 D).
// ⛔ Nothing device- or keyboard-specific ([SP-198] R3): the system's own keyboard handling and layout.
// ⚠️ Not here yet (EP-055 S2/S3): structural edits, history undo/redo, cross-scene cut/copy/paste, buffers, Find, Format.

struct ManuscriptTextView: UIViewRepresentable {

    var loader: ViewportSceneLoader
    var env: AppEnvironment
    var session: ProjectSession
    @Binding var navigateToSceneID: String?
    var showChapterTitles: Bool
    var typography: ManuscriptTypography
    var markupHints: Bool = true

    func makeCoordinator() -> Coordinator { Coordinator(self) }

    func makeUIView(context: Context) -> ManuscriptUITextView {
        // ⚠️ TextKit 2. ⛔ Never read `layoutManager`: it downgrades the view to TextKit 1 (as on the Mac, T-0531).
        let textView = ManuscriptUITextView()
        let sync = context.coordinator.sync
        textView.isEditable = true
        textView.allowsEditingTextAttributes = false
        // ⛔ [I-0283]: smart quotes rewrite stored escapes — off, as on the Mac.
        textView.smartQuotesType = .no
        textView.alwaysBounceVertical = true
        textView.textContainerInset = UIEdgeInsets(top: 24, left: 20, bottom: 24, right: 20)
        textView.backgroundColor = .systemBackground
        sync.presenter.typography = typography
        sync.presenter.setHints(markupHints, selection: textView.selectedRange, in: textView.textStorage)
        textView.typingAttributes = typography.bodyAttributes
        // ✅ EP-046: the presenter styles each paragraph as TextKit 2 lays it out; storage stays plain Markdown.
        (textView.textLayoutManager?.textContentManager as? NSTextContentStorage)?.delegate = sync.presenter
        textView.textStorage.delegate = sync.presenter
        sync.boundaryTable.observe(textView.textStorage)
        textView.delegate = context.coordinator
        context.coordinator.textView = textView
        loader.takeFocusHandler = { [weak textView] in
            DispatchQueue.main.async { _ = textView?.becomeFirstResponder() }
        }
        return textView
    }

    func updateUIView(_ tv: ManuscriptUITextView, context: Context) {
        let coordinator = context.coordinator
        coordinator.parent = self
        let sync = coordinator.sync

        let typographyChanged = typography != sync.presenter.typography
        if typographyChanged { sync.presenter.typography = typography; tv.typingAttributes = typography.bodyAttributes }
        if markupHints != sync.presenter.hintsEnabled {
            sync.presenter.setHints(markupHints, selection: tv.selectedRange, in: tv.textStorage)
        }
        let segIDs = loader.segments.map(\.id)
        let fingerprint = coordinator.chapterHeadingFingerprint()
        if segIDs != coordinator.lastSegmentIDs || showChapterTitles != coordinator.lastShowChapterTitles
            || fingerprint != coordinator.lastChapterTitleFingerprint || typographyChanged {
            coordinator.rebuildStorage(tv)
        }
        if !coordinator.didRestoreSurface {
            coordinator.didRestoreSurface = true
            coordinator.restoreWritingSurface(in: tv)
        }
        if let targetID = navigateToSceneID {
            coordinator.navigateToScene(targetID, in: tv)
            // Reset after this update pass — mutating bound state inside an update is a view-update violation.
            DispatchQueue.main.async { navigateToSceneID = nil }
        }
    }

    // MARK: — Coordinator

    @MainActor
    final class Coordinator: NSObject, UITextViewDelegate {
        var parent: ManuscriptTextView {
            didSet {
                sync.loader = parent.loader
                sync.env = parent.env
                sync.session = parent.session
                sync.showChapterTitles = parent.showChapterTitles
            }
        }
        let sync: ManuscriptSceneSync
        weak var textView: ManuscriptUITextView?

        var lastSegmentIDs: [String] = []
        var lastShowChapterTitles = false
        var lastChapterTitleFingerprint = ""
        var didRestoreSurface = false
        private var lastDiagCaret = 0
        /// I-0131: while set, scroll-driven viewport retargeting is suppressed, so the scroll a navigation CAUSES cannot
        /// overwrite the scene the writer asked for.
        private var navigationLockUntil: Date?

        init(_ parent: ManuscriptTextView) {
            self.parent = parent
            sync = ManuscriptSceneSync(loader: parent.loader, env: parent.env, session: parent.session,
                                       showChapterTitles: parent.showChapterTitles)
        }

        /// The chapter titles as stored, so a rename (same scene IDs, new heading) rebuilds (I-0095).
        func chapterHeadingFingerprint() -> String {
            var seen = Set<String>()
            var parts: [String] = []
            for info in parent.loader.allScenes where seen.insert(info.chapterID).inserted {
                parts.append("\(info.chapterID)=\(info.chapterTitle)")
            }
            return parts.joined(separator: "\u{1f}")
        }

        /// The whole storage from the scene list — the shared render.
        func rebuildStorage(_ tv: ManuscriptUITextView) {
            caretDiag("[SCRIVI-CARET] rebuildStorage (caret was \(tv.selectedRange.location))")
            sync.isRebuilding = true
            defer { sync.isRebuilding = false }
            let (rendered, built) = sync.renderedManuscript(parent.loader.segments)
            tv.textStorage.setAttributedString(rendered)
            // The ranges were built exactly while appending — install them (EP-045 AC11).
            sync.boundaryTable.reset(built)
            lastSegmentIDs = parent.loader.segments.map(\.id)
            lastShowChapterTitles = parent.showChapterTitles
            lastChapterTitleFingerprint = chapterHeadingFingerprint()
        }

        // MARK: Place

        /// Resume where the writer left off (I-0058): the restored scene and its scene-local caret.
        func restoreWritingSurface(in tv: ManuscriptUITextView) {
            let loader = parent.loader
            guard let sceneID = loader.viewportSceneID,
                  let start = sceneStart(sceneID, in: tv) else { return }
            let sceneLen = loader.segments.first(where: { $0.sceneID == sceneID })?.text.utf16.count ?? 0
            let target = start + min(max(0, loader.restoredSelectionOffset ?? 0), sceneLen)
            loader.restoredSelectionOffset = nil
            place(target, in: tv)
            loader.setViewportScene(sceneID)
            // The writer can type at once, as on the Mac, whose window opens with the manuscript as first responder.
            loader.takeFocus()
        }

        /// A scene reached from elsewhere (the Navigator, a link): caret at its start, scrolled into view.
        func navigateToScene(_ sceneID: String, in tv: ManuscriptUITextView) {
            guard let segIdx = parent.loader.segments.firstIndex(where: { $0.sceneID == sceneID }),
                  let start = sceneStart(sceneID, in: tv) else { return }
            sync.scrollTask?.cancel()
            sync.highlightTask?.cancel()
            place(start, in: tv)
            parent.loader.setViewportScene(sceneID)
            parent.loader.setCurrentIndex(segIdx)
        }

        private func sceneStart(_ sceneID: String, in tv: ManuscriptUITextView) -> Int? {
            guard let segIdx = parent.loader.segments.firstIndex(where: { $0.sceneID == sceneID }) else { return nil }
            let boundaries = sync.sceneBoundaries(in: tv.textStorage)
            return boundaries.indices.contains(segIdx) ? boundaries[segIdx].location
                                                       : parent.loader.storageOffset(forSceneID: sceneID)
        }

        private func place(_ storageOffset: Int, in tv: ManuscriptUITextView) {
            navigationLockUntil = Date().addingTimeInterval(0.5)
            let range = NSRange(location: min(storageOffset, tv.textStorage.length), length: 0)
            tv.selectedRange = range
            // After this pass's layout, so the scroll lands on laid-out text.
            DispatchQueue.main.async { [weak tv] in tv?.centre(range.location) }
        }

        // MARK: UITextViewDelegate

        func textViewDidChange(_ textView: UITextView) {
            guard !sync.isRebuilding else { return }
            sync.textChanged(in: textView.textStorage, caret: textView.selectedRange.location)
        }

        func textViewDidChangeSelection(_ textView: UITextView) {
            // ⚠️ [SP-198 C] DIAGNOSTIC (Debug only; remove when the caret jump is found): every move, and the call stack of a
            // move of more than 200 characters that no keystroke made.
            let loc = textView.selectedRange.location
            let scene = sync.segmentIndex(for: loc, in: textView.textStorage) ?? -1
            var line = "[SCRIVI-CARET] sel \(lastDiagCaret) -> \(loc) len=\(textView.selectedRange.length) scene=\(scene) "
                + "firstResponder=\(textView.isFirstResponder) rebuilding=\(sync.isRebuilding) storage=\(textView.textStorage.length)"
            if abs(loc - lastDiagCaret) > 200, !((textView as? ManuscriptUITextView)?.diagTyping ?? false) {
                line += " stack=" + Thread.callStackSymbols.dropFirst(1).prefix(14).map { frame -> String in
                    let parts = frame.split(separator: " ", omittingEmptySubsequences: true)
                    return parts.count > 3 ? "\(parts[1]):\(parts[3])" : String(frame)
                }.joined(separator: " < ")
            }
            caretDiag(line)
            lastDiagCaret = loc
            guard !sync.isRebuilding else { return }
            // ✅ [I-0313]: at a line's start typing would take the divider's attributes — typed text is always body text.
            textView.typingAttributes = sync.presenter.typography.bodyAttributes
            sync.selectionChanged(in: textView.textStorage, caret: textView.selectedRange.location)
        }

        /// ✅ Every user edit passes the shared rules (headings, dividers, escape pairs, balanced emphasis) — the Mac's
        /// `shouldChangeText`.
        func textView(_ textView: UITextView, shouldChangeTextIn range: NSRange, replacementText text: String) -> Bool {
            guard let tv = textView as? ManuscriptUITextView else { return true }
            return tv.shouldChange(range, to: text)
        }

        // MARK: Scrolling — the viewport scene (Navigator highlight) and the scroll fraction (I-0058)

        func scrollViewDidScroll(_ scrollView: UIScrollView) {
            guard let tv = scrollView as? ManuscriptUITextView, !sync.isRebuilding else { return }
            let scrollable = max(0, scrollView.contentSize.height - scrollView.bounds.height)
            parent.loader.updateScrollFraction(scrollable > 0 ? Double(scrollView.contentOffset.y / scrollable) : 0)
            // The centre of the visible area, so the scene only changes once a boundary has clearly passed the middle.
            let centre = CGPoint(x: tv.textContainerInset.left + 1, y: scrollView.contentOffset.y + scrollView.bounds.height / 2)
            sync.scrollTask?.cancel()
            let loader = parent.loader
            sync.scrollTask = Task { @MainActor [weak self, weak tv] in
                try? await Task.sleep(nanoseconds: 120_000_000)   // 120 ms debounce
                guard !Task.isCancelled, let self, let tv else { return }
                if let until = self.navigationLockUntil {
                    if Date() < until { return }
                    self.navigationLockUntil = nil
                }
                guard let pos = tv.closestPosition(to: centre) else { return }
                let idx = tv.offset(from: tv.beginningOfDocument, to: pos)
                guard let segIdx = self.sync.segmentIndex(for: idx, in: tv.textStorage),
                      loader.segments.indices.contains(segIdx) else { return }
                loader.setViewportScene(loader.segments[segIdx].sceneID)
            }
        }
    }
}

// MARK: — The text view: applies `ManuscriptEditRules` to UIKit's text input

final class ManuscriptUITextView: UITextView {

    /// ⚠️ Measured 2026-10-30: made through `UITextView(usingTextLayoutManager:)`, this subclass's stored properties were
    /// never initialised — they read as nil. A designated init initialises them; a text view made with no container is
    /// TextKit 2 (the "iOS manuscript" tests assert it).
    init() { super.init(frame: .zero, textContainer: nil) }
    required init?(coder: NSCoder) { fatalError("ManuscriptUITextView is not loaded from a nib") }

    /// ✅ EP-046 E2-S1 — the view's presenter, found through TextKit 2's own delegate slot (as the Mac's is), so a view
    /// built without a coordinator (the test fixtures) works the same way.
    var presenter: ManuscriptPresenter? {
        (textLayoutManager?.textContentManager as? NSTextContentStorage)?.delegate as? ManuscriptPresenter
    }
    /// The pasteboard copy and paste use. ⚠️ Tests give it a private one: the simulator can share the Mac's clipboard.
    var pasteboard: UIPasteboard = .general
    private var bodyAttributes: [NSAttributedString.Key: Any] {
        (presenter?.typography ?? .default).bodyAttributes
    }

    /// True while an edit the presenter computed is applied — it must not be balanced again.
    private var applyingBalancedEdit = false
    /// True while Scrivi's OWN copy is pasted: it keeps its own formatting.
    private var pastingOwnCopy = false
    /// Scrivi's last copy: the pasteboard's change count then, and the balanced SOURCE it stood for.
    private var ownCopy: (changeCount: Int, source: String)?

    // ✅ [SP-198] Q-undo (ruled 2026-10-30): Scrivi's history owns undo, as on the Mac (`allowsUndo = false`). Until the
    // history is wired on iOS (S2) the system can undo NOTHING here, so shake and the keyboard's undo key do nothing rather
    // than undo behind the history's back. ⚠️ Measured 2026-10-30: `disableUndoRegistration()` is not enough — UITextView
    // still registered (canUndo became true), so the manager refuses instead.
    private final class InertUndoManager: UndoManager {
        override var canUndo: Bool { false }
        override var canRedo: Bool { false }
        override func undo() {}
        override func redo() {}
    }
    private let inertUndoManager: UndoManager = {
        let u = InertUndoManager()
        u.levelsOfUndo = 1   // what UITextView registers is never used; keep none of it
        return u
    }()
    override var undoManager: UndoManager? { inertUndoManager }

    /// Scrolls storage offset `loc` to the middle of the view — the Mac's `centerStorageOffset`. ⛔ `scrollRangeToVisible`
    /// alone moved the least distance (the scene at an edge), and after a long jump TextKit 2's viewport was left on
    /// estimated positions: a tap then hit-tested tens of thousands of characters away (SP-198 live pass: typing at the end).
    func centre(_ loc: Int) {
        let loc = min(loc, textStorage.length)
        scrollRangeToVisible(NSRange(location: loc, length: 0))
        textLayoutManager?.textViewportLayoutController.layoutViewport()
        guard let pos = position(from: beginningOfDocument, offset: loc) else { return }
        // Each layout replaces estimated heights near the viewport with real ones, which moves the target: re-measure until it
        // stays put (the Mac re-centres a restore the same way, `applyPendingRestoreCentre`).
        for _ in 0..<6 {
            let rect = caretRect(for: pos)
            let maxY = max(-adjustedContentInset.top, contentSize.height - bounds.height + adjustedContentInset.bottom)
            let y = min(max(rect.midY - bounds.height / 2, -adjustedContentInset.top), maxY)
            if abs(y - contentOffset.y) < 1 { break }
            setContentOffset(CGPoint(x: contentOffset.x, y: y), animated: false)
            textLayoutManager?.textViewportLayoutController.layoutViewport()
        }
    }

    // MARK: The rules

    /// The delegate's gate for an edit UIKit is about to make.
    func shouldChange(_ range: NSRange, to replacement: String) -> Bool {
        switch ManuscriptEditRules.verdict(changing: range, with: replacement, in: textStorage, presenter: presenter,
                                           applyingBalancedEdit: applyingBalancedEdit, pastingOwnCopy: pastingOwnCopy) {
        case .allow: return true
        case .refuse: return false
        case let .reissue(r, s, caret, balanced):
            store(s, over: r, selecting: NSRange(location: caret, length: 0), balanced: balanced)
            return false
        }
    }

    /// Store `source` (already in stored form) over `range` — through the same rules, as the Mac's `insertVerbatim` goes
    /// through `shouldChangeText` — then tell the model sync, as a typed edit would.
    func store(_ source: String, over range: NSRange, selecting selection: NSRange, balanced: Bool = false) {
        let was = applyingBalancedEdit
        if balanced { applyingBalancedEdit = true }
        defer { applyingBalancedEdit = was }
        switch ManuscriptEditRules.verdict(changing: range, with: source, in: textStorage, presenter: presenter,
                                           applyingBalancedEdit: applyingBalancedEdit, pastingOwnCopy: pastingOwnCopy) {
        case .refuse:
            return
        case let .reissue(r, s, caret, b):
            store(s, over: r, selecting: NSRange(location: caret, length: 0), balanced: b)
        case .allow:
            textStorage.replaceCharacters(in: range, with: NSAttributedString(string: source, attributes: bodyAttributes))
            selectedRange = NSRange(location: min(selection.location, textStorage.length), length: selection.length)
            delegate?.textViewDidChange?(self)
        }
    }

    // MARK: Typing

    /// ✅ Every keystroke from either keyboard, and dictation, arrives here. Typed text is stored ESCAPED; Return stores a
    /// paragraph (or continues a list) — `ManuscriptEditRules`.
    /// ⚠️ Measured 2026-10-30: UIKit does NOT ask the delegate's `shouldChangeTextIn` for every `insertText` /
    /// `deleteBackward`, so these overrides pass the rules themselves before handing an edit to `super`.
    /// ⚠️ [SP-198 C] DIAGNOSTIC: true while a keystroke is applied, so its caret move is not reported as a jump.
    private(set) var diagTyping = false

    override func insertText(_ text: String) {
        caretDiag("[SCRIVI-CARET] insertText \(text.debugDescription) at \(selectedRange.location) len=\(selectedRange.length) "
                  + "marked=\(markedTextRange != nil)")
        diagTyping = true
        defer { diagTyping = false }
        guard markedTextRange == nil else { return super.insertText(text) }
        if text == "\n" {
            switch ManuscriptEditRules.returnEdit(in: textStorage, selection: selectedRange, presenter: presenter) {
            case let .command(edit):
                store(edit.replacement, over: edit.range, selecting: edit.selection, balanced: true)
            case let .paragraph(range, source):
                store(source, over: range, selecting: NSRange(location: range.location + source.utf16.count, length: 0))
            }
            return
        }
        let source = MarkdownEscapes.escape(text)
        guard shouldChange(selectedRange, to: source) else { return }
        super.insertText(source)
    }

    override func deleteBackward() {
        guard markedTextRange == nil else { return super.deleteBackward() }
        guard selectedRange.length == 0 else {
            if shouldChange(selectedRange, to: "") { super.deleteBackward() }
            return
        }
        let loc = selectedRange.location
        switch ManuscriptEditRules.deleteBackwardEdit(at: loc, in: textStorage, presenter: presenter) {
        case .system:
            // The character ⌫ removes: the composed sequence before the caret.
            let target = (textStorage.string as NSString).rangeOfComposedCharacterSequence(at: loc - 1)
            if shouldChange(target, to: "") { super.deleteBackward() }
        case .nothing: return
        case let .verbatim(range, source):
            store(source, over: range, selecting: NSRange(location: range.location + source.utf16.count, length: 0))
        }
    }

    // MARK: Copy and paste (within one scene; across scenes is EP-055 S3)

    /// ✅ EP-046 AC12 + [SP-162] Q2, as on the Mac: other apps get what the writer SEES; Scrivi's own paste gets the
    /// balanced SOURCE.
    override func copy(_ sender: Any?) {
        let sel = selectedRange
        guard sel.length > 0, !crossesScene(sel) else { return }
        let raw = (textStorage.string as NSString).substring(with: sel)
        let copy: (source: String, presented: String) = presenter?.balancedCopy(in: textStorage, sel)
            ?? (raw, MarkdownEscapes.map(raw).presented)
        pasteboard.string = copy.presented
        ownCopy = (pasteboard.changeCount, copy.source)
    }

    override func cut(_ sender: Any?) {
        let sel = selectedRange
        guard sel.length > 0, !crossesScene(sel) else { return }
        copy(sender)
        store("", over: sel, selecting: NSRange(location: sel.location, length: 0))
    }

    override func paste(_ sender: Any?) {
        guard let text = pasteboard.string else { return }
        if let own = ownCopy, own.changeCount == pasteboard.changeCount {
            pastingOwnCopy = true
            defer { pastingOwnCopy = false }
            store(own.source, over: selectedRange,
                  selecting: NSRange(location: selectedRange.location + own.source.utf16.count, length: 0))
        } else {
            let source = MarkdownEscapes.escape(text)
            store(source, over: selectedRange,
                  selecting: NSRange(location: selectedRange.location + source.utf16.count, length: 0))
        }
    }

    /// A selection that holds a divider spans scenes — a structural cut, not here yet (the rules would refuse its delete).
    private func crossesScene(_ sel: NSRange) -> Bool {
        var found = false
        textStorage.enumerateAttribute(.scriviDivider, in: sel, options: []) { value, _, stop in
            if value != nil { found = true; stop.pointee = true }
        }
        return found
    }
}

/// ⚠️ [SP-198 C] DIAGNOSTIC — Debug builds only.
private func caretDiag(_ message: @autoclosure () -> String) {
    #if DEBUG
    NSLog("%@", message())
    #endif
}
#endif
