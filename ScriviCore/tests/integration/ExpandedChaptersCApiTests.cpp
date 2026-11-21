// SP-226 (T-0657) — the Navigator's expanded chapters, tested THROUGH `scrivi_*` (a facade test cannot see the boundary):
// the set round-trips through scrivi_open_project, and neither open_scene nor save_scene (both rewrite the same file) drops it.

#include <catch2/catch_test_macros.hpp>

#include "scrivi/scrivi.h"
#include "util/Json.hpp"

#include <atomic>
#include <cstdint>
#include <filesystem>
#include <string>
#include <vector>
#include <unistd.h>

namespace fs = std::filesystem;
using scrivi::util::JsonDoc;
using scrivi::util::parseJson;

namespace {

JsonDoc envelope(const char* raw) {
    REQUIRE(raw != nullptr);
    auto parsed = parseJson(raw);
    scrivi_free(raw);
    REQUIRE(parsed.ok());
    return std::move(parsed.value());
}

struct Fixture {
    fs::path projectDir, appSupportDir;
    std::string projectID, sceneID, chapterID, metadataPath, contentPath;

    Fixture() {
        static std::atomic<int> counter{0};
        const std::string stem = "scrivi-expanded-" + std::to_string(::getpid()) + "-" + std::to_string(counter.fetch_add(1));
        projectDir    = fs::temp_directory_path() / (stem + ".inkwyn");
        appSupportDir = fs::temp_directory_path() / (stem + "-appsupport");
        fs::create_directories(projectDir);
        fs::create_directories(appSupportDir);
        REQUIRE(envelope(scrivi_create_project(root(), appSupport(), "Expanded", "expanded",
                                               "identity-001", "persona-001", "Test Author")).getBool("ok"));
        JsonDoc opened = open();
        projectID = opened.getString("projectID");
        REQUIRE(opened.arraySize("scenes") > 0);
        JsonDoc first = opened.arrayItem("scenes", 0);
        sceneID      = first.getString("sceneID");
        chapterID    = first.getString("chapterID");
        metadataPath = first.getString("metadataPath");
        contentPath  = first.getString("contentPath");
    }
    ~Fixture() {
        std::error_code ec;
        fs::remove_all(projectDir, ec);
        fs::remove_all(appSupportDir, ec);
    }
    [[nodiscard]] const char* root() const { return projectDir.c_str(); }
    [[nodiscard]] const char* appSupport() const { return appSupportDir.c_str(); }
    JsonDoc open() {
        JsonDoc env = envelope(scrivi_open_project(root(), appSupport(), "identity-001"));
        REQUIRE(env.getBool("ok"));
        return env.getSubDoc("result");
    }
    JsonDoc set(const char* json) {
        return envelope(scrivi_set_expanded_chapters(root(), appSupport(), projectID.c_str(), json));
    }
};

}  // namespace

TEST_CASE("[SP-226] the expanded set round-trips through open, survives open_scene and save_scene", "[workspace][capi]") {
    Fixture f;
    REQUIRE(f.open().getStringArray("expandedChapterIDs").empty());   // Q1: a project with no stored choice

    const std::string json = R"([")" + f.chapterID + R"(","chapter_other"])";
    JsonDoc set = f.set(json.c_str());
    REQUIRE(set.getBool("ok"));
    const std::vector<std::string> expected{f.chapterID, "chapter_other"};
    REQUIRE(f.open().getStringArray("expandedChapterIDs") == expected);

    REQUIRE(envelope(scrivi_open_scene(f.root(), f.appSupport(), f.projectID.c_str(), f.sceneID.c_str())).getBool("ok"));
    REQUIRE(f.open().getStringArray("expandedChapterIDs") == expected);

    REQUIRE(envelope(scrivi_save_scene(f.projectID.c_str(), f.root(), f.appSupport(), f.sceneID.c_str(),
                                       f.metadataPath.c_str(), f.contentPath.c_str(), "Changed text.",
                                       0, 0, 0.0, "identity-001", "persona-001", "Test Author")).getBool("ok"));
    REQUIRE(f.open().getStringArray("expandedChapterIDs") == expected);

    REQUIRE(f.set("[]").getBool("ok"));
    REQUIRE(f.open().getStringArray("expandedChapterIDs").empty());
}

TEST_CASE("[SP-226] set_expanded_chapters refuses what is not an array", "[workspace][capi]") {
    Fixture f;
    REQUIRE_FALSE(f.set("not json").getBool("ok"));
    REQUIRE_FALSE(f.set(R"({"a":1})").getBool("ok"));
    REQUIRE_FALSE(envelope(scrivi_set_expanded_chapters(f.root(), f.appSupport(), "", "[]")).getBool("ok"));
}
