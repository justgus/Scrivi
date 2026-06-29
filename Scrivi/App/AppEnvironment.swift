import Foundation
#if os(macOS)
import AppKit
import UniformTypeIdentifiers
#endif

// AppEnvironment — app-global state and orchestration (EP-018 / T-0192, T-0194).
//
// Holds only what is genuinely app-wide: the engine, the local author identity, and
// open/create/deep-link orchestration. Per-project state lives on ProjectSession, one
// per open project, tracked in OpenProjectRegistry and shown in its own AppKit window
// via ProjectWindowManager.
//
// `activeSession` is a convenience pointer to the most-recently-resolved window's
// session; menu commands use the frontmost window's session via @FocusedValue, not this.
@Observable @MainActor final class AppEnvironment {

    let engine = ScriviEngine()

    var identityResult: IdentityResult?
    var authorshipRef: AuthorshipRef?
    var bootstrapError: ScriviError?

    // The single open project's session (single-window until T-0194). nil = no project.
    var activeSession: ProjectSession?

    // Authoritative record of which projects are open, keyed by projectID. The R3
    // non-reentrancy guard (EP-018 / T-0193): we check this before opening so an
    // already-open project is reused rather than duplicated. Single-window until T-0194,
    // then it spans all open windows.
    let openProjects = OpenProjectRegistry()

    // Open/create failure surfaced before (or instead of) a session existing. Shown by
    // LandingView / NewProjectSheet. Distinct from a session's own runtime state.
    var projectError: ScriviError?

    #if os(macOS)
    // Owns the AppKit NSWindow for each open project (deterministic lifecycle; replaces
    // the abandoned WindowGroup(for:) which cached dead windows). T-0194.
    let windows = ProjectWindowManager()
    #endif

    // One-time guard so launch setup (bootstrap + URL handler + restore) runs once, even
    // though the Welcome window's .task re-fires whenever Welcome reopens.
    var didLaunchSetup = false

    // The frontmost project window's session — drives the menu bar's enabled state and
    // per-window View toggles. Set by each AppKit window when it becomes key; cleared
    // when its project closes or the Welcome window is frontmost. (AppKit windows don't
    // feed SwiftUI's @FocusedValue, so we track focus explicitly.)
    var frontmostSession: ProjectSession?

    // Bridges `openWindow(id: "welcome")` so orchestration can reopen the Welcome window
    // when the last project closes. Installed by the scene on appear.
    var openWelcomeAction: (() -> Void)?

    // Set by the File ▸ New Project… menu command to ask the Welcome window to present
    // its New Project sheet. LandingView observes and clears it. The menu also reopens
    // the Welcome window so the sheet has a host.
    var requestNewProject: Bool = false

    // iOS only: set by the File ▸ Open Project… menu command to ask the showing iOS view to
    // present its .fileImporter (the iPad menu bar can't toggle a view's local @State directly).
    // Observed and cleared by the iOS root / Landing view. On macOS, Open uses an NSOpenPanel
    // directly (presentOpenProjectPanel), so this flag is unused there.
    var requestOpenImporter: Bool = false

    // File ▸ New Project…: ensure the Landing/Welcome surface is present, then ask it to show
    // the New Project sheet. On macOS the Welcome window hosts the sheet. On iOS the app is
    // single-scene — the sheet's host (LandingView) only exists when no project is open, so we
    // close the active project first to surface Landing, which then consumes the request flag.
    func presentNewProject() {
        #if os(macOS)
        openWelcomeAction?()
        #else
        closeActiveProjectForLanding()
        #endif
        requestNewProject = true
    }

    // File ▸ Open Project… on iOS: surface Landing (closing any open project on this single-scene
    // platform), then ask it to raise the document importer. (macOS uses an NSOpenPanel directly.)
    func presentOpenImporter() {
        #if !os(macOS)
        closeActiveProjectForLanding()
        #endif
        requestOpenImporter = true
    }

    #if !os(macOS)
    // iOS single-scene helper: close the open project so the Landing view (which hosts the
    // New Project sheet and the Open document importer) returns to the single window.
    private func closeActiveProjectForLanding() {
        if let pid = activeSession?.openProjectResult?.projectID {
            closeProject(projectID: pid)
        }
    }
    #endif

    // Opens — or, for an already-open project, focuses (R3) — the AppKit window for a
    // project whose session is already loaded+registered. The registry is the
    // authoritative R3 guard.
    func requestOpenWindow(for projectID: String) {
        guard let session = openProjects.session(for: projectID) else { return }
        #if os(macOS)
        windows.openOrFocus(session: session, env: self)
        #endif
    }

    // Called by ProjectWindowManager when a project window is closed by the user (red
    // button or ⌘W). Tears down the session and reopens Welcome if it was the last.
    func didCloseProjectWindow(projectID: String) {
        closeProject(projectID: projectID)
    }

    var appSupportRoot: String {
        FileManager.default.urls(for: .applicationSupportDirectory, in: .userDomainMask)
            .first!.path(percentEncoded: false)
    }

    func bootstrap() async {
        let displayName = ProcessInfo.processInfo.hostName
        do {
            let result = try engine.ensureLocalIdentity(
                displayName: displayName,
                appSupportRoot: appSupportRoot
            )
            identityResult = result
            authorshipRef = AuthorshipRef(
                identityID: result.identityID,
                personaID: result.defaultPersonaID,
                displayName: result.displayName
            )
            print("[Scrivi] Identity: \(result.identityID) (new: \(result.createdNewIdentity))")
        } catch let e as ScriviError {
            bootstrapError = e
            print("[Scrivi] Bootstrap failed: \(e.message)")
        } catch {
            print("[Scrivi] Bootstrap unexpected error: \(error)")
        }
    }

    // True from applicationWillTerminate onward. While terminating, the cascade of window
    // closes must NOT rewrite the manifest (that would wipe it to empty) — the manifest is
    // frozen to the set that was open at quit. (R4 / T-0195.)
    private var isTerminating = false

    // Writes the current open-project set to the session manifest (R4 / T-0195), so the
    // next launch restores exactly these windows. Suppressed during termination.
    private func persistOpenManifest() {
        guard !isTerminating else { return }
        OpenSessionManifest.save(projectIDs: openProjects.openProjectIDs)
    }

    // Called on applicationWillTerminate: freeze the manifest to the still-open set
    // BEFORE the quit-time window-close cascade empties the registry.
    func beginTermination() {
        OpenSessionManifest.save(projectIDs: openProjects.openProjectIDs)
        isTerminating = true
    }

    // Restores all project windows that were open at last quit (R4 / T-0195). Skips any
    // project whose bookmark no longer resolves (moved/deleted) — best-effort. Called
    // once after bootstrap.
    func restoreOpenProjects() async {
        let saved = OpenSessionManifest.load()
        guard !saved.isEmpty else { return }
        for projectID in saved {
            // Only attempt projects we can still resolve; a failed one is silently skipped
            // (its window simply won't reappear). ensureOpenAndShow opens each window.
            if ProjectBookmarkStore.hasBookmark(projectID: projectID) {
                await ensureOpenAndShow(projectID: projectID)
            }
        }
        // Rewrite the manifest to the set that actually came back.
        persistOpenManifest()
    }

    // Builds a fresh ProjectSession with the app-global dependencies injected.
    private func makeSession() -> ProjectSession {
        ProjectSession(
            engine: engine,
            authorshipRef: authorshipRef,
            appSupportRoot: appSupportRoot,
            identityID: identityResult?.identityID ?? ""
        )
    }

    // Loads the project at `path` into a registered session and returns it. If the
    // project is already open (matched by projectID via the registry), the existing
    // session is reused rather than duplicated (R3). Records a bookmark so the project
    // can later be reopened/restored by projectID alone. On failure, surfaces
    // projectError and returns nil.
    //
    // This does NOT open a window — callers open/focus a window via
    // `requestOpenWindow(for:using:)` with the returned session's projectID.
    @discardableResult
    func loadProject(at path: String, bookmarkURL: URL? = nil) async -> ProjectSession? {
        projectError = nil
        let session = makeSession()
        do {
            let result = try session.load(at: path)
            // R3: if this project is already open, reuse the registered session and
            // discard the one we just loaded.
            if let existing = openProjects.session(for: result.projectID) {
                session.close()
                activeSession = existing
                return existing
            }
            openProjects.register(session)
            activeSession = session
            // Record a bookmark so this project can be reopened/restored by projectID
            // (the per-window path resolves the path from here). Panel-opens pass the
            // user-picked URL; other callers may omit it.
            if let url = bookmarkURL {
                ProjectBookmarkStore.record(projectID: result.projectID, url: url)
            }
            persistOpenManifest()
            return session
        } catch let e as ScriviError {
            projectError = e
            return nil
        } catch {
            projectError = ScriviError(code: -1, message: error.localizedDescription)
            return nil
        }
    }

    // Ensures a project is open and its window is shown/focused (EP-018 / T-0194).
    //   • already open → focus its existing window (R3).
    //   • not open → resolve the path from ProjectBookmarkStore, load, register, open a
    //     window (also the restore-on-launch path for R4 / T-0195).
    // Returns the live session, or nil if it can't be resolved/loaded.
    @discardableResult
    func ensureOpenAndShow(projectID: String) async -> ProjectSession? {
        if let existing = openProjects.session(for: projectID) {
            requestOpenWindow(for: projectID)   // focus existing window
            return existing
        }
        guard let resolved = ProjectBookmarkStore.resolve(projectID: projectID) else {
            projectError = ScriviError(
                code: -1,
                message: "This project could not be reopened. Open it once from the Welcome window.")
            return nil
        }
        let session = await loadProject(at: resolved.url.path(percentEncoded: false))
        if let session {
            session.deepLinkAccessURL = resolved.didStartAccess ? resolved.url : nil
            if let pid = session.openProjectResult?.projectID {
                requestOpenWindow(for: pid)
            }
        } else if resolved.didStartAccess {
            resolved.url.stopAccessingSecurityScopedResource()
        }
        return session
    }

    func createProject(at path: String, title: String, slug: String) async {
        projectError = nil
        guard let ref = authorshipRef else {
            projectError = ScriviError(code: -1, message: "Identity not bootstrapped")
            return
        }
        do {
            _ = try engine.createProject(
                projectRootPath: path,
                appSupportRoot: appSupportRoot,
                title: title,
                slug: slug,
                authorshipRef: ref
            )
            let url = URL(fileURLWithPath: path)
            if let session = await loadProject(at: path, bookmarkURL: url),
               let projectID = session.openProjectResult?.projectID {
                requestOpenWindow(for: projectID)
            }
        } catch let e as ScriviError {
            projectError = e
        } catch {
            projectError = ScriviError(code: -1, message: error.localizedDescription)
        }
    }

    @MainActor
    func presentOpenProjectPanel() {
        #if os(macOS)
        let panel = NSOpenPanel()
        // A .scrivi project is a *package* (a directory with a registered UTI). The panel must
        // present it as a single selectable item, not traverse into it. Allow files of the .scrivi
        // type and treat packages as opaque (not directories); fall back to allowing directories so
        // a .scrivi that lost its UTI registration is still choosable. This mirrors the Welcome
        // screen's .fileImporter, which allows the same com.caposoft.scrivi.project type.
        let scriviType = UTType("com.caposoft.scrivi.project") ?? .package
        panel.allowedContentTypes = [scriviType]
        panel.canChooseFiles = true
        panel.canChooseDirectories = true
        panel.treatsFilePackagesAsDirectories = false
        panel.allowsMultipleSelection = false
        panel.title = "Open Scrivi Project"
        panel.prompt = "Open"
        guard panel.runModal() == .OK, let url = panel.url else { return }
        Task {
            let session = await loadProject(at: url.path(percentEncoded: false), bookmarkURL: url)
            if let result = session?.openProjectResult, result.mode == "repairRequired",
               let issue = result.repairIssues.first {
                // Repair required — do not open a window; tear the session back down.
                if let pid = result.projectID as String?, !pid.isEmpty {
                    closeProject(projectID: pid)
                }
                projectError = ScriviError(code: -1, message: "Repair required: \(issue.title)")
            } else if let projectID = session?.openProjectResult?.projectID {
                requestOpenWindow(for: projectID)
            }
        }
        #endif
    }

    // Opens a project from a URL the user picked in a document importer (iOS/iPadOS/visionOS).
    // Picked URLs are security-scoped — access must be started before reading and the scope held
    // for the life of the open. On the single-window platforms, loadProject sets activeSession,
    // which the iOS root view observes to show the editor (no requestOpenWindow needed).
    @MainActor
    func openProjectFromPickedURL(_ url: URL) async {
        projectError = nil
        let scoped = url.startAccessingSecurityScopedResource()
        let session = await loadProject(at: url.path(percentEncoded: false), bookmarkURL: url)
        if let result = session?.openProjectResult, result.mode == "repairRequired",
           let issue = result.repairIssues.first {
            if !result.projectID.isEmpty { closeProject(projectID: result.projectID) }
            projectError = ScriviError(code: -1, message: "Repair required: \(issue.title)")
            if scoped { url.stopAccessingSecurityScopedResource() }
        } else if let projectID = session?.openProjectResult?.projectID {
            requestOpenWindow(for: projectID)   // no-op on iOS; editor shows via activeSession
            // NOTE: scope is intentionally not released here on success — the session reads the
            // package for the duration it is open. Released when the project closes (a future
            // iOS-close path); acceptable for the current single-project iOS model.
        } else if scoped {
            url.stopAccessingSecurityScopedResource()
        }
    }

    // Creates a project inside a user-picked parent directory (iOS/iPadOS/visionOS). The macOS
    // path uses NSSavePanel directly in NewProjectSheet; iOS picks a folder, then we compose
    // <parent>/<slug>.scrivi. Security scope is started for the create + initial read.
    @MainActor
    func createProjectInPickedDirectory(_ directory: URL, title: String, slug: String) async {
        let scoped = directory.startAccessingSecurityScopedResource()
        let projectURL = directory.appendingPathComponent("\(slug).scrivi", isDirectory: true)
        await createProject(at: projectURL.path(percentEncoded: false), title: title, slug: slug)
        // On success the session holds the package open; release on failure.
        if projectError != nil, scoped {
            directory.stopAccessingSecurityScopedResource()
        }
    }

    // Closes a specific project's session and deregisters it from the registry. Called by
    // ProjectWindowManager when the window closes (any path) and by repair-abort. When the
    // last project closes, reopens the Welcome window. Idempotent.
    func closeProject(projectID: String) {
        guard let session = openProjects.session(for: projectID) else { return }
        openProjects.deregister(projectID: projectID)
        session.close()
        if activeSession === session { activeSession = nil }
        if frontmostSession === session { frontmostSession = nil }
        projectError = nil
        persistOpenManifest()
        if openProjects.isEmpty {
            // Last project closed → reopen Welcome. Defer to the next runloop tick so the
            // closing AppKit window has finished tearing down, and activate the app first
            // so SwiftUI's openWindow(id:) reliably surfaces the window (calling it from
            // an AppKit windowWillClose otherwise no-ops — the app may not be active).
            // The app does not terminate on last-window-close (see AppDelegate).
            let action = openWelcomeAction
            Task { @MainActor in
                #if os(macOS)
                NSApp.activate(ignoringOtherApps: true)
                #endif
                action?()
            }
        }
    }

    // Handles a scrivi://open?project=…&item=… deep link (URL scheme or Spotlight
    // continuation). Opens or focuses the target project's window (R3 via the registry),
    // then asks that window's editor to select the target scene. Best-effort and
    // user-facing on error.
    @MainActor
    func handleDeepLink(_ url: URL) async {
        guard let link = ScriviDeepLink(url: url) else {
            projectError = ScriviError(code: -1, message: "Unrecognized Scrivi link")
            return
        }

        // Already open? Focus its window and select the item.
        if let existing = openProjects.session(for: link.projectID) {
            existing.pendingNavigationSceneID = link.targetSceneID
            requestOpenWindow(for: link.projectID)
            return
        }

        // Not open — load it (if it has a bookmark) and open its window. The scene is
        // applied to the session before the window shows.
        guard ProjectBookmarkStore.hasBookmark(projectID: link.projectID) else {
            projectError = ScriviError(
                code: -1,
                message: "Open this project in Scrivi once so it can be reopened from Spotlight.")
            return
        }
        if let session = await ensureOpenAndShow(projectID: link.projectID) {
            session.pendingNavigationSceneID = link.targetSceneID
        }
    }

    // Called by the application delegate on willResignActive. Saves every open session's
    // current scene, then refreshes each project's Spotlight index.
    func onAppResign() async {
        for session in openProjects.sessions.values {
            await session.saveAllDirty()
        }
    }

    // Handles a tapped Spotlight result.
    //
    // Two carriers can identify the result, in order of preference:
    //   1. `relatedURL` — the full `scrivi://open?project=…&item=…` deep link. The donor
    //      sets it as each item's `relatedUniqueIdentifier` (SpotlightDonor), so it carries
    //      the **projectID** and is sufficient to open even a *closed* project (via its
    //      bookmark). When present we route through the canonical `handleDeepLink`.
    //   2. `uid` — the item's `uniqueIdentifier` ("<kind>:<id>"), the only field guaranteed
    //      on the continuation activity. A "project:<projectID>" id carries its project; a
    //      "scene:<id>" id does NOT, so without (1) a scene tap can only select when that
    //      project is already open.
    //
    // The `scrivi://` URL scheme remains the primary, fully-reliable route; this is its
    // best-effort Spotlight-continuation companion (T-0184).
    @MainActor
    func handleSpotlightItem(uniqueIdentifier uid: String, relatedURL: URL? = nil) async {
        // Preferred: the donated deep link carries the projectID — open/closed both work.
        if let relatedURL, ScriviDeepLink(url: relatedURL) != nil {
            await handleDeepLink(relatedURL)
            return
        }

        if uid.hasPrefix("project:") {
            let projectID = String(uid.dropFirst("project:".count))
            await handleDeepLink(URL(string: "scrivi://open?project=\(projectID)&item=\(uid)")!)
            return
        }
        if uid.hasPrefix("scene:") {
            let sceneID = String(uid.dropFirst("scene:".count))
            // No projectID available (no related URL): we can only act if some open project
            // contains this scene. Select it in that project's window.
            if let session = openProjects.sessions.values.first(where: {
                $0.openProjectResult?.scenes.contains(where: { $0.sceneID == sceneID }) == true
            }), let projectID = session.openProjectResult?.projectID {
                session.pendingNavigationSceneID = sceneID
                requestOpenWindow(for: projectID)
            } else {
                projectError = ScriviError(
                    code: -1,
                    message: "Open the project first, then this Spotlight result will jump to the scene.")
            }
        }
    }
}
