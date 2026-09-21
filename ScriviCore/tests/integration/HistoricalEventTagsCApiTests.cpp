// EP-041 SP-142 (T-0542 / [I-0241]) — historical-event `tags` AT THE C ABI.
//
// ⚠️ WHY THIS FILE EXISTS. `scrivi_create_historical_event` and
// `scrivi_update_historical_event` both ACCEPT `tagsJSON`, but
// `scrivi_list_historical_events` DID NOT PROJECT IT — so tags could be written
// and never read back through the boundary.
//
// ⚠️ THAT GAP HAD A REAL COST. Linux answered it by reading the package itself:
// `readHistoricalEventTagsFromDisk` (`platforms/linux/src/EditorShell.cpp`) listed
// and parsed EVERY file in `objects/historical-events/` to recover one field —
// ⛔ the [I-0197] bypass shape, and the same pattern Apple retired in SP-129.
//
// ⚠️ WORSE, ONE CALLER MADE IT A DATA-LOSS PATH. `onHistoricalEventDragged`
// re-read tags off disk ONLY to re-send them, because `updateHistoricalEvent`
// overwrites all fields. ⛔ The disk read returned an empty list on EVERY failure,
// so a read that failed for any reason SILENTLY ERASED that event's tags.
//
// ⚠️ THIS FILE IS AT THE ABI, NOT THE FACADE. `TimelineTests.cpp` exercises
// `core().listHistoricalEvents()` directly and was GREEN throughout — a facade
// test cannot see a projection gap at the boundary, which is exactly how I-0113
// shipped (`feedback_boundary_tests_not_facade`).

#include <catch2/catch_test_macros.hpp>

#include "scrivi/scrivi.h"
#include "util/Json.hpp"

#include <atomic>
#include <cstdint>
#include <filesystem>
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

struct EventFixture {
    fs::path projectDir;
    fs::path appSupportDir;

    EventFixture() {
        static std::atomic<int> counter{0};
        const std::string stem =
            "scrivi-hist-tags-" + std::to_string(counter.fetch_add(1)) + "-" +
            std::to_string(reinterpret_cast<std::uintptr_t>(this));

        projectDir    = fs::temp_directory_path() / (stem + ".scrivi");
        appSupportDir = fs::temp_directory_path() / (stem + "-appsupport");
        fs::create_directories(projectDir);
        fs::create_directories(appSupportDir);

        okResult(scrivi_create_project(root(), appSupport(), "Historical Tags",
                                       "historical-tags",
                                       "identity-001", "persona-001", "Test Author"));
    }

    ~EventFixture() {
        std::error_code ec;
        fs::remove_all(projectDir, ec);
        fs::remove_all(appSupportDir, ec);
    }

    [[nodiscard]] const char* root() const { return projectDir.c_str(); }
    [[nodiscard]] const char* appSupport() const { return appSupportDir.c_str(); }

    // Creates an event and returns its eventID.
    std::string create(const char* title, int64_t offsetMs, const char* tagsJSON) {
        auto r = okResult(scrivi_create_historical_event(
            root(), title, offsetMs, "a description", tagsJSON,
            "identity-001", "persona-001", "Test Author"));
        const std::string id = r.getString("eventID");
        REQUIRE_FALSE(id.empty());
        return id;
    }

    // The listed event with this eventID, as the ABI projects it.
    JsonDoc listed(const std::string& eventID) {
        auto r = okResult(scrivi_list_historical_events(root()));
        auto events = parseJson(r.getString("eventsJSON"));
        REQUIRE(events.ok());
        const auto n = events.value().arraySize("events");
        for (std::size_t i = 0; i < n; ++i) {
            JsonDoc item = events.value().arrayItem("events", i);
            if (item.getString("eventID") == eventID) { return item; }
        }
        FAIL("eventID " << eventID << " not found in listHistoricalEvents");
        return JsonDoc{};
    }
};

} // namespace

TEST_CASE("historical events: list PROJECTS tags", "[historical-events][capi]") {
    EventFixture fx;

    // ⚠️ THE ASSERTION T-0542 EXISTS FOR. Before the fix, `tags` was parsed into the
    // event struct and then dropped from the projection — so this read empty while
    // the bytes sat on disk.
    const std::string id = fx.create("The Sundering", 1000,
                                     R"(["cataclysm","first-age"])");

    JsonDoc item = fx.listed(id);
    const auto tags = item.getStringArray("tags");

    REQUIRE(tags.size() == 2);
    REQUIRE(tags[0] == "cataclysm");
    REQUIRE(tags[1] == "first-age");
}

TEST_CASE("historical events: tags survive an update", "[historical-events][capi]") {
    EventFixture fx;
    const std::string id = fx.create("The Sundering", 1000, R"(["cataclysm"])");

    // ⚠️ THE DATA-LOSS SHAPE, AT THE BOUNDARY. `update` overwrites ALL fields, so a
    // caller that cannot READ tags cannot RE-SEND them — which is why Linux read the
    // package directly, and why a failed read silently erased them. With tags
    // projected, a caller can round-trip what it was given.
    JsonDoc before = fx.listed(id);
    const auto carried = before.getStringArray("tags");
    REQUIRE(carried.size() == 1);

    // Re-send exactly what the list gave back, with a new offset (the drag case).
    okResult(scrivi_update_historical_event(fx.root(), id.c_str(), "The Sundering",
                                            5000, "a description",
                                            R"(["cataclysm"])"));

    JsonDoc after = fx.listed(id);
    REQUIRE(after.getInt64("offsetMs") == 5000);
    REQUIRE(after.getStringArray("tags") == carried);   // ⛔ NOT blanked
}

TEST_CASE("historical events: an event with NO tags lists cleanly",
          "[historical-events][capi]") {
    EventFixture fx;
    const std::string id = fx.create("A quiet year", 2000, "[]");

    // ⚠️ "No tags" must be distinguishable from "could not read" by the CALLER, and
    // at this layer that means: the call succeeds and the list is simply empty.
    // ⛔ It must NOT be an error — an untagged event is entirely normal.
    JsonDoc item = fx.listed(id);
    REQUIRE(item.getStringArray("tags").empty());
    REQUIRE(item.getString("title") == "A quiet year");
}

TEST_CASE("historical events: BOTH tagsJSON shapes are accepted",
          "[historical-events][capi]") {
    EventFixture fx;

    // ⚠️ THE SHAPES ARE NOT INTERCHANGEABLE IN THE WILD, AND BOTH ARE REAL:
    //   • `{"tags":[…]}` is what LINUX ACTUALLY SENDS (`tagsToJson`, EditorShell.cpp),
    //     and what Apple's `tagsJSON: String = "{}"` default implies.
    //   • `[…]` is what `scrivi.h` DOCUMENTS.
    // ⛔ Accepting only one breaks either a live caller or the published contract.
    // ⚠️ This test is the reason the T-0542 fix is not a one-liner: narrowing the
    // parse to the documented root array would have SILENTLY BROKEN LINUX, whose
    // tags happened to work through the old buggy path.
    const std::string wrapped = fx.create("Wrapped", 1000, R"({"tags":["alpha"]})");
    const std::string bare    = fx.create("Bare",    2000, R"(["beta"])");

    REQUIRE(fx.listed(wrapped).getStringArray("tags")
            == std::vector<std::string>{"alpha"});
    REQUIRE(fx.listed(bare).getStringArray("tags")
            == std::vector<std::string>{"beta"});
}

TEST_CASE("historical events: tags are projected for EVERY event, not just the first",
          "[historical-events][capi]") {
    EventFixture fx;

    // ⚠️ The Linux disk walk stopped at the first matching file. A projection that
    // only populated one entry would satisfy a single-event test and still be wrong.
    const std::string a = fx.create("First",  1000, R"(["alpha"])");
    const std::string b = fx.create("Second", 2000, R"(["beta","gamma"])");

    REQUIRE(fx.listed(a).getStringArray("tags") == std::vector<std::string>{"alpha"});
    REQUIRE(fx.listed(b).getStringArray("tags")
            == std::vector<std::string>{"beta", "gamma"});
}
