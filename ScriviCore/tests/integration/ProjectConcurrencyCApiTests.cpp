// SP-173 / I-0285 — D1 (one exclusive lock per project) and D2 (the project revision), tested THROUGH `scrivi_*`: a facade
// test cannot see the boundary, which is where the lock and the revision live. Design: docs/Scrivi_Core_Concurrency_Design_v0_1.md.

#include <catch2/catch_test_macros.hpp>

#include "scrivi/scrivi.h"
#include "public_api/ProjectLock.hpp"
#include "util/Json.hpp"

#include <atomic>
#include <chrono>
#include <cstdint>
#include <filesystem>
#include <string>
#include <thread>
#include <vector>

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

struct ConcurrencyFixture {
    fs::path projectDir;
    fs::path appSupportDir;

    ConcurrencyFixture() {
        static std::atomic<int> counter{0};
        const std::string stem = "scrivi-concurrency-" + std::to_string(counter.fetch_add(1)) + "-" +
                                 std::to_string(reinterpret_cast<std::uintptr_t>(this));
        projectDir    = fs::temp_directory_path() / (stem + ".scrivi");
        appSupportDir = fs::temp_directory_path() / (stem + "-appsupport");
        fs::create_directories(projectDir);
        fs::create_directories(appSupportDir);
        JsonDoc env = envelope(scrivi_create_project(root(), appSupport(), "Concurrency", "concurrency",
                                                     "identity-001", "persona-001", "Test Author"));
        REQUIRE(env.getBool("ok"));
    }
    ~ConcurrencyFixture() {
        std::error_code ec;
        fs::remove_all(projectDir, ec);
        fs::remove_all(appSupportDir, ec);
    }
    [[nodiscard]] const char* root() const { return projectDir.c_str(); }
    [[nodiscard]] const char* appSupport() const { return appSupportDir.c_str(); }
};

// ⚠️ For WORKER threads: no Catch2 assertion (Catch2 is not thread-safe — ThreadSanitizer caught the first version of these
// tests asserting from workers). A parse failure comes back as an empty document; the caller counts it.
JsonDoc envelopeNoAssert(const char* raw) {
    if (raw == nullptr) { return JsonDoc{}; }
    auto parsed = parseJson(raw);
    scrivi_free(raw);
    return parsed.ok() ? std::move(parsed.value()) : JsonDoc{};
}

JsonDoc readSettings(const char* root) { return envelopeNoAssert(scrivi_get_project_settings(root)); }

JsonDoc writeSettings(const char* root, const std::string& subtitle) {
    const std::string doc = R"({"subtitle":")" + subtitle + R"("})";
    return envelopeNoAssert(scrivi_put_project_settings(root, doc.c_str()));
}

}  // namespace

TEST_CASE("D2: every project envelope carries the revision; a write bumps it, a read does not",
          "[concurrency][capi]") {
    ConcurrencyFixture f;
    JsonDoc r0 = readSettings(f.root());
    REQUIRE(r0.getBool("ok"));
    REQUIRE(r0.contains("revision"));
    const int64_t base = r0.getInt64("revision");

    CHECK(readSettings(f.root()).getInt64("revision") == base);                 // a read changes nothing

    JsonDoc w = writeSettings(f.root(), "one");
    REQUIRE(w.getBool("ok"));
    CHECK(w.getInt64("revision") == base + 1);                                  // a write reports the revision it made
    CHECK(readSettings(f.root()).getInt64("revision") == base + 1);

    // An ERROR envelope from a project endpoint carries it too: the caller learns the state it was refused against.
    JsonDoc bad = envelope(scrivi_put_project_settings(f.root(), "not json"));
    REQUIRE_FALSE(bad.getBool("ok"));
    CHECK(bad.contains("revision"));
}

TEST_CASE("D1: two spellings of one project root share ONE lock and ONE revision", "[concurrency][capi]") {
    ConcurrencyFixture f;
    const std::string slashed = f.projectDir.string() + "/";
    const std::string dotted  = (f.projectDir / ".").string();
    const int64_t base = readSettings(f.root()).getInt64("revision");
    REQUIRE(writeSettings(slashed.c_str(), "via slash").getBool("ok"));
    CHECK(readSettings(dotted.c_str()).getInt64("revision") == base + 1);
    CHECK(readSettings(f.root()).getInt64("revision") == base + 1);
}

TEST_CASE("D1: a call WAITS while another call holds its project's lock — and only its own project's",
          "[concurrency][capi]") {
    ConcurrencyFixture held;
    ConcurrencyFixture other;
    constexpr auto kHold = std::chrono::milliseconds(150);

    std::atomic<bool> locked{false};
    std::thread holder([&] {
        scrivi::abi::ProjectCallGuard guard(held.root(), scrivi::abi::ProjectCallGuard::Kind::write);
        locked = true;
        std::this_thread::sleep_for(kHold);
    });
    while (!locked) { std::this_thread::yield(); }

    // Another project is not held: no wait.
    JsonDoc free = readSettings(other.root());
    CHECK_FALSE(free.contains("lockWaitMs"));

    // The held project: the call waits for the holder, and says how long.
    const auto t0 = std::chrono::steady_clock::now();
    JsonDoc waited = readSettings(held.root());
    const auto elapsed = std::chrono::steady_clock::now() - t0;
    holder.join();
    REQUIRE(waited.getBool("ok"));
    CHECK(elapsed >= kHold / 2);
    REQUIRE(waited.contains("lockWaitMs"));
    CHECK(waited.getInt64("lockWaitMs") >= kHold.count() / 2);
}

TEST_CASE("D1 + D2 stress: a writer and a reader on one project — every call succeeds and revisions never go back",
          "[concurrency][capi]") {
    ConcurrencyFixture f;
    constexpr int kRounds = 200;
    std::atomic<int> failures{0};
    std::vector<int64_t> writerRevisions;
    std::vector<int64_t> readerRevisions;

    std::thread writer([&] {
        for (int i = 0; i < kRounds; ++i) {
            JsonDoc w = writeSettings(f.root(), "round-" + std::to_string(i));
            if (!w.getBool("ok")) { ++failures; }
            writerRevisions.push_back(w.getInt64("revision"));
        }
    });
    std::thread reader([&] {
        for (int i = 0; i < kRounds; ++i) {
            JsonDoc r = readSettings(f.root());
            if (!r.getBool("ok")) { ++failures; }
            // L1: a read sees a whole write or none — the subtitle is always one the writer wrote in full.
            const std::string subtitle = r.getSubDoc("result").getString("subtitle");
            if (!subtitle.empty() && subtitle.rfind("round-", 0) != 0) { ++failures; }
            readerRevisions.push_back(r.getInt64("revision"));
        }
    });
    writer.join();
    reader.join();

    CHECK(failures == 0);
    for (size_t i = 1; i < writerRevisions.size(); ++i) {
        CHECK(writerRevisions[i] == writerRevisions[i - 1] + 1);   // each write bumps exactly once
    }
    for (size_t i = 1; i < readerRevisions.size(); ++i) {
        CHECK(readerRevisions[i] >= readerRevisions[i - 1]);       // a reader never sees the project go back in time
    }
}

TEST_CASE("D3: scrivi_load_timeline returns exactly what the five standalone endpoints return, and one revision",
          "[concurrency][capi]") {
    ConcurrencyFixture f;
    // Real data in two parts, so the comparison is not empty against empty.
    JsonDoc opened = envelope(scrivi_open_project(f.root(), f.appSupport(), ""));
    REQUIRE(opened.getBool("ok"));
    const std::string sceneID = opened.getSubDoc("result").getSubDoc("activeScene").getString("sceneID");
    REQUIRE_FALSE(sceneID.empty());
    REQUIRE(envelope(scrivi_set_scene_story_time(f.root(), sceneID.c_str(), 5000, "manual", 0, 60000, "manual"))
                .getBool("ok"));
    REQUIRE(envelope(scrivi_create_historical_event(f.root(), "Coronation", -86400000, "A crown", "[\"war\"]",
                                                    "identity-001", "persona-001", "Test Author")).getBool("ok"));

    JsonDoc all = envelope(scrivi_load_timeline(f.root()));
    INFO("envelope: " << all.dump());
    REQUIRE(all.getBool("ok"));
    REQUIRE(all.contains("revision"));
    const JsonDoc parts = all.getSubDoc("result");

    struct Pair { const char* key; const char* (*standalone)(const char*); };
    const Pair pairs[] = {
        {"timeline",          scrivi_get_timeline},
        {"storyTimes",        scrivi_list_story_times},
        {"storyStructure",    scrivi_get_story_structure},
        {"historicalEvents",  scrivi_list_historical_events},
        {"importedTimelines", scrivi_list_imported_timelines},
    };
    for (const auto& p : pairs) {
        INFO("part: " << p.key);
        JsonDoc one = envelope(p.standalone(f.root()));
        REQUIRE(one.getBool("ok"));
        REQUIRE(parts.contains(p.key));
        CHECK(parts.getSubDoc(p.key).dump() == one.getSubDoc("result").dump());
        if (std::string(p.key) == "storyTimes")       { CHECK(one.getSubDoc("result").getInt("count") == 1); }
        if (std::string(p.key) == "historicalEvents") { CHECK(one.getSubDoc("result").getInt("count") == 1); }
        CHECK(one.getInt64("revision") == all.getInt64("revision"));   // reads: the revision did not move
    }
}

TEST_CASE("D3: a missing project root is refused, not answered with empty parts", "[concurrency][capi]") {
    JsonDoc env = envelope(scrivi_load_timeline(nullptr));
    CHECK_FALSE(env.getBool("ok"));
}

TEST_CASE("Q2: two projects in ONE process write into the world they share at the same time — both QUEUE, neither is refused",
          "[concurrency][capi][world]") {
    ConcurrencyFixture a;
    ConcurrencyFixture b;
    const fs::path pkg = a.appSupportDir / "Shared.scrivworld";
    JsonDoc created = envelope(scrivi_create_world(a.root(), pkg.c_str(), "Shared", "Year One"));
    REQUIRE(created.getBool("ok"));
    const std::string worldID = created.getSubDoc("result").getString("worldID");
    REQUIRE(envelope(scrivi_add_world(b.root(), pkg.c_str())).getBool("ok"));

    constexpr int kEach = 30;
    std::atomic<int> refused{0};
    std::atomic<int> failed{0};
    auto writer = [&](const ConcurrencyFixture& f, const char* who) {
        for (int i = 0; i < kEach; ++i) {
            const std::string name = std::string(who) + "-" + std::to_string(i);
            JsonDoc env = envelopeNoAssert(scrivi_create_object(f.root(), "artifact", name.c_str(), "",
                                                                "identity-001", "persona-001", "Test Author",
                                                                worldID.c_str()));
            if (!env.getBool("ok")) {
                ++failed;
                if (env.getSubDoc("error").getString("message").find("lock") != std::string::npos) { ++refused; }
            }
        }
    };
    std::thread ta(writer, std::cref(a), "a");
    std::thread tb(writer, std::cref(b), "b");
    ta.join();
    tb.join();

    CHECK(refused == 0);
    CHECK(failed == 0);
    JsonDoc listed = envelope(scrivi_list_objects(a.root(), "artifact"));
    REQUIRE(listed.getBool("ok"));
    INFO("list: " << listed.dump().substr(0, 400));
    int inWorld = 0;
    const JsonDoc res = listed.getSubDoc("result");
    for (std::size_t i = 0; i < res.arraySize("objects"); ++i) {
        if (res.arrayItem("objects", i).getString("worldID") == worldID) { ++inWorld; }
    }
    CHECK(inWorld == 2 * kEach);
}

TEST_CASE("D4: scrivi_get_project_revision reports the current revision without bumping it", "[concurrency][capi]") {
    ConcurrencyFixture f;
    JsonDoc r0 = envelope(scrivi_get_project_revision(f.root()));
    REQUIRE(r0.getBool("ok"));
    const int64_t base = r0.getInt64("revision");
    CHECK(envelope(scrivi_get_project_revision(f.root())).getInt64("revision") == base);
    REQUIRE(writeSettings(f.root(), "x").getBool("ok"));
    CHECK(envelope(scrivi_get_project_revision(f.root())).getInt64("revision") == base + 1);
    CHECK_FALSE(envelope(scrivi_get_project_revision(nullptr)).getBool("ok"));
}
