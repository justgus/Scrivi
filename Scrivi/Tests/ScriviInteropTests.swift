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

        // MARK: — EP-034 SP-120 T-0453: the `attributes` map

        /// ⚠️ **THE test of this Task, and it is written FIRST.**
        ///
        /// `attributes` serializes as an ARRAY of `{"k":…,"v":…}` pairs
        /// (`ObjectJson.cpp:46-50`), **not** as a JSON object. Writing
        /// `{"author": "…"}` would parse back as an EMPTY map — every citation
        /// field silently dropped, with the object file looking perfectly
        /// reasonable to a human reading it.
        ///
        /// ⚠️ **This is not a hypothetical.** It is the identical shape trap
        /// T-0449 hit with `tags`, where a plain string array read back empty.
        /// The bug is invisible to any test that does not round-trip.
        @Test("attributes survive a save→reopen round trip in their {k,v} wire form")
        func attributesRoundTrip() throws {
            let json = """
            { "objectID": "source_1", "displayName": "A Wizard of Earthsea" }
            """

            let first = try ObjectDetail(json: json, kind: "source")
            #expect(first.attributes.isEmpty)

            let patched = try first.applyingEdits(
                displayName: "A Wizard of Earthsea",
                subtitle: "Le Guin, 1968",
                notes: "Cited for the naming magic.",
                attributes: ["author": "Ursula K. Le Guin", "year": "1968"]
            )

            // The wire form is an ARRAY of pairs — assert the shape, because an
            // object would decode as nothing.
            let root = try #require(try JSONSerialization.jsonObject(
                with: Data(patched.utf8)) as? [String: Any])
            let raw = try #require(root["attributes"] as? [[String: Any]])
            #expect(raw.count == 2)
            #expect(raw.allSatisfy { $0["k"] is String && $0["v"] is String })

            // ...and it must come BACK. This is the half that catches the trap.
            let reopened = try ObjectDetail(json: patched, kind: "source")
            #expect(reopened.attributes["author"] == "Ursula K. Le Guin")
            #expect(reopened.attributes["year"] == "1968")
        }

        /// ⚠️ Follows the `tags` precedent exactly: `nil` means "not edited".
        ///
        /// A caller that does not handle attributes must not be able to erase
        /// them — the Detail Sheet's own name/subtitle/notes save passes no
        /// attributes at all, and it saves an object that has them.
        @Test("attributes: nil leaves existing keys untouched")
        func attributesNilPreserves() throws {
            let json = """
            { "objectID": "source_1", "displayName": "Earthsea",
              "attributes": [ { "k": "author", "v": "Le Guin" } ] }
            """
            let detail = try ObjectDetail(json: json, kind: "source")
            #expect(detail.attributes["author"] == "Le Guin")

            // No `attributes:` argument at all — the ordinary sheet save.
            let patched = try detail.applyingEdits(displayName: "Earthsea",
                                                   subtitle: "", notes: "")
            let reopened = try ObjectDetail(json: patched, kind: "source")
            #expect(reopened.attributes["author"] == "Le Guin")
        }

        /// Empty removes the key entirely, matching how the core writes an
        /// object that has never had attributes — and matching `tags`.
        @Test("attributes: an empty map removes the key rather than writing []")
        func attributesEmptyRemovesKey() throws {
            let json = """
            { "objectID": "source_1", "displayName": "Earthsea",
              "attributes": [ { "k": "author", "v": "Le Guin" } ] }
            """
            let detail = try ObjectDetail(json: json, kind: "source")
            let patched = try detail.applyingEdits(displayName: "Earthsea",
                                                   subtitle: "", notes: "",
                                                   attributes: [:])
            let root = try #require(try JSONSerialization.jsonObject(
                with: Data(patched.utf8)) as? [String: Any])
            #expect(root["attributes"] == nil)
        }

        /// ⚠️ Unknown keys are PRESERVED, not just unshown (S11 §5.2).
        ///
        /// The sheet edits six citation fields. An attribute written by a future
        /// build — or by another platform — must survive a save by this one,
        /// which is the same patch-don't-reconstruct rule applied one level down.
        @Test("attributes this build does not display still survive an edit")
        func unknownAttributeKeysSurvive() throws {
            let json = """
            { "objectID": "source_1", "displayName": "Earthsea",
              "attributes": [ { "k": "isbn", "v": "978-0-14-303477-0" },
                              { "k": "author", "v": "Le Guin" } ] }
            """
            let detail = try ObjectDetail(json: json, kind: "source")

            // Edit only `author`, carrying the decoded map back — which is what
            // the citation editor does.
            var edited = detail.attributes
            edited["author"] = "Ursula K. Le Guin"
            let patched = try detail.applyingEdits(displayName: "Earthsea",
                                                   subtitle: "", notes: "",
                                                   attributes: edited)

            let reopened = try ObjectDetail(json: patched, kind: "source")
            #expect(reopened.attributes["author"] == "Ursula K. Le Guin")
            #expect(reopened.attributes["isbn"] == "978-0-14-303477-0")
        }

        /// Blank values are dropped rather than stored as empty strings — an
        /// unfilled citation field is absent, not present-and-empty.
        @Test("attributes: blank values are dropped, and keys are trimmed")
        func attributesDropBlanks() throws {
            let json = """
            { "objectID": "source_1", "displayName": "Earthsea" }
            """
            let detail = try ObjectDetail(json: json, kind: "source")
            let patched = try detail.applyingEdits(
                displayName: "Earthsea", subtitle: "", notes: "",
                attributes: ["author": "Le Guin", "publisher": "   ", "year": ""])
            let reopened = try ObjectDetail(json: patched, kind: "source")
            #expect(reopened.attributes.count == 1)
            #expect(reopened.attributes["author"] == "Le Guin")
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

    @Suite("Detail Sheet handoff (SP-117 T-0435, reduced by T-0547)")
    struct ObjectDetailHistoryTests {

        private func entry(_ id: String) -> ObjectDetailHistory.Entry {
            .init(objectID: id, kind: "character", worldID: "w", displayName: id)
        }

        // ⚠️ THIS SUITE USED TO TEST A BROWSER CURSOR — back, forward, and the
        // truncation rule. ⛔ All of it is GONE: `NavigationStack` owns navigation since
        // [T-0547], and the user retired forward from the requirements entirely.
        // ✅ What is left to test is what is left to break: the HANDOFF.
        // ⛔ Tests for a retired requirement are not coverage — they read as proof while
        // proving nothing (`feedback_fix_red_tests_dont_label_them`).

        @Test("the handoff carries the object the sheet should open on")
        func handoffCarries() {
            let h = ObjectDetailHistory()
            #expect(h.current == nil)

            h.visit(entry("Myton"))
            #expect(h.current?.objectID == "Myton")
            #expect(h.current?.displayName == "Myton", "the NAME travels — never an ID")
        }

        @Test("re-visiting the object already handed off is a no-op")
        func revisitIsNoOp() {
            // ⚠️ Without this, opening the same sheet twice would re-seed and the
            // writer's trail would restart under her — the shape of [I-0132], where a
            // re-selection wrote an unchanged value and the update was coalesced away.
            let h = ObjectDetailHistory()
            h.visit(entry("Myton"))
            h.visit(entry("Myton"))
            #expect(h.current?.objectID == "Myton")
        }

        @Test("a DIFFERENT object replaces the handoff")
        func differentObjectReplaces() {
            let h = ObjectDetailHistory()
            h.visit(entry("Myton"))
            h.visit(entry("Brother Colm"))
            #expect(h.current?.objectID == "Brother Colm")
        }

        @Test("reset clears the handoff so the next open starts fresh")
        func resetClears() {
            // ⚠️ [I-0168]'s guarded re-entry depends on this: the host calls `reset()`
            // on close, so reopening does not resume a trail the writer has left.
            let h = ObjectDetailHistory()
            h.visit(entry("Myton"))
            h.reset()
            #expect(h.current == nil)
        }

        @Test("Entry is Hashable — NavigationPath requires it")
        func entryIsHashable() {
            // ⚠️ [T-0547]: the sheet pushes `Entry` onto a `NavigationPath` and keys
            // `navigationDestination(for:)` on the type. ⛔ Dropping Hashable would break
            // navigation at runtime, not at compile time in the sheet.
            let a = entry("Myton"), b = entry("Myton"), c = entry("Brother Colm")
            #expect(a == b)
            #expect(a.hashValue == b.hashValue)
            #expect(Set([a, b, c]).count == 2)
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
        // boundary. SP-102's frozen-graph refusal (detail == "worldUnavailable:<status>")
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
            // A duplicate is not a world-away refusal; the two must not be conflated.
            #expect(error.isWorldUnavailable == false)
            #expect(error.unavailableWorldStatus == nil)
        }
    }

    @Test("worldUnavailable detail parses into a typed status without guessing")
    func unavailableStatusParsing() {
        // ⚠️ PURE PARSING ONLY. This asserts the decoder's behaviour on inputs it
        // constructs itself, which is useful for the FALLBACK rules below and
        // useless for proving the core actually emits this spelling.
        // ✅ `worldUnavailableDetailCrossesTheBoundary` is the test that does that.
        let offline = ScriviError(code: 1, message: "frozen", detail: "worldUnavailable:offline")
        #expect(offline.isWorldUnavailable)
        #expect(offline.unavailableWorldStatus == .offline)

        let missing = ScriviError(code: 1, message: "frozen", detail: "worldUnavailable:missing")
        #expect(missing.unavailableWorldStatus == .missing)

        // ⚠️ An unrecognised status falls back to the honest generic, never to a
        // guess: a wrong "missing" invites restoring from backup when the NAS was
        // merely unreachable (Doc 2 §7.2.1).
        let future = ScriviError(code: 1, message: "frozen", detail: "worldUnavailable:teleported")
        #expect(future.unavailableWorldStatus == .unavailable)

        let ordinary = ScriviError(code: 1, message: "something else")
        #expect(ordinary.isWorldUnavailable == false)
        #expect(ordinary.unavailableWorldStatus == nil)

        // ⚠️ I-0222: `worldPending:` was the OTHER spelling until 2026-09-17 and is
        // now retired. A core still emitting it would be a regression, and this
        // asserts the merged decoder does NOT quietly accept it.
        let retired = ScriviError(code: 1, message: "frozen", detail: "worldPending:unmounted")
        #expect(retired.isWorldUnavailable == false,
                "worldPending: was retired by the I-0222 ruling — the core must emit one spelling")
    }

    /// ⚠️ **THE TEST I-0222 EXISTED FOR.** The previous suite asserted the decoder
    /// against a string the TEST wrote, so it could never discover which spelling
    /// the CORE actually emits. This one takes `detail` from a real `scrivi_*` call.
    ///
    /// ⚠️ **It uses `openObject`, NOT `listObjects`** — ✅ measured 2026-09-17:
    /// `listObjects` does not fail on an unreachable world at all
    /// (`ObjectIndex::loadAllVisible` skips it and returns `ok:true`), so it cannot
    /// carry the discriminator. `openObject` goes through `ObjectStore::kindDirFor`,
    /// which is the site that actually produces `worldUnavailable:<status>`.
    @Test("worldUnavailable detail crosses the C ABI from a real unreachable world")
    func worldUnavailableDetailCrossesTheBoundary() throws {
        let f = try makeFixture()
        let worldDir = try TempDir()
        let packagePath = worldDir.url.appendingPathComponent("Vanishing.scrivworld")
            .path(percentEncoded: false)

        let created = try f.engine.createWorld(
            projectRootPath: f.root, packagePath: packagePath,
            displayName: "Vanishing", epochLabel: "Age of Loss"
        )

        let relic = try f.engine.createObject(
            projectRootPath: f.root, objectKind: "artifact",
            displayName: "Relic", authorshipRef: f.ref, worldID: created.worldID
        )

        // It opens while the package is present.
        _ = try f.engine.openObject(
            projectRootPath: f.root, objectKind: "artifact",
            objectID: relic.objectID, worldID: created.worldID)

        // ⚠️ Remove the PACKAGE while the binding still points at it — the fixture
        // equivalent of pulling the drive. The binding is project-local and stays.
        try FileManager.default.removeItem(atPath: packagePath)

        do {
            _ = try f.engine.openObject(
                projectRootPath: f.root, objectKind: "artifact",
                objectID: relic.objectID, worldID: created.worldID)
            Issue.record("expected a world-unavailable failure once the package was removed")
        } catch let error as ScriviError {
            // ✅ THE ASSERTION THAT WOULD HAVE CAUGHT I-0222: the discriminator the
            // CORE emits must be the one the Swift decoder matches. Before the
            // merge, RelationshipStore said `worldPending:` and this said
            // `worldUnavailable:`, and no test compared them.
            #expect(error.isWorldUnavailable,
                    "the core's detail spelling must match the Swift decoder")
            #expect(error.unavailableWorldStatus != nil)
            #expect(error.unavailableWorldStatus != .available)

            // ✅ And the writer-facing string must be the core's own message, not
            // Foundation's "<Module>.<Type> <code>" fallback (the LocalizedError gap).
            #expect(error.localizedDescription.contains("ScriviError") == false,
                    "localizedDescription leaked the type name — LocalizedError conformance is missing")
            #expect(error.localizedDescription == error.message
                    || error.localizedDescription.hasPrefix(error.message),
                    "localizedDescription must be built from the core's message")
        }
    }

    /// ⚠️ **THE OTHER HALF, and the more surprising one.** ✅ An unreachable world
    /// makes `listObjects` return FEWER OBJECTS, not an error — so a card that
    /// renders only what the index returns goes silently empty with nothing to
    /// report. ⛔ **This is why the writer saw empty lists**, and any future
    /// "show pending rows" work must not assume an error arrives to trigger it.
    @Test("listObjects silently omits an unreachable world's objects, without failing")
    func listObjectsOmitsUnreachableWorldWithoutError() throws {
        let f = try makeFixture()
        let worldDir = try TempDir()
        let packagePath = worldDir.url.appendingPathComponent("Fading.scrivworld")
            .path(percentEncoded: false)

        let created = try f.engine.createWorld(
            projectRootPath: f.root, packagePath: packagePath,
            displayName: "Fading", epochLabel: "Dusk"
        )
        _ = try f.engine.createObject(
            projectRootPath: f.root, objectKind: "artifact",
            displayName: "Lantern", authorshipRef: f.ref, worldID: created.worldID
        )

        let before = try f.engine.listObjects(projectRootPath: f.root, kind: "artifact").objects
        #expect(before.isEmpty == false, "the object must list while its world is present")

        try FileManager.default.removeItem(atPath: packagePath)

        // ⚠️ NOT a throw. This is the measured behaviour, asserted so a future
        // change to it is a deliberate decision rather than a surprise.
        let after = try f.engine.listObjects(projectRootPath: f.root, kind: "artifact").objects
        #expect(after.isEmpty,
                "an unreachable world's objects are omitted from the listing, not reported")
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

        let layout = InspectorLayoutStore(engine: ScriviEngine(), projectRootPath: NSTemporaryDirectory())
        let stack = layout.resolvedStack(sceneID: "scene-1", stack: .worldbuilding)
        #expect(stack.entries.isEmpty, "the default Worldbuilding stack ships EMPTY")
    }

    // MARK: — T-0536 / [I-0215]: the layout round trip must be lossless

    /// ⚠️ **THE TEST THE SPRINT EXISTS FOR.** ⛔ A test that writes a key this build
    /// KNOWS and reads it back proves nothing — it passed before the fix. This one
    /// uses a key the build has never heard of, which is the only thing that was
    /// being destroyed.
    @Test("an unknown inspector-layout.json key survives a load→mutate→save cycle")
    func unknownLayoutKeysSurviveRoundTrip() throws {
        let dir = URL(fileURLWithPath: NSTemporaryDirectory())
            .appendingPathComponent("scrivi-t0536-\(UUID().uuidString)")
        try FileManager.default.createDirectory(at: dir, withIntermediateDirectories: true)
        defer { try? FileManager.default.removeItem(at: dir) }

        let url = dir.appendingPathComponent("inspector-layout.json")

        // A file this build can read, carrying a key it cannot: exactly what a newer
        // Scrivi — or Linux, which preserves the whole document — leaves behind.
        let onDisk: [String: Any] = [
            "schema": "scrivi.inspector-layout.v1",
            "selectedTab": "writing",
            "inspectorHidden": false,
            "defaultStacks": ["writing": [], "worldbuilding": []],
            "stackSort": ["writing": "manual", "worldbuilding": "manual"],
            "scenes": [:],
            // ⚠️ THE KEY UNDER TEST.
            "futureCardOrder": ["scene_x": ["a", "b"]],
        ]
        try JSONSerialization.data(withJSONObject: onDisk, options: [.prettyPrinted])
            .write(to: url)

        let store = InspectorLayoutStore(engine: ScriviEngine(), projectRootPath: dir.path)
        #expect(store.loadError == nil, "the fixture must load cleanly")

        // Mutate a key this build DOES own, then save.
        store.setInspectorHidden(true)

        let reread = try JSONSerialization.jsonObject(with: Data(contentsOf: url))
            as? [String: Any]
        let survivor = reread?["futureCardOrder"] as? [String: [String]]

        // ✅ THE ASSERTION I-0215 WAS FILED FOR.
        #expect(survivor?["scene_x"] == ["a", "b"],
                "a key this build does not understand must survive the round trip")

        // ⚠️ AND THE OTHER HALF, WITHOUT WHICH THIS TEST IS WORTHLESS: a "fix" that
        // preserved unknown keys by never re-encoding anything would satisfy the
        // check above while silently ending all layout saves.
        #expect(reread?["inspectorHidden"] as? Bool == true,
                "the mutated known property must also have been written")
    }

    /// I-0255 — timeline visibility persists in the SAME document as `inspectorHidden`.
    /// An older layout has no `timelineHidden` key and must read as visible.
    @Test("timelineHidden + navigatorHidden round-trip through inspector-layout.json; absent reads false (I-0255)")
    func timelineHiddenRoundTrips() throws {
        let dir = URL(fileURLWithPath: NSTemporaryDirectory())
            .appendingPathComponent("scrivi-i0255-\(UUID().uuidString)")
        try FileManager.default.createDirectory(at: dir, withIntermediateDirectories: true)
        defer { try? FileManager.default.removeItem(at: dir) }
        let url = dir.appendingPathComponent("inspector-layout.json")

        // A layout written before the ruling: no `timelineHidden` key at all.
        let onDisk: [String: Any] = [
            "schema": "scrivi.inspector-layout.v1",
            "selectedTab": "writing",
            "inspectorHidden": false,
            "defaultStacks": ["writing": [], "worldbuilding": []],
            "stackSort": ["writing": "manual", "worldbuilding": "manual"],
            "scenes": [:],
        ]
        try JSONSerialization.data(withJSONObject: onDisk, options: [.prettyPrinted])
            .write(to: url)

        let store = InspectorLayoutStore(engine: ScriviEngine(), projectRootPath: dir.path)
        #expect(store.loadError == nil)
        #expect(store.document.timelineHidden == false, "an absent key must read as visible")

        store.setTimelineHidden(true)
        let reread = try JSONSerialization.jsonObject(with: Data(contentsOf: url))
            as? [String: Any]
        #expect(reread?["timelineHidden"] as? Bool == true, "the choice must be written")
        #expect(reread?["inspectorHidden"] as? Bool == false, "a neighbour key must be untouched")

        // A fresh store — the next launch — reads it back.
        let relaunched = InspectorLayoutStore(engine: ScriviEngine(), projectRootPath: dir.path)
        #expect(relaunched.document.timelineHidden == true)

        // The Scene Navigator (same Issue, same document): absent reads shown; a write
        // persists and leaves `timelineHidden` alone.
        #expect(relaunched.document.navigatorHidden == false, "an absent key must read as shown")
        relaunched.setNavigatorHidden(true)
        let third = try JSONSerialization.jsonObject(with: Data(contentsOf: url)) as? [String: Any]
        #expect(third?["navigatorHidden"] as? Bool == true)
        #expect(third?["timelineHidden"] as? Bool == true, "the timeline choice must survive")
        #expect(InspectorLayoutStore(engine: ScriviEngine(), projectRootPath: dir.path)
                    .document.navigatorHidden == true)
    }

    /// ⚠️ **THE CROSS-PLATFORM CASE, run against a REAL project's layout file.**
    ///
    /// ⛔ **A first attempt at this test took the project path from an environment
    /// variable and `guard`ed on it. The variable never reached the test runner, so
    /// the test returned early and PASSED WITHOUT RUNNING THE STORE** — caught only by
    /// checking the file on disk afterwards. ✅ **This version stages its own fixture
    /// from bytes it controls, so there is no path by which it can pass vacuously.**
    ///
    /// ⚠️ The fixture is a REAL `inspector-layout.json` shape (39 KB of scene entries
    /// in production) reduced to its structure, plus two keys this build has never
    /// heard of — what a newer Scrivi, or a future Linux build, leaves behind.
    @Test("unknown keys survive alongside a populated scenes map, as on a shared project")
    func unknownKeysSurviveAlongsidePopulatedScenes() throws {
        let dir = URL(fileURLWithPath: NSTemporaryDirectory())
            .appendingPathComponent("scrivi-t0536c-\(UUID().uuidString)")
        try FileManager.default.createDirectory(at: dir, withIntermediateDirectories: true)
        defer { try? FileManager.default.removeItem(at: dir) }
        let url = dir.appendingPathComponent("inspector-layout.json")

        let onDisk: [String: Any] = [
            "schema": "scrivi.inspector-layout.v1",
            "selectedTab": "worldbuilding",
            "inspectorHidden": false,
            "defaultStacks": ["writing": [["type": "tags", "collapsed": false]],
                              "worldbuilding": []],
            "stackSort": ["writing": "manual", "worldbuilding": "manual"],
            // A populated scenes map: the merge must not disturb it.
            "scenes": [
                "scene_019fa3be": ["writing": [["type": "outline", "collapsed": true]],
                                   "worldbuilding": [["type": "objects.character",
                                                      "collapsed": false]]],
            ],
            // ⚠️ THE KEYS UNDER TEST — neither is in `InspectorLayoutDocument`.
            "futureCardOrder": ["scene_x": ["a", "b"]],
            "linuxOnlyPreference": ["paneWidth": 320],
        ]
        try JSONSerialization.data(withJSONObject: onDisk, options: [.prettyPrinted])
            .write(to: url)

        let store = InspectorLayoutStore(engine: ScriviEngine(), projectRootPath: dir.path)
        #expect(store.loadError == nil, "the fixture must load cleanly")

        store.setInspectorHidden(true)
        store.setSelectedTab(.writing)

        let back = try JSONSerialization.jsonObject(with: Data(contentsOf: url))
            as? [String: Any]

        // ✅ Both unknown keys survive, with their values intact.
        #expect((back?["futureCardOrder"] as? [String: [String]])?["scene_x"] == ["a", "b"])
        #expect((back?["linuxOnlyPreference"] as? [String: Int])?["paneWidth"] == 320)

        // ⚠️ AND the store actually wrote — ⛔ without these the test passes vacuously
        // if the store is never exercised at all.
        #expect(back?["inspectorHidden"] as? Bool == true, "the store must have written")
        #expect(back?["selectedTab"] as? String == "writing")

        // ✅ The populated scenes map is preserved through the merge, not flattened.
        let scenes = back?["scenes"] as? [String: Any]
        #expect(scenes?["scene_019fa3be"] != nil, "existing scene layouts must survive")
    }

    @Test("a layout file with no unknown keys still round-trips, and stays sorted")
    func knownOnlyLayoutRoundTripsUnchanged() throws {
        let dir = URL(fileURLWithPath: NSTemporaryDirectory())
            .appendingPathComponent("scrivi-t0536b-\(UUID().uuidString)")
        try FileManager.default.createDirectory(at: dir, withIntermediateDirectories: true)
        defer { try? FileManager.default.removeItem(at: dir) }

        // No file on disk: the store falls back to defaults and has nothing to
        // preserve. ⚠️ This is the path where `rawDocument` is nil — it must still save.
        let store = InspectorLayoutStore(engine: ScriviEngine(), projectRootPath: dir.path)
        store.setInspectorHidden(true)

        let url = dir.appendingPathComponent("inspector-layout.json")
        #expect(FileManager.default.fileExists(atPath: url.path),
                "a store with no prior file must still write one")

        let text = try String(contentsOf: url, encoding: .utf8)
        // ⚠️ Git-visible project state: unstable key order would churn the diff on
        // every save, so sorting is a requirement rather than a nicety.
        let schemaIdx = try #require(text.range(of: "\"schema\"")).lowerBound
        let tabIdx = try #require(text.range(of: "\"selectedTab\"")).lowerBound
        #expect(schemaIdx < tabIdx, "keys must be written in sorted order")
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

#if os(macOS)
import AppKit

/// EP-045 AC1 — only a DIVIDER is a scene boundary. ⛔ Before AC1 every reader tested
/// `.attachment`, so the first non-divider attachment any feature inserted would split a scene,
/// and `sceneBoundaries` is what the save path slices scene bytes with.
@Suite("Typed scene dividers (EP-045 AC1)")
@MainActor
struct TypedSceneDividerTests {

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


    private let body: [NSAttributedString.Key: Any] = [.font: NSFont.systemFont(ofSize: 12)]

    /// "Alpha" | divider | "Beta"
    private func twoScenes() -> NSMutableAttributedString {
        let s = NSMutableAttributedString(string: "Alpha", attributes: body)
        s.append(SceneDivider.string(NSTextAttachment(), state: .sceneBreak, newlineAttributes: body))
        s.append(NSAttributedString(string: "Beta", attributes: body))
        return s
    }

    @Test("a divider splits scenes; its key carries the render state")
    func dividerSplits() {
        let s = twoScenes()
        #expect(SceneDivider.sceneBoundaries(in: s) == [NSRange(location: 0, length: 5),
                                                        NSRange(location: 7, length: 4)])
        #expect(SceneDivider.isDivider(in: s, at: 5))
        #expect(!SceneDivider.isDivider(in: s, at: 6), "the trailing \\n does not carry the key")
        #expect(s.attribute(.scriviDivider, at: 5, effectiveRange: nil) as? DividerRenderState == .sceneBreak)
    }

    @Test("a NON-divider attachment inside a scene is ordinary text, not a boundary")
    func strayAttachmentIsNotABoundary() {
        let s = twoScenes()
        // A plain attachment in the middle of "Beta" — what EP-032 / EP-046 will insert.
        s.insert(NSAttributedString(attachment: NSTextAttachment()), at: 9)

        // ⚠️ Proof this test could fail: the storage now holds TWO attachments, so the old
        // `.attachment` rule would have produced THREE scenes and sliced "Beta" in half.
        var attachments = 0
        s.enumerateAttribute(.attachment, in: NSRange(location: 0, length: s.length)) { v, _, _ in
            if v != nil { attachments += 1 }
        }
        #expect(attachments == 2)

        // ✅ Still two scenes; the second simply grew by the one attachment character.
        #expect(SceneDivider.sceneBoundaries(in: s) == [NSRange(location: 0, length: 5),
                                                        NSRange(location: 7, length: 5)])
        #expect(!SceneDivider.isDivider(in: s, at: 9))
    }

    @Test("an undo-style replace of a scene's range leaves the divider and its key intact")
    func sceneReplaceKeepsDivider() {
        let s = twoScenes()
        // What `applySceneChange` does: replace exactly the scene's boundary range.
        s.replaceCharacters(in: NSRange(location: 7, length: 4),
                            with: NSAttributedString(string: "Gamma!", attributes: body))
        #expect(SceneDivider.isDivider(in: s, at: 5))
        #expect(SceneDivider.sceneBoundaries(in: s) == [NSRange(location: 0, length: 5),
                                                        NSRange(location: 7, length: 6)])
    }

    @Test("chapter headings are skipped and the chapter-end state is carried")
    func headingsAndChapterEnd() {
        let heading: [NSAttributedString.Key: Any] = [.font: NSFont.systemFont(ofSize: 12),
                                                      .scriviHeading: true]
        let s = NSMutableAttributedString(string: "One\n", attributes: heading)      // 0..<4
        s.append(NSAttributedString(string: "Alpha", attributes: body))             // 4..<9
        s.append(SceneDivider.string(NSTextAttachment(), state: .chapterEnd,
                                     newlineAttributes: body))                      // 9, 10
        s.append(NSAttributedString(string: "Two\n", attributes: heading))          // 11..<15
        s.append(NSAttributedString(string: "Beta", attributes: body))              // 15..<19
        #expect(SceneDivider.sceneBoundaries(in: s) == [NSRange(location: 4, length: 5),
                                                        NSRange(location: 15, length: 4)])
        #expect(s.attribute(.scriviDivider, at: 9, effectiveRange: nil) as? DividerRenderState == .chapterEnd)
    }

    /// EP-045 AC9 — save fidelity, through the C ABI on a real temp project. ✅ Builds the storage
    /// the way `rebuildStorage` does (heading + typed dividers), slices it with the SAME function the
    /// save path uses, saves every slice through `scrivi_save_scene`, and reloads: each scene's file
    /// bytes must be unchanged.
    @Test("edit → save → reload round-trips every scene's bytes unchanged (EP-045 AC9)")
    func saveFidelityRoundTrip() throws {
        let appSupport = try TempDir()
        let projectDir = try TempDir()
        let engine = ScriviEngine()
        let identity = try engine.ensureLocalIdentity(displayName: "Test Author",
                                                      appSupportRoot: appSupport.path)
        let ref = AuthorshipRef(identityID: identity.identityID,
                                personaID: identity.defaultPersonaID,
                                displayName: identity.displayName)
        _ = try engine.createProject(projectRootPath: projectDir.path, appSupportRoot: appSupport.path,
                                     title: "AC9", slug: "ac9", authorshipRef: ref)
        let opened = try engine.openProject(projectRootPath: projectDir.path,
                                            appSupportRoot: appSupport.path)
        guard let first = opened.scenes.first else { Issue.record("no first scene"); return }

        // Bytes that are easy to mangle: multi-unit UTF-16 (é, 👋), Markdown punctuation, trailing
        // spaces, blank lines, a tab, a trailing newline.
        let bodies = [
            "Café — *not* emphasis? # nor a heading.\n\nTrailing spaces   \n\tTabbed.\n",
            "👋 Hello, “quoted” _under_ `code` \\ backslash.\n\n\n",
            "Last scene, no trailing newline",
        ]
        var scenes = [first]
        for _ in 1..<bodies.count {
            let made = try engine.createScene(projectRootPath: projectDir.path,
                                              appSupportRoot: appSupport.path,
                                              projectID: opened.projectID, chapterID: first.chapterID,
                                              afterSceneID: scenes.last!.sceneID, authorshipRef: ref)
            let reopened = try engine.openProject(projectRootPath: projectDir.path,
                                                  appSupportRoot: appSupport.path)
            guard let info = reopened.scenes.first(where: { $0.sceneID == made.sceneID }) else {
                Issue.record("created scene not listed"); return
            }
            scenes.append(info)
        }
        func save(_ i: Int, _ text: String) throws {
            _ = try engine.saveScene(projectID: opened.projectID, projectRootPath: projectDir.path,
                                     appSupportRoot: appSupport.path, sceneID: scenes[i].sceneID,
                                     sceneMetadataPath: scenes[i].metadataPath,
                                     sceneContentPath: scenes[i].contentPath,
                                     markdown: text, authorshipRef: ref)
        }
        func load(_ i: Int) throws -> String {
            try engine.openScene(projectRootPath: projectDir.path, appSupportRoot: appSupport.path,
                                 projectID: opened.projectID, sceneID: scenes[i].sceneID).markdown
        }
        for (i, b) in bodies.enumerated() { try save(i, b) }
        let loaded = try (0..<bodies.count).map(load)
        #expect(loaded == bodies, "the core must store each body verbatim before the cycle starts")

        // Storage as `rebuildStorage` builds it: a chapter heading, then scenes split by dividers.
        let heading: [NSAttributedString.Key: Any] = [.font: NSFont.systemFont(ofSize: 12),
                                                      .scriviHeading: true]
        let storage = NSMutableAttributedString(string: "Chapter One\n", attributes: heading)
        for (i, text) in loaded.enumerated() {
            if i > 0 {
                storage.append(SceneDivider.string(NSTextAttachment(), state: .sceneBreak,
                                                   newlineAttributes: body))
            }
            storage.append(NSAttributedString(string: text, attributes: body))
        }

        // The save path: slice by `sceneBoundaries`, save each slice.
        guard let ranges = SceneDivider.sceneBoundaries(in: storage) else {
            Issue.record("no boundaries"); return
        }
        #expect(ranges.count == bodies.count)
        for (i, r) in ranges.enumerated() {
            try save(i, (storage.string as NSString).substring(with: r))
        }

        // Reload: every scene byte-identical.
        let reloaded = try (0..<bodies.count).map(load)
        for i in bodies.indices {
            #expect(Array(reloaded[i].utf8) == Array(bodies[i].utf8), "scene \(i) bytes changed")
        }
    }
}
#endif

/// EP-045 AC3 — SOURCE ↔ PRESENTED offsets. ✅ The ORACLE is Apple's own Markdown parser
/// (`AttributedString(markdown:)`), not this code's idea of CommonMark (design §4.2: *"Ten probes is
/// not a proof — AC3's test obligation is a CORPUS"*).
@Suite("Markdown escapes: source ↔ presented (EP-045 AC3)")
struct MarkdownEscapeMapTests {

    /// What the writer would SEE if Apple's parser rendered `source` (inline syntax only).
    private func oracle(_ source: String) throws -> String {
        let a = try AttributedString(
            markdown: source,
            options: .init(interpretedSyntax: .inlineOnlyPreservingWhitespace))
        return String(a.characters)
    }

    /// The two maps must be mutually consistent boundaries, monotonic, and end-to-end.
    private func checkMapShape(_ source: String, _ m: MarkdownEscapes.Map,
                               sourceLocation: SourceLocation = #_sourceLocation) {
        let srcLen = source.utf16.count
        #expect(m.presentedToSource.count == m.presented.utf16.count + 1, sourceLocation: sourceLocation)
        #expect(m.sourceToPresented.count == srcLen + 1, sourceLocation: sourceLocation)
        #expect(m.presentedToSource.last == srcLen, sourceLocation: sourceLocation)
        for (p, s) in m.presentedToSource.enumerated() {
            #expect(m.sourceToPresented[s] == p, "round trip at presented \(p)", sourceLocation: sourceLocation)
        }
        #expect(m.presentedToSource == m.presentedToSource.sorted(), sourceLocation: sourceLocation)
        #expect(m.sourceToPresented == m.sourceToPresented.sorted(), sourceLocation: sourceLocation)
    }

    @Test("fixed edge cases match the oracle")
    func edgeCases() throws {
        let cases: [String] = [
            "", "plain prose", #"\*"#, #"a\*b"#, #"\\"#, #"a\\b"#, #"\a stays"#, #"ends with \"#,
            #"👋\*é\_"#, ##"\# not a heading"##, #"Mr\. Smith\, in \"quotes\" — really\?"#,
            #"\\\*"#, "tab\there", "line one\nline two",
            // Existing text only (R2): a backslash before a newline is a HARD LINE BREAK.
            // ⚠️ `"a\\\n"` (the backslash ENDS the text) moved to `paragraphEndBackslash`: under
            // `.full` it stays literal (Q1 = (a)), where this inline-only oracle has no paragraphs.
            "foo\\\nbar", "x\\  y",
        ]
        for src in cases {
            let m = MarkdownEscapes.map(src)
            #expect(m.presented == (try oracle(src)), "presented text disagrees with Apple's parser for \(src.debugDescription)")
            checkMapShape(src, m)
        }
    }

    @Test("each of the 32 escapable marks presents as itself")
    func all32() throws {
        let marks = ##"!"#$%&'()*+,-./:;<=>?@[\]^_`{|}~"##
        #expect(marks.utf16.count == 32)
        #expect(MarkdownEscapes.escapable.count == 32)
        for ch in marks {
            let src = MarkdownEscapes.escape("x\(ch)y")
            let m = MarkdownEscapes.map(src)
            #expect(m.presented == "x\(ch)y")
            #expect(try oracle(src) == "x\(ch)y", "Apple's parser disagrees for \(ch)")
            // The caret before the mark maps to BEFORE ITS BACKSLASH (source 1), never between.
            #expect(m.presentedToSource == [0, 1, 3, 4])
            #expect(m.sourceToPresented == [0, 1, 1, 2, 3])
        }
    }

    @Test("a 2,000-string typed corpus: escape → present round-trips and matches the oracle")
    func corpus() throws {
        // Deterministic generator, so a failure reproduces.
        var state: UInt64 = 0x5C21_0453
        func next() -> UInt64 { state = state &* 6364136223846793005 &+ 1442695040888963407; return state >> 33 }
        let alphabet: [String] = Array(##"!"#$%&'()*+,-./:;<=>?@[\]^_`{|}~"##).map(String.init)
            + ["a", "b", "Z", "é", "👋", " ", "\t", "\n"]
        var oracleMismatches: [String] = []
        for _ in 0..<2_000 {
            let len = Int(next() % 24)
            let typed = (0..<len).map { _ in alphabet[Int(next() % UInt64(alphabet.count))] }.joined()
            let src = MarkdownEscapes.escape(typed)
            let m = MarkdownEscapes.map(src)
            #expect(m.presented == typed, "escape→present must return what was typed: \(typed.debugDescription)")
            checkMapShape(src, m)
            if try oracle(src) != typed { oracleMismatches.append(typed) }
        }
        #expect(oracleMismatches.isEmpty,
                "Apple's parser disagreed on \(oracleMismatches.count) strings, e.g. \(oracleMismatches.prefix(5).map(\.debugDescription))")
    }

    /// ✅ EP-045 AC6c / Q1 = (a): a hard break cannot END a paragraph. Checked against `.full`
    /// (AC8's mode) — ⚠️ not this suite's inline-only oracle, which has no notion of paragraphs.
    /// `.full` collapses whitespace, so presented texts are compared with whitespace removed.
    @Test("a backslash before a blank line or the end stays literal (Q1 = (a), .full)")
    func paragraphEndBackslash() throws {
        func full(_ s: String) throws -> String {
            String(try AttributedString(markdown: s, options: .init(interpretedSyntax: MarkdownEscapes.interpretedSyntax)).characters)
        }
        func squeezed(_ s: String) -> String { s.filter { !$0.isWhitespace } }
        let cases: [(source: String, continues: Bool, literal: Bool)] = [
            ("end.\\\n\nnext.", false, true),     // AC6's Enter result
            ("end.\\\n \t\nnext.", false, true), // a whitespace-only line is blank too
            ("end.\\\n", false, true),             // the end of the text
            ("a\\\n", false, true),
            ("end.\\\nnext.", false, false),       // after a ⌫ merge: a HARD BREAK, hidden
        ]
        for c in cases {
            let m = MarkdownEscapes.map(c.source, continues: c.continues)
            #expect(m.presented.contains("\\") == c.literal, "map for \(c.source.debugDescription)")
            #expect(squeezed(m.presented) == squeezed(try full(c.source)), "oracle for \(c.source.debugDescription)")
            checkMapShape(c.source, m)
        }
        // The styler maps ONE line: it says whether the next line continues the paragraph.
        #expect(MarkdownEscapes.map("end.\\\n", continues: true).presented == "end.\n")
        #expect(MarkdownEscapes.map("end.\\\n", continues: false).presented == "end.\\\n")
    }

    // MARK: EP-045 AC7 — unexposed block intents are PROSE (Q-AC7 = (a))

    /// The PROSE reading of `source`: `.full` (AC8) with every line's leading indentation removed,
    /// so no line can be a code block. ⚠️ `.full` collapses whitespace: compare squeezed.
    private func proseOracle(_ source: String) throws -> String {
        let unindented = source.split(separator: "\n", omittingEmptySubsequences: false)
            .map { String($0.drop(while: { $0 == " " || $0 == "\t" })) }
            .joined(separator: "\n")
        return String(try AttributedString(
            markdown: unindented, options: .init(interpretedSyntax: MarkdownEscapes.interpretedSyntax)).characters)
    }
    private func squeezed(_ s: String) -> String { s.filter { !$0.isWhitespace } }

    @Test("AC7: design's test — a 4-leading-space (or tab-led) paragraph presents as PROSE")
    func indentedParagraphIsProse() throws {
        for src in ["    a\\*b", "\ta\\*b", "\t\ta\\_b\n\nnext"] {
            let m = MarkdownEscapes.map(src)
            #expect(!m.presented.contains("\\"), "escape hidden as in prose: \(src.debugDescription)")
            #expect(squeezed(m.presented) == squeezed(try proseOracle(src)))
            // ⚠️ Plain `.full` reads it as a CODE BLOCK and shows the backslash — what AC7 overrides.
            let codeBlock = String(try AttributedString(
                markdown: src, options: .init(interpretedSyntax: MarkdownEscapes.interpretedSyntax)).characters)
            #expect(codeBlock.contains("\\"), "the oracle this test departs from: \(src.debugDescription)")
        }
    }

    @Test("AC7: the 2,000-string corpus agrees with the PROSE oracle (the 42 tab-led included)")
    func corpusIsProse() throws {
        var state: UInt64 = 0x5C21_0453   // the AC3 corpus seed, so these are the same strings
        func next() -> UInt64 { state = state &* 6364136223846793005 &+ 1442695040888963407; return state >> 33 }
        let alphabet: [String] = Array(##"!"#$%&'()*+,-./:;<=>?@[\]^_`{|}~"##).map(String.init)
            + ["a", "b", "Z", "é", "👋", " ", "\t", "\n"]
        var mismatches: [String] = []
        var codeBlockCases = 0
        for _ in 0..<2_000 {
            let typed = (0..<Int(next() % 24)).map { _ in alphabet[Int(next() % UInt64(alphabet.count))] }.joined()
            let src = MarkdownEscapes.escape(typed)
            let presented = MarkdownEscapes.map(src).presented
            if squeezed(presented) != squeezed(try proseOracle(src)) { mismatches.append(typed) }
            let full = String(try AttributedString(
                markdown: src, options: .init(interpretedSyntax: MarkdownEscapes.interpretedSyntax)).characters)
            if squeezed(presented) != squeezed(full) { codeBlockCases += 1 }
        }
        #expect(mismatches.isEmpty, "e.g. \(mismatches.prefix(5).map(\.debugDescription))")
        #expect(codeBlockCases == 42, "the design §4.5 count of indented (code-block) cases")
    }

    // MARK: R3 = (c) — the caret never rests inside a hidden escape

    /// `ab\*cd` with the backslash (offset 2) a stop run whose home is BEFORE it, as the presenter reports it.
    /// ⚠️ EP-046: hidden-ness is a LOOKUP now (storage carries no hiding attribute).
    private let text: NSString = #"ab\*cd"#
    private let escape: (Int) -> MarkdownEscapes.StopRun? = { $0 == 2 ? (NSRange(location: 2, length: 1), false) : nil }

    @Test("only the boundary between a hidden backslash and its mark is unreachable")
    func unreachable() {
        #expect((0...6).filter { MarkdownEscapes.isUnreachable($0, length: 6, runAt: escape) } == [3])
        // ⚠️ No stop run → nothing snapped.
        #expect((0...6).filter { MarkdownEscapes.isUnreachable($0, length: 6, runAt: { _ in nil }) }.isEmpty)
    }

    @Test("caret: → steps past the mark; ←, clicks and jumps land before the backslash")
    func caretSnap() {
        #expect(MarkdownEscapes.snapCaret(3, from: 2, length: 6, runAt: escape) == 4)   // → from before the backslash
        #expect(MarkdownEscapes.snapCaret(3, from: 4, length: 6, runAt: escape) == 2)   // ← from after the mark
        #expect(MarkdownEscapes.snapCaret(3, from: 0, length: 6, runAt: escape) == 2)   // click / jump: same visual spot
        #expect(MarkdownEscapes.snapCaret(3, from: 6, length: 6, runAt: escape) == 2)
        #expect(MarkdownEscapes.snapCaret(4, from: 2, length: 6, runAt: escape) == nil) // a reachable spot is left alone
    }

    @Test("a selection never splits a hidden backslash from its mark")
    func selectionSnap() {
        #expect(MarkdownEscapes.snapSelection(NSRange(location: 0, length: 3), in: text, runAt: escape) == NSRange(location: 0, length: 4))
        #expect(MarkdownEscapes.snapSelection(NSRange(location: 3, length: 3), in: text, runAt: escape) == NSRange(location: 2, length: 4))
        #expect(MarkdownEscapes.snapSelection(NSRange(location: 0, length: 4), in: text, runAt: escape) == nil)
    }

    @Test("EP-046 AC5 + [SP-162] Q1: a heading prefix or opener sends the caret AFTER it; ← steps out past the character before")
    func stopRuns() {
        // `x⏎## H`: the prefix (2..<5) is a stop run whose home is AFTER it.
        let prefix: (Int) -> MarkdownEscapes.StopRun? = { (2..<5).contains($0) ? (NSRange(location: 2, length: 3), true) : nil }
        #expect(MarkdownEscapes.snapCaret(2, from: 1, length: 6, runAt: prefix) == 5)  // → onto the line: after `## `
        #expect(MarkdownEscapes.snapCaret(3, from: 9, length: 6, runAt: prefix) == 5)  // a click inside the prefix
        #expect(MarkdownEscapes.snapCaret(4, from: 5, length: 6, runAt: prefix) == 1)  // ← from home: end of the line above
        #expect(MarkdownEscapes.snapCaret(5, from: 2, length: 6, runAt: prefix) == nil) // home is left alone
        // A closer (`**b**`, closer 3..<5): home BEFORE it; → from home steps past the character after.
        let closer: (Int) -> MarkdownEscapes.StopRun? = { (3..<5).contains($0) ? (NSRange(location: 3, length: 2), false) : nil }
        #expect(MarkdownEscapes.snapCaret(5, from: 9, length: 7, runAt: closer) == 3)  // just past it → home
        #expect(MarkdownEscapes.snapCaret(4, from: 3, length: 7, runAt: closer) == 6)  // → from home
        // ⛔ E1 moved a selection's end FORWARD whatever the direction, so shift-← from `ab\*c|d`
        // landed between `\` and `*` and was pushed straight back (SP-161 step 1).
        #expect(MarkdownEscapes.snapSelection(NSRange(location: 0, length: 3), previousEnd: 4,
                                              in: text, runAt: escape) == NSRange(location: 0, length: 2))
    }
}

#if os(macOS)
/// The REAL `ManuscriptNSTextView` with the REAL `ManuscriptPresenter` installed as the app installs
/// it, in a window so AppKit's key and pasteboard dispatch run as they do in the app.
@MainActor fileprivate final class ManuscriptFixture {
    let tv = ManuscriptNSTextView(usingTextLayoutManager: true)
    let presenter = ManuscriptPresenter()
    let window: NSWindow
    init(_ text: String = "") {
        tv.isRichText = false
        tv.allowsUndo = false
        tv.font = ManuscriptTypography.default.bodyFont
        tv.frame = NSRect(x: 0, y: 0, width: 500, height: 100)
        tv.textContentStorage?.delegate = presenter
        tv.textStorage?.delegate = presenter
        window = NSWindow(contentRect: tv.frame, styleMask: [.titled], backing: .buffered, defer: false)
        window.contentView = tv
        window.makeFirstResponder(tv)
        // ⚠️ EP-047: the text carries the app's BODY attributes, as every app write does. (It was a bare String into empty
        // storage — NO font at all — so the size checks below compared EMPTY sets, and `isSubset` passed vacuously.)
        tv.typingAttributes = ManuscriptTypography.default.bodyAttributes
        if !text.isEmpty {
            tv.textStorage?.replaceCharacters(in: NSRange(location: 0, length: 0),
                                              with: NSAttributedString(string: text, attributes: ManuscriptTypography.default.bodyAttributes))
        }
    }
    var text: String { tv.string }
    /// Hidden as the presenter presents it NOW (under the current selection's reveal).
    func hidden(_ i: Int) -> Bool {
        presenter.isHidden(i, in: tv.textStorage!, revealing: tv.selectedRange())
    }
    func caret(_ i: Int) { tv.setSelectedRange(NSRange(location: i, length: 0)) }
    func type(_ s: String) { tv.insertText(s, replacementRange: tv.selectedRange()) }
}

/// EP-045 AC4 — the escape layer, driven through the REAL `ManuscriptNSTextView` with the REAL
/// `ManuscriptPresenter` (EP-046 E2-S1; it replaced `EscapeHidingStyler`). ⚠️ Private pasteboards only — a test run must
/// never touch the writer's clipboard.
@Suite("Escape layer (EP-045 AC4)")
@MainActor
struct EscapeLayerTests {

    private typealias Fixture = ManuscriptFixture

    /// ⚠️ The type AppKit ACTUALLY passes on copy/paste (measured) — ⛔ NOT `.string`. The first
    /// version of these tests named `.string` and so passed while the real copy and paste failed.
    private let appKitStringType = NSPasteboard.PasteboardType("NSStringPboardType")

    private func pasteboard() -> NSPasteboard {
        let pb = NSPasteboard(name: .init("scrivi.test.\(UUID().uuidString)"))
        pb.clearContents()
        return pb
    }

    @Test("typing escapes a mark and the presenter hides its backslash")
    func typingEscapesAndHides() {
        let f = Fixture()
        f.type("a*b")
        #expect(f.text == #"a\*b"#)
        #expect(f.hidden(1), "the escape backslash is hidden")
        #expect(!f.hidden(0) && !f.hidden(2) && !f.hidden(3))
    }

    @Test("⌫ after the mark and ⌦ before the backslash remove the WHOLE pair")
    func pairDeletion() {
        let f = Fixture(#"a\*b"#)
        f.caret(3); f.tv.deleteBackward(nil)
        #expect(f.text == "ab", "no orphaned backslash")
        let g = Fixture(#"a\*b"#)
        g.caret(1); g.tv.deleteForward(nil)
        #expect(g.text == "ab", "no live, unescaped mark")
    }

    @Test("copy puts what the writer SEES on the pasteboard; pasting it back restores the stored text")
    func ownCopyRoundTrip() {
        let f = Fixture(#"a\*b"#)
        let pb = pasteboard()
        f.tv.setSelectedRange(NSRange(location: 0, length: 4))
        #expect(f.tv.writeSelection(to: pb, type: appKitStringType))
        #expect(pb.string(forType: .string) == "a*b", "other apps get the writer's text, not backslashes")
        f.caret(4)
        #expect(f.tv.readSelection(from: pb))
        #expect(f.text == #"a\*ba\*b"#, "not double-escaped")
    }

    @Test("an in-app copy of EXISTING markup stays markup (R2)")
    func ownCopyKeepsIntendedMarkup() {
        let f = Fixture("## Scene")
        let pb = pasteboard()
        f.tv.setSelectedRange(NSRange(location: 0, length: 8))
        _ = f.tv.writeSelection(to: pb, type: appKitStringType)
        f.caret(8)
        _ = f.tv.readSelection(from: pb)
        #expect(f.text == "## Scene## Scene", "re-escaping would have turned the intended ## literal")
    }

    @Test("text pasted from another app is escaped like typing (R1)")
    func externalPasteEscapes() {
        let f = Fixture()
        let pb = pasteboard()
        pb.setString("x_y", forType: .string)
        #expect(f.tv.readSelection(from: pb))
        #expect(f.text == #"x\_y"#)
        #expect(f.hidden(1))
    }

    @Test("a TextEdit-style paste (rich text + plain) is escaped too")
    func richPasteEscapes() throws {
        let f = Fixture()
        let pb = pasteboard()
        let rich = NSAttributedString(string: "x*y")
        let rtf = try rich.data(from: NSRange(location: 0, length: rich.length),
                                documentAttributes: [.documentType: NSAttributedString.DocumentType.rtf])
        pb.setData(rtf, forType: .rtf)
        pb.setString("x*y", forType: .string)
        #expect(f.tv.readSelection(from: pb))
        #expect(f.text == #"x\*y"#)
    }

    @Test("a backslash that stops escaping anything is un-hidden")
    func unhidesBrokenEscape() {
        let f = Fixture(#"a\*b"#)
        #expect(f.hidden(1))
        // A programmatic replace (as undo's apply does) turns `\*` into `\c`.
        f.tv.textStorage?.replaceCharacters(in: NSRange(location: 2, length: 1), with: "c")
        #expect(f.text == #"a\cb"#)
        #expect(!f.hidden(1), "a backslash before `c` is ordinary text and must be visible")
    }

    @Test("design AC4: type → store → parse (.full, AC8) → presented equals what was typed, all 32")
    func roundTripAll32() throws {
        for ch in ##"!"#$%&'()*+,-./:;<=>?@[\]^_`{|}~"## {
            let f = Fixture()
            f.type("x\(ch)y")
            let parsed = try AttributedString(
                markdown: f.text, options: .init(interpretedSyntax: MarkdownEscapes.interpretedSyntax))
            #expect(String(parsed.characters) == "x\(ch)y", "\(ch) → stored \(f.text.debugDescription)")
        }
    }
}
/// EP-045 AC11 (T-0583) — the scene-boundary table is MAINTAINED from each edit; after every kind of
/// edit it must EQUAL a full rescan. ⚠️ It must also NOT have rescanned for an in-scene edit — a table
/// that silently rescans every time would pass the equality check and save nothing.
@Suite("Scene-boundary table (EP-045 AC11)")
@MainActor
struct SceneBoundaryTableTests {

    private let body: [NSAttributedString.Key: Any] = [.font: ManuscriptTypography.default.bodyFont, .foregroundColor: NSColor.textColor]

    /// heading | "Alpha one." | divider | "Beta two." | divider | heading | "Gamma three."
    private func manuscript() -> NSAttributedString {
        let heading: [NSAttributedString.Key: Any] = [.font: NSFont.boldSystemFont(ofSize: 14), .scriviHeading: true]
        let s = NSMutableAttributedString(string: "Chapter 1\n", attributes: heading)
        s.append(NSAttributedString(string: "Alpha one.", attributes: body))
        s.append(SceneDivider.string(NSTextAttachment(), state: .sceneBreak, newlineAttributes: body))
        s.append(NSAttributedString(string: "Beta two.", attributes: body))
        s.append(SceneDivider.string(NSTextAttachment(), state: .chapterEnd, newlineAttributes: body))
        s.append(NSAttributedString(string: "\nChapter 2\n", attributes: heading))
        s.append(NSAttributedString(string: "Gamma three.", attributes: body))
        return s
    }

    private func fixture() -> (ManuscriptFixture, SceneBoundaryTable) {
        let f = ManuscriptFixture()
        let ts = f.tv.textStorage!
        ts.setAttributedString(manuscript())
        let table = SceneBoundaryTable()
        table.observe(ts)
        table.reset(SceneDivider.sceneBoundaries(in: ts)!)
        return (f, table)
    }

    private func backspace(_ f: ManuscriptFixture) {
        let e = NSEvent.keyEvent(with: .keyDown, location: .zero, modifierFlags: [], timestamp: 0,
                                 windowNumber: f.window.windowNumber, context: nil, characters: "\u{7f}",
                                 charactersIgnoringModifiers: "\u{7f}", isARepeat: false, keyCode: 51)!
        f.tv.keyDown(with: e)
    }
    private func pressReturn(_ f: ManuscriptFixture) {
        let e = NSEvent.keyEvent(with: .keyDown, location: .zero, modifierFlags: [], timestamp: 0,
                                 windowNumber: f.window.windowNumber, context: nil, characters: "\r",
                                 charactersIgnoringModifiers: "\r", isARepeat: false, keyCode: 36)!
        f.tv.keyDown(with: e)
    }

    /// The table equals a fresh full scan, and (when `local`) it got there WITHOUT rescanning.
    private func check(_ f: ManuscriptFixture, _ table: SceneBoundaryTable, local: Bool, _ what: String,
                       sourceLocation: SourceLocation = #_sourceLocation) {
        if local { #expect(!table.isDirty, "\(what): an in-scene edit must not need a rescan", sourceLocation: sourceLocation) }
        let ts = f.tv.textStorage!
        #expect(table.ranges(in: ts) == SceneDivider.sceneBoundaries(in: ts)!, "\(what)", sourceLocation: sourceLocation)
    }

    @Test("typing, Return, ⌫-join and an in-scene paste are maintained without a rescan")
    func inSceneEdits() {
        let (f, table) = fixture()
        let ns = f.tv.string as NSString
        let beta = ns.range(of: "Beta two.")
        f.caret(beta.location + 4); f.type(" *x*")
        check(f, table, local: true, "typing (with escapes) in the middle scene")
        f.caret(ns.range(of: "Alpha one.").location + 5); pressReturn(f)
        check(f, table, local: true, "Return in the first scene")
        f.caret((f.tv.string as NSString).range(of: " one.").location); backspace(f)
        check(f, table, local: true, "⌫ at a paragraph start (Q3 join)")
        let gamma = (f.tv.string as NSString).range(of: "Gamma")
        f.caret(gamma.location); f.type("Pasted ")
        check(f, table, local: true, "insertion at the start of a scene that follows a heading")
        f.caret((f.tv.string as NSString).length); f.type(" End.")
        check(f, table, local: true, "typing at the very end of the manuscript")
    }

    @Test("an undo-style scene replacement is maintained; a structural edit rescans and is still exact")
    func replacementsAndStructure() {
        let (f, table) = fixture()
        let ts = f.tv.textStorage!
        // `applySceneChange` replaces a scene's whole range with its restored text.
        let mid = table.ranges(in: ts)[1]
        ts.replaceCharacters(in: mid, with: NSAttributedString(string: "Restored beta, longer than before.", attributes: body))
        check(f, table, local: true, "undo apply over a whole scene")
        // An empty scene, then text typed into it.
        ts.replaceCharacters(in: table.ranges(in: ts)[1], with: NSAttributedString(string: "", attributes: body))
        check(f, table, local: true, "a scene emptied")
        f.caret(table.ranges(in: ts)[1].location); f.type("Refilled")
        check(f, table, local: true, "typing into an empty scene")
        // A cross-scene replacement (as a structural op would make) must rescan — and be exact.
        let a = table.ranges(in: ts)[0], c = table.ranges(in: ts)[2]
        ts.replaceCharacters(in: NSRange(location: a.location + 2, length: c.location + 2 - (a.location + 2)),
                             with: NSAttributedString(string: "--", attributes: body))
        #expect(table.isDirty, "an edit spanning scenes marks the table dirty")
        check(f, table, local: false, "cross-scene replacement")
        // Inserting a NEW divider (a split) brings structure: rescan, exact.
        let at = table.ranges(in: ts)[0].location + 3
        ts.replaceCharacters(in: NSRange(location: at, length: 0),
                             with: SceneDivider.string(NSTextAttachment(), state: .sceneBreak, newlineAttributes: body))
        #expect(table.isDirty, "an inserted divider marks the table dirty")
        check(f, table, local: false, "inserted divider")
        // A rebuild (whole-storage replacement) — then `reset` with exact ranges, as rebuildStorage does.
        ts.setAttributedString(manuscript())
        #expect(table.isDirty)
        table.reset(SceneDivider.sceneBoundaries(in: ts)!)
        check(f, table, local: true, "after a rebuild + reset")
    }
}

/// EP-045 AC7 — indented text TYPED into the real view stays prose: its escapes are hidden.
@Suite("Block intents as prose (EP-045 AC7)")
@MainActor
struct BlockIntentsAsProseTests {
    @Test("a tab-led and a 4-space-led paragraph keep their escapes hidden")
    func indentedTypingIsProse() {
        for (indent, mark) in [("\t", "*"), ("    ", "_")] {
            let f = ManuscriptFixture()
            f.type("\(indent)a\(mark)b")
            let backslash = indent.utf16.count + 1
            #expect(f.text == "\(indent)a\\\(mark)b")
            #expect(f.hidden(backslash), "indentation does not make it a code block: \(indent.debugDescription)")
        }
    }
}

/// Finds the test bundle (Swift Testing suites are structs; a class gives `Bundle(for:)` its anchor).
private final class CorpusBundleToken {}

/// EP-049 AC8 (SP-160) — the manuscript STORAGE FORMAT, shared with Linux. The SAME cases
/// (`ScriviCore/tests/fixtures/manuscript_format_corpus.json`, bundled as a test resource — the host is
/// sandboxed) run here through the real `ManuscriptNSTextView` and AppKit's own dispatch, and on Linux
/// through the real `ManuscriptEditor` (`escape_smoke`). ⚠️ A failure on either side means the two
/// platforms would write different bytes for the same gesture.
@Suite("Manuscript format corpus, shared with Linux (EP-049 AC8)")
@MainActor
struct ManuscriptFormatCorpusTests {

    private let appKitStringType = NSPasteboard.PasteboardType("NSStringPboardType")

    private func key(_ f: ManuscriptFixture, code: UInt16, chars: String, _ mods: NSEvent.ModifierFlags = []) {
        let e = NSEvent.keyEvent(with: .keyDown, location: .zero, modifierFlags: mods, timestamp: 0,
                                 windowNumber: f.window.windowNumber, context: nil, characters: chars,
                                 charactersIgnoringModifiers: chars, isARepeat: false, keyCode: code)!
        f.tv.keyDown(with: e)
    }

    @Test("every corpus gesture stores the corpus bytes")
    func corpus() throws {
        let url = try #require(Bundle(for: CorpusBundleToken.self)
            .url(forResource: "manuscript_format_corpus", withExtension: "json"), "the corpus is bundled")
        let doc = try #require(try JSONSerialization.jsonObject(with: Data(contentsOf: url)) as? [String: Any])
        let cases = try #require(doc["cases"] as? [[String: Any]])
        #expect(cases.count >= 18)
        for c in cases {
            let name = c["name"] as? String ?? "?"
            let f = ManuscriptFixture(c["before"] as? String ?? "")
            if let sel = c["selection"] as? [Int] {
                f.tv.setSelectedRange(NSRange(location: sel[0], length: sel[1] - sel[0]))
            } else {
                f.caret(c["caret"] as? Int ?? 0)
            }
            let text = c["text"] as? String ?? ""
            let pb = NSPasteboard(name: .init("scrivi.test.\(UUID().uuidString)"))
            pb.clearContents()
            switch c["gesture"] as? String {
            case "type": f.type(text)
            case "return": key(f, code: 36, chars: "\r")
            case "enter": key(f, code: 76, chars: "\u{3}", [.numericPad])
            case "shiftReturn": key(f, code: 36, chars: "\r", [.shift])
            case "altReturn": key(f, code: 36, chars: "\r", [.option])
            case "backspace": key(f, code: 51, chars: "\u{7f}")
            case "delete": key(f, code: 117, chars: "\u{F728}", [.function])
            case "paste":
                pb.setString(text, forType: .string)
                _ = f.tv.readSelection(from: pb)
            case "copy":
                _ = f.tv.writeSelection(to: pb, type: appKitStringType)
                #expect(pb.string(forType: .string) == c["clipboard"] as? String, "\(name): clipboard")
            case "copyThenPasteAtEnd":
                _ = f.tv.writeSelection(to: pb, type: appKitStringType)
                f.caret((f.text as NSString).length)
                _ = f.tv.readSelection(from: pb)
            default:
                Issue.record("\(name): unknown gesture")
            }
            #expect(f.text == c["after"] as? String,
                    "\(name): stored \(f.text.debugDescription), expected \(String(describing: c["after"]).debugDescription)")
        }
    }
}

/// EP-045 AC5 + AC6 — Return, Backspace and trailing whitespace, through the REAL view and
/// AppKit's OWN key dispatch (`keyDown` → `interpretKeyEvents` → the action), ⚠️ never by
/// calling `insertNewline` directly (`feedback_test_through_the_real_dispatch`: T-0579's tests
/// named the override's input themselves and passed while AppKit took another route).
@Suite("Return and Backspace (EP-045 AC5/AC6)")
@MainActor
struct ReturnAndBackspaceTests {

    private typealias Fixture = ManuscriptFixture

    /// Press Return-family keys as the keyboard does. Key codes: 36 Return, 76 keypad Enter.
    private func press(_ f: Fixture, keyCode: UInt16 = 36, _ mods: NSEvent.ModifierFlags = []) {
        let ch = keyCode == 76 ? "\u{3}" : "\r"
        let e = NSEvent.keyEvent(with: .keyDown, location: .zero, modifierFlags: mods, timestamp: 0,
                                 windowNumber: f.window.windowNumber, context: nil, characters: ch,
                                 charactersIgnoringModifiers: ch, isARepeat: false, keyCode: keyCode)!
        f.tv.keyDown(with: e)
    }

    @Test("AC5a: Return stores a blank line and leaves the caret after it")
    func returnInsertsBlankLine() {
        let f = Fixture("x")
        f.caret(1); press(f)
        #expect(f.text == "x\n\n")
        #expect(f.tv.selectedRange() == NSRange(location: 3, length: 0))
        let mid = Fixture("abcd")
        mid.caret(2); press(mid)
        #expect(mid.text == "ab\n\ncd", "Return mid-paragraph splits it")
        let sel = Fixture("abXYZ")
        sel.tv.setSelectedRange(NSRange(location: 2, length: 3)); press(sel)
        #expect(sel.text == "ab\n\n", "Return replaces a selection")
    }

    @Test("AC5a: keypad Enter and Shift-Return send the same action; Option-Return stores a hard break (T-0584)")
    func returnFamily() {
        let k = Fixture("x"); k.caret(1); press(k, keyCode: 76, [.numericPad])
        #expect(k.text == "x\n\n")
        let s = Fixture("x"); s.caret(1); press(s, [.shift])
        #expect(s.text == "x\n\n", "measured: Shift-Return sends insertNewline: like Return")
        let o = Fixture("x"); o.caret(1); press(o, [.option])
        // ✅ [T-0584] Q-E2-4 = (b), ruled 2026-10-05 — this expectation pinned the old, unruled behaviour.
        #expect(o.text == "x\\\n", "Option-Return (insertNewlineIgnoringFieldEditor:) stores `\\` + `\\n`")
    }

    @Test("AC6a: trailing spaces are reduced to AT MOST ONE (design: x␣␣␣ + Return → x␣⏎⏎)")
    func trailingSpaces() {
        for (before, after) in [("x   ", "x \n\n"), ("x  ", "x \n\n"), ("x ", "x \n\n"), ("x", "x\n\n")] {
            let f = Fixture(before)
            f.caret(before.utf16.count); press(f)
            #expect(f.text == after, "\(before.debugDescription)")
        }
        let mid = Fixture("ab   cd")
        mid.caret(5); press(mid)
        #expect(mid.text == "ab \n\ncd", "only the run BEFORE the caret is touched")
        let tabs = Fixture("x\t\t")
        tabs.caret(3); press(tabs)
        #expect(tabs.text == "x\t\t\n\n", "tabs are not a hard break and are left alone")
    }

    @Test("AC6b: a trailing typed backslash collapses to a bare one; only the FINAL pair")
    func trailingBackslash() {
        let f = Fixture()
        f.type("end\\")
        #expect(f.text == #"end\\"#)
        press(f)
        #expect(f.text == "end\\\n\n")
        #expect(!f.hidden(3), "Q1 = (a): before a blank line the backslash is LITERAL, so it shows")

        let two = Fixture()
        two.type("a\\\\")
        #expect(two.text == #"a\\\\"#)
        press(two)
        #expect(two.text == "a\\\\\\\n\n", "one literal backslash, then the break marker")

        let mark = Fixture()
        mark.type("a*")
        press(mark)
        #expect(mark.text == "a\\*\n\n", "an escaped MARK is never collapsed")
    }

    @Test("AC-undo: one Return is ONE edit and ONE textDidChange — so ONE history event")
    func returnIsOneEdit() {
        // ⚠️ The coordinator records history per `textDidChange` and commits on `\n`
        // (`isCommitBoundary`); one notification therefore means one undo step.
        let f = Fixture()
        f.type("end\\")
        var count = 0
        let token = NotificationCenter.default.addObserver(
            forName: NSText.didChangeNotification, object: f.tv, queue: nil) { _ in count += 1 }
        defer { NotificationCenter.default.removeObserver(token) }
        press(f)
        let g = Fixture("x   ")
        g.caret(4)
        let token2 = NotificationCenter.default.addObserver(
            forName: NSText.didChangeNotification, object: g.tv, queue: nil) { _ in count += 1 }
        defer { NotificationCenter.default.removeObserver(token2) }
        press(g)
        #expect(count == 2, "backslash collapse and space trim each happen inside their Return's one edit")
    }

    /// Backspace as the keyboard sends it (key code 51 → `deleteBackward:`).
    private func backspace(_ f: Fixture) {
        let e = NSEvent.keyEvent(with: .keyDown, location: .zero, modifierFlags: [], timestamp: 0,
                                 windowNumber: f.window.windowNumber, context: nil, characters: "\u{7f}",
                                 charactersIgnoringModifiers: "\u{7f}", isARepeat: false, keyCode: 51)!
        f.tv.keyDown(with: e)
    }

    @Test("Q3: ⌫ at a paragraph start JOINS it with one space — one edit (subsumes Q2)")
    func backspaceJoinsWithSpace() {
        let f = Fixture("a.\n\nb.")
        f.caret(4)
        var count = 0
        let token = NotificationCenter.default.addObserver(
            forName: NSText.didChangeNotification, object: f.tv, queue: nil) { _ in count += 1 }
        defer { NotificationCenter.default.removeObserver(token) }
        backspace(f)
        #expect(f.text == "a. b.", "what EP-046 would render for the soft break")
        #expect(f.tv.selectedRange() == NSRange(location: 3, length: 0), "caret stays before b")
        #expect(count == 1, "ONE edit, so ONE undo step")

        // Q2: trailing spaces become exactly one — no hard break can form (no newline is left).
        for (before, caret) in [("x   \n\nb", 6), ("x \n\nb", 4)] {
            let g = Fixture(before)
            g.caret(caret); backspace(g)
            #expect(g.text == "x b", "\(before.debugDescription)")
        }
        // §4B.6's inverse route: Return FIRST, spaces added to the line above later.
        let late = Fixture("x\n\nb")
        late.caret(1); late.type("  ")
        late.caret(5); backspace(late)
        #expect(late.text == "x b")
        // An escaped (literal) backslash is not a break marker: join as usual.
        let lit = Fixture("end\\\\\n\nnext")
        lit.caret(7); backspace(lit)
        #expect(lit.text == "end\\\\ next")
    }

    @Test("Q3: ⌫ only removes one newline when a paragraph is EMPTY, or after a single soft break")
    func backspaceOneCharacterCases() {
        let below = Fixture("a.\n\n")
        below.caret(4); backspace(below)
        #expect(below.text == "a.\n", "the paragraph being joined is empty")
        let above = Fixture("a.\n\n\n\nb")
        above.caret(6); backspace(above)
        #expect(above.text == "a.\n\n\nb", "the line above is blank: just remove a blank line")
        let soft = Fixture("x  \nb")
        soft.caret(4); backspace(soft)
        #expect(soft.text == "x  b", "a single soft break (existing text) is one character")
    }

    @Test("the deliberate break: \\ + Return, then ⌫ merges to a HIDDEN hard break")
    func backslashBreakAfterMerge() {
        let f = Fixture()
        f.type("end\\")
        press(f)
        f.type("next")
        #expect(f.text == "end\\\n\nnext")
        #expect(!f.hidden(3))
        f.caret(6); backspace(f)
        #expect(f.text == "end\\\nnext", "Q3 exception: the deliberate break survives the join")
        #expect(f.hidden(3), "the line ABOVE the edit is restyled: now a hard break, hidden")
        // A second ⌫ removes the break and its hidden marker together (AC4 pair-widening).
        f.tv.deleteBackward(nil)
        #expect(f.text == "endnext")
    }
}
/// Counts `textDidChange` through the delegate (a notification observer's closure is `@Sendable`).
@MainActor private final class DidChangeCounter: NSObject, NSTextViewDelegate {
    var count = 0
    func textDidChange(_ notification: Notification) { count += 1 }
}

/// EP-046 E2-S1 (SP-161, T-0588) — the PRESENTER (route (a′)): headings render, storage stays plain,
/// the caret's line reveals its prefix, and the caret never rests on an invisible stop.
/// ⚠️ Where it matters, these read what TextKit LAID OUT, not only what the presenter answers —
/// a presenter that is never asked again after an edit would pass a query-only test.
@Suite("Manuscript presenter (EP-046 E2-S1)")
@MainActor
struct ManuscriptPresenterTests {

    private let doc = "Body line.\n\n## Heading two\n\nMore body."   // `##` at 12; its line is 12..<27
    private let headingLine = NSRange(location: 12, length: 15)

    private func presented(_ f: ManuscriptFixture, _ r: NSRange) throws -> NSAttributedString {
        let cs = try #require(f.tv.textContentStorage)
        return try #require(f.presenter.textContentStorage(cs, textParagraphWith: r)).attributedString
    }

    /// The laid-out width of the first line fragment of the paragraph holding `loc`.
    private func laidOutWidth(_ f: ManuscriptFixture, at loc: Int) throws -> CGFloat {
        let tlm = try #require(f.tv.textLayoutManager), cs = try #require(f.tv.textContentStorage)
        tlm.ensureLayout(for: tlm.documentRange)
        let at = try #require(cs.location(cs.documentRange.location, offsetBy: loc))
        let frag = try #require(tlm.textLayoutFragment(for: at))
        return try #require(frag.textLineFragments.first).typographicBounds.width
    }

    private func caretPoint(_ f: ManuscriptFixture) -> NSPoint {
        f.tv.textLayoutManager?.ensureLayout(for: f.tv.textLayoutManager!.documentRange)
        return f.tv.firstRect(forCharacterRange: f.tv.selectedRange(), actualRange: nil).origin
    }

    @Test("the analyzer finds the ATX headings the parser reports, and only those (AC2, AC7)")
    func analyzer() {
        func h(_ s: String) -> [MarkdownBlocks.Heading] { MarkdownBlocks.analyze(s).headings }
        // The dumas fixture's form, trailing space included.
        #expect(h("## The claim stated plainly ") ==
                [.init(line: NSRange(location: 0, length: 28), prefix: NSRange(location: 0, length: 3), level: 2)])
        for n in 1...6 { #expect(h(String(repeating: "#", count: n) + " T").first?.level == n) }
        #expect(h("####### seven").isEmpty)
        #expect(h("\\#\\# typed").isEmpty, "typed `#` is escaped and never a heading")
        #expect(h("    ## indented").isEmpty, "AC7: an indented block is prose")
        #expect(h("> ## quoted").isEmpty, "AC7: a quote is prose")
        #expect(h("- ## listed").isEmpty)
        #expect(h("Title\n=====").isEmpty, "setext: drawn as stored (design §13)")
        #expect(h("para line\n## Interrupts") ==
                [.init(line: NSRange(location: 10, length: 13), prefix: NSRange(location: 10, length: 3), level: 2)])
    }

    @Test("AC1 + AC2: the heading is PRESENTED (Q1 font, hidden prefix); storage keeps no rendering attribute")
    func headingPresented() throws {
        let f = ManuscriptFixture(doc)
        f.caret(0)
        let p = try presented(f, headingLine)
        #expect(p.length == headingLine.length, "Apple's contract: the SAME length")
        #expect((p.attribute(.font, at: 3, effectiveRange: nil) as? NSFont)?.pointSize == ManuscriptTypography.default.headingSize(level: 2),
                "Q1: H2 = 18/13 of the body size (EP-047 keeps Apple's heading sizes as ratios)")
        #expect(((p.attribute(.font, at: 0, effectiveRange: nil) as? NSFont)?.pointSize ?? 99) < 0.1, "prefix hidden")
        #expect(f.hidden(12) && f.hidden(14) && !f.hidden(15))
        // ✅ AC1: nothing the presenter shows is in STORAGE.
        let ts = f.tv.textStorage!
        var sizes = Set<CGFloat>()
        ts.enumerateAttribute(.font, in: NSRange(location: 0, length: ts.length)) { v, _, _ in
            if let font = v as? NSFont { sizes.insert(font.pointSize) }
        }
        #expect(sizes == [ManuscriptTypography.default.size], "storage fonts: \(sizes)")
        // ✅ And TextKit LAID IT OUT that way: the line is as wide as the heading text alone.
        let target = ("Heading two" as NSString).size(withAttributes: [.font: ManuscriptTypography.default.font(level: 2, style: 0)]).width
        #expect(abs(try laidOutWidth(f, at: 12) - target) < 0.5)
    }

    @Test("AC4 (line half): the caret's line shows its prefix — attributes only, no textDidChange")
    func lineReveal() throws {
        let f = ManuscriptFixture(doc)
        let counter = DidChangeCounter()
        f.tv.delegate = counter
        f.caret(0)
        let hiddenWidth = try laidOutWidth(f, at: 12)
        f.caret(17)                                   // inside "Heading"
        #expect(!f.hidden(12), "revealed on the caret's line")
        let p = try presented(f, headingLine)
        #expect(p.attribute(.foregroundColor, at: 0, effectiveRange: nil) as? NSColor == .tertiaryLabelColor, "Q2: dimmed")
        #expect((p.attribute(.font, at: 0, effectiveRange: nil) as? NSFont)?.pointSize == ManuscriptTypography.default.size, "Q2: body font")
        #expect(try laidOutWidth(f, at: 12) > hiddenWidth + 10, "TextKit re-laid the revealed line")
        f.caret(0)
        #expect(f.hidden(12))
        #expect(abs(try laidOutWidth(f, at: 12) - hiddenWidth) < 0.01, "hidden again when the caret leaves")
        #expect(counter.count == 0, "a reveal is not an edit — no history event")
        #expect(f.text == doc)
    }

    @Test("AC5: arrowing through escapes and a heading never rests on an invisible stop; shift-selection never stalls")
    func noInvisibleStops() {
        let text = "a \\*b\n\n## Head \\#x\n\nend"
        let f = ManuscriptFixture(text)
        let n = (text as NSString).length
        f.caret(0)
        var stalls: [Int] = []
        var last = caretPoint(f)
        while f.tv.selectedRange().location < n {
            let before = f.tv.selectedRange().location
            f.tv.moveRight(nil)
            let p = caretPoint(f)
            if abs(p.x - last.x) < 0.05 && abs(p.y - last.y) < 0.05 { stalls.append(f.tv.selectedRange().location) }
            if f.tv.selectedRange().location == before { break }
            last = p
        }
        #expect(stalls.isEmpty, "→ rested where the caret did not visibly move: \(stalls)")
        // ← back to the start
        stalls = []
        last = caretPoint(f)
        while f.tv.selectedRange().location > 0 {
            let before = f.tv.selectedRange().location
            f.tv.moveLeft(nil)
            let p = caretPoint(f)
            if abs(p.x - last.x) < 0.05 && abs(p.y - last.y) < 0.05 { stalls.append(f.tv.selectedRange().location) }
            if f.tv.selectedRange().location == before { break }
            last = p
        }
        #expect(stalls.isEmpty, "← rested where the caret did not visibly move: \(stalls)")
        // Shift-selection, both ways: every step moves the moving end (E1's snap stalled shrinking).
        f.caret(0)
        var ends = [0]
        for _ in 0..<n { f.tv.moveRightAndModifySelection(nil); ends.append(NSMaxRange(f.tv.selectedRange())) }
        #expect(ends.last == n && zip(ends, ends.dropFirst()).allSatisfy { $0 < $1 || $0 == n }, "shift-→ ends: \(ends)")
        ends = [n]
        for _ in 0..<n { f.tv.moveLeftAndModifySelection(nil); ends.append(NSMaxRange(f.tv.selectedRange())) }
        #expect(ends.last == 0 && zip(ends, ends.dropFirst()).allSatisfy { $0 > $1 || $0 == 0 }, "shift-← ends: \(ends)")
    }

    @Test("a block ends at the divider CHARACTER: a scene with no final newline does not swallow the next heading")
    func blockEndsAtDivider() throws {
        let f = ManuscriptFixture()
        let s = NSMutableAttributedString(string: "testr")       // the four such dumas files (step 1)
        s.append(NSAttributedString(string: "\u{FFFC}", attributes: [.scriviDivider: DividerRenderState.sceneBreak]))
        s.append(NSAttributedString(string: "\n## Next scene\n\nbody"))
        f.tv.textStorage!.setAttributedString(s)
        f.caret(s.length)
        let ts = f.tv.textStorage!
        #expect(f.presenter.block(in: ts, at: 0) == NSRange(location: 0, length: 5), "the scene text before the divider")
        #expect(f.presenter.block(in: ts, at: 5) == nil, "the divider is not scene text")
        #expect(f.hidden(7), "`## Next scene` is a heading of its own block")
        // A chapter heading (view-inserted, not scene text) is never presented.
        let g = ManuscriptFixture()
        g.tv.textStorage!.setAttributedString(NSAttributedString(string: "## Chapter\n", attributes: [.scriviHeading: true]))
        #expect(g.presenter.block(in: g.tv.textStorage!, at: 0) == nil)
    }

    @Test("an edit re-presents its WHOLE block: a ⌫ merge hides the hard break on the line ABOVE, as laid out")
    func editRepresentsBlock() throws {
        let f = ManuscriptFixture("end\\\n\nnext")
        f.caret(6)
        let visible = try laidOutWidth(f, at: 0)
        f.tv.deleteBackward(nil)
        #expect(f.text == "end\\\nnext")
        #expect(try laidOutWidth(f, at: 0) < visible - 1, "the backslash stopped taking width")
        // ⚠️ Compared with a LAID-OUT `end` (the fixture's text carries the view's default font).
        let plain = ManuscriptFixture("end")
        #expect(abs(try laidOutWidth(f, at: 0) - laidOutWidth(plain, at: 0)) < 0.05)
    }
}
/// EP-046 E2-S2 (SP-162, T-0590) — bold and italic, the span reveal, the caret's home beside a marker
/// (Q1), and edits that keep emphasis BALANCED (AC12, Q3). ⚠️ Stored bytes are asserted exactly: the
/// balancing writes the fewest markers, so the expected text is canonical.
@Suite("Inline emphasis (EP-046 E2-S2)")
@MainActor
struct InlineEmphasisTests {

    private let appKitStringType = NSPasteboard.PasteboardType("NSStringPboardType")

    /// A fixture whose storage carries the BODY font, as the app's does (so widths compare).
    private func fixture(_ text: String) -> ManuscriptFixture {
        let f = ManuscriptFixture()
        f.tv.textStorage!.setAttributedString(NSAttributedString(string: text, attributes: [
            .font: ManuscriptTypography.default.bodyFont, .foregroundColor: NSColor.textColor]))
        return f
    }

    private func pasteboard() -> NSPasteboard {
        let pb = NSPasteboard(name: .init("scrivi.test.\(UUID().uuidString)"))
        pb.clearContents()
        return pb
    }

    private func laidOutWidth(_ f: ManuscriptFixture, at loc: Int) throws -> CGFloat {
        let tlm = try #require(f.tv.textLayoutManager), cs = try #require(f.tv.textContentStorage)
        tlm.ensureLayout(for: tlm.documentRange)
        let at = try #require(cs.location(cs.documentRange.location, offsetBy: loc))
        return try #require(tlm.textLayoutFragment(for: at)?.textLineFragments.first).typographicBounds.width
    }

    private func key(_ f: ManuscriptFixture, code: UInt16, chars: String) {
        let e = NSEvent.keyEvent(with: .keyDown, location: .zero, modifierFlags: [], timestamp: 0,
                                 windowNumber: f.window.windowNumber, context: nil, characters: chars,
                                 charactersIgnoringModifiers: chars, isARepeat: false, keyCode: code)!
        f.tv.keyDown(with: e)
    }

    @Test("the analyzer: markers (opening/closing), styles, spans — and what is NOT a marker")
    func analyzer() {
        let a = MarkdownBlocks.analyze("This is **emphasized** and *it*.")
        #expect(a.markers == [.init(range: NSRange(location: 8, length: 2), opens: true),
                              .init(range: NSRange(location: 20, length: 2), opens: false),
                              .init(range: NSRange(location: 27, length: 1), opens: true),
                              .init(range: NSRange(location: 30, length: 1), opens: false)])
        #expect(a.style(at: 10) == MarkdownBlocks.bold && a.style(at: 28) == MarkdownBlocks.italic && a.style(at: 0) == 0)
        #expect(a.spans == [NSRange(location: 8, length: 14), NSRange(location: 27, length: 4)])
        let nested = MarkdownBlocks.analyze("**a *b* c**")
        #expect(nested.style(at: 5) == MarkdownBlocks.bold | MarkdownBlocks.italic)
        #expect(nested.markers.map(\.opens) == [true, true, false, false])
        #expect(nested.spans == [NSRange(location: 0, length: 11)], "one span: the reveal shows all its markers")
        #expect(MarkdownBlocks.analyze("* item one").markers.isEmpty, "a bullet is not emphasis")
        #expect(MarkdownBlocks.analyze("**unclosed bold").markers.isEmpty, "Q-E2-7: shown as the parser reads it")
        #expect(MarkdownBlocks.analyze(#"2 \* 3 \* 4"#).markers.isEmpty)
        // ✅ AC7 (inline half): an indented paragraph and a quote render their emphasis; a table does not.
        #expect(MarkdownBlocks.analyze("    an **indented** line").markers.count == 2)
        #expect(MarkdownBlocks.analyze("> a **quoted** line").markers.count == 2)
        #expect(MarkdownBlocks.analyze("| **a** | b |\n|---|---|\n| 1 | 2 |").markers.isEmpty)
    }

    @Test("live pass: bold is a REAL weight step (Bold 0.40 on body, Heavy in a heading) — not Semibold")
    func boldWeight() throws {
        let f = fixture("x **bold** y\n\n## a **b** c")
        f.caret(0)
        let cs = try #require(f.tv.textContentStorage)
        func weight(_ para: NSRange, _ at: Int) throws -> CGFloat {
            let p = try #require(f.presenter.textContentStorage(cs, textParagraphWith: para)).attributedString
            let font = try #require(p.attribute(.font, at: at, effectiveRange: nil) as? NSFont)
            let traits = font.fontDescriptor.object(forKey: .traits) as? [NSFontDescriptor.TraitKey: Any]
            return CGFloat((traits?[.weight] as? Double) ?? -1)
        }
        #expect(try weight(NSRange(location: 0, length: 13), 4) >= NSFont.Weight.bold.rawValue - 0.01, "body bold is Bold, not Semibold (0.30)")
        #expect(try weight(NSRange(location: 14, length: 12), 7) >= NSFont.Weight.heavy.rawValue - 0.01, "bold inside a heading is Heavy")
    }

    @Test("AC3: bold is PRESENTED and laid out without its markers; storage keeps one plain font")
    func boldPresented() throws {
        let f = fixture("This is **emphasized**.")
        f.caret(0)
        #expect(f.hidden(8) && f.hidden(9) && f.hidden(20) && f.hidden(21) && !f.hidden(10))
        // ⚠️ EP-047: compare with the SAME text, styled the same, WITHOUT markers. (It compared bold "emphasized" with PLAIN
        // "emphasized", which only matched while bold and regular were the same width — a monospaced face.)
        let t = ManuscriptTypography.default
        let expected = NSMutableAttributedString(string: "This is ", attributes: [.font: t.bodyFont])
        expected.append(NSAttributedString(string: "emphasized", attributes: [.font: t.font(level: nil, style: MarkdownBlocks.bold)]))
        expected.append(NSAttributedString(string: ".", attributes: [.font: t.bodyFont]))
        let laid = try laidOutWidth(f, at: 0)
        #expect(abs(laid - expected.size().width) < 0.5, "the markers take no width (laid out \(laid), unmarked \(expected.size().width))")
        let cs = try #require(f.tv.textContentStorage)
        let p = try #require(f.presenter.textContentStorage(cs, textParagraphWith: NSRange(location: 0, length: 23))).attributedString
        #expect((p.attribute(.font, at: 10, effectiveRange: nil) as? NSFont)?.fontDescriptor.symbolicTraits.contains(.bold) == true)
        var sizes = Set<CGFloat>()
        f.tv.textStorage!.enumerateAttribute(.font, in: NSRange(location: 0, length: 23)) { v, _, _ in
            if let font = v as? NSFont, !font.fontDescriptor.symbolicTraits.contains(.bold) { sizes.insert(font.pointSize) }
        }
        #expect(sizes == [ManuscriptTypography.default.size], "AC1: no rendering attribute reached storage")
    }

    @Test("AC4 (span half): markers show only with the caret at the span's FIRST or LAST character — no textDidChange")
    func spanReveal() throws {
        let f = fixture("a **bold** b *it* c")
        let counter = DidChangeCounter()
        f.tv.delegate = counter
        f.caret(0)
        let hidden = try laidOutWidth(f, at: 0)
        f.caret(4)                                    // the first character of "bold"
        #expect(!f.hidden(2) && !f.hidden(8), "this span's markers show")
        #expect(f.hidden(13) && f.hidden(16), "the other span's do not")
        #expect(try laidOutWidth(f, at: 0) > hidden + 10, "TextKit re-laid the revealed span")
        // ✅ Live-pass amendment (user, 2026-10-05): in the MIDDLE of the span the hints go away…
        f.caret(6)
        #expect(f.hidden(2) && f.hidden(8), "mid-span: hidden")
        // …and come back at its last character.
        f.caret(8)
        #expect(!f.hidden(2) && !f.hidden(8), "end of the span: shown")
        f.caret(0)
        #expect(f.hidden(2))
        #expect(abs(try laidOutWidth(f, at: 0) - hidden) < 0.01)
        #expect(counter.count == 0)
    }

    @Test("Q1: the caret goes AFTER an opener (hint to its left) and BEFORE a closer; → and ← never stall")
    func caretHomes() {
        let f = fixture("a **bold** b")             // `**` at 2..<4 and 8..<10
        // ⚠️ From a NEUTRAL caret each time: a placement one unit from home looks exactly like an arrow
        // step out of the span, and is (correctly) treated as one.
        f.caret(0); f.caret(2); #expect(f.tv.selectedRange().location == 4, "start of the bold word: inside, after `**`")
        f.caret(0); f.caret(3); #expect(f.tv.selectedRange().location == 4)
        f.caret(0); f.caret(10); #expect(f.tv.selectedRange().location == 8, "end of the bold word: before the closer")
        f.caret(0); f.caret(9); #expect(f.tv.selectedRange().location == 8)
        f.caret(0)
        var seq = [0]
        for _ in 0..<8 { f.tv.moveRight(nil); seq.append(f.tv.selectedRange().location) }
        #expect(seq == [0, 1, 4, 5, 6, 7, 8, 11, 12], "→ stops: \(seq)")
        var back = [12]
        for _ in 0..<8 { f.tv.moveLeft(nil); back.append(f.tv.selectedRange().location) }
        #expect(back == [12, 11, 8, 7, 6, 5, 4, 1, 0], "← stops: \(back)")
        // Headings too (*"headers too"*): a caret proposed at the line start lands after `## `.
        let g = fixture("x\n\n## Head")
        g.caret(3); #expect(g.tv.selectedRange().location == 6)
    }

    @Test("AC12: a cut that starts INSIDE bold and ends outside closes the span; the cut text carries its own markers")
    func cutInsideToOutside() {
        let f = fixture("**bold** and")
        let counter = DidChangeCounter()
        f.tv.delegate = counter
        let pb = pasteboard()
        f.tv.setSelectedRange(NSRange(location: 4, length: 7))       // "ld** an"
        #expect(f.tv.writeSelection(to: pb, type: appKitStringType))
        #expect(pb.string(forType: .string) == "ld an", "Q2: other apps get no markers")
        f.tv.deleteBackward(nil)
        #expect(f.text == "**bo**d")
        #expect(counter.count == 1, "one balanced edit, one history event")
        f.caret((f.text as NSString).length)
        #expect(f.tv.readSelection(from: pb))
        #expect(f.text == "**bo**d**ld** an", "Scrivi's own paste keeps the bold")
    }

    @Test("AC12: a cut that starts OUTSIDE and ends inside re-opens the span; inside on both ends keeps it")
    func cutOutsideToInside() {
        let f = fixture("pre **bold**")
        let pb = pasteboard()
        f.tv.setSelectedRange(NSRange(location: 2, length: 6))       // "e **bo"
        _ = f.tv.writeSelection(to: pb, type: appKitStringType)
        f.tv.deleteBackward(nil)
        #expect(f.text == "pr**ld**")
        let g = fixture("**abcd**")
        let pb2 = pasteboard()
        g.tv.setSelectedRange(NSRange(location: 3, length: 2))       // "bc"
        _ = g.tv.writeSelection(to: pb2, type: appKitStringType)
        g.tv.deleteBackward(nil)
        #expect(g.text == "**ad**")
        g.caret(3)                                                   // between a and d — inside the bold
        #expect(g.tv.readSelection(from: pb2))
        #expect(g.text == "**abcd**", "bold pasted into bold MERGES — it does not toggle")
    }

    @Test("Q3: typing over, Return inside, and ⌫ of a span's last character all leave it balanced")
    func everyReplacementBalances() {
        let f = fixture("**bold** and")
        f.tv.setSelectedRange(NSRange(location: 4, length: 7))
        f.type("X")
        #expect(f.text == "**boX**d", "the typed text takes the style of the first selected character")
        let g = fixture("**bold**")
        g.caret(4)
        key(g, code: 36, chars: "\r")
        #expect(g.text == "**bo**\n\n**ld**", "Return inside bold splits it into two balanced spans")
        #expect(g.tv.selectedRange().location == 10, "the caret starts the new paragraph inside its bold")
        let h = fixture("a **b** c")
        h.caret(5)
        h.tv.deleteBackward(nil)
        #expect(h.text == "a  c", "an emptied span takes its markers with it")
    }

    @Test("markers are ATOMIC: ⌫ after an opener, ⌫ at a heading's start, ⌦ before a closer")
    func atomicMarkers() {
        let f = fixture("a **bold**")
        f.caret(4)
        f.tv.deleteBackward(nil)
        #expect(f.text == "a**bold**", "⌫ deletes the character before the marker, not the marker")
        #expect(f.tv.selectedRange().location == 3)
        let g = fixture("x\n\n## Head")
        g.caret(6)
        g.tv.deleteBackward(nil)
        #expect(g.text == "x\n\nHead", "⌫ at the start of a heading makes it body text")
        let h = fixture("**bold** x")
        h.caret(6)
        h.tv.deleteForward(nil)
        #expect(h.text == "**bold**x", "⌦ deletes the character after the marker")
    }
}
/// EP-046 E2-S3 (SP-163, T-0592 + T-0584 + T-0591) — the Format commands (the ONLY way formatting enters a manuscript),
/// lists, the pending pair (Q1) and balancing across scene boundaries. Driven through `applyFormat`, the method the
/// Format menu calls, and AppKit's own key dispatch for Return; stored bytes asserted exactly.
@Suite("Format commands (EP-046 E2-S3)")
@MainActor
struct FormatCommandTests {

    private func fixture(_ text: String) -> ManuscriptFixture {
        let f = ManuscriptFixture()
        f.tv.textStorage!.setAttributedString(NSAttributedString(string: text, attributes: [
            .font: ManuscriptTypography.default.bodyFont, .foregroundColor: NSColor.textColor]))
        return f
    }

    private func key(_ f: ManuscriptFixture, code: UInt16, chars: String) {
        let e = NSEvent.keyEvent(with: .keyDown, location: .zero, modifierFlags: [], timestamp: 0,
                                 windowNumber: f.window.windowNumber, context: nil, characters: chars,
                                 charactersIgnoringModifiers: chars, isARepeat: false, keyCode: code)!
        f.tv.keyDown(with: e)
    }

    @Test("⌘B on a selection: ONE edit, whitespace at the edges left out; again toggles it off")
    func boldToggle() {
        let f = fixture("Some words to make bold here.")
        let counter = DidChangeCounter()
        f.tv.delegate = counter
        f.tv.setSelectedRange(NSRange(location: 4, length: 15))           // " words to make " — with its spaces
        f.tv.applyFormat(.bold)
        #expect(f.text == "Some **words to make** bold here.")
        #expect(counter.count == 1, "one command, one history event")
        // The formatted text stays selected; the selection snap takes the opening `**` with it (E2-S2: a selection's
        // start never sits just after a marker).
        #expect(f.tv.selectedRange() == NSRange(location: 5, length: 15), "the formatted text stays selected")
        f.tv.applyFormat(.bold)
        #expect(f.text == "Some words to make bold here.", "the same command again takes it off")
    }

    @Test("⌘I writes `*` and nests inside bold; un-bolding the middle of a span splits it")
    func italicAndSplit() {
        let f = fixture("**alpha beta gamma**")
        f.tv.setSelectedRange(NSRange(location: 8, length: 4))            // "beta"
        f.tv.applyFormat(.italic)
        #expect(f.text == "**alpha *beta* gamma**")
        let g = fixture("**alpha beta gamma**")
        g.tv.setSelectedRange(NSRange(location: 8, length: 4))
        g.tv.applyFormat(.bold)
        #expect(g.text == "**alpha** beta **gamma**")
    }

    @Test("Q1: ⌘B with the caret IN a word formats the word; the caret stays in place")
    func caretInWord() {
        let f = fixture("Some words here")
        f.caret(7)                                                       // "wo|rds"
        f.tv.applyFormat(.bold)
        #expect(f.text == "Some **words** here")
        #expect(f.tv.selectedRange() == NSRange(location: 9, length: 0))
    }

    @Test("Q1: BETWEEN words — a pending pair; typing fills it, leaving it or ⌘B again removes it, nothing recorded")
    func pendingPair() {
        let f = fixture("a  b")
        let counter = DidChangeCounter()
        f.tv.delegate = counter
        f.caret(2)
        f.tv.applyFormat(.bold)
        #expect(f.text == "a **** b", "the hints, smashed together")
        #expect(f.tv.selectedRange().location == 4, "the caret between them")
        #expect(!f.hidden(2) && !f.hidden(5), "shown as hints")
        #expect(counter.count == 0, "nothing recorded yet")
        f.type("x")
        #expect(f.text == "a **x** b", "typing makes it a real span")
        #expect(counter.count == 1)
        // Abandoned: the caret leaves an EMPTY pair → it goes, and nothing was recorded.
        let g = fixture("a  b")
        let c2 = DidChangeCounter()
        g.tv.delegate = c2
        g.caret(2); g.tv.applyFormat(.italic)
        #expect(g.text == "a ** b")
        g.caret(0)
        #expect(g.text == "a  b")
        #expect(c2.count == 0, "an abandoned pair leaves no history")
        // ⌘B again on the pending pair takes it away.
        let h = fixture("a  b")
        h.caret(2); h.tv.applyFormat(.bold); h.tv.applyFormat(.bold)
        #expect(h.text == "a  b")
        // Whitespace typed into it goes BEFORE it; the pair stays pending. (⚠️ Between TWO spaces: a caret just before
        // a letter is IN that word, per Q1.)
        let k = fixture("a  b")
        k.caret(2); k.tv.applyFormat(.bold); k.type(" ")
        #expect(k.text == "a  **** b")
        #expect(k.tv.selectedRange().location == 5)
    }

    @Test("headings and body: set, replace (Q5), toggle back; the caret keeps its text")
    func headings() {
        let f = fixture("Title line\n\nbody")
        f.caret(3)
        f.tv.applyFormat(.heading(2))
        #expect(f.text == "## Title line\n\nbody")
        f.tv.applyFormat(.heading(1))
        #expect(f.text == "# Title line\n\nbody", "a heading command replaces the level")
        f.tv.applyFormat(.heading(1))
        #expect(f.text == "Title line\n\nbody", "the same level again is body")
        f.tv.applyFormat(.bulletList)
        f.tv.applyFormat(.heading(3))
        #expect(f.text == "### Title line\n\nbody", "Q5: a heading replaces a list prefix")
    }

    @Test("lists: per paragraph, numbered SEQUENTIALLY (Q4); Return continues and renumbers; an empty item ends it")
    func lists() {
        let f = fixture("alpha\n\nbeta")
        f.tv.setSelectedRange(NSRange(location: 0, length: 11))
        f.tv.applyFormat(.bulletList)
        #expect(f.text == "- alpha\n\n- beta")
        f.tv.setSelectedRange(NSRange(location: 0, length: (f.text as NSString).length))
        f.tv.applyFormat(.numberedList)
        #expect(f.text == "1. alpha\n\n2. beta")
        f.caret(8)                                                       // end of "alpha"
        key(f, code: 36, chars: "\r")
        #expect(f.text == "1. alpha\n\n2. \n\n3. beta", "the next item, and what follows renumbered")
        #expect(f.tv.selectedRange().location == 13, "the caret after the new prefix")
        key(f, code: 36, chars: "\r")
        #expect(f.text == "1. alpha\n\n\n\n2. beta", "Return on an EMPTY item ends it; what follows closes the gap (Q4)")
        let g = fixture("- item")
        g.caret(2)
        g.tv.deleteBackward(nil)
        #expect(g.text == "item", "⌫ at an item's start makes it body text")
    }

    @Test("Q7: a list prefix stays VISIBLE, dimmed, with a hanging indent; the caret's home is after it")
    func listRendering() throws {
        let f = fixture("- an item long enough to wrap")
        f.caret(0)
        #expect(f.tv.selectedRange().location == 2, "the caret goes after `- `")
        #expect(!f.hidden(0) && !f.hidden(1))
        let cs = try #require(f.tv.textContentStorage)
        let p = try #require(f.presenter.textContentStorage(cs, textParagraphWith: NSRange(location: 0, length: 29))).attributedString
        #expect(p.attribute(.foregroundColor, at: 0, effectiveRange: nil) as? NSColor == .tertiaryLabelColor)
        #expect(((p.attribute(.paragraphStyle, at: 2, effectiveRange: nil) as? NSParagraphStyle)?.headIndent ?? 0) > 5,
                "wrapped lines align under the text")
    }

    @Test("live pass, step 12: Return at the START of item 1 inserts an empty item above; the caret stays with its text")
    func returnAtItemStart() {
        let f = fixture("1. alpha\n\n2. beta\n\n3. gamma")
        f.caret(0)
        #expect(f.tv.selectedRange().location == 3, "the caret's home is after `1. `")
        key(f, code: 36, chars: "\r")
        #expect(f.text == "1. \n\n2. alpha\n\n3. beta\n\n4. gamma", "the prefix of item 1 is kept")
        #expect(f.tv.selectedRange().location == 8, "the caret stays at the start of `alpha`, now item 2")
    }

    @Test("live pass, step 17: Scrivi's own paste keeps ITS formatting — plain text pasted inside bold stays plain")
    func ownPasteKeepsFormatting() {
        let f = fixture("**abcd** xy")
        let pb = NSPasteboard(name: .init("scrivi.test.\(UUID().uuidString)"))
        pb.clearContents()
        f.tv.setSelectedRange(NSRange(location: 9, length: 2))            // "xy" (plain)
        _ = f.tv.writeSelection(to: pb, type: NSPasteboard.PasteboardType("NSStringPboardType"))
        f.caret(0); f.caret(4)                                            // between "ab" and "cd" — inside the bold
        #expect(f.tv.readSelection(from: pb))
        let toks = MarkdownEmphasis.tokens(of: f.text).filter { ManuscriptPresenter.isWordUnit($0.unit) }
        #expect(toks.map { $0.style == MarkdownBlocks.bold } == [true, true, false, false, true, true, false, false],
                "stored \(f.text.debugDescription): ab cd bold, the pasted xy plain")
    }

    @Test("live pass: a SPACE typed or left at a span's edge goes OUTSIDE it — the span never shows its markers")
    func edgeWhitespace() {
        let f = fixture("x **bold** y")
        f.caret(0); f.caret(4)                                           // the start of "bold" (home after `**`)
        f.type(" ")
        #expect(f.text == "x  **bold** y", "a space at the start goes before the opener")
        let g = fixture("x **bold** y")
        g.caret(0); g.caret(8)                                           // the end of "bold" (home before `**`)
        g.type(" ")
        #expect(g.text == "x **bold**  y", "a space at the end goes after the closer")
        #expect(g.tv.selectedRange().location == 11, "the caret is after the space, outside the bold")
        let h = fixture("**a b**")
        h.caret(0); h.caret(3)                                           // after "a"
        h.tv.deleteBackward(nil)
        #expect(h.text == " **b**", "⌫ that leaves whitespace at the edge moves it out")
        // A LETTER at the edge still joins the span ([T-0589] rules 2–3).
        let k = fixture("x **bold** y")
        k.caret(0); k.caret(8); k.type("s")
        #expect(k.text == "x **bolds** y")
        k.caret(0); k.caret(4); k.type("A")
        #expect(k.text == "x **Abolds** y")
    }

    @Test("I-0279: a held cross-scene fragment goes stale the moment anything else is copied")
    func structuredClipboardGoesStale() throws {
        let json = #"{"schema":"scrivi.fragment.v1","pieces":[{"opensWith":"none","text":"**w** a"},{"opensWith":"scene","text":"b"}],"plainText":"w a\n\nb"}"#
        let frag = try JSONDecoder().decode(FragmentResult.self, from: Data(json.utf8))
        var clip = StructuredClipboard()
        clip.hold(frag, changeCount: 41)
        #expect(clip.fragment(currentChangeCount: 41) != nil, "still the pasteboard's contents: paste it")
        #expect(clip.fragment(currentChangeCount: 42) == nil, "something else was copied: the normal paste runs")
    }

    @Test("live pass: the Format menu sits BETWEEN Edit and View")
    func formatMenuPlacement() throws {
        let titles = try #require(NSApp.mainMenu).items.map(\.title)
        let edit = try #require(titles.firstIndex(of: "Edit")), format = try #require(titles.firstIndex(of: "Format"))
        let view = try #require(titles.firstIndex(of: "View"))
        #expect(edit < format && format < view, "menus: \(titles)")
    }

    @Test("AC6 + Q3: random selections over escaped prose — the command always lands, and only on the selection")
    func commandCorpus() {
        let base = [
            #"d\'Artagnan arrives in Paris with a yellow horse and no money\; Edmond Dantes arrives in Marseille\."#,
            #"Both are provincials\, walking into a system that has \"already decided\" what they are worth\."#,
            #"He wrote at speed\, for serial publication \(and was paid by the line\)\; the pattern is real\."#,
        ]
        var rng = SystemRandomNumberGenerator()
        var refused = 0, leaked = 0, missed = 0
        for _ in 0..<300 {
            let src = base.randomElement(using: &rng)!
            let f = fixture(src)
            let n = (src as NSString).length
            let a = Int.random(in: 0..<(n - 1), using: &rng)
            let b = min(n, a + Int.random(in: 1...30, using: &rng))
            f.tv.setSelectedRange(NSRange(location: a, length: b - a))
            let sel = f.tv.selectedRange()                                // after the snap
            let before = f.text
            // A selection with no LETTER or DIGIT in it (a lone space, or punctuation glued to a letter, which Q3 shrinks
            // away) has nothing that can carry the format — no change is right there.
            if !(before as NSString).substring(with: sel).utf16.contains(where: ManuscriptPresenter.isWordUnit) { continue }
            f.tv.applyFormat(.bold)
            if f.text == before { refused += 1; continue }
            // Read the result back: every visible LETTER of the selection is bold, nothing outside it is.
            let a2 = MarkdownBlocks.analyze(f.text)
            let toks = MarkdownEmphasis.tokens(of: f.text)
            let selectedText = MarkdownEmphasis.tokens(of: (before as NSString).substring(with: sel))
            _ = a2
            let boldLetters = toks.filter { $0.style & MarkdownBlocks.bold != 0 && ManuscriptPresenter.isWordUnit($0.unit) }.count
            let wanted = selectedText.filter { ManuscriptPresenter.isWordUnit($0.unit) }.count
            if boldLetters > wanted { leaked += 1 }
            if boldLetters < wanted { missed += 1 }
        }
        #expect(refused == 0 && leaked == 0 && missed == 0, "refused \(refused), leaked \(leaked), missed \(missed) of 300")
    }

    @Test("T-0591: each scene part of a cross-scene cut stays balanced; a cross-scene paste into bold continues it")
    func acrossScenes() throws {
        let f = fixture("")
        let s = NSMutableAttributedString(string: "x **bold** y", attributes: [.font: ManuscriptTypography.default.bodyFont])
        s.append(NSAttributedString(string: "\u{FFFC}", attributes: [.scriviDivider: DividerRenderState.sceneBreak]))
        s.append(NSAttributedString(string: "\nnext **two** z", attributes: [.font: ManuscriptTypography.default.bodyFont]))
        f.tv.textStorage!.setAttributedString(s)
        let ts = f.tv.textStorage!
        // A selection from inside "bold" (after "bo") to inside "two" (before "o"): scene 1 part 6..<12 ("ld** y"),
        // scene 2 part 14..<23 ("next **tw").
        let first = try #require(f.presenter.balancedEdit(in: ts, replacing: NSRange(location: 6, length: 6), with: ""))
        let second = try #require(f.presenter.balancedEdit(in: ts, replacing: NSRange(location: 14, length: 9), with: ""))
        ts.replaceCharacters(in: second.range, with: second.replacement)   // back to front, as deleteAcrossScenes does
        ts.replaceCharacters(in: first.range, with: first.replacement)
        #expect(f.text == "x **bo**\u{FFFC}\n**o** z", "each scene closes / re-opens its own span")
        // The copy of a part: it carries its own markers.
        let g = fixture("x **bold** y")
        #expect(g.presenter.balancedCopy(in: g.tv.textStorage!, NSRange(location: 6, length: 6)).source == "**ld** y")
        // Paste into bold: the pasted text KEEPS ITS OWN formatting ([SP-163] live pass) — the first piece closes the
        // span the caret was in, the last re-opens it for the text after the split (`ld**`); bold stays bold.
        #expect(ManuscriptPresenter.balancePastePieces(["one", "mid", "last"], caretStyle: MarkdownBlocks.bold)
                == ["**one", "mid", "last**"])
        #expect(ManuscriptPresenter.balancePastePieces(["**w** one", "mid", "last"], caretStyle: MarkdownBlocks.bold)
                == ["w** one", "mid", "last**"], "only the word that was bold stays bold")
    }
}
/// EP-046 E2-S4 (SP-164, T-0593 + T-0585) — Find and Replace over what the writer SEES (Q-E2-5): AppKit's find bar (Q1) over a
/// presented-text client. ⚠️ `NSTextFinder` reads its query from the SYSTEM find pasteboard — the tests that drive it save and
/// restore the writer's find string.
@Suite("Find and Replace (EP-046 E2-S4)")
@MainActor
struct FindReplaceTests {

    private func fixture(_ text: NSAttributedString) -> (ManuscriptFixture, NSScrollView) {
        let f = ManuscriptFixture()
        f.tv.textStorage!.setAttributedString(text)
        let sv = NSScrollView(frame: NSRect(x: 0, y: 0, width: 500, height: 300))
        sv.documentView = f.tv
        f.window.contentView = sv
        // ⚠️ The find bar needs its window ON SCREEN and key (measured in the [SP-164] spike).
        f.window.makeKeyAndOrderFront(nil)
        f.window.makeFirstResponder(f.tv)
        _ = f.tv.textFinder                     // as in the app: the find bar exists before any Find or Replace
        return (f, sv)
    }

    private func body(_ s: String) -> NSAttributedString {
        NSAttributedString(string: s, attributes: [.font: ManuscriptTypography.default.bodyFont, .foregroundColor: NSColor.textColor])
    }

    @Test("the presented text: no escapes, markers or heading prefixes; titles + dividers IN it (EP-050 Q1/Q3) but unsearched; one chunk per scene")
    func presentedText() {
        let s = NSMutableAttributedString(string: "Chapter One\n", attributes: [.scriviHeading: true])
        s.append(body(#"Mr\. **Smith** said \*no\*\."#))
        s.append(NSAttributedString(string: "\u{FFFC}", attributes: [.scriviDivider: DividerRenderState.sceneBreak]))
        s.append(body("\n## Head\n\n- item"))
        let (f, _) = fixture(s)
        let p = PresentedText.build(f.tv.textStorage!, presenter: f.presenter)
        #expect(p.string as String == "Chapter One\nMr. Smith said *no*.Scene break\nHead\n\n- item", "presented: \((p.string as String).debugDescription)")
        #expect(p.chunks.count == 2, "a match never crosses the scene break")
        #expect(p.firstMatch(of: "Chapter") == nil, "Q4: a chapter title is on the page but never searched")
        // A match across a hidden marker maps back WITHOUT the closer that follows it.
        let smith = p.string.range(of: "Smith")
        #expect((f.text as NSString).substring(with: p.storageRange(smith)) == "Smith")
        #expect(p.firstMatch(of: "mr. smith").map { (f.text as NSString).substring(with: $0) } == #"Mr\. **Smith"#,
                "Q5: the Navigator's jump finds a query with punctuation across a hidden marker")
    }

    @Test("Q1: AppKit's find bar, through the client — Use Selection for Find, then Find Next, over what the writer SEES")
    func findBar() {
        // ⚠️ Driven with Use Selection for Find: a test-host app is not the ACTIVE app, so its find bar never loads the find
        // pasteboard on its own (measured: window not key) — the selection route needs no activation.
        let (f, _) = fixture(body(#"x Mr\. **Smith** said \*no\*\. y Mr\. Smith said \*no\*\. z"#))
        let pb = NSPasteboard(name: .find)
        let saved = pb.string(forType: .string)
        defer { pb.clearContents(); if let saved { pb.setString(saved, forType: .string) } }
        f.tv.setSelectedRange(NSRange(location: 2, length: 26))          // `Mr\. **Smith** said \*no\*` — the FIRST one
        f.tv.performFind(.useSelectionForFind)
        #expect(pb.string(forType: .string) == "Mr. Smith said *no*", "the query is what the writer SEES")
        f.tv.performFind(.nextMatch)
        RunLoop.main.run(until: Date().addingTimeInterval(0.3))
        let found = (f.text as NSString).substring(with: f.tv.selectedRange())
        #expect(found == #"Mr\. Smith said \*no\*"#, "the SECOND occurrence, selected in storage: \(found.debugDescription)")
    }

    @Test("Replace writes ESCAPED text and takes the match's first character's style (Q3); Replace All in one pass")
    func replace() {
        let (f, _) = fixture(body("a **Smith** b"))
        let client = f.tv.finderClient
        client.textView = f.tv
        let p = client.presented
        client.replaceCharacters(in: p.string.range(of: "Smith"), with: "J. Jones")
        #expect(f.text == #"a **J\. Jones** b"#, "escaped, and bold like the word it replaced")
        // Replace All, in the order AppKit drives it: should (→ true, so the find bar reports its count), one
        // replaceCharacters per range, did. ✅ Applied ONCE, in one pass, from the ranges `should` was given.
        let (g, _) = fixture(body("cat and **cat** and cat"))
        let gc = g.tv.finderClient
        gc.textView = g.tv
        let q = gc.presented
        let ranges = [0, 8, 16].map { NSValue(range: NSRange(location: $0, length: 3)) }
        #expect(q.string as String == "cat and cat and cat")
        #expect(gc.shouldReplaceCharacters(inRanges: ranges, with: ["d*g", "d*g", "d*g"]), "true: the find bar reports the count")
        for r in ranges.reversed() { gc.replaceCharacters(in: r.rangeValue, with: "d*g") }
        gc.didReplaceCharacters()
        #expect(g.text == #"d\*g and **d\*g** and d\*g"#, "escaped, and the bold one stays bold: \(g.text.debugDescription)")
        // The batch is over: a single Replace acts again.
        gc.replaceCharacters(in: gc.presented.string.range(of: "d*g"), with: "x")
        #expect(g.text.hasPrefix("x and"), "\(g.text.debugDescription)")
    }

    @Test("Replace All of 1,200 matches is one editing pass — fast ([SP-164] live pass: ~1 min through the typing path)")
    func replaceAllIsFast() {
        let para = #"He said **cat**, and Mr\. cat went on\. The cat sat; the *cat* ran past a cat.\#n\#n"#
        let (f, _) = fixture(body(String(repeating: para, count: 240)))
        let client = f.tv.finderClient
        client.textView = f.tv
        let p = client.presented
        var ranges: [NSValue] = []
        var at = 0
        while true {
            let r = p.string.range(of: "cat", range: NSRange(location: at, length: p.length - at))
            if r.location == NSNotFound { break }
            ranges.append(NSValue(range: r)); at = NSMaxRange(r)
        }
        #expect(ranges.count == 1200)
        let t0 = Date()
        _ = client.shouldReplaceCharacters(inRanges: ranges, with: Array(repeating: "dog", count: ranges.count))
        for r in ranges.reversed() { client.replaceCharacters(in: r.rangeValue, with: "dog") }
        client.didReplaceCharacters()
        let ms = Date().timeIntervalSince(t0) * 1000
        #expect(!f.text.contains("cat"))
        #expect(f.text.components(separatedBy: "**dog**").count == 241, "bold stays bold")
        #expect(f.text.components(separatedBy: "*dog*").count == 481, "italic stays italic (and bold contains *dog*)")
        #expect(ms < 2000, "Replace All took \(Int(ms)) ms")
        print("[SCRIVI-FIND-TEST] replace all 1200: \(Int(ms)) ms")
    }

    @Test("Q5: the Navigator's filter searches what the writer SEES — a period, across markers, a heading's text")
    func navigatorSearchable() {
        #expect(MarkdownEmphasis.searchable(#"Mr\. **Smith** said \*no\*\."#) == "Mr. Smith said *no*.")
        #expect(MarkdownEmphasis.searchable("## Head\n\nText") == "Head\n\nText")
        #expect(MarkdownEmphasis.searchable("- item\n- two") == "- item\n- two", "list prefixes are visible")
        #expect(MarkdownEmphasis.searchable("plain, text. here") == "plain, text. here")
        #expect(MarkdownEmphasis.searchable(#"nation\. Conceived"#).localizedStandardContains("nation. Conceived"))
    }

    @Test("Edit ▸ Find exists, with its five commands")
    func findMenu() throws {
        let edit = try #require(NSApp.mainMenu?.items.first { $0.title == "Edit" }?.submenu)
        let find = try #require(edit.items.first { $0.title == "Find" }?.submenu, "Edit ▸ Find: \(edit.items.map(\.title))")
        #expect(Set(find.items.map(\.title)).isSuperset(of: ["Find…", "Find and Replace…", "Find Next", "Find Previous", "Use Selection for Find"]))
    }
}
// MARK: - EP-048 L2 — the core analyzer agrees with Apple's

/// ✅ EP-048 L2 (SP-165, T-0594): ScriviCore's md4c analyzer (`scrivi_analyze_markdown`, what Linux renders with)
/// and Apple's `MarkdownBlocks.analyze` (`AttributedString`) must report the SAME analysis for every block — or
/// Linux and Apple show one manuscript two ways. ⚠️ Two parsers, one set of rules: this is the guard on their drift.
/// Every disagreement is LISTED, never averaged away.
@Suite("Markdown analyzer agreement: ScriviCore md4c vs MarkdownBlocks (EP-048 L2)")
struct MarkdownAnalyzerAgreementTests {
    private let engine = ScriviEngine()

    /// Blocks as the presenter cuts them: maximal runs of non-blank lines, each line keeping its newline.
    static func blocks(_ text: String) -> [String] {
        let lines = text.split(separator: "\n", omittingEmptySubsequences: false)
        var out: [String] = [], cur = ""
        for (i, l) in lines.enumerated() {
            let line = String(l) + (i < lines.count - 1 ? "\n" : "")
            if MarkdownEscapes.isBlankLine(line) { if !cur.isEmpty { out.append(cur); cur = "" } } else { cur += line }
        }
        if !cur.isEmpty { out.append(cur) }
        return out
    }

    /// The core's analysis converted to UTF-16 units, shaped as `MarkdownBlocks.Analysis`.
    private func core(_ block: String) throws -> MarkdownBlocks.Analysis {
        let r = try engine.analyzeMarkdown(block: block)
        var u16 = [Int](repeating: 0, count: r.length + 1)
        var b = 0, u = 0
        for scalar in block.unicodeScalars {
            let nb = String(scalar).utf8.count
            for k in 0..<nb { u16[b + k] = u }
            b += nb; u += scalar.utf16.count
        }
        u16[b] = u
        func ns(_ x: MarkdownAnalysisResult.ByteRange) -> NSRange {
            NSRange(location: u16[x.start], length: u16[x.end] - u16[x.start])
        }
        var a = MarkdownBlocks.Analysis()
        a.headings = (r.headings ?? []).map { .init(line: ns($0.line), prefix: ns($0.prefix), level: $0.level) }
        a.listItems = (r.listItems ?? []).map {
            .init(line: ns($0.line), prefix: ns($0.prefix), ordered: $0.ordered, number: $0.number)
        }
        a.markers = (r.markers ?? []).map { .init(range: ns($0.range), opens: $0.opens) }
        a.spans = (r.spans ?? []).map(ns)
        if let runs = r.styleRuns, !runs.isEmpty {
            a.styles = [UInt8](repeating: 0, count: u)
            // Every UTF-16 unit of a scalar takes its bytes' style (👋 is TWO units).
            for run in runs {
                for unit in u16[run.range.start]..<u16[run.range.end] { a.styles[unit] = UInt8(run.bits) }
            }
        }
        return a
    }

    /// `styles` is empty when a block has no emphasis — compare as all-zero instead.
    private static func normalized(_ a: MarkdownBlocks.Analysis, _ n: Int) -> MarkdownBlocks.Analysis {
        var a = a
        if a.styles.isEmpty { a.styles = [UInt8](repeating: 0, count: n) }
        return a
    }

    /// Runs every block of `texts` through both analyzers; returns the disagreeing blocks with both readings.
    private func disagreements(_ texts: [String]) throws -> (blocks: Int, diffs: [String]) {
        var count = 0, diffs: [String] = []
        for text in texts {
            for block in Self.blocks(text) {
                count += 1
                let n = block.utf16.count
                let apple = Self.normalized(MarkdownBlocks.analyze(block), n)
                let ours = Self.normalized(try core(block), n)
                if apple != ours { diffs.append("\(block.debugDescription)\n  apple: \(apple)\n  core:  \(ours)") }
            }
        }
        return (count, diffs)
    }

    /// The AC3 corpus (EP-045): the same seed and alphabet, so these are the same 2,000 strings.
    static func ac3Typed() -> [String] {
        var state: UInt64 = 0x5C21_0453
        func next() -> UInt64 { state = state &* 6364136223846793005 &+ 1442695040888963407; return state >> 33 }
        let alphabet: [String] = Array(##"!"#$%&'()*+,-./:;<=>?@[\]^_`{|}~"##).map(String.init)
            + ["a", "b", "Z", "é", "👋", " ", "\t", "\n"]
        return (0..<2_000).map { _ in
            let len = Int(next() % 24)
            return (0..<len).map { _ in alphabet[Int(next() % UInt64(alphabet.count))] }.joined()
        }
    }

    /// The S5 corpus (design §6.1, [SP-163]): escaped dumas prose with an arbitrary selection wrapped in `**` or `*` —
    /// RAW, without the Q3 shrink, so the flanking failures are in it too. Seeded, so a failure reproduces.
    static func s5() -> [String] {
        let base = [
            #"d\'Artagnan arrives in Paris with a yellow horse and no money\; Edmond Dantes arrives in Marseille\."#,
            #"Both are provincials\, walking into a system that has \"already decided\" what they are worth\."#,
            #"He wrote at speed\, for serial publication \(and was paid by the line\)\; the pattern is real\."#,
        ]
        var state: UInt64 = 0x0E48_05C5
        func next() -> Int { state = state &* 6364136223846793005 &+ 1442695040888963407; return Int(state >> 33) }
        var out: [String] = []
        for i in 0..<4_000 {
            let src = Array(base[next() % base.count])
            let a = next() % (src.count - 1)
            let b = min(src.count, a + 1 + next() % 30)
            let d = i % 2 == 0 ? "**" : "*"
            out.append(String(src[..<a]) + d + String(src[a..<b]) + d + String(src[b...]))
        }
        return out
    }

    /// Block shapes neither random corpus reaches: headings, lists, quotes, code, tables, the SP-165 spike.
    static let structural = [
        "# One", "## Two **bold**", "###### Six", "####### seven", "#nospace", "  ### indented", "# Heading \\#5",
        "Title\n=====", "> # quoted", "> **q**", "- # in a list", "- one\n- two", "* item", "* **x**", "+ plus",
        "1. one\n2. two", "3) paren", "2. ", "- ", "- a\n  - nested\n- b", "1. *it*\n2. **b**",
        "    indented **code**", "\tsome *it*", "```\nfenced *x*\n```", "| a | **b** |\n| - | - |\n| 1 | 2 |",
        "Mr\\. Smith\\, ok", "2 \\* 3 \\* 4", "2 * 3 * 4", "**bold** and *it* x", "line one\\\nline two",
        "a &amp; b **c**", "\\\\back", "_x_ and \\_y\\_", "*it***bold**", "**a**\n*b*", "***both***",
        "**bold *both* bold**", "**see [here](x.md) now**", "é👋 **b**", "wheth_er Dumas_ inten", "`code *x*` **y**",
        "~~strike~~ **b**", "<em>html</em> *x*", "a  \nb *c*", "**unclosed", "__dunder__ and _u_",
        // ✅ [I-0281] (SP-169): a first line indented 1–3 spaces — continuation lines now agree with md4c.
        " She said\n*no* twice.", "   Three\nlines *of* it\nand *more* here.", " x\n  *c* d", "  ab\n  *c* d",
    ]

    /// ⚠️ The disagreements MEASURED 2026-10-07 ([SP-165]), each reduced to a minimal block. ✅ The symbol and `~` classes are
    /// ACCEPTED (user, SP-165 Q4); the Apple position errors are Apple's to fix ([I-0281] → EP-047).
    /// Every one must STILL disagree — when a fix lands on either side this fails, and the entry is removed.
    static let knownDisagreements: [(block: String, why: String)] = [
        // md4c 0.5.2 = CommonMark 0.31: Unicode SYMBOLS (S*) count as punctuation for flanking; Apple's parser: P* only.
        ("👋*{*", "symbol-as-punctuation"), ("*<*👋", "symbol-as-punctuation"), ("_._👋", "symbol-as-punctuation"),
        // GFM strikethrough: a lone `~` between `*`s — Apple: no emphasis; md4c: emphasis.
        ("*~*", "tilde"),
        // Apple source positions: the leftover `*` of `**` is attributed to the inner position.
        ("**$*", "apple-position-leftover"),
    ]

    /// The typed AC3 strings whose RAW blocks disagree, every one an instance of a class above. ([I-0281]'s — a leading-space
    /// first line — AGREES since SP-169 and left this list.)
    static let ac3RawKnown: Set<String> = [
        "%/*^#/ ~*_👋(<b],`+[\t[", "/>👋*{*/?}*.Z\'?. \"(%aZ",
        "|\t;!%{***$~$$=*👋*{=>\'[}", "<;>__.a&_^>- |@+$,*<*👋", ";*`_.\'[👋_👋é\"!$( &%.^@)#",
    ]

    @Test("the AC3 corpus agrees — escaped as typed (all), and RAW (all but the six known)")
    func ac3() throws {
        let typed = Self.ac3Typed()
        let escaped = try disagreements(typed.map(MarkdownEscapes.escape))
        #expect(escaped.diffs.isEmpty, "\(escaped.diffs.count)/\(escaped.blocks) escaped blocks disagree:\n\(escaped.diffs.prefix(8).joined(separator: "\n"))")
        let raw = try disagreements(typed.filter { !Self.ac3RawKnown.contains($0) })
        #expect(raw.diffs.isEmpty, "\(raw.diffs.count)/\(raw.blocks) raw blocks disagree:\n\(raw.diffs.prefix(8).joined(separator: "\n"))")
        #expect(Self.ac3RawKnown.isSubset(of: Set(typed)), "the known strings are in the corpus")
    }

    @Test("the known disagreements still disagree (accepted, or awaiting an Apple fix)")
    func known() throws {
        for k in Self.knownDisagreements {
            #expect(try !disagreements([k.block]).diffs.isEmpty, "now AGREES — remove it: \(k.why) \(k.block.debugDescription)")
        }
    }

    @Test("the S5 corpus agrees — 4,000 arbitrary selections wrapped in ** or *")
    func s5Corpus() throws {
        let r = try disagreements(Self.s5())
        #expect(r.diffs.isEmpty, "\(r.diffs.count)/\(r.blocks) blocks disagree:\n\(r.diffs.prefix(8).joined(separator: "\n"))")
    }

    @Test("structural blocks agree")
    func structuralBlocks() throws {
        let r = try disagreements(Self.structural)
        #expect(r.diffs.isEmpty, "\(r.diffs.count)/\(r.blocks) blocks disagree:\n\(r.diffs.joined(separator: "\n"))")
    }
}

// MARK: - EP-047 S1 — project settings that TRAVEL ([I-0278], SP-167)

/// ✅ EP-047 AC1–AC3: `ProjectPreferences` reads and writes the PACKAGE (`project-settings.json` and `project.json`'s
/// title) through ScriviCore, keeps keys it did not write, and migrates this Mac's old `UserDefaults` record ONCE
/// per the Q2 ruling (package wins; a title renamed on this Mac goes to `project.json` only while the package has
/// no settings yet). Each case uses its own `UserDefaults` suite — never the writer's.
@Suite("Project settings travel with the project (EP-047 S1)")
@MainActor
struct ProjectSettingsTravelTests {

    @MainActor private struct Fixture {
        let engine = ScriviEngine()
        let root: String
        let projectID: String
        let defaults: UserDefaults
        let suite: String

        init() throws {
            let base = FileManager.default.temporaryDirectory.appendingPathComponent("scrivi-settings-\(UUID().uuidString)")
            let appSupport = base.appendingPathComponent("support").path(percentEncoded: false)
            root = base.appendingPathComponent("p.scrivi").path(percentEncoded: false)
            try FileManager.default.createDirectory(atPath: appSupport, withIntermediateDirectories: true)
            try FileManager.default.createDirectory(atPath: root, withIntermediateDirectories: true)
            let id = try engine.ensureLocalIdentity(displayName: "Test", appSupportRoot: appSupport)
            let ref = AuthorshipRef(identityID: id.identityID, personaID: id.defaultPersonaID, displayName: id.displayName)
            projectID = try engine.createProject(projectRootPath: root, appSupportRoot: appSupport,
                                                 title: "Schema Title", slug: "schema-title", authorshipRef: ref).projectID
            suite = "scrivi.tests.settings.\(UUID().uuidString)"
            defaults = UserDefaults(suiteName: suite)!
        }

        func prefs() -> ProjectPreferences {
            ProjectPreferences(projectID: projectID, projectRootPath: root, schemaTitle: schemaTitle(), engine: engine,
                               defaults: defaults)
        }
        func schemaTitle() -> String {
            let data = FileManager.default.contents(atPath: root + "/project.json") ?? Data()
            return ((try? JSONSerialization.jsonObject(with: data)) as? [String: Any])?["title"] as? String ?? ""
        }
        func settings() -> [String: Any]? {
            guard let json = try? engine.getProjectSettings(projectRootPath: root).documentJSON else { return nil }
            return (try? JSONSerialization.jsonObject(with: Data(json.utf8))) as? [String: Any]
        }
        func legacy(title: String, subtitle: String, show: Bool) {
            let data = try! JSONEncoder().encode(ProjectPreferences.LegacyStored(
                showChapterTitles: show, projectTitle: title, projectSubtitle: subtitle))
            defaults.set(data, forKey: ProjectPreferences.legacyKey(for: projectID))
        }
        var legacyPresent: Bool { defaults.data(forKey: ProjectPreferences.legacyKey(for: projectID)) != nil }
        func cleanUp() { defaults.removePersistentDomain(forName: suite) }
    }

    @Test("a fresh project: defaults, the schema title, and no settings file written on open")
    func fresh() throws {
        let f = try Fixture(); defer { f.cleanUp() }
        let p = f.prefs()
        #expect(p.projectTitle == "Schema Title" && p.projectSubtitle.isEmpty && !p.showChapterTitles)
        #expect(try f.engine.getProjectSettings(projectRootPath: f.root).status == .absent)
    }

    @Test("subtitle and Show chapter titles are written to the package and read back by a new session")
    func roundTrip() throws {
        let f = try Fixture(); defer { f.cleanUp() }
        let p = f.prefs()
        p.projectSubtitle = "A Novel"
        p.showChapterTitles = true
        let again = f.prefs()
        #expect(again.projectSubtitle == "A Novel" && again.showChapterTitles)
        #expect(f.settings()?["subtitle"] as? String == "A Novel")
    }

    @Test("a save keeps keys this build did not write (another platform's) — [I-0215]")
    func keepsUnknownKeys() throws {
        let f = try Fixture(); defer { f.cleanUp() }
        try f.engine.putProjectSettings(projectRootPath: f.root, documentJson: #"{"linuxOnly":{"x":1}}"#)
        let p = f.prefs()
        p.projectSubtitle = "Sub"
        #expect((f.settings()?["linuxOnly"] as? [String: Any])?["x"] as? Int == 1)
    }

    @Test("a rename is written to project.json; an empty title is not")
    func title() throws {
        let f = try Fixture(); defer { f.cleanUp() }
        let p = f.prefs()
        p.projectTitle = "Renamed"
        #expect(f.schemaTitle() == "Renamed")
        p.projectTitle = "   "
        #expect(f.schemaTitle() == "Renamed")
    }

    @Test("Q2 migration, package has no settings: this Mac's values move in, its rename goes to project.json, key deleted")
    func migrateIntoEmptyPackage() throws {
        let f = try Fixture(); defer { f.cleanUp() }
        f.legacy(title: "Mac Title", subtitle: "Mac Sub", show: true)
        let p = f.prefs()
        #expect(p.projectTitle == "Mac Title" && p.projectSubtitle == "Mac Sub" && p.showChapterTitles)
        #expect(f.schemaTitle() == "Mac Title")
        #expect(f.settings()?["subtitle"] as? String == "Mac Sub")
        #expect(!f.legacyPresent)
    }

    @Test("Q2 migration, package already has settings: the PACKAGE wins and the title is untouched; key deleted")
    func packageWins() throws {
        let f = try Fixture(); defer { f.cleanUp() }
        try f.engine.putProjectSettings(projectRootPath: f.root, documentJson: #"{"subtitle":"Package Sub","showChapterTitles":false}"#)
        f.legacy(title: "Mac Title", subtitle: "Mac Sub", show: true)
        let p = f.prefs()
        #expect(p.projectSubtitle == "Package Sub" && !p.showChapterTitles)
        #expect(p.projectTitle == "Schema Title" && f.schemaTitle() == "Schema Title")
        #expect(!f.legacyPresent)
    }

    @Test("an UNREADABLE settings file is neither migrated over nor overwritten on open; the key is kept")
    func unreadable() throws {
        let f = try Fixture(); defer { f.cleanUp() }
        try "{ damaged".write(toFile: f.root + "/project-settings.json", atomically: true, encoding: .utf8)
        f.legacy(title: "Mac Title", subtitle: "Mac Sub", show: true)
        let p = f.prefs()
        #expect(p.unreadableMessage != nil)
        #expect(try String(contentsOfFile: f.root + "/project-settings.json", encoding: .utf8) == "{ damaged")
        #expect(f.legacyPresent)
        #expect(f.schemaTitle() == "Schema Title")
    }
}

// MARK: - EP-047 S2 — the manuscript's type: one source, bundled faces (SP-168)

/// ✅ EP-047 AC4 + AC5: every path that WRITES manuscript text writes the project's typography (face, size, the 1.45 line), and
/// the faces are the BUNDLED ones. Each test uses a NON-default typography, so passing cannot mean "it happened to be the default".
@Suite("Manuscript typography — one source, bundled faces (EP-047 S2)")
@MainActor
struct ManuscriptTypographyTests {

    /// Static files (Courier Prime) and a variable face (Figtree), each at a non-default size.
    static let courier = ManuscriptTypography(faceName: "Courier Prime", size: 20)
    static let figtree = ManuscriptTypography(faceName: "Figtree", size: 14)

    /// Every storage run in `range` is body text in `t`: the face, the size, and the fixed 1.45 line.
    private func expectBody(_ ts: NSTextStorage, _ t: ManuscriptTypography, _ range: NSRange? = nil, _ what: String) {
        let r = range ?? NSRange(location: 0, length: ts.length)
        ts.enumerateAttributes(in: r) { a, run, _ in
            let font = a[.font] as? NSFont
            let ps = a[.paragraphStyle] as? NSParagraphStyle
            #expect(font?.familyName == t.face?.name, "\(what): face at \(run) — \(font?.familyName ?? "none")")
            #expect(font?.pointSize == t.size, "\(what): size at \(run) — \(font?.pointSize ?? 0)")
            #expect(ps?.minimumLineHeight == t.size * ManuscriptTypography.lineSpacing, "\(what): line height at \(run)")
        }
    }

    private func fixture(_ t: ManuscriptTypography, _ text: String = "") -> ManuscriptFixture {
        let f = ManuscriptFixture()
        f.presenter.typography = t                    // as `makeNSView` / `updateNSView` do
        f.tv.typingAttributes = t.bodyAttributes
        if !text.isEmpty {
            f.tv.textStorage?.replaceCharacters(in: NSRange(location: 0, length: 0),
                                                with: NSAttributedString(string: text, attributes: t.bodyAttributes))
        }
        return f
    }

    @Test("the bundled faces load from Fonts/fonts.json: seven, default Literata, every file present, each drawn in its family")
    func bundledFaces() throws {
        let faces = BundledFonts.faces
        #expect(faces.count == 7, "\(faces.map(\.name))")
        #expect(BundledFonts.manifest.default == "Literata")
        for face in faces {
            for file in face.files {
                let url = try #require(BundledFonts.url(of: file, in: face))
                #expect(FileManager.default.fileExists(atPath: url.path), "\(face.name): \(file.file) is in the bundle")
            }
            let t = ManuscriptTypography(faceName: face.name, size: 16)
            #expect(t.bodyFont.familyName == face.name, "\(face.name) draws in its own family, not a fallback")
            // ✅ Italic is the face's ITALIC FILE; bold is a real heavier weight.
            #expect(t.font(level: nil, style: MarkdownBlocks.italic).fontDescriptor.symbolicTraits.contains(.italic), "\(face.name) italic")
            let w = { (f: NSFont) in (f.fontDescriptor.object(forKey: .traits) as? [NSFontDescriptor.TraitKey: Any])?[.weight] as? Double ?? 0 }
            #expect(w(t.font(level: nil, style: MarkdownBlocks.bold)) > w(t.bodyFont), "\(face.name) bold is heavier")
        }
        // The licenses ship with the fonts (OFL §2).
        for face in faces {
            let dir = try #require(BundledFonts.directory).appendingPathComponent(face.folder)
            #expect(FileManager.default.fileExists(atPath: dir.appendingPathComponent("OFL.txt").path), "\(face.name) OFL.txt")
        }
    }

    @Test("default: Literata at 16 pt on a 1.45 line; headings keep Apple's ratios; chapter titles are Heading 1 in the text colour")
    func defaults() {
        let t = ManuscriptTypography.default
        #expect(t.faceName == "Literata" && t.size == 16)
        #expect(t.bodyFont.familyName == "Literata")
        #expect(abs(t.headingSize(level: 1) - 16 * 22 / 13) < 0.001 && abs(t.headingSize(level: 2) - 16 * 18 / 13) < 0.001)
        #expect(t.chapterTitleFont.pointSize == t.headingSize(level: 1))
        #expect(t.chapterTitleAttributes[.foregroundColor] as? NSColor == NSColor.textColor)
        #expect((t.paragraphStyle().minimumLineHeight) == 16 * 1.45)
    }

    @Test("a face this build does not bundle: the name is KEPT, the drawing falls back to Literata")
    func unknownFaceFallsBack() {
        let t = ManuscriptTypography(faceName: "Garamond From The Future", size: 16)
        #expect(t.faceName == "Garamond From The Future")
        #expect(t.bodyFont.familyName == "Literata")
    }

    @Test("typing and Return write the project's type (AC4)")
    func typingAndReturn() {
        let f = fixture(Self.courier)
        f.type("Typed words.")
        f.tv.insertNewline(nil)
        f.type("More.")
        expectBody(f.tv.textStorage!, Self.courier, nil, "typing + Return")
    }

    @Test("paste writes the project's type (AC4)")
    func paste() {
        let f = fixture(Self.figtree, "start ")
        let pb = NSPasteboard(name: .init("scrivi.test.\(UUID().uuidString)"))
        pb.clearContents()
        pb.setString("pasted text", forType: .string)
        f.caret(6)
        _ = f.tv.readSelection(from: pb)
        #expect(f.text.contains("pasted text"))
        expectBody(f.tv.textStorage!, Self.figtree, nil, "paste")
    }

    @Test("Replace All writes the project's type (AC4)")
    func replaceAll() {
        let f = fixture(Self.courier, "one two one two")
        f.tv.applyReplacements([(NSRange(location: 0, length: 3), "uno"), (NSRange(location: 8, length: 3), "uno")])
        #expect(f.text == "uno two uno two")
        expectBody(f.tv.textStorage!, Self.courier, nil, "Replace All")
    }

    @Test("the REBUILD (open, a typeface change) writes the project's type; chapter titles in the face at H1; scene text unchanged")
    func rebuild() throws {
        let info = try JSONDecoder().decode(SceneInfo.self, from: Data(#"""
            {"sceneID":"s1","chapterID":"c1","title":"S","chapterTitle":"The Harbour","slug":"s",
             "metadataPath":"","contentPath":"","chapterMetadataPath":""}
            """#.utf8))
        let loader = ViewportSceneLoader(engine: ScriviEngine(), projectRootPath: "/tmp/typography-test",
                                         appSupportRoot: "/tmp/typography-test-support", projectID: "p", allScenes: [info])
        let session = ProjectSession(engine: ScriviEngine(), authorshipRef: nil, appSupportRoot: "/tmp/typography-test-support", identityID: "")
        let t = Self.courier
        let view = ManuscriptTextView(loader: loader, env: AppEnvironment(), session: session, navigateToSceneID: .constant(nil),
                                      showChapterTitles: true, typography: t)
        let c = view.makeCoordinator()
        let tv = ManuscriptNSTextView(usingTextLayoutManager: true)
        tv.frame = NSRect(x: 0, y: 0, width: 500, height: 200)
        let window = NSWindow(contentRect: tv.frame, styleMask: [.titled], backing: .buffered, defer: false)
        window.contentView = tv
        tv.textContentStorage?.delegate = c.presenter
        tv.textStorage?.delegate = c.presenter
        c.textView = tv
        c.presenter.typography = t
        let body = "The ship came in on the evening tide."
        c.rebuildStorage(tv, segments: [SceneSegment(id: "s1", sceneID: "s1", chapterID: "c1", metadataPath: "", contentPath: "", text: body)])
        let ts = try #require(tv.textStorage)
        let titleRange = (ts.string as NSString).range(of: "The Harbour")
        #expect(titleRange.location != NSNotFound, "the chapter title is drawn: \(ts.string.debugDescription)")
        let titleFont = ts.attribute(.font, at: titleRange.location, effectiveRange: nil) as? NSFont
        #expect(titleFont?.familyName == "Courier Prime" && titleFont?.pointSize == t.headingSize(level: 1), "chapter title: H1 in the face")
        #expect(ts.attribute(.foregroundColor, at: titleRange.location, effectiveRange: nil) as? NSColor == NSColor.textColor)
        let bodyRange = (ts.string as NSString).range(of: body)
        #expect(bodyRange.location != NSNotFound, "scene text unchanged by the rebuild")
        expectBody(ts, t, bodyRange, "rebuild")
    }

    /// A real coordinator + text view inside a SCROLL VIEW and a window — what `rebuildKeepingReadingPosition` needs.
    /// `layoutAll: false` is what the APP does — TextKit 2 lays out only what is near the viewport, and ESTIMATES the rest.
    private func scrolledHarness(_ t: ManuscriptTypography, body: String, layoutAll: Bool = true) throws -> (ManuscriptTextView.Coordinator, ManuscriptNSTextView, NSScrollView, NSWindow) {
        let info = try JSONDecoder().decode(SceneInfo.self, from: Data(#"""
            {"sceneID":"s1","chapterID":"c1","title":"S","chapterTitle":"The Harbour","slug":"s",
             "metadataPath":"","contentPath":"","chapterMetadataPath":""}
            """#.utf8))
        let loader = ViewportSceneLoader(engine: ScriviEngine(), projectRootPath: "/tmp/typography-test",
                                         appSupportRoot: "/tmp/typography-test-support", projectID: "p", allScenes: [info])
        let session = ProjectSession(engine: ScriviEngine(), authorshipRef: nil, appSupportRoot: "/tmp/typography-test-support", identityID: "")
        let view = ManuscriptTextView(loader: loader, env: AppEnvironment(), session: session, navigateToSceneID: .constant(nil),
                                      showChapterTitles: true, typography: t)
        let c = view.makeCoordinator()
        let scroll = NSScrollView(frame: NSRect(x: 0, y: 0, width: 600, height: 400))
        let tv = ManuscriptNSTextView(usingTextLayoutManager: true)
        tv.frame = NSRect(x: 0, y: 0, width: 600, height: 400)
        tv.isVerticallyResizable = true
        tv.autoresizingMask = [.width]
        tv.textContainer?.widthTracksTextView = true
        scroll.documentView = tv
        scroll.hasVerticalScroller = true
        let window = NSWindow(contentRect: scroll.frame, styleMask: [.titled], backing: .buffered, defer: false)
        window.contentView = scroll
        tv.textContentStorage?.delegate = c.presenter
        tv.textStorage?.delegate = c.presenter
        c.textView = tv
        c.presenter.typography = t
        c.rebuildStorage(tv, segments: [SceneSegment(id: "s1", sceneID: "s1", chapterID: "c1", metadataPath: "", contentPath: "", text: body)])
        if layoutAll { tv.textLayoutManager?.ensureLayout(for: tv.textLayoutManager!.documentRange) }
        return (c, tv, scroll, window)
    }

    private static let longBody = (0..<120).map { "Paragraph \($0): the ship came in on the evening tide, her sails the colour of old parchment against a sky already turning to brass." }
        .joined(separator: "\n\n")

    @Test("[I-0282] a face or size change keeps the CARET's line where it was on screen, and the selection")
    func keepsCaretPosition() throws {
        let (c, tv, scroll, window) = try scrolledHarness(.default, body: Self.longBody)
        _ = window
        let caret = (tv.string as NSString).range(of: "Paragraph 60:").location
        tv.setSelectedRange(NSRange(location: caret, length: 0))
        tv.scrollRangeToVisible(NSRange(location: caret, length: 0))
        tv.textLayoutManager?.textViewportLayoutController.layoutViewport()
        let before = try #require(c.readingAnchor(tv))
        #expect(before.index == caret, "the caret is on screen, so it is the anchor")
        for t in [ManuscriptTypography(faceName: "Courier Prime", size: 24), ManuscriptTypography(faceName: "Inter", size: 11)] {
            c.applyTypography(t, to: tv, segments: [SceneSegment(id: "s1", sceneID: "s1", chapterID: "c1", metadataPath: "", contentPath: "", text: Self.longBody)])   // the app's path (updateNSView)
            #expect(tv.selectedRange() == NSRange(location: caret, length: 0), "\(t.faceName): the selection is kept")
            let line = try #require(c.lineRect(forCharacterIndex: caret, in: tv))
            let offset = line.minY - scroll.contentView.bounds.minY
            #expect(abs(offset - before.offset) < 2, "\(t.faceName) \(t.size): the caret's line stays at \(before.offset) (now \(offset))")
        }
    }

    @Test("[I-0282] scrolled away from the caret: the TOP of the visible text stays at the top")
    func keepsTopOfView() throws {
        let (c, tv, scroll, window) = try scrolledHarness(.default, body: Self.longBody)
        _ = window
        tv.setSelectedRange(NSRange(location: 0, length: 0))      // caret at the start, off screen below
        let target = (tv.string as NSString).range(of: "Paragraph 70:").location
        // Scroll there first: `lineRect` measures only what the viewport has DRAWN ([I-0282] re-check 2).
        tv.scrollRangeToVisible(NSRange(location: target, length: 0))
        tv.textLayoutManager?.textViewportLayoutController.layoutViewport()
        tv.textLayoutManager?.textViewportLayoutController.layoutViewport()
        let r = try #require(c.lineRect(forCharacterIndex: target, in: tv))
        scroll.contentView.scroll(to: NSPoint(x: 0, y: r.minY))
        scroll.reflectScrolledClipView(scroll.contentView)
        tv.textLayoutManager?.textViewportLayoutController.layoutViewport()
        let before = try #require(c.readingAnchor(tv))
        #expect(before.index != 0, "not the caret: the top of the view")
        c.applyTypography(ManuscriptTypography(faceName: "Courier Prime", size: 24), to: tv, segments: [SceneSegment(id: "s1", sceneID: "s1", chapterID: "c1", metadataPath: "", contentPath: "", text: Self.longBody)])   // the app's path (updateNSView)
        let line = try #require(c.lineRect(forCharacterIndex: before.index, in: tv))
        #expect(abs((line.minY - scroll.contentView.bounds.minY) - before.offset) < 2, "the same text is at the top of the view")
        #expect(tv.selectedRange().location == 0, "the caret did not move")
    }

    /// The user's [I-0282] re-check (2026-10-07): Literata 20 → Courier Prime 16 with the caret deep in a REAL-SIZED manuscript
    /// (Chapter 51 of dumas) moved the view to Chapter 25. 120 paragraphs never showed it: on a long document TextKit 2 holds
    /// ESTIMATED heights for everything above the viewport.
    private static let hugeBody = (0..<6000).map { "Paragraph \($0): the ship came in on the evening tide, her sails the colour of old parchment against a sky already turning to brass, and the boy on the quay watched." }
        .joined(separator: "\n\n")

    @Test("[I-0282] at manuscript scale (1.8 MB): the caret's text stays on screen at the same height across a face + size change")
    func keepsCaretPositionAtScale() throws {
        let (c, tv, scroll, window) = try scrolledHarness(ManuscriptTypography(faceName: "Literata", size: 20), body: Self.hugeBody, layoutAll: false)
        _ = window
        let seg = [SceneSegment(id: "s1", sceneID: "s1", chapterID: "c1", metadataPath: "", contentPath: "", text: Self.hugeBody)]
        let caret = (tv.string as NSString).range(of: "Paragraph 5100:").location
        tv.setSelectedRange(NSRange(location: caret, length: 0))
        tv.scrollRangeToVisible(NSRange(location: caret, length: 0))
        tv.textLayoutManager?.textViewportLayoutController.layoutViewport()
        let before = try #require(c.readingAnchor(tv))
        #expect(before.index == caret)
        c.applyTypography(ManuscriptTypography(faceName: "Courier Prime", size: 16), to: tv, segments: seg)   // the app's path (updateNSView)
        // What the WRITER sees: the character at the top of the visible area must be near the caret's paragraph,
        // and the caret's line on screen at the same height.
        let clip = scroll.contentView
        let topIndex = tv.characterIndexForInsertion(at: NSPoint(x: tv.textContainerInset.width + 1, y: clip.bounds.minY + 1))
        let topText = (tv.string as NSString).substring(with: NSRange(location: topIndex, length: min(30, (tv.string as NSString).length - topIndex)))
        #expect(abs(topIndex - caret) < 3_000, "the view shows the caret's neighbourhood, not elsewhere — top of view: \(topText.debugDescription)")
        let line = try #require(c.lineRect(forCharacterIndex: caret, in: tv))
        #expect(abs((line.minY - clip.bounds.minY) - before.offset) < 2, "the caret's line at \(before.offset) (now \(line.minY - clip.bounds.minY))")
        #expect(tv.selectedRange().location == caret)
    }

    @Test("[I-0282] a dumas-shaped manuscript (1,200 scenes, 60 chapters, dividers + titles): face then size change keeps the place")
    func keepsPlaceInScenedManuscript() throws {
        // 60 chapters × 20 scenes; each scene a few paragraphs — the shape of dumas-prose.
        var infos: [SceneInfo] = []
        var segs: [SceneSegment] = []
        for ch in 0..<60 {
            for sc in 0..<20 {
                let id = "s\(ch)_\(sc)"
                infos.append(try JSONDecoder().decode(SceneInfo.self, from: Data("""
                    {"sceneID":"\(id)","chapterID":"c\(ch)","title":"S","chapterTitle":"Chapter \(ch + 1)","slug":"s",
                     "metadataPath":"","contentPath":"","chapterMetadataPath":""}
                    """.utf8)))
                let text = (0..<4).map { "Ch \(ch + 1) sc \(sc + 1) p \($0): the ship came in on the evening tide, her sails the colour of old parchment against a sky already turning to brass." }.joined(separator: "\n\n")
                segs.append(SceneSegment(id: id, sceneID: id, chapterID: "c\(ch)", metadataPath: "", contentPath: "", text: text))
            }
        }
        let loader = ViewportSceneLoader(engine: ScriviEngine(), projectRootPath: "/tmp/typography-test",
                                         appSupportRoot: "/tmp/typography-test-support", projectID: "p", allScenes: infos)
        let session = ProjectSession(engine: ScriviEngine(), authorshipRef: nil, appSupportRoot: "/tmp/typography-test-support", identityID: "")
        let start = ManuscriptTypography(faceName: "Literata", size: 20)
        let view = ManuscriptTextView(loader: loader, env: AppEnvironment(), session: session, navigateToSceneID: .constant(nil),
                                      showChapterTitles: true, typography: start)
        let c = view.makeCoordinator()
        let scroll = NSScrollView(frame: NSRect(x: 0, y: 0, width: 900, height: 700))
        let tv = ManuscriptNSTextView(usingTextLayoutManager: true)
        tv.frame = NSRect(x: 0, y: 0, width: 900, height: 700)
        tv.isVerticallyResizable = true
        tv.autoresizingMask = [.width]
        tv.textContainer?.widthTracksTextView = true
        tv.textContainerInset = NSSize(width: 60, height: 40)          // the app's
        scroll.documentView = tv
        scroll.hasVerticalScroller = true
        let window = NSWindow(contentRect: scroll.frame, styleMask: [.titled], backing: .buffered, defer: false)
        window.contentView = scroll
        _ = window
        tv.textContentStorage?.delegate = c.presenter
        tv.textStorage?.delegate = c.presenter
        c.textView = tv
        c.presenter.typography = start
        c.rebuildStorage(tv, segments: segs)
        // The caret on the BLANK line between two paragraphs of Chapter 51, scene 8 — as the writer had it.
        let ns = tv.string as NSString
        let caret = ns.range(of: "Ch 51 sc 8 p 1:").location - 1
        tv.setSelectedRange(NSRange(location: caret, length: 0))
        tv.scrollRangeToVisible(NSRange(location: caret, length: 0))
        tv.textLayoutManager?.textViewportLayoutController.layoutViewport()
        let before = try #require(c.readingAnchor(tv))
        for t in [ManuscriptTypography(faceName: "Courier Prime", size: 20), ManuscriptTypography(faceName: "Courier Prime", size: 16)] {
            c.applyTypography(t, to: tv, segments: segs)   // the app's path (updateNSView)
            let clip = scroll.contentView
            let top = tv.characterIndexForInsertion(at: NSPoint(x: tv.textContainerInset.width + 1, y: clip.bounds.minY + 1))
            let topText = (tv.string as NSString).substring(with: NSRange(location: top, length: 24))
            #expect(abs(top - caret) < 4_000, "\(t.faceName) \(t.size): the view shows Chapter 51 — top of view: \(topText.debugDescription)")
            let line = try #require(c.lineRect(forCharacterIndex: caret, in: tv))
            #expect(abs((line.minY - clip.bounds.minY) - before.offset) < 2, "\(t.size): caret line at \(before.offset) (now \(line.minY - clip.bounds.minY))")
            #expect(tv.selectedRange().location == caret)
        }
    }

    @Test("[I-0282] after reading around a long manuscript, the caret in mid-view is the anchor, measured from what is DRAWN")
    func anchorsOnDrawnCaretAfterScrolling() throws {
        let (c, tv, scroll, window) = try scrolledHarness(ManuscriptTypography(faceName: "Literata", size: 20), body: Self.hugeBody, layoutAll: false)
        _ = window
        let seg = [SceneSegment(id: "s1", sceneID: "s1", chapterID: "c1", metadataPath: "", contentPath: "", text: Self.hugeBody)]
        let ns = tv.string as NSString
        // Read around: jump to several places, as a writer does over a session — TextKit 2's estimates drift from the truth.
        for p in ["Paragraph 300:", "Paragraph 4000:", "Paragraph 1200:", "Paragraph 5800:", "Paragraph 2500:"] {
            tv.scrollRangeToVisible(NSRange(location: ns.range(of: p).location, length: 0))
            tv.textLayoutManager?.textViewportLayoutController.layoutViewport()
        }
        // The caret near the END, centred in the view (the user's position: Chapter 51 of ~60).
        let caret = ns.range(of: "Paragraph 5500:").location - 1
        tv.setSelectedRange(NSRange(location: caret, length: 0))
        tv.scrollRangeToVisible(NSRange(location: caret, length: 0))
        tv.textLayoutManager?.textViewportLayoutController.layoutViewport()
        let clip = scroll.contentView
        if let r = c.lineRect(forCharacterIndex: caret, in: tv) {             // centre it
            clip.scroll(to: NSPoint(x: 0, y: max(0, r.minY - clip.bounds.height / 2)))
            scroll.reflectScrolledClipView(clip)
            tv.textLayoutManager?.textViewportLayoutController.layoutViewport()
        }
        let before = try #require(c.readingAnchor(tv))
        #expect(before.index == caret, "the caret is on screen, so IT is the anchor (was: the top of the view, Chapter 25)")
        c.applyTypography(ManuscriptTypography(faceName: "Courier Prime", size: 16), to: tv, segments: seg)   // the app's path (updateNSView)
        let line = try #require(c.lineRect(forCharacterIndex: caret, in: tv), "the caret is drawn after the change")
        #expect(abs((line.minY - clip.bounds.minY) - before.offset) < 2, "caret line at \(before.offset) (now \(line.minY - clip.bounds.minY))")
        let top = try #require(c.firstVisibleLine(in: tv))
        #expect(abs(top.index - caret) < 4_000, "the view shows the caret's neighbourhood (top index \(top.index), caret \(caret))")
    }

    @Test("EP-047 S3: the indent survives the REBUILD (applyTypography) — the presenter draws it; storage carries none")
    func indentSurvivesRebuild() throws {
        let body = "First paragraph.\n\nSecond paragraph.\n\nThird paragraph."
        let (c, tv, _, window) = try scrolledHarness(ManuscriptTypography(faceName: "Literata", size: 16, indent: .none), body: body)
        _ = window
        let seg = [SceneSegment(id: "s1", sceneID: "s1", chapterID: "c1", metadataPath: "", contentPath: "", text: body)]
        let every = ManuscriptTypography(faceName: "Literata", size: 16, indent: .every, indentEm: 2)
        c.applyTypography(every, to: tv, segments: seg)
        let ns = tv.string as NSString
        let cs = try #require(tv.textContentStorage)
        for word in ["First", "Second", "Third"] {
            let r = ns.paragraphRange(for: NSRange(location: ns.range(of: word).location, length: 0))
            let p = try #require(c.presenter.textContentStorage(cs, textParagraphWith: r), "\(word) is presented")
            let ps = p.attributedString.attribute(.paragraphStyle, at: 0, effectiveRange: nil) as? NSParagraphStyle
            #expect(ps?.firstLineHeadIndent == every.firstLineIndent, "\(word): 2 em after the rebuild")
            let stored = tv.textStorage!.attribute(.paragraphStyle, at: r.location, effectiveRange: nil) as? NSParagraphStyle
            #expect((stored?.firstLineHeadIndent ?? 0) == 0, "\(word): storage carries NO indent (presentation only)")
        }
    }

    @Test("settings: face and size round-trip; ABSENT means the default and nothing is written until the writer chooses")
    func settings() throws {
        let base = FileManager.default.temporaryDirectory.appendingPathComponent("scrivi-type-\(UUID().uuidString)")
        let support = base.appendingPathComponent("s").path(percentEncoded: false)
        let root = base.appendingPathComponent("p.scrivi").path(percentEncoded: false)
        try FileManager.default.createDirectory(atPath: support, withIntermediateDirectories: true)
        try FileManager.default.createDirectory(atPath: root, withIntermediateDirectories: true)
        defer { try? FileManager.default.removeItem(at: base) }
        let engine = ScriviEngine()
        let id = try engine.ensureLocalIdentity(displayName: "T", appSupportRoot: support)
        let pid = try engine.createProject(projectRootPath: root, appSupportRoot: support, title: "T", slug: "t",
            authorshipRef: AuthorshipRef(identityID: id.identityID, personaID: id.defaultPersonaID, displayName: "T")).projectID
        let defaults = UserDefaults(suiteName: "scrivi.tests.type.\(UUID().uuidString)")!
        func prefs() -> ProjectPreferences {
            ProjectPreferences(projectID: pid, projectRootPath: root, schemaTitle: "T", engine: engine, defaults: defaults)
        }
        func file() -> [String: Any] {
            let json = (try? engine.getProjectSettings(projectRootPath: root).documentJSON) ?? nil
            return json.flatMap { (try? JSONSerialization.jsonObject(with: Data($0.utf8))) as? [String: Any] } ?? [:]
        }
        let p = prefs()
        #expect(p.typography == .default)
        p.projectSubtitle = "Sub"                              // ANOTHER setting saves …
        #expect(file()["typeface"] == nil && file()["textSize"] == nil, "… without pinning today's default into the project")
        p.typeface = "Newsreader"
        p.textSize = 18
        let again = prefs()
        #expect(again.typeface == "Newsreader" && again.textSize == 18)
        #expect(again.typography == ManuscriptTypography(faceName: "Newsreader", size: 18))
        #expect(file()["typeface"] as? String == "Newsreader")
    }
}

// MARK: - EP-047 S2 — cost of the new type (SP-168 plan step 7)

/// ⚠️ A MEASUREMENT, not a gate: the relative cost of laying out and typing in the bundled faces against the OLD storage font
/// (13 pt monospaced system, reconstructed here — the app no longer builds it). Same harness, same text, so only the face differs.
/// Run on demand: `TEST_RUNNER_SCRIVI_PERF=1 scripts/run-interop-tests.sh -only-testing:…/TypographyCostTests/cost()`.
@Suite("Typography cost (EP-047 S2, on demand)")
@MainActor
struct TypographyCostTests {
    @Test("1.8 MB: first layout, a keystroke and an arrow near the end — old monospace vs bundled faces",
          .enabled(if: ProcessInfo.processInfo.environment["SCRIVI_PERF"] != nil))
    func cost() throws {
        var paras: [String] = []
        var n = 0
        var total = 0
        while total < 1_800_000 {
            let p = "\(n) " + #"He wrote at speed\, for *serial* publication \(and was **paid** by the line\)\; the pattern is real\. D\'Artagnan arrives in Paris with a yellow horse and no money\."#
            paras.append(p); total += p.utf16.count + 2; n += 1
        }
        let text = paras.joined(separator: "\n\n")
        let old: [NSAttributedString.Key: Any] = [.font: NSFont.monospacedSystemFont(ofSize: 13, weight: .regular),   // type-source-ok
                                                  .foregroundColor: NSColor.textColor]
        let cases: [(String, [NSAttributedString.Key: Any], ManuscriptTypography)] = [
            ("old 13 pt monospaced (system)", old, .default),
            ("Literata 16 pt, indent NONE", ManuscriptTypography.default.bodyAttributes,
             ManuscriptTypography(faceName: "Literata", size: 16, indent: .none)),
            ("Literata 16 pt, Book indent (default)", ManuscriptTypography.default.bodyAttributes, .default),
            ("Courier Prime 16 pt", ManuscriptTypography(faceName: "Courier Prime", size: 16).bodyAttributes,
             ManuscriptTypography(faceName: "Courier Prime", size: 16)),
            ("Inter 16 pt", ManuscriptTypography(faceName: "Inter", size: 16).bodyAttributes, ManuscriptTypography(faceName: "Inter", size: 16)),
        ]
        func ms(_ start: Date) -> Double { Date().timeIntervalSince(start) * 1000 }
        for (name, attrs, t) in cases {
            let f = ManuscriptFixture()
            f.presenter.typography = t
            f.tv.frame = NSRect(x: 0, y: 0, width: 900, height: 800)
            f.window.setContentSize(f.tv.frame.size)
            f.tv.typingAttributes = attrs
            let t0 = Date()
            f.tv.textStorage!.setAttributedString(NSAttributedString(string: text, attributes: attrs))
            let end = (text as NSString).length - 5
            f.caret(end)
            f.tv.scrollRangeToVisible(NSRange(location: end, length: 0))
            f.tv.textLayoutManager?.textViewportLayoutController.layoutViewport()
            let open = ms(t0)
            var keys: [Double] = [], arrows: [Double] = []
            for _ in 0..<7 {
                let k = Date(); f.type("a"); f.tv.textLayoutManager?.textViewportLayoutController.layoutViewport(); keys.append(ms(k))
                let a = Date(); f.tv.moveLeft(nil); f.tv.textLayoutManager?.textViewportLayoutController.layoutViewport(); arrows.append(ms(a))
            }
            let med = { (v: [Double]) in v.sorted()[v.count / 2] }
            print(String(format: "[TYPE-COST] %-32@ open+viewport %7.1f ms · keystroke near end %6.1f ms · arrow %6.1f ms",
                         name as NSString, open, med(keys), med(arrows)))
        }
    }
}

// MARK: - EP-047 S3 — the first-line indent (SP-169)

/// ✅ EP-047 AC6 (P3, P10): the indent is DRAWN by the presenter — measured here where TextKit actually lays out each line,
/// so an edit that TextKit is not re-asked about would fail. ⛔ Zero characters ever reach storage.
@Suite("Manuscript indent (EP-047 S3)")
@MainActor
struct ManuscriptIndentTests {

    private func fixture(_ text: String, _ indent: ManuscriptTypography.ParagraphIndent = .book, em: CGFloat = 1.5) -> (ManuscriptFixture, ManuscriptTypography) {
        let t = ManuscriptTypography(faceName: "Literata", size: 16, indent: indent, indentEm: em)
        let f = ManuscriptFixture()
        f.presenter.typography = t
        f.tv.frame = NSRect(x: 0, y: 0, width: 800, height: 600)
        f.window.setContentSize(f.tv.frame.size)
        f.tv.typingAttributes = t.bodyAttributes
        f.tv.textStorage?.replaceCharacters(in: NSRange(location: 0, length: 0), with: NSAttributedString(string: text, attributes: t.bodyAttributes))
        return (f, t)
    }

    /// Where TextKit DREW the first glyph of the line holding `needle` (x, relative to the line's own fragment origin).
    private func indentOf(_ f: ManuscriptFixture, _ needle: String) throws -> CGFloat {
        let lm = try #require(f.tv.textLayoutManager)
        lm.ensureLayout(for: lm.documentRange)
        let loc = (f.text as NSString).range(of: needle).location
        let cm = try #require(lm.textContentManager)
        let tl = try #require(cm.location(cm.documentRange.location, offsetBy: loc))
        let frag = try #require(lm.textLayoutFragment(for: tl))
        let within = cm.offset(from: frag.rangeInElement.location, to: tl)
        let line = try #require(frag.textLineFragments.first { NSLocationInRange(within, $0.characterRange) })
        // ⚠️ Measured (probe, 2026-10-07): TextKit 2 applies `firstLineHeadIndent` by moving the paragraph's FRAGMENT frame
        // (x = 5 pt padding + 24 pt), not the line's own bounds (x = 0) — so the drawn x is frame + line − padding.
        return frag.layoutFragmentFrame.minX + line.typographicBounds.minX - (f.tv.textContainer?.lineFragmentPadding ?? 0)
    }
    /// The drawn height of the (blank) line at `loc`.
    private func lineHeight(_ f: ManuscriptFixture, at loc: Int) throws -> CGFloat {
        let lm = try #require(f.tv.textLayoutManager)
        lm.ensureLayout(for: lm.documentRange)
        let cm = try #require(lm.textContentManager)
        let tl = try #require(cm.location(cm.documentRange.location, offsetBy: loc))
        return try #require(lm.textLayoutFragment(for: tl)).layoutFragmentFrame.height
    }

    static let three = "First paragraph.\n\nSecond paragraph.\n\nThird paragraph."

    @Test("Book (the default): no indent on a scene's first paragraph; 1.5 em after body text")
    func book() throws {
        let (f, t) = fixture(Self.three)
        #expect(ManuscriptTypography.defaultIndent == .book && ManuscriptTypography.defaultIndentEm == 1.5)
        #expect(try indentOf(f, "First") < 0.5, "a scene's first paragraph is not indented")
        #expect(abs(try indentOf(f, "Second") - t.firstLineIndent) < 0.5, "indented after body text")
        #expect(abs(try indentOf(f, "Third") - 24) < 0.5, "1.5 em of 16 pt = 24 pt")
    }

    @Test("Every: every body paragraph, the first included · None: no indent at all")
    func everyAndNone() throws {
        let (e, t) = fixture(Self.three, .every)
        #expect(abs(try indentOf(e, "First") - t.firstLineIndent) < 0.5)
        let (n, _) = fixture(Self.three, .none)
        #expect(try indentOf(n, "Second") < 0.5 && indentOf(n, "Third") < 0.5)
    }

    @Test("Book: not after a heading, a list or a quote; headings and list items never indented")
    func bookRule() throws {
        let (f, t) = fixture("## A Heading\n\nAfter the heading.\n\nThen body.\n\n- a list item\n\nAfter the list.\n\n> a quote\n\nAfter the quote.")
        #expect(try indentOf(f, "A Heading") < 0.5, "a heading is never indented")
        #expect(try indentOf(f, "After the heading") < 0.5, "not after a heading")
        #expect(abs(try indentOf(f, "Then body") - t.firstLineIndent) < 0.5, "after body text")
        #expect(try indentOf(f, "After the list") < 0.5, "not after a list")
        #expect(try indentOf(f, "After the quote") < 0.5, "not after a quote")
    }

    @Test("only a paragraph's FIRST line: a soft break's next line is not indented")
    func firstLineOnly() throws {
        let (f, t) = fixture("Opening paragraph.\n\nLine one of two\nline two of two.")
        #expect(abs(try indentOf(f, "Line one") - t.firstLineIndent) < 0.5)
        #expect(try indentOf(f, "line two") < 0.5)
    }

    @Test("with an indent, the blank line between paragraphs is a SMALL GAP; with None, a full line")
    func gap() throws {
        let blank = ("A.\n\nB." as NSString).range(of: "\n\n").location + 1
        let (f, t) = fixture("A.\n\nB.")
        #expect(abs(try lineHeight(f, at: blank) - t.size * ManuscriptTypography.lineSpacing * ManuscriptTypography.gapFraction) < 0.5)
        let (n, tn) = fixture("A.\n\nB.", .none)
        #expect(abs(try lineHeight(n, at: blank) - tn.size * ManuscriptTypography.lineSpacing) < 0.5, "None: the full blank line")
    }

    @Test("zero characters: typing and Return keep the stored text exactly what was typed, and the indent follows")
    func zeroCharacters() throws {
        let (f, t) = fixture("First paragraph.")
        f.caret((f.text as NSString).length)
        f.tv.insertNewline(nil)                         // Return writes "\n\n" (EP-045 AC5)
        f.type("Typed second")
        #expect(f.text == "First paragraph.\n\nTyped second", "nothing but the writer's characters is stored")
        #expect(abs(try indentOf(f, "Typed second") - t.firstLineIndent) < 0.5, "the new paragraph is indented as it is typed")
    }

    @Test("an edit re-presents the FOLLOWING paragraph: making the one above a heading removes its indent (book rule)")
    func followingBlockRepresented() throws {
        let (f, t) = fixture("Opening.\n\nMiddle paragraph.\n\nLast paragraph.")
        #expect(abs(try indentOf(f, "Last") - t.firstLineIndent) < 0.5)
        let mid = (f.text as NSString).range(of: "Middle").location
        // Typing `#` stores `\#` (escaped), so write the heading as a FILE or the Format command does — through storage.
        f.tv.textStorage?.replaceCharacters(in: NSRange(location: mid, length: 0),
                                            with: NSAttributedString(string: "## ", attributes: t.bodyAttributes))
        #expect(f.text.contains("## Middle"), "\(f.text)")
        #expect(try indentOf(f, "Last") < 0.5, "the paragraph after a heading lost its indent — TextKit was re-asked")
    }

    @Test("settings: absent = Book at 1.5 em; a choice round-trips and is written only once chosen")
    func settings() throws {
        let base = FileManager.default.temporaryDirectory.appendingPathComponent("scrivi-indent-\(UUID().uuidString)")
        let support = base.appendingPathComponent("s").path(percentEncoded: false)
        let root = base.appendingPathComponent("p.scrivi").path(percentEncoded: false)
        try FileManager.default.createDirectory(atPath: support, withIntermediateDirectories: true)
        try FileManager.default.createDirectory(atPath: root, withIntermediateDirectories: true)
        defer { try? FileManager.default.removeItem(at: base) }
        let engine = ScriviEngine()
        let id = try engine.ensureLocalIdentity(displayName: "T", appSupportRoot: support)
        let pid = try engine.createProject(projectRootPath: root, appSupportRoot: support, title: "T", slug: "t",
            authorshipRef: AuthorshipRef(identityID: id.identityID, personaID: id.defaultPersonaID, displayName: "T")).projectID
        let defaults = UserDefaults(suiteName: "scrivi.tests.indent.\(UUID().uuidString)")!
        func prefs() -> ProjectPreferences { ProjectPreferences(projectID: pid, projectRootPath: root, schemaTitle: "T", engine: engine, defaults: defaults) }
        func file() -> [String: Any] {
            let json = (try? engine.getProjectSettings(projectRootPath: root).documentJSON) ?? nil
            return json.flatMap { (try? JSONSerialization.jsonObject(with: Data($0.utf8))) as? [String: Any] } ?? [:]
        }
        let p = prefs()
        #expect(p.typography.indent == .book && p.typography.indentEm == 1.5)
        p.projectSubtitle = "Sub"
        #expect(file()["paragraphIndent"] == nil && file()["indentEm"] == nil, "another setting's save pins no default")
        p.paragraphIndent = "none"
        p.indentEm = 2.5
        let again = prefs()
        #expect(again.typography.indent == .none && again.typography.indentEm == 2.5)
    }
}

// MARK: - EP-047 S4 — Markup Hints on/off (SP-170, T-0589)

/// ✅ EP-047 AC7: hints OFF hides every marker and heading prefix even beside the caret; the caret's HOME is unchanged (T-0589
/// rules 1–3); a toggle re-presents only — the text and its history are untouched. Through the real text view and presenter.
@Suite("Markup Hints on/off (EP-047 S4)")
@MainActor
struct MarkupHintsTests {

    //                    0         1         2
    //                    012345678901234567890123456789
    static let text = "## Heading\n\nSome **bold** words here."

    private func fixture() -> ManuscriptFixture {
        let f = ManuscriptFixture(Self.text)
        f.tv.frame = NSRect(x: 0, y: 0, width: 700, height: 300)
        f.window.setContentSize(f.tv.frame.size)
        return f
    }
    private func setHints(_ f: ManuscriptFixture, _ on: Bool) {
        f.presenter.setHints(on, selection: f.tv.selectedRange(), in: f.tv.textStorage!)
    }
    /// The presented point size at `loc` (a hidden character is drawn at 0.01 pt).
    private func drawnSize(_ f: ManuscriptFixture, at loc: Int) throws -> CGFloat {
        let ns = f.text as NSString
        let r = ns.paragraphRange(for: NSRange(location: loc, length: 0))
        let cs = try #require(f.tv.textContentStorage)
        let p = try #require(f.presenter.textContentStorage(cs, textParagraphWith: r))
        return (p.attributedString.attribute(.font, at: loc - r.location, effectiveRange: nil) as? NSFont)?.pointSize ?? -1
    }

    @Test("ON (the default): the caret at a heading's start reveals its prefix; at a bold word's first letter, its markers")
    func onReveals() throws {
        let f = fixture()
        #expect(f.presenter.hintsEnabled, "default ON (P11)")
        f.caret(3)                                            // the heading's visible start
        #expect(!f.hidden(0) && !f.hidden(1), "prefix revealed")
        let bold = (f.text as NSString).range(of: "bold").location
        f.caret(bold)
        #expect(!f.hidden(bold - 2) && !f.hidden(bold + 4), "the span's markers revealed")
    }

    @Test("OFF: nothing is revealed — prefix and markers stay hidden beside the caret, and are DRAWN hidden")
    func offHides() throws {
        let f = fixture()
        setHints(f, false)
        f.caret(3)
        #expect(f.hidden(0) && f.hidden(1), "prefix hidden with the caret on its line")
        #expect(try drawnSize(f, at: 0) < 0.1, "and DRAWN hidden")
        let bold = (f.text as NSString).range(of: "bold").location
        f.caret(bold)
        #expect(f.hidden(bold - 2) && f.hidden(bold + 4), "markers hidden with the caret at the span's first letter")
        #expect(try drawnSize(f, at: bold - 2) < 0.1)
        f.caret(bold + 4)                                     // last letter's end
        #expect(f.hidden(bold + 4) && f.hidden(bold + 5))
    }

    @Test("OFF keeps T-0589's caret rules: prefix and opener → AFTER, closer → BEFORE")
    func offCaretRules() throws {
        let f = fixture()
        setHints(f, false)
        f.caret(1)                                            // inside the hidden `## `
        #expect(f.tv.selectedRange().location == 3, "rule 1: the caret goes AFTER the hidden prefix (got \(f.tv.selectedRange().location))")
        let bold = (f.text as NSString).range(of: "bold").location
        f.caret(bold - 1)                                     // inside the hidden opener
        #expect(f.tv.selectedRange().location == bold, "rule 2: AFTER the opener — typing prepends to the bold (got \(f.tv.selectedRange().location))")
        f.caret(bold + 5)                                     // inside the hidden closer
        #expect(f.tv.selectedRange().location == bold + 4, "rule 3: BEFORE the closer — typing continues the bold (got \(f.tv.selectedRange().location))")
    }

    @Test("a toggle re-presents only: the text is unchanged and nothing reaches storage")
    func toggleIsPresentationOnly() throws {
        let f = fixture()
        f.caret(3)
        var changes = 0
        let token = NotificationCenter.default.addObserver(forName: NSText.didChangeNotification, object: f.tv, queue: nil) { _ in changes += 1 }
        defer { NotificationCenter.default.removeObserver(token) }
        #expect(try drawnSize(f, at: 0) > 0.1, "revealed before the toggle")
        setHints(f, false)
        #expect(f.hidden(0))
        #expect(try drawnSize(f, at: 0) < 0.1, "the toggle re-presents the caret's line WITHOUT the caret moving")
        setHints(f, true)
        #expect(!f.hidden(0))
        #expect(try drawnSize(f, at: 0) > 0.1)
        #expect(f.text == Self.text, "the stored text is untouched")
        #expect(changes == 0, "no textDidChange — so no history event and no save")
    }

    @Test("settings: absent = ON; a choice round-trips and is written only once chosen")
    func settings() throws {
        let base = FileManager.default.temporaryDirectory.appendingPathComponent("scrivi-hints-\(UUID().uuidString)")
        let support = base.appendingPathComponent("s").path(percentEncoded: false)
        let root = base.appendingPathComponent("p.scrivi").path(percentEncoded: false)
        try FileManager.default.createDirectory(atPath: support, withIntermediateDirectories: true)
        try FileManager.default.createDirectory(atPath: root, withIntermediateDirectories: true)
        defer { try? FileManager.default.removeItem(at: base) }
        let engine = ScriviEngine()
        let id = try engine.ensureLocalIdentity(displayName: "T", appSupportRoot: support)
        let pid = try engine.createProject(projectRootPath: root, appSupportRoot: support, title: "T", slug: "t",
            authorshipRef: AuthorshipRef(identityID: id.identityID, personaID: id.defaultPersonaID, displayName: "T")).projectID
        let defaults = UserDefaults(suiteName: "scrivi.tests.hints.\(UUID().uuidString)")!
        func prefs() -> ProjectPreferences { ProjectPreferences(projectID: pid, projectRootPath: root, schemaTitle: "T", engine: engine, defaults: defaults) }
        func file() -> [String: Any] {
            let json = (try? engine.getProjectSettings(projectRootPath: root).documentJSON) ?? nil
            return json.flatMap { (try? JSONSerialization.jsonObject(with: Data($0.utf8))) as? [String: Any] } ?? [:]
        }
        let p = prefs()
        #expect(p.markupHints, "absent = ON")
        p.projectSubtitle = "Sub"
        #expect(file()["markupHints"] == nil, "another setting's save pins no default")
        p.markupHints = false
        #expect(prefs().markupHints == false)
        #expect(file()["markupHints"] as? Bool == false)
    }
}

// MARK: - [I-0283] macOS Smart Quotes are OFF in the manuscript

/// ✅ [I-0283] (ruling (a), 2026-10-08): AppKit's Smart Quotes rewrote STORED text outside the escape layer — three typed apostrophes
/// became curly·straight·curly, a typed "hi" gained an extra quote, the caret thrown. Typed through the real `keyDown`, then AppKit's
/// own text-checking pass, with Smart Quotes switched ON for the view — the manuscript must ignore it.
@Suite("Smart Quotes off in the manuscript ([I-0283])")
@MainActor
struct SmartQuotesOffTests {
    private func type(_ chars: [String], into f: ManuscriptFixture) {
        for ch in chars {
            let e = NSEvent.keyEvent(with: .keyDown, location: .zero, modifierFlags: [], timestamp: 0, windowNumber: f.window.windowNumber,
                                     context: nil, characters: ch, charactersIgnoringModifiers: ch, isARepeat: false, keyCode: 0)!
            f.tv.keyDown(with: e)
        }
        f.tv.checkTextInDocument(nil)                      // AppKit's substitution pass — what rewrote the text
        RunLoop.current.run(until: Date().addingTimeInterval(0.2))
    }

    @Test("typed quotes stay as typed (escaped, straight) and the caret stays put — even with Smart Quotes switched on")
    func quotesUntouched() {
        let f = ManuscriptFixture("Before ")
        f.tv.isAutomaticQuoteSubstitutionEnabled = true    // the Edit ▸ Substitutions toggle
        #expect(f.tv.isAutomaticQuoteSubstitutionEnabled == false, "the manuscript never turns it on")
        f.caret(7)
        type(["'", "'", "'"], into: f)
        #expect(f.text == #"Before \'\'\'"#, "single quotes untouched — \(f.text.debugDescription)")
        #expect(f.tv.selectedRange().location == (f.text as NSString).length)
        let g = ManuscriptFixture("Before ")
        g.tv.isAutomaticQuoteSubstitutionEnabled = true
        g.caret(7)
        type(["\"", "h", "i", "\""], into: g)
        #expect(g.text == #"Before \"hi\""#, "double quotes untouched, none added — \(g.text.debugDescription)")
        #expect(g.tv.selectedRange().location == (g.text as NSString).length)
    }

    @Test("Edit ▸ Substitutions ▸ Smart Quotes is disabled for the manuscript")
    func menuDisabled() {
        let f = ManuscriptFixture("x")
        let item = NSMenuItem(title: "Smart Quotes", action: #selector(NSTextView.toggleAutomaticQuoteSubstitution(_:)), keyEquivalent: "")
        #expect(f.tv.validateUserInterfaceItem(item) == false)
    }
}

// MARK: — EP-050 S1 ([SP-171]) Plan 1 — measure before building

/// [SP-171] Plan 1(a) and 1(c). ⚠️ SPIKE: measurements, recorded in `Sprint-SP-171.md`. Production code is untouched — the probe
/// override is added through the ObjC runtime, so the question "is a member on `ManuscriptNSTextView` REACHED by AppKit's
/// attribute dispatch?" is asked without shipping one.
@Suite("Manuscript accessibility — Plan 1 spike (EP-050 S1)")
@MainActor
struct ManuscriptAccessibilitySpike {

    /// 1(a) static: does `NSTextView` implement the LEGACY attribute entry points itself? If it inherits them, the legacy
    /// path is NSObject/NSView's generic mapper onto the new-style members — which a subclass override reaches.
    @Test("1(a) which class implements the legacy attribute dispatch")
    func legacyDispatchOwner() {
        for name in ["accessibilityAttributeValue:", "accessibilityAttributeValue:forParameter:",
                     "accessibilityAttributeNames", "accessibilityParameterizedAttributeNames",
                     "accessibilityValue", "accessibilityStringForRange:", "accessibilityNumberOfCharacters"] {
            let sel = NSSelectorFromString(name)
            func owner(_ c: AnyClass) -> String {
                var k: AnyClass? = c
                let imp = class_getMethodImplementation(c, sel)
                var last = NSStringFromClass(c)
                while let cur = k, let sup = class_getSuperclass(cur), class_getMethodImplementation(sup, sel) == imp {
                    last = NSStringFromClass(sup); k = sup
                }
                return last
            }
            print("[SP-171 1a] \(name): ManuscriptNSTextView inherits it from \(owner(ManuscriptNSTextView.self))")
        }
    }

    // 1(a) dynamic (probe overrides asked through the LEGACY names) was REMOVED at Plan 3: its finding is recorded in
    // `Sprint-SP-171.md`, and 1(b) measured VoiceOver entering at the new-style members — which the members suite tests.

    /// 1(c) → AC3: the map's cost on a dumas-shaped 1.7 MB manuscript (60 chapters × 20 scenes, titles and dividers, escapes and
    /// emphasis in every paragraph). ⚠️ Generated: the test host is sandboxed and cannot read the dumas fixture; [SP-164]'s
    /// whole-build figure on the real dumas (166–178 ms) calibrates it. The first run of this test (the flat-array design) is
    /// recorded in `Sprint-SP-171.md`.
    @Test("1(c)/AC3 presented-map cost on 1.7 MB: whole build, a patched edit at the start and the end, a snapshot")
    func patchCost() throws {
        var infos: [SceneInfo] = []
        var segs: [SceneSegment] = []
        let para = #"Mr\. Smith said \*no\* \- the ship came in on the **evening** tide, her sails the colour of *old parchment* against a sky turning to brass\."#
        for ch in 0..<60 {
            for sc in 0..<20 {
                let id = "s\(ch)_\(sc)"
                infos.append(try JSONDecoder().decode(SceneInfo.self, from: Data("""
                    {"sceneID":"\(id)","chapterID":"c\(ch)","title":"S","chapterTitle":"Chapter \(ch + 1)","slug":"s",
                     "metadataPath":"","contentPath":"","chapterMetadataPath":""}
                    """.utf8)))
                let text = (sc == 0 ? "## Part \(ch + 1)\n\n" : "") + (0..<10).map { _ in para }.joined(separator: "\n\n")
                segs.append(SceneSegment(id: id, sceneID: id, chapterID: "c\(ch)", metadataPath: "", contentPath: "", text: text))
            }
        }
        let loader = ViewportSceneLoader(engine: ScriviEngine(), projectRootPath: "/tmp/ax-spike",
                                         appSupportRoot: "/tmp/ax-spike-support", projectID: "p", allScenes: infos)
        let session = ProjectSession(engine: ScriviEngine(), authorshipRef: nil, appSupportRoot: "/tmp/ax-spike-support", identityID: "")
        let view = ManuscriptTextView(loader: loader, env: AppEnvironment(), session: session, navigateToSceneID: .constant(nil),
                                      showChapterTitles: true, typography: .default)
        let c = view.makeCoordinator()
        let tv = ManuscriptNSTextView(usingTextLayoutManager: true)
        tv.frame = NSRect(x: 0, y: 0, width: 900, height: 700)
        tv.textContentStorage?.delegate = c.presenter
        tv.textStorage?.delegate = c.presenter
        c.textView = tv
        c.rebuildStorage(tv, segments: segs)
        let ts = try #require(tv.textStorage)
        print("[SP-171 AC3] storage length \(ts.length)")

        func ms(_ body: () -> Void) -> Double {
            let t0 = DispatchTime.now().uptimeNanoseconds; body()
            return Double(DispatchTime.now().uptimeNanoseconds - t0) / 1e6
        }
        func median(_ xs: [Double]) -> Double { xs.sorted()[xs.count / 2] }
        let start = (ts.string as NSString).range(of: "Mr").location + 4, end = ts.length - 20
        // The same one-character edit (and its undo) with no map, then with the map live: the difference is the patch.
        func edits(at loc: Int) -> Double {
            median((0..<7).map { _ in
                ms {
                    ts.replaceCharacters(in: NSRange(location: loc, length: 0), with: NSAttributedString(string: "x"))
                    ts.replaceCharacters(in: NSRange(location: loc, length: 1), with: NSAttributedString(string: ""))
                }
            })
        }
        let bareStart = edits(at: start), bareEnd = edits(at: end)
        var built = 0.0
        built = ms { _ = tv.presentedMap.current() }
        let lay = try #require(tv.presentedMap.current())
        let liveStart = edits(at: start), liveEnd = edits(at: end)
        let snap = ms { _ = tv.presentedMap.snapshot() }
        let whole = ms { _ = tv.presentedMap.snapshot().string }
        print(String(format: "[SP-171 AC3] whole build %.1f ms (%d segments, %d units) · 2 edits at START %.2f → %.2f ms · at END %.2f → %.2f ms · snapshot %.2f ms · whole presented string %.1f ms · builds %d",
                     built, lay.segments.count, lay.length, bareStart, liveStart, bareEnd, liveEnd, snap, whole, tv.presentedMap.builds))
        // VoiceOver's per-keystroke query set (measured in Plan 1(b)), after a typed edit near the START — the map's members only;
        // the LINE members are AppKit's own work on storage (they lay the text out) and are measured in the live pass.
        tv.setSelectedRange(NSRange(location: start, length: 0))
        let queries = median((0..<7).map { _ in
            ts.replaceCharacters(in: NSRange(location: start, length: 0), with: NSAttributedString(string: "x"))
            return ms {
                let k = tv.accessibilitySelectedTextRange().location
                for _ in 0..<2 { _ = tv.accessibilityNumberOfCharacters() }
                for d in 0..<7 { _ = tv.accessibilityString(for: NSRange(location: max(0, k - 40 + d * 10), length: 40)) }
                for _ in 0..<3 { _ = tv.accessibilitySelectedTextRange() }
                for _ in 0..<2 { _ = tv.accessibilityAttributedString(for: NSRange(location: max(0, k - 20), length: 40)) }
            }
        })
        print(String(format: "[SP-171 AC3] VoiceOver's per-keystroke query set (map members) after an edit: %.2f ms", queries))
        #expect(tv.presentedMap.builds == 1, "every edit PATCHED the map")
    }
}

/// EP-050 S1 ([SP-171]) Plan 2 / AC1 — THE ONE MAP, patched per edit, against a fresh build.
@Suite("Presented map (EP-050 S1)")
@MainActor
struct PresentedMapTests {

    private func body(_ s: String) -> NSAttributedString {
        NSAttributedString(string: s, attributes: ManuscriptTypography.default.bodyAttributes)
    }

    /// Titles, dividers, escapes, emphasis, headings, lists, hard breaks, blank lines.
    private func corpus() -> NSAttributedString {
        let s = NSMutableAttributedString()
        for ch in 0..<3 {
            s.append(NSAttributedString(string: "Chapter \(ch + 1)\n", attributes: [.scriviHeading: true]))
            for sc in 0..<3 {
                s.append(body(#"## A heading \*here\*"# + "\n\n"))
                s.append(body(#"Mr\. **Smith** said \*no\* to *her* \- twice\."# + "\n\n"))
                s.append(body("- one\n- two **bold**\n\n"))
                // ⚠️ [I-0284]: a block holding a hard break renders NO emphasis (screen and map agree), so the hard break and the
                // `_` / `***` emphasis are in separate paragraphs here.
                s.append(body(#"A hard break\"# + "\n" + #"then on\."# + "\n\n"))
                s.append(body("And _under_ and ***both***."))
                if sc < 2 { s.append(NSAttributedString(string: "\u{FFFC}", attributes: [.scriviDivider: DividerRenderState.sceneBreak])); s.append(body("\n")) }
            }
            if ch < 2 { s.append(NSAttributedString(string: "\u{FFFC}", attributes: [.scriviDivider: DividerRenderState.chapterEnd])); s.append(body("\n")) }
        }
        return s
    }

    /// The live map and a fresh build agree at EVERY position, both ways, and on the text and the chunks.
    private func expectAgrees(_ f: ManuscriptFixture, _ note: @autoclosure () -> String) {
        let ts = f.tv.textStorage!
        guard let live = f.tv.presentedMap.current() else { Issue.record("no map"); return }
        let fresh = PresentedLayout.build(ts, presenter: f.presenter)
        let storage = ts.string as NSString
        guard live.length == fresh.length, live.storageEnd == fresh.storageEnd else {
            Issue.record("length \(live.length) vs \(fresh.length), storage \(live.storageEnd) vs \(fresh.storageEnd) — \(note())")
            return
        }
        let whole = NSRange(location: 0, length: fresh.length)
        if live.units(in: whole, from: storage) != fresh.units(in: whole, from: storage) { Issue.record("text differs — \(note())"); return }
        for k in 0...fresh.length where live.storageIndex(k) != fresh.storageIndex(k) {
            Issue.record("storageIndex(\(k)) \(live.storageIndex(k)) vs \(fresh.storageIndex(k)) — \(note())"); return
        }
        for x in 0...ts.length where live.presentedIndex(x) != fresh.presentedIndex(x) {
            Issue.record("presentedIndex(\(x)) \(live.presentedIndex(x)) vs \(fresh.presentedIndex(x)) — \(note())"); return
        }
        if live.chunks() != fresh.chunks() { Issue.record("chunks differ — \(note())") }
        if live.outline() != fresh.outline() { Issue.record("outline differs — \(note())") }
    }

    private func storageLen(_ f: ManuscriptFixture) -> Int { f.tv.textStorage!.length }

    @Test("AC1: the page at rest — every mark hidden, list prefixes kept, titles and dividers present; round trips")
    func pageAtRest() throws {
        let f = ManuscriptFixture()
        f.tv.textStorage!.setAttributedString(corpus())
        f.caret(40)                                        // a caret beside markup reveals it on screen — never in the map (A1)
        let p = f.tv.presentedMap.snapshot()
        let text = p.string as String
        #expect(text.hasPrefix("Chapter 1\nA heading *here*\n\nMr. Smith said *no* to her - twice.\n\n- one\n- two bold\n\n"),
                "presented: \(text.prefix(120).debugDescription)")
        // ✅ The map hides EXACTLY what the presenter hides on screen at rest (the caret far away; hints cannot reveal anything there).
        let far = NSRange(location: f.text.utf16.count, length: 0)
        let hiddenByMap = Set((0..<storageLen(f)).filter { p.layout.presentedIndex($0) == p.layout.presentedIndex($0 + 1) })
        let hiddenOnScreen = Set((0..<storageLen(f)).filter { f.presenter.isHidden($0, in: f.tv.textStorage!, revealing: far) })
        #expect(hiddenByMap == hiddenOnScreen, "map-only: \(hiddenByMap.subtracting(hiddenOnScreen).sorted().prefix(10)) · screen-only: \(hiddenOnScreen.subtracting(hiddenByMap).sorted().prefix(10))")
        for mark in ["\\", "**", "## "] {
            let r = (text as NSString).range(of: mark)
            #expect(r.location == NSNotFound, "\(mark.debugDescription) presented in: \((text as NSString).substring(with: NSRange(location: max(0, r.location - 30), length: min(60, (text as NSString).length - max(0, r.location - 30)))).debugDescription)")
        }
        #expect(text.contains("Scene break") && text.contains("End of chapter") && !text.contains("\u{FFFC}"), "Q2: dividers present as WORDS")
        // Round trips: every presented unit maps to a storage character that IS that unit.
        let storage = f.text as NSString
        for k in 0..<p.length {
            let s = p.layout.storageIndex(k)
            if storage.character(at: s) == 0xFFFC {                 // a divider's words: every unit maps to the divider character
                #expect(p.layout.presentedIndex(s) <= k)
                continue
            }
            #expect(p.layout.presentedIndex(s) == k)
            if storage.character(at: s) != p.string.character(at: k) { Issue.record("unit \(k) ≠ storage \(s)"); break }
        }
    }

    @Test("Plan 2: after every edit the PATCHED map equals a fresh build — and no edit rebuilt it")
    func patchedEqualsFresh() {
        let f = ManuscriptFixture()
        f.tv.textStorage!.setAttributedString(corpus())
        _ = f.tv.presentedMap.current()
        #expect(f.tv.presentedMap.builds == 1)
        var rng = SystemRandomNumberGenerator()
        var seed: UInt64 = 0x5EED_0171
        func next(_ n: Int) -> Int {                       // deterministic (SplitMix64), so a failure reproduces
            seed &+= 0x9E37_79B9_7F4A_7C15
            var z = seed
            z = (z ^ (z >> 30)) &* 0xBF58_476D_1CE4_E5B9
            z = (z ^ (z >> 27)) &* 0x94D0_49BB_1331_11EB
            return Int((z ^ (z >> 31)) % UInt64(max(n, 1)))
        }
        _ = rng
        let tokens = ["a", " ", "\n", "\n\n", "*", "**", "\\", "\\*", "# ", "## ", "- ", "x y", "_", "***"]
        let ts = f.tv.textStorage!
        for step in 0..<300 {
            let len = ts.length
            let loc = next(len + 1)
            switch next(3) {
            case 0:
                let t = tokens[next(tokens.count)]
                ts.replaceCharacters(in: NSRange(location: loc, length: 0), with: body(t))
                expectAgrees(f, "step \(step): insert \(t.debugDescription) at \(loc)")
            case 1:
                let n = min(1 + next(6), len - loc)
                guard n > 0 else { continue }
                ts.replaceCharacters(in: NSRange(location: loc, length: n), with: body(""))
                expectAgrees(f, "step \(step): delete \(n) at \(loc)")
            default:
                let n = min(1 + next(4), len - loc)
                let t = tokens[next(tokens.count)]
                ts.replaceCharacters(in: NSRange(location: loc, length: max(0, n)), with: body(t))
                expectAgrees(f, "step \(step): replace \(n) at \(loc) with \(t.debugDescription)")
            }
        }
        #expect(f.tv.presentedMap.builds == 1, "every edit PATCHED the map (\(f.tv.presentedMap.builds) builds)")
    }

    @Test("Plan 2: typed edits through the view (escape layer, Return, ⌫) keep the map exact")
    func typedEdits() {
        let f = ManuscriptFixture()
        f.tv.textStorage!.setAttributedString(corpus())
        _ = f.tv.presentedMap.current()
        f.caret(20); f.type("Mr. *quoted* #1")
        expectAgrees(f, "typed punctuation")
        f.tv.insertNewline(nil); f.type("## new")
        expectAgrees(f, "Return then a heading")
        f.tv.deleteBackward(nil); f.tv.deleteBackward(nil)
        expectAgrees(f, "⌫ ⌫")
        #expect(f.tv.presentedMap.builds == 1)
    }

    @Test("Plan 2: the PENDING pair (⌘B between words) is hidden on the page at rest")
    func pendingPairHidden() {
        let f = ManuscriptFixture("one  two")
        _ = f.tv.presentedMap.current()
        f.caret(4)                                                // BETWEEN the words (in a word formats the word)
        f.tv.applyFormat(.bold)
        #expect(f.text == "one ****two" || f.text == "one **** two", "the pair is in storage: \(f.text.debugDescription)")
        #expect(f.tv.presentedMap.snapshot().string as String == "one  two")
        expectAgrees(f, "pending inserted")
        f.tv.applyFormat(.bold)                                   // the same command takes it away
        #expect(f.tv.presentedMap.snapshot().string as String == "one  two")
        expectAgrees(f, "pending removed")
    }

    @Test("Q4: Find never matches inside a chapter title or divider — the finder is handed them masked")
    func findSkipsTitles() {
        let f = ManuscriptFixture()
        f.tv.textStorage!.setAttributedString(corpus())
        let client = f.tv.finderClient
        client.textView = f.tv
        let p = client.presented
        var range = NSRange()
        var flag: ObjCBool = false
        let title = (p.string as NSString).range(of: "Chapter 2")
        let s = client.string(at: title.location + 2, effectiveRange: &range, endsWithSearchBoundary: &flag)
        #expect(!s.contains("Chapter"), "masked: \(s.debugDescription)")
        #expect(NSLocationInRange(title.location + 2, range) && (s as NSString).length == range.length)
        #expect(flag.boolValue)
    }
}

/// EP-050 S1 ([SP-171]) Plan 3 / AC1 — every accessibility member answers through the ONE map, and the members agree. ✅ Called
/// at the NEW-STYLE members (VoiceOver's entry, measured in Plan 1(b)); `AXValue` through its legacy attribute name.
@Suite("Manuscript accessibility members (EP-050 AC1)")
@MainActor
struct ManuscriptAccessibilityTests {

    private func fixture(_ text: String) -> ManuscriptFixture {
        let f = ManuscriptFixture(text)
        f.tv.frame = NSRect(x: 0, y: 0, width: 300, height: 2000)
        f.tv.textContainer?.widthTracksTextView = true
        f.tv.textLayoutManager?.ensureLayout(for: f.tv.textLayoutManager!.documentRange)
        return f
    }

    private let text = "## A heading \\*here\\*\n\nMr\\. **Smith** said \\*no\\* to *her* \\- twice, and then once more for the long line to wrap.\n\n- one\n- two **bold**"
    private let presented = "A heading *here*\n\nMr. Smith said *no* to her - twice, and then once more for the long line to wrap.\n\n- one\n- two bold"

    @Test("Q2: a divider is READ as words — \"Scene break\" / \"End of chapter\" (an AXAttachment label was ignored by VoiceOver)")
    func dividersNamed() throws {
        let f = fixture("")
        let s = NSMutableAttributedString(string: "one", attributes: ManuscriptTypography.default.bodyAttributes)
        s.append(NSAttributedString(string: "\u{FFFC}", attributes: [.scriviDivider: DividerRenderState.sceneBreak]))
        s.append(NSAttributedString(string: "\ntwo **b**", attributes: ManuscriptTypography.default.bodyAttributes))
        s.append(NSAttributedString(string: "\u{FFFC}", attributes: [.scriviDivider: DividerRenderState.chapterEnd]))
        s.append(NSAttributedString(string: "\nthree", attributes: ManuscriptTypography.default.bodyAttributes))
        f.tv.textStorage!.setAttributedString(s)
        let expected = "oneScene break\ntwo bEnd of chapter\nthree" as NSString
        #expect(f.tv.accessibilityAttributeValue(.value) as? String == expected as String)
        #expect(f.tv.accessibilityNumberOfCharacters() == expected.length)
        // Every range — including ranges that START or END inside the words — reads the same through both members.
        for loc in 0..<expected.length {
            for len in [1, 4, 9] where loc + len <= expected.length {
                let r = NSRange(location: loc, length: len)
                #expect(f.tv.accessibilityString(for: r) == expected.substring(with: r), "string(for: \(NSStringFromRange(r)))")
                #expect(f.tv.accessibilityAttributedString(for: r)?.string == expected.substring(with: r), "attributed(for: \(NSStringFromRange(r)))")
            }
        }
        // The words are ONE unit of the page: index and line ranges inside them cover all of them.
        let words = expected.range(of: "End of chapter")
        #expect(f.tv.accessibilityRange(for: words.location + 5) == words)
        // A caret set inside the words is proposed AT the divider character, then Scrivi's caret rules place it: here (no
        // coordinator, so no [T-0572] scene-gap move) before the `**` closer just ahead of it — a home, never past the divider.
        f.tv.setAccessibilitySelectedTextRange(NSRange(location: words.location + 3, length: 0))
        let c = f.tv.selectedRange().location
        let divider = (f.text as NSString).range(of: "\u{FFFC}", options: .backwards).location
        #expect(c <= divider && MarkdownEscapes.snapCaret(c, from: c, length: f.tv.textStorage!.length,
                                                          runAt: ManuscriptNSTextView.runLookup(f.presenter, f.tv.textStorage!)) == nil,
                "caret \(c), divider \(divider)")
    }

    @Test("[I-0277] AXValue, the character count and every range string are the PRESENTED text")
    func valueAndStrings() {
        let f = fixture(text)
        let value = f.tv.accessibilityAttributeValue(.value) as? String
        #expect(value == presented, "AXValue: \(value.debugDescription)")
        #expect(f.tv.accessibilityNumberOfCharacters() == (presented as NSString).length)
        let ns = presented as NSString
        for loc in stride(from: 0, to: ns.length, by: 7) {
            for len in [0, 1, 5, 23] where loc + len <= ns.length {
                let r = NSRange(location: loc, length: len)
                #expect(f.tv.accessibilityString(for: r) == ns.substring(with: r), "string(for: \(NSStringFromRange(r)))")
                #expect(f.tv.accessibilityAttributedString(for: r)?.string == ns.substring(with: r), "attributed(for: \(NSStringFromRange(r)))")
            }
        }
        #expect(f.tv.accessibilityString(for: NSRange(location: ns.length - 3, length: 50)) == ns.substring(from: ns.length - 3),
                "a range past the end is clamped")
        let rtf = f.tv.accessibilityRTF(for: NSRange(location: 18, length: 9))
        let back = rtf.flatMap { try? NSAttributedString(data: $0, options: [.documentType: NSAttributedString.DocumentType.rtf], documentAttributes: nil) }
        #expect(back?.string == "Mr. Smith")
    }

    @Test("AC1: the attributed string keeps the page's fonts — Smith is bold, the markers are gone")
    func attributedKeepsFonts() throws {
        let f = fixture(text)
        let ns = presented as NSString
        let smith = ns.range(of: "Smith")
        let a = try #require(f.tv.accessibilityAttributedString(for: NSRange(location: smith.location - 4, length: 14)))
        #expect(a.string == "Mr. Smith said")
        #expect(!a.string.contains("*"))
    }

    @Test("AC1: lines agree — line(for:) and range(forLine:) round-trip at every presented index")
    func linesAgree() {
        let f = fixture(text)
        let n = f.tv.accessibilityNumberOfCharacters()
        var lines = Set<Int>()
        for k in 0..<n {
            let line = f.tv.accessibilityLine(for: k)
            lines.insert(line)
            let r = f.tv.accessibilityRange(forLine: line)
            if !NSLocationInRange(k, r) { Issue.record("index \(k): line \(line) range \(NSStringFromRange(r)) does not hold it"); break }
            if NSMaxRange(r) > n { Issue.record("line \(line) range \(NSStringFromRange(r)) past the end \(n)"); break }
        }
        #expect(lines.count >= 6, "a wrapped line counts as more than one VISUAL line: \(lines.count)")
    }

    @Test("AC1: index, style and position ranges are presented ranges holding the index")
    func positionRanges() {
        let f = fixture(text)
        let n = f.tv.accessibilityNumberOfCharacters()
        for k in stride(from: 0, to: n, by: 3) {
            let r = f.tv.accessibilityRange(for: k)
            #expect(NSLocationInRange(k, r) && NSMaxRange(r) <= n, "rangeForIndex(\(k)) = \(NSStringFromRange(r))")
            let st = f.tv.accessibilityStyleRange(for: k)
            #expect(NSLocationInRange(k, st) && NSMaxRange(st) <= n, "styleRange(\(k)) = \(NSStringFromRange(st))")
        }
        let smith = (presented as NSString).range(of: "Smith")
        let frame = f.tv.accessibilityFrame(for: smith)
        // The SAME screen rect AppKit gives the stored "Smith" (the presented range is mapped before AppKit measures it).
        let sr = (f.text as NSString).range(of: "Smith")
        var stored = NSRect.zero
        if let tlm = f.tv.textLayoutManager, let cs = f.tv.textContentStorage,
           let a = cs.location(cs.documentRange.location, offsetBy: sr.location), let b = cs.location(a, offsetBy: sr.length),
           let tr = NSTextRange(location: a, end: b) {
            tlm.enumerateTextSegments(in: tr, type: .standard, options: []) { _, rect, _, _ in
                let inView = rect.offsetBy(dx: f.tv.textContainerOrigin.x, dy: f.tv.textContainerOrigin.y)
                stored = f.window.convertToScreen(f.tv.convert(inView, to: nil))
                return false
            }
        }
        #expect(frame.width > 10 && abs(frame.minX - stored.minX) < 0.5 && abs(frame.width - stored.width) < 0.5,
                "frame(for: Smith) = \(frame), stored Smith = \(stored)")
        let vis = f.tv.accessibilityVisibleCharacterRange()
        #expect(NSMaxRange(vis) <= n)
    }

    /// SP-172 Plan 5 finding (user): with VoiceOver on, a click sometimes shows a part of the manuscript EARLIER than the caret.
    @Test("Setting the visible range (presented) scrolls to that presented text — not to the same index in storage")
    func setVisibleRange() throws {
        let body = (0..<2_000).map { #"Para \#($0): Mr\. Smith said \*no\* \- twice\."# }.joined(separator: "\n\n")
        let f = fixture(body)
        let scroll = NSScrollView(frame: NSRect(x: 0, y: 0, width: 500, height: 300))
        f.tv.isVerticallyResizable = true
        f.tv.textContainer?.widthTracksTextView = true
        scroll.documentView = f.tv
        f.window.setContentSize(scroll.frame.size)
        f.window.contentView = scroll
        f.tv.textLayoutManager?.ensureLayout(for: f.tv.textLayoutManager!.documentRange)
        let presented = try #require(f.tv.accessibilityString(for: NSRange(location: 0, length: f.tv.accessibilityNumberOfCharacters())))
        let target = (presented as NSString).range(of: "Para 1500:")
        f.tv.setAccessibilityVisibleCharacterRange(target)
        let vis = f.tv.accessibilityVisibleCharacterRange()
        let shown = f.tv.accessibilityString(for: vis) ?? ""
        // ⚠️ Offscreen, AppKit reports the visible range with ZERO length — its location is the top of the viewport. Unmapped it
        // landed 6,888 presented characters early (53,502 for 60,390); a screen here is ~300 pt ≈ 15 paragraphs ≈ 600 characters.
        #expect(abs(vis.location - target.location) < 600, "visible \(vis) shows \(shown.prefix(40).debugDescription), target \(target)")
    }

    @Test("AC2: a caret VoiceOver sets at ANY presented index lands at home (never inside markup) and reads back as that index")
    func caretRoundTrip() {
        let f = fixture(text)
        let storage = f.tv.textStorage!
        let runAt = ManuscriptNSTextView.runLookup(f.presenter, storage)
        let n = f.tv.accessibilityNumberOfCharacters()
        // In order, in reverse, and re-set in place: VoiceOver's cursor and Scrivi's caret never drift apart.
        for k in Array(0...n) + Array((0...n).reversed()) {
            for _ in 0..<2 {                                              // the second set re-sets the SAME position
                f.tv.setAccessibilitySelectedTextRange(NSRange(location: k, length: 0))
                let c = f.tv.selectedRange().location
                // A HOME: the caret rules, asked to place a caret there, leave it there.
                if let moved = MarkdownEscapes.snapCaret(c, from: c, length: storage.length, runAt: runAt) {
                    Issue.record("presented \(k) → storage \(c): not a home (the rules move it to \(moved))"); return
                }
                let back = f.tv.accessibilitySelectedTextRange()
                // It reads back as k — ⚠️ except at a LIST PREFIX: visible ([SP-163] Q7) but never a caret home, so a caret set
                // before or inside `- ` lands after it. That is the ONLY move allowed.
                let atListPrefix = k < n && f.presenter.stopTest(in: storage)(f.tv.presentedMap.current()!.storageIndex(k))?.kind == .listPrefix
                if back.length != 0 || (back.location != k && !(atListPrefix && back.location > k)) {
                    Issue.record("presented \(k) → storage \(c) → reads back \(NSStringFromRange(back))"); return
                }
            }
        }
    }

    @Test("AC2: the homes — before an escape, after an opener and a heading prefix, before a closer")
    func caretHomes() {
        let f = fixture(text)
        let ns = f.text as NSString, p = presented as NSString
        func set(_ k: Int) -> Int { f.tv.setAccessibilitySelectedTextRange(NSRange(location: k, length: 0)); return f.tv.selectedRange().location }
        #expect(set(0) == 3, "after the `## ` prefix")
        #expect(set(p.range(of: "*here").location) == ns.range(of: #"\*here"#).location, "before the escape backslash")
        #expect(set(p.range(of: "Smith").location) == ns.range(of: "Smith").location, "after the opener")
        #expect(set(NSMaxRange(p.range(of: "her"))) == NSMaxRange(ns.range(of: "*her")) , "before the closer")
    }

    @Test("AC2 (mapping half): Scrivi's caret is reported in presented positions; a selection set by VoiceOver lands in storage")
    func selectionMapped() {
        let f = fixture(text)
        let storage = f.text as NSString
        let ns = presented as NSString
        f.caret(storage.range(of: "Smith").location + 2)                  // "Sm|ith"
        #expect(f.tv.accessibilitySelectedTextRange() == NSRange(location: ns.range(of: "Smith").location + 2, length: 0))
        f.tv.setSelectedRange(storage.range(of: "Smith"))
        #expect(f.tv.accessibilitySelectedText() == "Smith")
        #expect(f.tv.accessibilitySelectedTextRanges()?.first?.rangeValue == ns.range(of: "Smith"))
        f.tv.setAccessibilitySelectedTextRange(ns.range(of: "said *no*"))
        #expect(storage.substring(with: f.tv.selectedRange()) == #"said \*no\*"#)
    }
}

/// EP-050 S2 ([SP-172]) — the Headings rotor, called as VoiceOver calls it: `accessibilityCustomRotors`, then the delegate.
@Suite("Headings rotor (EP-050 S2)")
@MainActor
struct HeadingsRotorTests {

    private func search(_ rotor: NSAccessibilityCustomRotor, from current: NSRange?, next: Bool = true,
                        filter: String = "") -> NSAccessibilityCustomRotor.ItemResult? {
        let p = NSAccessibilityCustomRotor.SearchParameters()
        p.searchDirection = next ? .next : .previous
        p.filterString = filter
        if let current {
            let item = NSAccessibilityCustomRotor.ItemResult(targetElement: rotor.itemSearchDelegate as! NSAccessibilityElementProtocol)
            item.targetRange = current
            p.currentItem = item
        }
        return rotor.itemSearchDelegate?.rotor(rotor, resultFor: p)
    }

    @Test("AC4: one Headings rotor; chapter titles and Markdown headings in order, as presented text, both ways")
    func order() throws {
        let f = ManuscriptFixture()
        // ⚠️ Measured shape (Plan 1 run): a title run can begin with a newline; a heading line can end with a space.
        let s = NSMutableAttributedString(string: "\nChapter One\n", attributes: [.scriviHeading: true])
        s.append(NSAttributedString(string: "## First \\*part\\* \n\nbody **b**\n\n# Second\n\nmore", attributes: ManuscriptTypography.default.bodyAttributes))
        s.append(NSAttributedString(string: "\u{FFFC}", attributes: [.scriviDivider: DividerRenderState.chapterEnd]))
        s.append(NSAttributedString(string: "\n", attributes: ManuscriptTypography.default.bodyAttributes))
        s.append(NSAttributedString(string: "Chapter Two\n", attributes: [.scriviHeading: true]))
        s.append(NSAttributedString(string: "### Third", attributes: ManuscriptTypography.default.bodyAttributes))
        f.tv.textStorage!.setAttributedString(s)
        let rotors = f.tv.accessibilityCustomRotors()
        #expect(rotors.count == 1 && rotors.first?.type == .heading, "R1: one Headings rotor")
        let rotor = try #require(rotors.first)
        var labels: [String] = []
        var cur: NSRange? = nil
        while let r = search(rotor, from: cur) {
            labels.append(r.customLabel ?? "")
            #expect(f.tv.accessibilityString(for: r.targetRange) == r.customLabel, "targetRange is presented")
            cur = r.targetRange
            if labels.count > 10 { break }
        }
        #expect(labels == ["Chapter One", "First *part*", "Second", "Chapter Two", "Third"])
        var back: [String] = []
        cur = nil
        while let r = search(rotor, from: cur, next: false) { back.append(r.customLabel ?? ""); cur = r.targetRange; if back.count > 10 { break } }
        #expect(back == labels.reversed())
        #expect(search(rotor, from: nil, filter: "thi")?.customLabel == "Third", "type-ahead filter")
    }

    /// The manuscript as the app builds it (`rebuildStorage`), one scene per chapter — so R2's titles setting is the app's own.
    /// The text view and the coordinator that owns its presenter (the text view holds the presenter weakly).
    private struct Rebuilt { let tv: ManuscriptNSTextView; let owner: AnyObject }
    private func rebuilt(_ scenes: [String], titles: Bool, perChapter: Int = 1) -> Rebuilt {
        var infos: [SceneInfo] = []
        var segs: [SceneSegment] = []
        for (i, text) in scenes.enumerated() {
            let ch = i / perChapter
            infos.append(try! JSONDecoder().decode(SceneInfo.self, from: Data("""
                {"sceneID":"s\(i)","chapterID":"c\(ch)","title":"S","chapterTitle":"Chapter \(ch + 1)","slug":"s",
                 "metadataPath":"","contentPath":"","chapterMetadataPath":""}
                """.utf8)))
            segs.append(SceneSegment(id: "s\(i)", sceneID: "s\(i)", chapterID: "c\(ch)", metadataPath: "", contentPath: "", text: text))
        }
        let loader = ViewportSceneLoader(engine: ScriviEngine(), projectRootPath: "/tmp/ax-spike",
                                         appSupportRoot: "/tmp/ax-spike-support", projectID: "p", allScenes: infos)
        let session = ProjectSession(engine: ScriviEngine(), authorshipRef: nil, appSupportRoot: "/tmp/ax-spike-support", identityID: "")
        let view = ManuscriptTextView(loader: loader, env: AppEnvironment(), session: session, navigateToSceneID: .constant(nil),
                                      showChapterTitles: titles, typography: .default)
        let c = view.makeCoordinator()
        let tv = ManuscriptNSTextView(usingTextLayoutManager: true)
        tv.frame = NSRect(x: 0, y: 0, width: 900, height: 700)
        tv.textContentStorage?.delegate = c.presenter
        tv.textStorage?.delegate = c.presenter
        c.textView = tv
        c.rebuildStorage(tv, segments: segs)
        return Rebuilt(tv: tv, owner: c)
    }

    /// Every label, walked as VoiceOver builds its VO-U list (measured): `next` from `{0, 0}`, one call per item.
    private func list(_ view: Rebuilt, filter: String = "") throws -> [String] {
        let rotor = try #require(view.tv.accessibilityCustomRotors().first)
        var out: [String] = []
        var cur = NSRange(location: 0, length: 0)
        while let r = search(rotor, from: cur, filter: filter), out.count < 5_000 { out.append(r.customLabel ?? ""); cur = r.targetRange }
        return out
    }

    @Test("R2: chapter titles follow the page — listed when shown, absent when hidden; Markdown headings either way")
    func titlesFollowThePage() throws {
        let scenes = ["# Opening\n\nbody", "Plain body\n\n## Middle", "### Last\n\nend"]
        let on = try list(rebuilt(scenes, titles: true)), off = try list(rebuilt(scenes, titles: false))
        #expect(on == ["Chapter 1", "Opening", "Chapter 2", "Middle", "Chapter 3", "Last"])
        #expect(off == ["Opening", "Middle", "Last"],
                "titles OFF: the manuscript begins WITH a heading — VoiceOver's list (from {0, 0}) must still include it")
    }

    @Test("R3: next / previous from VoiceOver's reading position (a zero-length currentItem), not from the caret")
    func fromReadingPosition() throws {
        let r = rebuilt(["Before\n\n## Alpha\n\nmiddle text\n\n## Beta\n\nafter"], titles: true), tv = r.tv
        let rotor = try #require(tv.accessibilityCustomRotors().first)
        let presented = tv.accessibilityString(for: NSRange(location: 0, length: tv.accessibilityNumberOfCharacters())) ?? ""
        let mid = (presented as NSString).range(of: "middle").location
        tv.setSelectedRange(NSRange(location: tv.string.utf16.count, length: 0))      // the caret at the END — must not matter
        #expect(search(rotor, from: NSRange(location: mid, length: 0))?.customLabel == "Beta")
        #expect(search(rotor, from: NSRange(location: mid, length: 0), next: false)?.customLabel == "Alpha")
        #expect(search(rotor, from: nil)?.customLabel == "Chapter 1", "nil → the first item (the API)")
        #expect(search(rotor, from: nil, next: false)?.customLabel == "Beta", "nil → the last item (the API)")
        let beta = try #require(search(rotor, from: NSRange(location: mid, length: 0)))
        #expect(search(rotor, from: beta.targetRange) == nil, "past the last heading: none")
    }

    @Test("Type-ahead filter: case- and diacritic-insensitive, over the presented text")
    func filter() throws {
        let r = rebuilt(["## Élan vital\n\nx\n\n## Other \\*mark\\*"], titles: false)
        let elan = try list(r, filter: "ELAN"), mark = try list(r, filter: "*mark"), none = try list(r, filter: "zz")
        #expect(elan == ["Élan vital"])
        #expect(mark == ["Other *mark*"], "matched as PRESENTED (no escapes)")
        #expect(none.isEmpty)
    }

    @Test("Labels follow an edit — the cached outline is re-derived when the map is patched")
    func labelsFollowEdits() throws {
        let r = rebuilt(["## Alpha\n\nbody"], titles: false)
        #expect(try list(r) == ["Alpha"])
        let ts = try #require(r.tv.textStorage)
        ts.replaceCharacters(in: (ts.string as NSString).range(of: "Alpha"), with: NSAttributedString(string: "Omega"))
        let after = try list(r), filtered = try list(r, filter: "ome")
        #expect(after == ["Omega"] && filtered == ["Omega"])
    }

    /// VoiceOver re-walks the whole list each time the rotor opens and on every type-ahead letter (Plan 1, measured on dumas).
    @Test("Cost on 1.7 MB: VoiceOver's full list walk, unfiltered and filtered")
    func cost() throws {
        let para = #"Mr\. Smith said \*no\* \- the ship came in on the **evening** tide, her sails the colour of *old parchment* against a sky turning to brass\."#
        // dumas-shaped: 60 chapters × 20 scenes, a heading per scene → 1,260 items (dumas: ~1,200).
        let scenes = (0..<1_200).map { i in "## Part \(i + 1)\n\n" + (0..<10).map { _ in para }.joined(separator: "\n\n") }
        let r = rebuilt(scenes, titles: true, perChapter: 20)
        print("[SP-172 cost] storage length \(r.tv.textStorage!.length)")
        _ = r.tv.presentedMap.current()
        func ms(_ body: () throws -> Void) rethrows -> Double {
            let t0 = DispatchTime.now().uptimeNanoseconds; try body()
            return Double(DispatchTime.now().uptimeNanoseconds - t0) / 1e6
        }
        var n = 0, nf = 0
        let whole = try ms { n = try list(r).count }
        let filtered = try ms { nf = try list(r, filter: "part").count }
        print(String(format: "[SP-172 cost] list walk %d items %.1f ms · filtered walk %d items %.1f ms", n, whole, nf, filtered))
        #expect(n == 1_260 && nf == 1_200)
        #expect(whole < 250 && filtered < 250, "a rotor open or a type-ahead letter must not stall VoiceOver")
    }
}

#endif
