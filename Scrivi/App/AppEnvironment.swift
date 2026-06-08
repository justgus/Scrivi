import Foundation

@Observable @MainActor final class AppEnvironment {

    let engine = ScriviEngine()

    var identityResult: IdentityResult?
    var authorshipRef: AuthorshipRef?
    var bootstrapError: ScriviError?

    var openProjectResult: OpenProjectResult?
    var projectRootPath: String?
    var projectError: ScriviError?

    // Viewport scene loader — created when a project is opened, cleared when closed.
    var viewportLoader: ViewportSceneLoader?

    // Per-project preferences — created when a project is opened, cleared when closed.
    var projectPreferences: ProjectPreferences?

    var appSupportRoot: String {
        FileManager.default.urls(for: .applicationSupportDirectory, in: .userDomainMask)
            .first!.path(percentEncoded: false)
    }

    func bootstrap() async {
        let displayName = Host.current().localizedName ?? "Unknown Mac"
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

            // Create the viewport loader for this project.
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

    // Called by the application delegate on NSApplicationDelegate.applicationWillResignActive.
    // Saves the current scene immediately.
    func onAppResign() async {
        guard let loader = viewportLoader, let ref = authorshipRef else { return }
        await loader.saveAllDirty(engine: engine, ref: ref)
    }
}
