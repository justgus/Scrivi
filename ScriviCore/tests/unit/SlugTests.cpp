#include <catch2/catch_test_macros.hpp>
#include "util/Slug.hpp"

using namespace scrivi::util;

TEST_CASE("basic title becomes lowercase hyphenated slug", "[Slug]") {
    REQUIRE(makeSlug("The Dark Forest") == "the-dark-forest");
}

TEST_CASE("punctuation and special chars are stripped", "[Slug]") {
    REQUIRE(makeSlug("Hello, World!") == "hello-world");
    REQUIRE(makeSlug("It's a trap...") == "it-s-a-trap");
}

TEST_CASE("runs of separators collapse to single hyphen", "[Slug]") {
    REQUIRE(makeSlug("a   b---c") == "a-b-c");
}

TEST_CASE("leading and trailing hyphens are stripped", "[Slug]") {
    REQUIRE(makeSlug("  hello  ") == "hello");
    REQUIRE(makeSlug("---world---") == "world");
}

TEST_CASE("empty and whitespace-only input yields empty slug", "[Slug]") {
    REQUIRE(makeSlug("") == "");
    REQUIRE(makeSlug("   ") == "");
}

TEST_CASE("numbers are preserved in slugs", "[Slug]") {
    REQUIRE(makeSlug("Chapter 1") == "chapter-1");
    REQUIRE(makeSlug("001 Opening Scene") == "001-opening-scene");
}
