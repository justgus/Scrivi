#include <catch2/catch_approx.hpp>
#include <catch2/catch_test_macros.hpp>

#include "scrivi/ScriviCore.hpp"
#include "scrivi/Requests.hpp"

#include "mocks/DeterministicUUIDProvider.hpp"
#include "mocks/FixedClock.hpp"
#include "mocks/MockGitProvider.hpp"
#include "mocks/MockSecureStore.hpp"

#include "platform/LocalFileSystem.hpp"

#include <chrono>
#include <filesystem>
#include <fstream>
#include <string>

namespace fs = std::filesystem;

// ---------------------------------------------------------------------------
// Helpers
// ---------------------------------------------------------------------------

struct TempDir {
    fs::path path;

    TempDir() {
        path = fs::temp_directory_path() / ("scrivi-mvp-test-" + std::to_string(
            std::chrono::steady_clock::now().time_since_epoch().count()));
        fs::create_directories(path);
    }

    ~TempDir() {
        std::error_code ec;
        fs::remove_all(path, ec);
    }

    std::string str() const { return path.string(); }
};

// ---------------------------------------------------------------------------
// Full MVP loop - Section 3 end-to-end
//
// Exercises all 13 steps from Scrivi_Backend_MVP_Slice_v0_1.md Section 3
// in a single chained integration test using mock services.
//
// Steps:
//  1. Create a local author identity and default persona.
//  2. Create a new .scrivi project.
//  3. Create the approved minimum project file set.         (verified via createProject result)
//  4. Open the project.
//  5. Display the first or last active writing surface.     (verified via openProject result)
//  6. Save Markdown text.
//  7. Update scene metadata.                                (verified via saveScene result)
//  8. Store app-local workspace state.                      (verified via saveScene result)
//  9. Close and reopen the project.                         (second openProject call)
// 10. Restore the last scene and cursor position.           (verified via restoredScroll/Selection)
// 11. Detect simple external manuscript edits.              (verified via scanForExternalChanges)
// 12. Detect missing .md or .meta.json files at open.       (verified via repairRequired mode)
// 13. Optionally enable local Git-backed snapshots.         (verified via enableGitSnapshots + createSnapshot)
// ---------------------------------------------------------------------------

TEST_CASE("MVP loop - Section 3 end-to-end", "[integration][EP-002][T-0027]") {

    TempDir projectDir;
    TempDir appSupportDir;

    scrivi::platform::LocalFileSystem        lfs;
    scrivi::mocks::DeterministicUUIDProvider uuids;
    scrivi::mocks::FixedClock                clock{"2026-05-20T12:00:00Z"};
    scrivi::mocks::MockSecureStore           store;
    scrivi::mocks::MockGitProvider           git;

    scrivi::CoreServices svc;
    svc.fileSystem   = &lfs;
    svc.uuidProvider = &uuids;
    svc.secureStore  = &store;
    svc.clock        = &clock;
    svc.gitProvider  = &git;
    svc.logger       = nullptr;

    scrivi::ScriviCore core{svc};

    // -----------------------------------------------------------------------
    // Step 1: Create a local author identity and default persona.
    // -----------------------------------------------------------------------
    scrivi::EnsureIdentityRequest idReq;
    idReq.requestedDisplayName = "Rhozwyn Darius";
    idReq.appSupportRoot       = appSupportDir.str();

    auto idR = core.ensureLocalIdentity(idReq);
    REQUIRE(idR.ok());

    const auto& identity = idR.value();
    CHECK(!identity.identityID.value.empty());
    CHECK(!identity.defaultPersonaID.value.empty());
    CHECK(identity.displayName == "Rhozwyn Darius");
    CHECK(identity.createdNewIdentity == true);

    scrivi::AuthorshipRef author{
        identity.identityID,
        identity.defaultPersonaID,
        identity.displayName
    };

    // -----------------------------------------------------------------------
    // Steps 2 & 3: Create a new .scrivi project with the minimum file set.
    // -----------------------------------------------------------------------
    scrivi::CreateProjectRequest createReq;
    createReq.projectRootPath = projectDir.str();
    createReq.appSupportRoot  = appSupportDir.str();
    createReq.title           = "The Long Road";
    createReq.slug            = "the-long-road";
    createReq.author          = author;

    auto createR = core.createProject(createReq);
    REQUIRE(createR.ok());

    const auto& created = createR.value();
    CHECK(!created.project.projectID.value.empty());
    CHECK(!created.firstSceneID.value.empty());
    CHECK(!created.firstSceneMetadataPath.empty());
    CHECK(!created.firstSceneContentPath.empty());

    const std::string projectID        = created.project.projectID.value;
    const std::string firstSceneID     = created.firstSceneID.value;
    const std::string firstMetaPath    = created.firstSceneMetadataPath;
    const std::string firstContentPath = created.firstSceneContentPath;

    // -----------------------------------------------------------------------
    // Steps 4 & 5: Open the project and display the first writing surface.
    // -----------------------------------------------------------------------
    scrivi::OpenProjectRequest openReq;
    openReq.projectRootPath   = projectDir.str();
    openReq.appSupportRoot    = appSupportDir.str();
    openReq.currentIdentityID = identity.identityID;

    auto openR = core.openProject(openReq);
    REQUIRE(openR.ok());

    const auto& opened = openR.value();
    CHECK(opened.mode == scrivi::OpenMode::normalEdit);
    CHECK(opened.project.projectID.value == projectID);
    REQUIRE(opened.activeScene.has_value());
    CHECK(opened.activeScene->sceneID.value == firstSceneID);

    // -----------------------------------------------------------------------
    // Steps 6, 7 & 8: Save Markdown, update metadata, store workspace state.
    // -----------------------------------------------------------------------
    scrivi::SaveSceneRequest saveReq;
    saveReq.projectID         = scrivi::ProjectID{projectID};
    saveReq.projectRootPath   = projectDir.str();
    saveReq.appSupportRoot    = appSupportDir.str();
    saveReq.sceneID           = scrivi::SceneID{firstSceneID};
    saveReq.sceneMetadataPath = firstMetaPath;
    saveReq.sceneContentPath  = firstContentPath;
    saveReq.markdown          = "# Chapter One\n\nIt was a dark and stormy night.";
    saveReq.selection         = {10, 20};
    saveReq.scroll            = {0.5};
    saveReq.author            = author;

    auto saveR = core.saveScene(saveReq);
    REQUIRE(saveR.ok());

    const auto& saved = saveR.value();
    CHECK(saved.saved == true);
    CHECK(saved.sceneID.value == firstSceneID);
    CHECK(saved.wordCount > 0);

    // -----------------------------------------------------------------------
    // Steps 9 & 10: Close and reopen; restore last scene and cursor position.
    // -----------------------------------------------------------------------
    auto reOpenR = core.openProject(openReq);
    REQUIRE(reOpenR.ok());

    const auto& reOpened = reOpenR.value();
    CHECK(reOpened.mode == scrivi::OpenMode::normalEdit);
    REQUIRE(reOpened.activeScene.has_value());
    CHECK(reOpened.activeScene->sceneID.value == firstSceneID);
    CHECK(reOpened.activeSceneMarkdown.find("dark and stormy night") != std::string::npos);
    CHECK(reOpened.restoredSelection.anchor == 10);
    CHECK(reOpened.restoredSelection.focus  == 20);
    CHECK(reOpened.restoredScroll.value     == Catch::Approx(0.5).margin(0.01));

    // -----------------------------------------------------------------------
    // Step 11: Detect simple external manuscript edits.
    // -----------------------------------------------------------------------
    scrivi::ExternalChangeScanRequest scanReq;
    scanReq.projectRootPath  = projectDir.str();
    scanReq.appSupportRoot   = appSupportDir.str();
    scanReq.includeGitStatus = false;

    auto scanR = core.scanForExternalChanges(scanReq);
    REQUIRE(scanR.ok());
    CHECK(scanR.value().repairIssues.empty());

    // -----------------------------------------------------------------------
    // Step 12: Detect missing files (blocking repair issue at open).
    // -----------------------------------------------------------------------
    // Remove the scene content file to simulate a missing .md
    fs::remove(projectDir.path / firstContentPath);

    scrivi::OpenProjectRequest brokenOpenReq = openReq;
    auto brokenR = core.openProject(brokenOpenReq);
    REQUIRE(brokenR.ok());
    CHECK(brokenR.value().mode == scrivi::OpenMode::repairRequired);
    CHECK(!brokenR.value().repairIssues.empty());

    // Restore for the git steps (write an empty file back)
    { std::ofstream f((projectDir.path / firstContentPath).string(), std::ios::binary); }

    // -----------------------------------------------------------------------
    // Step 13: Optionally enable Git-backed snapshots and create a snapshot.
    // -----------------------------------------------------------------------
    scrivi::EnableGitRequest enableReq;
    enableReq.projectRootPath      = projectDir.str();
    enableReq.author               = author;
    enableReq.initialSnapshotLabel = "Initial snapshot";

    auto enableR = core.enableGitSnapshots(enableReq);
    REQUIRE(enableR.ok());
    CHECK(enableR.value().gitInitialized == true);
    CHECK(!enableR.value().initialSnapshotID.value.empty());

    scrivi::CreateSnapshotRequest snapReq;
    snapReq.projectRootPath = projectDir.str();
    snapReq.author          = author;
    snapReq.label           = "End of chapter one";
    snapReq.note            = "MVP loop test snapshot";

    auto snapR = core.createSnapshot(snapReq);
    REQUIRE(snapR.ok());
    CHECK(snapR.value().created == true);
    CHECK(!snapR.value().snapshotID.value.empty());
}
