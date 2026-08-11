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

    // Multiple copy buffers (EP-019 SP-056) — created on load, cleared on close.
    // Owns the in-memory mirror of the persistent slots 1–9; the editor coordinator
    // and the buffers palette both drive it via the session. The palette VISIBILITY is
    // app-global (AppEnvironment.buffersPaletteVisible) since one floating panel follows
    // the frontmost project — the session only owns the per-project buffer data.
    var bufferService: BufferService?

    // Structural editing bridges (EP-019 SP-056, T-0214) — installed by the editor
    // coordinator so the menu bar can invoke the same scene/chapter operations the
    // ⌘↩ / ⌘⇧↩ / ⌘⌫ / ⌘⇧⌫ keyboard commands do, for writers who prefer the mouse.
    // nil until the manuscript view's coordinator is live. Menu items disable when nil.
    var createSceneAction:  (() -> Void)?
    var createChapterAction: (() -> Void)?
    var mergeSceneAction:   (() -> Void)?
    var mergeChapterAction: (() -> Void)?

    // Scene a pending deep link wants selected once the project is open.
    // EditorView observes this and forwards it into its navigation.
    var pendingNavigationSceneID: String?

    // Spotlight domain identifier (projectID) for this project — retained so we can
    // delete-by-domain on close after openProjectResult is cleared.
    private var spotlightDomainIdentifier: String?

    // Security-scoped access currently held for a deep-link-opened project, so we can
    // release it on close. nil when the open project came from the panel.
    var deepLinkAccessURL: URL?

    // Scene Inspector card layout (EP-030 SP-090) — `inspector-layout.json` in the
    // project package. nil until a project is loaded.
    var inspectorLayout: InspectorLayoutStore?

    // Per-window UI toggles.
    //
    // `inspectorVisible` is restored from (and written back to) inspector-layout.json —
    // Doc 2 AC4 requires the hide/show state to persist. It was previously in-memory
    // only, so the inspector reappeared on every launch regardless of the writer's choice.
    var inspectorVisible: Bool = true {
        didSet {
            guard oldValue != inspectorVisible else { return }
            inspectorLayout?.setInspectorHidden(!inspectorVisible)
        }
    }
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
        // Resume at the last-edited scene / cursor / scroll from the previous session
        // (I-0058). The backend openProject flow returns these; wire them into the loader
        // so the editor reopens where the writer left off instead of at the first scene.
        loader.loadAll(
            activeSceneID: result.activeScene?.sceneID,
            restoredSelection: result.restored?.anchor,
            restoredScroll: result.restored?.scroll
        )
        viewportLoader = loader
        let prefs = ProjectPreferences(projectID: result.projectID)
        // Show the real project.json title instead of "Untitled" (I-0093). The backend now returns
        // it in the open envelope; seed the display title from it when the writer hasn't set one on
        // this machine (an explicit Project-Settings rename, persisted to UserDefaults, still wins).
        prefs.seedTitleFromSchemaIfUnset(result.projectTitle)
        projectPreferences = prefs

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
        // I-0104: let the save path report the bytes it writes, so the head hash
        // persisted at close describes disk and the next open compares like with
        // like. Must come after validateScenes, which establishes this session's
        // baseline from the text actually on disk.
        loader.historyCapture = capture

        // Multiple copy buffers (EP-019 SP-056): mirror the persistent slots 1–9 for
        // this project. Reads history/buffers.json (empty when none loaded yet).
        bufferService = BufferService(engine: engine, projectRootPath: path)

        // Scene Inspector card layout (EP-030 SP-090). Project-level and Git-visible;
        // absent on first open, in which case the ruled defaults apply (Worldbuilding
        // empty, Writing = tags/outline/todo).
        let layout = InspectorLayoutStore(projectRootPath: path)
        // Restore the persisted hide/show state (Doc 2 AC4) BEFORE publishing the store,
        // so `inspectorVisible`'s didSet has no store to write back to. Otherwise
        // restoring would immediately re-save the value we just read.
        inspectorVisible = !layout.document.inspectorHidden
        inspectorLayout = layout

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

        bufferService = nil

        // Drop the layout store before the visibility toggle can fire its didSet against
        // a closing project (writes are already flushed — each mutation saves eagerly).
        inspectorLayout = nil

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
    // Synchronous flush for `applicationWillTerminate`, which does not await async
    // work (I-0104/I-0108). Commits pending history first — T-0396 defers the
    // save-time commit, so without this the last typing session would be lost — then
    // writes every dirty scene, which is also what tells history the true disk bytes.
    // No Spotlight re-donation: the process is going away.
    func saveAllDirtyBlocking() {
        guard let loader = viewportLoader, let ref = authorshipRef else { return }
        historyCapture?.flush(trigger: "flush")
        loader.saveAllDirtyBlocking(engine: engine, ref: ref)
    }

    // Closes the history on the quit path (I-0104). `scrivi_history_close` writes the
    // final `state.json` checkpoint; without it every externalChange repair made during
    // the session is discarded, so the same scenes re-flag on the next launch forever.
    // Must run AFTER saveAllDirtyBlocking so the checkpoint sees the saved bytes.
    func closeHistoryBlocking() {
        historyCapture?.close()
        historyCapture = nil
    }

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
