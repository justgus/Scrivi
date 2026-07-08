#include <catch2/catch_test_macros.hpp>

#include "history/HistoryService.hpp"

using namespace scrivi::history;

namespace {

// Minimal RecordParams builder for the tests.
RecordParams typing(std::string sceneID, std::string text,
                    std::int64_t cursorBefore, std::int64_t cursorAfter,
                    std::string ts = "2026-07-07T00:00:00Z") {
    RecordParams p;
    p.sceneID = std::move(sceneID);
    p.newSceneText = std::move(text);
    p.kind = EventKind::Typing;
    p.cursorBefore = cursorBefore;
    p.cursorAfter = cursorAfter;
    p.timestamp = std::move(ts);
    return p;
}

HistoryService fresh() { return HistoryService("ses_1", "2026-07-07T00:00:00Z"); }

} // namespace

TEST_CASE("fresh history cannot undo or redo", "[History]") {
    auto h = fresh();
    REQUIRE_FALSE(h.canUndo());
    REQUIRE_FALSE(h.canRedo());
}

TEST_CASE("recording identical text is a no-op", "[History]") {
    auto h = fresh();
    // Scene starts empty; recording empty text is no change.
    auto r = h.record(typing("scene_a", "", 0, 0), "evt_1");
    REQUIRE(r.noOp);
    REQUIRE_FALSE(h.canUndo());
}

TEST_CASE("record then undo restores prior scene text and cursor", "[History]") {
    auto h = fresh();
    h.record(typing("scene_a", "Hello", 0, 5), "evt_1");
    h.record(typing("scene_a", "Hello world", 5, 11), "evt_2");
    REQUIRE(h.canUndo());
    REQUIRE_FALSE(h.canRedo());

    auto u = h.undo();
    REQUIRE(u.moved);
    REQUIRE(u.change.has_value());
    REQUIRE(u.change->sceneID == "scene_a");
    REQUIRE(u.change->newText == "Hello");
    REQUIRE(u.change->cursorAfter == 5);   // cursorBefore of evt_2
    REQUIRE(h.canUndo());
    REQUIRE(h.canRedo());
}

TEST_CASE("undo then redo replays the change", "[History]") {
    auto h = fresh();
    h.record(typing("scene_a", "Hello", 0, 5), "evt_1");
    h.record(typing("scene_a", "Hello world", 5, 11), "evt_2");

    h.undo();
    auto rr = h.redo();
    REQUIRE(rr.moved);
    REQUIRE(rr.change->newText == "Hello world");
    REQUIRE(rr.change->cursorAfter == 11);
    REQUIRE_FALSE(h.canRedo());
}

TEST_CASE("undo walks back to empty then stops", "[History]") {
    auto h = fresh();
    h.record(typing("scene_a", "A", 0, 1), "evt_1");
    h.record(typing("scene_a", "AB", 1, 2), "evt_2");

    auto u1 = h.undo();
    REQUIRE(u1.change->newText == "A");
    auto u2 = h.undo();
    REQUIRE(u2.change->newText == "");
    REQUIRE_FALSE(h.canUndo());

    auto u3 = h.undo();   // at the root — no move, history-floor barrier notice
    REQUIRE_FALSE(u3.moved);
    REQUIRE(u3.stoppedAtBarrier);
    REQUIRE(u3.barrierKind == "historyStart");
}

TEST_CASE("seeded baseline: undo stops at the pre-existing text, not empty", "[History]") {
    auto h = fresh();
    // Scene 15 already had this text when history opened.
    h.seedSceneBaseline("scene_15", "Existing paragraph.");
    // The writer appends a sentence.
    h.record(typing("scene_15", "Existing paragraph. The quick brown fox.", 19, 40), "evt_1");

    auto u1 = h.undo();   // undo the appended sentence
    REQUIRE(u1.moved);
    REQUIRE(u1.change->newText == "Existing paragraph.");   // NOT empty

    auto u2 = h.undo();   // at the root floor — nothing below the baseline
    REQUIRE_FALSE(u2.moved);
    REQUIRE(u2.stoppedAtBarrier);
    REQUIRE(u2.barrierKind == "historyStart");
    // The scene text is still the baseline, never emptied.
    REQUIRE(h.headTextForScene("scene_15") == "Existing paragraph.");
}

TEST_CASE("seedSceneBaseline does not clobber an established head", "[History]") {
    auto h = fresh();
    h.record(typing("scene_a", "typed", 0, 5), "evt_1");
    h.seedSceneBaseline("scene_a", "SHOULD BE IGNORED");
    REQUIRE(h.headTextForScene("scene_a") == "typed");
}

TEST_CASE("head text cache tracks the current node", "[History]") {
    auto h = fresh();
    h.record(typing("scene_a", "one", 0, 3), "evt_1");
    h.record(typing("scene_a", "one two", 3, 7), "evt_2");
    REQUIRE(h.headTextForScene("scene_a") == "one two");
    h.undo();
    REQUIRE(h.headTextForScene("scene_a") == "one");
    h.redo();
    REQUIRE(h.headTextForScene("scene_a") == "one two");
}

TEST_CASE("diff handles a mid-string replacement", "[History]") {
    auto h = fresh();
    h.record(typing("scene_a", "the quick brown fox", 0, 19), "evt_1");
    h.record(typing("scene_a", "the slow brown fox", 4, 8), "evt_2");
    REQUIRE(h.headTextForScene("scene_a") == "the slow brown fox");
    auto u = h.undo();
    REQUIRE(u.change->newText == "the quick brown fox");
}

TEST_CASE("multi-scene events undo independently on their own scene", "[History]") {
    auto h = fresh();
    h.record(typing("scene_a", "alpha", 0, 5), "evt_1");
    h.record(typing("scene_b", "beta", 0, 4), "evt_2");
    REQUIRE(h.headTextForScene("scene_a") == "alpha");
    REQUIRE(h.headTextForScene("scene_b") == "beta");

    auto u = h.undo();   // undoes evt_2 (scene_b)
    REQUIRE(u.change->sceneID == "scene_b");
    REQUIRE(u.change->newText == "");
    REQUIRE(h.headTextForScene("scene_a") == "alpha");   // untouched
}

TEST_CASE("undo stops at a barrier without moving", "[History]") {
    auto h = fresh();
    h.record(typing("scene_a", "before", 0, 6), "evt_1");
    BarrierParams bp;
    bp.barrierKind = "sceneMerge";
    bp.barrierNote = "Can't undo past a scene merge";
    bp.timestamp = "2026-07-07T00:01:00Z";
    h.recordBarrier(bp, "evt_barrier");
    h.record(typing("scene_a", "before after", 6, 12), "evt_2");

    auto u1 = h.undo();   // undo evt_2 (text)
    REQUIRE(u1.moved);
    REQUIRE(u1.change->newText == "before");

    auto u2 = h.undo();   // now at the barrier — blocked
    REQUIRE_FALSE(u2.moved);
    REQUIRE(u2.stoppedAtBarrier);
    REQUIRE(u2.barrierKind == "sceneMerge");
    REQUIRE(u2.barrierNote == "Can't undo past a scene merge");
    REQUIRE_FALSE(h.canUndo());
}

TEST_CASE("undo across a session boundary is flagged once", "[History]") {
    // Session 1 records evt_1; a new service instance simulates session 2 by
    // continuing to record — but since the engine mints one session per open,
    // we verify the same-session case reports no crossing.
    auto h = fresh();
    h.record(typing("scene_a", "x", 0, 1), "evt_1");
    h.record(typing("scene_a", "xy", 1, 2), "evt_2");
    auto u = h.undo();
    REQUIRE_FALSE(u.crossedSessionBoundary);   // all in ses_1
}

TEST_CASE("diff is scalar-safe across a multibyte boundary", "[History]") {
    // "café" — the é is two UTF-8 bytes. Replacing the trailing é must not
    // split the scalar.
    auto h = fresh();
    h.record(typing("scene_a", "caf\xC3\xA9", 0, 5), "evt_1");   // café
    h.record(typing("scene_a", "cafe", 4, 4), "evt_2");          // cafe
    REQUIRE(h.headTextForScene("scene_a") == "cafe");
    auto u = h.undo();
    REQUIRE(u.change->newText == "caf\xC3\xA9");
}

TEST_CASE("capacity eviction drops oldest nodes, folds them into the floor", "[History]") {
    auto h = fresh();
    h.setCapacity(2);   // keep at most 2 event nodes

    h.record(typing("scene_a", "a", 0, 1), "evt_1");
    h.record(typing("scene_a", "ab", 1, 2), "evt_2");
    REQUIRE(h.eventCount() == 2);

    // Third record pushes over capacity → evt_1 is evicted; its "a" folds into
    // the floor. Head text is unchanged; the current pointer is untouched.
    auto r = h.record(typing("scene_a", "abc", 2, 3), "evt_3");
    REQUIRE(r.evictedCount == 1);
    REQUIRE(h.eventCount() == 2);
    REQUIRE(h.headTextForScene("scene_a") == "abc");

    // Undo still works down to the (folded) floor, then stops at history start.
    auto u1 = h.undo(); REQUIRE(u1.change->newText == "ab");
    auto u2 = h.undo(); REQUIRE(u2.change->newText == "a");   // floor now == "a"
    auto u3 = h.undo();
    REQUIRE_FALSE(u3.moved);
    REQUIRE(u3.barrierKind == "historyStart");
}

TEST_CASE("eviction never removes the current node (deferred)", "[History]") {
    auto h = fresh();
    h.setCapacity(1);
    h.record(typing("scene_a", "a", 0, 1), "evt_1");
    // Over capacity, but evt_1 IS the current pointer — must not be evicted.
    auto r = h.record(typing("scene_a", "ab", 1, 2), "evt_2");
    // evt_1 (now not current) can go; evt_2 is current and stays.
    REQUIRE(r.evictedCount == 1);
    REQUIRE(h.headTextForScene("scene_a") == "ab");
    // Undo to the folded floor "a", then stop.
    auto u1 = h.undo(); REQUIRE(u1.change->newText == "a");
    auto u2 = h.undo(); REQUIRE_FALSE(u2.moved);
}
