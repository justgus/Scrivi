#include <catch2/catch_test_macros.hpp>

#include "scrivi/ScriviCore.hpp"
#include "scrivi/Requests.hpp"

#include "mocks/DeterministicUUIDProvider.hpp"
#include "mocks/FixedClock.hpp"
#include "mocks/MockGitProvider.hpp"
#include "mocks/MockSecureStore.hpp"

#include "platform/LocalFileSystem.hpp"
#include "schemas/SceneMetaJson.hpp"

#include <filesystem>
#include <fstream>
#include <string>

namespace fs = std::filesystem;

// ---------------------------------------------------------------------------
// Helpers
// ---------------------------------------------------------------------------

static scrivi::CreateProjectRequest makeRequest(
    const std::string& root,
    const std::string& appSupport,
    bool enableGit = false)
{
    scrivi::CreateProjectRequest req;
    req.projectRootPath = root;
    req.appSupportRoot  = appSupport;
    req.title           = "My Novel";
    req.slug            = "my-novel";
    req.author          = {
        scrivi::IdentityID{"identity-001"},
        scrivi::PersonaID {"persona-001"},
        "Julie Stoddard"
    };
    req.enableGitSnapshots = enableGit;
    return req;
}

static std::string readFile(const std::string& path) {
    std::ifstream f(path);
    return {std::istreambuf_iterator<char>(f), {}};
}

static bool fileExists(const std::string& path) {
    return fs::exists(path);
}

// ---------------------------------------------------------------------------
// Fixture: temp directory that cleans itself up
// ---------------------------------------------------------------------------

struct TempDir {
    fs::path path;

    TempDir() {
        path = fs::temp_directory_path() / ("scrivi-test-" + std::to_string(
            std::chrono::steady_clock::now().time_since_epoch().count()));
        fs::create_directories(path);
    }

    ~TempDir() {
        std::error_code ec;
        fs::remove_all(path, ec);
    }

    std::string str() const { return path.string(); }
    std::string sub(const std::string& rel) const {
        return (path / rel).string();
    }
};

// ---------------------------------------------------------------------------
// Tests
// ---------------------------------------------------------------------------

TEST_CASE("createProject - minimum package structure", "[integration][T-0006]") {
    TempDir projectDir;
    TempDir appSupportDir;

    scrivi::platform::LocalFileSystem   fs;
    scrivi::mocks::DeterministicUUIDProvider uuids;
    scrivi::mocks::FixedClock           clock{"2026-05-20T00:00:00Z"};
    scrivi::mocks::MockSecureStore      store;
    scrivi::mocks::MockGitProvider      git;

    scrivi::CoreServices services;
    services.fileSystem   = &fs;
    services.uuidProvider = &uuids;
    services.clock        = &clock;
    services.secureStore  = &store;
    services.gitProvider  = &git;

    scrivi::ScriviCore core{services};

    auto req    = makeRequest(projectDir.str(), appSupportDir.str());
    auto result = core.createProject(req);

    REQUIRE(result.ok());
    auto& r = result.value();

    // IDs populated
    CHECK(!r.project.projectID.value.empty());
    CHECK(!r.manuscriptID.value.empty());
    CHECK(!r.firstChapterID.value.empty());
    CHECK(!r.firstSceneID.value.empty());

    // Required files exist
    CHECK(fileExists(projectDir.sub("project.json")));
    CHECK(fileExists(projectDir.sub("manuscript/manuscript.meta.json")));
    CHECK(fileExists(projectDir.sub("manuscript/chapter-001/chapter.meta.json")));
    CHECK(fileExists(projectDir.sub("manuscript/chapter-001/001-opening-scene.md")));
    CHECK(fileExists(projectDir.sub("manuscript/chapter-001/001-opening-scene.meta.json")));
    CHECK(fileExists(projectDir.sub("identities/project-members.json")));
    CHECK(fileExists(projectDir.sub("identities/project-personas.json")));

    // No Git files when not requested
    CHECK_FALSE(fileExists(projectDir.sub(".git")));
    CHECK_FALSE(fileExists(projectDir.sub(".gitignore")));
    CHECK(r.gitInitialized == false);
    CHECK(!r.initialSnapshotID.has_value());
}

TEST_CASE("createProject - project.json content is valid JSON with correct fields", "[integration][T-0006]") {
    TempDir projectDir;
    TempDir appSupportDir;

    scrivi::platform::LocalFileSystem   fs;
    scrivi::mocks::DeterministicUUIDProvider uuids;
    scrivi::mocks::FixedClock           clock{"2026-05-20T00:00:00Z"};
    scrivi::mocks::MockSecureStore      store;
    scrivi::mocks::MockGitProvider      git;

    scrivi::CoreServices services;
    services.fileSystem   = &fs;
    services.uuidProvider = &uuids;
    services.clock        = &clock;
    services.secureStore  = &store;
    services.gitProvider  = &git;

    scrivi::ScriviCore core{services};

    auto result = core.createProject(makeRequest(projectDir.str(), appSupportDir.str()));
    REQUIRE(result.ok());

    auto json = readFile(projectDir.sub("project.json"));
    CHECK(json.find("\"schema\"") != std::string::npos);
    CHECK(json.find("\"My Novel\"") != std::string::npos);
    CHECK(json.find("\"my-novel\"") != std::string::npos);
    CHECK(json.find("\"identity-001\"") != std::string::npos);
    CHECK(json.find("\"Julie Stoddard\"") != std::string::npos);
    CHECK(json.find("\"gitSnapshots\"") != std::string::npos);
}

TEST_CASE("createProject - scene metadata round-trips through schema parser", "[integration][T-0006]") {
    TempDir projectDir;
    TempDir appSupportDir;

    scrivi::platform::LocalFileSystem   lfs;
    scrivi::mocks::DeterministicUUIDProvider uuids;
    scrivi::mocks::FixedClock           clock{"2026-05-20T00:00:00Z"};
    scrivi::mocks::MockSecureStore      store;
    scrivi::mocks::MockGitProvider      git;

    scrivi::CoreServices services;
    services.fileSystem   = &lfs;
    services.uuidProvider = &uuids;
    services.clock        = &clock;
    services.secureStore  = &store;
    services.gitProvider  = &git;

    scrivi::ScriviCore core{services};

    auto result = core.createProject(makeRequest(projectDir.str(), appSupportDir.str()));
    REQUIRE(result.ok());

    auto sceneMeta = readFile(
        projectDir.sub("manuscript/chapter-001/001-opening-scene.meta.json"));

    // Must parse without error
    auto parsed = scrivi::schemas::parseSceneMeta(sceneMeta);
    REQUIRE(parsed.ok());
    CHECK(parsed.value().title == "Opening Scene");
    CHECK(parsed.value().status == "draft");
    CHECK(parsed.value().wordCount == 0);
    CHECK(parsed.value().contentPath == "manuscript/chapter-001/001-opening-scene.md");
}

TEST_CASE("createProject - workspace state written to appSupportRoot", "[integration][T-0006]") {
    TempDir projectDir;
    TempDir appSupportDir;

    scrivi::platform::LocalFileSystem   fs;
    scrivi::mocks::DeterministicUUIDProvider uuids;
    scrivi::mocks::FixedClock           clock{"2026-05-20T00:00:00Z"};
    scrivi::mocks::MockSecureStore      store;
    scrivi::mocks::MockGitProvider      git;

    scrivi::CoreServices services;
    services.fileSystem   = &fs;
    services.uuidProvider = &uuids;
    services.clock        = &clock;
    services.secureStore  = &store;
    services.gitProvider  = &git;

    scrivi::ScriviCore core{services};

    auto result = core.createProject(makeRequest(projectDir.str(), appSupportDir.str()));
    REQUIRE(result.ok());

    auto& r          = result.value();
    auto  projectID  = r.project.projectID.value;

    // Workspace state file must exist under appSupportRoot/state/projects/<id>/
    auto wsPath = appSupportDir.str() + "/state/projects/" + projectID + "/workspace-state.json";
    REQUIRE(fileExists(wsPath));

    auto json = readFile(wsPath);
    CHECK(json.find(projectID) != std::string::npos);
    CHECK(json.find("\"identity-001\"") != std::string::npos);

    // Result workspace state points to first scene
    REQUIRE(r.workspaceState.lastWritingSurface.has_value());
    CHECK(r.workspaceState.lastWritingSurface->sceneID.value == r.firstSceneID.value);
}

TEST_CASE("createProject - Git-enabled path initializes repo and creates snapshot", "[integration][T-0006]") {
    TempDir projectDir;
    TempDir appSupportDir;

    scrivi::platform::LocalFileSystem   fs;
    scrivi::mocks::DeterministicUUIDProvider uuids;
    scrivi::mocks::FixedClock           clock{"2026-05-20T00:00:00Z"};
    scrivi::mocks::MockSecureStore      store;
    scrivi::mocks::MockGitProvider      git;

    scrivi::CoreServices services;
    services.fileSystem   = &fs;
    services.uuidProvider = &uuids;
    services.clock        = &clock;
    services.secureStore  = &store;
    services.gitProvider  = &git;

    scrivi::ScriviCore core{services};

    auto result = core.createProject(
        makeRequest(projectDir.str(), appSupportDir.str(), /*enableGit=*/true));
    REQUIRE(result.ok());

    auto& r = result.value();
    CHECK(r.gitInitialized == true);
    CHECK(r.initialSnapshotID.has_value());

    // Mock records correct sequence: init → addAll → commit
    CHECK(git.initCalls.size()     == 1);
    CHECK(git.addAllCalls.size()   == 1);
    CHECK(git.commitRequests.size() == 1);
    CHECK(git.commitRequests[0].message == "Initial project");
    CHECK(git.commitRequests[0].author.name == "Julie Stoddard");

    // .gitignore and snapshots/ written
    CHECK(fileExists(projectDir.sub(".gitignore")));
    CHECK(fileExists(projectDir.sub("snapshots/scrivi-snapshots.json")));
}

TEST_CASE("createProject - rejects empty identityID", "[integration][T-0006]") {
    TempDir projectDir;
    TempDir appSupportDir;

    scrivi::platform::LocalFileSystem   fs;
    scrivi::mocks::DeterministicUUIDProvider uuids;
    scrivi::mocks::FixedClock           clock{"2026-05-20T00:00:00Z"};
    scrivi::mocks::MockSecureStore      store;
    scrivi::mocks::MockGitProvider      git;

    scrivi::CoreServices services;
    services.fileSystem   = &fs;
    services.uuidProvider = &uuids;
    services.clock        = &clock;
    services.secureStore  = &store;
    services.gitProvider  = &git;

    scrivi::ScriviCore core{services};

    scrivi::CreateProjectRequest req;
    req.projectRootPath = projectDir.str();
    req.appSupportRoot  = appSupportDir.str();
    req.title           = "Test";
    req.slug            = "test";
    req.author          = {scrivi::IdentityID{""}, scrivi::PersonaID{"p-001"}, "Name"};

    auto result = core.createProject(req);
    REQUIRE_FALSE(result.ok());
    CHECK(result.error().code == scrivi::ErrorCode::invalidArgument);
}
