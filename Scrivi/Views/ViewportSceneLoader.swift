import Foundation

// A scene segment held in memory by the viewport loader.
struct SceneSegment: Identifiable {
    let id: String          // == sceneID
    let sceneID: String
    let chapterID: String
    let metadataPath: String
    let contentPath: String
    var text: String        // mutable — edited by the author
    var isDirty: Bool = false
}

// ViewportSceneLoader owns the ordered array of loaded SceneSegments.
//
// All scenes are loaded into memory at once on project open (all-in-memory model).
// The dynamic fill/release cycle is not used.
//
// Thread-safety: all mutations happen on @MainActor via AppEnvironment.
@Observable @MainActor final class ViewportSceneLoader {

    // All scene segments in manuscript order.
    private(set) var segments: [SceneSegment] = []

    // Index into `segments` of the scene the cursor is currently in.
    // @ObservationIgnored so cursor movement does not trigger SwiftUI view updates.
    @ObservationIgnored private(set) var currentIndex: Int = 0

    private var engine: ScriviEngine
    private var projectRootPath: String
    private var appSupportRoot: String
    private(set) var projectID: String

    // Full ordered scene list. Used by SceneNavigatorView to build the sidebar.
    private(set) var allScenes: [SceneInfo]

    // Live first-line titles keyed by sceneID.
    private(set) var liveTitles: [String: String] = [:]

    // Manuscript-coordinate start position for each scene (separator chars excluded).
    // Built by rebuildSceneStartMap(); updated after every structural change.
    private(set) var sceneStartMap: [String: Int] = [:]

    // Storage-coordinate start offset for each scene (separator chars included).
    // Used by ManuscriptTextView to place the cursor after navigate/delete.
    private(set) var sceneStorageOffsetMap: [String: Int] = [:]

    // The sceneID the cursor (insertion point) is currently in.
    // @ObservationIgnored — must not trigger SwiftUI view updates on every keystroke.
    @ObservationIgnored private(set) var cursorSceneID: String?

    // The sceneID currently visible in the viewport — drives the Navigator highlight.
    // Updates on scroll (debounced) and on cursor movement.
    // Only this property is observable by SwiftUI.
    private(set) var viewportSceneID: String?

    // Current cursor position in manuscript coordinates (separator chars excluded).
    // @ObservationIgnored so cursor movement does not trigger SwiftUI view updates.
    @ObservationIgnored private(set) var manuscriptCursorPosition: Int = 0

    // Scene-local cursor offset within the current scene (I-0058). Persisted on
    // save as WorkspaceState selection so the cursor is restored on next open.
    @ObservationIgnored private(set) var currentSceneCursorOffset: Int = 0

    // Document scroll fraction (0.0–1.0), updated on scroll (I-0058). Persisted on
    // save as WorkspaceState scroll so scroll position is restored on next open.
    @ObservationIgnored private(set) var scrollFraction: Double = 0

    // Restored writing surface from the last session (I-0058), consumed once by the
    // editor on first appear to place the cursor and scroll. nil after consumption.
    @ObservationIgnored var restoredSelectionOffset: Int?
    @ObservationIgnored var restoredScrollFraction: Double?

    // Set by ManuscriptTextView.Coordinator.makeNSView; used to forward takeFocus calls.
    var takeFocusHandler: (() -> Void)?

    func takeFocus() { takeFocusHandler?() }

    // Called by the Coordinator when the visible scene changes (scroll or cursor movement).
    // This is the only path that updates the Navigator highlight.
    func setViewportScene(_ sceneID: String?) {
        viewportSceneID = sceneID
    }

    init(
        engine: ScriviEngine,
        projectRootPath: String,
        appSupportRoot: String,
        projectID: String,
        allScenes: [SceneInfo]
    ) {
        self.engine = engine
        self.projectRootPath = projectRootPath
        self.appSupportRoot = appSupportRoot
        self.projectID = projectID
        self.allScenes = allScenes
    }

    // Load all scenes into memory at once. Called once on project open.
    //
    // `activeSceneID` (I-0058) is the scene the writer last edited, as returned by the
    // backend openProject flow. When present and still in the manuscript, the loader
    // resumes there: currentIndex/cursorSceneID/viewportSceneID all point at it, so the
    // Navigator highlights it and the editor scrolls to it on first appear. When nil or
    // stale, behaviour is unchanged — resume at the first scene, no pre-selection.
    //
    // `restoredSelectionOffset`/`restoredScrollFraction` are the scene-local cursor
    // offset and document scroll fraction from the last session; the editor consumes
    // them once on appear to place the cursor and scroll.
    func loadAll(
        activeSceneID: String? = nil,
        restoredSelection: Int? = nil,
        restoredScroll: Double? = nil
    ) {
        segments.removeAll()
        for i in allScenes.indices {
            loadScene(at: i, insertAt: .end)
        }
        rebuildSceneStartMap()

        // Resume at the last-edited scene when the backend supplied one and it still exists.
        if let activeSceneID,
           let idx = segments.firstIndex(where: { $0.sceneID == activeSceneID }) {
            currentIndex = idx
            cursorSceneID = activeSceneID
            viewportSceneID = activeSceneID
            restoredSelectionOffset = restoredSelection
            restoredScrollFraction = restoredScroll
        } else {
            currentIndex = 0
            if let firstID = segments.first?.sceneID {
                cursorSceneID = firstID
                // viewportSceneID intentionally left nil — no scene is pre-selected
                // in the Navigator on load. The scroll observer sets it on first scroll.
            }
        }
    }

    var currentSegment: SceneSegment? {
        guard segments.indices.contains(currentIndex) else { return nil }
        return segments[currentIndex]
    }

    // Called by ManuscriptTextView when the author's cursor moves into a different segment.
    func setCurrentIndex(_ index: Int) {
        currentIndex = index
        if segments.indices.contains(index) {
            cursorSceneID = segments[index].sceneID
        }
    }

    // Called by ManuscriptTextView when the author edits text in a segment.
    func updateText(_ text: String, at index: Int) {
        guard segments.indices.contains(index) else { return }
        segments[index].text = text
        segments[index].isDirty = true
    }

    // Update the live navigator title for a scene (debounced by caller).
    func updateLiveTitle(_ title: String, forSceneID sceneID: String) {
        liveTitles[sceneID] = title
    }

    // Save the segment at `index` without removing it from memory.
    //
    // Only the *current* scene's save carries the live cursor offset + scroll fraction
    // (I-0058); the backend stamps WorkspaceState.lastWritingSurface on every save, so
    // the current scene must be the last write for resume to land on it (see saveAllDirty).
    func saveScene(at index: Int, engine: ScriviEngine, ref: AuthorshipRef) async {
        guard segments.indices.contains(index), segments[index].isDirty else { return }
        let seg = segments[index]
        let isCurrent = index == currentIndex
        _ = try? engine.saveScene(
            projectID: projectID,
            projectRootPath: projectRootPath,
            appSupportRoot: appSupportRoot,
            sceneID: seg.sceneID,
            sceneMetadataPath: seg.metadataPath,
            sceneContentPath: seg.contentPath,
            markdown: seg.text,
            selectionAnchor: isCurrent ? currentSceneCursorOffset : 0,
            selectionFocus: isCurrent ? currentSceneCursorOffset : 0,
            scroll: isCurrent ? scrollFraction : 0,
            authorshipRef: ref
        )
        segments[index].isDirty = false
    }

    // Save the current segment (used by debounce auto-save and app resign).
    func saveCurrentIfDirty(engine: ScriviEngine, ref: AuthorshipRef) async {
        await saveScene(at: currentIndex, engine: engine, ref: ref)
    }

    // Save all dirty segments (used on app resign to flush everything).
    // The current scene is saved LAST so its writing-surface state (cursor + scroll)
    // is the one the backend records as lastWritingSurface for next-session resume.
    func saveAllDirty(engine: ScriviEngine, ref: AuthorshipRef) async {
        for i in segments.indices where segments[i].isDirty && i != currentIndex {
            await saveScene(at: i, engine: engine, ref: ref)
        }
        await saveScene(at: currentIndex, engine: engine, ref: ref)
        // Then stamp the scrolled-to scene as the resume point (I-0058 follow-up),
        // so a scene the writer scrolled to but never edited still resumes correctly.
        await stampWritingSurface(engine: engine, ref: ref)
    }

    // Record the scrolled-to (viewport) scene as the backend's lastWritingSurface, even
    // when nothing is dirty (I-0058 follow-up). The backend stamps lastWritingSurface on
    // every saveScene; this forces one final save of the viewport scene so resume lands
    // on the scene the writer was *looking at*, not just the last one they edited.
    //
    // No-op when the viewport scene is the same as the cursor scene (already stamped by
    // the current-scene save above) or when the viewport scene isn't loaded.
    func stampWritingSurface(engine: ScriviEngine, ref: AuthorshipRef) async {
        guard let vpID = viewportSceneID,
              vpID != cursorSceneID,
              let seg = segments.first(where: { $0.sceneID == vpID }) else { return }
        // Scroll fraction is meaningful; the cursor isn't in this scene, so send offset 0
        // (§9.3: "scene changed / cursor not here" ⇒ restore scene + place cursor safely).
        _ = try? engine.saveScene(
            projectID: projectID,
            projectRootPath: projectRootPath,
            appSupportRoot: appSupportRoot,
            sceneID: seg.sceneID,
            sceneMetadataPath: seg.metadataPath,
            sceneContentPath: seg.contentPath,
            markdown: seg.text,
            selectionAnchor: 0,
            selectionFocus: 0,
            scroll: scrollFraction,
            authorshipRef: ref
        )
    }

    // Called by ManuscriptTextView.Coordinator.textViewDidChangeSelection.
    // Only updates the non-observable cursor position. Scene tracking is done
    // via setCurrentIndex() and setHighlightedScene() from the coordinator.
    func updateCursorPosition(_ manuscriptPosition: Int) {
        manuscriptCursorPosition = manuscriptPosition
    }

    // Record the scene-local cursor offset within the current scene (I-0058).
    // Sent as the WorkspaceState selection on the next save so it can be restored.
    func updateSceneCursorOffset(_ offset: Int) {
        currentSceneCursorOffset = max(0, offset)
    }

    // Record the current document scroll fraction (I-0058), sent on the next save.
    func updateScrollFraction(_ fraction: Double) {
        scrollFraction = min(max(fraction, 0), 1)
    }

    // Return the NSTextStorage offset for the start of a scene (separator chars included).
    func storageOffset(forSceneID sceneID: String) -> Int? {
        sceneStorageOffsetMap[sceneID]
    }

    // Return the NSTextStorage offset for cursor placement after a delete.
    // Returns the storage start of the scene at `segmentIndex`, or end-of-storage if out of range.
    func storageOffsetForCursorAfterDelete(nextSegmentIndex: Int, totalStorageLength: Int) -> Int {
        guard segments.indices.contains(nextSegmentIndex),
              let offset = sceneStorageOffsetMap[segments[nextSegmentIndex].sceneID]
        else { return totalStorageLength }
        return offset
    }

    // Insert a new scene after the current segment.
    // Returns the new segment's index.
    func insertScene(_ result: CreateSceneResult, after index: Int) -> Int {
        let seg = SceneSegment(
            id: result.sceneID,
            sceneID: result.sceneID,
            chapterID: result.chapterID,
            metadataPath: result.metadataPath,
            contentPath: result.contentPath,
            text: ""
        )
        let insertIdx = index + 1
        segments.insert(seg, at: insertIdx)

        // Also insert into allScenes so fill logic stays accurate.
        // Inherit chapterMetadataPath from the predecessor scene in the same chapter.
        if let allIdx = allScenes.firstIndex(where: { $0.sceneID == segments[index].sceneID }) {
            let chapterMetaPath = allScenes[allIdx].chapterMetadataPath
            let info = SceneInfo(
                sceneID: result.sceneID,
                chapterID: result.chapterID,
                title: "",
                chapterTitle: allScenes[allIdx].chapterTitle,
                slug: "",
                metadataPath: result.metadataPath,
                contentPath: result.contentPath,
                chapterMetadataPath: chapterMetaPath
            )
            allScenes.insert(info, at: allIdx + 1)
        }
        rebuildSceneStartMap()
        return insertIdx
    }

    // Update the stored title for a scene in allScenes (called after a successful renameScene).
    func updateSceneTitle(_ newTitle: String, forMetadataPath metadataPath: String) {
        guard let idx = allScenes.firstIndex(where: { $0.metadataPath == metadataPath }) else { return }
        let old = allScenes[idx]
        allScenes[idx] = SceneInfo(
            sceneID: old.sceneID,
            chapterID: old.chapterID,
            title: newTitle,
            chapterTitle: old.chapterTitle,
            slug: old.slug,
            metadataPath: old.metadataPath,
            contentPath: old.contentPath,
            chapterMetadataPath: old.chapterMetadataPath
        )
    }

    // Update the stored chapter title for all scenes in a chapter (called after renameChapter).
    func updateChapterTitle(_ newTitle: String, forChapterMetadataPath chapterMetadataPath: String) {
        for idx in allScenes.indices where allScenes[idx].chapterMetadataPath == chapterMetadataPath {
            let old = allScenes[idx]
            allScenes[idx] = SceneInfo(
                sceneID: old.sceneID,
                chapterID: old.chapterID,
                title: old.title,
                chapterTitle: newTitle,
                slug: old.slug,
                metadataPath: old.metadataPath,
                contentPath: old.contentPath,
                chapterMetadataPath: old.chapterMetadataPath
            )
        }
    }

    // Reorder a scene within allScenes to reflect a successful reorderScene call.
    // `targetChapterID` is the chapter the scene now belongs to.
    // `afterSceneID` is the scene it should follow; empty means "insert at beginning of chapter".
    func reorderScene(
        sceneID: String,
        targetChapterID: String,
        afterSceneID: String,
        targetChapterMetadataPath: String,
        targetChapterTitle: String
    ) {
        guard let fromIdx = allScenes.firstIndex(where: { $0.sceneID == sceneID }) else { return }
        var scene = allScenes.remove(at: fromIdx)
        // Update chapter membership fields if the scene moved across chapters.
        if scene.chapterID != targetChapterID {
            scene = SceneInfo(
                sceneID: scene.sceneID,
                chapterID: targetChapterID,
                title: scene.title,
                chapterTitle: targetChapterTitle,
                slug: scene.slug,
                metadataPath: scene.metadataPath,
                contentPath: scene.contentPath,
                chapterMetadataPath: targetChapterMetadataPath
            )
        }
        if afterSceneID.isEmpty {
            // Insert at the beginning of the target chapter's block.
            let insertAt = allScenes.firstIndex(where: { $0.chapterID == targetChapterID }) ?? allScenes.endIndex
            allScenes.insert(scene, at: insertAt)
        } else {
            let afterIdx = allScenes.firstIndex(where: { $0.sceneID == afterSceneID }) ?? allScenes.endIndex - 1
            allScenes.insert(scene, at: afterIdx + 1)
        }
        // Keep segments in sync.
        if let segIdx = segments.firstIndex(where: { $0.sceneID == sceneID }) {
            var seg = segments.remove(at: segIdx)
            seg = SceneSegment(
                id: seg.id,
                sceneID: seg.sceneID,
                chapterID: targetChapterID,
                metadataPath: seg.metadataPath,
                contentPath: seg.contentPath,
                text: seg.text,
                isDirty: seg.isDirty
            )
            if afterSceneID.isEmpty {
                let insertAt = segments.firstIndex(where: { $0.chapterID == targetChapterID }) ?? segments.endIndex
                segments.insert(seg, at: insertAt)
            } else {
                let afterIdx = segments.firstIndex(where: { $0.sceneID == afterSceneID }) ?? segments.endIndex - 1
                segments.insert(seg, at: afterIdx + 1)
            }
            currentIndex = segments.firstIndex(where: { $0.sceneID == currentSegment?.sceneID }) ?? 0
        }
    }

    // Reorder a chapter within allScenes to reflect a successful reorderChapter call.
    // `afterChapterID` is the chapter the moved chapter should follow; empty means first.
    func reorderChapter(chapterID: String, afterChapterID: String) {
        // Extract all scenes belonging to the moved chapter, preserving order.
        let movedScenes = allScenes.filter { $0.chapterID == chapterID }
        allScenes.removeAll { $0.chapterID == chapterID }

        let insertAt: Int
        if afterChapterID.isEmpty {
            insertAt = 0
        } else if let afterIdx = allScenes.lastIndex(where: { $0.chapterID == afterChapterID }) {
            insertAt = afterIdx + 1
        } else {
            insertAt = allScenes.endIndex
        }
        allScenes.insert(contentsOf: movedScenes, at: insertAt)

        // Keep segments in sync.
        let movedSegs = segments.filter { $0.chapterID == chapterID }
        segments.removeAll { $0.chapterID == chapterID }
        let segInsertAt: Int
        if afterChapterID.isEmpty {
            segInsertAt = 0
        } else if let afterIdx = segments.lastIndex(where: { $0.chapterID == afterChapterID }) {
            segInsertAt = afterIdx + 1
        } else {
            segInsertAt = segments.endIndex
        }
        segments.insert(contentsOf: movedSegs, at: segInsertAt)
        currentIndex = segments.firstIndex(where: { $0.sceneID == currentSegment?.sceneID }) ?? 0
    }

    // Remove a scene from allScenes and segments after a successful deleteScene call.
    // Returns the sceneID to navigate to next (nearest remaining scene), or nil if none left.
    func removeScene(sceneID: String) -> String? {
        allScenes.removeAll { $0.sceneID == sceneID }
        segments.removeAll { $0.sceneID == sceneID }
        liveTitles.removeValue(forKey: sceneID)

        if !segments.isEmpty {
            currentIndex = min(currentIndex, segments.count - 1)
            rebuildSceneStartMap()
            return segments[currentIndex].sceneID
        }
        rebuildSceneStartMap()
        return nil
    }

    // Remove all scenes belonging to chapterID from allScenes and segments.
    // Returns the sceneID to navigate to next (nearest remaining scene), or nil if none left.
    func removeChapter(chapterID: String) -> String? {
        let removedIDs = Set(allScenes.filter { $0.chapterID == chapterID }.map(\.sceneID))
        allScenes.removeAll { $0.chapterID == chapterID }
        segments.removeAll { removedIDs.contains($0.sceneID) }
        removedIDs.forEach { liveTitles.removeValue(forKey: $0) }

        if !segments.isEmpty {
            currentIndex = min(currentIndex, segments.count - 1)
            rebuildSceneStartMap()
            return segments[currentIndex].sceneID
        }
        rebuildSceneStartMap()
        return nil
    }

    // Split the scene at `index` at `splitOffset` (character offset within that scene's text).
    // Returns the index of the new (tail) scene, which is inserted at index+1.
    // The caller must have already saved `result` to disk and written the tail text to the new scene's file.
    func splitScene(_ result: CreateSceneResult, at index: Int, headText: String, tailText: String) -> Int {
        segments[index].text = headText
        segments[index].isDirty = false  // already saved by caller

        let newSeg = SceneSegment(
            id: result.sceneID,
            sceneID: result.sceneID,
            chapterID: result.chapterID,
            metadataPath: result.metadataPath,
            contentPath: result.contentPath,
            text: tailText,
            isDirty: false  // tail was saved by caller
        )
        let insertIdx = index + 1
        segments.insert(newSeg, at: insertIdx)

        if let allIdx = allScenes.firstIndex(where: { $0.sceneID == segments[index].sceneID }) {
            let predecessor = allScenes[allIdx]
            let info = SceneInfo(
                sceneID: result.sceneID,
                chapterID: result.chapterID,
                title: "",
                chapterTitle: predecessor.chapterTitle,
                slug: "",
                metadataPath: result.metadataPath,
                contentPath: result.contentPath,
                chapterMetadataPath: predecessor.chapterMetadataPath
            )
            allScenes.insert(info, at: allIdx + 1)
        }
        rebuildSceneStartMap()
        return insertIdx
    }

    // Merge the scene at `index` into the scene at `index-1`.
    // Updates both segments in-memory; caller saves both via ScriviEngine.
    // Returns the segment index of the merged (predecessor) scene.
    func mergeSceneIntoPredecessor(at index: Int, joinText: String) -> Int {
        let predecessorIdx = index - 1
        segments[predecessorIdx].text = joinText
        segments[predecessorIdx].isDirty = false  // caller saves

        let removedSceneID = segments[index].sceneID
        segments.remove(at: index)
        allScenes.removeAll { $0.sceneID == removedSceneID }
        liveTitles.removeValue(forKey: removedSceneID)

        currentIndex = predecessorIdx
        rebuildSceneStartMap()
        return predecessorIdx
    }

    // Recompute ordinal chapter titles ("Chapter N") for all chapters from `segmentIndex` onward.
    // Called after a chapter split to fix the in-memory titles — the engine already wrote the
    // correct ordinals to disk; this keeps the Navigator and tooltip display in sync.
    func renumberChapterTitlesFrom(segmentIndex: Int) {
        // Build the ordered list of distinct chapterIDs as they appear in allScenes.
        var seen = Set<String>()
        var orderedChapterIDs: [String] = []
        for info in allScenes {
            if seen.insert(info.chapterID).inserted {
                orderedChapterIDs.append(info.chapterID)
            }
        }
        // Find the ordinal of the chapter at segmentIndex so we know where renumbering begins.
        guard segments.indices.contains(segmentIndex) else { return }
        let startChapterID = segments[segmentIndex].chapterID
        guard let startOrdinalIdx = orderedChapterIDs.firstIndex(of: startChapterID) else { return }

        // Rewrite chapterTitle for every scene in every chapter from startOrdinalIdx onward.
        for ordinalIdx in startOrdinalIdx ..< orderedChapterIDs.count {
            let chID = orderedChapterIDs[ordinalIdx]
            let newTitle = "Chapter \(ordinalIdx + 1)"
            for j in allScenes.indices where allScenes[j].chapterID == chID {
                let old = allScenes[j]
                allScenes[j] = SceneInfo(
                    sceneID: old.sceneID,
                    chapterID: old.chapterID,
                    title: old.title,
                    chapterTitle: newTitle,
                    slug: old.slug,
                    metadataPath: old.metadataPath,
                    contentPath: old.contentPath,
                    chapterMetadataPath: old.chapterMetadataPath
                )
            }
        }
    }

    // Move scenes from `movingFrom` onward that still belong to `oldChapterID` into the new chapter.
    // Called after splitScene() for the Shift-Cmd-Enter chapter-split path.
    // `movingFrom` is the index of the first scene of the new chapter (already has new chapterID).
    // Subsequent scenes in the old chapter (at movingFrom+1, +2, ...) are re-assigned here.
    func splitChapter(_ result: CreateChapterResult, movingFrom index: Int, oldChapterID: String) {
        // Re-assign any remaining scenes at index+1... that still belong to oldChapterID.
        var i = index + 1
        while i < segments.count && segments[i].chapterID == oldChapterID {
            segments[i] = SceneSegment(
                id: segments[i].id,
                sceneID: segments[i].sceneID,
                chapterID: result.chapterID,
                metadataPath: segments[i].metadataPath,
                contentPath: segments[i].contentPath,
                text: segments[i].text,
                isDirty: segments[i].isDirty
            )
            i += 1
        }

        // Mirror in allScenes for the same scenes (index+1 onward in old chapter).
        for j in allScenes.indices where allScenes[j].chapterID == oldChapterID {
            if let segIdx = segments.firstIndex(where: { $0.sceneID == allScenes[j].sceneID }),
               segIdx > index {
                allScenes[j] = SceneInfo(
                    sceneID: allScenes[j].sceneID,
                    chapterID: result.chapterID,
                    title: allScenes[j].title,
                    chapterTitle: allScenes[j].chapterTitle,
                    slug: allScenes[j].slug,
                    metadataPath: allScenes[j].metadataPath,
                    contentPath: allScenes[j].contentPath,
                    chapterMetadataPath: result.chapterMetadataPath
                )
            }
        }

        rebuildSceneStartMap()
    }

    // Merge the chapter at `index`'s chapter into the predecessor chapter.
    // All scenes from the current chapter move to the predecessor chapter.
    // Caller must call deleteChapter on the engine after this.
    func mergeChapterIntoPredecessor(at index: Int,
                                      predecessorChapterID: String,
                                      predecessorChapterMetadataPath: String,
                                      predecessorChapterTitle: String) {
        let currentChapterID = segments[index].chapterID

        // Re-assign all segments in the current chapter to the predecessor chapter.
        for i in segments.indices where segments[i].chapterID == currentChapterID {
            segments[i] = SceneSegment(
                id: segments[i].id,
                sceneID: segments[i].sceneID,
                chapterID: predecessorChapterID,
                metadataPath: segments[i].metadataPath,
                contentPath: segments[i].contentPath,
                text: segments[i].text,
                isDirty: segments[i].isDirty
            )
        }

        // Mirror in allScenes.
        for j in allScenes.indices where allScenes[j].chapterID == currentChapterID {
            allScenes[j] = SceneInfo(
                sceneID: allScenes[j].sceneID,
                chapterID: predecessorChapterID,
                title: allScenes[j].title,
                chapterTitle: predecessorChapterTitle,
                slug: allScenes[j].slug,
                metadataPath: allScenes[j].metadataPath,
                contentPath: allScenes[j].contentPath,
                chapterMetadataPath: predecessorChapterMetadataPath
            )
        }

        currentIndex = index
        rebuildSceneStartMap()
    }

    // Append a new chapter's first scene after the current segment.
    func insertChapterFirstScene(_ result: CreateChapterResult, after index: Int) -> Int {
        let seg = SceneSegment(
            id: result.firstSceneID,
            sceneID: result.firstSceneID,
            chapterID: result.chapterID,
            metadataPath: result.firstSceneMetadataPath,
            contentPath: result.firstSceneContentPath,
            text: ""
        )
        let insertIdx = index + 1
        segments.insert(seg, at: insertIdx)

        let info = SceneInfo(
            sceneID: result.firstSceneID,
            chapterID: result.chapterID,
            title: "",
            slug: "",
            metadataPath: result.firstSceneMetadataPath,
            contentPath: result.firstSceneContentPath,
            chapterMetadataPath: result.chapterMetadataPath
        )
        if let allIdx = allScenes.firstIndex(where: { $0.sceneID == segments[index].sceneID }) {
            allScenes.insert(info, at: allIdx + 1)
        } else {
            allScenes.append(info)
        }
        rebuildSceneStartMap()
        return insertIdx
    }

    // MARK: — Private helpers

    // Build sceneStartMap and sceneStorageOffsetMap from the current segments array.
    // sceneStartMap: manuscript-coordinate positions (separator chars excluded).
    // sceneStorageOffsetMap: NSTextStorage offsets (separator chars included, 2 chars each).
    // Called after loadAll() and after any structural change.
    func rebuildSceneStartMap() {
        var manuscriptOffset = 0
        var storageOffset = 0
        var newStartMap: [String: Int] = [:]
        var newStorageMap: [String: Int] = [:]

        for (i, seg) in segments.enumerated() {
            if i > 0 {
                storageOffset += 2 // attachment char + \n for separator
            }
            newStartMap[seg.sceneID] = manuscriptOffset
            newStorageMap[seg.sceneID] = storageOffset

            manuscriptOffset += seg.text.count
            storageOffset += seg.text.count
        }
        sceneStartMap = newStartMap
        sceneStorageOffsetMap = newStorageMap
    }

    private func loadScene(at allIdx: Int, insertAt position: InsertPosition) {
        let info = allScenes[allIdx]
        let text: String
        if let result = try? engine.openScene(
            projectRootPath: projectRootPath,
            appSupportRoot: appSupportRoot,
            projectID: projectID,
            sceneID: info.sceneID
        ) {
            text = result.markdown
        } else {
            text = ""
        }
        let seg = SceneSegment(
            id: info.sceneID,
            sceneID: info.sceneID,
            chapterID: info.chapterID,
            metadataPath: info.metadataPath,
            contentPath: info.contentPath,
            text: text
        )

        let firstLine = text
            .components(separatedBy: .newlines)
            .first { !$0.trimmingCharacters(in: .whitespaces).isEmpty } ?? ""
        if !firstLine.isEmpty {
            liveTitles[info.sceneID] = firstLine
        }

        switch position {
        case .end:       segments.append(seg)
        case .beginning: segments.insert(seg, at: 0)
        }
    }

    private enum InsertPosition { case end, beginning }
}
