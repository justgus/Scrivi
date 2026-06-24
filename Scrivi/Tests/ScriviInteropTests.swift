import Testing
import Foundation
@testable import ScriviApp

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
        // createdNewIdentity depends on Keychain state across runs — not asserted here.
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
        // Skip when git can't be launched — e.g. not in PATH, or the sandboxed
        // test host denies Process exec. A bare return is a clean no-op skip;
        // withKnownIssue with an empty body would itself be flagged.
        guard ScriviInteropTests.gitAvailable() else { return }

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
        // See enableGitSnapshotsInitializesRepo — bare return is the clean skip.
        guard ScriviInteropTests.gitAvailable() else { return }

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

    // MARK: — Test 10: createObject / openObject / deleteObject — character

    @Test("createObject creates a character and openObject retrieves it")
    func createAndOpenCharacterObject() throws {
        let (engine, _, ref, projectDir, _) = try makeProjectFixture()

        let created = try engine.createObject(
            projectRootPath: projectDir.path,
            objectKind:      "character",
            displayName:     "Elara Voss",
            authorshipRef:   ref
        )

        #expect(!created.objectID.isEmpty)
        #expect(!created.slug.isEmpty)

        let opened = try engine.openObject(
            projectRootPath: projectDir.path,
            objectKind:      "character",
            objectID:        created.objectID
        )

        #expect(!opened.objectJson.isEmpty)
        #expect(!opened.path.isEmpty)
    }

    @Test("deleteObject removes a character object")
    func deleteCharacterObject() throws {
        let (engine, _, ref, projectDir, _) = try makeProjectFixture()

        let created = try engine.createObject(
            projectRootPath: projectDir.path,
            objectKind:      "character",
            displayName:     "Temp Character",
            authorshipRef:   ref
        )

        let deleted = try engine.deleteObject(
            projectRootPath: projectDir.path,
            objectKind:      "character",
            objectID:        created.objectID
        )

        #expect(deleted.deleted == true)
    }

    // MARK: — Test 11: importAsset / listAssets / removeAsset

    @Test("importAsset copies a file and listAssets returns it")
    func importAndListAssets() throws {
        let (engine, _, ref, projectDir, _) = try makeProjectFixture()

        // Write a synthetic source file outside the project.
        let srcDir = try TempDir()
        let srcURL = srcDir.url.appendingPathComponent("image.png")
        try "FAKE_PNG".data(using: .utf8)!.write(to: srcURL)

        let imported = try engine.importAsset(
            projectRootPath: projectDir.path,
            sourcePath:      srcURL.path(percentEncoded: false),
            category:        "image",
            title:           "Cover Image",
            authorshipRef:   ref
        )

        #expect(!imported.assetID.isEmpty)
        #expect(!imported.assetPath.isEmpty)

        let listed = try engine.listAssets(projectRootPath: projectDir.path)
        #expect(listed.count == 1)
    }

    @Test("removeAsset deletes the asset and sidecar")
    func removeAssetDeletesBothFiles() throws {
        let (engine, _, ref, projectDir, _) = try makeProjectFixture()

        let srcDir = try TempDir()
        let srcURL = srcDir.url.appendingPathComponent("doc.pdf")
        try "FAKE_PDF".data(using: .utf8)!.write(to: srcURL)

        let imported = try engine.importAsset(
            projectRootPath: projectDir.path,
            sourcePath:      srcURL.path(percentEncoded: false),
            category:        "document",
            title:           "Notes",
            authorshipRef:   ref
        )

        let removed = try engine.removeAsset(
            projectRootPath: projectDir.path,
            assetID:         imported.assetID
        )

        #expect(removed.deleted == true)

        let listed = try engine.listAssets(projectRootPath: projectDir.path)
        #expect(listed.count == 0)
    }

    // MARK: — Test 12: addComment / listComments / resolveComment

    @Test("addComment adds a comment and listComments returns count 1")
    func addAndListComments() throws {
        let (engine, _, ref, projectDir, _) = try makeProjectFixture()

        let added = try engine.addComment(
            projectRootPath: projectDir.path,
            scopeKind:       "scene",
            targetID:        "scene-abc",
            body:            "Needs more tension here.",
            authorshipRef:   ref
        )

        #expect(added.added == true)
        #expect(!added.commentID.isEmpty)

        let listed = try engine.listComments(
            projectRootPath: projectDir.path,
            scopeKind:       "scene",
            targetID:        "scene-abc"
        )

        #expect(listed.count == 1)
    }

    @Test("resolveComment marks a comment as resolved")
    func resolveCommentSetsResolvedFlag() throws {
        let (engine, _, ref, projectDir, _) = try makeProjectFixture()

        let added = try engine.addComment(
            projectRootPath: projectDir.path,
            scopeKind:       "object",
            targetID:        "char-xyz",
            body:            "Verify backstory.",
            authorshipRef:   ref
        )

        let resolved = try engine.resolveComment(
            projectRootPath: projectDir.path,
            scopeKind:       "object",
            targetID:        "char-xyz",
            commentID:       added.commentID,
            authorshipRef:   ref
        )

        #expect(resolved.resolved == true)
    }

    // MARK: — Test 13: listInbox / importFromInbox

    @Test("listInbox returns empty list on a fresh project")
    func listInboxReturnEmptyOnFreshProject() throws {
        let (engine, _, _, projectDir, _) = try makeProjectFixture()

        let result = try engine.listInbox(projectRootPath: projectDir.path)
        #expect(result.count == 0)
    }

    @Test("importFromInbox importAsAsset moves inbox file to assets")
    func importFromInboxMovesFileToAssets() throws {
        let (engine, _, ref, projectDir, _) = try makeProjectFixture()

        // Drop a file directly into inbox/dropped-files/
        let inboxDir = URL(fileURLWithPath: projectDir.path)
            .appendingPathComponent("inbox/dropped-files")
        try FileManager.default.createDirectory(
            at: inboxDir, withIntermediateDirectories: true)
        let fileURL = inboxDir.appendingPathComponent("hero.png")
        try "PNG_BYTES".data(using: .utf8)!.write(to: fileURL)

        let listBefore = try engine.listInbox(projectRootPath: projectDir.path)
        #expect(listBefore.count == 1)

        let result = try engine.importFromInbox(
            projectRootPath: projectDir.path,
            filename:        "hero.png",
            action:          "importAsAsset",
            category:        "image",
            authorshipRef:   ref
        )

        #expect(result.actionTaken == "importAsAsset")
        #expect(!result.assetID.isEmpty)

        let listAfter = try engine.listInbox(projectRootPath: projectDir.path)
        #expect(listAfter.count == 0)
    }

    // MARK: - Test 14: openProject returns scenes array (T-0059)

    @Test("openProject returns scenes array with one entry for a freshly created project")
    func openProjectReturnsScenesArray() throws {
        let (engine, _, _, projectDir, appSupport) = try makeProjectFixture()

        let opened = try engine.openProject(
            projectRootPath: projectDir.path,
            appSupportRoot:  appSupport.path
        )

        #expect(!opened.scenes.isEmpty)
        #expect(opened.scenes[0].sceneID == opened.activeScene?.sceneID)
        #expect(!opened.scenes[0].title.isEmpty)
        #expect(!opened.scenes[0].metadataPath.isEmpty)
        #expect(!opened.scenes[0].contentPath.isEmpty)
    }

    // MARK: - Test 15: openScene round-trip (T-0060)

    @Test("openScene returns correct scene content and openProject restores it as active scene")
    func openSceneRoundTrip() throws {
        let (engine, _, _, projectDir, appSupport) = try makeProjectFixture()

        let opened = try engine.openProject(
            projectRootPath: projectDir.path,
            appSupportRoot:  appSupport.path
        )

        guard let activeScene = opened.activeScene else {
            Issue.record("Expected activeScene after openProject")
            return
        }

        // Open the same scene via openScene
        let sceneResult = try engine.openScene(
            projectRootPath: projectDir.path,
            appSupportRoot:  appSupport.path,
            projectID:       opened.projectID,
            sceneID:         activeScene.sceneID
        )

        #expect(sceneResult.scene.sceneID == activeScene.sceneID)
        #expect(sceneResult.scene.metadataPath == activeScene.metadataPath)
        #expect(sceneResult.scene.contentPath  == activeScene.contentPath)

        // Re-open project - active scene should still be the same
        let reopened = try engine.openProject(
            projectRootPath: projectDir.path,
            appSupportRoot:  appSupport.path
        )
        #expect(reopened.activeScene?.sceneID == activeScene.sceneID)
    }

    // MARK: - Test 16: createScene round-trip (T-0076)

    @Test("createScene inserts a new scene and openProject reflects updated scene list")
    func createSceneInsertsNewScene() throws {
        let (engine, _, ref, projectDir, appSupport) = try makeProjectFixture()

        let opened = try engine.openProject(
            projectRootPath: projectDir.path,
            appSupportRoot:  appSupport.path
        )
        guard let activeScene = opened.activeScene else {
            Issue.record("Expected activeScene after openProject")
            return
        }

        // chapterID comes from the scenes list, not activeScene
        guard let firstSceneInfo = opened.scenes.first else {
            Issue.record("Expected at least one scene in scenes list")
            return
        }

        let created = try engine.createScene(
            projectRootPath: projectDir.path,
            appSupportRoot:  appSupport.path,
            projectID:       opened.projectID,
            chapterID:       firstSceneInfo.chapterID,
            afterSceneID:    activeScene.sceneID,
            authorshipRef:   ref
        )

        #expect(!created.sceneID.isEmpty)
        #expect(!created.metadataPath.isEmpty)
        #expect(!created.contentPath.isEmpty)
        #expect(created.chapterID == firstSceneInfo.chapterID)

        // openProject should now return 2 scenes
        let reopened = try engine.openProject(
            projectRootPath: projectDir.path,
            appSupportRoot:  appSupport.path
        )
        #expect(reopened.scenes.count == 2)
        let reopenedScenes = reopened.scenes
        #expect(reopenedScenes.count == 2)
        #expect(reopenedScenes[1].sceneID == created.sceneID)
    }

    // MARK: - Test 17: createChapter round-trip (T-0076)

    @Test("createChapter appends a new chapter and openProject reflects updated scene list")
    func createChapterAppendsNewChapter() throws {
        let (engine, _, ref, projectDir, appSupport) = try makeProjectFixture()

        let opened = try engine.openProject(
            projectRootPath: projectDir.path,
            appSupportRoot:  appSupport.path
        )

        let chapter = try engine.createChapter(
            projectRootPath: projectDir.path,
            appSupportRoot:  appSupport.path,
            projectID:       opened.projectID,
            authorshipRef:   ref
        )

        #expect(!chapter.chapterID.isEmpty)
        #expect(!chapter.chapterMetadataPath.isEmpty)
        #expect(!chapter.firstSceneID.isEmpty)
        #expect(!chapter.firstSceneMetadataPath.isEmpty)
        #expect(!chapter.firstSceneContentPath.isEmpty)

        // openProject should now return 2 scenes (original + chapter 2's first scene)
        let reopened = try engine.openProject(
            projectRootPath: projectDir.path,
            appSupportRoot:  appSupport.path
        )
        let reopenedScenes2 = reopened.scenes
        #expect(reopenedScenes2.count == 2)
        #expect(reopenedScenes2[1].sceneID == chapter.firstSceneID)
        #expect(reopenedScenes2[1].chapterID == chapter.chapterID)
    }

    // MARK: - Test 18: extractSearchableText decodes the indexing envelope (T-0181)

    @Test("extractSearchableText returns decoded project, scene, and object records")
    func extractSearchableTextDecodesRecords() throws {
        let (engine, _, ref, projectDir, appSupport) = try makeProjectFixture()

        // Put real Markdown into the opening scene so the scene record carries a
        // stripped contentDescription.
        let created = try engine.openProject(
            projectRootPath: projectDir.path,
            appSupportRoot:  appSupport.path
        )
        guard let scene = created.activeScene else {
            Issue.record("Expected an active scene")
            return
        }
        _ = try engine.saveScene(
            projectID:         created.projectID,
            projectRootPath:   projectDir.path,
            appSupportRoot:    appSupport.path,
            sceneID:           scene.sceneID,
            sceneMetadataPath: scene.metadataPath,
            sceneContentPath:  scene.contentPath,
            markdown:          "# The Beginning\n\nThe **silver mines** of *Khaz'tul*.",
            authorshipRef:     ref
        )

        // Add a world object so a non-scene record appears.
        _ = try engine.createObject(
            projectRootPath: projectDir.path,
            objectKind:      "character",
            displayName:     "Khaz'tul Miner",
            authorshipRef:   ref
        )

        let content = try engine.extractSearchableText(projectRootPath: projectDir.path)

        #expect(content.schema == "scrivi.searchableContent.v1")
        // domainIdentifier is the projectID (delete-by-domain key), not the identity.
        #expect(content.domainIdentifier == created.projectID)

        let project = content.items.first { $0.kind == "project" }
        #expect(project != nil)
        #expect(project?.uniqueIdentifier == "project:\(created.projectID)")

        let sceneItem = content.items.first { $0.kind == "scene" }
        #expect(sceneItem != nil)
        #expect(sceneItem?.containerTitle.isEmpty == false)
        // Markdown markup stripped to plain text.
        #expect(sceneItem?.contentDescription == "The Beginning\nThe silver mines of Khaz'tul.")
        #expect(sceneItem?.deepLink.hasPrefix("scrivi://open?project=\(created.projectID)") == true)

        let character = content.items.first { $0.kind == "character" }
        #expect(character != nil)
        #expect(character?.title == "Khaz'tul Miner")
    }

    // MARK: - Test 19: ScriviDeepLink parsing (T-0184)

    @Test("ScriviDeepLink parses a well-formed scene deep link")
    func deepLinkParsesScene() throws {
        let url = URL(string: "scrivi://open?project=project_abc&item=scene:scene_xyz")!
        let link = try #require(ScriviDeepLink(url: url))
        #expect(link.projectID == "project_abc")
        #expect(link.itemID == "scene:scene_xyz")
        #expect(link.targetSceneID == "scene_xyz")
    }

    @Test("ScriviDeepLink parses a project deep link (no scene target)")
    func deepLinkParsesProject() throws {
        let url = URL(string: "scrivi://open?project=project_abc&item=project:project_abc")!
        let link = try #require(ScriviDeepLink(url: url))
        #expect(link.projectID == "project_abc")
        #expect(link.targetSceneID == nil)
    }

    @Test("ScriviDeepLink requires a non-empty project and the open host")
    func deepLinkRejectsInvalid() {
        #expect(ScriviDeepLink(url: URL(string: "scrivi://open?item=scene:s1")!) == nil)      // no project
        #expect(ScriviDeepLink(url: URL(string: "scrivi://other?project=p1")!) == nil)         // wrong host
        #expect(ScriviDeepLink(url: URL(string: "https://example.com?project=p1")!) == nil)    // wrong scheme
    }

    @Test("ScriviDeepLink tolerates a missing item (project-only link)")
    func deepLinkMissingItem() throws {
        let link = try #require(ScriviDeepLink(url: URL(string: "scrivi://open?project=project_abc")!))
        #expect(link.projectID == "project_abc")
        #expect(link.itemID.isEmpty)
        #expect(link.targetSceneID == nil)
    }
}
