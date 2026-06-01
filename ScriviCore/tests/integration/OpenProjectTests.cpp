#include <catch2/catch_approx.hpp>
#include <catch2/catch_test_macros.hpp>

#include "scrivi/ScriviCore.hpp"
#include "scrivi/Requests.hpp"

#include "mocks/DeterministicUUIDProvider.hpp"
#include "mocks/FixedClock.hpp"
#include "mocks/MockGitProvider.hpp"
#include "mocks/MockSecureStore.hpp"

#include "platform/LocalFileSystem.hpp"

#include <filesystem>
#include <fstream>
#include <string>

namespace fs = std::filesystem;

// ---------------------------------------------------------------------------
// Helpers shared with CreateProjectTests (duplicated for isolation)
// ---------------------------------------------------------------------------

struct TempDir {
    fs::path path;

    TempDir() {
        path = fs::temp_directory_path() / ("scrivi-open-test-" + std::to_string(
            std::chrono::steady_clock::now().time_since_epoch().count()));
        fs::create_directories(path);
    }

    ~TempDir() {
        std::error_code ec;
        fs::remove_all(path, ec);
    }

    std::string str() const { return path.string(); }
    std::string sub(const std::string& rel) const { return (path / rel).string(); }
};

static scrivi::CoreServices makeServices(
    scrivi::platform::LocalFileSystem&      lfs,
    scrivi::mocks::DeterministicUUIDProvider& uuids,
    scrivi::mocks::FixedClock&              clock,
    scrivi::mocks::MockSecureStore&         store,
    scrivi::mocks::MockGitProvider&         git)
{
    scrivi::CoreServices s;
    s.fileSystem   = &lfs;
    s.uuidProvider = &uuids;
    s.clock        = &clock;
    s.secureStore  = &store;
    s.gitProvider  = &git;
    return s;
}

// Create a project and return the result; asserts ok().
static scrivi::CreateProjectResult createTestProject(
    scrivi::ScriviCore& core,
    const std::string& projectDir,
    const std::string& appSupportDir)
{
    scrivi::CreateProjectRequest req;
    req.projectRootPath = projectDir;
    req.appSupportRoot  = appSupportDir;
    req.title           = "Test Novel";
    req.slug            = "test-novel";
    req.author          = {
        scrivi::IdentityID{"identity-001"},
        scrivi::PersonaID {"persona-001"},
        "Test Author"
    };
    auto result = core.createProject(req);
    REQUIRE(result.ok());
    return result.value();
}

// ---------------------------------------------------------------------------
// Tests
// ---------------------------------------------------------------------------

TEST_CASE("openProject - opens valid project in normalEdit mode", "[integration][T-0007]") {
    TempDir projectDir;
    TempDir appSupportDir;

    scrivi::platform::LocalFileSystem        lfs;
    scrivi::mocks::DeterministicUUIDProvider uuids;
    scrivi::mocks::FixedClock                clock{"2026-05-20T00:00:00Z"};
    scrivi::mocks::MockSecureStore           store;
    scrivi::mocks::MockGitProvider           git;

    auto services = makeServices(lfs, uuids, clock, store, git);
    scrivi::ScriviCore core{services};

    auto created = createTestProject(core, projectDir.str(), appSupportDir.str());

    scrivi::OpenProjectRequest req;
    req.projectRootPath = projectDir.str();
    req.appSupportRoot  = appSupportDir.str();

    auto result = core.openProject(req);
    REQUIRE(result.ok());

    auto& r = result.value();
    CHECK(r.mode == scrivi::OpenMode::normalEdit);
    CHECK(r.project.projectID.value == created.project.projectID.value);
    CHECK(r.project.title           == "Test Novel");
    CHECK(r.repairIssues.empty());
    REQUIRE(r.activeScene.has_value());
    CHECK(r.activeScene->sceneID.value == created.firstSceneID.value);
}

TEST_CASE("openProject - restores workspace state (last scene, selection, scroll)", "[integration][T-0007]") {
    TempDir projectDir;
    TempDir appSupportDir;

    scrivi::platform::LocalFileSystem        lfs;
    scrivi::mocks::DeterministicUUIDProvider uuids;
    scrivi::mocks::FixedClock                clock{"2026-05-20T00:00:00Z"};
    scrivi::mocks::MockSecureStore           store;
    scrivi::mocks::MockGitProvider           git;

    auto services = makeServices(lfs, uuids, clock, store, git);
    scrivi::ScriviCore core{services};

    auto created = createTestProject(core, projectDir.str(), appSupportDir.str());

    // Save a scene to write a known workspace state with selection/scroll
    scrivi::SaveSceneRequest saveReq;
    saveReq.projectID        = created.project.projectID;
    saveReq.projectRootPath  = projectDir.str();
    saveReq.appSupportRoot   = appSupportDir.str();
    saveReq.sceneID          = created.firstSceneID;
    saveReq.sceneMetadataPath = created.firstSceneMetadataPath;
    saveReq.sceneContentPath  = created.firstSceneContentPath;
    saveReq.markdown         = "Hello world.";
    saveReq.selection        = {5, 10};
    saveReq.scroll           = {0.42};
    saveReq.author           = {
        scrivi::IdentityID{"identity-001"},
        scrivi::PersonaID {"persona-001"},
        "Test Author"
    };

    auto saveResult = core.saveScene(saveReq);
    REQUIRE(saveResult.ok());

    // Now open - workspace state should be restored
    scrivi::OpenProjectRequest openReq;
    openReq.projectRootPath = projectDir.str();
    openReq.appSupportRoot  = appSupportDir.str();

    auto result = core.openProject(openReq);
    REQUIRE(result.ok());

    auto& r = result.value();
    CHECK(r.mode == scrivi::OpenMode::normalEdit);
    REQUIRE(r.workspaceState.has_value());
    REQUIRE(r.workspaceState->lastWritingSurface.has_value());
    CHECK(r.workspaceState->lastWritingSurface->sceneID.value == created.firstSceneID.value);
    CHECK(r.restoredSelection.anchor == 5);
    CHECK(r.restoredSelection.focus  == 10);
    CHECK(r.restoredScroll.value     == Catch::Approx(0.42).margin(0.01));
    CHECK(r.activeSceneMarkdown      == "Hello world.");
}

TEST_CASE("openProject - falls back to first scene when no workspace state", "[integration][T-0007]") {
    TempDir projectDir;
    TempDir appSupportDirCreate;
    TempDir appSupportDirOpen;  // fresh dir - no workspace state

    scrivi::platform::LocalFileSystem        lfs;
    scrivi::mocks::DeterministicUUIDProvider uuids;
    scrivi::mocks::FixedClock                clock{"2026-05-20T00:00:00Z"};
    scrivi::mocks::MockSecureStore           store;
    scrivi::mocks::MockGitProvider           git;

    auto services = makeServices(lfs, uuids, clock, store, git);
    scrivi::ScriviCore core{services};

    auto created = createTestProject(core, projectDir.str(), appSupportDirCreate.str());

    // Open with a different appSupportRoot (no workspace state exists there)
    scrivi::OpenProjectRequest openReq;
    openReq.projectRootPath = projectDir.str();
    openReq.appSupportRoot  = appSupportDirOpen.str();

    auto result = core.openProject(openReq);
    REQUIRE(result.ok());

    auto& r = result.value();
    CHECK(r.mode == scrivi::OpenMode::normalEdit);
    CHECK_FALSE(r.workspaceState.has_value());
    REQUIRE(r.activeScene.has_value());
    // Falls back to first scene
    CHECK(r.activeScene->sceneID.value == created.firstSceneID.value);
    // Default selection and scroll are zero
    CHECK(r.restoredSelection.anchor == 0);
    CHECK(r.restoredSelection.focus  == 0);
    CHECK(r.restoredScroll.value     == Catch::Approx(0.0));
}

TEST_CASE("openProject - returns repairRequired when scene .meta.json is missing", "[integration][T-0007]") {
    TempDir projectDir;
    TempDir appSupportDir;

    scrivi::platform::LocalFileSystem        lfs;
    scrivi::mocks::DeterministicUUIDProvider uuids;
    scrivi::mocks::FixedClock                clock{"2026-05-20T00:00:00Z"};
    scrivi::mocks::MockSecureStore           store;
    scrivi::mocks::MockGitProvider           git;

    auto services = makeServices(lfs, uuids, clock, store, git);
    scrivi::ScriviCore core{services};

    auto created = createTestProject(core, projectDir.str(), appSupportDir.str());

    // Delete the scene meta file to simulate missing metadata
    fs::remove(projectDir.sub("manuscript/chapter-001/001-opening-scene.meta.json"));

    scrivi::OpenProjectRequest openReq;
    openReq.projectRootPath = projectDir.str();
    openReq.appSupportRoot  = appSupportDir.str();

    auto result = core.openProject(openReq);
    REQUIRE(result.ok());

    auto& r = result.value();
    CHECK(r.mode == scrivi::OpenMode::repairRequired);
    REQUIRE_FALSE(r.repairIssues.empty());
    CHECK(r.repairIssues[0].category == scrivi::RepairCategory::missingMetadata);
}

TEST_CASE("openProject - returns repairRequired when scene .md content is missing", "[integration][T-0007]") {
    TempDir projectDir;
    TempDir appSupportDir;

    scrivi::platform::LocalFileSystem        lfs;
    scrivi::mocks::DeterministicUUIDProvider uuids;
    scrivi::mocks::FixedClock                clock{"2026-05-20T00:00:00Z"};
    scrivi::mocks::MockSecureStore           store;
    scrivi::mocks::MockGitProvider           git;

    auto services = makeServices(lfs, uuids, clock, store, git);
    scrivi::ScriviCore core{services};

    auto created = createTestProject(core, projectDir.str(), appSupportDir.str());

    // Delete the scene content file to simulate missing content
    fs::remove(projectDir.sub("manuscript/chapter-001/001-opening-scene.md"));

    scrivi::OpenProjectRequest openReq;
    openReq.projectRootPath = projectDir.str();
    openReq.appSupportRoot  = appSupportDir.str();

    auto result = core.openProject(openReq);
    REQUIRE(result.ok());

    auto& r = result.value();
    CHECK(r.mode == scrivi::OpenMode::repairRequired);
    REQUIRE_FALSE(r.repairIssues.empty());
    CHECK(r.repairIssues[0].category == scrivi::RepairCategory::missingContent);
}

TEST_CASE("openProject - returns repairRequired when scene .meta.json is corrupt", "[integration][T-0007]") {
    TempDir projectDir;
    TempDir appSupportDir;

    scrivi::platform::LocalFileSystem        lfs;
    scrivi::mocks::DeterministicUUIDProvider uuids;
    scrivi::mocks::FixedClock                clock{"2026-05-20T00:00:00Z"};
    scrivi::mocks::MockSecureStore           store;
    scrivi::mocks::MockGitProvider           git;

    auto services = makeServices(lfs, uuids, clock, store, git);
    scrivi::ScriviCore core{services};

    auto created = createTestProject(core, projectDir.str(), appSupportDir.str());

    // Overwrite scene meta with garbage
    auto metaPath = projectDir.sub("manuscript/chapter-001/001-opening-scene.meta.json");
    {
        std::ofstream f(metaPath, std::ios::binary);
        f << "not valid json {{{";
    }

    scrivi::OpenProjectRequest openReq;
    openReq.projectRootPath = projectDir.str();
    openReq.appSupportRoot  = appSupportDir.str();

    auto result = core.openProject(openReq);
    REQUIRE(result.ok());

    auto& r = result.value();
    CHECK(r.mode == scrivi::OpenMode::repairRequired);
    REQUIRE_FALSE(r.repairIssues.empty());
    CHECK(r.repairIssues[0].category == scrivi::RepairCategory::corruptMetadata);
}

TEST_CASE("openProject - active scene markdown is returned correctly", "[integration][T-0007]") {
    TempDir projectDir;
    TempDir appSupportDir;

    scrivi::platform::LocalFileSystem        lfs;
    scrivi::mocks::DeterministicUUIDProvider uuids;
    scrivi::mocks::FixedClock                clock{"2026-05-20T00:00:00Z"};
    scrivi::mocks::MockSecureStore           store;
    scrivi::mocks::MockGitProvider           git;

    auto services = makeServices(lfs, uuids, clock, store, git);
    scrivi::ScriviCore core{services};

    auto created = createTestProject(core, projectDir.str(), appSupportDir.str());

    // Write known content directly to the scene file
    auto contentPath = projectDir.sub("manuscript/chapter-001/001-opening-scene.md");
    {
        std::ofstream f(contentPath, std::ios::binary);
        f << "# The Beginning\n\nIt was a dark and stormy night.";
    }

    scrivi::OpenProjectRequest openReq;
    openReq.projectRootPath = projectDir.str();
    openReq.appSupportRoot  = appSupportDir.str();

    auto result = core.openProject(openReq);
    REQUIRE(result.ok());

    auto& r = result.value();
    CHECK(r.mode == scrivi::OpenMode::normalEdit);
    CHECK(r.activeSceneMarkdown == "# The Beginning\n\nIt was a dark and stormy night.");
}
