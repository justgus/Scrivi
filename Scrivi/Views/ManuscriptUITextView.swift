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
// ✅ EP-055 S3a (SP-222): undo/redo, structural edits and the cross-scene clipboard are `ManuscriptStructure`, shared with the
// Mac; this host gives it the view (`ManuscriptStructureHost`, below).
// ⚠️ Not here yet (EP-055 S3b): buffers, Find, Format, the branch-point marks.

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
        // The toolbar's Navigate menu (SP-199): the Mac's six moves, the same shared arithmetic.
        let c = context.coordinator
        session.sceneStartAction      = { [weak c] in c?.move(.scene, .start) }
        session.sceneEndAction        = { [weak c] in c?.move(.scene, .end) }
        session.chapterStartAction    = { [weak c] in c?.move(.chapter, .start) }
        session.chapterEndAction      = { [weak c] in c?.move(.chapter, .end) }
        session.manuscriptStartAction = { [weak c] in c?.move(.manuscript, .start) }
        session.manuscriptEndAction   = { [weak c] in c?.move(.manuscript, .end) }
        // ✅ EP-055 S3a: the structural edits (the menu bar, the edit menu, the keys) and Import ▸ Into This Project ([T-0680]).
        textView.structure = c.structure
        session.createSceneAction   = { [weak c] in c?.structure.handleCreateScene() }
        session.createChapterAction = { [weak c] in c?.structure.handleCreateChapter() }
        session.mergeSceneAction    = { [weak c] in c?.structure.handleMergeScene() }
        session.mergeChapterAction  = { [weak c] in c?.structure.handleMergeChapter() }
        session.importRangeAction   = { [weak c] range in _ = c?.structure.pasteStructuredRange(range) }
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
        /// ✅ EP-055 S3a: undo/redo, structural edits and the cross-scene clipboard — the Mac's, shared.
        lazy var structure: ManuscriptStructure = {
            let s = ManuscriptStructure(sync: sync)
            s.host = self
            return s
        }()
        /// How many chapters the storage shows — a run render cannot renumber "Chapter N" titles outside its run.
        var renderedChapterCount = 0
        /// Tests answer these instead of the alerts: [I-0319] (true = undo), a barrier's notice ([I-0302]), the branch to
        /// redo (Q4; its eventID, nil = dismissed).
        var onEarlierSessionUndo: ((String) -> Bool)?
        var onBarrierNotice: ((String) -> Void)?
        var onRedoFork: ((HistoryForkAhead) -> String?)?

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
            renderedChapterCount = ManuscriptSceneSync.chapterCount(parent.loader.segments)
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

        func move(_ scope: ManuscriptSceneSync.NavigateScope, _ edge: ManuscriptEdge) {
            guard let tv = textView,
                  let target = sync.navigateTarget(scope, edge, caret: tv.selectedRange.location, in: tv.textStorage)
            else { return }
            place(target, in: tv)
            _ = tv.becomeFirstResponder()
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

        /// ✅ Q3 (SP-222): the structural edits in the text's own edit menu, where the caret allows them.
        func textView(_ textView: UITextView, editMenuForTextIn range: NSRange,
                      suggestedActions: [UIMenuElement]) -> UIMenu? {
            guard range.length == 0, !structure.caretInHeading(range.location) else { return nil }
            var items: [UIMenuElement] = [
                UIAction(title: "New Scene") { [weak self] _ in self?.structure.handleCreateScene() },
                UIAction(title: "New Chapter") { [weak self] _ in self?.structure.handleCreateChapter() },
            ]
            if structure.canMergeScene {
                items.append(UIAction(title: "Merge Scene with Previous") { [weak self] _ in self?.structure.handleMergeScene() })
            }
            if structure.canMergeChapter {
                items.append(UIAction(title: "Merge Chapter with Previous") { [weak self] _ in self?.structure.handleMergeChapter() })
            }
            return UIMenu(children: suggestedActions + [UIMenu(options: .displayInline, children: items)])
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

// MARK: — The iOS host of the shared structural edits (EP-055 S3a, `ManuscriptStructure`)

extension ManuscriptTextView.Coordinator: ManuscriptStructureHost {
    var structureStorage: NSTextStorage? { textView?.textStorage }
    var structureSelection: NSRange { textView?.selectedRange ?? NSRange(location: 0, length: 0) }
    /// ⚠️ Not `isEditable`: on iOS that resigns first responder and the keyboard goes. The view holds its input instead.
    func structureSetEditable(_ editable: Bool) { textView?.inputHeld = !editable }
    func structureRefuse() { UINotificationFeedbackGenerator().notificationOccurred(.error) }
    func structureFlashRefuse() { structureRefuse() }

    func structureRenderRun(old: Range<Int>, new: Range<Int>) {
        guard let tv = textView else { return }
        let segments = parent.loader.segments
        guard sync.runFits(in: tv.textStorage, old: old, new: new, renderedChapterCount: renderedChapterCount) else {
            return rebuildStorage(tv)
        }
        sync.isRebuilding = true
        defer { sync.isRebuilding = false }
        sync.spliceRun(in: tv.textStorage, old: old, new: new)
        renderedChapterCount = ManuscriptSceneSync.chapterCount(segments)
        lastSegmentIDs = segments.map(\.id)
        lastShowChapterTitles = parent.showChapterTitles
        lastChapterTitleFingerprint = chapterHeadingFingerprint()
    }

    func structureRevealCaret(_ loc: Int) { structurePlaceCaret(loc, reveal: true) }

    func structurePlaceCaret(_ loc: Int, reveal: Bool) {
        guard let tv = textView else { return }
        let range = NSRange(location: min(loc, tv.textStorage.length), length: 0)
        tv.selectedRange = range
        if reveal {
            tv.scrollRangeToVisible(range)
            tv.textLayoutManager?.textViewportLayoutController.layoutViewport()
        }
    }

    func structurePreservingScroll(_ body: () -> Void) {
        let offset = textView?.contentOffset
        body()
        if let offset { textView?.setContentOffset(offset, animated: false) }
    }

    func structureLanded() {}

    func structureConfirmEarlierSessionUndo(_ timestamp: String, _ answer: @escaping @MainActor (Bool) -> Void) {
        if let hook = onEarlierSessionUndo { return answer(hook(timestamp)) }
        let when = HistoryTimestamp.friendly(timestamp).map { "This undo changes text you wrote \($0)." }
            ?? "This undo changes text you wrote before this session."
        let alert = UIAlertController(title: "Undo a change from a previous session?", message: when, preferredStyle: .alert)
        alert.addAction(UIAlertAction(title: "Undo", style: .default) { _ in answer(true) })
        alert.addAction(UIAlertAction(title: "Cancel", style: .cancel) { _ in answer(false) })
        present(alert)
    }

    func structureBarrierNotice(_ message: String) {
        if let hook = onBarrierNotice { return hook(message) }
        let alert = UIAlertController(title: message, message: nil, preferredStyle: .alert)
        alert.addAction(UIAlertAction(title: "OK", style: .default))
        present(alert)
    }

    /// ✅ Q4 (SP-222): the system's choice — a popover at the caret on the iPad, a sheet on the iPhone.
    func structureChooseBranch(_ fork: HistoryForkAhead, _ choose: @escaping @MainActor (String) -> Void) {
        if let hook = onRedoFork {
            if let child = hook(fork) { choose(child) }
            return
        }
        guard let tv = textView, !fork.children.isEmpty else { return }
        let sheet = UIAlertController(title: "Redo which change?", message: nil, preferredStyle: .actionSheet)
        for child in fork.children {
            let preview = child.preview.isEmpty ? "(no preview)" : child.preview
            let when = HistoryTimestamp.friendly(child.timestamp).map { " — \($0)" } ?? ""
            sheet.addAction(UIAlertAction(title: preview + when, style: .default) { _ in choose(child.eventID) })
        }
        sheet.addAction(UIAlertAction(title: "Cancel", style: .cancel))
        if let pop = sheet.popoverPresentationController {
            pop.sourceView = tv
            if let at = tv.selectedTextRange?.start { pop.sourceRect = tv.caretRect(for: at) }
        }
        present(sheet)
    }

    func structureCloseBranchChooser() {
        if let sheet = textView?.window?.rootViewController?.presentedViewController as? UIAlertController,
           sheet.preferredStyle == .actionSheet {
            sheet.dismiss(animated: false)
        }
    }

    func structureDeleteSelection(_ sel: NSRange) {
        textView?.store("", over: sel, selecting: NSRange(location: sel.location, length: 0))
    }

    var structurePasteboardChangeCount: Int { textView?.pasteboard.changeCount ?? -1 }

    func structureWritePasteboard(_ presented: String) -> Int {
        guard let pb = textView?.pasteboard else { return -1 }
        pb.string = presented
        return pb.changeCount
    }

    /// From the topmost controller showing this view.
    private func present(_ controller: UIViewController) {
        var top = textView?.window?.rootViewController
        while let next = top?.presentedViewController { top = next }
        top?.present(controller, animated: true)
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

    /// ✅ EP-055 S3a: the history's structural edits and cross-scene clipboard; nil in a view with no project (test fixtures).
    weak var structure: ManuscriptStructure? {
        didSet { historyUndoManager.structure = structure }
    }
    /// True while a structural edit is in flight (SP-191 Q5's cue): the view takes no input.
    var inputHeld = false

    // ✅ [SP-198] Q-undo (ruled 2026-10-30): Scrivi's history owns undo, as on the Mac (`allowsUndo = false`). ✅ SP-222 Q3:
    // every undo the system offers — ⌘Z / ⇧⌘Z, the Edit menu, the keyboard's undo key, three-finger swipe, shake — reaches
    // this manager, and it runs Scrivi's history. ⚠️ Measured 2026-10-30: `disableUndoRegistration()` is not enough —
    // UITextView still registered — so what UITextView registers here is never used.
    private final class HistoryUndoManager: UndoManager {
        weak var structure: ManuscriptStructure?
        override var canUndo: Bool { MainActor.assumeIsolated { structure?.undoAvailable ?? false } }
        override var canRedo: Bool { MainActor.assumeIsolated { structure?.canRedo ?? false } }
        override func undo() { MainActor.assumeIsolated { structure?.performUndo() } }
        override func redo() { MainActor.assumeIsolated { structure?.performRedo() } }
    }
    private let historyUndoManager: HistoryUndoManager = {
        let u = HistoryUndoManager()
        u.levelsOfUndo = 1   // what UITextView registers is never used; keep none of it
        return u
    }()
    override var undoManager: UndoManager? { historyUndoManager }

    // ✅ Q3 (SP-222): the Mac's keys for the structural edits, ahead of the system's own ⌘⌫ (delete to the line's start).
    override var keyCommands: [UIKeyCommand]? {
        func key(_ input: String, _ flags: UIKeyModifierFlags, _ title: String, _ action: Selector) -> UIKeyCommand {
            let k = UIKeyCommand(title: title, action: action, input: input, modifierFlags: flags)
            k.wantsPriorityOverSystemBehavior = true
            return k
        }
        return (super.keyCommands ?? []) + [
            key("\r", .command, "New Scene", #selector(newScene)),
            key("\r", [.command, .shift], "New Chapter", #selector(newChapter)),
            key(UIKeyCommand.inputDelete, .command, "Merge Scene with Previous", #selector(mergeScene)),
            key(UIKeyCommand.inputDelete, [.command, .shift], "Merge Chapter with Previous", #selector(mergeChapter)),
        ]
    }
    @objc private func newScene() { structure?.handleCreateScene() }
    @objc private func newChapter() { structure?.handleCreateChapter() }
    @objc private func mergeScene() { structure?.handleMergeScene() }
    @objc private func mergeChapter() { structure?.handleMergeChapter() }

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
        if inputHeld { return false }
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
        if inputHeld { return }
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
        if inputHeld { return }
        guard markedTextRange == nil else { return super.deleteBackward() }
        guard selectedRange.length == 0 else {
            // ✅ [SP-188] A selection across scenes → the structural cut, minus the clipboard (design Q4).
            if structure?.cutAcrossScenes(selectedRange, keeping: .nothing) == true { return }
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

    // MARK: Copy and paste — across scenes, the shared structured clipboard (EP-055 S3a)

    private var history: HistoryCapture? { structure?.sync.session.historyCapture }

    /// ✅ EP-046 AC12 + [SP-162] Q2, as on the Mac: other apps get what the writer SEES; Scrivi's own paste gets the
    /// balanced SOURCE. Across scenes, a Range Scrivi's paste rebuilds as scenes ([SP-191]).
    override func copy(_ sender: Any?) {
        let sel = selectedRange
        guard sel.length > 0 else { return }
        if structure?.structuredCopyIfCrossBoundary() == true { return }
        guard !crossesScene(sel) else { return }
        let raw = (textStorage.string as NSString).substring(with: sel)
        let copy: (source: String, presented: String) = presenter?.balancedCopy(in: textStorage, sel)
            ?? (raw, MarkdownEscapes.map(raw).presented)
        pasteboard.string = copy.presented
        ownCopy = (pasteboard.changeCount, copy.source)
    }

    /// ✅ [SP-188] Across scenes, the structural cut (its Range on Scrivi's clipboard when it lands); within one, a `cut`
    /// history event as on the Mac.
    override func cut(_ sender: Any?) {
        let sel = selectedRange
        guard sel.length > 0, !inputHeld else { return }
        if structure?.cutAcrossScenes(sel, keeping: .clipboard) == true { return }
        guard !crossesScene(sel) else { return }
        copy(sender)
        history?.beginPasteOrCut(kind: "cut")
        store("", over: sel, selecting: NSRange(location: sel.location, length: 0))
        history?.flush(trigger: "cut", kind: "cut")
    }

    override func paste(_ sender: Any?) {
        if inputHeld { return }
        // A held structured Range rebuilds its scenes at the caret (or is refused in a chapter title).
        if structure?.hasInternalStructuredRange == true, structure?.structuredPasteIfAvailable() == true { return }
        // ✅ [SP-188] Q6: over a cross-scene selection, the structural cut first; the text goes in when it lands.
        if structure?.cutAcrossScenes(selectedRange, keeping: .nothing, then: { [weak self] in self?.paste(sender) }) == true {
            return
        }
        guard let text = pasteboard.string else { return }
        history?.beginPasteOrCut(kind: "paste")
        defer { history?.flush(trigger: "paste", kind: "paste") }
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

    /// A selection that holds a divider spans scenes — with no project (`structure` nil) there is no structural cut.
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
