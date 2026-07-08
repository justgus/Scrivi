import Foundation
import os

// ProjectSession — per-open-project state and lifecycle (EP-018 / T-0192).
//
// One instance per open project. Owns everything that is specific to a single open
// project: the open result, the viewport loader, per-project preferences, the timeline
// model, pending deep-link navigation, per-window UI toggles, and the project's
// Spotlight donation lifecycle.
//
// App-global dependencies (the engine, the local author identity, the app-support root)
// are injected at construction by AppEnvironment — the session never reaches back into
// AppEnvironment. AppEnvironment owns orchestration (which project/window to open);
// ProjectSession owns one project's state once opened.
//
// One ProjectSession per open project, tracked in OpenProjectRegistry and shown in its
// own AppKit window via ProjectWindowManager (T-0194).
@Observable @MainActor final class ProjectSession {

    // MARK: Injected app-global dependencies (read-only)

    let engine: ScriviEngine
    let authorshipRef: AuthorshipRef?
    let appSupportRoot: String
    let identityID: String

    // MARK: Per-project state

    var openProjectResult: OpenProjectResult?
    var projectRootPath: String?

    // Viewport scene loader — created on load, cleared on close.
    var viewportLoader: ViewportSceneLoader?

    // Per-project preferences — created on load, cleared on close.
    var projectPreferences: ProjectPreferences?

    // Timeline model — created on load, cleared on close.
    var timelineModel: TimelineViewModel?

    // Undo/redo history capture (EP-019) — created on load, closed on close.
    // Owned here (both are @MainActor) so the editor coordinator can reach it
    // via the session and drive capture/undo/redo.
    var historyCapture: HistoryCapture?

    // Scene a pending deep link wants selected once the project is open.
    // EditorView observes this and forwards it into its navigation.
    var pendingNavigationSceneID: String?

    // Spotlight domain identifier (projectID) for this project — retained so we can
    // delete-by-domain on close after openProjectResult is cleared.
    private var spotlightDomainIdentifier: String?

    // Security-scoped access currently held for a deep-link-opened project, so we can
    // release it on close. nil when the open project came from the panel.
    var deepLinkAccessURL: URL?

    // Per-window UI toggles.
    var inspectorVisible: Bool = true
    var timelineVisible: Bool = true
    var showProjectSettings: Bool = false

    init(engine: ScriviEngine,
         authorshipRef: AuthorshipRef?,
         appSupportRoot: String,
         identityID: String) {
        self.engine = engine
        self.authorshipRef = authorshipRef
        self.appSupportRoot = appSupportRoot
        self.identityID = identityID
    }

    // Loads the project at `path` into this session. Returns the result on success (so
    // the caller can inspect repair mode), or throws ScriviError. On success the loader,
    // preferences, timeline model, and Spotlight donation are all established.
    @discardableResult
    func load(at path: String) throws -> OpenProjectResult {
        let result = try engine.openProject(
            projectRootPath: path,
            appSupportRoot: appSupportRoot,
            identityID: identityID
        )
        projectRootPath = path
        openProjectResult = result

        let loader = ViewportSceneLoader(
            engine: engine,
            projectRootPath: path,
            appSupportRoot: appSupportRoot,
            projectID: result.projectID,
            allScenes: result.scenes
        )
        loader.loadAll()
        viewportLoader = loader
        projectPreferences = ProjectPreferences(projectID: result.projectID)

        let tlModel = TimelineViewModel()
        tlModel.load(engine: engine, projectRootPath: path, scenes: result.scenes)
        timelineModel = tlModel

        // Open the undo/redo history for this project (best-effort — never blocks open).
        let capture = HistoryCapture(engine: engine, projectRootPath: path)
        capture.open()
        // Head-hash validation (§6.b): flag any scene changed outside Scrivi since
        // last close with an externalChange barrier (never modifies the manuscript).
        capture.validateScenes(loader.segments.map { ($0.sceneID, $0.text) })
        historyCapture = capture

        // Donate the project's indexable content to Spotlight (best-effort).
        donateSpotlight(projectRootPath: path)
        return result
    }

    // Tears down this session's project: deletes Spotlight items by domain, releases any
    // security-scoped access, and clears all per-project state.
    func close() {
        if let domain = spotlightDomainIdentifier {
            SpotlightDonor.deleteProject(domainIdentifier: domain)
        }
        spotlightDomainIdentifier = nil

        if let url = deepLinkAccessURL {
            url.stopAccessingSecurityScopedResource()
            deepLinkAccessURL = nil
        }

        historyCapture?.close()
        historyCapture = nil

        openProjectResult = nil
        projectRootPath = nil
        viewportLoader = nil
        projectPreferences = nil
        timelineModel = nil
        showProjectSettings = false
        pendingNavigationSceneID = nil
    }

    // Saves the current scene immediately, then refreshes the Spotlight index. Called on
    // app resign (and any time the session must flush).
    func saveAllDirty() async {
        guard let loader = viewportLoader, let ref = authorshipRef else { return }
        await loader.saveAllDirty(engine: engine, ref: ref)
        // Re-donate after saving so indexed content reflects the latest edits.
        if let path = projectRootPath {
            donateSpotlight(projectRootPath: path)
        }
    }

    // Fetches the project's indexable records via the facade and donates them to
    // Spotlight. Best-effort: indexing failures must never disrupt open/save.
    func donateSpotlight(projectRootPath path: String) {
        let log = Logger(subsystem: "com.caposoft.scrivi", category: "Spotlight")
        do {
            log.notice("extract: requesting searchable content for \(path, privacy: .public)")
            let content = try engine.extractSearchableText(projectRootPath: path)
            log.notice("extract OK: \(content.items.count, privacy: .public) records")
            spotlightDomainIdentifier = content.domainIdentifier
            SpotlightDonor.donate(content)
        } catch {
            log.error("extract FAILED: \(String(describing: error), privacy: .public)")
        }
    }
}
