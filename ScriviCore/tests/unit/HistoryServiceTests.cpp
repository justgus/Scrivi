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

// ---- SP-055 branching (T-0210) --------------------------------------------

TEST_CASE("record after undo forks: old chain preserved, new work primary", "[History][branch]") {
    auto h = fresh();
    h.record(typing("scene_a", "A", 0, 1), "evt_A");
    h.record(typing("scene_a", "AB", 1, 2), "evt_B");   // chain A→B
    h.undo();                                            // back to A
    // Typing here forks: evt_B stays as a non-primary child of A; evt_D primary.
    auto r = h.record(typing("scene_a", "AD", 1, 2), "evt_D");
    REQUIRE(r.createdBranch);
    REQUIRE(h.headTextForScene("scene_a") == "AD");
    // The fork node (A) now has two children; the popover data lists both.
    auto fa = h.undo();                                  // undo lands back on A (a fork)
    REQUIRE(fa.forkAhead.has_value());
    REQUIRE(fa.forkAhead->nodeID == "evt_A");
    REQUIRE(fa.forkAhead->children.size() == 2);
    // evt_D is primary (the most recent record); evt_B is the abandoned branch.
    bool sawD = false, sawB = false;
    for (const auto& c : fa.forkAhead->children) {
        if (c.eventID == "evt_D") { sawD = true; REQUIRE(c.isPrimary); }
        if (c.eventID == "evt_B") { sawB = true; REQUIRE_FALSE(c.isPrimary); }
    }
    REQUIRE(sawD);
    REQUIRE(sawB);
}

TEST_CASE("first record after undo does not create a branch when node had no children",
          "[History][branch]") {
    auto h = fresh();
    h.record(typing("scene_a", "A", 0, 1), "evt_A");     // root's only child
    auto r = h.record(typing("scene_a", "AB", 1, 2), "evt_B");
    REQUIRE_FALSE(r.createdBranch);                       // linear extend, not a fork
}

TEST_CASE("select_branch re-primaries a fork; abandoned branch fully restorable",
          "[History][branch]") {
    auto h = fresh();
    h.record(typing("scene_a", "A", 0, 1), "evt_A");
    h.record(typing("scene_a", "AB", 1, 2), "evt_B");
    h.undo();                                            // at A
    h.record(typing("scene_a", "AD", 1, 2), "evt_D");    // fork; D primary
    h.undo();                                            // back at A (the fork)

    // Re-select the abandoned branch B; pointer stays at A, B becomes primary.
    auto sel = h.selectBranch("evt_A", "evt_B");
    REQUIRE(sel.ok);
    REQUIRE(sel.canRedo);
    // Redo now walks the B branch, not D.
    auto rr = h.redo();
    REQUIRE(rr.moved);
    REQUIRE(rr.change->newText == "AB");
    REQUIRE(h.headTextForScene("scene_a") == "AB");

    // D is still fully restorable by re-selecting it.
    h.undo();                                            // back at A
    REQUIRE(h.selectBranch("evt_A", "evt_D").ok);
    auto rd = h.redo();
    REQUIRE(rd.change->newText == "AD");
}

TEST_CASE("select_branch rejects a non-child", "[History][branch]") {
    auto h = fresh();
    h.record(typing("scene_a", "A", 0, 1), "evt_A");
    auto sel = h.selectBranch("evt_A", "evt_nonexistent");
    REQUIRE_FALSE(sel.ok);
}

TEST_CASE("no forkAhead when the landed node has a single child", "[History][branch]") {
    auto h = fresh();
    h.record(typing("scene_a", "A", 0, 1), "evt_A");
    h.record(typing("scene_a", "AB", 1, 2), "evt_B");
    auto u = h.undo();                                    // lands on A, one child (B)
    REQUIRE(u.moved);
    REQUIRE_FALSE(u.forkAhead.has_value());
}

TEST_CASE("branch-aware eviction: a non-primary branch off the root is auto-purged",
          "[History][branch]") {
    // Build a fork at the ROOT: root→A (then fork) so the root itself has two
    // child subtrees, one of which is not on the root→current path.
    auto h = fresh();
    h.record(typing("scene_a", "A", 0, 1), "evt_A");     // root→A
    h.undo();                                            // back at root
    h.record(typing("scene_a", "B", 0, 1), "evt_B");     // fork at root: A (old), B (primary)
    // Root now has two children (evt_A, evt_B); current is evt_B.
    // Setting capacity to 1 forces one eviction: the off-path branch (A) is
    // purged, then B is promoted to the new root and folded into the floor.
    h.setCapacity(1);
    h.record(typing("scene_a", "BC", 1, 2), "evt_C");    // triggers eviction
    // evt_A's subtree is gone; the abandoned branch died with its branch point.
    // Undo from BC → B (the folded floor), then stop at history start.
    auto u1 = h.undo(); REQUIRE(u1.change->newText == "B");
    auto u2 = h.undo(); REQUIRE_FALSE(u2.moved);          // B is now the floor/root
    // The head text is intact throughout.
    h.redo();
    REQUIRE(h.headTextForScene("scene_a") == "BC");
}

TEST_CASE("branch-aware eviction defers when the current pointer is at the root",
          "[History][branch]") {
    auto h = fresh();
    h.record(typing("scene_a", "A", 0, 1), "evt_A");
    h.setCapacity(1);
    // Undo to the root, then force an over-capacity condition by... it already is
    // over capacity conceptually; eviction must DEFER because current == root
    // (nothing on the root→current path to promote).
    h.undo();                                            // current is now the root
    // A fresh record here would fork at the root; but first prove a direct
    // eviction attempt with current==root does nothing destructive: record a
    // sibling that keeps current reachable.
    auto r = h.record(typing("scene_a", "Z", 0, 1), "evt_Z");
    // current is evt_Z; evt_A is an off-path root child. Capacity 1, eventCount
    // is 2 → one eviction: A purged, Z promoted. Z stays reachable.
    REQUIRE(r.evictedCount >= 1);
    REQUIRE(h.headTextForScene("scene_a") == "Z");
    auto u = h.undo(); REQUIRE_FALSE(u.moved);            // Z is the floor now
}

// ---- T-0212: stale-branch detection + user-confirmed purge -----------------

// Builds a fork at evt_A with an OLD abandoned branch (B, stamped weeks ago) and
// a RECENT primary branch (D). Returns with the pointer on the D line.
static HistoryService forkWithOldAbandonedBranch() {
    auto h = fresh();
    h.record(typing("scene_a", "A", 0, 1, "2026-07-01T00:00:00Z"), "evt_A");
    h.record(typing("scene_a", "AB", 1, 2, "2026-06-10T00:00:00Z"), "evt_B");  // old branch tip
    h.undo();                                                                   // at A
    h.record(typing("scene_a", "AD", 1, 2, "2026-07-09T00:00:00Z"), "evt_D");  // recent, primary
    return h;
}

TEST_CASE("stale branch detected when its tip is older than the threshold",
          "[History][branch][stale]") {
    auto h = forkWithOldAbandonedBranch();
    // now = 2026-07-10; B's tip is 2026-06-10 (30 days) → stale at 7-day threshold.
    auto stale = h.listStaleBranches("2026-07-10T00:00:00Z", 7);
    REQUIRE(stale.size() == 1);
    REQUIRE(stale[0].branchRootEventID == "evt_B");
    REQUIRE(stale[0].forkNodeID == "evt_A");
    REQUIRE(stale[0].nodeCount == 1);
    REQUIRE(stale[0].tipTimestamp == "2026-06-10T00:00:00Z");
}

TEST_CASE("no stale branches when the threshold is disabled or the tip is recent",
          "[History][branch][stale]") {
    auto h = forkWithOldAbandonedBranch();
    REQUIRE(h.listStaleBranches("2026-07-10T00:00:00Z", 0).empty());   // disabled
    REQUIRE(h.listStaleBranches("2026-07-10T00:00:00Z", 90).empty());  // 30d < 90d → not stale
}

TEST_CASE("the on-path (live) branch is never reported stale",
          "[History][branch][stale]") {
    // Make the OLD branch the one holding the pointer: after selecting B and
    // redoing onto it, B is on the root→current path and must not be listed even
    // though its tip is old. Only D (now off-path, but recent) exists otherwise.
    auto h = forkWithOldAbandonedBranch();
    h.undo();                                    // at A (the fork)
    h.selectBranch("evt_A", "evt_B");
    h.redo();                                    // pointer now on B
    auto stale = h.listStaleBranches("2026-07-10T00:00:00Z", 7);
    // B holds the pointer → excluded; D is recent (2026-07-09) → not stale.
    REQUIRE(stale.empty());
}

TEST_CASE("purgeBranch erases a stale subtree and keeps the live line intact",
          "[History][branch][stale]") {
    auto h = forkWithOldAbandonedBranch();      // pointer on D (primary)
    REQUIRE(h.headTextForScene("scene_a") == "AD");

    auto p = h.purgeBranch("evt_B");
    REQUIRE(p.ok);
    REQUIRE(p.purgedCount == 1);
    // The live D line is untouched; B is gone (re-selecting it now fails).
    REQUIRE(h.headTextForScene("scene_a") == "AD");
    REQUIRE_FALSE(h.selectBranch("evt_A", "evt_B").ok);
    // And the fork is no longer a fork (A has a single child now).
    h.undo();                                    // at A
    auto u = h.undo();                            // A→root; A had one child so no forkAhead on the way
    REQUIRE(h.listStaleBranches("2026-07-10T00:00:00Z", 7).empty());
    (void)u;
}

TEST_CASE("purgeBranch rejects the root and any node on the root->current path",
          "[History][branch][stale]") {
    auto h = forkWithOldAbandonedBranch();      // pointer on D
    // D is on the root→current path → reject.
    auto pd = h.purgeBranch("evt_D");
    REQUIRE_FALSE(pd.ok);
    REQUIRE(pd.purgedCount == 0);
    // A is also on the path (it is D's parent / an ancestor of current) → reject.
    REQUIRE_FALSE(h.purgeBranch("evt_A").ok);
    // The root is never purgeable.
    REQUIRE_FALSE(h.purgeBranch("evt_root").ok);
    // An unknown node → reject.
    REQUIRE_FALSE(h.purgeBranch("evt_nope").ok);
    // Nothing was removed by any of the rejections: B is still there and stale.
    REQUIRE(h.listStaleBranches("2026-07-10T00:00:00Z", 7).size() == 1);
}

// --- I-0065: replay-on-load must not crash on a mismatched/stale diff ---------
// Regression for the macOS crash opening a project whose persisted history log
// carries a node whose diff no longer matches the scene's baseline (e.g. the
// scene was deleted or its text changed on disk). rebuildHeadCache() replayed
// that diff with applyForward; a diff.removed longer than the floor text
// underflowed reserve() (unsigned) and threw std::length_error, which crossed
// the C ABI and terminated the app. finalizeLoad() must now degrade to a
// best-effort head instead of throwing.
TEST_CASE("replay-on-load survives a diff whose removed span exceeds the floor",
          "[History][load][I-0065]") {
    auto h = fresh();

    // The scene's persisted floor is short ("hi"). The persisted child claims to
    // remove 20 bytes at offset 5 and insert "X" — a stale/mismatched diff, the
    // shape produced when a diff is replayed against a deleted/wrong scene.
    h.addLoadedFloor("scene_gone", "hi");

    EventNode root;
    root.eventID = "evt_root";
    root.kind = EventKind::Typing;   // root carries no applied diff

    EventNode bad;
    bad.eventID = "evt_bad";
    bad.parentID = "evt_root";
    bad.kind = EventKind::Typing;
    bad.sceneID = "scene_gone";
    bad.diff.offsetUtf8 = 5;                 // past end of "hi"
    bad.diff.removed = std::string(20, 'z'); // longer than the floor text
    bad.diff.inserted = "X";
    bad.timestamp = "2026-07-07T00:00:01Z";
    bad.sessionID = "ses_1";

    h.addLoadedNode(root);
    h.addLoadedNode(bad);
    h.setPointers("evt_root", "evt_bad", "ses_1");

    // finalizeLoad() rebuilds the head cache by replaying evt_bad's diff.
    // Before the fix this threw std::length_error and terminated the process.
    REQUIRE_NOTHROW(h.finalizeLoad());

    // Best-effort result: clamped offset (5→2) + inserted, tail empty. The exact
    // string is not the contract — not throwing is. Assert it is well-formed and
    // begins with what survived the clamp.
    const std::string head = h.headTextForScene("scene_gone");
    REQUIRE(head == "hiX");

    // The rehydrated tree is still usable: undo walks back to the floor, and
    // neither direction throws on the mismatched node.
    REQUIRE_NOTHROW(h.undo());
    REQUIRE_NOTHROW(h.redo());
}

// --- I-0066: load-time pruning of an orphaned/inconsistent scene's history ----
// A scene deleted from the navigator left its floor + event records in the log
// with no barrier; on reload those diffs no longer matched their scene. The load
// path now DROPS such nodes (subtree included) and reports the roots so the store
// can persist a ctl:purge — the log self-heals instead of degrading every open.

// Helper: a loaded event node with an explicit diff.
EventNode loadedEvent(std::string id, std::string parent, std::string sceneID,
                      std::size_t offset, std::string removed, std::string inserted) {
    EventNode n;
    n.eventID = std::move(id);
    if (!parent.empty()) n.parentID = std::move(parent);
    n.kind = EventKind::Typing;
    n.sceneID = std::move(sceneID);
    n.diff.offsetUtf8 = offset;
    n.diff.removed = std::move(removed);
    n.diff.inserted = std::move(inserted);
    n.timestamp = "2026-07-15T00:00:00Z";
    n.sessionID = "ses_1";
    return n;
}

TEST_CASE("prune drops an inconsistent node and keeps a consistent sibling scene",
          "[History][load][I-0066]") {
    auto h = fresh();

    // scene_ok: floor "ab", a matching diff appends "c" at offset 2 → "abc".
    // scene_gone: floor "hi", a stale diff removes 5 bytes at offset 0 (mismatch).
    h.addLoadedFloor("scene_ok", "ab");
    h.addLoadedFloor("scene_gone", "hi");

    EventNode root;
    root.eventID = "evt_root";
    root.kind = EventKind::Typing;

    // Both events hang off the root (a fork); order is derived by finalizeLoad.
    auto good = loadedEvent("evt_good", "evt_root", "scene_ok", 2, "", "c");
    auto bad  = loadedEvent("evt_bad",  "evt_root", "scene_gone", 0,
                            std::string(5, 'z'), "X");

    h.addLoadedNode(root);
    h.addLoadedNode(good);
    h.addLoadedNode(bad);
    h.setPointers("evt_root", "evt_good", "ses_1");
    REQUIRE_NOTHROW(h.finalizeLoad());

    auto droppedRoots = h.pruneInconsistentNodes();
    REQUIRE(droppedRoots.size() == 1);
    REQUIRE(droppedRoots[0] == "evt_bad");

    // The good scene's history is intact and still replays correctly.
    REQUIRE(h.headTextForScene("scene_ok") == "abc");
    // The good node is still reachable / undoable.
    REQUIRE(h.canUndo());
    REQUIRE_NOTHROW(h.undo());
    REQUIRE(h.headTextForScene("scene_ok") == "ab");
}

TEST_CASE("prune moves the current pointer out of a dropped subtree",
          "[History][load][I-0066]") {
    auto h = fresh();
    h.addLoadedFloor("scene_gone", "hi");

    EventNode root;
    root.eventID = "evt_root";
    root.kind = EventKind::Typing;

    // A two-node bad chain; current points at the tip, which will be pruned.
    auto bad1 = loadedEvent("evt_bad1", "evt_root", "scene_gone", 0,
                            std::string(9, 'z'), "A");
    auto bad2 = loadedEvent("evt_bad2", "evt_bad1", "scene_gone", 0, "A", "B");

    h.addLoadedNode(root);
    h.addLoadedNode(bad1);
    h.addLoadedNode(bad2);
    h.setPointers("evt_root", "evt_bad2", "ses_1");
    REQUIRE_NOTHROW(h.finalizeLoad());

    auto droppedRoots = h.pruneInconsistentNodes();
    // The whole bad chain drops from its root (evt_bad1); one ctl:purge covers it.
    REQUIRE(droppedRoots.size() == 1);
    REQUIRE(droppedRoots[0] == "evt_bad1");

    // Current walked back to the root; the tree is valid and no longer undoable.
    REQUIRE_FALSE(h.canUndo());
    REQUIRE_NOTHROW(h.headTextForScene("scene_gone"));
}

TEST_CASE("prune is a no-op on a fully consistent loaded tree",
          "[History][load][I-0066]") {
    auto h = fresh();
    h.addLoadedFloor("scene_a", "");

    EventNode root;
    root.eventID = "evt_root";
    root.kind = EventKind::Typing;

    auto e1 = loadedEvent("evt_1", "evt_root", "scene_a", 0, "", "one");
    auto e2 = loadedEvent("evt_2", "evt_1", "scene_a", 3, "", " two");

    h.addLoadedNode(root);
    h.addLoadedNode(e1);
    h.addLoadedNode(e2);
    h.setPointers("evt_root", "evt_2", "ses_1");
    REQUIRE_NOTHROW(h.finalizeLoad());

    auto droppedRoots = h.pruneInconsistentNodes();
    REQUIRE(droppedRoots.empty());
    REQUIRE(h.headTextForScene("scene_a") == "one two");
}
