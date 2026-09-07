#include <catch2/catch_test_macros.hpp>

#include "scrivi/ScriviCore.hpp"
#include "scrivi/Requests.hpp"

#include "mocks/DeterministicUUIDProvider.hpp"
#include "mocks/FixedClock.hpp"
#include "mocks/MockGitProvider.hpp"
#include "mocks/MockSecureStore.hpp"

#include "platform/AppSupportLayout.hpp"
#include "platform/LocalFileSystem.hpp"

#include <algorithm>
#include <chrono>
#include <cstdlib>      // getenv / setenv / unsetenv — used by the platform tests below
#include <filesystem>
#include <string>
#include <vector>

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
// Apple platform coverage (T-0414, SP-106)
//
// WHY THIS BLOCK EXISTS. Until SP-106 the Apple branch of platformDefault() had
// NO test at all: Linux had 7 platform-specific tests and macOS had zero, so the
// only assertion covering Apple was the shared "non-empty and ends in Scrivi"
// case above — which passes for literally any path ending in "Scrivi", including
// the Linux one. The asymmetry was invisible because the shared test looked like
// coverage. These mirror the Linux XDG pair so both platforms assert the actual
// location rule they promise, not merely the leaf name.
// ---------------------------------------------------------------------------
#if defined(__APPLE__)
TEST_CASE("platformDefault - Apple returns ~/Library/Application Support/Scrivi",
          "[integration][T-0057][T-0414][apple]") {
    const char* original = std::getenv("HOME");
    REQUIRE(original != nullptr);          // the fallback path is covered below

    setenv("HOME", "/tmp/apple-home-test", 1);
    auto result = scrivi::util::platformDefault();
    REQUIRE(result.ok());
    CHECK(result.value() == "/tmp/apple-home-test/Library/Application Support/Scrivi");

    setenv("HOME", original, 1);
}

TEST_CASE("platformDefault - Apple honours HOME and lands under Application Support",
          "[integration][T-0057][T-0414][apple]") {
    auto result = scrivi::util::platformDefault();
    REQUIRE(result.ok());
    fs::path p{result.value()};

    // Assert the whole documented shape, not just the leaf: .../Library/Application Support/Scrivi
    CHECK(p.filename()                             == "Scrivi");
    CHECK(p.parent_path().filename()               == "Application Support");
    CHECK(p.parent_path().parent_path().filename() == "Library");
    CHECK(p.is_absolute());
}

TEST_CASE("platformDefault - Apple falls back to the passwd home when HOME is unset",
          "[integration][T-0057][T-0414][apple]") {
    // The getpwuid() fallback is a real branch that nothing exercised. An empty
    // HOME must behave like an unset one (the guard tests home[0] == '\0').
    const char* original = std::getenv("HOME");
    unsetenv("HOME");

    auto result = scrivi::util::platformDefault();
    REQUIRE(result.ok());
    fs::path p{result.value()};
    CHECK(p.filename()               == "Scrivi");
    CHECK(p.parent_path().filename() == "Application Support");
    CHECK(p.is_absolute());

    if (original) setenv("HOME", original, 1);
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

// ---------------------------------------------------------------------------
// I-0191 — an invalid appSupportRoot must be REJECTED, not created
// ---------------------------------------------------------------------------
//
// Three directories named from raw control bytes appeared in the repository root
// on 2026-08-17, each holding the full app-support skeleton. `bootstrapAppSupport`
// validated its root in no way: an empty or relative path resolves against the
// process working directory, so a bad root created the tree in the CWD and
// reported SUCCESS.
//
// These are RED-then-GREEN against that exact byte string.

namespace {

// The literal name of one of the observed directories: \020 v \017 k \001.
const std::string kJunkBytes = std::string("\x10v\x0Fk\x01", 5);

// Entries directly under the CWD, so a test can prove it created nothing there.
std::vector<std::string> cwdEntries() {
    std::vector<std::string> names;
    std::error_code ec;
    for (const auto& e : fs::directory_iterator(fs::current_path(), ec)) {
        names.push_back(e.path().filename().string());
    }
    std::sort(names.begin(), names.end());
    return names;
}

} // namespace

TEST_CASE("bootstrapAppSupport - rejects a root of unprintable bytes and creates NOTHING",
          "[integration][I-0191]") {
    scrivi::platform::LocalFileSystem lfs;

    const auto before = cwdEntries();

    auto r = scrivi::util::bootstrapAppSupport(kJunkBytes, lfs);

    REQUIRE_FALSE(r.ok());
    CHECK(r.error().code == scrivi::ErrorCode::invalidArgument);

    // The whole point: nothing was created anywhere, least of all in the CWD.
    CHECK_FALSE(fs::exists(fs::path(kJunkBytes)));
    CHECK(cwdEntries() == before);
}

TEST_CASE("bootstrapAppSupport - rejects an EMPTY root instead of writing to the CWD",
          "[integration][I-0191]") {
    scrivi::platform::LocalFileSystem lfs;

    // An empty root is what the C ABI's S(NULL) used to produce. `join("", "identity")`
    // is the RELATIVE path "identity", so this once created ./identity in the CWD.
    const auto before = cwdEntries();

    auto r = scrivi::util::bootstrapAppSupport("", lfs);

    REQUIRE_FALSE(r.ok());
    CHECK(r.error().code == scrivi::ErrorCode::invalidArgument);
    CHECK_FALSE(fs::exists("identity"));
    CHECK_FALSE(fs::exists("logs"));
    CHECK(cwdEntries() == before);
}

TEST_CASE("bootstrapAppSupport - rejects a RELATIVE root", "[integration][I-0191]") {
    scrivi::platform::LocalFileSystem lfs;

    const auto before = cwdEntries();

    auto r = scrivi::util::bootstrapAppSupport("some-relative-dir", lfs);

    REQUIRE_FALSE(r.ok());
    CHECK(r.error().code == scrivi::ErrorCode::invalidArgument);
    CHECK_FALSE(fs::exists("some-relative-dir"));
    CHECK(cwdEntries() == before);
}

TEST_CASE("LocalFileSystem - refuses to create ANY directory holding control characters",
          "[integration][I-0191]") {
    scrivi::platform::LocalFileSystem lfs;
    TempDir tmp;

    // AC2 is enforced at the FileSystem chokepoint, so it holds for every core
    // write path — not just app-support bootstrap. An absolute, otherwise-valid
    // parent with a junk leaf must still be refused.
    const auto target = (tmp.path / kJunkBytes).string();

    auto r = lfs.createDirectories(target);

    REQUIRE_FALSE(r.ok());
    CHECK(r.error().code == scrivi::ErrorCode::invalidArgument);
    CHECK_FALSE(fs::exists(fs::path(target)));
}

TEST_CASE("LocalFileSystem - still accepts a valid non-ASCII (UTF-8) directory name",
          "[integration][I-0191]") {
    scrivi::platform::LocalFileSystem lfs;
    TempDir tmp;

    // Guards the signedness trap: UTF-8 continuation bytes are >= 0x80 and would
    // sign-extend negative on a platform with signed `char`, tripping a naive
    // `c < 0x20` test and rejecting perfectly legitimate paths.
    const auto target = (tmp.path / "Ünïcode-Ω-世界").string();

    auto r = lfs.createDirectories(target);

    REQUIRE(r.ok());
    CHECK(fs::is_directory(fs::path(target)));
}
