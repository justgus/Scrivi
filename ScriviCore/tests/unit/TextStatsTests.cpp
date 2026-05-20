#include <catch2/catch_test_macros.hpp>
#include "util/TextStats.hpp"

using namespace scrivi::util;

TEST_CASE("word count for simple prose", "[TextStats]") {
    auto s = countText("The quick brown fox");
    REQUIRE(s.wordCount == 4);
}

TEST_CASE("word count handles extra whitespace", "[TextStats]") {
    auto s = countText("  hello   world  ");
    REQUIRE(s.wordCount == 2);
}

TEST_CASE("empty string has zero counts", "[TextStats]") {
    auto s = countText("");
    REQUIRE(s.wordCount == 0);
    REQUIRE(s.characterCount == 0);
}

TEST_CASE("character count matches code point count for ASCII", "[TextStats]") {
    auto s = countText("hello");
    REQUIRE(s.characterCount == 5);
}

TEST_CASE("markdown-style text word count", "[TextStats]") {
    auto s = countText("# Chapter One\n\nOnce upon a time.");
    REQUIRE(s.wordCount == 7); // #, Chapter, One, Once, upon, a, time.
}

TEST_CASE("single word has word count of one", "[TextStats]") {
    auto s = countText("hello");
    REQUIRE(s.wordCount == 1);
}
