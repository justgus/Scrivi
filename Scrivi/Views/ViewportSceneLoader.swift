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
        if let allIdx = allScenes.firstIndex(where: { $0.sceneID == segments[index].sceneID }) {
            let info = SceneInfo(
                sceneID: result.sceneID,
                chapterID: result.chapterID,
                title: "",
                slug: "",
                metadataPath: result.metadataPath,
                contentPath: result.contentPath
            )
            allScenes.insert(info, at: allIdx + 1)
        }
        return insertIdx
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
            contentPath: result.firstSceneContentPath
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

        while charCount < viewportCharBudget + 1, nextAllIdx < allScenes.count {
            loadScene(at: nextAllIdx, insertAt: .end)
            charCount += segments.last?.text.count ?? 0
            nextAllIdx += 1
        }
    }

    private func fillBackward() {
        guard let firstSeg = segments.first,
              let firstAllIdx = allScenes.firstIndex(where: { $0.sceneID == firstSeg.sceneID }),
              firstAllIdx > 0
        else { return }

        var charCount = segments.map(\.text.count).reduce(0, +)
        var prevAllIdx = firstAllIdx - 1

        while charCount < viewportCharBudget + 1, prevAllIdx >= 0 {
            loadScene(at: prevAllIdx, insertAt: .beginning)
            charCount += segments.first?.text.count ?? 0
            prevAllIdx -= 1
            currentIndex += 1  // segment shifted right
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
        switch position {
        case .end:       segments.append(seg)
        case .beginning: segments.insert(seg, at: 0)
        }
    }
}
