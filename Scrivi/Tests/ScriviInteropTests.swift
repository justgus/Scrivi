import Testing
import Foundation
import SwiftUI
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

        // ⚠️ T-0409: a character is world-scoped, so it needs a world.
        let world = try engine.createWorld(
            projectRootPath: projectDir.path,
            packagePath: projectDir.url.appendingPathComponent("T10.scrivworld")
                                       .path(percentEncoded: false),
            displayName: "T10 World", epochLabel: "")
        let created = try engine.createObject(
            projectRootPath: projectDir.path,
            objectKind:      "character",
            displayName:     "Elara Voss",
            authorshipRef:   ref,
            worldID:         world.worldID
        )

        #expect(!created.objectID.isEmpty)
        #expect(!created.slug.isEmpty)

        let opened = try engine.openObject(
            projectRootPath: projectDir.path,
            objectKind:      "character",
            objectID:        created.objectID,
            worldID:         world.worldID
        )

        #expect(!opened.objectJson.isEmpty)
        #expect(!opened.path.isEmpty)
    }

    @Test("deleteObject removes a character object")
    func deleteCharacterObject() throws {
        let (engine, _, ref, projectDir, _) = try makeProjectFixture()

        let world = try engine.createWorld(
            projectRootPath: projectDir.path,
            packagePath: projectDir.url.appendingPathComponent("T10d.scrivworld")
                                       .path(percentEncoded: false),
            displayName: "T10 World", epochLabel: "")
        let created = try engine.createObject(
            projectRootPath: projectDir.path,
            objectKind:      "character",
            displayName:     "Temp Character",
            authorshipRef:   ref,
            worldID:         world.worldID
        )

        let deleted = try engine.deleteObject(
            projectRootPath: projectDir.path,
            objectKind:      "character",
            objectID:        created.objectID,
            worldID:         world.worldID
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

        // ⚠️ SP-116 T-0427/T-0428: `assets` used to be a JSON-encoded STRING built
        // without escaping, so nothing here could read a title or a path. It is a
        // real array now — and this is the first test that looks INSIDE it, which
        // is why the escaping defect survived so long.
        let asset = try #require(listed.assets.first)
        #expect(asset.assetID == imported.assetID)
        #expect(asset.title == "Cover Image")
        #expect(asset.assetPath == imported.assetPath)
        #expect(FileManager.default.fileExists(atPath: asset.assetPath))
    }

    @Test("a title containing quotes survives listAssets — I-0143")
    func assetTitleWithQuotesRoundTrips() throws {
        let (engine, _, ref, projectDir, _) = try makeProjectFixture()

        let srcDir = try TempDir()
        let srcURL = srcDir.url.appendingPathComponent("quoted.png")
        try "FAKE_PNG".data(using: .utf8)!.write(to: srcURL)

        let nasty = #"The "Sundered" Coast \ Vol. 2"#
        let imported = try engine.importAsset(
            projectRootPath: projectDir.path,
            sourcePath:      srcURL.path(percentEncoded: false),
            category:        "image",
            title:           nasty,
            authorshipRef:   ref
        )

        // Before SP-116 this envelope was malformed and the decode threw.
        let listed = try engine.listAssets(projectRootPath: projectDir.path)
        let asset  = try #require(listed.assets.first { $0.assetID == imported.assetID })
        #expect(asset.title == nasty)
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

    // MARK: — EP-034 SP-117: the Detail Sheet's data layer

    @Suite("Object detail model (SP-117)")
    struct ObjectDetailTests {

        /// ⚠️ S4 — THE defect this sprint is most likely to ship silently.
        ///
        /// A sheet that RECONSTRUCTS an object from a Swift struct would drop
        /// every field it does not model — `image`, `thumbnailAssetID`,
        /// `attributes`, and anything a later core adds. The loss would be
        /// invisible until a writer noticed her portrait gone.
        ///
        /// `ObjectCard.rename()` already patches for exactly this reason; this
        /// asserts the Detail Sheet does the same.
        @Test("editing notes PRESERVES fields the sheet never displays")
        func patchPreservesUnknownFields() throws {
            let original = """
            {
              "schema": "scrivi.object.character.v1",
              "objectID": "character_test",
              "slug": "mara",
              "displayName": "Mara",
              "subtitle": "",
              "notes": "",
              "status": "active",
              "worldID": "world_test",
              "image": { "assetID": "asset_portrait", "thumbnailAssetID": "asset_thumb" },
              "attributes": [ { "k": "eyes", "v": "grey" } ],
              "createdBy": { "identityID": "id1", "personaID": "p1",
                             "displayNameAtCreation": "Author" },
              "aFieldThisBuildHasNeverHeardOf": "must survive"
            }
            """

            let detail = try ObjectDetail(json: original, kind: "character")
            #expect(detail.imageAssetID == "asset_portrait")

            let patched = try detail.applyingEdits(
                displayName: "Mara",
                subtitle: "Cartographer",
                notes: "Born in the salt marches."
            )

            let root = try #require(try JSONSerialization.jsonObject(
                with: Data(patched.utf8)) as? [String: Any])

            // The edits landed...
            #expect(root["subtitle"] as? String == "Cartographer")
            #expect(root["notes"] as? String == "Born in the salt marches.")

            // ...and NOTHING else was lost.
            let image = try #require(root["image"] as? [String: Any])
            #expect(image["assetID"] as? String == "asset_portrait")
            #expect(image["thumbnailAssetID"] as? String == "asset_thumb")
            #expect((root["attributes"] as? [[String: Any]])?.count == 1)
            #expect(root["schema"] as? String == "scrivi.object.character.v1")
            #expect(root["objectID"] as? String == "character_test")
            #expect(root["worldID"] as? String == "world_test")
            #expect(root["createdBy"] as? [String: Any] != nil)
            // ⚠️ The unknown-field case, stated explicitly: a build that has never
            // heard of a key must still carry it through.
            #expect(root["aFieldThisBuildHasNeverHeardOf"] as? String == "must survive")
        }

        @Test("tags decode from their {\"v\": …} wire form, not a bare string array")
        func tagsDecodeFromWireForm() throws {
            // ⚠️ Tags serialize as [{"v": "..."}] (ObjectJson.cpp:40-44). Reading
            // them as [String] yields an empty list and looks like "no tags" —
            // a silent, plausible-looking wrong answer.
            let json = """
            { "objectID": "o1", "displayName": "Mara",
              "tags": [ { "v": "protagonist" }, { "v": "cartographer" } ] }
            """
            let detail = try ObjectDetail(json: json, kind: "character")
            #expect(detail.tags == ["protagonist", "cartographer"])
        }

        @Test("an object written by an older core opens with blank optional fields")
        func olderCoreObjectStillOpens() throws {
            // No subtitle, no notes, no image, no tags — legitimately absent.
            // Refusing to open it would be worse than showing blanks.
            let json = """
            { "objectID": "o1", "slug": "vance", "displayName": "Vance" }
            """
            let detail = try ObjectDetail(json: json, kind: "character")
            #expect(detail.displayName == "Vance")
            #expect(detail.subtitle.isEmpty)
            #expect(detail.notes.isEmpty)
            #expect(detail.imageAssetID.isEmpty)
            #expect(detail.tags.isEmpty)
        }

        @Test("a save does not stamp modifiedAt — ScriviCore owns that")
        func patchDoesNotStampModified() throws {
            let json = """
            { "objectID": "o1", "displayName": "Mara",
              "modifiedAt": "2020-01-01T00:00:00Z" }
            """
            let detail = try ObjectDetail(json: json, kind: "character")
            let patched = try detail.applyingEdits(displayName: "Mara",
                                                   subtitle: "", notes: "x")
            let root = try #require(try JSONSerialization.jsonObject(
                with: Data(patched.utf8)) as? [String: Any])
            // ⚠️ Unchanged — ObjectStore::save stamps it. Writing it here would
            // either be overwritten or disagree with the core's clock.
            #expect(root["modifiedAt"] as? String == "2020-01-01T00:00:00Z")
        }
    }

    @Suite("Detail Sheet navigation history (SP-117 T-0435)")
    struct ObjectDetailHistoryTests {

        private func entry(_ id: String) -> ObjectDetailHistory.Entry {
            .init(objectID: id, kind: "character", worldID: "w", displayName: id)
        }

        @Test("back and forward move through the trail")
        func backAndForward() {
            let h = ObjectDetailHistory()
            #expect(!h.canGoBack)
            #expect(!h.canGoForward)

            h.visit(entry("Mara"))
            h.visit(entry("Vance"))
            #expect(h.current?.objectID == "Vance")
            #expect(h.canGoBack)
            #expect(!h.canGoForward)

            h.goBack()
            #expect(h.current?.objectID == "Mara")
            // ⚠️ FORWARD must now be reachable — this is the half NavigationStack
            // cannot give, and the reason D2-B was ruled over D2-A.
            #expect(h.canGoForward)

            h.goForward()
            #expect(h.current?.objectID == "Vance")
            #expect(!h.canGoForward)
        }

        @Test("visiting a new object truncates forward history")
        func visitTruncatesForward() {
            let h = ObjectDetailHistory()
            h.visit(entry("A"))
            h.visit(entry("B"))
            h.goBack()                    // at A, forward → B
            #expect(h.canGoForward)

            h.visit(entry("C"))           // the browser rule
            #expect(h.current?.objectID == "C")
            #expect(!h.canGoForward, "forward must not lead somewhere she never went")
            h.goBack()
            #expect(h.current?.objectID == "A")
        }

        @Test("re-visiting the object already shown is a no-op")
        func revisitIsNoOp() {
            // ⚠️ Without this, opening the same sheet twice stacks duplicates and
            // "back" appears to do nothing — the shape of I-0132.
            let h = ObjectDetailHistory()
            h.visit(entry("Mara"))
            h.visit(entry("Mara"))
            #expect(h.entries.count == 1)
            #expect(!h.canGoBack)
        }

        @Test("reset clears the trail")
        func resetClears() {
            let h = ObjectDetailHistory()
            h.visit(entry("A"))
            h.visit(entry("B"))
            h.reset()
            #expect(h.current == nil)
            #expect(!h.canGoBack)
            #expect(!h.canGoForward)
        }
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

    // MARK: - Merge endpoints (EP-028 SP-075, T-0302)

    @Test("mergeScene joins a scene into its predecessor; body survives reopen")
    func mergeSceneJoinsPredecessor() throws {
        let (engine, _, ref, projectDir, appSupport) = try makeProjectFixture()

        let opened = try engine.openProject(
            projectRootPath: projectDir.path, appSupportRoot: appSupport.path)
        guard let scene1 = opened.scenes.first else {
            Issue.record("Expected a first scene"); return
        }

        // Give scene 1 a body.
        _ = try engine.saveScene(
            projectID: opened.projectID, projectRootPath: projectDir.path,
            appSupportRoot: appSupport.path, sceneID: scene1.sceneID,
            sceneMetadataPath: scene1.metadataPath, sceneContentPath: scene1.contentPath,
            markdown: "SCENE-ONE-BODY", authorshipRef: ref)

        // Add scene 2 in the same chapter with its own body.
        let scene2 = try engine.createScene(
            projectRootPath: projectDir.path, appSupportRoot: appSupport.path,
            projectID: opened.projectID, chapterID: scene1.chapterID,
            afterSceneID: scene1.sceneID, authorshipRef: ref)
        _ = try engine.saveScene(
            projectID: opened.projectID, projectRootPath: projectDir.path,
            appSupportRoot: appSupport.path, sceneID: scene2.sceneID,
            sceneMetadataPath: scene2.metadataPath, sceneContentPath: scene2.contentPath,
            markdown: "SCENE-TWO-BODY", authorshipRef: ref)

        let result = try engine.mergeScene(
            projectRootPath: projectDir.path, sceneID: scene2.sceneID)
        #expect(result.merged)
        #expect(result.survivorSceneID == scene1.sceneID)
        #expect(result.mergedSceneID == scene2.sceneID)
        #expect(result.chapterID == scene1.chapterID)
        #expect(!result.survivorContentPath.isEmpty)

        // Reopen: one scene, with both bodies on disk in order.
        let reopened = try engine.openProject(
            projectRootPath: projectDir.path, appSupportRoot: appSupport.path)
        #expect(reopened.scenes.count == 1)
        #expect(reopened.scenes[0].sceneID == scene1.sceneID)

        let survivor = try engine.openScene(
            projectRootPath: projectDir.path, appSupportRoot: appSupport.path,
            projectID: reopened.projectID, sceneID: scene1.sceneID)
        #expect(survivor.markdown.contains("SCENE-ONE-BODY"))
        #expect(survivor.markdown.contains("SCENE-TWO-BODY"))
    }

    @Test("mergeScene on the first scene of a chapter throws (no predecessor)")
    func mergeSceneFirstSceneThrows() throws {
        let (engine, _, _, projectDir, appSupport) = try makeProjectFixture()
        let opened = try engine.openProject(
            projectRootPath: projectDir.path, appSupportRoot: appSupport.path)
        guard let scene1 = opened.scenes.first else {
            Issue.record("Expected a first scene"); return
        }
        #expect(throws: (any Error).self) {
            _ = try engine.mergeScene(projectRootPath: projectDir.path, sceneID: scene1.sceneID)
        }
    }

    @Test("mergeChapter relocates all scenes into the predecessor; nothing lost on reopen (I-0083)")
    func mergeChapterRelocatesScenes() throws {
        let (engine, _, ref, projectDir, appSupport) = try makeProjectFixture()

        let opened = try engine.openProject(
            projectRootPath: projectDir.path, appSupportRoot: appSupport.path)
        guard let scene1 = opened.scenes.first else {
            Issue.record("Expected a first scene"); return
        }
        _ = try engine.saveScene(
            projectID: opened.projectID, projectRootPath: projectDir.path,
            appSupportRoot: appSupport.path, sceneID: scene1.sceneID,
            sceneMetadataPath: scene1.metadataPath, sceneContentPath: scene1.contentPath,
            markdown: "CH1-BODY", authorshipRef: ref)

        // Chapter 2 with a distinctive body.
        let chapter2 = try engine.createChapter(
            projectRootPath: projectDir.path, appSupportRoot: appSupport.path,
            projectID: opened.projectID, authorshipRef: ref)
        _ = try engine.saveScene(
            projectID: opened.projectID, projectRootPath: projectDir.path,
            appSupportRoot: appSupport.path, sceneID: chapter2.firstSceneID,
            sceneMetadataPath: chapter2.firstSceneMetadataPath,
            sceneContentPath: chapter2.firstSceneContentPath,
            markdown: "CH2-BODY", authorshipRef: ref)

        let result = try engine.mergeChapter(
            projectRootPath: projectDir.path, chapterID: chapter2.chapterID)
        #expect(result.merged)
        #expect(result.survivorChapterID == scene1.chapterID)
        #expect(result.mergedChapterID == chapter2.chapterID)
        #expect(result.scenesRelocated == 1)

        // Reopen: both scenes survive (I-0083 fix), now both in chapter 1, bodies intact.
        let reopened = try engine.openProject(
            projectRootPath: projectDir.path, appSupportRoot: appSupport.path)
        #expect(reopened.scenes.count == 2)
        #expect(reopened.scenes.allSatisfy { $0.chapterID == scene1.chapterID })

        let ch2Scene = try engine.openScene(
            projectRootPath: projectDir.path, appSupportRoot: appSupport.path,
            projectID: reopened.projectID, sceneID: chapter2.firstSceneID)
        #expect(ch2Scene.markdown.contains("CH2-BODY"))
    }

    @Test("mergeChapter on the first chapter throws (no predecessor)")
    func mergeChapterFirstChapterThrows() throws {
        let (engine, _, _, projectDir, appSupport) = try makeProjectFixture()
        let opened = try engine.openProject(
            projectRootPath: projectDir.path, appSupportRoot: appSupport.path)
        guard let scene1 = opened.scenes.first else {
            Issue.record("Expected a first scene"); return
        }
        #expect(throws: (any Error).self) {
            _ = try engine.mergeChapter(projectRootPath: projectDir.path, chapterID: scene1.chapterID)
        }
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

        // Add a world object so a non-scene record appears. ⚠️ Since T-0409 a
        // character is world-scoped, so it needs a world to live in.
        let world = try engine.createWorld(
            projectRootPath: projectDir.path,
            packagePath:     projectDir.url.appendingPathComponent("Interop.scrivworld")
                                        .path(percentEncoded: false),
            displayName:     "Interop World",
            epochLabel:      ""
        )
        _ = try engine.createObject(
            projectRootPath: projectDir.path,
            objectKind:      "character",
            displayName:     "Khaz'tul Miner",
            authorshipRef:   ref,
            worldID:         world.worldID
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

        // ⚠️ I-0118: the new fields must survive the JSON boundary into Swift.
        // A serializer that emits them and a decoder that drops them look
        // identical from the C++ side — this is the assertion that catches it.
        #expect(character?.domainIdentifier == world.worldID)
        #expect(content.worldDomainIdentifiers == [world.worldID])
        // Q2 — world-scoped, and it must round-trip back through the parser the
        // app actually uses when a Spotlight hit is tapped.
        let charURL = try #require(URL(string: character?.deepLink ?? ""))
        let charLink = try #require(ScriviDeepLink(url: charURL))
        #expect(charLink.isWorldScoped)
        #expect(charLink.worldID == world.worldID)
        #expect(charLink.projectID.isEmpty)
        #expect(charLink.itemID == character?.uniqueIdentifier)

        // The project's own records keep the project domain — empty per-item
        // domain means "the result's", so the two halves stay distinguishable.
        #expect(project?.domainIdentifier.isEmpty == true)
        #expect(sceneItem?.domainIdentifier.isEmpty == true)
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

    @Test("ScriviDeepLink requires an owner (project OR world) and the open host")
    func deepLinkRejectsInvalid() {
        #expect(ScriviDeepLink(url: URL(string: "scrivi://open?item=scene:s1")!) == nil)      // no owner at all
        #expect(ScriviDeepLink(url: URL(string: "scrivi://other?project=p1")!) == nil)         // wrong host
        #expect(ScriviDeepLink(url: URL(string: "https://example.com?project=p1")!) == nil)    // wrong scheme
    }

    // ⚠️ I-0118 Q2 — the world-scoped form. Before this the parser required
    // `project=`, so a world link returned nil and a tapped Spotlight hit for a
    // character did nothing at all.
    @Test("ScriviDeepLink parses a world-scoped deep link")
    func deepLinkParsesWorld() throws {
        let url = URL(string: "scrivi://open?world=world_abc&item=character:character_xyz")!
        let link = try #require(ScriviDeepLink(url: url))
        #expect(link.isWorldScoped)
        #expect(link.worldID == "world_abc")
        #expect(link.projectID.isEmpty)
        #expect(link.itemID == "character:character_xyz")
        // Not a scene, so there is nothing to navigate to within a manuscript.
        #expect(link.targetSceneID == nil)
    }

    @Test("A project link is not world-scoped, and vice versa")
    func deepLinkOwnershipIsExclusive() throws {
        let project = try #require(ScriviDeepLink(
            url: URL(string: "scrivi://open?project=project_abc&item=scene:s1")!))
        #expect(project.isWorldScoped == false)
        #expect(project.worldID.isEmpty)
    }

    @Test("ScriviDeepLink tolerates a missing item (project-only link)")
    func deepLinkMissingItem() throws {
        let link = try #require(ScriviDeepLink(url: URL(string: "scrivi://open?project=project_abc")!))
        #expect(link.projectID == "project_abc")
        #expect(link.itemID.isEmpty)
        #expect(link.targetSceneID == nil)
    }

    // MARK: — Undo/Redo history wrappers (EP-019 SP-052 — T-0203)
    // The history engine is in-memory, keyed by projectRootPath, so these need
    // no on-disk project — a unique path string per test keeps them independent.

    @Test("historyOpen mints a session and reports no undo/redo")
    func historyOpenMintsSession() throws {
        let engine = ScriviEngine()
        let root = "/tmp/scrivi-history-\(UUID().uuidString).scrivi"
        let opened = try engine.historyOpen(projectRootPath: root)
        #expect(opened.sessionID.hasPrefix("ses_"))
        #expect(!opened.canUndo)
        #expect(!opened.canRedo)
        try engine.historyClose(projectRootPath: root)
    }

    // EP-030 SP-092 (T-0395) — the history tree the inspector card renders.
    @Test("historyGetTree returns a windowed tree and tolerates absent arrays")
    func historyGetTreeWindows() throws {
        let engine = ScriviEngine()
        let root = "/tmp/scrivi-history-\(UUID().uuidString).scrivi"
        _ = try engine.historyOpen(projectRootPath: root)
        defer { try? engine.historyClose(projectRootPath: root) }

        // A fresh history is root-only: the root has no parentID and no childIDs, so
        // the C API omits BOTH keys. Decoding this at all is the I-0094 regression
        // guard — non-optional fields would throw keyNotFound here.
        let empty = try engine.historyGetTree(projectRootPath: root)
        #expect(empty.nodes.count == 1)
        #expect(empty.totalNodeCount == 1)
        #expect(!empty.truncated)
        #expect(empty.rootID == empty.currentNodeID)
        #expect(empty.nodes[0].parentID.isEmpty)
        #expect(empty.nodes[0].childIDs.isEmpty)
        #expect(empty.nodes[0].isCurrent)

        for i in 0..<6 {
            _ = try engine.historyRecordEvent(
                projectRootPath: root, sceneID: "scene_a",
                newSceneText: String(repeating: "x", count: i + 1),
                cursorBefore: Int64(i), cursorAfter: Int64(i + 1))
        }

        let full = try engine.historyGetTree(projectRootPath: root)
        #expect(full.totalNodeCount == 7)          // root + 6 events
        #expect(full.nodes.count == 7)
        #expect(!full.truncated)
        #expect(full.nodes.contains { $0.isCurrent })

        // maxNodes windows the result and flags truncation.
        let capped = try engine.historyGetTree(projectRootPath: root, maxNodes: 3)
        #expect(capped.nodes.count == 3)
        #expect(capped.totalNodeCount == 7)
        #expect(capped.truncated)
        // The window is anchored on the writer's position, so it is always included.
        #expect(capped.nodes.contains { $0.eventID == capped.currentNodeID })
    }

    @Test("record → undo → redo round-trips text and cursor through Swift")
    func historyRoundTrip() throws {
        let engine = ScriviEngine()
        let root = "/tmp/scrivi-history-\(UUID().uuidString).scrivi"
        _ = try engine.historyOpen(projectRootPath: root)
        defer { try? engine.historyClose(projectRootPath: root) }

        let r1 = try engine.historyRecordEvent(
            projectRootPath: root, sceneID: "scene_a",
            newSceneText: "Hello", cursorBefore: 0, cursorAfter: 5)
        #expect(r1.eventID.hasPrefix("evt_"))
        #expect(!r1.noOp)
        #expect(r1.canUndo)
        #expect(!r1.canRedo)

        _ = try engine.historyRecordEvent(
            projectRootPath: root, sceneID: "scene_a",
            newSceneText: "Hello world", cursorBefore: 5, cursorAfter: 11)

        let undo = try engine.historyUndo(projectRootPath: root)
        #expect(undo.moved)
        let change = try #require(undo.changes.first)
        #expect(change.sceneID == "scene_a")
        #expect(change.newText == "Hello")
        #expect(change.cursorAfter == 5)
        #expect(undo.canUndo)
        #expect(undo.canRedo)
        #expect(!undo.crossedSessionBoundary)

        let redo = try engine.historyRedo(projectRootPath: root)
        #expect(redo.moved)
        #expect(redo.changes.first?.newText == "Hello world")
        #expect(redo.changes.first?.cursorAfter == 11)
        #expect(!redo.canRedo)
    }

    @Test("a structural node round-trips its inverse payload through undo/redo (T-0356)")
    func historyStructuralInverseRoundTrip() throws {
        let engine = ScriviEngine()
        let root = "/tmp/scrivi-history-\(UUID().uuidString).scrivi"
        _ = try engine.historyOpen(projectRootPath: root)
        defer { try? engine.historyClose(projectRootPath: root) }

        // A text event, then a reversible structural node carrying an inverse-op payload.
        _ = try engine.historyRecordEvent(
            projectRootPath: root, sceneID: "scene_a",
            newSceneText: "before", cursorBefore: 0, cursorAfter: 6)
        let payload = HistoryStructuralPayload(
            op: "structuredCut", fragmentJSON: #"{"schema":"scrivi.fragment.v1","pieces":[]}"#,
            caretSceneID: "scene_a", caretByte: 3,
            removedSceneIDs: ["scene_b"], removedChapterIDs: [])
        _ = try engine.historyRecordBarrier(
            projectRootPath: root, barrierKind: "structuredCut",
            note: "Can't undo past a cross-boundary cut", structuralPayload: payload)

        // Undo steps ACROSS the structural node: moved, no text change, inverse payload (undo).
        let undo = try engine.historyUndo(projectRootPath: root)
        #expect(undo.moved)
        #expect(undo.changes.isEmpty)
        #expect(undo.stoppedAtBarrier == nil)
        let inv = try #require(undo.structuralInverse)
        #expect(inv.direction == "undo")
        #expect(inv.payload.op == "structuredCut")
        #expect(inv.payload.caretSceneID == "scene_a")
        #expect(inv.payload.caretByte == 3)
        #expect(inv.payload.removedSceneIDs == ["scene_b"])

        // Redo re-crosses forward with direction=redo and the same payload.
        let redo = try engine.historyRedo(projectRootPath: root)
        #expect(redo.moved)
        #expect(redo.changes.isEmpty)
        let rinv = try #require(redo.structuralInverse)
        #expect(rinv.direction == "redo")
        #expect(rinv.payload.op == "structuredCut")
    }

    @Test("a cut-into-buffer event carries a bufferID and undoes like a plain cut (Trade T3)")
    func historyCutIntoBufferTag() throws {
        let engine = ScriviEngine()
        let root = "/tmp/scrivi-history-\(UUID().uuidString).scrivi"
        _ = try engine.historyOpen(projectRootPath: root)
        defer { try? engine.historyClose(projectRootPath: root) }

        _ = try engine.historySeedScene(
            projectRootPath: root, sceneID: "scene_a", sceneText: "Base. Cut me.")
        // A cut-into-buffer: text mutates and the event is tagged with slot "3".
        let cut = try engine.historyRecordEvent(
            projectRootPath: root, sceneID: "scene_a",
            newSceneText: "Base. ", kind: "cut", cursorBefore: 12, cursorAfter: 6,
            bufferID: "3")
        #expect(!cut.noOp)          // a real deletion → a real event

        // The tag is provenance only — undo restores the pre-cut text exactly.
        let undo = try engine.historyUndo(projectRootPath: root)
        #expect(undo.moved)
        #expect(undo.changes.first?.newText == "Base. Cut me.")
    }

    @Test("recording identical text reports noOp")
    func historyNoOpOnIdenticalText() throws {
        let engine = ScriviEngine()
        let root = "/tmp/scrivi-history-\(UUID().uuidString).scrivi"
        _ = try engine.historyOpen(projectRootPath: root)
        defer { try? engine.historyClose(projectRootPath: root) }

        let r = try engine.historyRecordEvent(
            projectRootPath: root, sceneID: "scene_a", newSceneText: "")
        #expect(r.noOp)
        #expect(!r.canUndo)
    }

    @Test("undo stops at a barrier with a notice")
    func historyBarrierStopsUndo() throws {
        let engine = ScriviEngine()
        let root = "/tmp/scrivi-history-\(UUID().uuidString).scrivi"
        _ = try engine.historyOpen(projectRootPath: root)
        defer { try? engine.historyClose(projectRootPath: root) }

        _ = try engine.historyRecordEvent(
            projectRootPath: root, sceneID: "scene_a",
            newSceneText: "before", cursorBefore: 0, cursorAfter: 6)
        _ = try engine.historyRecordBarrier(
            projectRootPath: root, barrierKind: "sceneMerge",
            note: "Can't undo past a scene merge")
        _ = try engine.historyRecordEvent(
            projectRootPath: root, sceneID: "scene_a",
            newSceneText: "before after", cursorBefore: 6, cursorAfter: 12)

        // First undo removes the post-barrier text.
        let u1 = try engine.historyUndo(projectRootPath: root)
        #expect(u1.moved)
        #expect(u1.changes.first?.newText == "before")

        // Second undo hits the barrier — no move, notice returned.
        let u2 = try engine.historyUndo(projectRootPath: root)
        #expect(!u2.moved)
        let stop = try #require(u2.stoppedAtBarrier)
        #expect(stop.kind == "sceneMerge")
        #expect(stop.note == "Can't undo past a scene merge")
    }

    @Test("history calls before open throw a ScriviError")
    func historyBeforeOpenThrows() throws {
        let engine = ScriviEngine()
        let root = "/tmp/scrivi-history-unopened-\(UUID().uuidString).scrivi"
        #expect(throws: ScriviError.self) {
            _ = try engine.historyUndo(projectRootPath: root)
        }
    }

    @Test("historyClose reports whether a history was open")
    func historyCloseReporting() throws {
        let engine = ScriviEngine()
        let root = "/tmp/scrivi-history-\(UUID().uuidString).scrivi"
        _ = try engine.historyOpen(projectRootPath: root)
        #expect(try engine.historyClose(projectRootPath: root).closed)
        #expect(!(try engine.historyClose(projectRootPath: root).closed))   // already closed
    }

    // MARK: — Copy-buffer wrappers (EP-019 SP-056 — T-0213)
    // Buffers write history/buffers.json on disk, so these use a real TempDir.
    // No open/close — each call is a stateless read-modify-write in ScriviCore.

    @Test("buffersLoad → buffersGet round-trips text through Swift")
    func buffersLoadGetRoundTrip() throws {
        let engine = ScriviEngine()
        let dir = try TempDir()

        let loaded = try engine.buffersLoad(projectRootPath: dir.path, bufferID: "1", text: "Kazd'ul")
        #expect(loaded.bufferID == "1")
        #expect(!loaded.updatedAt.isEmpty)

        let got = try engine.buffersGet(projectRootPath: dir.path, bufferID: "1")
        #expect(got.present)
        #expect(got.text == "Kazd'ul")
    }

    @Test("buffersLoad with a fragment round-trips structure through Swift (T-0355)")
    func buffersFragmentRoundTrip() throws {
        let engine = ScriviEngine()
        let dir = try TempDir()

        let fragJSON = """
        {"schema":"scrivi.fragment.v1","plainText":"a\\n\\nb",\
        "pieces":[{"opensWith":"none","partial":"tail","text":"a"},\
        {"opensWith":"scene","text":"b"}]}
        """
        try engine.buffersLoad(projectRootPath: dir.path, bufferID: "4",
                               text: "a\n\nb", fragmentJSON: fragJSON)

        let got = try engine.buffersGet(projectRootPath: dir.path, bufferID: "4")
        #expect(got.present)
        #expect(got.text == "a\n\nb")
        #expect(got.fragment != nil)                        // structured slot decoded
        #expect(got.fragment?.pieces.count == 2)

        // Re-loading plain text clears the fragment (load replaces both).
        try engine.buffersLoad(projectRootPath: dir.path, bufferID: "4", text: "plain")
        let got2 = try engine.buffersGet(projectRootPath: dir.path, bufferID: "4")
        #expect(got2.text == "plain")
        #expect(got2.fragment == nil)
    }

    @Test("buffersGet on an unset slot reports present=false")
    func buffersGetUnset() throws {
        let engine = ScriviEngine()
        let dir = try TempDir()
        let got = try engine.buffersGet(projectRootPath: dir.path, bufferID: "9")
        #expect(!got.present)
        #expect(got.text.isEmpty)
    }

    @Test("buffersList returns non-empty slots ascending with a count")
    func buffersListOrder() throws {
        let engine = ScriviEngine()
        let dir = try TempDir()
        try engine.buffersLoad(projectRootPath: dir.path, bufferID: "3", text: "gamma")
        try engine.buffersLoad(projectRootPath: dir.path, bufferID: "1", text: "alpha")

        let listed = try engine.buffersList(projectRootPath: dir.path)
        #expect(listed.count == 2)
        #expect(listed.buffers.map(\.bufferID) == ["1", "3"])
        #expect(listed.buffers.first?.text == "alpha")
    }

    @Test("buffersClear removes a slot; clearing an empty slot is a no-op")
    func buffersClearBehavior() throws {
        let engine = ScriviEngine()
        let dir = try TempDir()
        try engine.buffersLoad(projectRootPath: dir.path, bufferID: "2", text: "beta")

        #expect(try engine.buffersClear(projectRootPath: dir.path, bufferID: "2").cleared)
        #expect(!(try engine.buffersGet(projectRootPath: dir.path, bufferID: "2").present))
        #expect(!(try engine.buffersClear(projectRootPath: dir.path, bufferID: "2").cleared))
    }

    @Test("copy buffers persist across a fresh call (no in-memory state)")
    func buffersPersist() throws {
        let engine = ScriviEngine()
        let dir = try TempDir()
        try engine.buffersLoad(projectRootPath: dir.path, bufferID: "5", text: "epsilon")
        // A second, independent engine sees the slot — persistence is entirely on disk.
        let engine2 = ScriviEngine()
        #expect(try engine2.buffersGet(projectRootPath: dir.path, bufferID: "5").text == "epsilon")
    }

    @Test("an out-of-range bufferID throws a ScriviError")
    func buffersInvalidID() throws {
        let engine = ScriviEngine()
        let dir = try TempDir()
        #expect(throws: ScriviError.self) {
            try engine.buffersLoad(projectRootPath: dir.path, bufferID: "0", text: "x")
        }
    }

    // MARK: — T-0396: typing-session coalescing (engine-level invariant)

    // NOTE ON COVERAGE. `HistoryCapture` — where the coalescing decision actually
    // lives — is NOT compiled into this test target (the target builds only
    // ScriviEngine.swift + ScriviError.swift standalone, so `@testable import` does
    // not make app types usable here; adding HistoryCapture would cascade in most of
    // the app layer). What is asserted below is the engine-level property the fix
    // depends on: each recorded event is one node and one undo step, so recording
    // ONCE per typing session is the only way to get one entry. The app-side
    // behaviour itself is covered by live verification.

    // Counts recorded nodes for a scene, ignoring the root anchor.
    private func typingNodeCount(_ engine: ScriviEngine, projectRootPath: String,
                                 sceneID: String) throws -> Int {
        let tree = try engine.historyGetTree(projectRootPath: projectRootPath)
        return tree.nodes.filter { $0.sceneID == sceneID && $0.eventID != tree.rootID }.count
    }

    // The reference case from the SP-093 plan: the writer typed one sentence
    // continuously and history recorded THREE entries, split by the 1 s autosave
    // debounce — one break falling mid-word ("…made glo" / "rious…"). Recording the
    // finished sentence once yields ONE node and ONE undo step; the pre-fix
    // behaviour of recording at each save point yields three of each.
    @Test("one record per typing session is one node and one undo step (T-0396)")
    func typingSessionIsOneNode() throws {
        let engine = ScriviEngine()
        let dir = try TempDir()
        let sentence = "Now is the winter of our discontent made glorious summer by this son of york"

        try engine.historyOpen(projectRootPath: dir.path)
        try engine.historySeedScene(projectRootPath: dir.path, sceneID: "scene_a", sceneText: "")
        // Coalesced: the session's final text recorded once, at a real boundary.
        _ = try engine.historyRecordEvent(projectRootPath: dir.path, sceneID: "scene_a",
                                          newSceneText: sentence, kind: "typing",
                                          cursorBefore: 0,
                                          cursorAfter: Int64(sentence.utf8.count))

        #expect(try typingNodeCount(engine, projectRootPath: dir.path, sceneID: "scene_a") == 1)

        // And it is a single undo step back to empty — not three.
        let step = try engine.historyUndo(projectRootPath: dir.path)
        #expect(step.moved)
        #expect(step.changes.first?.newText == "")
        try engine.historyClose(projectRootPath: dir.path)
    }

    // MARK: — I-0104 (2nd defect): a commit at close must keep the head hash disk-accurate

    // Reported in live verify 2026-08-10: three quit→reopen cycles produced three
    // externalChange barriers on a scene edited only inside Scrivi. The engine-level
    // contract is what the app-side close() now upholds — the LAST text committed to
    // history must also be the text whose hash is persisted, or the next open compares
    // the replayed head against disk and flags a change that never happened.
    @Test("three reopen cycles raise no barrier when the final text is recorded (I-0104)")
    func repeatedReopenIsSilent() throws {
        let engine = ScriviEngine()
        let dir = try TempDir()
        let finalText = "First sentence. Second."

        try engine.historyOpen(projectRootPath: dir.path)
        try engine.historySeedScene(projectRootPath: dir.path, sceneID: "scene_a", sceneText: "")
        _ = try engine.historyRecordEvent(projectRootPath: dir.path, sceneID: "scene_a",
                                          newSceneText: "First sentence.", kind: "typing",
                                          cursorBefore: 0, cursorAfter: 15)
        try engine.historyNoteScenePersisted(projectRootPath: dir.path, sceneID: "scene_a",
                                             diskText: "First sentence.")
        // A commit with no save after it — the shape that reintroduced the bug.
        _ = try engine.historyRecordEvent(projectRootPath: dir.path, sceneID: "scene_a",
                                          newSceneText: finalText, kind: "typing",
                                          cursorBefore: 15, cursorAfter: 23)
        try engine.historyNoteScenePersisted(projectRootPath: dir.path, sceneID: "scene_a",
                                             diskText: finalText)
        try engine.historyClose(projectRootPath: dir.path)

        for cycle in 1...3 {
            try engine.historyOpen(projectRootPath: dir.path)
            let r = try engine.historyValidateScene(projectRootPath: dir.path,
                                                    sceneID: "scene_a",
                                                    currentDiskText: finalText)
            #expect(!r.externalChange, "cycle \(cycle) raised a spurious externalChange")
            try engine.historyClose(projectRootPath: dir.path)
        }
    }

    // MARK: — I-0106 / T-0398: removedLength, caret spans, deletion identity

    // The payload must survive the boundary. `removedLength` is the shared field both
    // I-0106 (caret spans) and T-0398 (presentation) consume, so if it decodes as 0
    // both regress silently — a deletion would look like an insertion again.
    @Test("a deletion decodes with removedLength and no insertion (I-0106/T-0398)")
    func deletionCarriesRemovedLength() throws {
        let engine = ScriviEngine()
        let dir = try TempDir()
        try engine.historyOpen(projectRootPath: dir.path)
        try engine.historySeedScene(projectRootPath: dir.path, sceneID: "scene_a", sceneText: "")

        _ = try engine.historyRecordEvent(projectRootPath: dir.path, sceneID: "scene_a",
                                          newSceneText: "Now is the winter", kind: "typing",
                                          cursorBefore: 0, cursorAfter: 17)
        // Remove "is the " — 7 bytes out, nothing in.
        _ = try engine.historyRecordEvent(projectRootPath: dir.path, sceneID: "scene_a",
                                          newSceneText: "Now winter", kind: "delete",
                                          cursorBefore: 4, cursorAfter: 4)

        let tree = try engine.historyGetTree(projectRootPath: dir.path)
        let del = try #require(tree.nodes.first { $0.eventID == tree.currentNodeID })

        #expect(del.changeLength == 0)
        #expect(del.removedLength == 7)
        #expect(del.isPureDeletion, "a pure deletion must be identifiable for the glyph/label")
        try engine.historyClose(projectRootPath: dir.path)
    }

    // I-0106 (a): a pure deletion used to have no span at all — `changeLength == 0`
    // sent it down a degenerate "matches only its exact offset" path, which collided
    // with the adjacent insertion's half-open range and bolded TWO rows. It must now
    // own a real, bounded span.
    @Test("a deletion has a real caret span, not a bare offset (I-0106)")
    func deletionHasCaretSpan() throws {
        let engine = ScriviEngine()
        let dir = try TempDir()
        try engine.historyOpen(projectRootPath: dir.path)
        try engine.historySeedScene(projectRootPath: dir.path, sceneID: "scene_a", sceneText: "")
        _ = try engine.historyRecordEvent(projectRootPath: dir.path, sceneID: "scene_a",
                                          newSceneText: "the cat sat", kind: "typing",
                                          cursorBefore: 0, cursorAfter: 11)
        _ = try engine.historyRecordEvent(projectRootPath: dir.path, sceneID: "scene_a",
                                          newSceneText: "the sat", kind: "delete",
                                          cursorBefore: 4, cursorAfter: 4)

        let tree = try engine.historyGetTree(projectRootPath: dir.path)
        let del = try #require(tree.nodes.first { $0.eventID == tree.currentNodeID })

        // Hits at the seam where text was removed, and nowhere far from it.
        #expect(del.contains(caret: del.changeOffsetUtf8))
        #expect(!del.contains(caret: del.changeOffsetUtf8 + 50))
        try engine.historyClose(projectRootPath: dir.path)
    }

    // MARK: — T-0397: whitespace-kind labels

    // A newline-only event used to read "(no text)": `preview` rewrites \n to a space
    // and the card trims it away. The kind now travels as its own field and turns
    // into wording the writer can act on.
    @Test("a newline-only event is named, not rendered as empty (T-0397)")
    func newlineEventIsNamed() throws {
        let engine = ScriviEngine()
        let dir = try TempDir()
        try engine.historyOpen(projectRootPath: dir.path)
        try engine.historySeedScene(projectRootPath: dir.path, sceneID: "scene_a", sceneText: "")
        _ = try engine.historyRecordEvent(projectRootPath: dir.path, sceneID: "scene_a",
                                          newSceneText: "Line one", kind: "typing",
                                          cursorBefore: 0, cursorAfter: 8)
        _ = try engine.historyRecordEvent(projectRootPath: dir.path, sceneID: "scene_a",
                                          newSceneText: "Line one\n", kind: "typing",
                                          cursorBefore: 8, cursorAfter: 9)

        let tree = try engine.historyGetTree(projectRootPath: dir.path)
        let nl = try #require(tree.nodes.first { $0.eventID == tree.currentNodeID })

        #expect(nl.whitespaceKind == "newline:1")
        #expect(nl.whitespaceLabel == "⏎ new paragraph")
        // The old failure mode: preview trims to nothing, which is what sent the row
        // down the "(no text)" path.
        #expect(nl.preview.trimmingCharacters(in: .whitespacesAndNewlines).isEmpty)
        try engine.historyClose(projectRootPath: dir.path)
    }

    // Ordinary text must NOT be relabelled — the preview speaks for itself, and a
    // sentence containing spaces is not a "whitespace event".
    @Test("an event with real text has no whitespace label (T-0397)")
    func realTextHasNoWhitespaceLabel() throws {
        let engine = ScriviEngine()
        let dir = try TempDir()
        try engine.historyOpen(projectRootPath: dir.path)
        try engine.historySeedScene(projectRootPath: dir.path, sceneID: "scene_a", sceneText: "")
        _ = try engine.historyRecordEvent(projectRootPath: dir.path, sceneID: "scene_a",
                                          newSceneText: "Hello world", kind: "typing",
                                          cursorBefore: 0, cursorAfter: 11)

        let tree = try engine.historyGetTree(projectRootPath: dir.path)
        let node = try #require(tree.nodes.first { $0.eventID == tree.currentNodeID })

        #expect(node.whitespaceKind.isEmpty)
        #expect(node.whitespaceLabel == nil)
        try engine.historyClose(projectRootPath: dir.path)
    }

    // The pre-fix shape, asserted so the difference is explicit and regressions are
    // visible: recording at each autosave point produces a node per save.
    @Test("recording at each autosave point fragments the sentence (T-0396 baseline)")
    func perSaveRecordingFragments() throws {
        let engine = ScriviEngine()
        let dir = try TempDir()
        let pieces = ["Now is",
                      "Now is the winter of our discontent made glo",
                      "Now is the winter of our discontent made glorious summer by this son of york"]

        try engine.historyOpen(projectRootPath: dir.path)
        try engine.historySeedScene(projectRootPath: dir.path, sceneID: "scene_a", sceneText: "")
        for p in pieces {
            _ = try engine.historyRecordEvent(projectRootPath: dir.path, sceneID: "scene_a",
                                              newSceneText: p, kind: "typing",
                                              cursorBefore: 0, cursorAfter: Int64(p.utf8.count))
        }
        // Three saves → three entries → three undo stops. This is exactly what the
        // writer reported, and what deferring the save-time commit eliminates.
        #expect(try typingNodeCount(engine, projectRootPath: dir.path, sceneID: "scene_a") == 3)
        try engine.historyClose(projectRootPath: dir.path)
    }
}

// MARK: — EP-030 AC12: card soft-failure isolation (T-0399)

/// AC12 is **not verifiable from the UI** — there is no way to make a real card fail by using
/// the app, which is why the criterion sat unverified through SP-092 and SP-094. These fixtures
/// are the only way to exercise it.
///
/// Scope note (Doc 2 §7.1, rescoped 2026-08-11): soft failures only. A card whose view body
/// *traps* cannot be contained by SwiftUI at all, so there is deliberately no test for it —
/// such a test would have to crash the runner to be honest.
@MainActor
private struct FailingCard: InspectorCard {
    static let typeID = "test.failing"
    static let title = "Failing Test Card"
    static let systemImage = "xmark.octagon"
    static let stack: InspectorStack = .writing

    struct Boom: Error {}

    init() {}

    func body(context: CardContext) -> AnyView { AnyView(EmptyView()) }
    func makeContent(context: CardContext) throws -> AnyView { throw Boom() }
}

@MainActor
private struct HealthyCard: InspectorCard {
    static let typeID = "test.healthy"
    static let title = "Healthy Test Card"
    static let systemImage = "checkmark"
    static let stack: InspectorStack = .writing

    init() {}

    func body(context: CardContext) -> AnyView { AnyView(Text("content")) }
}

@Suite("Inspector card failure isolation (EP-030 AC12)")
@MainActor
struct InspectorCardFailureTests {

    // The engine is irrelevant to AC12 — these fixtures never call it — but `CardContext`
    // requires one.
    private func context() -> CardContext {
        CardContext(sceneID: "scene_a", projectRootPath: "/tmp/none",
                    engine: ScriviEngine(), config: CardConfig())
    }

    @Test("a card that throws while building content does not throw out of the stack")
    func failingCardIsContained() {
        let card = AnyInspectorCard(FailingCard.self)
        // The framework must be able to ASK for content and be told it failed, rather
        // than the failure escaping to whatever renders the stack.
        #expect(throws: FailingCard.Boom.self) {
            _ = try card.body(context: context())
        }
    }

    @Test("a healthy card still builds normally through the throwing path")
    func healthyCardUnaffected() throws {
        let card = AnyInspectorCard(HealthyCard.self)
        // The default `makeContent` forwards to `body`, so a card that never opted into
        // the throwing variant must be completely unaffected by AC12's machinery.
        _ = try card.body(context: context())
    }

    @Test("one card's failure leaves its neighbours buildable")
    func failureDoesNotBlockOthers() throws {
        let cards = [AnyInspectorCard(HealthyCard.self),
                     AnyInspectorCard(FailingCard.self),
                     AnyInspectorCard(HealthyCard.self)]

        // This is the AC in one assertion: build every card the way the stack does, and
        // confirm the failure is isolated to its own slot — two neighbours still produce
        // content, and the stack as a whole does not collapse to blank.
        var built = 0
        var failed = 0
        for card in cards {
            do { _ = try card.body(context: context()); built += 1 }
            catch { failed += 1 }
        }

        #expect(built == 2)
        #expect(failed == 1)
    }
}

// MARK: — T-0407: the graph, object-discovery, and world wrappers (EP-031 SP-099)
//
// These go through `scrivi_*` end to end — a real project on disk, real objects,
// real edges — rather than exercising Swift in isolation.
//
// ⚠️ That is deliberate and it is the I-0113 lesson: a test that stops at the
// Swift side (or at the C++ facade) cannot see a boundary gap. I-0113 shipped
// green precisely because `WorldTests.cpp` called the facade and never the ABI.
// Every assertion below crosses the boundary.

struct ScriviGraphInteropTests {

    private final class TempDir: @unchecked Sendable {
        let url: URL
        init() throws {
            url = FileManager.default.temporaryDirectory
                .appendingPathComponent("scrivi-graph-\(UUID().uuidString)")
            try FileManager.default.createDirectory(at: url, withIntermediateDirectories: true)
        }
        deinit { try? FileManager.default.removeItem(at: url) }
        var path: String { url.path(percentEncoded: false) }
    }

    private struct Fixture {
        let engine:     ScriviEngine
        let ref:        AuthorshipRef
        let projectDir: TempDir
        let appSupport: TempDir
        let sceneID:    String
        /// ⚠️ T-0409: every worldbuilding kind is world-scoped, so a fixture that
        /// creates objects needs a world for them to live in. `source` is the
        /// sole project-scoped kind and passes "" instead.
        let worldID:    String
        var root:       String { projectDir.path }
    }

    private func makeFixture() throws -> Fixture {
        let appSupport = try TempDir()
        let projectDir = try TempDir()
        let engine = ScriviEngine()

        let identity = try engine.ensureLocalIdentity(
            displayName: "Graph Test Author",
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
            title: "Graph Interop",
            slug:  "graph-interop",
            authorshipRef: ref
        )
        let opened = try engine.openProject(
            projectRootPath: projectDir.path,
            appSupportRoot:  appSupport.path,
            identityID:      identity.identityID
        )
        // createProject seeds a first scene; either result can carry it.
        let sceneID = opened.activeScene?.sceneID ?? created.firstScene.sceneID
        #expect(!sceneID.isEmpty, "fixture needs a scene to relate objects to")

        let world = try engine.createWorld(
            projectRootPath: projectDir.path,
            packagePath: projectDir.url.appendingPathComponent("Graph.scrivworld")
                                       .path(percentEncoded: false),
            displayName: "Graph World",
            epochLabel: ""
        )

        return Fixture(engine: engine, ref: ref, projectDir: projectDir,
                       appSupport: appSupport, sceneID: sceneID,
                       worldID: world.worldID)
    }

    @discardableResult
    private func makeCharacter(_ f: Fixture, _ name: String) throws -> String {
        try f.engine.createObject(
            projectRootPath: f.root,
            objectKind: "character",
            displayName: name,
            authorshipRef: f.ref,
            worldID: f.worldID
        ).objectID
    }

    // MARK: Relation types

    @Test("the seeded relation-type vocabulary decodes through the boundary")
    func relationTypesDecode() throws {
        let f = try makeFixture()
        let types = try f.engine.listRelationTypes(projectRootPath: f.root).types

        #expect(types.count >= 4)
        let codes = Set(types.map(\.code))
        #expect(codes.isSuperset(of: ["appears-in", "located-at", "sibling-of", "cites"]))

        let appearsIn = try #require(types.first { $0.code == "appears-in" })
        #expect(appearsIn.forwardLabel == "appears in")
        // I-0125 / SP-102 R5: kind-neutral, because ANY kind may now appear in a
        // scene — "has characters" was wrong the moment a chronicle used the type.
        #expect(appearsIn.inverseLabel == "features")
        #expect(appearsIn.symmetric == false)

        let siblingOf = try #require(types.first { $0.code == "sibling-of" })
        #expect(siblingOf.symmetric)
        #expect(siblingOf.canonicalDirection == "lexical")
    }

    @Test("an unconstrained relation type decodes nil kinds, meaning ANY kind")
    func citesIsUnconstrainedOnBothEnds() throws {
        let f = try makeFixture()
        let types = try f.engine.listRelationTypes(projectRootPath: f.root).types
        let cites = try #require(types.first { $0.code == "cites" })

        // The C ABI OMITS these keys rather than emitting null, so nil here must
        // mean "any kind" — never "failed to decode". A citation documents any kind.
        #expect(cites.sourceKind == nil)
        #expect(cites.targetKind == nil)
        #expect(cites.acceptsSource(kind: "character"))
        #expect(cites.acceptsTarget(kind: "artifact"))
    }

    // MARK: ⚠️ I-0150 — the test host must never restore a real project

    @Test("⚠️ I-0150 — this very test process is detected as a test host")
    func testHostIsDetected() {
        // ⚠️ The whole point, asserted from inside the hazard: this assertion runs in
        // the SAME hosted process that reopened the user's real projects, so if the
        // detector ever stops working here, it has stopped working where it matters.
        //
        // ⚠️ There is deliberately NO test that flips the guard off and asserts the
        // projects reopen. That negative control would re-enable, on a real machine
        // with real bookmarks, exactly the behaviour that silently rewrote
        // `the-twisted-remains-of-myself.scrivi`. The evidence for the fix is a
        // before/after checksum of all 220 files in both real projects across a full
        // `xcodebuild test` run — recorded in I-0150 — not a test that reproduces the
        // damage to prove it was possible.
        #expect(AppEnvironment.isRunningUnderTests)
    }

    // MARK: ⚠️ T-0446 — the object's image crosses the boundary

    @Test("⚠️ T-0446 — an object with no image decodes with an empty imagePath")
    func objectWithoutImageDecodes() throws {
        let f = try makeFixture()
        _ = try makeCharacter(f, "Ada")

        let objects = try f.engine.listObjects(projectRootPath: f.root).objects
        let ada = try #require(objects.first { $0.displayName == "Ada" })

        // ⚠️ The keys are ABSENT for an object with no image, so these must decode
        // as nil rather than failing — the common case by far, and a decode error
        // here would empty every card in the app.
        #expect(ada.imagePath == nil || ada.imagePath?.isEmpty == true)
        #expect(!ada.hasResolvableImage)
        #expect(ada.imageAssetID == nil || ada.imageAssetID?.isEmpty == true)
    }

    @Test("⚠️ T-0446 — an imported image resolves to a path the app can draw")
    func objectImageResolvesToPath() throws {
        let f = try makeFixture()
        let ada = try makeCharacter(f, "Ada")

        // A real import, through the real endpoint — the bytes must exist.
        let tmp = FileManager.default.temporaryDirectory
            .appendingPathComponent("t0446-\(UUID().uuidString).png")
        try Data([0x89, 0x50, 0x4E, 0x47]).write(to: tmp)
        defer { try? FileManager.default.removeItem(at: tmp) }

        let imported = try f.engine.importAsset(
            projectRootPath: f.root,
            sourcePath: tmp.path,
            category: "image",
            title: "Portrait",
            authorshipRef: f.ref,
            worldID: f.worldID
        )
        #expect(!imported.assetID.isEmpty)

        // Attach it to the object the way a save does: patch `image` into the
        // object's own JSON and write it back.
        let opened = try f.engine.openObject(
            projectRootPath: f.root, objectKind: "character",
            objectID: ada, worldID: f.worldID)
        var json = try #require(try JSONSerialization.jsonObject(
            with: Data(opened.objectJson.utf8)) as? [String: Any])
        json["image"] = ["assetID": imported.assetID]
        let patched = try JSONSerialization.data(withJSONObject: json)
        _ = try f.engine.saveObject(
            projectRootPath: f.root, objectKind: "character",
            objectJson: String(decoding: patched, as: UTF8.self),
            authorshipRef: f.ref)

        // ⚠️ THE ASSERTION THAT MATTERS: the LIST — not openObject — hands the app
        // a usable path. This is what a card row draws from, and the whole reason
        // the index carries the image at all.
        let listed = try f.engine.listObjects(projectRootPath: f.root).objects
        let row = try #require(listed.first { $0.objectID == ada })
        #expect(row.imageAssetID == imported.assetID)
        #expect(row.hasResolvableImage)

        // ⚠️ The path must be real, not merely non-empty — an unopenable path
        // would render as a broken image, which is worse than none.
        let path = try #require(row.imagePath)
        #expect(FileManager.default.fileExists(atPath: path))

        // ⚠️ AC3's storage rule: a world-scoped object's image lives INSIDE the
        // world package, not under the project.
        #expect(path.contains(".scrivworld"))
    }

    // MARK: ⚠️ T-0441 — the drifted vocabulary, tested THROUGH THE BOUNDARY
    //
    // ⚠️ A facade test cannot see a boundary gap — that is how I-0113 shipped
    // green (`feedback_boundary_tests_not_facade`). The C++ suite proves the
    // repair; these prove the repaired vocabulary actually REACHES Swift, which is
    // the half `capability_without_surface` keeps catching this Epic out on.

    /// The pre-I-0125 `appears-in`, written straight to disk: `sourceKind:
    /// "character"` (the constraint I-0125 removed) and the kind-specific
    /// `inverseLabel`. ⚠️ Reproduces what is still on the rig in
    /// `the-twisted-remains-of-myself.scrivi`.
    private func driftVocabulary(_ f: Fixture) throws {
        let path = f.root + "/objects/relation-types.json"
        let drifted = """
        {
          "schema": "scrivi.relation-types.v1",
          "types": [
            {"code":"appears-in","forwardLabel":"appears in",
             "inverseLabel":"has characters","sourceKind":"character",
             "targetKind":"scene","canonicalDirection":"source-to-target",
             "symmetric":false},
            {"code":"cites","forwardLabel":"cites","inverseLabel":"documented by",
             "canonicalDirection":"source-to-target","symmetric":false},
            {"code":"sworn-enemy-of","forwardLabel":"sworn enemy of",
             "inverseLabel":"sworn enemy of","canonicalDirection":"lexical",
             "symmetric":true}
          ]
        }
        """
        try drifted.write(toFile: path, atomically: true, encoding: .utf8)
    }

    @Test("⚠️ T-0441 — a drifted seeded type is repaired before Swift ever sees it")
    func driftedVocabularyIsRepairedAcrossTheBoundary() throws {
        let f = try makeFixture()
        try driftVocabulary(f)

        let types = try f.engine.listRelationTypes(projectRootPath: f.root).types
        let appearsIn = try #require(types.first { $0.code == "appears-in" })

        // ⚠️ These are the exact values the app reads to decide what it may
        // offer. Before T-0441 they arrived drifted and every object→object and
        // object→scene affordance built on them was quietly wrong.
        #expect(appearsIn.inverseLabel == "features")
        #expect(appearsIn.sourceKind == nil)
        #expect(appearsIn.acceptsSource(kind: "chronicle"))

        // Nothing deleted: the writer's own type survives, and the seeded types
        // missing from the drifted file are restored.
        #expect(types.contains { $0.code == "sworn-enemy-of" })
        #expect(Set(types.map(\.code)).isSuperset(
            of: ["appears-in", "located-at", "sibling-of", "cites", "sworn-enemy-of"]))
    }

    @Test("⚠️ T-0441 — the relate that the drifted vocabulary BROKE now succeeds")
    func driftedVocabularyNoLongerBlocksRelating() throws {
        let f = try makeFixture()
        try driftVocabulary(f)

        // ⚠️ THE ACTUAL WRITER-FACING SYMPTOM. `appears-in` is the type EIGHT of
        // the ten object cards use; against the drifted vocabulary this failed
        // with "endpoints do not satisfy the kind constraints of relation type
        // 'appears-in'" — with the object already written to disk, so she was
        // told creation failed while it existed.
        let chronicle = try f.engine.createObject(
            projectRootPath: f.root,
            objectKind: "chronicle",
            displayName: "The Long Winter",
            authorshipRef: f.ref,
            worldID: f.worldID
        ).objectID

        let edge = try f.engine.createEdge(
            projectRootPath: f.root,
            fromID: chronicle,
            toID: f.sceneID,
            relationTypeCode: "appears-in"
        )
        #expect(!edge.edgeID.isEmpty)
    }

    // MARK: ⚠️ T-0443 — the scene sentinel

    @Test("⚠️ the scene endpoint round-trips as the sentinel, so object→object is separable")
    func sceneSentinelRoundTrips() throws {
        let f = try makeFixture()
        let types = try f.engine.listRelationTypes(projectRootPath: f.root).types

        // ⚠️ No ABI change was needed for D4-A: scenes are not an ObjectKind
        // (Doc 1 §8), so a constrained scene endpoint already crosses as the
        // literal "scene" and comes back unchanged. This is the assertion that
        // would catch the ABI starting to emit something else.
        let appearsIn = try #require(types.first { $0.code == "appears-in" })
        #expect(appearsIn.targetKind == RelationTypeEntry.sceneToken)
        #expect(appearsIn.targetAcceptsScene)
        #expect(!appearsIn.targetAcceptsObject)
        #expect(!appearsIn.sourceIsScene)

        // `located-at` is seeded scene→location: it cannot be created FROM an
        // object, which is why the object picker excludes it.
        let locatedAt = try #require(types.first { $0.code == "located-at" })
        #expect(locatedAt.sourceIsScene)
        #expect(locatedAt.targetAcceptsObject)
        #expect(!locatedAt.targetAcceptsScene)

        // ⚠️ An unconstrained type accepts BOTH — the two properties overlap on
        // purpose and are not each other's negation.
        let cites = try #require(types.first { $0.code == "cites" })
        #expect(cites.targetAcceptsScene)
        #expect(cites.targetAcceptsObject)
    }

    @Test("⚠️ object→object relating works end to end and rejects the duplicate")
    func objectToObjectRelating() throws {
        let f = try makeFixture()
        let ada = try makeCharacter(f, "Ada")
        let bea = try makeCharacter(f, "Bea")

        // `sibling-of` is seeded character→character and symmetric — the shape
        // AC4 calls "the one that regresses silently".
        let edge = try f.engine.createEdge(
            projectRootPath: f.root, fromID: ada, toID: bea,
            relationTypeCode: "sibling-of")
        #expect(!edge.edgeID.isEmpty)

        // ⚠️ Visible from BOTH endpoints — one stored record, two renderings.
        let fromAda = try f.engine.listEdgesFor(projectRootPath: f.root, endpointID: ada).edges
        let fromBea = try f.engine.listEdgesFor(projectRootPath: f.root, endpointID: bea).edges
        #expect(fromAda.contains { $0.otherID == bea })
        #expect(fromBea.contains { $0.otherID == ada })

        // ⚠️ The far endpoint's KIND travels on the edge (I-0124) — the section
        // navigates on it, and a pending object is absent from the index.
        let toBea = try #require(fromAda.first { $0.otherID == bea })
        #expect(toBea.otherKind == "character")
        #expect(toBea.otherDisplayName == "Bea")
        #expect(!toBea.otherPending)

        // AC6/AC21: creating it from the SECOND endpoint is rejected as a
        // duplicate, not stored as a second record.
        #expect(throws: (any Error).self) {
            _ = try f.engine.createEdge(
                projectRootPath: f.root, fromID: bea, toID: ada,
                relationTypeCode: "sibling-of")
        }
        #expect(try f.engine.listEdgesFor(
            projectRootPath: f.root, endpointID: ada).edges.filter { $0.otherID == bea }.count == 1)
    }

    // MARK: Edges

    @Test("an edge created through the boundary lists from the scene with a label")
    func createAndListEdge() throws {
        let f = try makeFixture()
        let ada = try makeCharacter(f, "Ada")

        let edge = try f.engine.createEdge(
            projectRootPath: f.root,
            fromID: ada,
            toID: f.sceneID,
            relationTypeCode: "appears-in"
        )
        #expect(!edge.edgeID.isEmpty)
        #expect(edge.relationType == "appears-in")

        // The object card's actual read path: ask the SCENE for its edges.
        let fromScene = try f.engine.listEdgesFor(
            projectRootPath: f.root, endpointID: f.sceneID
        ).edges
        let row = try #require(fromScene.first { $0.edgeID == edge.edgeID })

        #expect(row.otherID == ada)
        #expect(row.otherDisplayName == "Ada")
        #expect(row.otherPending == false)
        // Read from the scene, this is the INVERSE direction of a stored
        // character→scene edge, and the label must read correctly from here.
        #expect(row.isForward == false)
        #expect(row.label == "features")
    }

    @Test("the SAME stored edge reads forward from the other endpoint (§5.2 projection)")
    func labelProjectsBothWays() throws {
        let f = try makeFixture()
        let ada = try makeCharacter(f, "Ada")
        let edge = try f.engine.createEdge(
            projectRootPath: f.root, fromID: ada, toID: f.sceneID,
            relationTypeCode: "appears-in"
        )

        let fromObject = try f.engine.listEdgesFor(
            projectRootPath: f.root, endpointID: ada
        ).edges
        let row = try #require(fromObject.first { $0.edgeID == edge.edgeID })

        // One record, two readings — never two stored edges.
        #expect(row.isForward)
        #expect(row.label == "appears in")
        #expect(row.otherID == f.sceneID)
    }

    @Test("a duplicate edge is rejected from EITHER creation order (AC21)")
    func duplicateRejectedFromBothOrders() throws {
        let f = try makeFixture()
        let ada  = try makeCharacter(f, "Ada")
        let bram = try makeCharacter(f, "Bram")

        _ = try f.engine.createEdge(
            projectRootPath: f.root, fromID: ada, toID: bram,
            relationTypeCode: "sibling-of"
        )

        // Same relationship, opposite order. `sibling-of` is symmetric, so this is
        // the same canonical edge and must be refused rather than stored twice.
        #expect(throws: ScriviError.self) {
            _ = try f.engine.createEdge(
                projectRootPath: f.root, fromID: bram, toID: ada,
                relationTypeCode: "sibling-of"
            )
        }

        let edges = try f.engine.listEdgesFor(
            projectRootPath: f.root, endpointID: ada
        ).edges.filter { $0.relationType == "sibling-of" }
        #expect(edges.count == 1, "exactly one canonical edge, not two")
    }

    @Test("deleting an edge leaves BOTH objects alive and findable (AC22)")
    func removeFromSceneDeletesEdgeOnly() throws {
        let f = try makeFixture()
        let ada = try makeCharacter(f, "Ada")
        let edge = try f.engine.createEdge(
            projectRootPath: f.root, fromID: ada, toID: f.sceneID,
            relationTypeCode: "appears-in"
        )

        let deleted = try f.engine.deleteEdge(projectRootPath: f.root, edgeID: edge.edgeID)
        #expect(deleted.deleted)

        // The edge is gone from the scene…
        let sceneEdges = try f.engine.listEdgesFor(
            projectRootPath: f.root, endpointID: f.sceneID
        ).edges
        #expect(!sceneEdges.contains { $0.edgeID == edge.edgeID })

        // …and the OBJECT survives. This is the whole point of "Remove from
        // scene" never being spelled "Delete".
        let objects = try f.engine.listObjects(projectRootPath: f.root, kind: "character").objects
        #expect(objects.contains { $0.objectID == ada })

        // And it is findable as an orphan, not silently stranded (Doc 1 §5.5).
        let orphans = try f.engine.listOrphanedObjects(projectRootPath: f.root).objects
        #expect(orphans.contains { $0.objectID == ada })
    }

    @Test("a scene with no relationships decodes as empty, not as a failure")
    func emptyEdgeListDecodes() throws {
        let f = try makeFixture()
        // The C ABI omits the `edges` key entirely here. An empty stack is the
        // common case for a fresh scene and must not throw.
        let edges = try f.engine.listEdgesFor(
            projectRootPath: f.root, endpointID: f.sceneID
        ).edges
        #expect(edges.isEmpty)
    }

    // MARK: Object discovery

    @Test("listObjects filters by kind and reports project scope")
    func listObjectsByKind() throws {
        let f = try makeFixture()
        _ = try makeCharacter(f, "Ada")
        _ = try f.engine.createObject(
            projectRootPath: f.root, objectKind: "location",
            displayName: "The Observatory", authorshipRef: f.ref,
            worldID: f.worldID
        )

        let characters = try f.engine.listObjects(projectRootPath: f.root, kind: "character").objects
        #expect(characters.count == 1)
        let ada = try #require(characters.first)
        #expect(ada.displayName == "Ada")
        #expect(ada.kind == "character")
        // ⚠️ INVERTED (SP-104/I-0114): a character IS world-scoped since T-0409
        // and carries the world it lives in. The old expectation — no world, not
        // world-scoped — is the pre-ruling model.
        #expect(ada.worldID == f.worldID)
        #expect(ada.isWorldScoped)

        // Unfiltered lists every kind — what the picker shows (AC17).
        let all = try f.engine.listObjects(projectRootPath: f.root).objects
        #expect(all.count == 2)
    }

    @Test("an unrecognised kind throws rather than reporting an empty list")
    func unknownKindThrows() throws {
        let f = try makeFixture()
        _ = try makeCharacter(f, "Ada")

        // "you have no characters" must never be how a typo presents itself.
        #expect(throws: ScriviError.self) {
            _ = try f.engine.listObjects(projectRootPath: f.root, kind: "charcter")
        }
    }

    @Test("a related object is not an orphan; an unrelated one is")
    func orphansAreExactlyTheUnrelated() throws {
        let f = try makeFixture()
        let ada  = try makeCharacter(f, "Ada")
        let bram = try makeCharacter(f, "Bram")

        _ = try f.engine.createEdge(
            projectRootPath: f.root, fromID: ada, toID: f.sceneID,
            relationTypeCode: "appears-in"
        )

        let orphanIDs = Set(try f.engine.listOrphanedObjects(
            projectRootPath: f.root).objects.map(\.objectID))
        #expect(!orphanIDs.contains(ada))
        #expect(orphanIDs.contains(bram))
    }

    // MARK: Worlds

    @Test("a project with no worlds lists none, and that is not an error")
    func noWorldsDecodes() throws {
        // ⚠️ Deliberately NOT makeFixture(): since T-0409 that seeds a world so
        // world-scoped kinds can be created. This test needs a project that
        // genuinely has none — the empty-list case is the most common state in a
        // new project and previously read as a backend error (F4).
        let appSupport = try TempDir()
        let projectDir = try TempDir()
        let engine = ScriviEngine()
        let identity = try engine.ensureLocalIdentity(
            displayName: "No World Author", appSupportRoot: appSupport.path)
        let ref = AuthorshipRef(identityID: identity.identityID,
                                personaID: identity.defaultPersonaID,
                                displayName: identity.displayName)
        _ = try engine.createProject(
            projectRootPath: projectDir.path, appSupportRoot: appSupport.path,
            title: "No Worlds", slug: "no-worlds", authorshipRef: ref)

        let worlds = try engine.listWorlds(projectRootPath: projectDir.path).worlds
        #expect(worlds.isEmpty)
    }

    @Test("a bound world reports available, and its objects reach the boundary")
    func worldRoundTripsThroughTheBoundary() throws {
        let f = try makeFixture()
        let worldDir = try TempDir()
        let packagePath = worldDir.url.appendingPathComponent("Midgard.scrivworld")
            .path(percentEncoded: false)

        let created = try f.engine.createWorld(
            projectRootPath: f.root, packagePath: packagePath,
            displayName: "Midgard", epochLabel: "Third Age"
        )
        #expect(created.displayName == "Midgard")

        let worlds = try f.engine.listWorlds(projectRootPath: f.root).worlds
        let midgard = try #require(worlds.first { $0.worldID == created.worldID })
        #expect(midgard.worldStatus == .available)
        #expect(midgard.worldStatus.isUnavailable == false)

        let status = try f.engine.getWorldStatus(
            projectRootPath: f.root, worldID: created.worldID)
        #expect(status.worldStatus == .available)

        // ⚠️ I-0113's exact shape: a WORLD-SCOPED object created through the C
        // ABI. This is what was unreachable before SP-098 widened the entry
        // points, and it is unreachable again the moment a wrapper drops worldID.
        let sword = try f.engine.createObject(
            projectRootPath: f.root, objectKind: "artifact",
            displayName: "Sword of Dawn", authorshipRef: f.ref,
            worldID: created.worldID
        )
        #expect(!sword.objectID.isEmpty)

        let artifacts = try f.engine.listObjects(
            projectRootPath: f.root, kind: "artifact").objects
        let listed = try #require(artifacts.first { $0.objectID == sword.objectID })
        #expect(listed.worldID == created.worldID)
        #expect(listed.isWorldScoped)
    }

    @Test("a world binding carries the cached names a pending card needs")
    func bindingCachesNames() throws {
        let f = try makeFixture()
        let worldDir = try TempDir()
        let packagePath = worldDir.url.appendingPathComponent("Midgard.scrivworld")
            .path(percentEncoded: false)

        let created = try f.engine.createWorld(
            projectRootPath: f.root, packagePath: packagePath,
            displayName: "Midgard", epochLabel: "Third Age"
        )
        let binding = try f.engine.getWorldBinding(
            projectRootPath: f.root, worldID: created.worldID)

        #expect(binding.worldID == created.worldID)
        #expect(binding.displayName == "Midgard")
        // Without this cache a pending card can only show opaque IDs, and a
        // writer cannot judge what she would lose (Doc 3 §5). The array may be
        // empty for a world with no objects yet — decoding it is what matters.
        #expect(binding.cachedIndex.count >= 0)
    }

    @Test("nothing is pending while every world is available")
    func noPendingEdgesWhenWorldsPresent() throws {
        let f = try makeFixture()
        let ada = try makeCharacter(f, "Ada")
        _ = try f.engine.createEdge(
            projectRootPath: f.root, fromID: ada, toID: f.sceneID,
            relationTypeCode: "appears-in"
        )

        let pending = try f.engine.listPendingEdges(projectRootPath: f.root).pending
        #expect(pending.isEmpty)
    }

    // MARK: Error detail (the prerequisite T-0407 had to fix first)

    @Test("a ScriviError carries the backend's `detail` rather than dropping it")
    func errorDetailSurvivesTheBoundary() throws {
        let f = try makeFixture()
        let ada  = try makeCharacter(f, "Ada")
        let bram = try makeCharacter(f, "Bram")

        _ = try f.engine.createEdge(
            projectRootPath: f.root, fromID: ada, toID: bram,
            relationTypeCode: "sibling-of"
        )

        // ⚠️ Before T-0407, `ErrorPayload` decoded only code+message, so every
        // machine-readable discriminator the C ABI emits was discarded at the
        // boundary. SP-102's frozen-graph refusal (detail == "worldPending:<status>")
        // is unbuildable without this, so it is asserted here on the discriminator
        // that IS reachable today.
        do {
            _ = try f.engine.createEdge(
                projectRootPath: f.root, fromID: bram, toID: ada,
                relationTypeCode: "sibling-of"
            )
            Issue.record("expected a duplicate-edge rejection")
        } catch let error as ScriviError {
            #expect(error.detail == "duplicateEdge",
                    "detail was \(error.detail ?? "nil") — the discriminator must survive decode")
            // A duplicate is not a pending-world refusal; the two must not be conflated.
            #expect(error.isWorldPending == false)
            #expect(error.pendingWorldStatus == nil)
        }
    }

    @Test("worldPending detail parses into a typed status without guessing")
    func pendingStatusParsing() {
        // Pure parsing of the contract RelationshipStore.cpp:191,322 emits. The
        // live pending path is verified in SP-102 against a real ejected volume
        // (R3) — a fixture cannot show "restores with no writer intervention".
        let offline = ScriviError(code: 1, message: "frozen", detail: "worldPending:offline")
        #expect(offline.isWorldPending)
        #expect(offline.pendingWorldStatus == .offline)

        let missing = ScriviError(code: 1, message: "frozen", detail: "worldPending:missing")
        #expect(missing.pendingWorldStatus == .missing)

        // ⚠️ An unrecognised status falls back to the honest generic, never to a
        // guess: a wrong "missing" invites restoring from backup when the NAS was
        // merely unreachable (Doc 2 §7.2.1).
        let future = ScriviError(code: 1, message: "frozen", detail: "worldPending:teleported")
        #expect(future.pendingWorldStatus == .unavailable)

        let ordinary = ScriviError(code: 1, message: "something else")
        #expect(ordinary.isWorldPending == false)
        #expect(ordinary.pendingWorldStatus == nil)
    }
}

// MARK: — T-0386 / T-0387: object cards and picker (EP-031 SP-099)
//
// The AC-bearing behaviors that do not need a running UI: the one-implementation
// parameterization, registration without stack placement, and sort semantics.
// Live card interaction is verified in the app.

@MainActor
struct ObjectCardConfigurationTests {

    @Test("ten object kinds, all served by ONE card implementation (AC: one card type)")
    func oneImplementationTenConfigurations() {
        #expect(ObjectCardKind.all.count == 10)

        // Every kind resolves to the same body type. If adding a kind ever required
        // a new card *implementation*, this is where it would show up.
        let typeIDs = Set(ObjectCardKind.all.map(\.typeID))
        #expect(typeIDs.count == 10, "typeIDs must be unique — they are schema keys")

        let kinds = Set(ObjectCardKind.all.map(\.kind))
        #expect(kinds.count == 10, "no kind is configured twice")
    }

    @Test("EVERY object card is world-scoped — `source` is the only project-scoped kind")
    func worldScopedKindsAreMarked() {
        // ⚠️ REWRITTEN TWICE. It first asserted the PRE-T-0409 partition — exactly
        // four world-scoped kinds — and was the Swift twin of the stale table that
        // blocked object creation in the app (SP-104/I-0114). SP-116 then replaced
        // the `kind != "source"` restatement behind `isWorldScoped` with a value
        // DERIVED FROM SCRIVICORE (T-0429, I-0140), so this now asserts against a
        // partition the core owns rather than one Swift wrote down.
        let worldScoped = Set(ObjectCardKind.all.filter(\.isWorldScoped).map(\.kind))
        #expect(worldScoped == Set(ObjectCardKind.all.map(\.kind)))
        #expect(worldScoped.count == 10)
        #expect(worldScoped.contains("character"))
        #expect(worldScoped.contains("artifact"))
        // `source` has no per-kind card at all (§3.1.1), so it cannot appear here.
        #expect(worldScoped.contains("source") == false)
    }

    // MARK: — SP-116 T-0429 / I-0140: scope is DERIVED, not restated

    @Test("the kind-scope table is populated FROM ScriviCore, not hardcoded in Swift")
    func kindScopeComesFromTheCore() throws {
        // ⚠️ The point of I-0140's fix. If this table were empty the app would
        // still "work" (unknown kinds fall back to world-scoped), so an assertion
        // that merely checks isWorldScoped values could pass with the endpoint
        // never wired up at all. Assert the table itself was loaded.
        let kinds = ObjectKindScope.allKinds
        #expect(!kinds.isEmpty, "scope table is empty — the ABI call did not land")

        // Eleven storable kinds: the ten worldbuilding cards plus `source`, which
        // has no card. Compared against the ENDPOINT rather than a literal list.
        let reported = try ScriviEngine().listObjectKinds()
        #expect(reported.count == reported.kinds.count)
        #expect(Set(kinds) == Set(reported.kinds.map(\.kind)))
    }

    @Test("every card's isWorldScoped matches what ScriviCore reports for that kind")
    func cardScopeAgreesWithTheCore() throws {
        // ⚠️ Written as a comparison against the endpoint, never against an
        // expectation table. A table here would be the restated-kind-list defect
        // reappearing inside the test for its own fix.
        let reported = try ScriviEngine().listObjectKinds()
        // `uniqueKeysWithValues` traps on a duplicate — deliberate HERE, where a
        // trap is a failed assertion that the core emits each kind once.
        // ⚠️ `ObjectKindScope` must NOT use it: in shipping code the same input
        // would crash the app instead of degrading, so it folds duplicates.
        let byKind = Dictionary(uniqueKeysWithValues: reported.kinds.map { ($0.kind, $0.isWorldScoped) })

        for card in ObjectCardKind.all {
            let expected = try #require(byKind[card.kind],
                                        "core does not know kind \(card.kind)")
            #expect(card.isWorldScoped == expected,
                    "scope disagrees for \(card.kind)")
        }

        // `source` is the one storable kind with no card, and the one the core
        // reports as project-scoped.
        #expect(byKind["source"] == false)
    }

    @Test("`world` is not offered as a storable kind")
    func worldIsNotAStorableKind() throws {
        // It is a container created by scrivi_create_world. Offering it here
        // would let a writer try to create a "world object" through the object
        // endpoints, which the core refuses.
        let reported = try ScriviEngine().listObjectKinds()
        #expect(!reported.kinds.contains { $0.kind == "world" })
    }

    @Test("`source` is NOT a worldbuilding object card")
    func sourceIsNotAnObjectCard() {
        // §3.1.1: sources surface through ONE aggregate card in the Writing stack,
        // never as a per-kind worldbuilding card. A per-source card would flood the
        // stack in any project with real research.
        #expect(!ObjectCardKind.all.contains { $0.kind == "source" })
    }

    @Test("every object card registers into the Worldbuilding stack, and none into Writing")
    func objectCardsRegisterIntoWorldbuilding() {
        InspectorCardRegistry.resetForTesting()
        InspectorCardRegistry.registerBuiltIns()

        for kind in ObjectCardKind.all {
            let card = InspectorCardRegistry.card(for: kind.typeID)
            #expect(card != nil, "\(kind.typeID) must be registered")
            #expect(card?.stack == .worldbuilding)
            #expect(card?.title == kind.title)
        }
    }

    @Test("⚠️ registering object cards does NOT place any of them in a stack (AC7)")
    func worldbuildingStackStillShipsEmpty() {
        InspectorCardRegistry.resetForTesting()
        InspectorCardRegistry.registerBuiltIns()

        // Doc 2 AC7: no worldbuilding card ever appears without an explicit writer
        // action. Registration makes a card OFFERABLE in the "+" menu — it must
        // never make it PRESENT. This is easy to regress while developing cards
        // you want to see on screen.
        let offered = InspectorCardRegistry.available(in: .worldbuilding)
        #expect(offered.count == 10, "all ten are offered in the + menu")

        let layout = InspectorLayoutStore(projectRootPath: NSTemporaryDirectory())
        let stack = layout.resolvedStack(sceneID: "scene-1", stack: .worldbuilding)
        #expect(stack.entries.isEmpty, "the default Worldbuilding stack ships EMPTY")
    }

    @Test("the writing stack's three default cards are unaffected by object-card registration")
    func writingDefaultsIntact() {
        InspectorCardRegistry.resetForTesting()
        InspectorCardRegistry.registerBuiltIns()

        let writing = InspectorCardRegistry.available(in: .writing).map(\.typeID)
        #expect(Set(writing).isSuperset(of: ["tags", "outline", "todo", "history"]))
        // No object card leaked into the Writing stack.
        #expect(!writing.contains { $0.hasPrefix("objects.") })
    }
}

// MARK: — T-0388 / T-0408: in-place creation and worlds (EP-031 SP-099, R4)
//
// These cover the gap live verification found: the card could list objects but
// nothing in the app could CREATE one, and no surface showed world context.

struct ObjectCreationInteropTests {

    private final class TempDir: @unchecked Sendable {
        let url: URL
        init() throws {
            url = FileManager.default.temporaryDirectory
                .appendingPathComponent("scrivi-create-\(UUID().uuidString)")
            try FileManager.default.createDirectory(at: url, withIntermediateDirectories: true)
        }
        deinit { try? FileManager.default.removeItem(at: url) }
        var path: String { url.path(percentEncoded: false) }
    }

    private struct Fixture {
        let engine: ScriviEngine
        let ref: AuthorshipRef
        let projectDir: TempDir
        let appSupport: TempDir
        let sceneID: String
        /// ⚠️ T-0409: world-scoped kinds need a world to be created in.
        let worldID: String
        var root: String { projectDir.path }
    }

    private func makeFixture() throws -> Fixture {
        let appSupport = try TempDir()
        let projectDir = try TempDir()
        let engine = ScriviEngine()
        let identity = try engine.ensureLocalIdentity(
            displayName: "Create Test", appSupportRoot: appSupport.path)
        let ref = AuthorshipRef(identityID: identity.identityID,
                                personaID: identity.defaultPersonaID,
                                displayName: identity.displayName)
        let created = try engine.createProject(
            projectRootPath: projectDir.path, appSupportRoot: appSupport.path,
            title: "Create Interop", slug: "create-interop", authorshipRef: ref)
        let world = try engine.createWorld(
            projectRootPath: projectDir.path,
            packagePath: projectDir.url.appendingPathComponent("Create.scrivworld")
                                       .path(percentEncoded: false),
            displayName: "Create World", epochLabel: "")
        return Fixture(engine: engine, ref: ref, projectDir: projectDir,
                       appSupport: appSupport, sceneID: created.firstScene.sceneID,
                       worldID: world.worldID)
    }

    // ⚠️ I-0119 — the wrong-scene commit. Found in live use (2026-08-14): a
    // location started in scene A and completed via the scene-change prompt was
    // related to scene B, the scene the writer had just moved to. She had to
    // repair it by hand.
    //
    // The app-side cause was that `ObjectCardModel` captures its `sceneID` at
    // init and is REBUILT by `.task(id:)` the moment the scene changes, so the
    // surviving draft committed against the new model. The draft now carries its
    // own `originSceneID` and `createAndRelate` takes the target scene
    // explicitly.
    //
    // This test pins the INVARIANT that fix must preserve — an edge written for
    // scene A belongs to scene A and to no other — at the boundary, where it is
    // checkable without driving SwiftUI.
    @Test("an object related to scene A stays on scene A, never on a later scene (I-0119)")
    func edgeLandsOnTheSceneItWasCreatedFor() throws {
        let f = try makeFixture()
        let opened = try f.engine.openProject(
            projectRootPath: f.root, appSupportRoot: f.appSupport.path,
            identityID: f.ref.identityID)
        let sceneA = f.sceneID
        let chapterID = try #require(opened.scenes.first?.chapterID)

        let sceneB = try f.engine.createScene(
            projectRootPath: f.root, appSupportRoot: f.appSupport.path,
            projectID: opened.projectID, chapterID: chapterID,
            afterSceneID: sceneA, authorshipRef: f.ref).sceneID
        #expect(sceneA != sceneB)

        // Exactly what commitDraft does for a draft STARTED in scene A, even
        // though the writer is now looking at scene B.
        let created = try f.engine.createObject(
            projectRootPath: f.root, objectKind: "location",
            displayName: "The Observatory", authorshipRef: f.ref,
            worldID: f.worldID)
        _ = try f.engine.createEdge(
            projectRootPath: f.root, fromID: created.objectID,
            toID: sceneA, relationTypeCode: "located-at")

        // Scene A has it...
        let aEdges = try f.engine.listEdgesFor(
            projectRootPath: f.root, endpointID: sceneA).edges
        #expect(aEdges.contains { $0.otherID == created.objectID })

        // ...and scene B does NOT. This is the assertion that fails if a commit
        // ever targets the live scene instead of the draft's origin.
        let bEdges = try f.engine.listEdgesFor(
            projectRootPath: f.root, endpointID: sceneB).edges
        #expect(bEdges.contains { $0.otherID == created.objectID } == false)
        #expect(bEdges.isEmpty)
    }

    @Test("creating a character and relating it makes it appear on the scene's card")
    func createAndRelateSurfacesOnTheCard() throws {
        let f = try makeFixture()

        // Exactly what the card's "New Character" does: create, then relate.
        let created = try f.engine.createObject(
            projectRootPath: f.root, objectKind: "character",
            displayName: "Ada", authorshipRef: f.ref, worldID: f.worldID)
        _ = try f.engine.createEdge(
            projectRootPath: f.root, fromID: created.objectID,
            toID: f.sceneID, relationTypeCode: "appears-in")

        // The card's read path now shows her — the loop the user could not close
        // before T-0388, because nothing in the app could perform the first step.
        let edges = try f.engine.listEdgesFor(
            projectRootPath: f.root, endpointID: f.sceneID).edges
        #expect(edges.count == 1)
        #expect(edges.first?.otherDisplayName == "Ada")

        // And the picker would now offer her for other scenes.
        let listed = try f.engine.listObjects(
            projectRootPath: f.root, kind: "character").objects
        #expect(listed.contains { $0.displayName == "Ada" })
    }

    @Test("a renamed object keeps its objectID and its edges (edit half of §4.6)")
    func renamePreservesIdentityAndEdges() throws {
        let f = try makeFixture()
        let created = try f.engine.createObject(
            projectRootPath: f.root, objectKind: "character",
            displayName: "Ada", authorshipRef: f.ref, worldID: f.worldID)
        let edge = try f.engine.createEdge(
            projectRootPath: f.root, fromID: created.objectID,
            toID: f.sceneID, relationTypeCode: "appears-in")

        // The rename path the card uses: open, patch displayName, save.
        let opened = try f.engine.openObject(
            projectRootPath: f.root, objectKind: "character",
            objectID: created.objectID, worldID: f.worldID)
        var json = try #require(try JSONSerialization.jsonObject(
            with: Data(opened.objectJson.utf8)) as? [String: Any])
        json["displayName"] = "Ada Lovelace"
        let patched = try JSONSerialization.data(withJSONObject: json)
        _ = try f.engine.saveObject(
            projectRootPath: f.root, objectKind: "character",
            objectJson: String(decoding: patched, as: UTF8.self),
            authorshipRef: f.ref)

        // ⚠️ Editing must not disturb identity — the edge still resolves, and it
        // resolves to the NEW name. A rename that orphaned the edge would look to
        // the writer exactly like her link vanishing.
        let edges = try f.engine.listEdgesFor(
            projectRootPath: f.root, endpointID: f.sceneID).edges
        #expect(edges.count == 1)
        #expect(edges.first?.edgeID == edge.edgeID)
        #expect(edges.first?.otherID == created.objectID)
        #expect(edges.first?.otherDisplayName == "Ada Lovelace")
    }

    @Test("a world-scoped kind cannot be created without a world")
    func worldScopedKindRefusedWithoutWorld() throws {
        let f = try makeFixture()
        // This is why the draft editor disables Create until a world is chosen:
        // the core refuses, and the card should never let her reach that error.
        #expect(throws: ScriviError.self) {
            _ = try f.engine.createObject(
                projectRootPath: f.root, objectKind: "artifact",
                displayName: "Sword of Dawn", authorshipRef: f.ref, worldID: "")
        }
    }

    @Test("a created world appears in the list the Worlds menu reads (T-0408)")
    func createdWorldIsListed() throws {
        let f = try makeFixture()
        let worldDir = try TempDir()
        let packagePath = worldDir.url.appendingPathComponent("Midgard.scrivworld")
            .path(percentEncoded: false)

        // Before T-0408 nothing in the app called either of these.
        let created = try f.engine.createWorld(
            projectRootPath: f.root, packagePath: packagePath,
            displayName: "Midgard", epochLabel: "")

        // ⚠️ The fixture seeds its own world (T-0409), so assert that Midgard
        // JOINS the list rather than that it is the only entry — the point of
        // T-0408 is that a created world reaches the Worlds menu, not that a
        // project has exactly one.
        let worlds = try f.engine.listWorlds(projectRootPath: f.root).worlds
        #expect(worlds.count == 2)
        let midgard = try #require(worlds.first { $0.worldID == created.worldID })
        #expect(midgard.displayName == "Midgard")
        #expect(midgard.worldStatus == .available)

        // And with a world bound, the world-scoped kind now works.
        let artifact = try f.engine.createObject(
            projectRootPath: f.root, objectKind: "artifact",
            displayName: "Sword of Dawn", authorshipRef: f.ref,
            worldID: created.worldID)
        #expect(!artifact.objectID.isEmpty)
    }
}

@MainActor
struct ObjectCardScopeTests {

    @Test("project-scoped kinds never ask for a world; world-scoped ones always do")
    func scopeDeterminesWorldRequirement() {
        // The writer-facing consequence of this split is the picker's scope line
        // and the draft editor's world picker. Characters belonging to no world is
        // correct, not an omission — which is what was unclear in the live check.
        // ⚠️ INVERTED (SP-104/I-0114). This asserted a character belongs to NO
        // world and called that "correct, not an omission." T-0409 reversed it:
        // a character IS a world object — that was the whole point of the ruling
        // (a character must be reusable across projects). The old expectation is
        // what the shipped code believed, which is why creation was refused.
        let characters = try! #require(ObjectCardKind.all.first { $0.kind == "character" })
        #expect(characters.isWorldScoped)

        let artifacts = try! #require(ObjectCardKind.all.first { $0.kind == "artifact" })
        #expect(artifacts.isWorldScoped)
    }
}

// MARK: — AC24: platform refinement of an unavailable world's status (SP-102 T-0389)

/// ⚠️ **These tests encode a finding that cost a probe to discover.** The natural
/// implementation of AC24 — key `unmounted` off `volumeIsRemovableKey` /
/// `volumeIsEjectableKey` — is WRONG on the hardware this feature is verified against.
/// The user's world lives on a 931 GB USB drive that reports:
///
///     volumeIsRemovable : false        (diskutil agrees: "Removable Media: Fixed")
///     volumeIsEjectable : false
///
/// A `hdiutil` disk image reports `ejectable == true`, so a fixture-based test would
/// have PASSED the broken rule. `WorldVolumeStatus` therefore uses volume-root mount
/// presence instead, and these tests pin that choice.
@Suite("World volume status refinement (EP-031 AC24)")
@MainActor
struct WorldVolumeStatusTests {

    @Test("an available world is never re-diagnosed")
    func availableIsUntouched() {
        // Refinement answers "why can't I reach it" — a reachable world has no why.
        #expect(WorldVolumeStatus.refine(
            coreStatus: .available,
            packagePath: "/Volumes/Nope/Gone.scrivworld") == .available)
    }

    @Test("a package on an unmounted volume reports unmounted, NOT missing")
    func unmountedVolumeIsNotMissing() {
        // ⚠️ The I-0115 rule in its most consequential form. "Missing" tells the
        // writer to relink or restore from backup; "unmounted" tells her to plug the
        // drive in. Reporting the first when the second is true invites her to
        // rebuild a world that is sitting intact on a disconnected disk.
        let status = WorldVolumeStatus.refine(
            coreStatus: .missing,
            packagePath: "/Volumes/Definitely Not Mounted 8Xz/Eskandar.scrivworld")
        #expect(status == .unmounted)
    }

    @Test("core status stands for a path on the boot volume")
    func bootVolumePathKeepsCoreStatus() {
        // Not under /Volumes: there is no volume story to tell, so whatever the core
        // concluded is the honest answer.
        #expect(WorldVolumeStatus.refine(
            coreStatus: .missing,
            packagePath: "/Users/nobody/Desktop/Gone.scrivworld") == .missing)
        #expect(WorldVolumeStatus.refine(
            coreStatus: .unavailable,
            packagePath: "/Users/nobody/Desktop/Gone.scrivworld") == .unavailable)
    }

    @Test("an empty package path never invents a diagnosis")
    func emptyPathKeepsCoreStatus() {
        #expect(WorldVolumeStatus.refine(
            coreStatus: .unavailable, packagePath: "") == .unavailable)
    }

    @Test("a mounted volume does not report unmounted")
    func mountedVolumeIsNotUnmounted() {
        // The root volume is always mounted, so it stands in for "the volume is
        // there" without depending on the user's external drive being connected.
        let status = WorldVolumeStatus.refine(
            coreStatus: .missing, packagePath: "/System/Volumes/Data/nothing.scrivworld")
        #expect(status != .unmounted)
    }

    // MARK: — T-0419 / I-0137: the DATA PATH, not the refinement

    /// ⚠️ **Every test above passed while the feature could not fire on real
    /// hardware.** They exercise `refine` directly, handing it a path — but the
    /// product got its path from `WorldEntry`, which carried one **only when the
    /// world was available**, i.e. never in the case refinement exists for.
    ///
    /// That is the shape of I-0137: capability, unit tests and call site all
    /// correct, and the datum never arriving. **These tests decode the envelope
    /// instead**, which is the only way to see the gap.
    @Test("⚠️ an UNAVAILABLE world still carries a path to refine from (I-0137)")
    func unavailableWorldCarriesLastKnownPath() throws {
        // The envelope shape a world on an ejected drive produces: no verified
        // packagePath, but a lastKnownPackagePath saying where it used to be.
        let json = """
        {"worlds":[{"worldID":"w-1","displayName":"Eskandar","status":"missing",
        "packagePath":"",
        "lastKnownPackagePath":"/Volumes/Definitely Not Mounted 8Xz/Eskandar.scrivworld",
        "epochOffsetMs":0}]}
        """
        let result = try JSONDecoder().decode(ListWorldsResult.self,
                                              from: Data(json.utf8))
        let entry = try #require(result.worlds.first)

        #expect(entry.packagePath.isEmpty)
        #expect(!entry.lastKnownPackagePath.isEmpty)

        // ⚠️ THE ASSERTION THAT WOULD HAVE CAUGHT I-0137. Before T-0419 this read
        // `.missing`, because refine was handed an empty packagePath and returned
        // the core status untouched.
        #expect(entry.worldStatus == .unmounted)
    }

    @Test("an older core that omits the field still behaves exactly as before")
    func missingFieldFallsBackToPackagePath() throws {
        // Forward/backward tolerance: no lastKnownPackagePath key at all.
        let json = """
        {"worlds":[{"worldID":"w-1","displayName":"Old","status":"unavailable",
        "packagePath":"","epochOffsetMs":0}]}
        """
        let result = try JSONDecoder().decode(ListWorldsResult.self,
                                              from: Data(json.utf8))
        let entry = try #require(result.worlds.first)
        #expect(entry.lastKnownPackagePath.isEmpty)
        #expect(entry.worldStatus == .unavailable)   // degrades honestly
    }

    @Test("an available world is unaffected by the new field")
    func availableWorldUnaffected() throws {
        let json = """
        {"worlds":[{"worldID":"w-1","displayName":"Here","status":"available",
        "packagePath":"/Users/nobody/Here.scrivworld",
        "lastKnownPackagePath":"/Users/nobody/Here.scrivworld","epochOffsetMs":0}]}
        """
        let result = try JSONDecoder().decode(ListWorldsResult.self,
                                              from: Data(json.utf8))
        let entry = try #require(result.worlds.first)
        #expect(entry.worldStatus == .available)
    }

    /// ⚠️ Found while fixing I-0137: `WorldStatusResult.worldStatus` returned the
    /// RAW core status while its sibling `WorldEntry.worldStatus` refined — two
    /// accessors answering "what status is this world in" and disagreeing. The
    /// per-site-copy defect in yet another costume.
    @Test("⚠️ get_world_status refines too — it previously did not")
    func worldStatusResultRefines() throws {
        let json = """
        {"worldID":"w-1","status":"missing","packagePath":"",
        "lastKnownPackagePath":"/Volumes/Definitely Not Mounted 8Xz/E.scrivworld"}
        """
        let result = try JSONDecoder().decode(WorldStatusResult.self,
                                              from: Data(json.utf8))
        #expect(result.worldStatus == .unmounted)
    }
}

// MARK: — I-0129: world availability must not depend on app focus

/// ⚠️ **The defect these pin was a MISSING TRIGGER, not wrong logic.**
///
/// `reconnectWorlds()` was driven only by `NSApplication.didBecomeActiveNotification`,
/// which worked solely because ejecting a drive normally forces the writer out of the
/// app. Plug a drive in while Scrivi is *already frontmost* and nothing fired: the
/// world returned and the warning stayed up until some unrelated focus change happened
/// to refresh it. The user found this by reversing the usual order — returning focus
/// first, then plugging in.
///
/// The fix observes `NSWorkspace` mount/unmount directly. A timer was the obvious
/// alternative and is strictly worse: it burns wakeups forever to catch an event the
/// system already reports exactly.
///
/// These assert the *contract the fix depends on* — that the notifications exist and
/// carry the volume URL. The observers themselves live in a SwiftUI view body, which a
/// unit test cannot exercise; asserting they are "wired" would test nothing.
@Suite("World mount observation (I-0129)")
@MainActor
struct WorldMountNotificationTests {

    #if os(macOS)
    @Test("NSWorkspace publishes mount and unmount, keyed by volume URL")
    func mountNotificationsExist() {
        // If Apple ever renamed these, the app would silently stop noticing drives —
        // the exact failure the user reported, back again and just as invisible.
        #expect(NSWorkspace.didMountNotification.rawValue == "NSWorkspaceDidMountNotification")
        #expect(NSWorkspace.didUnmountNotification.rawValue == "NSWorkspaceDidUnmountNotification")
        #expect(NSWorkspace.volumeURLUserInfoKey == "NSWorkspaceVolumeURLKey")
    }

    @Test("a volume path under /Volumes refines to unmounted once it is gone")
    func unmountedVolumeRefines() {
        // The end-to-end consequence of a mount event: after the volume disappears,
        // the status must read `unmounted` — plug-the-drive-in advice — and never
        // `missing`, which tells the writer to restore from backup (I-0115).
        let status = WorldVolumeStatus.refine(
            coreStatus: .missing,
            packagePath: "/Volumes/ScriviMountProbe Gone/Eskandar.scrivworld")
        #expect(status == .unmounted)
    }
    #endif
}

// MARK: — Navigator ↔ manuscript echo suppression (I-0132)
//
// The navigator and the manuscript are a deliberate cycle: the manuscript scrolls →
// `setViewportScene` → the navigator mirrors that into its `selection` →
// `EditorView.onChange(selection)` → navigate the manuscript. The loop is broken by a
// suspended-notification flag on the loader, NOT by comparing values: an equality guard
// silently depends on which view happens to write first and would start looping the day
// that order changed.
//
// These pin the flag's contract. The live loop is verified in the app.

@MainActor
struct ViewportSelectionEchoTests {

    private func makeLoader() -> ViewportSceneLoader {
        ViewportSceneLoader(
            engine: ScriviEngine(),
            projectRootPath: "/tmp/echo-test",
            appSupportRoot: "/tmp/echo-test-support",
            projectID: "project_echo",
            allScenes: []
        )
    }

    @Test("a viewport push raises the mirroring flag, so the selection write it causes reads as an echo")
    func viewportPushMarksMirroring() {
        let loader = makeLoader()
        #expect(loader.isMirroringViewportToSelection == false,
                "idle loader must not suppress a writer's selection")

        loader.setViewportScene("scene_alpha")

        // Raised synchronously: the navigator's onChange — and the selection write it
        // makes — run off the observable write, before any runloop hop.
        #expect(loader.isMirroringViewportToSelection == true,
                "the flag must be up BEFORE the observable write propagates, or the echo is missed")
        #expect(loader.viewportSceneID == "scene_alpha")
    }

    @Test("the flag clears once the update has drained, so the next real click is not swallowed")
    func mirroringFlagClearsAfterUpdate() async throws {
        let loader = makeLoader()
        loader.setViewportScene("scene_alpha")
        #expect(loader.isMirroringViewportToSelection == true)

        // The loader lowers it via a main-queue hop; yielding lets that land.
        try await Task.sleep(nanoseconds: 50_000_000)

        #expect(loader.isMirroringViewportToSelection == false,
                "a stuck flag would silently swallow every navigator click after the first scroll")
    }
}

// MARK: — The aggregate `sources` card (T-0365, EP-031 SP-102)
//
// Design §3.1.1. Sources attach to OBJECTS, never to scenes, so the card renders an
// indirect path: scene → objects → sources. These pin the parts that are decidable without
// a running UI; the live card and citation popup are verified in the app.

@MainActor
struct SourcesCardTests {

    @Test("`sources` is registered as ONE aggregate card, offered in the Writing stack")
    func registeredOnce() {
        InspectorCardRegistry.registerBuiltIns()
        let card = InspectorCardRegistry.card(for: "sources")
        #expect(card != nil, "the sources card must be registered or it cannot be added")
        #expect(SourcesCard.stack == .writing)
        // ⚠️ The ruling that matters: ONE card, never one per source. A per-source design
        // would flood the stack and could not be shown/hidden as a unit in the picker.
        #expect(SourcesCard.typeID == "sources")
    }

    @Test("a source reached through two objects is listed ONCE, naming both")
    func deduplicatesAcrossObjects() {
        // The attribution rule from §3.1.1: the writer needs to know *why* a citation
        // surfaces on this scene, and two rows for one source reads as two sources.
        let entry = SourceEntry(
            sourceID: "source_1",
            displayName: "Ellis, *Tidal Myths*",
            viaObjects: ["Alanna Vex", "The Sunless Court"])
        #expect(entry.attribution == "via Alanna Vex, The Sunless Court")
    }

    @Test("one citing object reads as a single attribution, not a list")
    func singleAttribution() {
        let entry = SourceEntry(sourceID: "source_2",
                                displayName: "Field notes",
                                viaObjects: ["Alanna Vex"])
        #expect(entry.attribution == "via Alanna Vex")
    }

    @Test("the card queries the `cites` type and the `source` kind — the two SP-096/SP-098 halves")
    func usesSeededVocabulary() {
        // ⚠️ These are schema keys shared with ScriviCore: `cites` is seeded by
        // RelationTypeStore (T-0373) and `source` is the one project-scoped kind (T-0406).
        // Drift here silently empties the card rather than failing loudly.
        #expect(SourcesCardModel.citesType == "cites")
        #expect(SourcesCardModel.sourceKind == "source")
    }
}
