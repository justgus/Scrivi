// RenameSceneTests.cpp
// Integration tests for T-0099/T-0100: renameScene and renameChapter (EP-010 SP-028).

#include <catch2/catch_test_macros.hpp>

#include "scrivi/ScriviCore.hpp"
#include "scrivi/Requests.hpp"

#include "mocks/DeterministicUUIDProvider.hpp"
#include "mocks/FixedClock.hpp"
#include "mocks/MockGitProvider.hpp"
#include "mocks/MockSecureStore.hpp"

#include "platform/LocalFileSystem.hpp"
#include "schemas/SceneMetaJson.hpp"
#include "schemas/ChapterMetaJson.hpp"
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
        path = fs::temp_directory_path() / ("scrivi-rename-" + std::to_string(
            std::chrono::steady_clock::now().time_since_epoch().count()));
        fs::create_directories(path);
    }

    ~TempDir() {
        std::error_code ec;
        fs::remove_all(path, ec);
    }

    std::string str() const { return path.string(); }
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
// renameScene tests (T-0099)
// ---------------------------------------------------------------------------

TEST_CASE("renameScene - writes new title to scene.meta.json",
          "[integration][T-0099]")
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
    auto created = core.createProject(req);
    REQUIRE(created.ok());

    const auto sceneMetaPath = created.value().firstSceneMetadataPath;

    scrivi::RenameSceneRequest renameReq;
    renameReq.projectRootPath = projectDir.str();
    renameReq.metadataPath    = sceneMetaPath;
    renameReq.newTitle        = "The Opening";

    auto result = core.renameScene(renameReq);
    REQUIRE(result.ok());
    CHECK(result.value().renamed);
    CHECK(result.value().newTitle == "The Opening");
    CHECK(result.value().metadataPath == sceneMetaPath);

    // Verify the title was actually written to disk
    auto readR = lfs.readTextFile(scrivi::util::join(projectDir.str(), sceneMetaPath));
    REQUIRE(readR.ok());
    auto parsedR = scrivi::schemas::parseSceneMeta(readR.value());
    REQUIRE(parsedR.ok());
    CHECK(parsedR.value().title == "The Opening");
}

TEST_CASE("renameScene - blank title saved as empty string",
          "[integration][T-0099]")
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
    auto created = core.createProject(req);
    REQUIRE(created.ok());

    const auto sceneMetaPath = created.value().firstSceneMetadataPath;

    scrivi::RenameSceneRequest renameReq;
    renameReq.projectRootPath = projectDir.str();
    renameReq.metadataPath    = sceneMetaPath;
    renameReq.newTitle        = "   ";  // whitespace only

    auto result = core.renameScene(renameReq);
    REQUIRE(result.ok());
    CHECK(result.value().renamed);
    CHECK(result.value().newTitle == "");

    auto readR = lfs.readTextFile(scrivi::util::join(projectDir.str(), sceneMetaPath));
    REQUIRE(readR.ok());
    auto parsedR = scrivi::schemas::parseSceneMeta(readR.value());
    REQUIRE(parsedR.ok());
    CHECK(parsedR.value().title == "");
}

TEST_CASE("renameScene - returns error for empty metadataPath",
          "[integration][T-0099]")
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

    scrivi::RenameSceneRequest renameReq;
    renameReq.projectRootPath = projectDir.str();
    renameReq.metadataPath    = "";
    renameReq.newTitle        = "Whatever";

    auto result = core.renameScene(renameReq);
    CHECK_FALSE(result.ok());
    CHECK(result.error().code == scrivi::ErrorCode::invalidArgument);
}

// ---------------------------------------------------------------------------
// renameChapter tests (T-0100)
// ---------------------------------------------------------------------------

TEST_CASE("renameChapter - writes new title to chapter.meta.json",
          "[integration][T-0100]")
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
    auto created = core.createProject(req);
    REQUIRE(created.ok());

    // Get chapter metadata path from the manuscript index
    auto msR = lfs.readTextFile(scrivi::util::join(projectDir.str(),
                                "manuscript/manuscript.meta.json"));
    REQUIRE(msR.ok());
    auto msParsed = scrivi::schemas::parseManuscriptMeta(msR.value());
    REQUIRE(msParsed.ok());
    REQUIRE_FALSE(msParsed.value().chapters.empty());
    const std::string chMetaPath = msParsed.value().chapters[0].path;

    scrivi::RenameChapterRequest renameReq;
    renameReq.projectRootPath = projectDir.str();
    renameReq.metadataPath    = chMetaPath;
    renameReq.newTitle        = "Part One: The Beginning";

    auto result = core.renameChapter(renameReq);
    REQUIRE(result.ok());
    CHECK(result.value().renamed);
    CHECK(result.value().newTitle == "Part One: The Beginning");
    CHECK(result.value().metadataPath == chMetaPath);

    // Verify on disk
    auto chR = lfs.readTextFile(scrivi::util::join(projectDir.str(), chMetaPath));
    REQUIRE(chR.ok());
    auto chParsed = scrivi::schemas::parseChapterMeta(chR.value());
    REQUIRE(chParsed.ok());
    CHECK(chParsed.value().title == "Part One: The Beginning");
}

TEST_CASE("renameChapter - whitespace-only title saved as empty string",
          "[integration][T-0100]")
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

    auto msR = lfs.readTextFile(scrivi::util::join(projectDir.str(),
                                "manuscript/manuscript.meta.json"));
    REQUIRE(msR.ok());
    auto msParsed = scrivi::schemas::parseManuscriptMeta(msR.value());
    REQUIRE(msParsed.ok());
    const std::string chMetaPath = msParsed.value().chapters[0].path;

    scrivi::RenameChapterRequest renameReq;
    renameReq.projectRootPath = projectDir.str();
    renameReq.metadataPath    = chMetaPath;
    renameReq.newTitle        = "\t\n  ";

    auto result = core.renameChapter(renameReq);
    REQUIRE(result.ok());
    CHECK(result.value().newTitle == "");
}

TEST_CASE("renameChapter - returns error for empty metadataPath",
          "[integration][T-0100]")
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

    scrivi::RenameChapterRequest renameReq;
    renameReq.projectRootPath = projectDir.str();
    renameReq.metadataPath    = "";
    renameReq.newTitle        = "Whatever";

    auto result = core.renameChapter(renameReq);
    CHECK_FALSE(result.ok());
    CHECK(result.error().code == scrivi::ErrorCode::invalidArgument);
}
