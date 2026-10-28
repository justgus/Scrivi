// EP-052 (SP-194, T-0624) — the per-project write queue, tested THROUGH `scrivi_*` (a facade test cannot see the boundary).
// Design: docs/Scrivi_Background_Writes_Design_v0_1.md W1–W7, docs/Scrivi_Background_Writes_Design_v0_2.md §3.
//
// To see ORDER and COALESCING, a test holds the project's disk lock from another thread: the writer's first job then
// waits for it, and everything queued meanwhile is still pending.

#include <catch2/catch_test_macros.hpp>

#include "scrivi/scrivi.h"
#include "public_api/ProjectLock.hpp"
#include "util/Json.hpp"

#include <algorithm>
#include <atomic>
#include <chrono>
#include <cstdint>
#include <filesystem>
#include <fstream>
#include <sstream>
#include <string>
#include <thread>
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

JsonDoc ok(const char* raw) {
    JsonDoc env = envelope(raw);
    INFO("envelope: " << env.dump());
    REQUIRE(env.getBool("ok"));
    return env.getSubDoc("result");
}

// ⚠️ For worker threads: no Catch2 assertion (Catch2 is not thread-safe).
JsonDoc quiet(const char* raw) {
    if (raw == nullptr) { return JsonDoc{}; }
    auto parsed = parseJson(raw);
    scrivi_free(raw);
    return parsed.ok() ? std::move(parsed.value()) : JsonDoc{};
}

struct Scene { std::string id, metadataPath, contentPath; };

// A project of `n` scenes in one chapter, each scene's text "Text of <id>.".
struct QueueProject {
    fs::path base, root, appSupport;
    std::string projectID;
    std::vector<Scene> scenes;

    explicit QueueProject(int n) {
        static std::atomic<int> counter{0};
        base = fs::temp_directory_path() / ("scrivi-queue-" + std::to_string(::getpid()) + "-" +
                                            std::to_string(counter.fetch_add(1)));
        fs::remove_all(base);
        root = base / "Queue.scrivi";
        appSupport = base / "support";
        fs::create_directories(appSupport);
        projectID = ok(scrivi_create_project(r(), as(), "Queue", "queue", "identity-001", "persona-001", "T"))
                        .getString("projectID");
        JsonDoc opened = ok(scrivi_open_project(r(), as(), ""));
        const std::string chapterID = opened.arrayItem("scenes", 0).getString("chapterID");
        std::string after = opened.getSubDoc("activeScene").getString("sceneID");
        for (int i = 1; i < n; ++i) {
            after = ok(scrivi_create_scene(r(), as(), projectID.c_str(), chapterID.c_str(), after.c_str(), "",
                                           "identity-001", "persona-001", "T")).getString("sceneID");
        }
        JsonDoc reopened = ok(scrivi_open_project(r(), as(), ""));
        for (std::size_t i = 0; i < reopened.arraySize("scenes"); ++i) {
            const JsonDoc s = reopened.arrayItem("scenes", i);
            scenes.push_back({s.getString("sceneID"), s.getString("metadataPath"), s.getString("contentPath")});
            std::ofstream(root / s.getString("contentPath"), std::ios::binary | std::ios::trunc)
                << "Text of " << s.getString("sceneID") << ".";
        }
        REQUIRE(static_cast<int>(scenes.size()) == n);
    }
    ~QueueProject() {
        scrivi_free(scrivi_wait_for_writes(r(), 0, -1));
        scrivi_free(scrivi_history_close(r()));
        scrivi_free(scrivi_close_project(r()));
        std::error_code ec;
        fs::remove_all(base, ec);
    }
    [[nodiscard]] const char* r() const { return root.c_str(); }
    [[nodiscard]] const char* as() const { return appSupport.c_str(); }

    // One scene's entry for scrivi_enqueue_save_scenes.
    std::string saves(const std::vector<std::pair<const Scene*, std::string>>& list) const {
        JsonDoc req;
        req.setString("projectID", projectID);
        req.setString("appSupportRoot", appSupport.string());
        req.setString("identityID", "identity-001");
        req.setString("personaID", "persona-001");
        req.setString("displayName", "T");
        for (const auto& [s, text] : list) {
            JsonDoc e;
            e.setString("sceneID", s->id);
            e.setString("metadataPath", s->metadataPath);
            e.setString("contentPath", s->contentPath);
            e.setString("markdown", text);
            req.appendToArray("scenes", std::move(e));
        }
        return req.dump(-1);
    }
    int64_t enqueue(const Scene& s, const std::string& text) const {
        return ok(scrivi_enqueue_save_scenes(r(), saves({{&s, text}}).c_str())).getInt64("ticket");
    }
    std::string onDisk(const Scene& s) const {
        std::ifstream in(root / s.contentPath, std::ios::binary);
        std::stringstream ss;
        ss << in.rdbuf();
        return ss.str();
    }
    std::string historyLog() const {
        std::ifstream in(root / "history" / "log-000001.jsonl", std::ios::binary);
        std::stringstream ss;
        ss << in.rdbuf();
        return ss.str();
    }
};

// Holds the project's disk lock on another thread until `release()`.
struct DiskHold {
    std::atomic<bool> held{false}, go{false};
    std::thread t;
    explicit DiskHold(const char* root) {
        t = std::thread([this, root] {
            scrivi::abi::ProjectCallGuard guard(root, scrivi::abi::ProjectCallGuard::Kind::write);
            held = true;
            while (!go) { std::this_thread::sleep_for(std::chrono::milliseconds(1)); }
        });
        while (!held) { std::this_thread::yield(); }
    }
    void release() {
        go = true;
        if (t.joinable()) { t.join(); }
    }
    ~DiskHold() { release(); }
};

std::vector<JsonDoc> outcomesOf(const JsonDoc& waited) {
    std::vector<JsonDoc> out;
    for (std::size_t i = 0; i < waited.arraySize("outcomes"); ++i) { out.push_back(waited.arrayItem("outcomes", i)); }
    return out;
}

}  // namespace

TEST_CASE("EP-052 queue: saves land in the order queued, each with an outcome, and tickets increase",
          "[write-queue][capi]") {
    QueueProject p(5);
    std::vector<int64_t> tickets;
    for (const auto& s : p.scenes) { tickets.push_back(p.enqueue(s, "Saved " + s.id + ".")); }
    for (std::size_t i = 1; i < tickets.size(); ++i) { CHECK(tickets[i] > tickets[i - 1]); }

    const JsonDoc waited = ok(scrivi_wait_for_writes(p.r(), tickets.back(), -1));
    CHECK(waited.getBool("landed"));
    const auto outcomes = outcomesOf(waited);
    REQUIRE(outcomes.size() == p.scenes.size());
    for (std::size_t i = 0; i < outcomes.size(); ++i) {
        CHECK(outcomes[i].getInt64("ticket") == tickets[i]);
        CHECK(outcomes[i].getString("kind") == "save");
        CHECK(outcomes[i].getString("sceneID") == p.scenes[i].id);
        CHECK(outcomes[i].getBool("ok"));
        CHECK(p.onDisk(p.scenes[i]) == "Saved " + p.scenes[i].id + ".");
    }
    // Reported once: the next wait has nothing new.
    CHECK_FALSE(ok(scrivi_wait_for_writes(p.r(), 0, 0)).contains("outcomes"));
}

TEST_CASE("EP-052 queue: a scene queued again before its write starts is written once, with the newer text",
          "[write-queue][capi]") {
    QueueProject p(2);
    const Scene& a = p.scenes[0];
    const Scene& b = p.scenes[1];
    int64_t first = 0, v2 = 0, other = 0, v3 = 0;
    {
        DiskHold hold(p.r());
        first = p.enqueue(a, "A one.");   // the writer takes it, and waits for the disk
        std::this_thread::sleep_for(std::chrono::milliseconds(50));
        v2 = p.enqueue(a, "A two.");
        other = p.enqueue(b, "B one.");
        v3 = p.enqueue(a, "A three.");    // replaces v2, which has not started
    }
    const auto outcomes = outcomesOf(ok(scrivi_wait_for_writes(p.r(), 0, -1)));
    REQUIRE(outcomes.size() == 4);
    CHECK(outcomes[0].getInt64("ticket") == v2);
    CHECK(outcomes[0].getInt64("supersededBy") == v3);
    CHECK(outcomes[1].getInt64("ticket") == first);
    CHECK(outcomes[2].getInt64("ticket") == other);
    CHECK(outcomes[3].getInt64("ticket") == v3);
    CHECK(p.onDisk(a) == "A three.");
    CHECK(p.onDisk(b) == "B one.");
}

TEST_CASE("EP-052 queue: a save is never coalesced across a queued structural operation (W7e)", "[write-queue][capi]") {
    QueueProject p(2);
    const Scene& a = p.scenes[0];
    int64_t v2 = 0, structural = 0, v3 = 0;
    {
        DiskHold hold(p.r());
        p.enqueue(a, "A one.");
        std::this_thread::sleep_for(std::chrono::milliseconds(50));
        v2 = p.enqueue(a, "A two.");
        // A cut of a Range the manuscript does not hold: refused when it RUNS (W7c), but it is a structural job.
        structural = ok(scrivi_enqueue_range_op(p.r(), R"({"op":"cut","range":{"entries":[]}})")).getInt64("ticket");
        v3 = p.enqueue(a, "A three.");
    }
    const auto outcomes = outcomesOf(ok(scrivi_wait_for_writes(p.r(), 0, -1)));
    REQUIRE(outcomes.size() == 4);
    for (const auto& o : outcomes) { CHECK_FALSE(o.contains("supersededBy")); }
    CHECK(outcomes[1].getInt64("ticket") == v2);
    CHECK(outcomes[2].getInt64("ticket") == structural);
    CHECK(outcomes[2].getString("kind") == "structural");
    CHECK_FALSE(outcomes[2].getBool("ok"));
    CHECK(outcomes[2].contains("error"));
    CHECK(outcomes[3].getInt64("ticket") == v3);
    CHECK(p.onDisk(a) == "A three.");
}

TEST_CASE("EP-052 queue: a synchronous read sees every write queued before it (W3)", "[write-queue][capi]") {
    QueueProject p(2);
    const Scene& a = p.scenes[1];
    JsonDoc read;
    {
        DiskHold hold(p.r());
        p.enqueue(p.scenes[0], "Ahead of it.");   // the writer takes this one and waits for the disk with the read
        std::this_thread::sleep_for(std::chrono::milliseconds(50));
        p.enqueue(a, "Queued, not yet written.");   // so this one can only be written before the read if the read waits
        std::thread releaser([&] {
            std::this_thread::sleep_for(std::chrono::milliseconds(100));
            hold.release();
        });
        read = ok(scrivi_read_scene_texts(p.r(), ("[\"" + a.id + "\"]").c_str()));
        releaser.join();
    }
    CHECK(read.arrayItem("scenes", 0).getString("markdown") == "Queued, not yet written.");
}

TEST_CASE("EP-052 queue: a memory call returns at once while the writer waits for the disk", "[write-queue][capi]") {
    QueueProject p(1);
    ok(scrivi_history_open(p.r()));
    DiskHold hold(p.r());
    p.enqueue(p.scenes[0], "Start. More.");
    const auto t0 = std::chrono::steady_clock::now();
    // A sentence end COMMITS a step: its history append is queued, not written here.
    const JsonDoc edit = ok(scrivi_capture_note_edit(p.r(), p.scenes[0].id.c_str(), "Start. More.", 12, "Start.", 1000));
    const JsonDoc caret = ok(scrivi_capture_caret_moved(p.r(), p.scenes[0].id.c_str(), 0, 0));
    const auto elapsed = std::chrono::steady_clock::now() - t0;
    hold.release();
    CHECK(edit.getBool("committed"));
    CHECK(elapsed < std::chrono::milliseconds(100));
    ok(scrivi_wait_for_writes(p.r(), 0, -1));
    CHECK(p.historyLog().find("\"rec\":\"event\"") != std::string::npos);   // the step's append landed, after the save
}

namespace {
// SP-195 Q1 (a): a scene whose folder is gone, as when the drive is pulled; `restore()` brings the folder back.
struct Unreachable {
    Scene scene;
    fs::path folder;
    Unreachable(const QueueProject& p, const Scene& s) : scene(s) {
        scene.contentPath = "gone/" + s.contentPath;
        folder = (p.root / scene.contentPath).parent_path();
        scrivi::abi::projectLockEntry(p.r()).queue.setRetryInterval(std::chrono::milliseconds(20));
    }
    void restore() const { fs::create_directories(folder); }
};

// A held queue's wait returns at once, so a test waits for the retry by asking again; the outcomes go to `into`.
JsonDoc waitUntilLanded(const QueueProject& p, std::vector<JsonDoc>& into) {
    const auto deadline = std::chrono::steady_clock::now() + std::chrono::seconds(10);
    for (;;) {
        JsonDoc waited = ok(scrivi_wait_for_writes(p.r(), 0, 100));
        for (auto& o : outcomesOf(waited)) { into.push_back(std::move(o)); }
        if (waited.getBool("landed") || std::chrono::steady_clock::now() > deadline) { return waited; }
    }
}

int64_t count(const std::vector<JsonDoc>& outcomes, int64_t ticket, bool okValue) {
    return std::count_if(outcomes.begin(), outcomes.end(), [&](const JsonDoc& o) {
        return o.getInt64("ticket") == ticket && o.getBool("ok") == okValue && !o.contains("supersededBy");
    });
}
}  // namespace

TEST_CASE("EP-052 queue: a save that cannot reach the disk HOLDS the queue, is retried, and lands when it can "
          "(SP-195 Q1 (a))", "[write-queue][capi]") {
    QueueProject p(2);
    ok(scrivi_history_open(p.r()));
    const Unreachable gone(p, p.scenes[0]);
    const Scene& later = p.scenes[1];
    const int64_t held = p.enqueue(gone.scene, "Kept until the drive is back.");
    const int64_t behind = p.enqueue(later, "Queued behind it.");

    // A wait returns at once: nothing hangs behind a held queue.
    const auto t0 = std::chrono::steady_clock::now();
    JsonDoc waited = ok(scrivi_wait_for_writes(p.r(), 0, -1));
    CHECK(std::chrono::steady_clock::now() - t0 < std::chrono::seconds(2));
    CHECK_FALSE(waited.getBool("landed"));
    CHECK(waited.getBool("held"));
    CHECK_FALSE(waited.getString("heldReason").empty());
    CHECK(waited.getInt64("pending") >= 2);
    std::vector<JsonDoc> outcomes = outcomesOf(waited);

    // A synchronous file call fails at once instead of waiting behind it.
    const JsonDoc refused = envelope(scrivi_read_scene_texts(p.r(), ("[\"" + later.id + "\"]").c_str()));
    CHECK_FALSE(refused.getBool("ok"));
    CHECK(refused.getSubDoc("error").getString("detail") == "writeQueueHeld");

    // Retried several times, but its failure is reported once; the job behind it has not run.
    std::this_thread::sleep_for(std::chrono::milliseconds(150));
    for (auto& o : outcomesOf(ok(scrivi_wait_for_writes(p.r(), 0, 0)))) { outcomes.push_back(std::move(o)); }
    CHECK(count(outcomes, held, false) == 1);
    CHECK(std::any_of(outcomes.begin(), outcomes.end(),
                      [&](const JsonDoc& o) { return o.getInt64("ticket") == held && o.getBool("held"); }));
    CHECK(count(outcomes, behind, true) == 0);
    CHECK(p.onDisk(later) == "Text of " + later.id + ".");
    CHECK(p.historyLog().find("\"sceneID\":\"" + gone.scene.id + "\",\"sha256\"") == std::string::npos);   // W2

    gone.restore();
    waited = waitUntilLanded(p, outcomes);
    CHECK(waited.getBool("landed"));
    CHECK_FALSE(waited.getBool("held"));
    CHECK(count(outcomes, held, true) == 1);
    CHECK(count(outcomes, behind, true) == 1);
    CHECK(p.onDisk(gone.scene) == "Kept until the drive is back.");
    CHECK(p.onDisk(later) == "Queued behind it.");
    ok(scrivi_history_close(p.r()));
    CHECK(p.historyLog().find("\"rec\":\"persisted\",\"sceneID\":\"" + gone.scene.id) != std::string::npos);
}

TEST_CASE("EP-052 queue: a structural operation never runs over a held save (SP-195 Q1 (a))", "[write-queue][capi]") {
    QueueProject p(2);
    const Scene& a = p.scenes[0];
    const Scene& b = p.scenes[1];
    const auto aLen = static_cast<long long>(p.onDisk(a).size());
    JsonDoc op;
    op.setString("op", "cut");
    op.setSubDoc("range", ok(scrivi_range_read(p.r(), a.id.c_str(), aLen, b.id.c_str(), 0)).getSubDoc("range"));
    op.setString("identityID", "identity-001");
    op.setString("personaID", "persona-001");
    op.setString("displayName", "T");
    const Unreachable gone(p, b);
    const int64_t save = p.enqueue(gone.scene, "Lost if the merge ran first.");
    const int64_t merge = ok(scrivi_enqueue_range_op(p.r(), op.dump(-1).c_str())).getInt64("ticket");
    std::this_thread::sleep_for(std::chrono::milliseconds(100));
    CHECK(p.onDisk(a) == "Text of " + a.id + ".");   // the merge waits

    gone.restore();
    std::vector<JsonDoc> outcomes;
    waitUntilLanded(p, outcomes);
    // The save landed first, then the merge (the stand-in folder is not b's, so the merged text is b's own).
    const auto at = [&](int64_t t) { return std::find_if(outcomes.begin(), outcomes.end(), [&](const JsonDoc& o) {
        return o.getInt64("ticket") == t && o.getBool("ok"); }); };
    REQUIRE(at(save) != outcomes.end());
    REQUIRE(at(merge) != outcomes.end());
    CHECK(at(save) < at(merge));
    CHECK(p.onDisk(a) == "Text of " + a.id + ".Text of " + b.id + ".");
}

TEST_CASE("EP-052 queue: a write that fails for any other reason is reported, and the queue goes on (W2, W4)",
          "[write-queue][capi]") {
    QueueProject p(2);
    ok(scrivi_history_open(p.r()));
    const Scene good = p.scenes[0];
    const Scene bad = p.scenes[1];
    auto parsedReq = parseJson(p.saves({{&bad, "Never written."}}));
    JsonDoc req = std::move(parsedReq.value());
    req.setString("appSupportRoot", "");   // refused (invalidArgument): never worth retrying
    const int64_t failed = ok(scrivi_enqueue_save_scenes(p.r(), req.dump(-1).c_str())).getInt64("ticket");
    const int64_t after = p.enqueue(good, "Written.");
    const JsonDoc waited = ok(scrivi_wait_for_writes(p.r(), after, -1));
    CHECK(waited.getBool("landed"));
    CHECK_FALSE(waited.getBool("held"));
    const auto outcomes = outcomesOf(waited);
    REQUIRE(outcomes.size() == 2);
    CHECK(outcomes[0].getInt64("ticket") == failed);
    CHECK_FALSE(outcomes[0].getBool("ok"));
    CHECK(outcomes[0].contains("error"));
    CHECK_FALSE(outcomes[0].getBool("held"));
    CHECK(outcomes[1].getBool("ok"));
    ok(scrivi_history_close(p.r()));
    const std::string log = p.historyLog();
    CHECK(log.find("\"rec\":\"persisted\",\"sceneID\":\"" + good.id) != std::string::npos);
    CHECK(log.find("\"sceneID\":\"" + bad.id + "\",\"sha256\"") == std::string::npos);
}

// [SP-196] rig: an undo of a split removed a scene, and a save of it queued after the undo held the queue for good (and the
// first rig run left its .md behind with no meta).
TEST_CASE("EP-052 queue: a save of a scene a queued structural job removed is refused, never held, and writes nothing",
          "[write-queue][capi]") {
    QueueProject p(2);
    const Scene a = p.scenes[0];
    const Scene gone = p.scenes[1];
    const auto aLen = static_cast<long long>(p.onDisk(a).size());
    JsonDoc op;   // the break between the two scenes cut: a merge, which removes the second
    op.setString("op", "cut");
    op.setSubDoc("range", ok(scrivi_range_read(p.r(), a.id.c_str(), aLen, gone.id.c_str(), 0)).getSubDoc("range"));
    op.setString("identityID", "identity-001");
    op.setString("personaID", "persona-001");
    op.setString("displayName", "T");
    ok(scrivi_enqueue_range_op(p.r(), op.dump(-1).c_str()));
    const int64_t refused = p.enqueue(gone, "Never written.");   // queued after the merge, as the autosave did
    const int64_t after = p.enqueue(a, "Written.");
    const JsonDoc waited = ok(scrivi_wait_for_writes(p.r(), after, -1));
    CHECK(waited.getBool("landed"));
    CHECK_FALSE(waited.getBool("held"));
    const auto outcomes = outcomesOf(waited);
    REQUIRE(outcomes.size() == 3);
    CHECK(outcomes[0].getBool("ok"));   // the merge
    CHECK(outcomes[1].getInt64("ticket") == refused);
    CHECK_FALSE(outcomes[1].getBool("ok"));
    CHECK_FALSE(outcomes[1].getBool("held"));
    CHECK(outcomes[2].getBool("ok"));
    CHECK_FALSE(fs::exists(p.root / gone.contentPath));
    CHECK(p.onDisk(a) == "Written.");
}

TEST_CASE("EP-052 queue: closing with a held queue returns, and its writes land when the project is back "
          "(SP-195 Q1 (a))", "[write-queue][capi]") {
    QueueProject p(1);
    const Unreachable gone(p, p.scenes[0]);
    p.enqueue(gone.scene, "Written after the close.");
    std::this_thread::sleep_for(std::chrono::milliseconds(50));
    const auto t0 = std::chrono::steady_clock::now();
    ok(scrivi_history_close(p.r()));
    ok(scrivi_close_project(p.r()));
    CHECK(std::chrono::steady_clock::now() - t0 < std::chrono::seconds(2));
    gone.restore();
    std::vector<JsonDoc> outcomes;
    CHECK(waitUntilLanded(p, outcomes).getBool("landed"));
    CHECK(p.onDisk(gone.scene) == "Written after the close.");
}

TEST_CASE("EP-052 queue: a queued structural operation's outcome carries its delta (W7-Q1 (a))", "[write-queue][capi]") {
    QueueProject p(2);
    const Scene& a = p.scenes[0];
    const Scene& b = p.scenes[1];
    const auto aLen = static_cast<long long>(p.onDisk(a).size());
    // The zero-length Range across the break between the two scenes: cutting it is a scene merge.
    JsonDoc op;
    op.setString("op", "cut");
    op.setSubDoc("range", ok(scrivi_range_read(p.r(), a.id.c_str(), aLen, b.id.c_str(), 0)).getSubDoc("range"));
    op.setString("identityID", "identity-001");
    op.setString("personaID", "persona-001");
    op.setString("displayName", "T");
    const int64_t ticket = ok(scrivi_enqueue_range_op(p.r(), op.dump(-1).c_str())).getInt64("ticket");
    const auto outcomes = outcomesOf(ok(scrivi_wait_for_writes(p.r(), ticket, -1)));
    REQUIRE(outcomes.size() == 1);
    INFO("outcome: " << outcomes[0].dump());
    CHECK(outcomes[0].getString("kind") == "structural");
    REQUIRE(outcomes[0].getBool("ok"));
    CHECK(outcomes[0].getSubDoc("result").contains("delta"));
    CHECK(p.onDisk(a) == "Text of " + a.id + ".Text of " + b.id + ".");
}

TEST_CASE("EP-052 queue: closing the project waits for its queued writes", "[write-queue][capi]") {
    QueueProject p(3);
    {
        DiskHold hold(p.r());
        for (const auto& s : p.scenes) { p.enqueue(s, "Before close: " + s.id); }
        std::thread releaser([&] {
            std::this_thread::sleep_for(std::chrono::milliseconds(100));
            hold.release();
        });
        ok(scrivi_close_project(p.r()));
        releaser.join();
    }
    for (const auto& s : p.scenes) { CHECK(p.onDisk(s) == "Before close: " + s.id); }
}

TEST_CASE("EP-052 queue: stress — saves and captures on one thread, reads and the revision on another",
          "[write-queue][capi][stress]") {
    QueueProject p(8);
    ok(scrivi_history_open(p.r()));
    constexpr int kRounds = 60;
    std::atomic<int> readFailures{0}, wrongTexts{0};
    std::atomic<bool> done{false};
    std::thread reader([&] {
        std::string ids = "[";
        for (std::size_t i = 0; i < p.scenes.size(); ++i) { ids += (i ? ",\"" : "\"") + p.scenes[i].id + "\""; }
        ids += "]";
        while (!done) {
            JsonDoc env = quiet(scrivi_read_scene_texts(p.r(), ids.c_str()));
            if (!env.getBool("ok")) { ++readFailures; continue; }
            const JsonDoc r = env.getSubDoc("result");
            for (std::size_t i = 0; i < r.arraySize("scenes"); ++i) {
                const std::string text = r.arrayItem("scenes", i).getString("markdown");
                if (!text.starts_with("Text of ") && !text.starts_with("Round ")) { ++wrongTexts; }
            }
            quiet(scrivi_get_project_revision(p.r()));
        }
    });
    for (int round = 0; round < kRounds; ++round) {
        const Scene& s = p.scenes[static_cast<std::size_t>(round) % p.scenes.size()];
        p.enqueue(s, "Round " + std::to_string(round) + ".");
        quiet(scrivi_capture_note_edit(p.r(), s.id.c_str(), ("Round " + std::to_string(round) + ".").c_str(), 1,
                                       "", 1000 + round));
        quiet(scrivi_capture_caret_moved(p.r(), s.id.c_str(), 0, 0));
    }
    const JsonDoc waited = ok(scrivi_wait_for_writes(p.r(), 0, -1));
    done = true;
    reader.join();
    CHECK(readFailures == 0);
    CHECK(wrongTexts == 0);
    for (const auto& o : outcomesOf(waited)) { CHECK(o.getBool("ok")); }
    for (std::size_t i = 0; i < p.scenes.size(); ++i) {
        int last = -1;
        for (int round = 0; round < kRounds; ++round) {
            if (static_cast<std::size_t>(round) % p.scenes.size() == i) { last = round; }
        }
        CHECK(p.onDisk(p.scenes[i]) == "Round " + std::to_string(last) + ".");
    }
}
