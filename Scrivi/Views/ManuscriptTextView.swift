import SwiftUI
#if os(macOS)
import AppKit

// Custom attribute key used to mark chapter title heading ranges as non-editable.
extension NSAttributedString.Key {
    static let scriviHeading = NSAttributedString.Key("scrivi.heading")
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
        textView.font = NSFont.monospacedSystemFont(ofSize: NSFont.systemFontSize, weight: .regular)
        textView.autoresizingMask = [.width]
        textView.isVerticallyResizable = true
        textView.textContainer?.widthTracksTextView = true
        textView.textContainerInset = NSSize(width: 60, height: 40)
        textView.delegate = context.coordinator
        context.coordinator.textView = textView
        // Register takeFocus with both the coordinator and the loader so any
        // caller (Navigator, delete handler) can transfer first-responder directly.
        let takeFocus: () -> Void = { [weak textView] in
            textView?.window?.makeFirstResponder(textView)
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
        session.createSceneAction   = { [weak coordinator] in coordinator?.takeFocus(); coordinator?.handleCreateScene() }
        session.createChapterAction = { [weak coordinator] in coordinator?.takeFocus(); coordinator?.handleCreateChapter() }
        session.mergeSceneAction    = { [weak coordinator] in coordinator?.takeFocus(); coordinator?.handleMergeScene() }
        session.mergeChapterAction  = { [weak coordinator] in coordinator?.takeFocus(); coordinator?.handleMergeChapter() }

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
        let segIDs = loader.segments.map(\.id)
        let chapterTitleFingerprint = coordinator.chapterHeadingFingerprint(for: loader)
        if segIDs != coordinator.lastSegmentIDs
            || showChapterTitles != coordinator.lastShowChapterTitles
            || chapterTitleFingerprint != coordinator.lastChapterTitleFingerprint {
            coordinator.lastSegmentIDs = segIDs
            coordinator.lastShowChapterTitles = showChapterTitles
            coordinator.lastChapterTitleFingerprint = chapterTitleFingerprint
            coordinator.rebuildStorage(tv, segments: loader.segments)
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

        // Character range for each scene segment in the NSTextStorage.
        // Dividers occupy 1 character each between segments.
        var sceneBoundaries: [NSRange] = []

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
            guard step.moved, let change = step.changes.first,
                  let tv = textView, let storage = tv.textStorage else {
                // A step that did not move (nothing left to undo/redo) still
                // means we are no longer on the previous fork — dismiss it.
                forkPopover.close()
                capture.refreshCanState()
                return
            }

            // Map the changed scene to its loaded segment / storage range.
            guard let segIdx = parent.loader.segments.firstIndex(where: { $0.sceneID == change.sceneID }) else {
                capture.refreshCanState()
                return
            }

            recomputeBoundaries(tv)
            guard sceneBoundaries.indices.contains(segIdx) else {
                capture.refreshCanState()
                return
            }
            let range = sceneBoundaries[segIdx]

            // Apply the full scene text into the scene's boundary under the
            // rebuild/apply guard so textDidChange does not record it as an edit.
            // Dividers and scriviHeading runs sit outside [range] and are untouched.
            capture.withApplying {
                // Match rebuildStorage's body-text attributes exactly (regular
                // monospaced font, default text color) so replaced text does not
                // pick up typing attributes (e.g. bold) or a mismatched color.
                let attrs: [NSAttributedString.Key: Any] = [
                    .font: NSFont.monospacedSystemFont(ofSize: NSFont.systemFontSize, weight: .regular)
                ]
                storage.replaceCharacters(in: range, with: NSAttributedString(string: change.newText, attributes: attrs))
                recomputeBoundaries(tv)

                // Restore the cursor: scene-local UTF-8 byte offset → storage char.
                if sceneBoundaries.indices.contains(segIdx) {
                    let newRange = sceneBoundaries[segIdx]
                    let sceneText = (tv.string as NSString).substring(with: newRange)
                    let charOffset = charOffsetForByteOffset(Int(change.cursorAfter), in: sceneText)
                    let storageLoc = min(newRange.location + charOffset, (tv.string as NSString).length)
                    tv.setSelectedRange(NSRange(location: storageLoc, length: 0))
                    tv.scrollRangeToVisible(NSRange(location: storageLoc, length: 0))
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
                recomputeBoundaries(tv)
                guard let layoutManager = tv.layoutManager,
                      let textContainer = tv.textContainer else { return }
                let centerPoint = NSPoint(x: 0, y: centerY)
                let glyphIdx = layoutManager.glyphIndex(
                    for: centerPoint,
                    in: textContainer,
                    fractionOfDistanceThroughGlyph: nil
                )
                let charIdx = layoutManager.characterIndexForGlyph(at: glyphIdx)
                guard let segIdx = segmentIndex(for: charIdx),
                      loader.segments.indices.contains(segIdx) else { return }
                let sceneID = loader.segments[segIdx].sceneID
                loader.setViewportScene(sceneID)
            }
        }

        // Rebuild the entire NSTextStorage from the current segments.
        // Called when the segment list changes (new scene inserted, viewport shifted, toggle flipped).
        func rebuildStorage(_ tv: NSTextView, segments: [SceneSegment]) {
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

            let storage = tv.textStorage!
            storage.beginEditing()
            storage.setAttributedString(NSAttributedString(string: ""))

            let font = NSFont.monospacedSystemFont(ofSize: NSFont.systemFontSize, weight: .regular)
            let attrs: [NSAttributedString.Key: Any] = [.font: font]
            let showTitles = parent.showChapterTitles

            sceneBoundaries = []
            var offset = 0

            for (i, seg) in segments.enumerated() {
                let isChapterBoundary = i == 0 || segments[i - 1].chapterID != seg.chapterID

                if i > 0 {
                    // Insert divider between every pair of adjacent scenes.
                    let divider = makeDividerAttachment()
                    let divStr = NSMutableAttributedString(attachment: divider)
                    divStr.append(NSAttributedString(string: "\n", attributes: attrs))
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
                    let headingFont = NSFont.boldSystemFont(ofSize: NSFont.systemFontSize + 2)
                    let headingAttrs: [NSAttributedString.Key: Any] = [
                        .font: headingFont,
                        .foregroundColor: NSColor.secondaryLabelColor,
                        .scriviHeading: true
                    ]
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

                sceneBoundaries.append(NSRange(location: start, length: segStr.length))
            }

            storage.endEditing()

            // Sync the change-detection keys so the NEXT updateNSView pass — which the
            // @Observable segment mutation that prompted this rebuild will schedule — sees no
            // change and does NOT rebuild storage again. A redundant rebuild there re-runs
            // setAttributedString and drops the selection to 0, which stole the caret after a
            // scene/chapter merge (SP-075). Any handler that rebuilds storage then places the
            // caret can now trust that placement to survive.
            lastSegmentIDs = segments.map(\.id)
            lastShowChapterTitles = parent.showChapterTitles
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
            guard let text = parent.session.bufferService?.text(inSlot: bufferID),
                  !text.isEmpty else { return }   // empty slot → silent no-op
            // Restore first responder so a palette-driven paste inserts into the editor
            // (the panel click steals focus; insertText on a non-first-responder text
            // view is dropped — the root cause of the palette paste doing nothing).
            // No-op on the keyboard path. This is why the earlier row-click paste failed.
            tv.window?.makeFirstResponder(tv)
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

            // Recompute boundaries from live storage — they shift with every keystroke.
            recomputeBoundaries(tv)

            guard let segIdx = segmentIndex(for: loc) else { return }

            // Extract this segment's text from storage.
            let range = sceneBoundaries[segIdx]
            let extracted = (tv.string as NSString).substring(with: range)

            // The scene's text *before* this edit (for first-edit baseline seeding).
            let preEditText = loader(for: segIdx)?.text ?? ""

            // Update loader in-memory; segment stays loaded.
            parent.loader.updateText(extracted, at: segIdx)

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
                    // §4.d invariant: commit any pending history event before the
                    // scene file is written, so disk always equals a history node.
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

                do {
                    let result = try env.engine.createScene(
                        projectRootPath: rootPath,
                        appSupportRoot: env.appSupportRoot,
                        projectID: proj.projectID,
                        chapterID: currentSeg.chapterID,
                        afterSceneID: currentSeg.sceneID,
                        authorshipRef: ref
                    )

                    if isAtEnd {
                        // Append empty scene — original behaviour.
                        let newIdx = loader.insertScene(result, after: segIdx)
                        loader.setCurrentIndex(newIdx)
                        insertDividerAndMoveCursor(after: segIdx, placeCursorAtStart: true)
                    } else {
                        // Split: head stays in current scene, tail goes to the new scene.
                        let headText = splitHead(of: currentText, at: splitOffsetInSeg)
                        let tailText = splitTail(of: currentText, at: splitOffsetInSeg)

                        // Save head into current scene.
                        _ = try? env.engine.saveScene(
                            projectID: proj.projectID,
                            projectRootPath: rootPath,
                            appSupportRoot: env.appSupportRoot,
                            sceneID: currentSeg.sceneID,
                            sceneMetadataPath: currentSeg.metadataPath,
                            sceneContentPath: currentSeg.contentPath,
                            markdown: headText,
                            authorshipRef: ref
                        )
                        // Save tail into new scene.
                        _ = try? env.engine.saveScene(
                            projectID: proj.projectID,
                            projectRootPath: rootPath,
                            appSupportRoot: env.appSupportRoot,
                            sceneID: result.sceneID,
                            sceneMetadataPath: result.metadataPath,
                            sceneContentPath: result.contentPath,
                            markdown: tailText,
                            authorshipRef: ref
                        )
                        let newIdx = loader.splitScene(result, at: segIdx, headText: headText, tailText: tailText)
                        loader.setCurrentIndex(newIdx)
                        insertDividerAndMoveCursor(after: segIdx, placeCursorAtStart: true)
                    }

                    session.timelineModel?.reloadSceneDots(
                        engine: env.engine, projectRootPath: rootPath, scenes: loader.allScenes)
                    session.timelineModel?.updateDotTitles(liveTitles: loader.liveTitles, allScenes: loader.allScenes)
                } catch {
                    print("[Scrivi] createScene failed: \(error)")
                }
            }
        }

        // Shift-Cmd-Enter: split at cursor creating a new chapter, or append empty chapter at end.
        func handleCreateChapter() {
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
                    let result = try env.engine.createChapter(
                        projectRootPath: rootPath,
                        appSupportRoot: env.appSupportRoot,
                        projectID: proj.projectID,
                        authorshipRef: ref
                    )

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
                        _ = try? env.engine.saveScene(
                            projectID: proj.projectID,
                            projectRootPath: rootPath,
                            appSupportRoot: env.appSupportRoot,
                            sceneID: currentSeg.sceneID,
                            sceneMetadataPath: currentSeg.metadataPath,
                            sceneContentPath: currentSeg.contentPath,
                            markdown: headText,
                            authorshipRef: ref
                        )
                        // Save tail into new chapter's first scene.
                        _ = try? env.engine.saveScene(
                            projectID: proj.projectID,
                            projectRootPath: rootPath,
                            appSupportRoot: env.appSupportRoot,
                            sceneID: result.firstSceneID,
                            sceneMetadataPath: result.firstSceneMetadataPath,
                            sceneContentPath: result.firstSceneContentPath,
                            markdown: tailText,
                            authorshipRef: ref
                        )

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
                        // Re-assign subsequent scenes in the old chapter to the new chapter.
                        loader.splitChapter(result, movingFrom: newIdx, oldChapterID: oldChapterID)
                        // Fix chapter titles in-memory — engine wrote correct ordinals to disk.
                        loader.renumberChapterTitlesFrom(segmentIndex: newIdx)
                        loader.setCurrentIndex(newIdx)
                        insertDividerAndMoveCursor(after: segIdx, placeCursorAtStart: true)
                    }

                    session.timelineModel?.reloadSceneDots(
                        engine: env.engine, projectRootPath: rootPath, scenes: loader.allScenes)
                    session.timelineModel?.updateDotTitles(liveTitles: loader.liveTitles, allScenes: loader.allScenes)
                } catch {
                    print("[Scrivi] createChapter failed: \(error)")
                }
            }
        }

        // Cmd-Backspace: merge scene with previous scene (only if cursor at position 0 of scene,
        // and the scene is not the first scene in its chapter).
        func handleMergeScene() {
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
                        let cursorLoc = sceneBoundaries[segIdx].location
                        tv.setSelectedRange(NSRange(location: cursorLoc, length: 0))
                        tv.scrollRangeToVisible(NSRange(location: cursorLoc, length: 0))
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
                let loc = sceneBoundaries[newSegIdx].location
                tv.setSelectedRange(NSRange(location: loc, length: 0))
                tv.scrollRangeToVisible(NSRange(location: loc, length: 0))
            }
        }

        // Rebuild storage and place the cursor at `textOffset` characters into segment `segIdx`.
        private func rebuildStorageAndPlaceCursor(at segIdx: Int, textOffset: Int) {
            guard let tv = textView else { return }
            rebuildStorage(tv, segments: parent.loader.segments)
            if sceneBoundaries.indices.contains(segIdx) {
                let loc = sceneBoundaries[segIdx].location + textOffset
                let clamped = min(loc, tv.string.count)
                tv.setSelectedRange(NSRange(location: clamped, length: 0))
                tv.scrollRangeToVisible(NSRange(location: clamped, length: 0))
            }
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
        func recomputeBoundaries(_ tv: NSTextView) {
            guard let storage = tv.textStorage else { return }
            let fullLen = storage.length
            guard fullLen > 0 else { return }

            // Skip forward past any scriviHeading-attributed characters at `pos`.
            func skipHeading(from pos: Int) -> Int {
                var p = pos
                while p < fullLen {
                    var effectiveRange = NSRange(location: p, length: 1)
                    let val = storage.attribute(.scriviHeading, at: p, effectiveRange: &effectiveRange)
                    if val != nil {
                        p = effectiveRange.location + effectiveRange.length
                    } else {
                        break
                    }
                }
                return p
            }

            // First segment starts at position 0, after any opening heading.
            var segStart = skipHeading(from: 0)
            var newBoundaries: [NSRange] = []
            var pos = segStart

            while pos < fullLen {
                if storage.attribute(.attachment, at: pos, effectiveRange: nil) != nil {
                    // Found a divider — close the current segment boundary.
                    newBoundaries.append(NSRange(location: segStart, length: pos - segStart))
                    pos += 2  // skip attachment + \n
                    // Skip any heading text that follows the divider.
                    segStart = skipHeading(from: pos)
                    pos = segStart
                } else {
                    pos += 1
                }
            }
            // Last (or only) segment runs to end of storage.
            newBoundaries.append(NSRange(location: segStart, length: fullLen - segStart))

            sceneBoundaries = newBoundaries
        }

        // Resume the writing surface restored from the last session (I-0058).
        // Places the cursor at the restored scene's storage offset + scene-local
        // selection offset, and scrolls it into view. No-op when nothing was restored
        // (fresh open at scene 0 — the existing scroll observer handles highlight).
        func restoreWritingSurface(in tv: NSTextView) {
            let loader = parent.loader
            guard let sceneID = loader.viewportSceneID,
                  let sceneStorageStart = loader.storageOffset(forSceneID: sceneID) else { return }

            // Clamp the scene-local offset to the scene's text length so a shrunken
            // scene (edited externally) can't place the cursor past its bounds.
            let localOffset = loader.restoredSelectionOffset ?? 0
            let sceneLen = loader.segments.first(where: { $0.sceneID == sceneID })?.text.count ?? 0
            let clampedLocal = min(max(0, localOffset), sceneLen)
            let target = sceneStorageStart + clampedLocal

            recomputeBoundaries(tv)
            placeCursorAt(target, in: tv)
            loader.setViewportScene(sceneID)

            // Consume the one-shot restore state so it can't reapply on a later rebuild.
            loader.restoredSelectionOffset = nil
            loader.restoredScrollFraction = nil
        }

        // Navigate to a scene by ID — scrolls to that scene without moving the cursor.
        func navigateToScene(_ sceneID: String, in tv: NSTextView) {
            guard let storageOffset = parent.loader.storageOffset(forSceneID: sceneID) else { return }
            tv.scrollRangeToVisible(NSRange(location: storageOffset, length: 0))
            // Update the Navigator highlight immediately for explicit navigation.
            scrollTask?.cancel()
            highlightTask?.cancel()
            parent.loader.setViewportScene(sceneID)
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
                if storage.attribute(.attachment, at: pos, effectiveRange: nil) != nil {
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

        private func makeDividerAttachment() -> NSTextAttachment {
            let attachment = NSTextAttachment()
            attachment.attachmentCell = DividerAttachmentCell()
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
            recomputeBoundaries(tv)

            // Place the caret at the target scene + scene-local byte offset.
            if let segIdx = loader.segments.firstIndex(where: { $0.sceneID == caretSceneID }),
               sceneBoundaries.indices.contains(segIdx) {
                let base = sceneBoundaries[segIdx].location
                let charOff = charOffsetForByteOffset(caretByteOffset, in: loader.segments[segIdx].text)
                placeCursorAt(base + charOff, in: tv)
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
        private var internalClipboardFragment: FragmentResult?

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
            guard let frag = try? parent.env.engine.fragmentExtract(
                projectRootPath: rootPath, spans: spans) else { return false }
            internalClipboardFragment = frag
            let pb = NSPasteboard.general
            pb.clearContents()
            pb.setString(frag.plainText, forType: .string)
            return true
        }

        // Cross-boundary ⌘X: fragmentCut (delete + collapse), flat plainText to the pasteboard,
        // hold the fragment internally, reload from disk, place the caret at the survivor, and
        // record a structural barrier. Returns true if handled.
        func structuredCutIfCrossBoundary() -> Bool {
            guard let tv = textView else { return false }
            let sel = tv.selectedRange()
            guard selectionCrossesBoundary(sel), let spans = fragmentSpans(for: sel),
                  let rootPath = parent.session.projectRootPath else { return false }
            // Flush pending edits so the on-disk bodies the cut reads are current.
            parent.session.historyCapture?.flush(trigger: "flush")
            guard let result = try? parent.env.engine.fragmentCut(
                projectRootPath: rootPath, spans: spans) else { return false }
            internalClipboardFragment = result.fragment
            let pb = NSPasteboard.general
            pb.clearContents()
            pb.setString(result.fragment.plainText, forType: .string)

            // Structural op — barrier (reversible undo is T-0356). The survivor keeps the caret
            // at the fold point (end of the head prefix = the first span's start byte).
            parent.session.historyCapture?.recordBarrier(
                kind: "structuredCut", note: "Can't undo past a cross-boundary cut")
            let caretByte = spans.first?.start ?? 0
            reloadManuscriptFromDisk(caretSceneID: result.survivingSceneID, caretByteOffset: caretByte)
            return true
        }

        // ⌘V when the internal clipboard holds a structured fragment: reconstruct it at the caret.
        // Refuses (flash) if the caret is in a heading. Returns true if handled; false → caller
        // uses the normal system-pasteboard paste (no structured fragment, or single-scene).
        func structuredPasteIfAvailable() -> Bool {
            guard let tv = textView, let frag = internalClipboardFragment else { return false }
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
            guard let result = try? parent.env.engine.fragmentPaste(
                projectRootPath: rootPath,
                appSupportRoot: parent.session.appSupportRoot,
                projectID: projectID,
                fragmentJSON: frag.toJSON(),
                caretSceneID: seg.sceneID,
                caretByteOffset: caretByte,
                authorshipRef: ref) else { return false }

            parent.session.historyCapture?.recordBarrier(
                kind: "structuredPaste", note: "Can't undo past a cross-boundary paste")
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

    override func shouldChangeText(in affectedCharRange: NSRange, replacementString: String?) -> Bool {
        guard let storage = textStorage, affectedCharRange.length > 0 || (replacementString?.isEmpty == false) else {
            return super.shouldChangeText(in: affectedCharRange, replacementString: replacementString)
        }
        // Block any edit that touches a character with the scriviHeading attribute.
        // For pure insertions (length == 0, non-empty replacement) check only the
        // insertion point itself — do NOT look backward, as that would land inside
        // heading text when the cursor is at the start of a scene.
        // For deletions (length > 0) or backspace-style (length == 0, empty replacement)
        // check the character being removed, falling back one position for backspace.
        let isInsertion = affectedCharRange.length == 0 && replacementString?.isEmpty == false
        let checkRange: NSRange
        if isInsertion {
            checkRange = NSRange(location: affectedCharRange.location, length: 0)
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
        return super.shouldChangeText(in: affectedCharRange, replacementString: replacementString)
    }

    // MARK: — Custom undo/redo routing (EP-019 T-0205, T-0199-validated mechanism)
    //
    // allowsUndo is false and there is NO undoManager override. The Edit ▸ Undo /
    // Redo menu items send undo:/redo: down the responder chain; as first
    // responder this text view implements them and delegates to the Coordinator's
    // custom HistoryService-backed apply path. validateUserInterfaceItem drives
    // the enable state (and could set the menu titles). ⌘Z/⇧⌘Z arrive as these
    // same actions via the SwiftUI menu item's key equivalents.

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
        // A cross-boundary selection cuts as a structured fragment (delete + collapse; barrier).
        // A single-scene selection uses the normal cut (AC5). EP-029 SP-089.
        if coordinator?.structuredCutIfCrossBoundary() == true { return }
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
        insertText(text, replacementRange: selectedRange())
    }

    // Deletes the current selection, routed through AppKit's delete so the removal
    // is captured by textDidChange. Used by cutIntoBuffer (the text is already saved
    // into the slot before this runs). No-op when there is no selection.
    func deleteSelectionForBuffer() {
        guard selectedRange().length > 0 else { return }
        deleteBackward(nil)
    }

    override func keyDown(with event: NSEvent) {
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

    override func deleteBackward(_ sender: Any?) {
        guard let storage = textStorage else { super.deleteBackward(sender); return }
        let loc = selectedRange().location
        guard loc > 0 else { return }
        // The character that would be deleted is at loc-1.
        let target = loc - 1
        if isSeparatorPosition(target, in: storage) { return }
        super.deleteBackward(sender)
    }

    override func deleteForward(_ sender: Any?) {
        guard let storage = textStorage else { super.deleteForward(sender); return }
        let loc = selectedRange().location
        guard loc < storage.length else { return }
        // The character that would be deleted is at loc.
        if isSeparatorPosition(loc, in: storage) { return }
        super.deleteForward(sender)
    }

    // Returns true if the character at `pos` is part of a separator (attachment or its \n).
    private func isSeparatorPosition(_ pos: Int, in storage: NSTextStorage) -> Bool {
        guard pos >= 0, pos < storage.length else { return false }
        // Direct attachment character.
        if storage.attribute(.attachment, at: pos, effectiveRange: nil) != nil { return true }
        // The \n that immediately follows an attachment.
        if (storage.string as NSString).character(at: pos) == 10,
           pos >= 1,
           storage.attribute(.attachment, at: pos - 1, effectiveRange: nil) != nil { return true }
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

private final class DividerAttachmentCell: NSTextAttachmentCell {

    override func cellFrame(
        for textContainer: NSTextContainer,
        proposedLineFragment lineFrag: NSRect,
        glyphPosition position: NSPoint,
        characterIndex charIndex: Int
    ) -> NSRect {
        let width = textContainer.size.width
        return NSRect(x: 0, y: 0, width: width, height: dividerCellHeight)
    }

    override func draw(withFrame cellFrame: NSRect, in controlView: NSView?) {
        let lineY = cellFrame.midY
        let path = NSBezierPath()
        path.lineWidth = 1.0
        path.move(to: NSPoint(x: cellFrame.minX + 20, y: lineY))
        path.line(to: NSPoint(x: cellFrame.maxX - 20, y: lineY))
        NSColor.separatorColor.setStroke()
        path.stroke()
    }

    override func draw(
        withFrame cellFrame: NSRect,
        in controlView: NSView?,
        characterIndex charIndex: Int
    ) {
        draw(withFrame: cellFrame, in: controlView)
    }

    override func draw(
        withFrame cellFrame: NSRect,
        in controlView: NSView?,
        characterIndex charIndex: Int,
        layoutManager: NSLayoutManager
    ) {
        draw(withFrame: cellFrame, in: controlView)
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

    var body: some View {
        Text("Manuscript editor not yet available on this platform.")
            .foregroundStyle(.secondary)
            .frame(maxWidth: .infinity, maxHeight: .infinity)
    }

    func takeFocus() {}
}

#endif
