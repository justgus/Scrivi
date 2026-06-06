// DeleteSceneTests.cpp
// Integration tests for T-0097/T-0098: deleteScene and deleteChapter (EP-010 SP-027).

#include <catch2/catch_test_macros.hpp>

#include "scrivi/ScriviCore.hpp"
#include "scrivi/Requests.hpp"

#include "mocks/DeterministicUUIDProvider.hpp"
#include "mocks/FixedClock.hpp"
#include "mocks/MockGitProvider.hpp"
#include "mocks/MockSecureStore.hpp"

#include "platform/LocalFileSystem.hpp"
#include "manuscript/ManuscriptOrderResolver.hpp"
#include "schemas/ManuscriptMetaJson.hpp"
#include "util/PathUtils.hpp"

#include <chrono>
#include <filesystem>
#include <string>

namespace fs = std::filesystem;

// ---------------------------------------------------------------------------
// Helpers
// ---------------------------------------------------------------------------

struct TempDir {
    fs::path path;

    TempDir() {
        path = fs::temp_directory_path() / ("scrivi-delete-" + std::to_string(
            std::chrono::steady_clock::now().time_since_epoch().count()));
        fs::create_directories(path);
    }

    ~TempDir() {
        std::error_code ec;
        fs::remove_all(path, ec);
    }

    std::string str() const { return path.string(); }
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

static scrivi::AuthorshipRef testAuthor()
{
    return {
        scrivi::IdentityID{"identity-001"},
        scrivi::PersonaID {"persona-001"},
        "Test Author"
    };
}

// Creates a project, adds a second scene, and returns the core + IDs.
struct TwoSceneProject {
    TempDir projectDir;
    TempDir appSupportDir;
    scrivi::platform::LocalFileSystem        lfs;
    scrivi::mocks::DeterministicUUIDProvider uuids;
    scrivi::mocks::FixedClock                clock{"2026-06-04T00:00:00Z"};
    scrivi::mocks::MockSecureStore           store;
    scrivi::mocks::MockGitProvider           git;
    scrivi::CoreServices                     services;
    scrivi::ScriviCore                       core;

    scrivi::ProjectID  projectID;
    scrivi::ChapterID  chapterID;
    scrivi::SceneID    scene1ID;
    scrivi::SceneID    scene2ID;

    TwoSceneProject()
        : services(makeServices(lfs, uuids, clock, store, git))
        , core(services)
    {
        scrivi::CreateProjectRequest req;
        req.projectRootPath = projectDir.str();
        req.appSupportRoot  = appSupportDir.str();
        req.title           = "Test Novel";
        req.slug            = "test-novel";
        req.author          = testAuthor();
        auto created = core.createProject(req);
        REQUIRE(created.ok());

        projectID = created.value().project.projectID;
        chapterID = created.value().firstChapterID;
        scene1ID  = created.value().firstSceneID;

        scrivi::CreateSceneRequest s2Req;
        s2Req.projectRootPath = projectDir.str();
        s2Req.appSupportRoot  = appSupportDir.str();
        s2Req.projectID       = projectID;
        s2Req.chapterID       = chapterID;
        s2Req.afterSceneID    = scene1ID;
        s2Req.author          = testAuthor();
        auto s2 = core.createScene(s2Req);
        REQUIRE(s2.ok());
        scene2ID = s2.value().sceneID;
    }
};

// ---------------------------------------------------------------------------
// deleteScene tests
// ---------------------------------------------------------------------------

TEST_CASE("deleteScene - removes scene from chapter index and disk",
          "[integration][T-0097]")
{
    TwoSceneProject p;

    // Resolve order before delete: should be [scene1, scene2]
    scrivi::manuscript::ManuscriptOrderResolver resolver{p.services};
    auto before = resolver.resolve(p.projectDir.str());
    REQUIRE(before.ok());
    REQUIRE(before.value().size() == 2);

    // Delete scene2
    scrivi::DeleteSceneRequest delReq;
    delReq.projectRootPath = p.projectDir.str();
    delReq.sceneID         = p.scene2ID;

    auto result = p.core.deleteScene(delReq);
    REQUIRE(result.ok());
    CHECK(result.value().deleted);
    CHECK(result.value().sceneID.value == p.scene2ID.value);

    // Resolve order after: should be [scene1] only
    auto after = resolver.resolve(p.projectDir.str());
    REQUIRE(after.ok());
    REQUIRE(after.value().size() == 1);
    CHECK(after.value()[0].sceneID.value == p.scene1ID.value);
}

TEST_CASE("deleteScene - deletes first scene, second scene remains",
          "[integration][T-0097]")
{
    TwoSceneProject p;

    scrivi::DeleteSceneRequest delReq;
    delReq.projectRootPath = p.projectDir.str();
    delReq.sceneID         = p.scene1ID;

    auto result = p.core.deleteScene(delReq);
    REQUIRE(result.ok());
    CHECK(result.value().deleted);

    scrivi::manuscript::ManuscriptOrderResolver resolver{p.services};
    auto after = resolver.resolve(p.projectDir.str());
    REQUIRE(after.ok());
    REQUIRE(after.value().size() == 1);
    CHECK(after.value()[0].sceneID.value == p.scene2ID.value);
}

TEST_CASE("deleteScene - returns error for unknown sceneID",
          "[integration][T-0097]")
{
    TempDir projectDir;
    TempDir appSupportDir;

    scrivi::platform::LocalFileSystem        lfs;
    scrivi::mocks::DeterministicUUIDProvider uuids;
    scrivi::mocks::FixedClock                clock{"2026-06-04T00:00:00Z"};
    scrivi::mocks::MockSecureStore           store;
    scrivi::mocks::MockGitProvider           git;
    scrivi::ScriviCore core{makeServices(lfs, uuids, clock, store, git)};

    scrivi::CreateProjectRequest req;
    req.projectRootPath = projectDir.str();
    req.appSupportRoot  = appSupportDir.str();
    req.title           = "Novel";
    req.slug            = "novel";
    req.author          = testAuthor();
    REQUIRE(core.createProject(req).ok());

    scrivi::DeleteSceneRequest delReq;
    delReq.projectRootPath = projectDir.str();
    delReq.sceneID         = scrivi::SceneID{"scene-does-not-exist"};

    auto result = core.deleteScene(delReq);
    CHECK_FALSE(result.ok());
    CHECK(result.error().code == scrivi::ErrorCode::invalidArgument);
}

TEST_CASE("deleteScene - returns error for empty sceneID",
          "[integration][T-0097]")
{
    TempDir projectDir;
    TempDir appSupportDir;

    scrivi::platform::LocalFileSystem        lfs;
    scrivi::mocks::DeterministicUUIDProvider uuids;
    scrivi::mocks::FixedClock                clock{"2026-06-04T00:00:00Z"};
    scrivi::mocks::MockSecureStore           store;
    scrivi::mocks::MockGitProvider           git;
    scrivi::ScriviCore core{makeServices(lfs, uuids, clock, store, git)};

    scrivi::CreateProjectRequest req;
    req.projectRootPath = projectDir.str();
    req.appSupportRoot  = appSupportDir.str();
    req.title           = "Novel";
    req.slug            = "novel";
    req.author          = testAuthor();
    REQUIRE(core.createProject(req).ok());

    scrivi::DeleteSceneRequest delReq;
    delReq.projectRootPath = projectDir.str();
    delReq.sceneID         = scrivi::SceneID{""};

    auto result = core.deleteScene(delReq);
    CHECK_FALSE(result.ok());
    CHECK(result.error().code == scrivi::ErrorCode::invalidArgument);
}

// ---------------------------------------------------------------------------
// deleteChapter tests
// ---------------------------------------------------------------------------

TEST_CASE("deleteChapter - removes chapter and all its scenes from index and disk",
          "[integration][T-0098]")
{
    TempDir projectDir;
    TempDir appSupportDir;

    scrivi::platform::LocalFileSystem        lfs;
    scrivi::mocks::DeterministicUUIDProvider uuids;
    scrivi::mocks::FixedClock                clock{"2026-06-04T00:00:00Z"};
    scrivi::mocks::MockSecureStore           store;
    scrivi::mocks::MockGitProvider           git;
    auto services = makeServices(lfs, uuids, clock, store, git);
    scrivi::ScriviCore core{services};

    // Create project (chapter 1 with 1 scene)
    scrivi::CreateProjectRequest req;
    req.projectRootPath = projectDir.str();
    req.appSupportRoot  = appSupportDir.str();
    req.title           = "Novel";
    req.slug            = "novel";
    req.author          = testAuthor();
    auto created = core.createProject(req);
    REQUIRE(created.ok());

    const auto ch1ID = created.value().firstChapterID;

    // Create chapter 2
    scrivi::CreateChapterRequest chReq;
    chReq.projectRootPath = projectDir.str();
    chReq.appSupportRoot  = appSupportDir.str();
    chReq.projectID       = created.value().project.projectID;
    chReq.author          = testAuthor();
    auto ch2Result = core.createChapter(chReq);
    REQUIRE(ch2Result.ok());
    const auto ch2ID        = ch2Result.value().chapterID;
    const auto ch2SceneID   = ch2Result.value().firstSceneID;
    const auto ch2MetaPath  = ch2Result.value().chapterMetadataPath;

    // Confirm 2 chapters, 2 scenes before delete
    scrivi::manuscript::ManuscriptOrderResolver resolver{services};
    auto before = resolver.resolve(projectDir.str());
    REQUIRE(before.ok());
    REQUIRE(before.value().size() == 2);

    // Delete chapter 2
    scrivi::DeleteChapterRequest delReq;
    delReq.projectRootPath = projectDir.str();
    delReq.chapterID       = ch2ID;

    auto result = core.deleteChapter(delReq);
    REQUIRE(result.ok());
    CHECK(result.value().deleted);
    CHECK(result.value().chapterID.value == ch2ID.value);
    CHECK(result.value().scenesDeleted == 1);

    // Chapter 2 directory should be gone
    CHECK_FALSE(projectDir.exists(ch2MetaPath));

    // Only chapter 1's scene remains
    auto after = resolver.resolve(projectDir.str());
    REQUIRE(after.ok());
    REQUIRE(after.value().size() == 1);
    CHECK(after.value()[0].chapterID.value == ch1ID.value);
    CHECK(after.value()[0].sceneID.value != ch2SceneID.value);
}

TEST_CASE("deleteChapter - returns error for unknown chapterID",
          "[integration][T-0098]")
{
    TempDir projectDir;
    TempDir appSupportDir;

    scrivi::platform::LocalFileSystem        lfs;
    scrivi::mocks::DeterministicUUIDProvider uuids;
    scrivi::mocks::FixedClock                clock{"2026-06-04T00:00:00Z"};
    scrivi::mocks::MockSecureStore           store;
    scrivi::mocks::MockGitProvider           git;
    scrivi::ScriviCore core{makeServices(lfs, uuids, clock, store, git)};

    scrivi::CreateProjectRequest req;
    req.projectRootPath = projectDir.str();
    req.appSupportRoot  = appSupportDir.str();
    req.title           = "Novel";
    req.slug            = "novel";
    req.author          = testAuthor();
    REQUIRE(core.createProject(req).ok());

    scrivi::DeleteChapterRequest delReq;
    delReq.projectRootPath = projectDir.str();
    delReq.chapterID       = scrivi::ChapterID{"chapter-does-not-exist"};

    auto result = core.deleteChapter(delReq);
    CHECK_FALSE(result.ok());
    CHECK(result.error().code == scrivi::ErrorCode::invalidArgument);
}

TEST_CASE("deleteChapter - returns error for empty chapterID",
          "[integration][T-0098]")
{
    TempDir projectDir;
    TempDir appSupportDir;

    scrivi::platform::LocalFileSystem        lfs;
    scrivi::mocks::DeterministicUUIDProvider uuids;
    scrivi::mocks::FixedClock                clock{"2026-06-04T00:00:00Z"};
    scrivi::mocks::MockSecureStore           store;
    scrivi::mocks::MockGitProvider           git;
    scrivi::ScriviCore core{makeServices(lfs, uuids, clock, store, git)};

    scrivi::CreateProjectRequest req;
    req.projectRootPath = projectDir.str();
    req.appSupportRoot  = appSupportDir.str();
    req.title           = "Novel";
    req.slug            = "novel";
    req.author          = testAuthor();
    REQUIRE(core.createProject(req).ok());

    scrivi::DeleteChapterRequest delReq;
    delReq.projectRootPath = projectDir.str();
    delReq.chapterID       = scrivi::ChapterID{""};

    auto result = core.deleteChapter(delReq);
    CHECK_FALSE(result.ok());
    CHECK(result.error().code == scrivi::ErrorCode::invalidArgument);
}
