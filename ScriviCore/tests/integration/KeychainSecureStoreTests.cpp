// KeychainSecureStore tests ([I-0216], SP-151).
//
// Apple-only: the store is compiled only where SCRIVI_HAS_KEYCHAIN_SECURE_STORE is
// defined (ScriviCore/CMakeLists.txt). Guarded so the Linux build compiles this TU to
// nothing.
//
// ⚠️ THESE TOUCH THE REAL LOGIN KEYCHAIN of whoever runs them — under a TEST service
// name, never the app's (`kDefaultService`), and each removes what it creates.
// ✅ So they are HIDDEN (`[.keychain]`): a routine `ctest`, or the macOS CI job, does
// not run them. Run explicitly:   ScriviCoreTests "[keychain]"
#if defined(SCRIVI_HAS_KEYCHAIN_SECURE_STORE)

#include <catch2/catch_test_macros.hpp>

#include "mocks/DeterministicUUIDProvider.hpp"
#include "mocks/FixedClock.hpp"
#include "mocks/MockGitProvider.hpp"
#include "platform/KeychainSecureStore.hpp"
#include "platform/LocalFileSystem.hpp"
#include "scrivi/Requests.hpp"
#include "scrivi/ScriviCore.hpp"

#include <chrono>
#include <cstring>
#include <filesystem>
#include <string>
#include <unistd.h>

namespace fs = std::filesystem;
using scrivi::platform::KeychainSecureStore;

namespace {

// ⚠️ Unique per run, so two runs (or a crashed one) can never collide, and ⛔ never
// equal to the production service.
std::string testService()
{
    return "com.caposoft.scrivi.securestore.test."
         + std::to_string(::getpid()) + "."
         + std::to_string(std::chrono::steady_clock::now().time_since_epoch().count());
}

scrivi::SecretBytes bytesOf(std::string_view s)
{
    scrivi::SecretBytes b(s.size());
    std::memcpy(b.data(), s.data(), s.size());
    return b;
}

std::string stringOf(const scrivi::SecretBytes& b)
{
    return {reinterpret_cast<const char*>(b.data()), b.size()};
}

// Removes the item on scope exit, pass or fail.
struct ItemCleanup {
    std::string service;
    std::string key;
    ~ItemCleanup() { (void)KeychainSecureStore(service).removeSecret(key); }
};

} // namespace

TEST_CASE("KeychainSecureStore - the test service is never the app's",
          "[.keychain][I-0216]") {
    REQUIRE(testService() != KeychainSecureStore::kDefaultService);
}

TEST_CASE("KeychainSecureStore - an absent key reads as ABSENT, not as an error",
          "[.keychain][I-0216]") {
    KeychainSecureStore store(testService());
    auto has = store.containsSecret("never.written");
    REQUIRE(has.ok());
    REQUIRE_FALSE(has.value());
    REQUIRE_FALSE(store.getSecret("never.written").ok());
}

TEST_CASE("KeychainSecureStore - a SECOND instance reads what the first wrote",
          "[.keychain][I-0216]") {
    // ✅ THE PERSISTENCE PROOF. The two stores share NO memory, so the second can only
    // answer from the Keychain itself. ⛔ The old in-memory store fails exactly here.
    const auto service = testService();
    ItemCleanup cleanup{service, "scrivi.identity.v1"};

    REQUIRE(KeychainSecureStore(service).putSecret("scrivi.identity.v1",
                                                   bytesOf("{\"identityID\":\"x\"}")).ok());

    KeychainSecureStore fresh(service);
    auto has = fresh.containsSecret("scrivi.identity.v1");
    REQUIRE(has.ok());
    REQUIRE(has.value());
    auto got = fresh.getSecret("scrivi.identity.v1");
    REQUIRE(got.ok());
    REQUIRE(stringOf(got.value()) == "{\"identityID\":\"x\"}");
}

TEST_CASE("KeychainSecureStore - writing an existing key REPLACES it",
          "[.keychain][I-0216]") {
    // ⚠️ Exercises the SecItemUpdate path; ⛔ SecItemAdd alone would fail with
    // errSecDuplicateItem on the second write.
    const auto service = testService();
    ItemCleanup cleanup{service, "k"};
    KeychainSecureStore store(service);
    REQUIRE(store.putSecret("k", bytesOf("first")).ok());
    REQUIRE(store.putSecret("k", bytesOf("second")).ok());
    REQUIRE(stringOf(store.getSecret("k").value()) == "second");
}

TEST_CASE("KeychainSecureStore - the identity SURVIVES a new core instance ([I-0216])",
          "[.keychain][I-0216]") {
    // ✅ The defect, end to end: `ensureLocalIdentity` on a SECOND core with a SECOND
    // store must FIND the identity the first one minted.
    // ⚠️ `createdNewIdentity` is the discriminating assertion: DeterministicUUIDProvider
    // restarts its sequence per instance, so a re-mint would produce the SAME id and an
    // id comparison alone would pass for the wrong reason.
    const auto service = testService();
    ItemCleanup cleanup{service, "scrivi.identity.v1"};

    const fs::path appSupport = fs::temp_directory_path() / ("scrivi-kc-" + service);
    fs::create_directories(appSupport);

    auto ensure = [&](bool expectNew) {
        scrivi::platform::LocalFileSystem        lfs;
        scrivi::mocks::DeterministicUUIDProvider uuids;
        scrivi::mocks::FixedClock                clock{"2026-09-30T12:00:00Z"};
        scrivi::mocks::MockGitProvider           git;
        KeychainSecureStore                      store(service);
        scrivi::CoreServices svc;
        svc.fileSystem   = &lfs;
        svc.uuidProvider = &uuids;
        svc.secureStore  = &store;
        svc.clock        = &clock;
        svc.gitProvider  = &git;
        scrivi::ScriviCore core{svc};

        scrivi::EnsureIdentityRequest req;
        req.requestedDisplayName = "Ada";
        req.appSupportRoot       = appSupport.string();
        auto r = core.ensureLocalIdentity(req);
        REQUIRE(r.ok());
        REQUIRE(r.value().createdNewIdentity == expectNew);
        return r.value().identityID.value;
    };

    const auto first  = ensure(/*expectNew=*/true);
    const auto second = ensure(/*expectNew=*/false);
    REQUIRE(second == first);

    std::error_code ec;
    fs::remove_all(appSupport, ec);
}

#endif // SCRIVI_HAS_KEYCHAIN_SECURE_STORE
