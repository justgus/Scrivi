#include <catch2/catch_test_macros.hpp>

#include "mocks/FixedClock.hpp"
#include "mocks/DeterministicUUIDProvider.hpp"
#include "mocks/MockSecureStore.hpp"
#include "mocks/MockGitProvider.hpp"

using namespace scrivi;
using namespace scrivi::mocks;

TEST_CASE("FixedClock returns configured timestamp", "[mocks]") {
    FixedClock clock{"2026-05-19T12:00:00Z"};
    REQUIRE(clock.nowUTC() == "2026-05-19T12:00:00Z");
    REQUIRE(clock.nowUTC() == "2026-05-19T12:00:00Z"); // stable across calls
}

TEST_CASE("DeterministicUUIDProvider produces sequential IDs", "[mocks]") {
    DeterministicUUIDProvider uuids;
    REQUIRE(uuids.newProjectID().value    == "proj-001");
    REQUIRE(uuids.newProjectID().value    == "proj-002");
    REQUIRE(uuids.newSceneID().value      == "scene-001");
    REQUIRE(uuids.newChapterID().value    == "ch-001");
    REQUIRE(uuids.newManuscriptID().value == "ms-001");
}

TEST_CASE("DeterministicUUIDProvider counters are independent per type", "[mocks]") {
    DeterministicUUIDProvider uuids;
    uuids.newProjectID();
    uuids.newProjectID();
    REQUIRE(uuids.newSceneID().value == "scene-001"); // scene counter unaffected
}

TEST_CASE("MockSecureStore round-trips secret bytes", "[mocks]") {
    MockSecureStore store;
    SecretBytes secret = {std::byte{0x01}, std::byte{0xFF}, std::byte{0xAB}};
    REQUIRE(store.putSecret("key", secret).ok());
    REQUIRE(store.containsSecret("key").value() == true);
    auto got = store.getSecret("key");
    REQUIRE(got.ok());
    REQUIRE(got.value().size() == secret.size());
    for (std::size_t i = 0; i < secret.size(); ++i)
        REQUIRE(std::to_integer<int>(got.value()[i]) == std::to_integer<int>(secret[i]));
}

TEST_CASE("MockSecureStore missing key returns error", "[mocks]") {
    MockSecureStore store;
    REQUIRE(store.containsSecret("missing").value() == false);
    auto got = store.getSecret("missing");
    REQUIRE_FALSE(got.ok());
    REQUIRE(got.error().code == ErrorCode::secureStoreError);
}

TEST_CASE("MockGitProvider records calls on success", "[mocks]") {
    MockGitProvider git;
    REQUIRE(git.initRepository("/tmp/project").ok());
    REQUIRE(git.addAll("/tmp/project").ok());
    auto commitResult = git.commit("/tmp/project", {"Initial commit", {"Author", "a@b.com"}});
    REQUIRE(commitResult.ok());
    REQUIRE(commitResult.value().value == "mock-commit-1");
    REQUIRE(git.initCalls.size()      == 1);
    REQUIRE(git.addAllCalls.size()    == 1);
    REQUIRE(git.commitRequests.size() == 1);
}

TEST_CASE("MockGitProvider returns failure when shouldFail is set", "[mocks]") {
    MockGitProvider git;
    git.shouldFail = true;
    REQUIRE_FALSE(git.initRepository("/tmp/project").ok());
    REQUIRE_FALSE(git.addAll("/tmp/project").ok());
    REQUIRE_FALSE(git.commit("/tmp/project", {"msg", {"A", "a@b.com"}}).ok());
    REQUIRE(git.initCalls.empty()); // calls not recorded on failure
}
