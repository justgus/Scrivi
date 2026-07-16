// ChapterMigrationTests.cpp
// EP-027 P3: migrating an OLD-FORMAT project (legacy `chapter-NNN` folders whose numeric
// sort does NOT match the intended reading order in the index array) to order-key slugs
// so the folder-key sort reproduces the reading order (B3).

#include <catch2/catch_test_macros.hpp>

#include "manuscript/ChapterIndex.hpp"
#include "platform/LocalFileSystem.hpp"
#include "schemas/ChapterMetaJson.hpp"
#include "schemas/ManuscriptMetaJson.hpp"
#include "schemas/SceneMetaJson.hpp"
#include "util/OrderKey.hpp"

#include <chrono>
#include <filesystem>
#include <fstream>
#include <string>
#include <vector>

namespace fs = std::filesystem;

namespace {

struct TempDir {
    fs::path path;
    TempDir() {
        path = fs::temp_directory_path() / ("scrivi-mig-" + std::to_string(
            std::chrono::steady_clock::now().time_since_epoch().count()));
        fs::create_directories(path);
    }
    ~TempDir() { std::error_code ec; fs::remove_all(path, ec); }
    std::string str() const { return path.string(); }
};

void write(const fs::path& p, const std::string& s) {
    fs::create_directories(p.parent_path());
    std::ofstream(p, std::ios::binary) << s;
}

// Build a legacy chapter folder `chapter-NNN` with one scene, and return its rel meta path.
std::string makeLegacyChapter(const fs::path& root, const std::string& nnn,
                              const std::string& chapterID, const std::string& sceneID,
                              const std::string& body) {
    const std::string dir = "manuscript/chapter-" + nnn;
    const std::string sceneMetaRel = dir + "/001-scene.meta.json";
    const std::string sceneMdRel   = dir + "/001-scene.md";

    scrivi::schemas::SceneMetaData s;
    s.sceneID = scrivi::SceneID{sceneID};
    s.slug = "001-scene";
    s.contentPath = sceneMdRel;
    write(root / sceneMetaRel, scrivi::schemas::serializeSceneMeta(s));
    write(root / sceneMdRel, body);

    scrivi::schemas::ChapterMetaData ch;
    ch.chapterID = scrivi::ChapterID{chapterID};
    ch.title = "Chapter " + nnn;
    ch.slug = "chapter-" + nnn;
    ch.scenes.push_back({scrivi::SceneID{sceneID}, sceneMetaRel});
    write(root / (dir + "/chapter.meta.json"), scrivi::schemas::serializeChapterMeta(ch));
    return dir + "/chapter.meta.json";
}

} // namespace

TEST_CASE("migrateChapterOrderKeys reorders legacy folders to match index-array order",
          "[Chapter][migration][EP-027]") {
    TempDir tmp;

    // Three legacy chapters on disk: chapter-001/002/003. But the index array puts them in
    // a DIFFERENT reading order — [ch2, ch1, ch3] — as a legacy reorder would have (it
    // shuffled the array, not the folders). Numeric folder sort (001<002<003) => ch1,ch2,ch3
    // which is WRONG; migration must make folder-key sort reproduce ch2,ch1,ch3.
    auto p1 = makeLegacyChapter(tmp.path, "001", "ch1", "s1", "Body one");
    auto p2 = makeLegacyChapter(tmp.path, "002", "ch2", "s2", "Body two");
    auto p3 = makeLegacyChapter(tmp.path, "003", "ch3", "s3", "Body three");

    scrivi::schemas::ManuscriptMetaData ms;
    ms.manuscriptID = scrivi::ManuscriptID{"m1"};
    ms.title = "Legacy Novel";
    ms.chapters.push_back({scrivi::ChapterID{"ch2"}, p2});   // intended order: ch2 first
    ms.chapters.push_back({scrivi::ChapterID{"ch1"}, p1});
    ms.chapters.push_back({scrivi::ChapterID{"ch3"}, p3});
    write(tmp.path / "manuscript/manuscript.meta.json",
          scrivi::schemas::serializeManuscriptMeta(ms));

    scrivi::platform::LocalFileSystem lfs;

    // Pre-migration: folder-key sort is 001<002<003 => ch1, ch2, ch3 (WRONG order).
    {
        auto pre = scrivi::manuscript::listChaptersByOrder(lfs, tmp.str());
        REQUIRE(pre.ok());
        std::vector<std::string> ids;
        for (auto& e : pre.value()) ids.push_back(e.chapterID.value);
        REQUIRE(ids == std::vector<std::string>{"ch1", "ch2", "ch3"});
    }

    // Migrate.
    auto mig = scrivi::manuscript::migrateChapterOrderKeys(lfs, tmp.str());
    REQUIRE(mig.ok());
    REQUIRE(mig.value() == true);   // it renamed folders

    // Post-migration: folder-key sort now reproduces the index-array order ch2, ch1, ch3,
    // and every scene body is still resolvable (paths were rewritten).
    auto post = scrivi::manuscript::listChaptersByOrder(lfs, tmp.str());
    REQUIRE(post.ok());
    std::vector<std::string> ids;
    for (auto& e : post.value()) ids.push_back(e.chapterID.value);
    REQUIRE(ids == std::vector<std::string>{"ch2", "ch1", "ch3"});

    // Scene bodies intact via the (rewritten) content paths.
    for (auto& e : post.value()) {
        auto chText = lfs.readTextFile(tmp.str() + "/" + e.chapterMetadataRelPath);
        REQUIRE(chText.ok());
        auto ch = scrivi::schemas::parseChapterMeta(chText.value());
        REQUIRE(ch.ok());
        REQUIRE(ch.value().scenes.size() == 1);
        auto sMeta = lfs.readTextFile(tmp.str() + "/" + ch.value().scenes[0].metadataPath);
        REQUIRE(sMeta.ok());
        auto s = scrivi::schemas::parseSceneMeta(sMeta.value());
        REQUIRE(s.ok());
        auto body = lfs.readTextFile(tmp.str() + "/" + s.value().contentPath);
        REQUIRE(body.ok());
        REQUIRE_FALSE(body.value().empty());
    }

    // Idempotent: a second migration is a no-op.
    auto again = scrivi::manuscript::migrateChapterOrderKeys(lfs, tmp.str());
    REQUIRE(again.ok());
    REQUIRE(again.value() == false);
}

TEST_CASE("migrateChapterOrderKeys is a no-op when folders already read in order",
          "[Chapter][migration][EP-027]") {
    TempDir tmp;
    // Legacy folders whose numeric order ALREADY matches the index array => no migration.
    auto p1 = makeLegacyChapter(tmp.path, "001", "ch1", "s1", "one");
    auto p2 = makeLegacyChapter(tmp.path, "002", "ch2", "s2", "two");

    scrivi::schemas::ManuscriptMetaData ms;
    ms.manuscriptID = scrivi::ManuscriptID{"m1"};
    ms.chapters.push_back({scrivi::ChapterID{"ch1"}, p1});
    ms.chapters.push_back({scrivi::ChapterID{"ch2"}, p2});
    write(tmp.path / "manuscript/manuscript.meta.json",
          scrivi::schemas::serializeManuscriptMeta(ms));

    scrivi::platform::LocalFileSystem lfs;
    auto mig = scrivi::manuscript::migrateChapterOrderKeys(lfs, tmp.str());
    REQUIRE(mig.ok());
    REQUIRE(mig.value() == false);
}
