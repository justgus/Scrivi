// ChapterIndexTests.cpp
// Unit tests for the filesystem-authoritative chapter identity/order helpers (EP-027 P2,
// B3): order comes from the folder order-key sort, identity from the sidecar.

#include <catch2/catch_test_macros.hpp>

#include "manuscript/ChapterIndex.hpp"
#include "platform/LocalFileSystem.hpp"
#include "schemas/ChapterMetaJson.hpp"

#include <chrono>
#include <filesystem>
#include <fstream>
#include <string>

namespace fs = std::filesystem;

namespace {

struct TempDir {
    fs::path path;
    TempDir() {
        path = fs::temp_directory_path() / ("scrivi-chidx-" + std::to_string(
            std::chrono::steady_clock::now().time_since_epoch().count()));
        fs::create_directories(path);
    }
    ~TempDir() { std::error_code ec; fs::remove_all(path, ec); }
    std::string str() const { return path.string(); }
};

// Create manuscript/chapter-<key>/chapter.meta.json with the given chapterID.
void makeChapter(const fs::path& root, const std::string& key, const std::string& id) {
    const fs::path dir = root / "manuscript" / ("chapter-" + key);
    fs::create_directories(dir);
    scrivi::schemas::ChapterMetaData ch;
    ch.chapterID = scrivi::ChapterID{ id };
    ch.title     = "Chapter " + id;
    ch.slug      = "chapter-" + key;
    std::ofstream(dir / "chapter.meta.json", std::ios::binary)
        << scrivi::schemas::serializeChapterMeta(ch);
}

} // namespace

TEST_CASE("listChaptersByOrder returns chapters in order-key sort order (not id order)",
          "[ChapterIndex][EP-027]") {
    TempDir tmp;
    // Deliberately: order-keys a0 < a0m < a1, but ids are given out of that order to
    // prove ordering is by KEY, not id.
    makeChapter(tmp.path, "a1",  "chapter_third");
    makeChapter(tmp.path, "a0",  "chapter_first");
    makeChapter(tmp.path, "a0m", "chapter_second");

    scrivi::platform::LocalFileSystem lfs;
    auto r = scrivi::manuscript::listChaptersByOrder(lfs, tmp.str());
    REQUIRE(r.ok());
    const auto& v = r.value();
    REQUIRE(v.size() == 3);
    REQUIRE(v[0].orderKey == "a0");
    REQUIRE(v[1].orderKey == "a0m");
    REQUIRE(v[2].orderKey == "a1");
    REQUIRE(v[0].chapterID.value == "chapter_first");
    REQUIRE(v[1].chapterID.value == "chapter_second");
    REQUIRE(v[2].chapterID.value == "chapter_third");
    REQUIRE(v[0].chapterMetadataRelPath == "manuscript/chapter-a0/chapter.meta.json");
}

TEST_CASE("listChaptersByOrder ignores non-chapter entries", "[ChapterIndex][EP-027]") {
    TempDir tmp;
    makeChapter(tmp.path, "a0", "chapter_only");
    // A stray manuscript.meta.json and a non-chapter folder must be skipped.
    std::ofstream(tmp.path / "manuscript" / "manuscript.meta.json", std::ios::binary) << "{}";
    fs::create_directories(tmp.path / "manuscript" / "notes");

    scrivi::platform::LocalFileSystem lfs;
    auto r = scrivi::manuscript::listChaptersByOrder(lfs, tmp.str());
    REQUIRE(r.ok());
    REQUIRE(r.value().size() == 1);
    REQUIRE(r.value()[0].chapterID.value == "chapter_only");
}

TEST_CASE("findChapterByID locates a chapter by its sidecar id", "[ChapterIndex][EP-027]") {
    TempDir tmp;
    makeChapter(tmp.path, "a0", "chapter_alpha");
    makeChapter(tmp.path, "a1", "chapter_beta");

    scrivi::platform::LocalFileSystem lfs;
    auto r = scrivi::manuscript::findChapterByID(lfs, tmp.str(),
                                                 scrivi::ChapterID{"chapter_beta"});
    REQUIRE(r.ok());
    REQUIRE(r.value().orderKey == "a1");
    REQUIRE(r.value().chapterMetadataRelPath == "manuscript/chapter-a1/chapter.meta.json");
}

TEST_CASE("findChapterByID fails for an unknown id", "[ChapterIndex][EP-027]") {
    TempDir tmp;
    makeChapter(tmp.path, "a0", "chapter_alpha");

    scrivi::platform::LocalFileSystem lfs;
    auto r = scrivi::manuscript::findChapterByID(lfs, tmp.str(),
                                                 scrivi::ChapterID{"chapter_ghost"});
    REQUIRE_FALSE(r.ok());
    REQUIRE(r.error().code == scrivi::ErrorCode::invalidArgument);
}

TEST_CASE("resolveChapterID reads the id from a sidecar path", "[ChapterIndex][EP-027]") {
    TempDir tmp;
    makeChapter(tmp.path, "a0m", "chapter_mid");

    scrivi::platform::LocalFileSystem lfs;
    auto r = scrivi::manuscript::resolveChapterID(
        lfs, tmp.str(), "manuscript/chapter-a0m/chapter.meta.json");
    REQUIRE(r.ok());
    REQUIRE(r.value().value == "chapter_mid");
}
