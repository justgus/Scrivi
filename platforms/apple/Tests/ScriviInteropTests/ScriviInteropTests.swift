import Testing
import Foundation
@testable import Scrivi

// End-to-end interop tests for T-0011.
// These prove the Swift/C++ boundary works: ensureLocalIdentity → createProject → openProject → saveScene.
//
// Each test uses a fresh temporary directory so tests are independent.

struct ScriviInteropTests {

    // Temporary directory that removes itself on deinit.
    private final class TempDir: @unchecked Sendable {
        let url: URL

        init() throws {
            url = FileManager.default.temporaryDirectory
                .appendingPathComponent("scrivi-interop-\(UUID().uuidString)")
            try FileManager.default.createDirectory(at: url, withIntermediateDirectories: true)
        }

        deinit {
            try? FileManager.default.removeItem(at: url)
        }

        var path: String { url.path(percentEncoded: false) }
    }

    // MARK: — Test 1: ensureLocalIdentity returns real IDs

    @Test("ensureLocalIdentity returns non-empty identityID and personaID with correct prefixes")
    func ensureLocalIdentityReturnsRealIDs() throws {
        let appSupport = try TempDir()

        let engine = ScriviEngine()
        let identity = try engine.ensureLocalIdentity(
            displayName: "Test Author",
            appSupportRoot: appSupport.path
        )

        #expect(!identity.identityID.isEmpty)
        #expect(!identity.defaultPersonaID.isEmpty)
        #expect(identity.identityID.hasPrefix("identity_"))
        #expect(identity.defaultPersonaID.hasPrefix("persona_"))
        #expect(identity.createdNewIdentity == true)
    }

    // MARK: — Test 2: createProject succeeds with real AuthorshipRef

    @Test("createProject succeeds with AuthorshipRef from ensureLocalIdentity")
    func createProjectWithRealIdentity() throws {
        let appSupport  = try TempDir()
        let projectDir  = try TempDir()

        let engine = ScriviEngine()

        let identity = try engine.ensureLocalIdentity(
            displayName: "Test Author",
            appSupportRoot: appSupport.path
        )

        let ref = AuthorshipRef(
            identityID:  identity.identityID,
            personaID:   identity.defaultPersonaID,
            displayName: identity.displayName
        )

        let project = try engine.createProject(
            projectRootPath: projectDir.path,
            appSupportRoot:  appSupport.path,
            title: "Interop Test Novel",
            slug:  "interop-test-novel",
            authorshipRef: ref
        )

        #expect(!project.projectID.isEmpty)
        #expect(!project.firstScene.sceneID.isEmpty)
        #expect(!project.firstScene.metadataPath.isEmpty)
        #expect(!project.firstScene.contentPath.isEmpty)
    }

    // MARK: — Test 3: openProject returns Markdown

    @Test("openProject returns active scene after createProject")
    func openProjectReturnsActiveScene() throws {
        let appSupport = try TempDir()
        let projectDir = try TempDir()

        let engine = ScriviEngine()

        let identity = try engine.ensureLocalIdentity(
            displayName: "Test Author",
            appSupportRoot: appSupport.path
        )
        let ref = AuthorshipRef(
            identityID:  identity.identityID,
            personaID:   identity.defaultPersonaID,
            displayName: identity.displayName
        )

        _ = try engine.createProject(
            projectRootPath: projectDir.path,
            appSupportRoot:  appSupport.path,
            title: "Interop Open Test",
            slug:  "interop-open-test",
            authorshipRef: ref
        )

        let opened = try engine.openProject(
            projectRootPath: projectDir.path,
            appSupportRoot:  appSupport.path,
            identityID: identity.identityID
        )

        #expect(!opened.projectID.isEmpty)
        #expect(!opened.activeScene.sceneID.isEmpty)
    }

    // MARK: — Test 4: saveScene persists Markdown

    @Test("saveScene persists Markdown and returns saved=true")
    func saveScenePersistsMarkdown() throws {
        let appSupport = try TempDir()
        let projectDir = try TempDir()

        let engine = ScriviEngine()

        let identity = try engine.ensureLocalIdentity(
            displayName: "Test Author",
            appSupportRoot: appSupport.path
        )
        let ref = AuthorshipRef(
            identityID:  identity.identityID,
            personaID:   identity.defaultPersonaID,
            displayName: identity.displayName
        )

        let created = try engine.createProject(
            projectRootPath: projectDir.path,
            appSupportRoot:  appSupport.path,
            title: "Interop Save Test",
            slug:  "interop-save-test",
            authorshipRef: ref
        )

        let markdown = "# Chapter One\n\nIt was a dark and stormy night."

        let saved = try engine.saveScene(
            projectID:         created.projectID,
            projectRootPath:   projectDir.path,
            appSupportRoot:    appSupport.path,
            sceneID:           created.firstScene.sceneID,
            sceneMetadataPath: created.firstScene.metadataPath,
            sceneContentPath:  created.firstScene.contentPath,
            markdown:          markdown,
            authorshipRef:     ref
        )

        #expect(saved.saved == true)
        #expect(!saved.sceneID.isEmpty)
        #expect(saved.wordCount > 0)

        // Verify markdown persisted by reopening
        let reopened = try engine.openProject(
            projectRootPath: projectDir.path,
            appSupportRoot:  appSupport.path,
            identityID: identity.identityID
        )
        #expect(reopened.activeScene.markdown.contains("dark and stormy night"))
    }

    // MARK: — Test 5: ScriviError on bad path

    @Test("openProject on nonexistent path throws ScriviError")
    func openProjectOnBadPathThrows() throws {
        let appSupport = try TempDir()
        let engine = ScriviEngine()

        #expect(throws: ScriviError.self) {
            _ = try engine.openProject(
                projectRootPath: "/tmp/does-not-exist-scrivi-interop",
                appSupportRoot:  appSupport.path
            )
        }
    }
}
