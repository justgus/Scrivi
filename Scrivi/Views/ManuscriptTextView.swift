import SwiftUI
import AppKit

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
    @Binding var navigateToSceneID: String?

    func makeCoordinator() -> Coordinator { Coordinator(self) }

    func makeNSView(context: Context) -> NSScrollView {
        let textView = ManuscriptNSTextView()
        textView.isEditable = true
        textView.isRichText = false
        textView.font = NSFont.monospacedSystemFont(ofSize: NSFont.systemFontSize, weight: .regular)
        textView.autoresizingMask = [.width]
        textView.isVerticallyResizable = true
        textView.textContainer?.widthTracksTextView = true
        textView.textContainerInset = NSSize(width: 60, height: 40)
        textView.delegate = context.coordinator
        context.coordinator.textView = textView

        let scroll = NSScrollView()
        scroll.documentView = textView
        scroll.hasVerticalScroller = true
        scroll.autohidesScrollers = true
        return scroll
    }

    func updateNSView(_ scrollView: NSScrollView, context: Context) {
        guard let tv = scrollView.documentView as? NSTextView else { return }
        let coordinator = context.coordinator

        // Keep coordinator current so delegate callbacks always see the latest env and loader.
        coordinator.parent = self

        // Rebuild text storage when segment list changes.
        let segIDs = loader.segments.map(\.id)
        if segIDs != coordinator.lastSegmentIDs {
            coordinator.lastSegmentIDs = segIDs
            coordinator.rebuildStorage(tv, segments: loader.segments)
        }

        // Scroll to a navigator-selected scene.
        if let targetID = navigateToSceneID {
            if let segIdx = loader.segments.firstIndex(where: { $0.sceneID == targetID }),
               coordinator.sceneBoundaries.indices.contains(segIdx) {
                let range = coordinator.sceneBoundaries[segIdx]
                tv.scrollRangeToVisible(range)
                tv.setSelectedRange(NSRange(location: range.location, length: 0))
            }
            // Clear the binding so this doesn't re-trigger.
            DispatchQueue.main.async { self.navigateToSceneID = nil }
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
        private var saveTask: Task<Void, Never>?
        private var titleTask: Task<Void, Never>?

        // Tracks which segment index the cursor was in at last check.
        private var lastCursorSegmentIndex: Int = 0

        init(_ parent: ManuscriptTextView) { self.parent = parent }

        // Rebuild the entire NSTextStorage from the current segments.
        // Called when the segment list changes (new scene inserted, viewport shifted).
        func rebuildStorage(_ tv: NSTextView, segments: [SceneSegment]) {
            let storage = tv.textStorage!
            storage.beginEditing()
            storage.setAttributedString(NSAttributedString(string: ""))

            let font = NSFont.monospacedSystemFont(ofSize: NSFont.systemFontSize, weight: .regular)
            let attrs: [NSAttributedString.Key: Any] = [.font: font]

            sceneBoundaries = []
            var offset = 0

            for (i, seg) in segments.enumerated() {
                if i > 0 {
                    // Insert divider (1 char placeholder + line break).
                    let divider = makeDividerAttachment()
                    let divStr = NSMutableAttributedString(attachment: divider)
                    divStr.append(NSAttributedString(string: "\n", attributes: attrs))
                    storage.append(divStr)
                    offset += divStr.length
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
            guard let tv = notification.object as? NSTextView else { return }
            let loc = tv.selectedRange().location

            // Recompute boundaries from live storage — they shift with every keystroke.
            recomputeBoundaries(tv)

            guard let segIdx = segmentIndex(for: loc) else { return }

            // Extract this segment's text from storage.
            let range = sceneBoundaries[segIdx]
            let extracted = (tv.string as NSString).substring(with: range)

            // Update loader.
            parent.loader.updateText(extracted, at: segIdx)

            // Handle cursor crossing a segment boundary (scene-exit).
            if segIdx != lastCursorSegmentIndex {
                let oldIdx = lastCursorSegmentIndex
                lastCursorSegmentIndex = segIdx
                parent.loader.setCurrentIndex(segIdx)

                // Save and release the scene we left.
                let loader = parent.loader
                let env = parent.env
                Task { @MainActor in
                    if let ref = env.authorshipRef {
                        await loader.saveAndRelease(at: oldIdx, engine: env.engine, ref: ref)
                    }
                }
            }

            // Debounce 1-second auto-save.
            saveTask?.cancel()
            let loader = parent.loader
            let env = parent.env
            saveTask = Task { @MainActor in
                try? await Task.sleep(nanoseconds: 1_000_000_000)
                guard !Task.isCancelled else { return }
                if let ref = env.authorshipRef {
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
            }
        }

        func textViewDidChangeSelection(_ notification: Notification) {
            guard let tv = notification.object as? NSTextView else { return }
            let loc = tv.selectedRange().location
            guard let segIdx = segmentIndex(for: loc) else { return }

            if segIdx != lastCursorSegmentIndex {
                let oldIdx = lastCursorSegmentIndex
                lastCursorSegmentIndex = segIdx
                parent.loader.setCurrentIndex(segIdx)

                let loader = parent.loader
                let env = parent.env
                Task { @MainActor in
                    if let ref = env.authorshipRef {
                        await loader.saveAndRelease(at: oldIdx, engine: env.engine, ref: ref)
                    }
                }
            }
        }

        // MARK: — Scene/Chapter creation (called from ManuscriptNSTextView)

        func handleCreateScene() {
            guard let tv = textView else { return }
            let loc = tv.selectedRange().location
            guard let segIdx = segmentIndex(for: loc) else { return }

            let loader = parent.loader
            let env = parent.env
            Task { @MainActor in
                guard let ref = env.authorshipRef,
                      let rootPath = env.projectRootPath,
                      let proj = env.openProjectResult,
                      let currentSeg = loader.segments.indices.contains(segIdx)
                          ? loader.segments[segIdx] : nil
                else { return }

                // Save current scene first.
                await loader.saveCurrentIfDirty(engine: env.engine, ref: ref)

                do {
                    let result = try env.engine.createScene(
                        projectRootPath: rootPath,
                        appSupportRoot: env.appSupportRoot,
                        projectID: proj.projectID,
                        chapterID: currentSeg.chapterID,
                        afterSceneID: currentSeg.sceneID,
                        authorshipRef: ref
                    )
                    let newIdx = loader.insertScene(result, after: segIdx)
                    loader.setCurrentIndex(newIdx)
                    insertDividerAndMoveCursor(after: segIdx)
                } catch {
                    print("[Scrivi] createScene failed: \(error)")
                }
            }
        }

        func handleCreateChapter() {
            guard let tv = textView else { return }
            let loc = tv.selectedRange().location
            guard let segIdx = segmentIndex(for: loc) else { return }

            let loader = parent.loader
            let env = parent.env
            Task { @MainActor in
                guard let ref = env.authorshipRef,
                      let rootPath = env.projectRootPath,
                      let proj = env.openProjectResult
                else { return }

                await loader.saveCurrentIfDirty(engine: env.engine, ref: ref)

                do {
                    let result = try env.engine.createChapter(
                        projectRootPath: rootPath,
                        appSupportRoot: env.appSupportRoot,
                        projectID: proj.projectID,
                        authorshipRef: ref
                    )
                    let newIdx = loader.insertChapterFirstScene(result, after: segIdx)
                    loader.setCurrentIndex(newIdx)
                    insertDividerAndMoveCursor(after: segIdx)
                } catch {
                    print("[Scrivi] createChapter failed: \(error)")
                }
            }
        }

        // After inserting a new segment into `loader.segments`, rebuild storage
        // and position the cursor at the start of the new (empty) segment.
        private func insertDividerAndMoveCursor(after segIdx: Int) {
            guard let tv = textView else { return }
            rebuildStorage(tv, segments: parent.loader.segments)
            let newSegIdx = segIdx + 1
            if sceneBoundaries.indices.contains(newSegIdx) {
                let loc = sceneBoundaries[newSegIdx].location
                tv.setSelectedRange(NSRange(location: loc, length: 0))
                tv.scrollRangeToVisible(NSRange(location: loc, length: 0))
            }
        }

        // MARK: — Helpers

        // Recompute sceneBoundaries by scanning the live text storage for divider
        // attachment characters (U+FFFC). Each divider occupies 2 chars (attachment + \n).
        // Segment N spans from end-of-divider-N to start-of-divider-(N+1).
        func recomputeBoundaries(_ tv: NSTextView) {
            guard let storage = tv.textStorage else { return }
            let fullLen = storage.length
            guard fullLen > 0 else { return }

            // Find positions of all divider attachments.
            var dividerStarts: [Int] = []
            var pos = 0
            while pos < fullLen {
                if storage.attribute(.attachment, at: pos, effectiveRange: nil) != nil {
                    dividerStarts.append(pos)
                    pos += 2  // skip attachment + \n
                } else {
                    pos += 1
                }
            }

            // Build boundaries from divider positions.
            var newBoundaries: [NSRange] = []
            var segStart = 0
            for divStart in dividerStarts {
                newBoundaries.append(NSRange(location: segStart, length: divStart - segStart))
                segStart = divStart + 2  // character after attachment + \n
            }
            // Last (or only) segment runs to end of storage.
            newBoundaries.append(NSRange(location: segStart, length: fullLen - segStart))

            sceneBoundaries = newBoundaries
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

    override func keyDown(with event: NSEvent) {
        let isReturn = event.keyCode == 36  // kVK_Return
        let cmd = event.modifierFlags.contains(.command)
        let shift = event.modifierFlags.contains(.shift)

        if isReturn && cmd && shift {
            coordinator?.handleCreateChapter()
            return
        }
        if isReturn && cmd && !shift {
            coordinator?.handleCreateScene()
            return
        }
        super.keyDown(with: event)
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
