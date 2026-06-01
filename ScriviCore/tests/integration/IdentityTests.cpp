#include <catch2/catch_test_macros.hpp>

#include "scrivi/ScriviCore.hpp"
#include "scrivi/Requests.hpp"

#include "mocks/DeterministicUUIDProvider.hpp"
#include "mocks/FixedClock.hpp"
#include "mocks/MockGitProvider.hpp"
#include "mocks/MockSecureStore.hpp"

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
};

// Failing SecureStore: putSecret always returns an error.
class FailingSecureStore final : public scrivi::SecureStore {
public:
    scrivi::Result<bool> containsSecret(std::string_view) override {
        return scrivi::Result<bool>::success(false);
    }
    scrivi::Result<void> putSecret(std::string_view, const scrivi::SecretBytes&) override {
        return scrivi::Result<void>::failure(
            {scrivi::ErrorCode::secureStoreError, "simulated failure"});
    }
    scrivi::Result<scrivi::SecretBytes> getSecret(std::string_view) override {
        return scrivi::Result<scrivi::SecretBytes>::failure(
            {scrivi::ErrorCode::secureStoreError, "simulated failure"});
    }
};

static scrivi::CoreServices makeServices(
    scrivi::platform::LocalFileSystem& localFs,
    scrivi::mocks::DeterministicUUIDProvider& uuids,
    scrivi::mocks::FixedClock& clock,
    scrivi::SecureStore& store,
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
// Tests - T-0012: Identity Service and UUID Provider
// ---------------------------------------------------------------------------

TEST_CASE("ensureLocalIdentity - creates new identity on first call",
          "[integration][T-0012]") {
    TempDir appSupportDir;

    scrivi::platform::LocalFileSystem        localFs;
    scrivi::mocks::DeterministicUUIDProvider uuids;
    scrivi::mocks::FixedClock                clock{"2026-05-20T00:00:00Z"};
    scrivi::mocks::MockSecureStore           store;
    scrivi::mocks::MockGitProvider           git;

    auto services = makeServices(localFs, uuids, clock, store, git);
    scrivi::ScriviCore core{services};

    scrivi::EnsureIdentityRequest req;
    req.requestedDisplayName = "Rhozwyn Darius";
    req.appSupportRoot       = appSupportDir.str();

    auto result = core.ensureLocalIdentity(req);

    REQUIRE(result.ok());
    CHECK(result.value().createdNewIdentity == true);
    CHECK_FALSE(result.value().identityID.value.empty());
    CHECK_FALSE(result.value().defaultPersonaID.value.empty());
    CHECK(result.value().displayName == "Rhozwyn Darius");
}

TEST_CASE("ensureLocalIdentity - DeterministicUUIDProvider gives prefixed IDs",
          "[integration][T-0012]") {
    TempDir appSupportDir;

    scrivi::platform::LocalFileSystem        localFs;
    scrivi::mocks::DeterministicUUIDProvider uuids;
    scrivi::mocks::FixedClock                clock{"2026-05-20T00:00:00Z"};
    scrivi::mocks::MockSecureStore           store;
    scrivi::mocks::MockGitProvider           git;

    auto services = makeServices(localFs, uuids, clock, store, git);
    scrivi::ScriviCore core{services};

    scrivi::EnsureIdentityRequest req;
    req.requestedDisplayName = "Test Author";
    req.appSupportRoot       = appSupportDir.str();

    auto result = core.ensureLocalIdentity(req);

    REQUIRE(result.ok());
    // DeterministicUUIDProvider produces "identity-001", "persona-001"
    CHECK(result.value().identityID.value    == "identity-001");
    CHECK(result.value().defaultPersonaID.value == "persona-001");
}

TEST_CASE("ensureLocalIdentity - idempotent: second call returns same identity",
          "[integration][T-0012]") {
    TempDir appSupportDir;

    scrivi::platform::LocalFileSystem        localFs;
    scrivi::mocks::DeterministicUUIDProvider uuids;
    scrivi::mocks::FixedClock                clock{"2026-05-20T00:00:00Z"};
    scrivi::mocks::MockSecureStore           store;
    scrivi::mocks::MockGitProvider           git;

    auto services = makeServices(localFs, uuids, clock, store, git);
    scrivi::ScriviCore core{services};

    scrivi::EnsureIdentityRequest req;
    req.requestedDisplayName = "Rhozwyn Darius";
    req.appSupportRoot       = appSupportDir.str();

    auto first  = core.ensureLocalIdentity(req);
    auto second = core.ensureLocalIdentity(req);

    REQUIRE(first.ok());
    REQUIRE(second.ok());

    CHECK(second.value().createdNewIdentity == false);
    CHECK(second.value().identityID.value    == first.value().identityID.value);
    CHECK(second.value().defaultPersonaID.value == first.value().defaultPersonaID.value);
    CHECK(second.value().displayName         == first.value().displayName);
}

TEST_CASE("ensureLocalIdentity - SecureStore failure returns secureStoreError",
          "[integration][T-0012]") {
    TempDir appSupportDir;

    scrivi::platform::LocalFileSystem        localFs;
    scrivi::mocks::DeterministicUUIDProvider uuids;
    scrivi::mocks::FixedClock                clock{"2026-05-20T00:00:00Z"};
    FailingSecureStore                       store;
    scrivi::mocks::MockGitProvider           git;

    auto services = makeServices(localFs, uuids, clock, store, git);
    scrivi::ScriviCore core{services};

    scrivi::EnsureIdentityRequest req;
    req.requestedDisplayName = "Test Author";
    req.appSupportRoot       = appSupportDir.str();

    auto result = core.ensureLocalIdentity(req);

    REQUIRE_FALSE(result.ok());
    CHECK(result.error().code == scrivi::ErrorCode::secureStoreError);
}

TEST_CASE("ensureLocalIdentity - identity not written inside appSupportRoot as plaintext",
          "[integration][T-0012]") {
    TempDir appSupportDir;

    scrivi::platform::LocalFileSystem        localFs;
    scrivi::mocks::DeterministicUUIDProvider uuids;
    scrivi::mocks::FixedClock                clock{"2026-05-20T00:00:00Z"};
    scrivi::mocks::MockSecureStore           store;
    scrivi::mocks::MockGitProvider           git;

    auto services = makeServices(localFs, uuids, clock, store, git);
    scrivi::ScriviCore core{services};

    scrivi::EnsureIdentityRequest req;
    req.requestedDisplayName = "Rhozwyn Darius";
    req.appSupportRoot       = appSupportDir.str();

    REQUIRE(core.ensureLocalIdentity(req).ok());

    // The identity/ subdir must exist (bootstrap creates it) but must be empty —
    // no plaintext identity files should be written there.
    auto identityDir = fs::path(appSupportDir.str()) / "identity";
    REQUIRE(fs::is_directory(identityDir));
    CHECK(fs::is_empty(identityDir));
}

TEST_CASE("ensureLocalIdentity - createProject accepts AuthorshipRef from result",
          "[integration][T-0012]") {
    TempDir projectDir;
    TempDir appSupportDir;

    scrivi::platform::LocalFileSystem        localFs;
    scrivi::mocks::DeterministicUUIDProvider uuids;
    scrivi::mocks::FixedClock                clock{"2026-05-20T00:00:00Z"};
    scrivi::mocks::MockSecureStore           store;
    scrivi::mocks::MockGitProvider           git;

    auto services = makeServices(localFs, uuids, clock, store, git);
    scrivi::ScriviCore core{services};

    scrivi::EnsureIdentityRequest idReq;
    idReq.requestedDisplayName = "Rhozwyn Darius";
    idReq.appSupportRoot       = appSupportDir.str();

    auto idResult = core.ensureLocalIdentity(idReq);
    REQUIRE(idResult.ok());

    scrivi::CreateProjectRequest projReq;
    projReq.projectRootPath = projectDir.str();
    projReq.appSupportRoot  = appSupportDir.str();
    projReq.title           = "The Long Road";
    projReq.slug            = "the-long-road";
    projReq.author          = {
        idResult.value().identityID,
        idResult.value().defaultPersonaID,
        idResult.value().displayName
    };

    auto projResult = core.createProject(projReq);
    REQUIRE(projResult.ok());
}
