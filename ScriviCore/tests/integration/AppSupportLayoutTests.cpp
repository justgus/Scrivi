#include <catch2/catch_test_macros.hpp>

#include "scrivi/ScriviCore.hpp"
#include "scrivi/Requests.hpp"

#include "mocks/DeterministicUUIDProvider.hpp"
#include "mocks/FixedClock.hpp"
#include "mocks/MockGitProvider.hpp"
#include "mocks/MockSecureStore.hpp"

#include "platform/AppSupportLayout.hpp"
#include "platform/LocalFileSystem.hpp"

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
        path = fs::temp_directory_path() / ("scrivi-test-" + std::to_string(
            std::chrono::steady_clock::now().time_since_epoch().count()));
        fs::create_directories(path);
    }

    ~TempDir() {
        std::error_code ec;
        fs::remove_all(path, ec);
    }

    std::string str() const { return path.string(); }
    bool hasDir(const std::string& rel) const {
        return fs::is_directory(path / rel);
    }
};

static scrivi::CoreServices makeServices(
    scrivi::platform::LocalFileSystem& localFs,
    scrivi::mocks::DeterministicUUIDProvider& uuids,
    scrivi::mocks::FixedClock& clock,
    scrivi::mocks::MockSecureStore& store,
    scrivi::mocks::MockGitProvider& git)
{
    scrivi::CoreServices s;
    s.fileSystem   = &localFs;
    s.uuidProvider = &uuids;
    s.clock        = &clock;
    s.secureStore  = &store;
    s.gitProvider  = &git;
    s.logger       = nullptr;
    return s;
}

// ---------------------------------------------------------------------------
// Tests - T-0057: platformDefault()
// ---------------------------------------------------------------------------

TEST_CASE("platformDefault - returns a non-empty path ending in 'Scrivi'",
          "[integration][T-0057]") {
    auto result = scrivi::util::platformDefault();
    REQUIRE(result.ok());
    std::filesystem::path p{result.value()};
    CHECK(!result.value().empty());
    CHECK(p.filename() == "Scrivi");
}

#if defined(__linux__)
TEST_CASE("platformDefault - Linux respects XDG_DATA_HOME when set",
          "[integration][T-0057][linux]") {
    // Save original value
    const char* original = std::getenv("XDG_DATA_HOME");

    setenv("XDG_DATA_HOME", "/tmp/xdg-test", 1);
    auto result = scrivi::util::platformDefault();
    REQUIRE(result.ok());
    CHECK(result.value() == "/tmp/xdg-test/Scrivi");

    // Restore
    if (original) setenv("XDG_DATA_HOME", original, 1);
    else           unsetenv("XDG_DATA_HOME");
}

TEST_CASE("platformDefault - Linux falls back to ~/.local/share when XDG_DATA_HOME unset",
          "[integration][T-0057][linux]") {
    const char* original = std::getenv("XDG_DATA_HOME");
    unsetenv("XDG_DATA_HOME");

    auto result = scrivi::util::platformDefault();
    REQUIRE(result.ok());
    std::filesystem::path p{result.value()};
    CHECK(p.filename() == "Scrivi");
    CHECK(p.parent_path().filename() == "share");
    CHECK(p.parent_path().parent_path().filename() == ".local");

    if (original) setenv("XDG_DATA_HOME", original, 1);
}
#endif

// ---------------------------------------------------------------------------
// Tests - T-0013: appSupportRoot Directory Bootstrap
// ---------------------------------------------------------------------------

TEST_CASE("bootstrap - all required subdirs created on empty appSupportRoot",
          "[integration][T-0013]") {
    TempDir appSupportDir;

    scrivi::platform::LocalFileSystem      localFs;
    scrivi::mocks::DeterministicUUIDProvider uuids;
    scrivi::mocks::FixedClock              clock{"2026-05-20T00:00:00Z"};
    scrivi::mocks::MockSecureStore         store;
    scrivi::mocks::MockGitProvider         git;

    auto services = makeServices(localFs, uuids, clock, store, git);
    scrivi::ScriviCore core{services};

    // Trigger bootstrap via ensureLocalIdentity (will fail with "not
    // implemented" after bootstrap, but bootstrap itself must have run)
    scrivi::EnsureIdentityRequest req;
    req.requestedDisplayName = "Test Author";
    req.appSupportRoot       = appSupportDir.str();

    core.ensureLocalIdentity(req);

    CHECK(appSupportDir.hasDir("identity"));
    CHECK(appSupportDir.hasDir("state/projects"));
    CHECK(appSupportDir.hasDir("cache/projects"));
    CHECK(appSupportDir.hasDir("logs"));
    CHECK(appSupportDir.hasDir("tmp"));
}

TEST_CASE("bootstrap - idempotent when dirs already exist",
          "[integration][T-0013]") {
    TempDir appSupportDir;

    scrivi::platform::LocalFileSystem      localFs;
    scrivi::mocks::DeterministicUUIDProvider uuids;
    scrivi::mocks::FixedClock              clock{"2026-05-20T00:00:00Z"};
    scrivi::mocks::MockSecureStore         store;
    scrivi::mocks::MockGitProvider         git;

    auto services = makeServices(localFs, uuids, clock, store, git);
    scrivi::ScriviCore core{services};

    scrivi::EnsureIdentityRequest req;
    req.requestedDisplayName = "Test Author";
    req.appSupportRoot       = appSupportDir.str();

    // First call
    REQUIRE(core.ensureLocalIdentity(req).ok());
    // Second call - must also succeed and not error
    auto result = core.ensureLocalIdentity(req);
    REQUIRE(result.ok());
    CHECK(result.value().createdNewIdentity == false);
}

TEST_CASE("bootstrap - createProject triggers bootstrap on fresh appSupportRoot",
          "[integration][T-0013]") {
    TempDir projectDir;
    TempDir appSupportDir;

    scrivi::platform::LocalFileSystem      localFs;
    scrivi::mocks::DeterministicUUIDProvider uuids;
    scrivi::mocks::FixedClock              clock{"2026-05-20T00:00:00Z"};
    scrivi::mocks::MockSecureStore         store;
    scrivi::mocks::MockGitProvider         git;

    auto services = makeServices(localFs, uuids, clock, store, git);
    scrivi::ScriviCore core{services};

    scrivi::CreateProjectRequest req;
    req.projectRootPath = projectDir.str();
    req.appSupportRoot  = appSupportDir.str();
    req.title           = "Bootstrap Novel";
    req.slug            = "bootstrap-novel";
    req.author          = {
        scrivi::IdentityID{"identity-001"},
        scrivi::PersonaID {"persona-001"},
        "Test Author"
    };

    auto result = core.createProject(req);

    REQUIRE(result.ok());
    CHECK(appSupportDir.hasDir("state/projects"));
    CHECK(appSupportDir.hasDir("cache/projects"));
    CHECK(appSupportDir.hasDir("logs"));
    CHECK(appSupportDir.hasDir("tmp"));
}

TEST_CASE("bootstrap - openProject triggers bootstrap on fresh appSupportRoot",
          "[integration][T-0013]") {
    TempDir projectDir;
    TempDir appSupportDir;

    scrivi::platform::LocalFileSystem      localFs;
    scrivi::mocks::DeterministicUUIDProvider uuids;
    scrivi::mocks::FixedClock              clock{"2026-05-20T00:00:00Z"};
    scrivi::mocks::MockSecureStore         store;
    scrivi::mocks::MockGitProvider         git;

    auto services = makeServices(localFs, uuids, clock, store, git);
    scrivi::ScriviCore core{services};

    // Create first so there is a valid project to open
    scrivi::CreateProjectRequest createReq;
    createReq.projectRootPath = projectDir.str();
    createReq.appSupportRoot  = appSupportDir.str();
    createReq.title           = "Bootstrap Novel";
    createReq.slug            = "bootstrap-novel";
    createReq.author          = {
        scrivi::IdentityID{"identity-001"},
        scrivi::PersonaID {"persona-001"},
        "Test Author"
    };
    REQUIRE(core.createProject(createReq).ok());

    // Now open from a fresh appSupportDir to prove openProject bootstraps too
    TempDir freshAppSupport;
    scrivi::OpenProjectRequest openReq;
    openReq.projectRootPath = projectDir.str();
    openReq.appSupportRoot  = freshAppSupport.str();

    auto result = core.openProject(openReq);

    REQUIRE(result.ok());
    CHECK(freshAppSupport.hasDir("state/projects"));
    CHECK(freshAppSupport.hasDir("cache/projects"));
}
