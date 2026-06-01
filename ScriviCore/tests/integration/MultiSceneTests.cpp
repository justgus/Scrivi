// MultiSceneTests.cpp
// Integration tests for T-0059 (scene list in openProject) and T-0060 (openScene).

#include <catch2/catch_test_macros.hpp>

#include "scrivi/ScriviCore.hpp"
#include "scrivi/Requests.hpp"

#include "mocks/DeterministicUUIDProvider.hpp"
#include "mocks/FixedClock.hpp"
#include "mocks/MockGitProvider.hpp"
#include "mocks/MockSecureStore.hpp"

#include "platform/LocalFileSystem.hpp"
#include "schemas/ChapterMetaJson.hpp"
#include "schemas/SceneMetaJson.hpp"
#include "util/PathUtils.hpp"

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
        path = fs::temp_directory_path() / ("scrivi-multiscene-" + std::to_string(
            std::chrono::steady_clock::now().time_since_epoch().count()));
        fs::create_directories(path);
    }

    ~TempDir() {
        std::error_code ec;
        fs::remove_all(path, ec);
    }

    std::string str() const { return path.string(); }
    std::string sub(const std::string& rel) const { return (path / rel).string(); }
    bool exists(const std::string& rel) const { return fs::exists(path / rel); }
};

static scrivi::CoreServices makeServices(
    scrivi::platform::LocalFileSystem&        lfs,
    scrivi::mocks::DeterministicUUIDProvider& uuids,
    scrivi::mocks::FixedClock&                clock,
    scrivi::mocks::MockSecureStore&           store,
    scrivi::mocks::MockGitProvider&           git)
{
    scrivi::CoreServices s;
    s.fileSystem   = &lfs;
    s.uuidProvider = &uuids;
    s.clock        = &clock;
    s.secureStore  = &store;
    s.gitProvider  = &git;
    return s;
}

// Creates a project with one scene, then writes a second scene into the same
// chapter by directly authoring the schema files. Returns the second sceneID.
static scrivi::SceneID addSecondScene(
    const std::string& projectDir,
    const scrivi::CreateProjectResult& created)
{
    const std::string chapterDir =
        projectDir + "/manuscript/chapter-001";
    const std::string scene2ID        = "scene-002";
    const std::string scene2Slug      = "002-second-scene";
    const std::string scene2MetaRel   = "manuscript/chapter-001/002-second-scene.meta.json";
    const std::string scene2ContentRel= "manuscript/chapter-001/002-second-scene.md";

    // Write scene meta
    scrivi::schemas::SceneMetaData meta2;
    meta2.sceneID.value         = scene2ID;
    meta2.title                 = "Second Scene";
    meta2.slug                  = scene2Slug;
    meta2.status                = "draft";
    meta2.createdAt             = "2026-06-01T00:00:00Z";
    meta2.createdByIdentityID   = "identity-001";
    meta2.createdByPersonaID    = "persona-001";
    meta2.createdByDisplayName  = "Test Author";
    meta2.modifiedAt            = "2026-06-01T00:00:00Z";
    meta2.modifiedByIdentityID  = "identity-001";
    meta2.modifiedByPersonaID   = "persona-001";
    meta2.modifiedByDisplayName = "Test Author";
    meta2.contentPath           = scene2ContentRel;

    {
        std::ofstream f(projectDir + "/manuscript/chapter-001/002-second-scene.meta.json",
                        std::ios::binary);
        f << scrivi::schemas::serializeSceneMeta(meta2);
    }

    // Write scene content
    {
        std::ofstream f(projectDir + "/manuscript/chapter-001/002-second-scene.md",
                        std::ios::binary);
        f << "# Second Scene\n\nContent of the second scene.";
    }

    // Rewrite chapter meta to include both scenes
    auto chapterMetaPath = projectDir + "/manuscript/chapter-001/chapter.meta.json";
    std::ifstream fin(chapterMetaPath);
    std::string chapterJson((std::istreambuf_iterator<char>(fin)),
                             std::istreambuf_iterator<char>());
    auto parsedR = scrivi::schemas::parseChapterMeta(chapterJson);
    REQUIRE(parsedR.ok());
    auto chapter = parsedR.value();

    scrivi::schemas::SceneRef ref2;
    ref2.sceneID.value   = scene2ID;
    ref2.metadataPath    = scene2MetaRel;
    chapter.scenes.push_back(ref2);

    {
        std::ofstream f(chapterMetaPath, std::ios::binary);
        f << scrivi::schemas::serializeChapterMeta(chapter);
    }

    return scrivi::SceneID{scene2ID};
}

// ---------------------------------------------------------------------------
// T-0059: openProject returns full scene list
// ---------------------------------------------------------------------------

TEST_CASE("openProject - scenes list contains all scenes in manuscript order",
          "[integration][T-0059]")
{
    TempDir projectDir;
    TempDir appSupportDir;

    scrivi::platform::LocalFileSystem        lfs;
    scrivi::mocks::DeterministicUUIDProvider uuids;
    scrivi::mocks::FixedClock                clock{"2026-06-01T00:00:00Z"};
    scrivi::mocks::MockSecureStore           store;
    scrivi::mocks::MockGitProvider           git;

    auto services = makeServices(lfs, uuids, clock, store, git);
    scrivi::ScriviCore core{services};

    scrivi::CreateProjectRequest req;
    req.projectRootPath = projectDir.str();
    req.appSupportRoot  = appSupportDir.str();
    req.title           = "Multi-Scene Novel";
    req.slug            = "multi-scene-novel";
    req.author          = {
        scrivi::IdentityID{"identity-001"},
        scrivi::PersonaID {"persona-001"},
        "Test Author"
    };
    auto created = core.createProject(req);
    REQUIRE(created.ok());

    auto scene2ID = addSecondScene(projectDir.str(), created.value());

    scrivi::OpenProjectRequest openReq;
    openReq.projectRootPath = projectDir.str();
    openReq.appSupportRoot  = appSupportDir.str();

    auto result = core.openProject(openReq);
    REQUIRE(result.ok());

    auto& r = result.value();
    CHECK(r.mode == scrivi::OpenMode::normalEdit);
    REQUIRE(r.scenes.size() == 2);

    // First scene comes from createProject (initial scene)
    CHECK(r.scenes[0].sceneID.value == created.value().firstSceneID.value);
    CHECK(r.scenes[0].title         == "Opening Scene");

    // Second scene is the one we appended
    CHECK(r.scenes[1].sceneID.value == scene2ID.value);
    CHECK(r.scenes[1].title         == "Second Scene");
}

TEST_CASE("openProject - scenes list has one entry for single-scene project",
          "[integration][T-0059]")
{
    TempDir projectDir;
    TempDir appSupportDir;

    scrivi::platform::LocalFileSystem        lfs;
    scrivi::mocks::DeterministicUUIDProvider uuids;
    scrivi::mocks::FixedClock                clock{"2026-06-01T00:00:00Z"};
    scrivi::mocks::MockSecureStore           store;
    scrivi::mocks::MockGitProvider           git;

    auto services = makeServices(lfs, uuids, clock, store, git);
    scrivi::ScriviCore core{services};

    scrivi::CreateProjectRequest req;
    req.projectRootPath = projectDir.str();
    req.appSupportRoot  = appSupportDir.str();
    req.title           = "Single Scene Novel";
    req.slug            = "single-scene-novel";
    req.author          = {
        scrivi::IdentityID{"identity-001"},
        scrivi::PersonaID {"persona-001"},
        "Test Author"
    };
    auto created = core.createProject(req);
    REQUIRE(created.ok());

    scrivi::OpenProjectRequest openReq;
    openReq.projectRootPath = projectDir.str();
    openReq.appSupportRoot  = appSupportDir.str();

    auto result = core.openProject(openReq);
    REQUIRE(result.ok());

    CHECK(result.value().scenes.size() == 1);
    CHECK(result.value().scenes[0].sceneID.value == created.value().firstSceneID.value);
}

// ---------------------------------------------------------------------------
// T-0060: openScene switches active scene
// ---------------------------------------------------------------------------

TEST_CASE("openScene - loads correct content for requested scene",
          "[integration][T-0060]")
{
    TempDir projectDir;
    TempDir appSupportDir;

    scrivi::platform::LocalFileSystem        lfs;
    scrivi::mocks::DeterministicUUIDProvider uuids;
    scrivi::mocks::FixedClock                clock{"2026-06-01T00:00:00Z"};
    scrivi::mocks::MockSecureStore           store;
    scrivi::mocks::MockGitProvider           git;

    auto services = makeServices(lfs, uuids, clock, store, git);
    scrivi::ScriviCore core{services};

    scrivi::CreateProjectRequest req;
    req.projectRootPath = projectDir.str();
    req.appSupportRoot  = appSupportDir.str();
    req.title           = "Two Scene Novel";
    req.slug            = "two-scene-novel";
    req.author          = {
        scrivi::IdentityID{"identity-001"},
        scrivi::PersonaID {"persona-001"},
        "Test Author"
    };
    auto created = core.createProject(req);
    REQUIRE(created.ok());

    auto scene2ID = addSecondScene(projectDir.str(), created.value());

    scrivi::OpenSceneRequest openReq;
    openReq.projectRootPath = projectDir.str();
    openReq.appSupportRoot  = appSupportDir.str();
    openReq.projectID       = created.value().project.projectID;
    openReq.sceneID         = scene2ID;

    auto result = core.openScene(openReq);
    REQUIRE(result.ok());

    auto& r = result.value();
    CHECK(r.scene.sceneID.value == scene2ID.value);
    CHECK(r.scene.title         == "Second Scene");
    CHECK(r.markdown            == "# Second Scene\n\nContent of the second scene.");
}

TEST_CASE("openScene - updates workspace state to newly opened scene",
          "[integration][T-0060]")
{
    TempDir projectDir;
    TempDir appSupportDir;

    scrivi::platform::LocalFileSystem        lfs;
    scrivi::mocks::DeterministicUUIDProvider uuids;
    scrivi::mocks::FixedClock                clock{"2026-06-01T00:00:00Z"};
    scrivi::mocks::MockSecureStore           store;
    scrivi::mocks::MockGitProvider           git;

    auto services = makeServices(lfs, uuids, clock, store, git);
    scrivi::ScriviCore core{services};

    scrivi::CreateProjectRequest req;
    req.projectRootPath = projectDir.str();
    req.appSupportRoot  = appSupportDir.str();
    req.title           = "Two Scene Novel";
    req.slug            = "two-scene-novel";
    req.author          = {
        scrivi::IdentityID{"identity-001"},
        scrivi::PersonaID {"persona-001"},
        "Test Author"
    };
    auto created = core.createProject(req);
    REQUIRE(created.ok());

    auto scene2ID = addSecondScene(projectDir.str(), created.value());

    // Open scene 2
    scrivi::OpenSceneRequest openReq;
    openReq.projectRootPath = projectDir.str();
    openReq.appSupportRoot  = appSupportDir.str();
    openReq.projectID       = created.value().project.projectID;
    openReq.sceneID         = scene2ID;
    REQUIRE(core.openScene(openReq).ok());

    // Re-open the project - active scene should now be scene 2
    scrivi::OpenProjectRequest projReq;
    projReq.projectRootPath = projectDir.str();
    projReq.appSupportRoot  = appSupportDir.str();

    auto result = core.openProject(projReq);
    REQUIRE(result.ok());

    REQUIRE(result.value().activeScene.has_value());
    CHECK(result.value().activeScene->sceneID.value == scene2ID.value);
    CHECK(result.value().activeSceneMarkdown ==
          "# Second Scene\n\nContent of the second scene.");
}

TEST_CASE("openScene - returns error for unknown sceneID",
          "[integration][T-0060]")
{
    TempDir projectDir;
    TempDir appSupportDir;

    scrivi::platform::LocalFileSystem        lfs;
    scrivi::mocks::DeterministicUUIDProvider uuids;
    scrivi::mocks::FixedClock                clock{"2026-06-01T00:00:00Z"};
    scrivi::mocks::MockSecureStore           store;
    scrivi::mocks::MockGitProvider           git;

    auto services = makeServices(lfs, uuids, clock, store, git);
    scrivi::ScriviCore core{services};

    scrivi::CreateProjectRequest req;
    req.projectRootPath = projectDir.str();
    req.appSupportRoot  = appSupportDir.str();
    req.title           = "Novel";
    req.slug            = "novel";
    req.author          = {
        scrivi::IdentityID{"identity-001"},
        scrivi::PersonaID {"persona-001"},
        "Test Author"
    };
    REQUIRE(core.createProject(req).ok());

    scrivi::OpenSceneRequest openReq;
    openReq.projectRootPath = projectDir.str();
    openReq.appSupportRoot  = appSupportDir.str();
    openReq.projectID       = scrivi::ProjectID{"proj-001"};
    openReq.sceneID         = scrivi::SceneID{"scene-does-not-exist"};

    auto result = core.openScene(openReq);
    CHECK_FALSE(result.ok());
    CHECK(result.error().code == scrivi::ErrorCode::invalidArgument);
}
