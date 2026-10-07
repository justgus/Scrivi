// EP-047 SP-167 (T-0596) — `project-settings.json` get/put and the project title, AT THE C ABI
// (`feedback_boundary_tests_not_facade`).
//
// ✅ The settings file shares the inspector layout's OPAQUE contract by construction (one helper pair
// in `scrivi_c_api.cpp`); these tests pin that it behaves the same for the second file, and that the
// two files never collide. ✅ The title write must keep every other `project.json` field — a project
// that no longer opens would be the cost of getting it wrong.

#include <catch2/catch_test_macros.hpp>

#include "scrivi/scrivi.h"
#include "util/Json.hpp"

#include <atomic>
#include <cstdint>
#include <filesystem>
#include <fstream>
#include <sstream>
#include <string>

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

JsonDoc okResult(const char* raw) {
    JsonDoc env = envelope(raw);
    INFO("envelope: " << env.dump());
    REQUIRE(env.getBool("ok"));
    return env.getSubDoc("result");
}

bool failed(const char* raw) { return !envelope(raw).getBool("ok"); }

struct SettingsFixture {
    fs::path projectDir;
    fs::path appSupportDir;

    SettingsFixture() {
        static std::atomic<int> counter{0};
        const std::string stem = "scrivi-project-settings-" + std::to_string(counter.fetch_add(1)) + "-" +
                                 std::to_string(reinterpret_cast<std::uintptr_t>(this));
        projectDir = fs::temp_directory_path() / (stem + ".scrivi");
        appSupportDir = fs::temp_directory_path() / (stem + "-appsupport");
        fs::create_directories(projectDir);
        fs::create_directories(appSupportDir);
        okResult(scrivi_create_project(root(), appSupportDir.c_str(), "Original Title", "original-title",
                                       "identity-001", "persona-001", "Test Author"));
    }
    ~SettingsFixture() {
        std::error_code ec;
        fs::remove_all(projectDir, ec);
        fs::remove_all(appSupportDir, ec);
    }
    [[nodiscard]] const char* root() const { return projectDir.c_str(); }
    [[nodiscard]] std::string read(const char* name) const {
        std::ifstream in(projectDir / name, std::ios::binary);
        std::stringstream ss;
        ss << in.rdbuf();
        return ss.str();
    }
    void write(const char* name, const std::string& text) const {
        std::ofstream out(projectDir / name, std::ios::binary | std::ios::trunc);
        out << text;
    }
};

} // namespace

TEST_CASE("project settings: absent is normal, not an error", "[project-settings][capi]") {
    SettingsFixture f;
    const auto r = okResult(scrivi_get_project_settings(f.root()));
    CHECK(r.getString("status") == "absent");
    CHECK_FALSE(r.contains("document"));
}

TEST_CASE("project settings: put then get round-trips every key, opaque", "[project-settings][capi]") {
    SettingsFixture f;
    const std::string doc =
        R"({"subtitle":"A Novel","showChapterTitles":true,"linuxOnlyKey":{"nested":[1,2,3]}})";
    CHECK(okResult(scrivi_put_project_settings(f.root(), doc.c_str())).getBool("saved"));
    const auto r = okResult(scrivi_get_project_settings(f.root()));
    REQUIRE(r.getString("status") == "ok");
    const auto d = r.getSubDoc("document");
    CHECK(d.getString("subtitle") == "A Novel");
    CHECK(d.getBool("showChapterTitles"));
    CHECK(d.getSubDoc("linuxOnlyKey").arraySize("nested") == 3);   // another platform's key survives verbatim
}

TEST_CASE("project settings: a non-object or invalid JSON is refused, not written", "[project-settings][capi]") {
    SettingsFixture f;
    CHECK(failed(scrivi_put_project_settings(f.root(), "[1,2]")));
    CHECK(failed(scrivi_put_project_settings(f.root(), "{not json")));
    CHECK(failed(scrivi_put_project_settings(f.root(), "")));
    CHECK(okResult(scrivi_get_project_settings(f.root())).getString("status") == "absent");
}

TEST_CASE("project settings: an unreadable file is reported and left alone", "[project-settings][capi]") {
    SettingsFixture f;
    f.write("project-settings.json", "{ damaged");
    const auto r = okResult(scrivi_get_project_settings(f.root()));
    CHECK(r.getString("status") == "unreadable");
    CHECK(f.read("project-settings.json") == "{ damaged");   // never repaired on read
    // The writer's explicit PUT may replace it.
    CHECK(okResult(scrivi_put_project_settings(f.root(), "{}")).getBool("saved"));
    CHECK(okResult(scrivi_get_project_settings(f.root())).getString("status") == "ok");
}

TEST_CASE("project settings and inspector layout are separate files", "[project-settings][capi]") {
    SettingsFixture f;
    okResult(scrivi_put_project_settings(f.root(), R"({"subtitle":"S"})"));
    okResult(scrivi_put_inspector_layout(f.root(), R"({"selectedTab":"writing"})"));
    CHECK(okResult(scrivi_get_project_settings(f.root())).getSubDoc("document").getString("subtitle") == "S");
    CHECK(okResult(scrivi_get_inspector_layout(f.root())).getSubDoc("document").getString("selectedTab") == "writing");
    CHECK_FALSE(okResult(scrivi_get_project_settings(f.root())).getSubDoc("document").contains("selectedTab"));
}

TEST_CASE("set project title: writes project.json's title, keeps every other field, still opens", "[project-settings][capi]") {
    SettingsFixture f;
    // A field this build does not know must survive the rename (no struct round trip).
    auto before = parseJson(f.read("project.json"));
    REQUIRE(before.ok());
    auto withExtra = std::move(before.value());
    withExtra.setString("futureField", "kept");
    f.write("project.json", withExtra.dump(2));

    CHECK(okResult(scrivi_set_project_title(f.root(), "Renamed")).getString("title") == "Renamed");
    auto after = parseJson(f.read("project.json"));
    REQUIRE(after.ok());
    CHECK(after.value().getString("title") == "Renamed");
    CHECK(after.value().getString("futureField") == "kept");
    CHECK(after.value().getString("projectID") == withExtra.getString("projectID"));
    CHECK(after.value().getString("slug") == withExtra.getString("slug"));
    // The project still opens, and reports the new title (I-0093's open envelope).
    const auto opened = okResult(scrivi_open_project(f.root(), f.appSupportDir.c_str(), "identity-001"));
    CHECK(opened.getString("projectTitle") == "Renamed");
}

TEST_CASE("set project title: an empty title is refused and nothing changes", "[project-settings][capi]") {
    SettingsFixture f;
    const std::string before = f.read("project.json");
    CHECK(failed(scrivi_set_project_title(f.root(), "")));
    CHECK(failed(scrivi_set_project_title(f.root(), "   ")));
    CHECK(failed(scrivi_set_project_title(nullptr, "x")));
    CHECK(f.read("project.json") == before);
}
