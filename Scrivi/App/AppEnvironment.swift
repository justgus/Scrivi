import Foundation
import os
#if os(macOS)
import AppKit
#endif

@Observable @MainActor final class AppEnvironment {

    let engine = ScriviEngine()

    var identityResult: IdentityResult?
    var authorshipRef: AuthorshipRef?
    var bootstrapError: ScriviError?

    var openProjectResult: OpenProjectResult?
    var projectRootPath: String?
    var projectError: ScriviError?

    // Spotlight domain identifier (projectID) for the open project — retained so
    // we can delete-by-domain on close after openProjectResult is cleared.
    private var spotlightDomainIdentifier: String?

    // Scene a pending deep link wants selected once the project is open.
    // ManuscriptEditorView observes this and forwards it into its navigation.
    var pendingNavigationSceneID: String?

    // Security-scoped access currently held for a deep-link-opened project, so we
    // can release it on close. nil when the open project came from the panel.
    private var deepLinkAccessURL: URL?

    // Viewport scene loader — created when a project is opened, cleared when closed.
    var viewportLoader: ViewportSceneLoader?

    // Per-project preferences — created when a project is opened, cleared when closed.
    var projectPreferences: ProjectPreferences?

    // Timeline model — created when a project is opened, cleared when closed.
    var timelineModel: TimelineViewModel?

    // Drives the Project Settings sheet from the menu bar.
    var showProjectSettings: Bool = false

    // Drives the Scene Inspector panel visibility via the View menu.
    var inspectorVisible: Bool = true

    // Drives the Timeline strip visibility via the View menu.
    var timelineVisible: Bool = true

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

    func openProject(at path: String) async {
        projectError = nil
        do {
            let result = try engine.openProject(
                projectRootPath: path,
                appSupportRoot: appSupportRoot,
                identityID: identityResult?.identityID ?? ""
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

            // Donate the project's indexable content to Spotlight (best-effort).
            donateSpotlight(projectRootPath: path)
        } catch let e as ScriviError {
            projectError = e
        } catch {
            projectError = ScriviError(code: -1, message: error.localizedDescription)
        }
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
            await openProject(at: path)
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
        panel.canChooseFiles = false
        panel.canChooseDirectories = true
        panel.allowsMultipleSelection = false
        panel.title = "Open Scrivi Project"
        panel.prompt = "Open"
        guard panel.runModal() == .OK, let url = panel.url else { return }
        Task {
            await openProject(at: url.path(percentEncoded: false))
            if let result = openProjectResult, result.mode == "repairRequired",
               let issue = result.repairIssues.first {
                openProjectResult = nil
                projectError = ScriviError(code: -1, message: "Repair required: \(issue.title)")
            } else if let result = openProjectResult {
                // Record a security-scoped bookmark so a deep link can reopen this
                // project later without a fresh panel (EP-017 T-0184).
                ProjectBookmarkStore.record(projectID: result.projectID, url: url)
            }
        }
        #endif
    }

    func closeProject() {
        // Remove this project's Spotlight items by domain before clearing state.
        if let domain = spotlightDomainIdentifier {
            SpotlightDonor.deleteProject(domainIdentifier: domain)
        }
        spotlightDomainIdentifier = nil

        // Release any security-scoped access acquired for a deep-link open.
        if let url = deepLinkAccessURL {
            url.stopAccessingSecurityScopedResource()
            deepLinkAccessURL = nil
        }

        openProjectResult = nil
        projectRootPath = nil
        projectError = nil
        viewportLoader = nil
        projectPreferences = nil
        timelineModel = nil
        showProjectSettings = false
        pendingNavigationSceneID = nil
    }

    // Handles a scrivi://open?project=…&item=… deep link (URL scheme or Spotlight
    // continuation). Opens the target project if it isn't already open, then asks
    // the editor to select the target scene. Best-effort and user-facing on error.
    @MainActor
    func handleDeepLink(_ url: URL) async {
        guard let link = ScriviDeepLink(url: url) else {
            projectError = ScriviError(code: -1, message: "Unrecognized Scrivi link")
            return
        }

        // Already open? Just select the item.
        if let current = openProjectResult, current.projectID == link.projectID {
            pendingNavigationSceneID = link.targetSceneID
            return
        }

        // Resolve the project's path from its stored security-scoped bookmark.
        guard let resolved = ProjectBookmarkStore.resolve(projectID: link.projectID) else {
            projectError = ScriviError(
                code: -1,
                message: "Open this project in Scrivi once so it can be reopened from Spotlight.")
            return
        }

        // Switch projects: close the current one (releases its access), then open
        // the deep-linked one and retain its access scope for the session.
        if openProjectResult != nil { closeProject() }
        await openProject(at: resolved.url.path(percentEncoded: false))

        if openProjectResult != nil {
            deepLinkAccessURL = resolved.didStartAccess ? resolved.url : nil
            pendingNavigationSceneID = link.targetSceneID
        } else if resolved.didStartAccess {
            resolved.url.stopAccessingSecurityScopedResource()
        }
    }

    // Called by the application delegate on NSApplicationDelegate.applicationWillResignActive.
    // Saves the current scene immediately, then refreshes the Spotlight index.
    func onAppResign() async {
        guard let loader = viewportLoader, let ref = authorshipRef else { return }
        await loader.saveAllDirty(engine: engine, ref: ref)
        // Re-donate after saving so indexed content reflects the latest edits.
        if let path = projectRootPath {
            donateSpotlight(projectRootPath: path)
        }
    }

    // Handles a tapped Spotlight result, identified only by the item's
    // uniqueIdentifier ("<kind>:<id>"). A "project:<projectID>" id carries the
    // project directly. A "scene:<id>" id does not encode its project, so it can
    // only be selected when that project is already open; otherwise we direct the
    // user to the scrivi:// path (which does carry the project). This is a
    // best-effort companion to onOpenURL, not the primary deep-link route.
    @MainActor
    func handleSpotlightItem(uniqueIdentifier uid: String) async {
        if uid.hasPrefix("project:") {
            let projectID = String(uid.dropFirst("project:".count))
            await handleDeepLink(URL(string: "scrivi://open?project=\(projectID)&item=\(uid)")!)
            return
        }
        if uid.hasPrefix("scene:") {
            let sceneID = String(uid.dropFirst("scene:".count))
            if openProjectResult?.scenes.contains(where: { $0.sceneID == sceneID }) == true {
                pendingNavigationSceneID = sceneID
            } else {
                projectError = ScriviError(
                    code: -1,
                    message: "Open the project first, then this Spotlight result will jump to the scene.")
            }
        }
    }

    // Fetches the project's indexable records via the facade and donates them to
    // Spotlight. Best-effort: indexing failures must never disrupt open/save.
    private func donateSpotlight(projectRootPath path: String) {
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
