// EP-041 SP-141 (T-0507) — `inspector-layout.json` get/put AT THE C ABI.
//
// ⚠️ WHY THIS FILE EXISTS, AND WHY IT IS AT THE ABI. These endpoints exist to end
// the [I-0197] Class B bypass, in which the Swift app and the Qt app EACH owned
// this file. They had already drifted: [I-0215] — Linux preserved keys it did not
// understand, Apple dropped them. A facade test cannot see a boundary gap
// (`feedback_boundary_tests_not_facade`), and I-0113 shipped green exactly that
// way, so every assertion here goes through scrivi_*.
//
// ⚠️ THE LOSSLESS ASSERTIONS ARE THE POINT. The fixture below is a REAL
// Apple-written layout shape — `selectedTab`, `inspectorHidden`, `defaultStacks`,
// `stackSort` and a per-scene `scenes` map. If a future change ever makes the core
// parse this document into a typed struct, these tests fail, which is the whole
// reason they name the keys explicitly instead of comparing blobs.
//
// ⚠️ ABSENCE SEMANTICS (ruled 2026-09-21, "core reports, app decides"): the GET
// ALWAYS succeeds and reports status ok | absent | unreadable. A missing layout is
// NORMAL — every project created before this file existed has none — so it must
// never surface as an error ([I-0222] precedent).

#include <catch2/catch_test_macros.hpp>

#include "scrivi/scrivi.h"
#include "util/Json.hpp"

#include <atomic>
#include <cstdint>
#include <filesystem>
#include <fstream>
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

JsonDoc errorOf(const char* raw) {
    JsonDoc env = envelope(raw);
    INFO("envelope: " << env.dump());
    REQUIRE_FALSE(env.getBool("ok"));
    return env.getSubDoc("error");
}

// A real project created through the C ABI, removed on destruction.
struct LayoutFixture {
    fs::path projectDir;
    fs::path appSupportDir;

    LayoutFixture() {
        static std::atomic<int> counter{0};
        const std::string stem =
            "scrivi-inspector-layout-" + std::to_string(counter.fetch_add(1)) + "-" +
            std::to_string(reinterpret_cast<std::uintptr_t>(this));

        projectDir    = fs::temp_directory_path() / (stem + ".scrivi");
        appSupportDir = fs::temp_directory_path() / (stem + "-appsupport");
        fs::create_directories(projectDir);
        fs::create_directories(appSupportDir);

        okResult(scrivi_create_project(root(), appSupport(), "Inspector Layout",
                                       "inspector-layout",
                                       "identity-001", "persona-001", "Test Author"));
    }

    ~LayoutFixture() {
        std::error_code ec;
        fs::remove_all(projectDir, ec);
        fs::remove_all(appSupportDir, ec);
    }

    [[nodiscard]] const char* root() const { return projectDir.c_str(); }
    [[nodiscard]] const char* appSupport() const { return appSupportDir.c_str(); }

    [[nodiscard]] fs::path layoutPath() const {
        return projectDir / "inspector-layout.json";
    }

    void writeLayoutFile(const std::string& contents) const {
        std::ofstream out(layoutPath(), std::ios::binary | std::ios::trunc);
        out << contents;
    }

    [[nodiscard]] std::string readLayoutFile() const {
        std::ifstream in(layoutPath(), std::ios::binary);
        return std::string(std::istreambuf_iterator<char>(in), {});
    }
};

// ⚠️ A REAL Apple-written layout shape. `stackSort`, `defaultStacks` and the
// per-scene `scenes` map are the keys a typed round trip would silently delete.
constexpr const char* kAppleLayout = R"({
  "schema": "scrivi.inspector-layout.v1",
  "selectedTab": "worldbuilding",
  "inspectorHidden": false,
  "stackSort": "manual",
  "defaultStacks": {
    "writing": [{"type": "outline"}, {"type": "tags"}],
    "worldbuilding": [{"type": "characters"}]
  },
  "scenes": {
    "scene-0001": {
      "writing": [{"type": "todo"}]
    }
  }
})";

} // namespace

TEST_CASE("inspector layout: absent is reported, not an error", "[inspector-layout][capi]") {
    LayoutFixture fx;

    // ⚠️ A brand-new project has NO layout file. This is the common first answer
    // for a real writer's project, so it must read as a normal outcome.
    REQUIRE_FALSE(fs::exists(fx.layoutPath()));

    auto result = okResult(scrivi_get_inspector_layout(fx.root()));
    REQUIRE(result.getString("status") == "absent");

    // ⛔ THE CORE MUST NOT INVENT DEFAULTS. It does not know what a tab is; the app
    // owns that meaning. An `absent` result carries no document at all.
    REQUIRE_FALSE(result.contains("document"));

    // ⛔ AND IT MUST NOT CREATE THE FILE ON READ.
    REQUIRE_FALSE(fs::exists(fx.layoutPath()));
}

TEST_CASE("inspector layout: put then get round-trips every key", "[inspector-layout][capi]") {
    LayoutFixture fx;

    okResult(scrivi_put_inspector_layout(fx.root(), kAppleLayout));

    auto result = okResult(scrivi_get_inspector_layout(fx.root()));
    REQUIRE(result.getString("status") == "ok");

    JsonDoc doc = result.getSubDoc("document");

    // ⚠️ EVERY KEY, NAMED EXPLICITLY. A typed round trip in the core would drop
    // the last three and this test is what would catch it.
    REQUIRE(doc.getString("schema")      == "scrivi.inspector-layout.v1");
    REQUIRE(doc.getString("selectedTab") == "worldbuilding");
    REQUIRE(doc.getBool("inspectorHidden") == false);
    REQUIRE(doc.getString("stackSort")   == "manual");
    REQUIRE(doc.contains("defaultStacks"));
    REQUIRE(doc.contains("scenes"));

    // Nested structure survives intact, not just the top-level key names.
    JsonDoc scenes = doc.getSubDoc("scenes");
    REQUIRE(scenes.getSubDoc("scene-0001").arraySize("writing") == 1);
    REQUIRE(scenes.getSubDoc("scene-0001").arrayItem("writing", 0).getString("type")
            == "todo");
}

TEST_CASE("inspector layout: a key the core has never heard of survives",
          "[inspector-layout][capi]") {
    LayoutFixture fx;

    // ⚠️ THIS IS [I-0215] IN ONE ASSERTION. The core must not interpret the
    // document, so a key invented by a FUTURE Scrivi must round-trip untouched.
    constexpr const char* future = R"({
      "schema": "scrivi.inspector-layout.v1",
      "selectedTab": "writing",
      "somethingFromScrivi2099": {"nested": [1, 2, 3]}
    })";

    okResult(scrivi_put_inspector_layout(fx.root(), future));
    auto doc = okResult(scrivi_get_inspector_layout(fx.root())).getSubDoc("document");

    REQUIRE(doc.contains("somethingFromScrivi2099"));
    REQUIRE(doc.getSubDoc("somethingFromScrivi2099").arraySize("nested") == 3);
}

TEST_CASE("inspector layout: a corrupt file is reported and LEFT ALONE",
          "[inspector-layout][capi]") {
    LayoutFixture fx;

    const std::string damaged = R"({"selectedTab": "writing", TRUNCATED)";
    fx.writeLayoutFile(damaged);

    auto result = okResult(scrivi_get_inspector_layout(fx.root()));

    // ⚠️ `unreadable` is DISTINCT from `absent` on purpose: the app defaults for
    // both but can only WARN about this one. Collapsing them loses that.
    REQUIRE(result.getString("status") == "unreadable");
    REQUIRE_FALSE(result.getString("message").empty());
    REQUIRE_FALSE(result.contains("document"));

    // ⛔ THE DAMAGED BYTES ARE STILL THERE, UNCHANGED. The writer's layout may be
    // recoverable by hand; clobbering it on open would destroy that chance.
    REQUIRE(fx.readLayoutFile() == damaged);
}

TEST_CASE("inspector layout: a put OVER a corrupt file succeeds",
          "[inspector-layout][capi]") {
    LayoutFixture fx;
    fx.writeLayoutFile(R"({"selectedTab": "writing", TRUNCATED)");

    // ⚠️ The READ path leaves damage alone; a WRITE is the writer's explicit act
    // and must not be blocked by the state of what it replaces.
    okResult(scrivi_put_inspector_layout(fx.root(), kAppleLayout));

    auto result = okResult(scrivi_get_inspector_layout(fx.root()));
    REQUIRE(result.getString("status") == "ok");
    REQUIRE(result.getSubDoc("document").getString("selectedTab") == "worldbuilding");
}

TEST_CASE("inspector layout: an empty object is a legal document",
          "[inspector-layout][capi]") {
    LayoutFixture fx;

    // ⚠️ A writer may legitimately clear their layout. The shape is what is
    // validated, NOT whether the document carries any keys.
    okResult(scrivi_put_inspector_layout(fx.root(), "{}"));

    auto result = okResult(scrivi_get_inspector_layout(fx.root()));
    REQUIRE(result.getString("status") == "ok");
    REQUIRE(result.getSubDoc("document").objectKeys().empty());
}

TEST_CASE("inspector layout: a non-object document is REJECTED, not stored",
          "[inspector-layout][capi]") {
    LayoutFixture fx;

    // ⚠️ These all parse as valid JSON. Storing one would manufacture exactly the
    // corruption the read path exists to report, so this is the single thing the
    // core validates about an otherwise opaque document.
    for (const char* bad : {R"(["an","array"])", "42", R"("a string")", "null"}) {
        INFO("payload: " << bad);
        auto err = errorOf(scrivi_put_inspector_layout(fx.root(), bad));
        REQUIRE(err.getString("message").find("must be a JSON object")
                != std::string::npos);
    }

    // Malformed JSON is rejected too, with a different message.
    auto err = errorOf(scrivi_put_inspector_layout(fx.root(), "{not json"));
    REQUIRE(err.getString("message").find("not valid JSON") != std::string::npos);

    // ⛔ NOTHING WAS WRITTEN by any of the rejections.
    REQUIRE_FALSE(fs::exists(fx.layoutPath()));
}

TEST_CASE("inspector layout: missing arguments are rejected", "[inspector-layout][capi]") {
    LayoutFixture fx;

    REQUIRE(errorOf(scrivi_get_inspector_layout(""))
                .getString("message").find("projectRootPath") != std::string::npos);
    REQUIRE(errorOf(scrivi_put_inspector_layout("", kAppleLayout))
                .getString("message").find("projectRootPath") != std::string::npos);
    REQUIRE(errorOf(scrivi_put_inspector_layout(fx.root(), ""))
                .getString("message").find("documentJson") != std::string::npos);

    // ⚠️ NULL is the documented ABI contract (scrivi.h:20): treated as an empty
    // string, never a crash. So it must behave exactly like "" above — an error
    // envelope naming the missing argument, not a segfault and not an ok.
    REQUIRE(errorOf(scrivi_get_inspector_layout(nullptr))
                .getString("message").find("projectRootPath") != std::string::npos);
    REQUIRE(errorOf(scrivi_put_inspector_layout(nullptr, nullptr))
                .getString("message").find("projectRootPath") != std::string::npos);
}
