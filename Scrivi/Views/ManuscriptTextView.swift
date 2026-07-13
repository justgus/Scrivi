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

        // Rebuild text storage when segment list or chapter title toggle changes.
        let segIDs = loader.segments.map(\.id)
        if segIDs != coordinator.lastSegmentIDs || showChapterTitles != coordinator.lastShowChapterTitles {
            coordinator.lastSegmentIDs = segIDs
            coordinator.lastShowChapterTitles = showChapterTitles
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
                      let proj = session.openProjectResult,
                      loader.segments.indices.contains(segIdx),
                      loader.segments.indices.contains(segIdx - 1)
                else { return }

                let currentSeg  = loader.segments[segIdx]
                let predecessorSeg = loader.segments[segIdx - 1]
                await loader.saveCurrentIfDirty(engine: env.engine, ref: ref)
                // Structural op — record a barrier so undo stops here (§4.5).
                session.historyCapture?.recordBarrier(kind: "sceneMerge", note: "Can't undo past a scene merge")

                // Join: predecessor text + current text (no separator).
                let joinText = predecessorSeg.text + currentSeg.text
                let joinPoint = predecessorSeg.text.count  // cursor lands here after merge

                // Save merged text into the predecessor scene.
                _ = try? env.engine.saveScene(
                    projectID: proj.projectID,
                    projectRootPath: rootPath,
                    appSupportRoot: env.appSupportRoot,
                    sceneID: predecessorSeg.sceneID,
                    sceneMetadataPath: predecessorSeg.metadataPath,
                    sceneContentPath: predecessorSeg.contentPath,
                    markdown: joinText,
                    authorshipRef: ref
                )
                // Delete the current (now empty) scene from disk.
                _ = try? env.engine.deleteScene(projectRootPath: rootPath, sceneID: currentSeg.sceneID)

                let mergedIdx = loader.mergeSceneIntoPredecessor(at: segIdx, joinText: joinText)
                loader.setCurrentIndex(mergedIdx)
                rebuildStorageAndPlaceCursor(at: mergedIdx, textOffset: joinPoint)

                session.timelineModel?.reloadSceneDots(
                    engine: env.engine, projectRootPath: rootPath, scenes: loader.allScenes)
                session.timelineModel?.updateDotTitles(liveTitles: loader.liveTitles, allScenes: loader.allScenes)
            }
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

                // Find predecessor chapter's metadata path from allScenes.
                let predecessorMeta = loader.allScenes.first(where: { $0.chapterID == predecessorChapterID })?.chapterMetadataPath ?? ""
                let predecessorTitle = loader.allScenes.first(where: { $0.chapterID == predecessorChapterID })?.chapterTitle ?? ""

                // Update in-memory state: move all scenes from current chapter to predecessor chapter.
                loader.mergeChapterIntoPredecessor(
                    at: segIdx,
                    predecessorChapterID: predecessorChapterID,
                    predecessorChapterMetadataPath: predecessorMeta,
                    predecessorChapterTitle: predecessorTitle
                )
                // Renumber all chapters from the predecessor onward — the deleted chapter
                // shifts every subsequent chapter's ordinal down by one.
                let renumberFrom = loader.segments.firstIndex(where: { $0.chapterID == predecessorChapterID }) ?? segIdx
                loader.renumberChapterTitlesFrom(segmentIndex: renumberFrom)

                // Delete the (now empty) chapter from disk.
                _ = try? env.engine.deleteChapter(projectRootPath: rootPath, chapterID: currentChapterID)

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
    override func paste(_ sender: Any?) {
        coordinator?.parent.session.historyCapture?.beginPasteOrCut(kind: "paste")
        super.paste(sender)
        coordinator?.parent.session.historyCapture?.flush(trigger: "paste", kind: "paste")
    }

    override func cut(_ sender: Any?) {
        coordinator?.parent.session.historyCapture?.beginPasteOrCut(kind: "cut")
        super.cut(sender)
        coordinator?.parent.session.historyCapture?.flush(trigger: "cut", kind: "cut")
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
