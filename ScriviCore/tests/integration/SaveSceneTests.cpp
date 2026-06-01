#include <catch2/catch_approx.hpp>
#include <catch2/catch_test_macros.hpp>

#include "scrivi/ScriviCore.hpp"
#include "scrivi/Requests.hpp"

#include "mocks/DeterministicUUIDProvider.hpp"
#include "mocks/FixedClock.hpp"
#include "mocks/MockGitProvider.hpp"
#include "mocks/MockSecureStore.hpp"

#include "platform/LocalFileSystem.hpp"
#include "schemas/SceneMetaJson.hpp"
#include "util/Hash.hpp"

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
        path = fs::temp_directory_path() / ("scrivi-save-test-" + std::to_string(
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

static std::string readFile(const std::string& path) {
    std::ifstream f(path);
    return {std::istreambuf_iterator<char>(f), {}};
}

static scrivi::CoreServices makeServices(
    scrivi::platform::LocalFileSystem&       lfs,
    scrivi::mocks::DeterministicUUIDProvider& uuids,
    scrivi::mocks::FixedClock&               clock,
    scrivi::mocks::MockSecureStore&          store,
    scrivi::mocks::MockGitProvider&          git)
{
    scrivi::CoreServices s;
    s.fileSystem   = &lfs;
    s.uuidProvider = &uuids;
    s.clock        = &clock;
    s.secureStore  = &store;
    s.gitProvider  = &git;
    return s;
}

static scrivi::CreateProjectResult createTestProject(
    scrivi::ScriviCore& core,
    const std::string& projectDir,
    const std::string& appSupportDir)
{
    scrivi::CreateProjectRequest req;
    req.projectRootPath = projectDir;
    req.appSupportRoot  = appSupportDir;
    req.title           = "Save Test Novel";
    req.slug            = "save-test";
    req.author          = {
        scrivi::IdentityID{"identity-001"},
        scrivi::PersonaID {"persona-001"},
        "Test Author"
    };
    auto result = core.createProject(req);
    REQUIRE(result.ok());
    return result.value();
}

static scrivi::SaveSceneRequest makeSaveRequest(
    const scrivi::CreateProjectResult& created,
    const std::string& projectDir,
    const std::string& appSupportDir,
    const std::string& markdown,
    scrivi::TextSelection selection = {0, 0},
    scrivi::ScrollPosition scroll   = {0.0})
{
    scrivi::SaveSceneRequest req;
    req.projectID         = created.project.projectID;
    req.projectRootPath   = projectDir;
    req.appSupportRoot    = appSupportDir;
    req.sceneID           = created.firstSceneID;
    req.sceneMetadataPath = created.firstSceneMetadataPath;
    req.sceneContentPath  = created.firstSceneContentPath;
    req.markdown          = markdown;
    req.selection         = selection;
    req.scroll            = scroll;
    req.author            = {
        scrivi::IdentityID{"identity-001"},
        scrivi::PersonaID {"persona-001"},
        "Test Author"
    };
    return req;
}

// ---------------------------------------------------------------------------
// Tests
// ---------------------------------------------------------------------------

TEST_CASE("saveScene - written content reads back identically", "[integration][T-0008]") {
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

    const std::string content = "# Chapter One\n\nOnce upon a time in a land far away.";
    auto req = makeSaveRequest(created, projectDir.str(), appSupportDir.str(), content);
    auto result = core.saveScene(req);

    REQUIRE(result.ok());
    CHECK(result.value().saved == true);

    auto onDisk = readFile(projectDir.sub(created.firstSceneContentPath));
    CHECK(onDisk == content);
}

TEST_CASE("saveScene - modifiedAt updated in scene metadata", "[integration][T-0008]") {
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

    auto req = makeSaveRequest(created, projectDir.str(), appSupportDir.str(), "New content.");
    REQUIRE(core.saveScene(req).ok());

    auto metaJson = readFile(projectDir.sub(created.firstSceneMetadataPath));
    auto parsed   = scrivi::schemas::parseSceneMeta(metaJson);
    REQUIRE(parsed.ok());
    CHECK(parsed.value().modifiedAt == "2026-05-20T00:00:00Z");
    CHECK(parsed.value().modifiedByIdentityID == "identity-001");
    CHECK(parsed.value().modifiedByDisplayName == "Test Author");
}

TEST_CASE("saveScene - word count in metadata matches content", "[integration][T-0008]") {
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

    // 10 words
    auto req = makeSaveRequest(created, projectDir.str(), appSupportDir.str(),
        "The quick brown fox jumps over the lazy dog now.");
    auto saveResult = core.saveScene(req);
    REQUIRE(saveResult.ok());

    auto& r = saveResult.value();
    CHECK(r.wordCount == 10);
    CHECK(r.metadataUpdated == true);

    auto metaJson = readFile(projectDir.sub(created.firstSceneMetadataPath));
    auto parsed   = scrivi::schemas::parseSceneMeta(metaJson);
    REQUIRE(parsed.ok());
    CHECK(parsed.value().wordCount == 10);
}

TEST_CASE("saveScene - open after save restores cursor and scroll", "[integration][T-0008]") {
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

    auto req = makeSaveRequest(created, projectDir.str(), appSupportDir.str(),
        "Some text.", {3, 7}, {0.75});
    REQUIRE(core.saveScene(req).ok());

    scrivi::OpenProjectRequest openReq;
    openReq.projectRootPath = projectDir.str();
    openReq.appSupportRoot  = appSupportDir.str();

    auto openResult = core.openProject(openReq);
    REQUIRE(openResult.ok());

    auto& r = openResult.value();
    CHECK(r.restoredSelection.anchor == 3);
    CHECK(r.restoredSelection.focus  == 7);
    CHECK(r.restoredScroll.value     == Catch::Approx(0.75).margin(0.01));
    CHECK(r.activeSceneMarkdown      == "Some text.");
}

TEST_CASE("saveScene - unchanged content hash skips write and returns saved=false", "[integration][T-0008]") {
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

    const std::string content = "Unchanged content.";

    // First save - establishes the file
    auto req1 = makeSaveRequest(created, projectDir.str(), appSupportDir.str(), content);
    REQUIRE(core.saveScene(req1).ok());

    // Compute hash of the content (same as what save would have computed)
    // Second save with the same hash - must not write
    auto req2 = req1;
    req2.previouslyLoadedContentHash = scrivi::util::sha256Hex(content);

    auto result = core.saveScene(req2);
    REQUIRE(result.ok());
    CHECK(result.value().saved == false);
    // But workspace state is still updated
    CHECK(result.value().workspaceStateUpdated == true);
}
