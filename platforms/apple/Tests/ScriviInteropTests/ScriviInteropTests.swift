import Testing
import Foundation
@testable import Scrivi

// End-to-end interop tests for T-0011 and T-0026.
// These prove the Swift/C++ boundary works end-to-end for all 7 facade operations.
//
// Each test uses a fresh temporary directory so tests are independent.
// Tests that require real git are skip-guarded when git is not available in PATH.

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

    // Returns true if `git` is reachable in PATH.
    private static func gitAvailable() -> Bool {
        let proc = Process()
        proc.executableURL = URL(fileURLWithPath: "/usr/bin/env")
        proc.arguments = ["git", "--version"]
        proc.standardOutput = FileHandle.nullDevice
        proc.standardError  = FileHandle.nullDevice
        do {
            try proc.run()
            proc.waitUntilExit()
            return proc.terminationStatus == 0
        } catch {
            return false
        }
    }

    // Shared helper: create a project and return engine, identity, ref, projectDir, appSupport.
    private func makeProjectFixture() throws -> (
        engine:     ScriviEngine,
        identity:   IdentityResult,
        ref:        AuthorshipRef,
        projectDir: TempDir,
        appSupport: TempDir
    ) {
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
            title: "Interop Git Test",
            slug:  "interop-git-test",
            authorshipRef: ref
        )
        return (engine, identity, ref, projectDir, appSupport)
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
        #expect(opened.activeScene != nil)
        #expect(!opened.activeScene!.sceneID.isEmpty)
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
        #expect(reopened.activeScene?.markdown.contains("dark and stormy night") == true)
    }

    // MARK: — Test 5: repairRequired on bad path

    @Test("openProject on nonexistent path returns repairRequired with blocking issues")
    func openProjectOnBadPathReturnsRepairRequired() throws {
        let appSupport = try TempDir()
        let engine = ScriviEngine()

        let result = try engine.openProject(
            projectRootPath: "/tmp/does-not-exist-scrivi-interop",
            appSupportRoot:  appSupport.path
        )
        #expect(result.mode == "repairRequired")
        #expect(!result.repairIssues.isEmpty)
        #expect(result.activeScene == nil)
    }

    // MARK: — Test 6: scanForExternalChanges returns zero issues on a fresh project

    @Test("scanForExternalChanges returns zero issues on a freshly created project")
    func scanForExternalChangesReturnsZeroIssues() throws {
        let (engine, _, _, projectDir, appSupport) = try makeProjectFixture()

        let scan = try engine.scanForExternalChanges(
            projectRootPath: projectDir.path,
            appSupportRoot:  appSupport.path,
            includeGitStatus: false
        )

        #expect(!scan.projectID.isEmpty)
        #expect(scan.repairIssues.isEmpty)
    }

    // MARK: — Test 7: enableGitSnapshots initializes git and returns a snapshot ID

    @Test("enableGitSnapshots initializes git and returns a non-empty snapshotID")
    func enableGitSnapshotsInitializesRepo() throws {
        guard ScriviInteropTests.gitAvailable() else {
            withKnownIssue("git not available in PATH — skip") { }
            return
        }

        let (engine, _, ref, projectDir, _) = try makeProjectFixture()

        let result = try engine.enableGitSnapshots(
            projectRootPath: projectDir.path,
            authorshipRef:   ref,
            initialSnapshotLabel: "Test initial snapshot"
        )

        #expect(result.gitInitialized == true)
        #expect(!result.initialSnapshotID.isEmpty)
        #expect(!result.initialCommitID.isEmpty)
    }

    // MARK: — Test 8: createSnapshot creates a snapshot after changes

    @Test("createSnapshot succeeds on a git-enabled project and returns created=true")
    func createSnapshotSucceeds() throws {
        guard ScriviInteropTests.gitAvailable() else {
            withKnownIssue("git not available in PATH — skip") { }
            return
        }

        let (engine, _, ref, projectDir, appSupport) = try makeProjectFixture()

        // Enable git first
        _ = try engine.enableGitSnapshots(
            projectRootPath: projectDir.path,
            authorshipRef:   ref
        )

        // Write something so there are uncommitted changes
        let opened = try engine.openProject(
            projectRootPath: projectDir.path,
            appSupportRoot:  appSupport.path,
            identityID: ref.identityID
        )
        guard let openedScene = opened.activeScene else { return }
        _ = try engine.saveScene(
            projectID:         opened.projectID,
            projectRootPath:   projectDir.path,
            appSupportRoot:    appSupport.path,
            sceneID:           openedScene.sceneID,
            sceneMetadataPath: openedScene.metadataPath,
            sceneContentPath:  openedScene.contentPath,
            markdown:          "# Draft\n\nSome content for snapshot test.",
            authorshipRef:     ref
        )

        let snapshot = try engine.createSnapshot(
            projectRootPath: projectDir.path,
            authorshipRef:   ref,
            label:           "Test snapshot",
            note:            "Created by interop test"
        )

        #expect(snapshot.created == true)
        #expect(!snapshot.snapshotID.isEmpty)
        #expect(!snapshot.commitID.isEmpty)
        #expect(!snapshot.createdAt.isEmpty)
    }

    // MARK: — Test 9: applyRepair applies createEmptyContentFile via adapter

    @Test("applyRepair createEmptyContentFile resolves missing-content issue end-to-end")
    func applyRepairCreateEmptyFileEndToEnd() throws {
        let appSupport  = try TempDir()
        let projectDir  = try TempDir()
        let engine      = ScriviEngine()

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
            title: "Repair Adapter Test",
            slug:  "repair-adapter-test",
            authorshipRef: ref
        )

        // Delete the scene content file to create a missing-content issue.
        let contentURL = URL(fileURLWithPath: projectDir.path)
            .appendingPathComponent(created.firstScene.contentPath)
        try FileManager.default.removeItem(at: contentURL)

        // Scan to surface the issue.
        let scan = try engine.scanForExternalChanges(
            projectRootPath: projectDir.path,
            appSupportRoot:  appSupport.path,
            includeGitStatus: false
        )

        guard let issue = scan.repairIssues.first(where: { $0.category == "missingContent" }) else {
            Issue.record("Expected a missingContent repair issue after deleting content file")
            return
        }

        // Apply the repair.
        let repairResult = try engine.applyRepair(
            issueID:        issue.issueID,
            projectRootPath: projectDir.path,
            appSupportRoot:  appSupport.path,
            actionKind:     "createEmptyContentFile",
            authorshipRef:  ref
        )

        #expect(repairResult.resolved == true)
        #expect(repairResult.actionApplied == "createEmptyContentFile")

        // The file should now exist.
        #expect(FileManager.default.fileExists(atPath: contentURL.path))
    }
}
