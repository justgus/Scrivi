#include <catch2/catch_test_macros.hpp>

#include "scrivi/ScriviCore.hpp"
#include "scrivi/Requests.hpp"

#include "mocks/DeterministicUUIDProvider.hpp"
#include "mocks/FixedClock.hpp"
#include "mocks/MockGitProvider.hpp"
#include "mocks/MockSecureStore.hpp"

#include "git/SystemGitProvider.hpp"
#include "platform/LocalFileSystem.hpp"
#include "util/Json.hpp"

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
        path = fs::temp_directory_path() / ("scrivi-git-test-" + std::to_string(
            std::chrono::steady_clock::now().time_since_epoch().count()));
        fs::create_directories(path);
    }

    ~TempDir() {
        std::error_code ec;
        fs::remove_all(path, ec);
    }

    std::string str()                       const { return path.string(); }
    std::string sub(const std::string& rel) const { return (path / rel).string(); }
};

static scrivi::CoreServices makeServices(
    scrivi::platform::LocalFileSystem&       lfs,
    scrivi::mocks::DeterministicUUIDProvider& uuids,
    scrivi::mocks::FixedClock&               clock,
    scrivi::mocks::MockSecureStore&          store,
    scrivi::GitProvider*                     git)
{
    scrivi::CoreServices s;
    s.fileSystem   = &lfs;
    s.uuidProvider = &uuids;
    s.clock        = &clock;
    s.secureStore  = &store;
    s.gitProvider  = git;
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
    req.title           = "Git Test Novel";
    req.slug            = "git-test";
    req.author          = {
        scrivi::IdentityID{"identity-001"},
        scrivi::PersonaID {"persona-001"},
        "Test Author"
    };
    auto result = core.createProject(req);
    REQUIRE(result.ok());
    return result.value();
}

static std::string readFile(const std::string& path) {
    std::ifstream f(path);
    return {std::istreambuf_iterator<char>(f), {}};
}

// ---------------------------------------------------------------------------
// Mock-based tests (always run — no real git required)
// ---------------------------------------------------------------------------

TEST_CASE("enableGitSnapshots — mock: init/addAll/commit called, snapshot metadata written", "[integration][T-0010]") {
    TempDir projectDir;
    TempDir appSupportDir;

    scrivi::platform::LocalFileSystem        lfs;
    scrivi::mocks::DeterministicUUIDProvider uuids;
    scrivi::mocks::FixedClock                clock{"2026-05-20T00:00:00Z"};
    scrivi::mocks::MockSecureStore           store;
    scrivi::mocks::MockGitProvider           git;

    auto services = makeServices(lfs, uuids, clock, store, &git);
    scrivi::ScriviCore core{services};

    createTestProject(core, projectDir.str(), appSupportDir.str());

    scrivi::EnableGitRequest req;
    req.projectRootPath      = projectDir.str();
    req.initialSnapshotLabel = "Initial project";
    req.author               = {
        scrivi::IdentityID{"identity-001"},
        scrivi::PersonaID {"persona-001"},
        "Test Author"
    };

    auto result = core.enableGitSnapshots(req);
    REQUIRE(result.ok());

    auto& r = result.value();
    CHECK(r.gitInitialized == true);
    CHECK(r.alreadyRepository == false);
    CHECK_FALSE(r.initialSnapshotID.value.empty());
    CHECK_FALSE(r.initialCommitID.value.empty());

    // Git call sequence: init → addAll → commit
    CHECK(git.initCalls.size()      == 1);
    CHECK(git.addAllCalls.size()    == 1);
    CHECK(git.commitRequests.size() == 1);
    CHECK(git.commitRequests[0].message     == "Initial project");
    CHECK(git.commitRequests[0].author.name == "Test Author");

    // .gitignore written
    CHECK(fs::exists(projectDir.sub(".gitignore")));

    // Snapshot metadata written with one entry
    auto snapJson = readFile(projectDir.sub("snapshots/scrivi-snapshots.json"));
    auto parsed   = scrivi::util::parseJson(snapJson);
    REQUIRE(parsed.ok());
    CHECK(parsed.value().arraySize("snapshots") == 1);
    auto entry = parsed.value().arrayItem("snapshots", 0);
    CHECK(entry.getString("label") == "Initial project");
    CHECK_FALSE(entry.getString("snapshotID").empty());
}

TEST_CASE("enableGitSnapshots — mock: alreadyRepository=true when repo exists", "[integration][T-0010]") {
    TempDir projectDir;
    TempDir appSupportDir;

    scrivi::platform::LocalFileSystem        lfs;
    scrivi::mocks::DeterministicUUIDProvider uuids;
    scrivi::mocks::FixedClock                clock{"2026-05-20T00:00:00Z"};
    scrivi::mocks::MockSecureStore           store;
    scrivi::mocks::MockGitProvider           git;

    git.repoExists = true;  // already a repository

    auto services = makeServices(lfs, uuids, clock, store, &git);
    scrivi::ScriviCore core{services};

    createTestProject(core, projectDir.str(), appSupportDir.str());

    scrivi::EnableGitRequest req;
    req.projectRootPath = projectDir.str();
    req.author          = {
        scrivi::IdentityID{"identity-001"},
        scrivi::PersonaID {"persona-001"},
        "Test Author"
    };

    auto result = core.enableGitSnapshots(req);
    REQUIRE(result.ok());

    CHECK(result.value().alreadyRepository == true);
    // initRepository should NOT have been called
    CHECK(git.initCalls.empty());
    // addAll and commit still called
    CHECK(git.addAllCalls.size()    == 1);
    CHECK(git.commitRequests.size() == 1);
}

TEST_CASE("createSnapshot — mock: addAll/commit called, metadata appended", "[integration][T-0010]") {
    TempDir projectDir;
    TempDir appSupportDir;

    scrivi::platform::LocalFileSystem        lfs;
    scrivi::mocks::DeterministicUUIDProvider uuids;
    scrivi::mocks::FixedClock                clock{"2026-05-20T00:00:00Z"};
    scrivi::mocks::MockSecureStore           store;
    scrivi::mocks::MockGitProvider           git;

    auto services = makeServices(lfs, uuids, clock, store, &git);
    scrivi::ScriviCore core{services};

    createTestProject(core, projectDir.str(), appSupportDir.str());

    // Enable git first so snapshots/ dir exists
    scrivi::EnableGitRequest enableReq;
    enableReq.projectRootPath      = projectDir.str();
    enableReq.initialSnapshotLabel = "Initial project";
    enableReq.author = {scrivi::IdentityID{"identity-001"}, scrivi::PersonaID{"persona-001"}, "Test Author"};
    REQUIRE(core.enableGitSnapshots(enableReq).ok());

    // Now create a named snapshot
    scrivi::CreateSnapshotRequest snapReq;
    snapReq.projectRootPath = projectDir.str();
    snapReq.label           = "Chapter 1 done";
    snapReq.note            = "Finished the first chapter draft.";
    snapReq.author          = {scrivi::IdentityID{"identity-001"}, scrivi::PersonaID{"persona-001"}, "Test Author"};

    auto result = core.createSnapshot(snapReq);
    REQUIRE(result.ok());

    auto& r = result.value();
    CHECK(r.created == true);
    CHECK_FALSE(r.snapshotID.value.empty());
    CHECK_FALSE(r.commitID.value.empty());

    // Two commits total: initial + named snapshot
    CHECK(git.commitRequests.size() == 2);
    CHECK(git.commitRequests[1].message == "Chapter 1 done");

    // Metadata file has two entries
    auto snapJson = readFile(projectDir.sub("snapshots/scrivi-snapshots.json"));
    auto parsed   = scrivi::util::parseJson(snapJson);
    REQUIRE(parsed.ok());
    CHECK(parsed.value().arraySize("snapshots") == 2);
    auto entry = parsed.value().arrayItem("snapshots", 1);
    CHECK(entry.getString("label") == "Chapter 1 done");
    CHECK(entry.getString("note")  == "Finished the first chapter draft.");
}

TEST_CASE("normal project without Git — createProject/openProject work without GitProvider calls", "[integration][T-0010]") {
    TempDir projectDir;
    TempDir appSupportDir;

    scrivi::platform::LocalFileSystem        lfs;
    scrivi::mocks::DeterministicUUIDProvider uuids;
    scrivi::mocks::FixedClock                clock{"2026-05-20T00:00:00Z"};
    scrivi::mocks::MockSecureStore           store;
    scrivi::mocks::MockGitProvider           git;

    // enableGitSnapshots = false (default)
    auto services = makeServices(lfs, uuids, clock, store, &git);
    scrivi::ScriviCore core{services};

    auto created = createTestProject(core, projectDir.str(), appSupportDir.str());
    CHECK(created.gitInitialized == false);

    // Open the project — no git calls
    scrivi::OpenProjectRequest openReq;
    openReq.projectRootPath = projectDir.str();
    openReq.appSupportRoot  = appSupportDir.str();
    auto openResult = core.openProject(openReq);
    REQUIRE(openResult.ok());

    // No Git methods invoked during create or open
    CHECK(git.initCalls.empty());
    CHECK(git.addAllCalls.empty());
    CHECK(git.commitRequests.empty());
    CHECK(git.statusCalls.empty());
}

// ---------------------------------------------------------------------------
// SystemGitProvider integration tests — skip if git not in PATH
// ---------------------------------------------------------------------------

TEST_CASE("SystemGitProvider — real git: enable creates .git directory and initial commit", "[integration][T-0010][realGit]") {
    if (!scrivi::git::SystemGitProvider::available()) {
        SKIP("git not available in PATH");
    }

    TempDir projectDir;
    TempDir appSupportDir;

    scrivi::platform::LocalFileSystem        lfs;
    scrivi::mocks::DeterministicUUIDProvider uuids;
    scrivi::mocks::FixedClock                clock{"2026-05-20T00:00:00Z"};
    scrivi::mocks::MockSecureStore           store;
    scrivi::git::SystemGitProvider           git;

    auto services = makeServices(lfs, uuids, clock, store, &git);
    scrivi::ScriviCore core{services};

    createTestProject(core, projectDir.str(), appSupportDir.str());

    scrivi::EnableGitRequest req;
    req.projectRootPath      = projectDir.str();
    req.initialSnapshotLabel = "Initial project";
    req.author               = {
        scrivi::IdentityID{"identity-001"},
        scrivi::PersonaID {"persona-001"},
        "Test Author"
    };

    auto result = core.enableGitSnapshots(req);
    REQUIRE(result.ok());

    CHECK(result.value().gitInitialized == true);
    CHECK(fs::exists(projectDir.sub(".git")));
    CHECK_FALSE(result.value().initialCommitID.value.empty());
    // Commit hash is 40 hex chars
    CHECK(result.value().initialCommitID.value.size() == 40);
}

TEST_CASE("SystemGitProvider — real git: createSnapshot adds a new commit", "[integration][T-0010][realGit]") {
    if (!scrivi::git::SystemGitProvider::available()) {
        SKIP("git not available in PATH");
    }

    TempDir projectDir;
    TempDir appSupportDir;

    scrivi::platform::LocalFileSystem        lfs;
    scrivi::mocks::DeterministicUUIDProvider uuids;
    scrivi::mocks::FixedClock                clock{"2026-05-20T00:00:00Z"};
    scrivi::mocks::MockSecureStore           store;
    scrivi::git::SystemGitProvider           git;

    auto services = makeServices(lfs, uuids, clock, store, &git);
    scrivi::ScriviCore core{services};

    createTestProject(core, projectDir.str(), appSupportDir.str());

    scrivi::EnableGitRequest enableReq;
    enableReq.projectRootPath      = projectDir.str();
    enableReq.initialSnapshotLabel = "Initial project";
    enableReq.author = {scrivi::IdentityID{"identity-001"}, scrivi::PersonaID{"persona-001"}, "Test Author"};
    REQUIRE(core.enableGitSnapshots(enableReq).ok());

    // Write some content then snapshot
    scrivi::SaveSceneRequest saveReq;
    saveReq.projectID         = scrivi::ProjectID{"proj-001"};
    saveReq.projectRootPath   = projectDir.str();
    saveReq.appSupportRoot    = appSupportDir.str();
    saveReq.sceneID           = scrivi::SceneID{"scene-001"};
    saveReq.sceneMetadataPath = "manuscript/chapter-001/001-opening-scene.meta.json";
    saveReq.sceneContentPath  = "manuscript/chapter-001/001-opening-scene.md";
    saveReq.markdown          = "It was a dark and stormy night.";
    saveReq.author            = {scrivi::IdentityID{"identity-001"}, scrivi::PersonaID{"persona-001"}, "Test Author"};
    REQUIRE(core.saveScene(saveReq).ok());

    scrivi::CreateSnapshotRequest snapReq;
    snapReq.projectRootPath = projectDir.str();
    snapReq.label           = "After scene 1";
    snapReq.note            = "";
    snapReq.author          = {scrivi::IdentityID{"identity-001"}, scrivi::PersonaID{"persona-001"}, "Test Author"};

    auto result = core.createSnapshot(snapReq);
    REQUIRE(result.ok());

    CHECK(result.value().created == true);
    CHECK(result.value().commitID.value.size() == 40);

    // Snapshot metadata has two entries
    auto snapJson = readFile(projectDir.sub("snapshots/scrivi-snapshots.json"));
    auto parsed   = scrivi::util::parseJson(snapJson);
    REQUIRE(parsed.ok());
    CHECK(parsed.value().arraySize("snapshots") == 2);
}
