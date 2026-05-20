#include <catch2/catch_test_macros.hpp>
#include "util/PathUtils.hpp"

using namespace scrivi::util;

TEST_CASE("join appends relative path to base", "[PathUtils]") {
    auto result = join("/projects/my-story", "scenes/chapter-01.md");
    REQUIRE(result == "/projects/my-story/scenes/chapter-01.md");
}

TEST_CASE("extension returns dot-prefixed extension", "[PathUtils]") {
    REQUIRE(extension("/foo/bar.json") == ".json");
    REQUIRE(extension("/foo/bar.md")   == ".md");
    REQUIRE(extension("/foo/bar")      == "");
}

TEST_CASE("filename returns final path component", "[PathUtils]") {
    REQUIRE(filename("/foo/bar/baz.txt") == "baz.txt");
}

TEST_CASE("parent returns containing directory", "[PathUtils]") {
    REQUIRE(parent("/foo/bar/baz.txt") == "/foo/bar");
}

TEST_CASE("makeAbsolute resolves relative against base", "[PathUtils]") {
    auto result = makeAbsolute("scenes/ch01.md", "/projects/my-story");
    REQUIRE(result.ok());
    REQUIRE(result.value() == "/projects/my-story/scenes/ch01.md");
}

TEST_CASE("replaceExtension swaps extension", "[PathUtils]") {
    REQUIRE(replaceExtension("/foo/bar.md", ".json") == "/foo/bar.json");
}
