#include <catch2/catch_test_macros.hpp>
#include "util/Json.hpp"

using namespace scrivi::util;

TEST_CASE("parse valid JSON succeeds", "[Json]") {
    auto result = parseJson(R"({"title":"My Story","draft":true})");
    REQUIRE(result.ok());
    REQUIRE(result.value().getString("title") == "My Story");
    REQUIRE(result.value().getBool("draft") == true);
}

TEST_CASE("parse malformed JSON returns error", "[Json]") {
    auto result = parseJson("{not valid json}");
    REQUIRE_FALSE(result.ok());
    REQUIRE(result.error().code == scrivi::ErrorCode::parseError);
}

TEST_CASE("missing key returns default value", "[Json]") {
    auto result = parseJson(R"({"a":"b"})");
    REQUIRE(result.ok());
    REQUIRE(result.value().getString("missing", "default") == "default");
    REQUIRE(result.value().getBool("missing", false) == false);
}

TEST_CASE("set and dump round-trips string field", "[Json]") {
    auto result = parseJson("{}");
    REQUIRE(result.ok());
    auto doc = std::move(result.value());
    doc.setString("slug", "my-story");
    auto json = parseJson(doc.dump());
    REQUIRE(json.ok());
    REQUIRE(json.value().getString("slug") == "my-story");
}

TEST_CASE("contains returns correct presence", "[Json]") {
    auto result = parseJson(R"({"x":1})");
    REQUIRE(result.ok());
    REQUIRE(result.value().contains("x") == true);
    REQUIRE(result.value().contains("y") == false);
}
