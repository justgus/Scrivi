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

// ---------------------------------------------------------------------------
// I-0221 — AppleDouble and shell artifacts must never be read as project content
// ---------------------------------------------------------------------------

TEST_CASE("isIgnorableFilesystemArtifact rejects AppleDouble sidecars", "[PathUtils][I-0221]") {
    // ⚠️ THE EXACT NAME THAT ABORTED A 1,224-SCENE PROJECT OPEN. It ends in
    // `.meta.json` and yields a non-empty order key, so every suffix test in the
    // manuscript scan accepted it before this predicate existed.
    REQUIRE(isIgnorableFilesystemArtifact("._001-scene.meta.json"));
    REQUIRE(isIgnorableFilesystemArtifact("._001-scene.md"));
    REQUIRE(isIgnorableFilesystemArtifact("._binding.json"));
    REQUIRE(isIgnorableFilesystemArtifact("._index.json"));
    REQUIRE(isIgnorableFilesystemArtifact("._chapter.meta.json"));
}

TEST_CASE("isIgnorableFilesystemArtifact accepts real scene files", "[PathUtils][I-0221]") {
    // ⚠️ THE OTHER HALF, and the one that matters more: the predicate must not
    // swallow content. A false positive here silently deletes scenes from a
    // manuscript -- a far worse failure than the one being fixed.
    REQUIRE_FALSE(isIgnorableFilesystemArtifact("001-scene.meta.json"));
    REQUIRE_FALSE(isIgnorableFilesystemArtifact("001-scene.md"));
    REQUIRE_FALSE(isIgnorableFilesystemArtifact("chapter.meta.json"));
    REQUIRE_FALSE(isIgnorableFilesystemArtifact("binding.json"));
    REQUIRE_FALSE(isIgnorableFilesystemArtifact("project.json"));
    REQUIRE_FALSE(isIgnorableFilesystemArtifact("world.json"));

    // A leading dot alone is NOT the signal -- only the `._` pair is. A scene
    // legitimately named with a leading dot stays visible.
    REQUIRE_FALSE(isIgnorableFilesystemArtifact(".hidden-but-real.md"));

    // `_` alone is not either: `_notes.md` is an ordinary filename.
    REQUIRE_FALSE(isIgnorableFilesystemArtifact("_notes.md"));
}

TEST_CASE("isIgnorableFilesystemArtifact rejects shell metadata", "[PathUtils][I-0221]") {
    REQUIRE(isIgnorableFilesystemArtifact(".DS_Store"));
    REQUIRE(isIgnorableFilesystemArtifact("Thumbs.db"));
    REQUIRE(isIgnorableFilesystemArtifact("desktop.ini"));

    // Case-insensitive for the Windows names: FAT and SMB volumes are routinely
    // case-insensitive, so `THUMBS.DB` is the same file as `Thumbs.db`.
    REQUIRE(isIgnorableFilesystemArtifact("THUMBS.DB"));
    REQUIRE(isIgnorableFilesystemArtifact("Desktop.ini"));

    // ⚠️ But a real file that merely CONTAINS one of those names is content.
    REQUIRE_FALSE(isIgnorableFilesystemArtifact("thumbs.db.md"));
    REQUIRE_FALSE(isIgnorableFilesystemArtifact("my-desktop.ini.meta.json"));
}

TEST_CASE("isIgnorableFilesystemArtifact rejects volume bookkeeping and dot entries",
          "[PathUtils][I-0221]") {
    REQUIRE(isIgnorableFilesystemArtifact(".Spotlight-V100"));
    REQUIRE(isIgnorableFilesystemArtifact(".fseventsd"));
    REQUIRE(isIgnorableFilesystemArtifact(".Trashes"));
    REQUIRE(isIgnorableFilesystemArtifact("."));
    REQUIRE(isIgnorableFilesystemArtifact(".."));
    REQUIRE(isIgnorableFilesystemArtifact(""));
}
