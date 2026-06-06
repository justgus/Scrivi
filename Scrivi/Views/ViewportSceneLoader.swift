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
// Loading rule: load forward and backward from the current scene until the
// combined character count fills an estimated viewport plus one scene of buffer
// in each direction. Short or empty scenes are absorbed transparently.
//
// Thread-safety: all mutations happen on @MainActor via AppEnvironment.
@Observable @MainActor final class ViewportSceneLoader {

    // Loaded scene segments in manuscript order.
    private(set) var segments: [SceneSegment] = []

    // Index into `segments` of the scene the cursor is currently in.
    private(set) var currentIndex: Int = 0

    // Approximate character count to consider "viewport full".
    // 3 000 chars ≈ ~1 screen of text at normal font size.
    private let viewportCharBudget: Int = 3_000

    private var engine: ScriviEngine
    private var projectRootPath: String
    private var appSupportRoot: String
    private(set) var projectID: String

    // Full ordered scene list (includes scenes not yet loaded into `segments`).
    // Used by SceneNavigatorView to build the complete sidebar list.
    private(set) var allScenes: [SceneInfo]

    // Live first-line titles keyed by sceneID. Updated by ManuscriptTextView
    // as the author types (debounced ~300ms). Navigator reads from here first,
    // falling back to "Scene X" if absent or empty.
    private(set) var liveTitles: [String: String] = [:]

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

    // Load the initial viewport starting at scene index 0.
    func loadInitial() {
        guard !allScenes.isEmpty else { return }
        loadScene(at: 0, insertAt: .end)
        currentIndex = 0
        fillForward()
    }

    var currentSegment: SceneSegment? {
        guard segments.indices.contains(currentIndex) else { return nil }
        return segments[currentIndex]
    }

    // Called by ManuscriptTextView when the author's cursor moves into a different segment.
    func setCurrentIndex(_ index: Int) {
        currentIndex = index
        fillForward()
        fillBackward()
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

    // Called by scroll detection when the visible top crosses into a different segment.
    // Saves the segment that scrolled out of view; updates currentIndex.
    // Does not reload the viewport — segments already in memory are kept.
    func scrollPromoteTo(index: Int, engine: ScriviEngine, ref: AuthorshipRef) async {
        guard index != currentIndex, segments.indices.contains(index) else { return }
        let departing = currentIndex
        currentIndex = index
        await saveAndRelease(at: departing, engine: engine, ref: ref)
        if segments.indices.contains(departing) {
            segments[departing].isDirty = false
        }
    }

    // Navigate to a scene by ID. Saves current scene, clears loaded segments,
    // reloads from the target scene outward. Returns the new currentIndex (0).
    func navigateTo(sceneID: String, engine: ScriviEngine, ref: AuthorshipRef) async {
        // Save current scene before switching.
        await saveCurrentIfDirty(engine: engine, ref: ref)

        // Find the target in allScenes.
        guard let targetAllIdx = allScenes.firstIndex(where: { $0.sceneID == sceneID }) else { return }

        // Clear current viewport and reload from target.
        segments.removeAll()
        currentIndex = 0
        loadScene(at: targetAllIdx, insertAt: .end)
        fillForward()
        fillBackward()
    }

    // Save and release the segment at `index`. Used on scene-exit.
    func saveAndRelease(at index: Int, engine: ScriviEngine, ref: AuthorshipRef) async {
        guard segments.indices.contains(index), segments[index].isDirty else { return }
        let seg = segments[index]
        _ = try? engine.saveScene(
            projectID: projectID,
            projectRootPath: projectRootPath,
            appSupportRoot: appSupportRoot,
            sceneID: seg.sceneID,
            sceneMetadataPath: seg.metadataPath,
            sceneContentPath: seg.contentPath,
            markdown: seg.text,
            authorshipRef: ref
        )
    }

    // Save the current segment without releasing (used by debounce auto-save).
    func saveCurrentIfDirty(engine: ScriviEngine, ref: AuthorshipRef) async {
        await saveAndRelease(at: currentIndex, engine: engine, ref: ref)
        if segments.indices.contains(currentIndex) {
            segments[currentIndex].isDirty = false
        }
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

        // Clamp currentIndex in case it pointed at or past the removed segment.
        if !segments.isEmpty {
            currentIndex = min(currentIndex, segments.count - 1)
            return segments[currentIndex].sceneID
        }
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
            return segments[currentIndex].sceneID
        }
        return nil
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
        return insertIdx
    }

    // MARK: — Private fill logic

    private enum InsertPosition { case end, beginning }

    private func fillForward() {
        guard let lastSeg = segments.last,
              let lastAllIdx = allScenes.firstIndex(where: { $0.sceneID == lastSeg.sceneID })
        else { return }

        var charCount = segments.map(\.text.count).reduce(0, +)
        var nextAllIdx = lastAllIdx + 1
        var loaded = 0

        // Always load at least one scene forward as a buffer, regardless of charCount.
        // Continue loading until the budget is met.
        while nextAllIdx < allScenes.count, (loaded == 0 || charCount < viewportCharBudget + 1) {
            loadScene(at: nextAllIdx, insertAt: .end)
            charCount += segments.last?.text.count ?? 0
            nextAllIdx += 1
            loaded += 1
        }
    }

    private func fillBackward() {
        guard let firstSeg = segments.first,
              let firstAllIdx = allScenes.firstIndex(where: { $0.sceneID == firstSeg.sceneID }),
              firstAllIdx > 0
        else { return }

        var charCount = segments.map(\.text.count).reduce(0, +)
        var prevAllIdx = firstAllIdx - 1
        var loaded = 0

        // Always load at least one scene backward as a buffer, regardless of charCount.
        // Continue loading until the budget is met.
        while prevAllIdx >= 0, (loaded == 0 || charCount < viewportCharBudget + 1) {
            loadScene(at: prevAllIdx, insertAt: .beginning)
            charCount += segments.first?.text.count ?? 0
            prevAllIdx -= 1
            currentIndex += 1  // segment shifted right
            loaded += 1
        }
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

        // Seed the live title from the loaded content so the Navigator shows
        // the first line immediately on launch, before the author types.
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
}
