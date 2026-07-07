#include <catch2/catch_test_macros.hpp>

#include "scrivi/scrivi.h"
#include "util/Json.hpp"

#include <string>

// Round-trips the scrivi_history_* C ABI through its JSON envelopes (T-0202).
// The history engine is in-memory and keyed by an arbitrary projectRootPath
// string, so these tests need no on-disk project.

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

const char* ROOT = "/tmp/scrivi-history-capi-test.scrivi";

} // namespace

TEST_CASE("C ABI: open mints a session and reports no undo/redo", "[HistoryCApi]") {
    auto res = okResult(scrivi_history_open(ROOT));
    REQUIRE(res.getString("sessionID").rfind("ses_", 0) == 0);
    REQUIRE_FALSE(res.getBool("canUndo"));
    REQUIRE_FALSE(res.getBool("canRedo"));
    scrivi_free(scrivi_history_close(ROOT));
}

TEST_CASE("C ABI: record then undo then redo round-trips", "[HistoryCApi]") {
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
    scrivi_free(scrivi_history_open(ROOT));
    // Scene starts empty; recording empty text is a no-op.
    auto r = okResult(scrivi_history_record_event(
        ROOT, "scene_a", "", recordParams("typing", 0, 0).c_str()));
    REQUIRE(r.getBool("noOp"));
    REQUIRE_FALSE(r.getBool("canUndo"));
    scrivi_free(scrivi_history_close(ROOT));
}

TEST_CASE("C ABI: undo stops at a barrier with a notice", "[HistoryCApi]") {
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
    // Ensure closed first.
    scrivi_free(scrivi_history_close(ROOT));
    auto env = envelope(scrivi_history_undo(ROOT));
    REQUIRE_FALSE(env.getBool("ok"));
    JsonDoc err = env.getSubDoc("error");
    REQUIRE(err.getString("message").find("history not open") != std::string::npos);
}

TEST_CASE("C ABI: close reports whether a history was open", "[HistoryCApi]") {
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
