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
// Helpers
// ---------------------------------------------------------------------------

struct TempDir {
    fs::path path;

    TempDir() {
        path = fs::temp_directory_path() / ("scrivi-scan-test-" + std::to_string(
            std::chrono::steady_clock::now().time_since_epoch().count()));
        fs::create_directories(path);
    }

    ~TempDir() {
        std::error_code ec;
        fs::remove_all(path, ec);
    }

    std::string str()                        const { return path.string(); }
    std::string sub(const std::string& rel)  const { return (path / rel).string(); }
};

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
    req.title           = "Scan Test";
    req.slug            = "scan-test";
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

TEST_CASE("scanForExternalChanges - clean project returns zero issues", "[integration][T-0009]") {
    TempDir projectDir;
    TempDir appSupportDir;

    scrivi::platform::LocalFileSystem        lfs;
    scrivi::mocks::DeterministicUUIDProvider uuids;
    scrivi::mocks::FixedClock                clock{"2026-05-20T00:00:00Z"};
    scrivi::mocks::MockSecureStore           store;
    scrivi::mocks::MockGitProvider           git;

    auto services = makeServices(lfs, uuids, clock, store, git);
    scrivi::ScriviCore core{services};

    createTestProject(core, projectDir.str(), appSupportDir.str());

    scrivi::ExternalChangeScanRequest req;
    req.projectRootPath  = projectDir.str();
    req.appSupportRoot   = appSupportDir.str();
    req.includeGitStatus = false;

    auto result = core.scanForExternalChanges(req);
    REQUIRE(result.ok());
    CHECK(result.value().repairIssues.empty());
    CHECK(result.value().indexesDirty == false);
}

TEST_CASE("scanForExternalChanges - missing scene .md returns missingContent", "[integration][T-0009]") {
    TempDir projectDir;
    TempDir appSupportDir;

    scrivi::platform::LocalFileSystem        lfs;
    scrivi::mocks::DeterministicUUIDProvider uuids;
    scrivi::mocks::FixedClock                clock{"2026-05-20T00:00:00Z"};
    scrivi::mocks::MockSecureStore           store;
    scrivi::mocks::MockGitProvider           git;

    auto services = makeServices(lfs, uuids, clock, store, git);
    scrivi::ScriviCore core{services};

    createTestProject(core, projectDir.str(), appSupportDir.str());

    // Remove the scene content file
    fs::remove(projectDir.sub("manuscript/chapter-001/001-opening-scene.md"));

    scrivi::ExternalChangeScanRequest req;
    req.projectRootPath  = projectDir.str();
    req.appSupportRoot   = appSupportDir.str();
    req.includeGitStatus = false;

    auto result = core.scanForExternalChanges(req);
    REQUIRE(result.ok());

    auto& issues = result.value().repairIssues;
    REQUIRE(issues.size() == 1);
    CHECK(issues[0].category == scrivi::RepairCategory::missingContent);
    CHECK(issues[0].severity == scrivi::RepairSeverity::blocking);
    CHECK_FALSE(issues[0].suggestedActions.empty());
}

TEST_CASE("scanForExternalChanges - missing scene .meta.json returns missingMetadata", "[integration][T-0009]") {
    TempDir projectDir;
    TempDir appSupportDir;

    scrivi::platform::LocalFileSystem        lfs;
    scrivi::mocks::DeterministicUUIDProvider uuids;
    scrivi::mocks::FixedClock                clock{"2026-05-20T00:00:00Z"};
    scrivi::mocks::MockSecureStore           store;
    scrivi::mocks::MockGitProvider           git;

    auto services = makeServices(lfs, uuids, clock, store, git);
    scrivi::ScriviCore core{services};

    createTestProject(core, projectDir.str(), appSupportDir.str());

    // Remove the scene metadata file
    fs::remove(projectDir.sub("manuscript/chapter-001/001-opening-scene.meta.json"));

    scrivi::ExternalChangeScanRequest req;
    req.projectRootPath  = projectDir.str();
    req.appSupportRoot   = appSupportDir.str();
    req.includeGitStatus = false;

    auto result = core.scanForExternalChanges(req);
    REQUIRE(result.ok());

    auto& issues = result.value().repairIssues;
    REQUIRE(issues.size() == 1);
    CHECK(issues[0].category == scrivi::RepairCategory::missingMetadata);
    CHECK(issues[0].severity == scrivi::RepairSeverity::blocking);
}

TEST_CASE("scanForExternalChanges - corrupt scene .meta.json returns corruptMetadata", "[integration][T-0009]") {
    TempDir projectDir;
    TempDir appSupportDir;

    scrivi::platform::LocalFileSystem        lfs;
    scrivi::mocks::DeterministicUUIDProvider uuids;
    scrivi::mocks::FixedClock                clock{"2026-05-20T00:00:00Z"};
    scrivi::mocks::MockSecureStore           store;
    scrivi::mocks::MockGitProvider           git;

    auto services = makeServices(lfs, uuids, clock, store, git);
    scrivi::ScriviCore core{services};

    createTestProject(core, projectDir.str(), appSupportDir.str());

    // Corrupt the scene metadata
    {
        std::ofstream f(projectDir.sub("manuscript/chapter-001/001-opening-scene.meta.json"), std::ios::binary);
        f << "{ this is not valid json !!!";
    }

    scrivi::ExternalChangeScanRequest req;
    req.projectRootPath  = projectDir.str();
    req.appSupportRoot   = appSupportDir.str();
    req.includeGitStatus = false;

    auto result = core.scanForExternalChanges(req);
    REQUIRE(result.ok());

    auto& issues = result.value().repairIssues;
    REQUIRE(issues.size() == 1);
    CHECK(issues[0].category == scrivi::RepairCategory::corruptMetadata);
    CHECK(issues[0].severity == scrivi::RepairSeverity::blocking);
}

TEST_CASE("scanForExternalChanges - unregistered .md file returns unregisteredManuscriptFile", "[integration][T-0009]") {
    TempDir projectDir;
    TempDir appSupportDir;

    scrivi::platform::LocalFileSystem        lfs;
    scrivi::mocks::DeterministicUUIDProvider uuids;
    scrivi::mocks::FixedClock                clock{"2026-05-20T00:00:00Z"};
    scrivi::mocks::MockSecureStore           store;
    scrivi::mocks::MockGitProvider           git;

    auto services = makeServices(lfs, uuids, clock, store, git);
    scrivi::ScriviCore core{services};

    createTestProject(core, projectDir.str(), appSupportDir.str());

    // Drop an unregistered .md file into the chapter directory
    {
        std::ofstream f(projectDir.sub("manuscript/chapter-001/interloper.md"), std::ios::binary);
        f << "An external scene added outside of Scrivi.";
    }

    scrivi::ExternalChangeScanRequest req;
    req.projectRootPath  = projectDir.str();
    req.appSupportRoot   = appSupportDir.str();
    req.includeGitStatus = false;

    auto result = core.scanForExternalChanges(req);
    REQUIRE(result.ok());

    auto& issues = result.value().repairIssues;
    REQUIRE(issues.size() == 1);
    CHECK(issues[0].category == scrivi::RepairCategory::unregisteredManuscriptFile);
    CHECK(issues[0].severity == scrivi::RepairSeverity::warning);
    CHECK(result.value().indexesDirty == true);
    CHECK_FALSE(issues[0].suggestedActions.empty());
}

TEST_CASE("scanForExternalChanges - Git status checked when requested and repo exists", "[integration][T-0009]") {
    TempDir projectDir;
    TempDir appSupportDir;

    scrivi::platform::LocalFileSystem        lfs;
    scrivi::mocks::DeterministicUUIDProvider uuids;
    scrivi::mocks::FixedClock                clock{"2026-05-20T00:00:00Z"};
    scrivi::mocks::MockSecureStore           store;
    scrivi::mocks::MockGitProvider           git;

    // Pretend it's already a repo with uncommitted changes
    git.repoExists = true;

    auto services = makeServices(lfs, uuids, clock, store, git);
    scrivi::ScriviCore core{services};

    createTestProject(core, projectDir.str(), appSupportDir.str());

    scrivi::ExternalChangeScanRequest req;
    req.projectRootPath  = projectDir.str();
    req.appSupportRoot   = appSupportDir.str();
    req.includeGitStatus = true;

    auto result = core.scanForExternalChanges(req);
    REQUIRE(result.ok());

    CHECK(result.value().gitStatusChecked == true);
    // MockGitProvider returns hasUncommittedChanges=false by default
    CHECK(result.value().hasUnsnapshottedChanges == false);
    CHECK(git.statusCalls.size() == 1);
}
