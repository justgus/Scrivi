import Foundation
import Scrivi

@Observable @MainActor final class AppEnvironment {

    let engine = ScriviEngine()

    var identityResult: IdentityResult?
    var authorshipRef: AuthorshipRef?
    var bootstrapError: ScriviError?

    var openProjectResult: OpenProjectResult?
    var projectRootPath: String?
    var projectError: ScriviError?

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
        } catch let e as ScriviError {
            projectError = e
        } catch {
            projectError = ScriviError(code: -1, message: error.localizedDescription)
        }
    }

    func saveScene(markdown: String, cursorOffset: Double, scrollPosition: Double) async {
        projectError = nil
        guard let result = openProjectResult,
              let scene = result.activeScene,
              let rootPath = projectRootPath,
              let ref = authorshipRef else { return }
        do {
            _ = try engine.saveScene(
                projectID: result.projectID,
                projectRootPath: rootPath,
                appSupportRoot: appSupportRoot,
                sceneID: scene.sceneID,
                sceneMetadataPath: scene.metadataPath,
                sceneContentPath: scene.contentPath,
                markdown: markdown,
                authorshipRef: ref
            )
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
}
