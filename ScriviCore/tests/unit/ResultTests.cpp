#include <catch2/catch_test_macros.hpp>
#include "scrivi/Result.hpp"

using namespace scrivi;

TEST_CASE("Result<int> success holds value", "[Result]") {
    auto r = Result<int>::success(42);
    REQUIRE(r.ok());
    REQUIRE(r.value() == 42);
}

TEST_CASE("Result<int> failure holds error", "[Result]") {
    auto r = Result<int>::failure({ErrorCode::ioError, "disk full"});
    REQUIRE_FALSE(r.ok());
    REQUIRE(r.error().code == ErrorCode::ioError);
    REQUIRE(r.error().message == "disk full");
}

TEST_CASE("Result<void> success is ok", "[Result]") {
    auto r = Result<void>::success();
    REQUIRE(r.ok());
}

TEST_CASE("Result<void> failure holds error", "[Result]") {
    auto r = Result<void>::failure({ErrorCode::parseError, "bad json"});
    REQUIRE_FALSE(r.ok());
    REQUIRE(r.error().code == ErrorCode::parseError);
}
