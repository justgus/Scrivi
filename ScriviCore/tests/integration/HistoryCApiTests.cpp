#include <catch2/catch_test_macros.hpp>

#include "scrivi/scrivi.h"
#include "util/Json.hpp"

#include <atomic>
#include <filesystem>
#include <fstream>
#include <sstream>
#include <string>

// Round-trips the scrivi_history_* C ABI through its JSON envelopes (T-0202).
// The history now persists to <root>/history/ (SP-054), so each test uses a
// UNIQUE temp root and removes it — otherwise one test's log would replay into
// the next test's open().

using scrivi::util::JsonDoc;
using scrivi::util::parseJson;

namespace {

// Parses an ABI envelope, requires ok==true, and returns the "result" sub-doc.
JsonDoc okResult(const char* raw) {
    REQUIRE(raw != nullptr);
    auto parsed = parseJson(raw);
    scrivi_free(raw);
    REQUIRE(parsed.ok());
    JsonDoc env = std::move(parsed.value());
    REQUIRE(env.getBool("ok"));
    return env.getSubDoc("result");
}

// Parses an ABI envelope and returns the whole doc (for error assertions).
JsonDoc envelope(const char* raw) {
    REQUIRE(raw != nullptr);
    auto parsed = parseJson(raw);
    scrivi_free(raw);
    REQUIRE(parsed.ok());
    return std::move(parsed.value());
}

// Builds the record-event params JSON.
std::string recordParams(const char* kind, int64_t before, int64_t after) {
    JsonDoc p;
    p.setString("kind", kind);
    p.setInt64("cursorBefore", before);
    p.setInt64("cursorAfter", after);
    return p.dump();
}

// A unique temp project root that removes itself (incl. its history/ dir).
struct HistoryRoot {
    std::string path;
    HistoryRoot() {
        static std::atomic<int> counter{0};
        path = (std::filesystem::temp_directory_path() /
                ("scrivi-history-capi-" + std::to_string(counter.fetch_add(1)) + "-" +
                 std::to_string(reinterpret_cast<std::uintptr_t>(this)) + ".scrivi"))
                   .string();
    }
    ~HistoryRoot() { std::error_code ec; std::filesystem::remove_all(path, ec); }
    const char* c() const { return path.c_str(); }
};

} // namespace

TEST_CASE("C ABI: open mints a session and reports no undo/redo", "[HistoryCApi]") {
    HistoryRoot ROOT_; const char* ROOT = ROOT_.c();
    auto res = okResult(scrivi_history_open(ROOT));
    REQUIRE(res.getString("sessionID").rfind("ses_", 0) == 0);
    REQUIRE_FALSE(res.getBool("canUndo"));
    REQUIRE_FALSE(res.getBool("canRedo"));
    scrivi_free(scrivi_history_close(ROOT));
}

TEST_CASE("C ABI: record then undo then redo round-trips", "[HistoryCApi]") {
    HistoryRoot ROOT_; const char* ROOT = ROOT_.c();
    scrivi_free(scrivi_history_open(ROOT));

    // Record two typing events on one scene.
    {
        auto r = okResult(scrivi_history_record_event(
            ROOT, "scene_a", "Hello", recordParams("typing", 0, 5).c_str()));
        REQUIRE(r.getString("eventID").rfind("evt_", 0) == 0);
        REQUIRE_FALSE(r.getBool("noOp"));
        REQUIRE(r.getBool("canUndo"));
        REQUIRE_FALSE(r.getBool("canRedo"));
    }
    scrivi_free(scrivi_history_record_event(
        ROOT, "scene_a", "Hello world", recordParams("typing", 5, 11).c_str()));

    // Undo → back to "Hello", cursor at 5.
    {
        auto r = okResult(scrivi_history_undo(ROOT));
        REQUIRE(r.getBool("moved"));
        REQUIRE(r.arraySize("changes") == 1);
        JsonDoc c = r.arrayItem("changes", 0);
        REQUIRE(c.getString("sceneID") == "scene_a");
        REQUIRE(c.getString("newText") == "Hello");
        REQUIRE(c.getInt64("cursorAfter") == 5);
        REQUIRE(r.getBool("canUndo"));
        REQUIRE(r.getBool("canRedo"));
        REQUIRE_FALSE(r.getBool("crossedSessionBoundary"));
    }

    // Redo → forward to "Hello world", cursor at 11.
    {
        auto r = okResult(scrivi_history_redo(ROOT));
        REQUIRE(r.getBool("moved"));
        JsonDoc c = r.arrayItem("changes", 0);
        REQUIRE(c.getString("newText") == "Hello world");
        REQUIRE(c.getInt64("cursorAfter") == 11);
        REQUIRE_FALSE(r.getBool("canRedo"));
    }

    scrivi_free(scrivi_history_close(ROOT));
}

TEST_CASE("C ABI: recording identical text reports noOp", "[HistoryCApi]") {
    HistoryRoot ROOT_; const char* ROOT = ROOT_.c();
    scrivi_free(scrivi_history_open(ROOT));
    // Scene starts empty; recording empty text is a no-op.
    auto r = okResult(scrivi_history_record_event(
        ROOT, "scene_a", "", recordParams("typing", 0, 0).c_str()));
    REQUIRE(r.getBool("noOp"));
    REQUIRE_FALSE(r.getBool("canUndo"));
    scrivi_free(scrivi_history_close(ROOT));
}

TEST_CASE("C ABI: undo stops at a barrier with a notice", "[HistoryCApi]") {
    HistoryRoot ROOT_; const char* ROOT = ROOT_.c();
    scrivi_free(scrivi_history_open(ROOT));
    scrivi_free(scrivi_history_record_event(
        ROOT, "scene_a", "before", recordParams("typing", 0, 6).c_str()));

    JsonDoc bp;
    bp.setString("barrierKind", "sceneMerge");
    bp.setString("note", "Can't undo past a scene merge");
    scrivi_free(scrivi_history_record_barrier(ROOT, bp.dump().c_str()));

    scrivi_free(scrivi_history_record_event(
        ROOT, "scene_a", "before after", recordParams("typing", 6, 12).c_str()));

    // First undo removes the text after the barrier.
    scrivi_free(scrivi_history_undo(ROOT));

    // Second undo hits the barrier — no move, notice returned.
    auto r = okResult(scrivi_history_undo(ROOT));
    REQUIRE_FALSE(r.getBool("moved"));
    REQUIRE(r.contains("stoppedAtBarrier"));
    JsonDoc b = r.getSubDoc("stoppedAtBarrier");
    REQUIRE(b.getString("kind") == "sceneMerge");
    REQUIRE(b.getString("note") == "Can't undo past a scene merge");

    scrivi_free(scrivi_history_close(ROOT));
}

TEST_CASE("C ABI: seeded scene baseline — undo stops at pre-existing text", "[HistoryCApi]") {
    HistoryRoot ROOT_; const char* ROOT = ROOT_.c();
    scrivi_free(scrivi_history_open(ROOT));

    // Seed the scene's pre-existing text, then append a sentence.
    scrivi_free(scrivi_history_seed_scene(ROOT, "scene_15", "Existing paragraph."));
    scrivi_free(scrivi_history_record_event(
        ROOT, "scene_15", "Existing paragraph. The quick brown fox.",
        recordParams("typing", 19, 40).c_str()));

    // Undo the appended sentence → back to the pre-existing text (not empty).
    {
        auto r = okResult(scrivi_history_undo(ROOT));
        REQUIRE(r.getBool("moved"));
        REQUIRE(r.arrayItem("changes", 0).getString("newText") == "Existing paragraph.");
    }
    // Undo again → at the history floor; a barrier notice, text untouched.
    {
        auto r = okResult(scrivi_history_undo(ROOT));
        REQUIRE_FALSE(r.getBool("moved"));
        REQUIRE(r.contains("stoppedAtBarrier"));
        REQUIRE(r.getSubDoc("stoppedAtBarrier").getString("kind") == "historyStart");
    }
    scrivi_free(scrivi_history_close(ROOT));
}

TEST_CASE("C ABI: calling before open returns an error envelope", "[HistoryCApi]") {
    HistoryRoot ROOT_; const char* ROOT = ROOT_.c();
    // Ensure closed first.
    scrivi_free(scrivi_history_close(ROOT));
    auto env = envelope(scrivi_history_undo(ROOT));
    REQUIRE_FALSE(env.getBool("ok"));
    JsonDoc err = env.getSubDoc("error");
    REQUIRE(err.getString("message").find("history not open") != std::string::npos);
}

TEST_CASE("C ABI: close reports whether a history was open", "[HistoryCApi]") {
    HistoryRoot ROOT_; const char* ROOT = ROOT_.c();
    scrivi_free(scrivi_history_open(ROOT));
    {
        auto r = okResult(scrivi_history_close(ROOT));
        REQUIRE(r.getBool("closed"));
    }
    {
        auto r = okResult(scrivi_history_close(ROOT));   // already closed
        REQUIRE_FALSE(r.getBool("closed"));
    }
}

TEST_CASE("C ABI: history persists across close/re-open (relaunch)", "[HistoryCApi]") {
    HistoryRoot ROOT_; const char* ROOT = ROOT_.c();

    // --- Session 1: seed + two events, then close (writes log + checkpoint) ---
    scrivi_free(scrivi_history_open(ROOT));
    scrivi_free(scrivi_history_seed_scene(ROOT, "scene_a", "Base."));
    scrivi_free(scrivi_history_record_event(
        ROOT, "scene_a", "Base. One.", recordParams("typing", 5, 10).c_str()));
    scrivi_free(scrivi_history_record_event(
        ROOT, "scene_a", "Base. One. Two.", recordParams("typing", 10, 15).c_str()));
    scrivi_free(scrivi_history_close(ROOT));

    // --- Session 2: re-open the SAME root — the log is replayed ---
    {
        auto r = okResult(scrivi_history_open(ROOT));
        REQUIRE(r.getBool("loaded"));        // an existing history was read
        REQUIRE(r.getBool("canUndo"));       // the tree came back
        REQUIRE_FALSE(r.getBool("canRedo"));
    }
    // Undo walks back through yesterday's events; crossing the session boundary
    // is flagged (session 1 nodes differ from the session 2 open).
    {
        auto r = okResult(scrivi_history_undo(ROOT));
        REQUIRE(r.getBool("moved"));
        REQUIRE(r.arrayItem("changes", 0).getString("newText") == "Base. One.");
        REQUIRE(r.getBool("crossedSessionBoundary"));
    }
    {
        auto r = okResult(scrivi_history_undo(ROOT));
        REQUIRE(r.arrayItem("changes", 0).getString("newText") == "Base.");
    }
    // Floor preserved — one more undo stops at the history start, text intact.
    {
        auto r = okResult(scrivi_history_undo(ROOT));
        REQUIRE_FALSE(r.getBool("moved"));
        REQUIRE(r.getSubDoc("stoppedAtBarrier").getString("kind") == "historyStart");
    }
    scrivi_free(scrivi_history_close(ROOT));
}

TEST_CASE("C ABI: external scene edit produces an externalChange barrier", "[HistoryCApi]") {
    HistoryRoot ROOT_; const char* ROOT = ROOT_.c();

    // Session 1: seed a scene, record an edit, close (persists head hash of "AB").
    scrivi_free(scrivi_history_open(ROOT));
    scrivi_free(scrivi_history_seed_scene(ROOT, "scene_a", "A"));
    scrivi_free(scrivi_history_record_event(
        ROOT, "scene_a", "AB", recordParams("typing", 1, 2).c_str()));
    scrivi_free(scrivi_history_close(ROOT));

    // Session 2: re-open, then validate the scene against a DIFFERENT on-disk
    // text (as if edited outside Scrivi) → externalChange barrier.
    scrivi_free(scrivi_history_open(ROOT));
    {
        auto r = okResult(scrivi_history_validate_scene(ROOT, "scene_a", "AB-edited-elsewhere"));
        REQUIRE(r.getBool("externalChange"));
    }
    // Validating the matching text does NOT re-trigger.
    {
        auto r = okResult(scrivi_history_validate_scene(ROOT, "scene_a", "AB-edited-elsewhere"));
        REQUIRE_FALSE(r.getBool("externalChange"));
    }
    // Undo now hits the externalChange barrier (the manuscript text is untouched).
    {
        auto r = okResult(scrivi_history_undo(ROOT));
        REQUIRE_FALSE(r.getBool("moved"));
        REQUIRE(r.getSubDoc("stoppedAtBarrier").getString("kind") == "externalChange");
    }
    scrivi_free(scrivi_history_close(ROOT));
}

TEST_CASE("C ABI: torn final log line is truncated, tree survives", "[HistoryCApi]") {
    HistoryRoot ROOT_; const char* ROOT = ROOT_.c();

    scrivi_free(scrivi_history_open(ROOT));
    scrivi_free(scrivi_history_seed_scene(ROOT, "scene_a", ""));
    scrivi_free(scrivi_history_record_event(
        ROOT, "scene_a", "kept.", recordParams("typing", 0, 5).c_str()));
    scrivi_free(scrivi_history_record_event(
        ROOT, "scene_a", "kept. torn", recordParams("typing", 5, 10).c_str()));
    scrivi_free(scrivi_history_close(ROOT));

    // Simulate a crash mid-append: corrupt the final line of the log.
    namespace fs = std::filesystem;
    fs::path logFile = fs::path(ROOT_.path) / "history" / "log-000001.jsonl";
    REQUIRE(fs::exists(logFile));
    {
        std::string content;
        { std::ifstream in(logFile, std::ios::binary); std::ostringstream ss; ss << in.rdbuf(); content = ss.str(); }
        content += "{\"rec\":\"event\",\"seq\":99,\"eventID\":\"evt_tor";  // truncated JSON, no newline
        std::ofstream out(logFile, std::ios::binary | std::ios::trunc);
        out << content;
    }

    // Re-open: the torn line is dropped; the last intact event ("kept. torn") remains.
    scrivi_free(scrivi_history_open(ROOT));
    {
        auto r = okResult(scrivi_history_undo(ROOT));
        REQUIRE(r.getBool("moved"));
        REQUIRE(r.arrayItem("changes", 0).getString("newText") == "kept.");
    }
    scrivi_free(scrivi_history_close(ROOT));
}

TEST_CASE("C ABI: select_branch re-primaries a fork and survives relaunch (SP-055 D4)",
          "[HistoryCApi][branch]") {
    HistoryRoot ROOT_; const char* ROOT = ROOT_.c();

    // --- Session 1: build a fork, select the abandoned branch, close ---
    scrivi_free(scrivi_history_open(ROOT));
    scrivi_free(scrivi_history_seed_scene(ROOT, "scene_a", "A"));
    std::string evtB;
    {
        auto r = okResult(scrivi_history_record_event(
            ROOT, "scene_a", "AB", recordParams("typing", 1, 2).c_str()));
        evtB = r.getString("eventID");            // the "AB" branch
    }
    // Undo back to the seed floor "A" (evtB's parent — the fork-to-be).
    scrivi_free(scrivi_history_undo(ROOT));
    // Type a competing branch "AD" → forks; AD becomes primary.
    {
        auto r = okResult(scrivi_history_record_event(
            ROOT, "scene_a", "AD", recordParams("typing", 1, 2).c_str()));
        REQUIRE(r.getBool("createdBranch"));
    }
    // Undo lands back on the fork node; forkAhead lists both branches.
    std::string forkNodeID;
    {
        auto r = okResult(scrivi_history_undo(ROOT));
        REQUIRE(r.getSubDoc("forkAhead").getString("nodeID").size() > 0);
        forkNodeID = r.getSubDoc("forkAhead").getString("nodeID");
        REQUIRE(r.getSubDoc("forkAhead").arraySize("children") == 2);
    }
    // Re-select the abandoned "AB" branch — it becomes primary.
    {
        auto r = okResult(scrivi_history_select_branch(ROOT, forkNodeID.c_str(), evtB.c_str()));
        REQUIRE(r.getBool("ok"));
        REQUIRE(r.getBool("canRedo"));
    }
    scrivi_free(scrivi_history_close(ROOT));

    // --- Session 2: reopen; the SAVED primary (AB) must survive, not snap to AD ---
    scrivi_free(scrivi_history_open(ROOT));
    {
        // We are at the fork; redo must follow the re-selected AB branch.
        auto r = okResult(scrivi_history_redo(ROOT));
        REQUIRE(r.getBool("moved"));
        REQUIRE(r.arrayItem("changes", 0).getString("newText") == "AB");
    }
    scrivi_free(scrivi_history_close(ROOT));
}

TEST_CASE("C ABI: branch-aware eviction persists — purged branch does not resurrect (SP-055 §4.1)",
          "[HistoryCApi][branch]") {
    HistoryRoot ROOT_; const char* ROOT = ROOT_.c();

    // --- Session 1: fork off the floor, then force capacity eviction ---
    scrivi_free(scrivi_history_open(ROOT));
    // Cap at 1 event node so extending the surviving branch evicts at the root.
    {
        JsonDoc s;
        s.setInt("capacityEvents", 1);
        auto r = okResult(scrivi_history_set_settings(ROOT, s.dump().c_str()));
        REQUIRE(r.getBool("updated"));
    }
    scrivi_free(scrivi_history_seed_scene(ROOT, "scene_a", "A"));
    // Branch 1 "AB" off the floor, then undo back to the floor.
    scrivi_free(scrivi_history_record_event(
        ROOT, "scene_a", "AB", recordParams("typing", 1, 2).c_str()));
    scrivi_free(scrivi_history_undo(ROOT));
    // Branch 2 "AC" forks (becomes primary); the root now has two children, so
    // eventCount is 2 > capacity 1 → eviction runs on THIS record: it purges the
    // non-primary "AB" branch and promotes the "AC" child to the new root floor.
    {
        auto r = okResult(scrivi_history_record_event(
            ROOT, "scene_a", "AC", recordParams("typing", 1, 2).c_str()));
        REQUIRE(r.getBool("createdBranch"));
        REQUIRE(r.getInt("evictedCount") > 0);
    }
    scrivi_free(scrivi_history_close(ROOT));

    // --- Session 2: reopen; the purged "AB" branch must NOT come back ---
    scrivi_free(scrivi_history_open(ROOT));
    {
        // Current head is the surviving branch tip "AC"; one undo hits the floor.
        auto r = okResult(scrivi_history_undo(ROOT));
        REQUIRE_FALSE(r.getBool("moved"));
        REQUIRE(r.getSubDoc("stoppedAtBarrier").getString("kind") == "historyStart");
        // No forkAhead: the abandoned "AB" branch was evicted, so the promoted
        // root ("AC") is the floor with no children. If AB had resurrected, the
        // floor would be the original "A" root with a two-child fork.
        REQUIRE(r.getSubDoc("forkAhead").getString("nodeID").empty());
    }
    scrivi_free(scrivi_history_close(ROOT));
}

TEST_CASE("C ABI: user-confirmed purge removes a branch and it does not resurrect (SP-055 T-0212)",
          "[HistoryCApi][branch][stale]") {
    HistoryRoot ROOT_; const char* ROOT = ROOT_.c();

    // --- Session 1: build a fork (AB abandoned, AD primary), purge AB ---
    scrivi_free(scrivi_history_open(ROOT));
    scrivi_free(scrivi_history_seed_scene(ROOT, "scene_a", "A"));
    std::string evtB, forkNodeID;
    {
        auto r = okResult(scrivi_history_record_event(
            ROOT, "scene_a", "AB", recordParams("typing", 1, 2).c_str()));
        evtB = r.getString("eventID");
    }
    scrivi_free(scrivi_history_undo(ROOT));                 // back at the floor (fork-to-be)
    scrivi_free(scrivi_history_record_event(
        ROOT, "scene_a", "AD", recordParams("typing", 1, 2).c_str()));   // AD forks, primary
    {
        auto r = okResult(scrivi_history_undo(ROOT));       // land on the fork
        forkNodeID = r.getSubDoc("forkAhead").getString("nodeID");
        REQUIRE(r.getSubDoc("forkAhead").arraySize("children") == 2);
    }

    // list_stale_branches returns a well-formed envelope. With fresh timestamps
    // nothing is stale yet, but the shape (staleBranchDays + branches array) must
    // be present. (Age-threshold logic is covered by the unit tests, which control
    // timestamps; the ABI stamps events with the real clock.)
    {
        auto r = okResult(scrivi_history_list_stale_branches(ROOT));
        REQUIRE(r.getInt("staleBranchDays") >= 0);
        REQUIRE(r.arraySize("branches") == 0);
    }

    // Purge the abandoned AB branch by its eventID (the user-confirmed action).
    {
        auto r = okResult(scrivi_history_purge_branch(ROOT, evtB.c_str()));
        REQUIRE(r.getBool("ok"));
        REQUIRE(r.getInt("purgedCount") == 1);
    }
    // AD (on the root→current path) is rejected; nothing removed.
    {
        // The fork collapsed to a single child after the purge; redo follows AD.
        auto r = okResult(scrivi_history_redo(ROOT));
        REQUIRE(r.arrayItem("changes", 0).getString("newText") == "AD");
    }
    scrivi_free(scrivi_history_close(ROOT));

    // --- Session 2: reopen; the purged AB branch must NOT come back ---
    scrivi_free(scrivi_history_open(ROOT));
    {
        // At AD; one undo reaches the floor. If AB had resurrected the floor would
        // still be a two-child fork (forkAhead present); it must not be.
        auto r = okResult(scrivi_history_undo(ROOT));
        REQUIRE(r.getBool("moved"));
        REQUIRE(r.getSubDoc("forkAhead").getString("nodeID").empty());
        // And re-selecting the purged branch fails (it is gone).
        auto sel = okResult(scrivi_history_select_branch(ROOT, forkNodeID.c_str(), evtB.c_str()));
        REQUIRE_FALSE(sel.getBool("ok"));
    }
    scrivi_free(scrivi_history_close(ROOT));
}
