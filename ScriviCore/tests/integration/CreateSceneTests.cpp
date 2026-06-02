// CreateSceneTests.cpp
// Integration tests for T-0071: createScene and createChapter (EP-009 SP-021).

#include <catch2/catch_test_macros.hpp>

#include "scrivi/ScriviCore.hpp"
#include "scrivi/Requests.hpp"

#include "mocks/DeterministicUUIDProvider.hpp"
#include "mocks/FixedClock.hpp"
#include "mocks/MockGitProvider.hpp"
#include "mocks/MockSecureStore.hpp"

#include "platform/LocalFileSystem.hpp"
#include "manuscript/ManuscriptOrderResolver.hpp"
#include "schemas/ChapterMetaJson.hpp"
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
        path = fs::temp_directory_path() / ("scrivi-createscene-" + std::to_string(
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

// ---------------------------------------------------------------------------
// createScene tests
// ---------------------------------------------------------------------------

TEST_CASE("createScene - appends new scene to single-scene project",
          "[integration][T-0071]")
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
    req.author          = testAuthor();
    auto created = core.createProject(req);
    REQUIRE(created.ok());

    const auto& firstChapterID = created.value().firstChapterID;

    scrivi::CreateSceneRequest sceneReq;
    sceneReq.projectRootPath = projectDir.str();
    sceneReq.appSupportRoot  = appSupportDir.str();
    sceneReq.projectID       = created.value().project.projectID;
    sceneReq.chapterID       = firstChapterID;
    sceneReq.afterSceneID    = created.value().firstSceneID;
    sceneReq.author          = testAuthor();

    auto result = core.createScene(sceneReq);
    REQUIRE(result.ok());

    const auto& r = result.value();
    CHECK_FALSE(r.sceneID.value.empty());
    CHECK(r.chapterID.value == firstChapterID.value);
    CHECK_FALSE(r.metadataPath.empty());
    CHECK_FALSE(r.contentPath.empty());

    // Files must exist on disk
    CHECK(projectDir.exists(r.metadataPath));
    CHECK(projectDir.exists(r.contentPath));
}

TEST_CASE("createScene - ManuscriptOrderResolver returns correct order after insert",
          "[integration][T-0071]")
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
    req.author          = testAuthor();
    auto created = core.createProject(req);
    REQUIRE(created.ok());

    const auto& firstSceneID  = created.value().firstSceneID;
    const auto& firstChapterID = created.value().firstChapterID;

    // Add scene 2 after scene 1
    scrivi::CreateSceneRequest s2Req;
    s2Req.projectRootPath = projectDir.str();
    s2Req.appSupportRoot  = appSupportDir.str();
    s2Req.projectID       = created.value().project.projectID;
    s2Req.chapterID       = firstChapterID;
    s2Req.afterSceneID    = firstSceneID;
    s2Req.author          = testAuthor();
    auto s2 = core.createScene(s2Req);
    REQUIRE(s2.ok());

    // Add scene 3 after scene 2
    scrivi::CreateSceneRequest s3Req = s2Req;
    s3Req.afterSceneID = s2.value().sceneID;
    auto s3 = core.createScene(s3Req);
    REQUIRE(s3.ok());

    // Resolve full order
    scrivi::manuscript::ManuscriptOrderResolver resolver{services};
    auto resolved = resolver.resolve(projectDir.str());
    REQUIRE(resolved.ok());

    const auto& scenes = resolved.value();
    REQUIRE(scenes.size() == 3);
    CHECK(scenes[0].sceneID.value == firstSceneID.value);
    CHECK(scenes[1].sceneID.value == s2.value().sceneID.value);
    CHECK(scenes[2].sceneID.value == s3.value().sceneID.value);
}

TEST_CASE("createScene - empty afterSceneID appends to end of chapter",
          "[integration][T-0071]")
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
    req.author          = testAuthor();
    auto created = core.createProject(req);
    REQUIRE(created.ok());

    scrivi::CreateSceneRequest sceneReq;
    sceneReq.projectRootPath = projectDir.str();
    sceneReq.appSupportRoot  = appSupportDir.str();
    sceneReq.projectID       = created.value().project.projectID;
    sceneReq.chapterID       = created.value().firstChapterID;
    sceneReq.afterSceneID    = scrivi::SceneID{""};  // empty = append
    sceneReq.author          = testAuthor();
    auto result = core.createScene(sceneReq);
    REQUIRE(result.ok());

    scrivi::manuscript::ManuscriptOrderResolver resolver{services};
    auto resolved = resolver.resolve(projectDir.str());
    REQUIRE(resolved.ok());
    REQUIRE(resolved.value().size() == 2);
    CHECK(resolved.value()[1].sceneID.value == result.value().sceneID.value);
}

TEST_CASE("createScene - returns error for unknown chapterID",
          "[integration][T-0071]")
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
    req.author          = testAuthor();
    REQUIRE(core.createProject(req).ok());

    scrivi::CreateSceneRequest sceneReq;
    sceneReq.projectRootPath = projectDir.str();
    sceneReq.appSupportRoot  = appSupportDir.str();
    sceneReq.projectID       = scrivi::ProjectID{"proj-001"};
    sceneReq.chapterID       = scrivi::ChapterID{"chapter-does-not-exist"};
    sceneReq.afterSceneID    = scrivi::SceneID{""};
    sceneReq.author          = testAuthor();

    auto result = core.createScene(sceneReq);
    CHECK_FALSE(result.ok());
    CHECK(result.error().code == scrivi::ErrorCode::invalidArgument);
}

// ---------------------------------------------------------------------------
// createChapter tests
// ---------------------------------------------------------------------------

TEST_CASE("createChapter - appends new chapter with auto first scene",
          "[integration][T-0071]")
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
    req.author          = testAuthor();
    REQUIRE(core.createProject(req).ok());

    scrivi::CreateChapterRequest chReq;
    chReq.projectRootPath = projectDir.str();
    chReq.appSupportRoot  = appSupportDir.str();
    chReq.projectID       = scrivi::ProjectID{"proj-001"};
    chReq.author          = testAuthor();

    auto result = core.createChapter(chReq);
    REQUIRE(result.ok());

    const auto& r = result.value();
    CHECK_FALSE(r.chapterID.value.empty());
    CHECK_FALSE(r.chapterMetadataPath.empty());
    CHECK_FALSE(r.firstSceneID.value.empty());
    CHECK_FALSE(r.firstSceneMetadataPath.empty());
    CHECK_FALSE(r.firstSceneContentPath.empty());

    // All files must exist on disk
    CHECK(projectDir.exists(r.chapterMetadataPath));
    CHECK(projectDir.exists(r.firstSceneMetadataPath));
    CHECK(projectDir.exists(r.firstSceneContentPath));
}

TEST_CASE("createChapter - ManuscriptOrderResolver returns correct chapter and scene order",
          "[integration][T-0071]")
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
    req.author          = testAuthor();
    auto created = core.createProject(req);
    REQUIRE(created.ok());

    const auto firstSceneID = created.value().firstSceneID;

    // Add a scene to chapter 1
    scrivi::CreateSceneRequest s2Req;
    s2Req.projectRootPath = projectDir.str();
    s2Req.appSupportRoot  = appSupportDir.str();
    s2Req.projectID       = created.value().project.projectID;
    s2Req.chapterID       = created.value().firstChapterID;
    s2Req.afterSceneID    = firstSceneID;
    s2Req.author          = testAuthor();
    auto s2 = core.createScene(s2Req);
    REQUIRE(s2.ok());

    // Create chapter 2
    scrivi::CreateChapterRequest chReq;
    chReq.projectRootPath = projectDir.str();
    chReq.appSupportRoot  = appSupportDir.str();
    chReq.projectID       = created.value().project.projectID;
    chReq.author          = testAuthor();
    auto ch2 = core.createChapter(chReq);
    REQUIRE(ch2.ok());

    // Resolve full manuscript order
    scrivi::manuscript::ManuscriptOrderResolver resolver{services};
    auto resolved = resolver.resolve(projectDir.str());
    REQUIRE(resolved.ok());

    const auto& scenes = resolved.value();
    REQUIRE(scenes.size() == 3);

    // Chapter 1: scene 1, scene 2
    CHECK(scenes[0].sceneID.value  == firstSceneID.value);
    CHECK(scenes[0].chapterID.value == created.value().firstChapterID.value);
    CHECK(scenes[1].sceneID.value  == s2.value().sceneID.value);
    CHECK(scenes[1].chapterID.value == created.value().firstChapterID.value);

    // Chapter 2: auto first scene
    CHECK(scenes[2].sceneID.value  == ch2.value().firstSceneID.value);
    CHECK(scenes[2].chapterID.value == ch2.value().chapterID.value);
}
