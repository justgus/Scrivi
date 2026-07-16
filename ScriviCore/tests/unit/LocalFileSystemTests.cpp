// LocalFileSystemTests.cpp
// Unit tests for the FileSystem port's renamePath primitive (EP-027 P1, AC5).
// Focus: the crash-safety-critical contract — atomic move, never-clobber, and the
// caller-error guards (missing source, existing destination). Directories and files.

#include <catch2/catch_test_macros.hpp>

#include "platform/LocalFileSystem.hpp"

#include <chrono>
#include <filesystem>
#include <fstream>
#include <string>

namespace fs = std::filesystem;

namespace {

struct TempDir {
    fs::path path;
    TempDir() {
        path = fs::temp_directory_path() / ("scrivi-lfs-" + std::to_string(
            std::chrono::steady_clock::now().time_since_epoch().count()));
        fs::create_directories(path);
    }
    ~TempDir() {
        std::error_code ec;
        fs::remove_all(path, ec);
    }
    std::string str() const { return path.string(); }
};

void writeFile(const fs::path& p, const std::string& contents) {
    std::ofstream out(p, std::ios::binary);
    out << contents;
}

std::string readFile(const fs::path& p) {
    std::ifstream in(p, std::ios::binary);
    std::string s((std::istreambuf_iterator<char>(in)), std::istreambuf_iterator<char>());
    return s;
}

} // namespace

TEST_CASE("renamePath moves a file and preserves its contents", "[FileSystem][renamePath][EP-027]") {
    TempDir tmp;
    scrivi::platform::LocalFileSystem fsys;

    const fs::path from = tmp.path / "a.txt";
    const fs::path to   = tmp.path / "b.txt";
    writeFile(from, "hello");

    auto r = fsys.renamePath(from.string(), to.string());
    REQUIRE(r.ok());
    REQUIRE_FALSE(fs::exists(from));
    REQUIRE(fs::exists(to));
    REQUIRE(readFile(to) == "hello");
}

TEST_CASE("renamePath moves a directory (with contents) to a new name",
          "[FileSystem][renamePath][EP-027]") {
    TempDir tmp;
    scrivi::platform::LocalFileSystem fsys;

    // Model a chapter folder: chapter-001/ with a sidecar + a scene file.
    const fs::path from = tmp.path / "chapter-001";
    const fs::path to   = tmp.path / "chapter-a0m";
    fs::create_directories(from);
    writeFile(from / "chapter.meta.json", "{\"chapterID\":\"chapter_x\"}");
    writeFile(from / "001-scene.md", "scene body");

    auto r = fsys.renamePath(from.string(), to.string());
    REQUIRE(r.ok());
    REQUIRE_FALSE(fs::exists(from));
    REQUIRE(fs::exists(to / "chapter.meta.json"));
    REQUIRE(readFile(to / "chapter.meta.json") == "{\"chapterID\":\"chapter_x\"}");
    REQUIRE(readFile(to / "001-scene.md") == "scene body");
}

TEST_CASE("renamePath refuses to clobber an existing destination (file)",
          "[FileSystem][renamePath][EP-027]") {
    TempDir tmp;
    scrivi::platform::LocalFileSystem fsys;

    const fs::path from = tmp.path / "src.txt";
    const fs::path to   = tmp.path / "dst.txt";
    writeFile(from, "SOURCE");
    writeFile(to,   "PRECIOUS");   // must NOT be destroyed

    auto r = fsys.renamePath(from.string(), to.string());
    REQUIRE_FALSE(r.ok());
    REQUIRE(r.error().code == scrivi::ErrorCode::invalidArgument);
    // Both survive untouched — the no-clobber invariant (I-0072 class).
    REQUIRE(readFile(from) == "SOURCE");
    REQUIRE(readFile(to)   == "PRECIOUS");
}

TEST_CASE("renamePath refuses to clobber an existing destination (directory)",
          "[FileSystem][renamePath][EP-027]") {
    TempDir tmp;
    scrivi::platform::LocalFileSystem fsys;

    const fs::path from = tmp.path / "chapter-001";
    const fs::path to   = tmp.path / "chapter-002";
    fs::create_directories(from);
    writeFile(from / "meta.json", "NEW");
    fs::create_directories(to);
    writeFile(to / "meta.json", "EXISTING");   // an occupied slug — must not be overwritten

    auto r = fsys.renamePath(from.string(), to.string());
    REQUIRE_FALSE(r.ok());
    REQUIRE(r.error().code == scrivi::ErrorCode::invalidArgument);
    REQUIRE(readFile(to / "meta.json") == "EXISTING");
    REQUIRE(fs::exists(from / "meta.json"));
}

TEST_CASE("renamePath fails when the source does not exist",
          "[FileSystem][renamePath][EP-027]") {
    TempDir tmp;
    scrivi::platform::LocalFileSystem fsys;

    const fs::path from = tmp.path / "nope";
    const fs::path to   = tmp.path / "somewhere";

    auto r = fsys.renamePath(from.string(), to.string());
    REQUIRE_FALSE(r.ok());
    REQUIRE(r.error().code == scrivi::ErrorCode::invalidArgument);
    REQUIRE_FALSE(fs::exists(to));
}
