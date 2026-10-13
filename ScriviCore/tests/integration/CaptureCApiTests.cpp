// SP-177 / T-0604 (EP-026 S1) — WHEN AN UNDO STEP ENDS, tested THROUGH `scrivi_capture_*` (a facade test cannot see the
// boundary). The rules were Apple's Swift (`HistoryCapture.swift`); these pin them in the core. The parity fixture
// (Apple's own answers, recorded before the Swift was removed) is replayed below.

#include <catch2/catch_test_macros.hpp>

#include "scrivi/scrivi.h"
#include "util/Json.hpp"

#include <atomic>
#include <chrono>
#include <filesystem>
#include <fstream>
#include <map>
#include <string>

using scrivi::util::JsonDoc;
using scrivi::util::parseJson;

namespace {

JsonDoc ok(const char* raw) {
    REQUIRE(raw != nullptr);
    auto parsed = parseJson(raw);
    scrivi_free(raw);
    REQUIRE(parsed.ok());
    JsonDoc env = std::move(parsed.value());
    INFO("envelope: " << env.dump());
    REQUIRE(env.getBool("ok"));
    return env.getSubDoc("result");
}

struct Root {
    std::string path;
    Root() {
        static std::atomic<int> counter{0};
        path = (std::filesystem::temp_directory_path() /
                ("scrivi-capture-capi-" + std::to_string(counter.fetch_add(1)) + "-" +
                 std::to_string(reinterpret_cast<std::uintptr_t>(this)) + ".scrivi"))
                   .string();
        std::error_code ec;
        std::filesystem::remove_all(path, ec);   // a crashed earlier run can leave this exact path behind with a log in it
        ok(scrivi_history_open(c()));
    }
    ~Root() {
        scrivi_free(scrivi_history_close(c()));
        std::error_code ec;
        std::filesystem::remove_all(path, ec);
    }
    const char* c() const { return path.c_str(); }
};

// Type `text` one character at a time into `scene` (which starts as `before`), `stepMs` apart, from `t`.
// ASCII only, so a character is one UTF-16 unit. Returns how many events were committed on the way.
int type(const Root& r, const char* scene, std::string before, const std::string& text, long long& t, int stepMs = 200) {
    int commits = 0;
    std::string now = before;
    for (char ch : text) {
        const std::string prev = now;
        now.push_back(ch);
        t += stepMs;
        if (ok(scrivi_capture_note_edit(r.c(), scene, now.c_str(), int(now.size()), prev.c_str(), t)).getBool("committed")) {
            ++commits;
        }
    }
    return commits;
}

int nodeCount(const Root& r) {
    const JsonDoc tree = ok(scrivi_history_get_tree(r.c(), "{}"));
    return int(tree.arraySize("nodes")) - 1;   // the root anchor is not an event
}

JsonDoc change(const JsonDoc& step, std::size_t i = 0) { return step.arrayItem("changes", i); }

}   // namespace

TEST_CASE("capture ABI: a continuously typed sentence is ONE undo step (the T-0396 reference case)", "[capture-abi]") {
    Root r;
    long long t = 0;
    const std::string sentence = "Now is the winter of our discontent made glorious summer by this son of york";
    CHECK(type(r, "s1", "", sentence, t) == 0);   // no terminator, no pause: nothing commits while typing
    JsonDoc st = ok(scrivi_capture_state(r.c()));
    CHECK(st.getBool("hasPending"));
    CHECK(st.getBool("canUndo"));                  // undo is possible while an edit is pending
    CHECK(st.getInt64("idleDeadlineMs") == t + 45'000);
    CHECK_FALSE(ok(scrivi_capture_before_save(r.c(), t + 1'000)).getBool("committed"));   // an autosave never seals it
    const JsonDoc u = ok(scrivi_capture_undo(r.c()));                                       // undo commits, then walks back
    CHECK(nodeCount(r) == 1);
    CHECK(change(u).getString("newText") == "");
    CHECK_FALSE(ok(scrivi_capture_undo(r.c())).getBool("moved"));
}

TEST_CASE("capture ABI: the boundaries — a terminator, 45 s idle, a caret move, another scene", "[capture-abi]") {
    Root r;
    long long t = 0;
    CHECK(type(r, "s1", "", "One.", t) == 1);                          // '.' commits
    CHECK(type(r, "s1", "One.", " Two", t) == 0);
    t += 45'000;                                                          // the writer walked away…
    CHECK(ok(scrivi_capture_idle(r.c(), t - 1)).getBool("committed") == false);   // …a timer a moment early does nothing
    CHECK(ok(scrivi_capture_idle(r.c(), t + 200)).getBool("committed"));          // the deadline: committed
    CHECK(type(r, "s1", "One. Two", " three", t) == 0);
    t += 46'000;
    CHECK(type(r, "s1", "One. Two three", "x", t) == 1);               // a 46 s gap commits BEFORE the new keystroke
    CHECK(ok(scrivi_capture_caret_moved(r.c(), "s1", 0, 0)).getBool("committed"));   // a caret move in the scene
    CHECK(type(r, "s1", "One. Two threex", "y", t) == 0);
    CHECK(ok(scrivi_capture_caret_moved(r.c(), "s2", 0, 0)).getBool("committed"));   // into another scene: hard
    CHECK(nodeCount(r) == 5);
}

TEST_CASE("capture ABI: whitespace waits for real text on a soft trigger, never on a hard one", "[capture-abi]") {
    Root r;
    long long t = 0;
    type(r, "s1", "", "Hi.", t);                                                       // committed at '.'
    CHECK(type(r, "s1", "Hi.", "  ", t) == 0);
    CHECK_FALSE(ok(scrivi_capture_caret_moved(r.c(), "s1", 0, 0)).getBool("committed"));   // soft: only whitespace
    CHECK(ok(scrivi_capture_state(r.c())).getBool("hasPending"));
    CHECK_FALSE(ok(scrivi_capture_caret_moved(r.c(), "s1", 1, 0)).getBool("committed"));   // edit in flight: no move
    CHECK(ok(scrivi_capture_caret_moved(r.c(), "s2", 0, 0)).getBool("committed"));         // another scene: hard
    CHECK(nodeCount(r) == 2);
}

TEST_CASE("capture ABI: paste and cut are their own steps, labelled; a cut into a buffer carries its slot", "[capture-abi]") {
    Root r;
    long long t = 0;
    type(r, "s1", "", "abc", t);
    ok(scrivi_capture_begin(r.c(), "paste", ""));                    // commits "abc" first
    ok(scrivi_capture_note_edit(r.c(), "s1", "abcPASTED", 9, "abc", t += 10));
    CHECK(ok(scrivi_capture_flush(r.c(), "paste", "paste")).getBool("committed"));
    ok(scrivi_capture_begin(r.c(), "cut", "3"));
    ok(scrivi_capture_note_edit(r.c(), "s1", "abc", 3, "abcPASTED", t += 10));
    CHECK(ok(scrivi_capture_flush(r.c(), "cut", "cut")).getBool("committed"));
    const JsonDoc tree = ok(scrivi_history_get_tree(r.c(), "{}"));
    std::string kinds, buffers;
    for (std::size_t i = 0; i < tree.arraySize("nodes"); ++i) {
        const JsonDoc n = tree.arrayItem("nodes", i);
        kinds += n.getString("kind", "") + ",";
        buffers += n.getString("bufferID", "") + ",";
    }
    CHECK(kinds.find("typing") != std::string::npos);
    CHECK(kinds.find("paste") != std::string::npos);
    CHECK(kinds.find("cut") != std::string::npos);
    CHECK(buffers.find('3') != std::string::npos);
}

TEST_CASE("capture ABI: a grouped edit across scenes is ONE undo step; carets are UTF-16 in and out", "[capture-abi]") {
    Root r;
    long long t = 0;
    // 👋 is 2 UTF-16 units and 4 UTF-8 bytes: the caret after "👋 a" is 4 units, 6 bytes.
    ok(scrivi_capture_note_edit(r.c(), "s1", "👋 a", 4, "", t += 10));
    ok(scrivi_capture_flush(r.c(), "flush", "typing"));
    const char* edits = R"([{"sceneID":"s1","textBefore":"👋 a","textAfter":"👋 ","caret":3},
                            {"sceneID":"s2","textBefore":"b c","textAfter":"c","caret":0}])";
    CHECK(ok(scrivi_capture_grouped_edit(r.c(), edits, "delete")).getBool("committed"));
    const JsonDoc u = ok(scrivi_capture_undo(r.c()));
    CHECK(u.arraySize("changes") == 2);                                // one step restores both scenes
    const JsonDoc u2 = ok(scrivi_capture_undo(r.c()));
    CHECK(change(u2).getString("newText") == "");
    const JsonDoc redo = ok(scrivi_capture_redo(r.c()));
    CHECK(change(redo).getString("newText") == "👋 a");
    CHECK(change(redo).getInt64("cursorAfter") == 6);                  // the engine's UTF-8 bytes…
    CHECK(change(redo).getInt64("cursorAfterUtf16") == 4);             // …and the platform's UTF-16 units
}

TEST_CASE("capture ABI: a barrier commits pending typing and takes the pending scene; close commits too", "[capture-abi]") {
    {
        Root r;
        long long t = 0;
        type(r, "s7", "", "pending", t);
        const JsonDoc b = ok(scrivi_capture_record_barrier(r.c(), R"({"barrierKind":"sceneSplit","note":"x"})"));
        CHECK_FALSE(b.getBool("hasPending"));
        const JsonDoc tree = ok(scrivi_history_get_tree(r.c(), "{}"));
        bool attributed = false;
        for (std::size_t i = 0; i < tree.arraySize("nodes"); ++i) {
            const JsonDoc n = tree.arrayItem("nodes", i);
            attributed |= n.getString("barrierKind", "") == "sceneSplit" && n.getString("sceneID", "") == "s7";
        }
        CHECK(attributed);
    }
    Root r;
    long long t = 0;
    type(r, "s1", "", "last words", t);
    scrivi_free(scrivi_history_close(r.c()));   // commits "last words" before closing
    ok(scrivi_history_open(r.c()));
    CHECK(nodeCount(r) == 1);
}

// [I-0287] — a split or merge REWRITES scene text, then records a barrier. The barrier must carry what the op left in
// each scene it rewrote, or the next edit diffs against the PRE-op text and undoing it puts that text back: the merged-in
// scene's words gone (its file already deleted), or a split's tail duplicated into the head. Proven at this ABI first
// (Linux `undo_smoke`, SP-178); Apple and Linux both record their barriers here.
TEST_CASE("capture ABI: a barrier's sceneTexts are what the next edit's undo returns to — merge, split, reopen, eviction",
          "[capture-abi]") {
    const std::string merged = "Alpha. One.\n\nBravo.";

    SECTION("merge: undoing the next edit keeps the merged-in text, also after a reopen") {
        Root r;
        ok(scrivi_capture_note_edit(r.c(), "S", "Alpha. One.", 11, "Alpha.", 1000));   // '.' → one step
        ok(scrivi_capture_record_barrier(
            r.c(), R"({"barrierKind":"sceneMerge","note":"m","sceneID":"S","sceneTexts":{"S":"Alpha. One.\n\nBravo."}})"));
        ok(scrivi_capture_note_edit(r.c(), "S", (merged + " Two.").c_str(), 24, merged.c_str(), 2000));
        CHECK(change(ok(scrivi_capture_undo(r.c()))).getString("newText") == merged);
        CHECK(ok(scrivi_capture_undo(r.c())).getSubDoc("stoppedAtBarrier").getString("kind") == "sceneMerge");   // stops
        CHECK(change(ok(scrivi_capture_redo(r.c()))).getString("newText") == merged + " Two.");

        scrivi_free(scrivi_history_close(r.c()));   // the barrier's texts survive the log replay
        ok(scrivi_history_open(r.c()));
        CHECK(change(ok(scrivi_capture_undo(r.c()))).getString("newText") == merged);
    }

    SECTION("split: the head's undo restores the HEAD, and the new tail scene is undoable from its own text") {
        Root r;
        ok(scrivi_capture_note_edit(r.c(), "H", "One. Two.", 9, "One.", 1000));
        ok(scrivi_capture_record_barrier(
            r.c(), R"({"barrierKind":"sceneSplit","note":"s","sceneID":"H","sceneTexts":{"H":"One.","T":" Two."}})"));
        ok(scrivi_capture_note_edit(r.c(), "H", "One. Uno.", 9, "One.", 2000));
        CHECK(change(ok(scrivi_capture_undo(r.c()))).getString("newText") == "One.");
        ok(scrivi_capture_note_edit(r.c(), "T", " Two. Dos.", 10, " Two.", 3000));   // forks off; the tail's first edit
        CHECK(change(ok(scrivi_capture_undo(r.c()))).getString("newText") == " Two.");
        // The tail entered history at the barrier: it has a floor, so an outside edit to it is still detected.
        scrivi_free(scrivi_history_close(r.c()));
        ok(scrivi_history_open(r.c()));
        CHECK(ok(scrivi_history_validate_scene(r.c(), "T", "changed outside")).getBool("externalChange"));
    }

    SECTION("eviction: a barrier promoted to the root becomes the floor") {
        Root r;
        ok(scrivi_history_set_settings(r.c(), R"({"capacityEvents":2})"));
        ok(scrivi_capture_note_edit(r.c(), "S", "Alpha. One.", 11, "Alpha.", 1000));
        ok(scrivi_capture_record_barrier(
            r.c(), R"({"barrierKind":"sceneMerge","note":"m","sceneID":"S","sceneTexts":{"S":"Alpha. One.\n\nBravo."}})"));
        ok(scrivi_capture_note_edit(r.c(), "S", (merged + " Two.").c_str(), 24, merged.c_str(), 2000));
        ok(scrivi_capture_note_edit(r.c(), "S", (merged + " Two. Three.").c_str(), 31, (merged + " Two.").c_str(), 3000));
        for (int pass = 0; pass < 2; ++pass) {   // in session, then from the log
            CHECK(change(ok(scrivi_capture_undo(r.c()))).getString("newText") == merged + " Two.");
            CHECK(change(ok(scrivi_capture_undo(r.c()))).getString("newText") == merged);
            ok(scrivi_capture_redo(r.c()));
            ok(scrivi_capture_redo(r.c()));
            scrivi_free(scrivi_history_close(r.c()));
            ok(scrivi_history_open(r.c()));
        }
    }
}

// [I-0288] — the `externalChange` repair (`HistoryStore::validateSceneHead`) used to RE-SEED the scene's floor from disk and
// persist that floor; the next open replayed every diff since the root onto it — including the ones computed against the
// OLD text — and undo restored a mix of the two ("Zulu. one. Alpha two."). Now the disk text rides on the barrier
// ([I-0287]'s `sceneTexts`). Measured: session 1 edits, the file is rewritten outside Scrivi, session 2 edits on top,
// session 3 must still undo to the outside text. `legacyLog` rewrites the log into the PRE-fix shape (a barrier without
// texts, then a `floor` record) before session 3: a project repaired by an older build must open correctly too.
namespace {
void runOutsideEditSessions(Root& r, bool legacyLog) {
    // Session 1: two steps on the original text, the bytes reported as written (the save path, I-0104).
    ok(scrivi_capture_note_edit(r.c(), "S", "Alpha one.", 10, "Alpha", 1000));
    ok(scrivi_capture_note_edit(r.c(), "S", "Alpha one. Alpha two.", 21, "Alpha one.", 2000));
    ok(scrivi_capture_flush(r.c(), "flush", "typing"));
    scrivi_free(scrivi_history_note_scene_persisted(r.c(), "S", "Alpha one. Alpha two."));
    scrivi_free(scrivi_history_close(r.c()));

    // Outside Scrivi: the file is rewritten — shorter, and nothing like the old text.
    const std::string outside = "Zulu.";
    ok(scrivi_history_open(r.c()));
    CHECK(ok(scrivi_history_validate_scene(r.c(), "S", outside.c_str())).getBool("externalChange"));

    // Session 2: one step on top of the outside text.
    ok(scrivi_capture_note_edit(r.c(), "S", "Zulu. Yankee.", 13, outside.c_str(), 3000));
    ok(scrivi_capture_flush(r.c(), "flush", "typing"));
    CHECK(change(ok(scrivi_capture_undo(r.c()))).getString("newText") == outside);
    CHECK(ok(scrivi_capture_undo(r.c())).getSubDoc("stoppedAtBarrier").getString("kind") == "externalChange");
    CHECK(change(ok(scrivi_capture_redo(r.c()))).getString("newText") == "Zulu. Yankee.");
    scrivi_free(scrivi_history_note_scene_persisted(r.c(), "S", "Zulu. Yankee."));
    scrivi_free(scrivi_history_close(r.c()));

    if (legacyLog) {
        namespace fs = std::filesystem;
        const std::string texts = R"(,"sceneTexts":{"S":"Zulu."})";
        int rewritten = 0;
        for (const auto& e : fs::directory_iterator(fs::path(r.path) / "history")) {
            if (e.path().extension() != ".jsonl") continue;
            std::ifstream in(e.path());
            std::string line, out;
            while (std::getline(in, line)) {
                const auto at = line.find(texts);
                if (line.find(R"("barrierKind":"externalChange")") != std::string::npos && at != std::string::npos) {
                    line.erase(at, texts.size());
                    out += line + "\n";
                    out += R"({"rec":"floor","seq":0,"sceneID":"S","reason":"externalChange","text":"Zulu.","sha256":"","timestamp":""})";
                    out += "\n";
                    ++rewritten;
                } else {
                    out += line + "\n";
                }
            }
            in.close();
            std::ofstream(e.path(), std::ios::trunc) << out;
        }
        REQUIRE(rewritten == 1);
    }

    // Session 3: the log replays. Disk matches the head (no second warning), and undo still returns to the outside text,
    // then stops at the barrier — never the pre-change text, never a mix of the two.
    ok(scrivi_history_open(r.c()));
    CHECK_FALSE(ok(scrivi_history_validate_scene(r.c(), "S", "Zulu. Yankee.")).getBool("externalChange"));
    CHECK(change(ok(scrivi_capture_undo(r.c()))).getString("newText") == outside);
    CHECK(ok(scrivi_capture_undo(r.c())).getSubDoc("stoppedAtBarrier").getString("kind") == "externalChange");
    CHECK(change(ok(scrivi_capture_redo(r.c()))).getString("newText") == "Zulu. Yankee.");
}
}   // namespace

TEST_CASE("capture ABI: after an outside edit, the next edits undo to the OUTSIDE text — in session and after a reopen",
          "[capture-abi][I-0288]") {
    SECTION("a log written by this build") {
        Root r;
        runOutsideEditSessions(r, false);
    }
    SECTION("a log the repair wrote BEFORE the fix (barrier, then a floor record)") {
        Root r;
        runOutsideEditSessions(r, true);
    }
}

// [I-0293] — a STRUCTURED paste (a reversible structural node, EP-029 AC6) rewrites the target scene too: head + first piece.
// It carried no texts, so a target already edited this session kept its PRE-paste head: the next edit's undo put the
// pre-paste text back (the tail now also in the last created scene), and the paste's own undo was then refused by
// `fragment_uncut_paste`. Proven through `scrivi_*` on a real project first ([SP-180] Q3). Now the node carries
// `sceneTexts` (after the op) and `sceneTextsBefore` (each existing scene before it): redo and replay read the first,
// undo the second, and a scene the node brings into history takes its BEFORE text as its floor.
TEST_CASE("capture ABI: a structured paste's texts are what undo returns to on either side of it — seeded or not, reopened",
          "[capture-abi][I-0293]") {
    const std::string before = "Alpha. Bravo.";
    const std::string after = "Alpha. one";   // a fragment [none:"one", scene:"two", tail:"three"] pasted at byte 7
    const char* paste =
        R"({"barrierKind":"structuredPaste","note":"p","sceneID":"S","structuralPayload":{"op":"structuredPaste"},)"
        R"("sceneTexts":{"S":"Alpha. one","N1":"two","N2":"threeBravo."},"sceneTextsBefore":{"S":"Alpha. Bravo."}})";

    for (const bool seeded : {true, false}) {
        DYNAMIC_SECTION((seeded ? "the target was edited before the paste" : "the target's first edit is after the paste")) {
            Root r;
            if (seeded) {
                ok(scrivi_capture_note_edit(r.c(), "S", "Alpha.", 6, "", 1000));
                ok(scrivi_capture_note_edit(r.c(), "S", before.c_str(), 13, "Alpha.", 2000));
            }
            ok(scrivi_capture_record_barrier(r.c(), paste));
            ok(scrivi_capture_note_edit(r.c(), "S", (after + ".").c_str(), 11, after.c_str(), 3000));

            auto reopen = [&] { scrivi_free(scrivi_history_close(r.c())); ok(scrivi_history_open(r.c())); };
            auto undoText = [&] { return change(ok(scrivi_capture_undo(r.c()))).getString("newText"); };
            auto crossed = [&](const char* raw) { return ok(raw).getSubDoc("structuralInverse").getString("direction"); };

            CHECK(undoText() == after);                                     // the edit after the paste
            CHECK(crossed(scrivi_capture_undo(r.c())) == "undo");          // the paste (the app uncuts it)
            CHECK(crossed(scrivi_capture_redo(r.c())) == "redo");          // and back (the app re-pastes)
            ok(scrivi_capture_note_edit(r.c(), "S", (after + "!").c_str(), 11, after.c_str(), 4000));
            CHECK(undoText() == after);                                     // after a redo over it

            reopen();                                                       // replayed from the log, AFTER the paste
            ok(scrivi_capture_note_edit(r.c(), "S", (after + "?").c_str(), 11, after.c_str(), 5000));
            CHECK(undoText() == after);

            CHECK(crossed(scrivi_capture_undo(r.c())) == "undo");          // BEFORE the paste: S reads `before` again
            ok(scrivi_capture_note_edit(r.c(), "S", (before + " X.").c_str(), 16, before.c_str(), 6000));
            CHECK(undoText() == before);

            reopen();                                                       // replayed from the log, BEFORE the paste
            ok(scrivi_capture_note_edit(r.c(), "S", (before + " Y.").c_str(), 16, before.c_str(), 7000));
            CHECK(undoText() == before);
        }
    }
}

// [I-0293], end to end on a real project: the calls a platform makes for ⌘V of a cross-scene fragment (Apple's
// `pasteStructuredFragment`), then the writer's next edit, its undo, and the paste's own undo (`fragment_uncut_paste`).
// Before the fix the edit's undo returned the pre-paste text and the uncut was REFUSED.
TEST_CASE("capture ABI: after a real structured paste, the next edit and then the paste undo cleanly", "[capture-abi][I-0293]") {
    namespace fs = std::filesystem;
    const fs::path base = fs::temp_directory_path() /
                          ("scrivi-i0293-" + std::to_string(std::chrono::steady_clock::now().time_since_epoch().count()));
    fs::remove_all(base);
    fs::create_directories(base / "as");
    const std::string root = (base / "p.scrivi").string(), as = (base / "as").string();
    const JsonDoc created = ok(scrivi_create_project(root.c_str(), as.c_str(), "I-0293", "i0293", "identity-001",
                                                     "persona-001", "T"));
    const std::string pid = created.getString("projectID");
    const JsonDoc first = created.getSubDoc("firstScene");
    const std::string S = first.getString("sceneID");
    auto save = [&](const std::string& text) {
        ok(scrivi_save_scene(pid.c_str(), root.c_str(), as.c_str(), S.c_str(), first.getString("metadataPath").c_str(),
                             first.getString("contentPath").c_str(), text.c_str(), 0, 0, 0.0, "identity-001",
                             "persona-001", "T"));
    };
    auto textOf = [&](const std::string& sceneID) {
        const JsonDoc r = ok(scrivi_read_scene_texts(root.c_str(), ("[\"" + sceneID + "\"]").c_str()));
        return r.arrayItem("scenes", 0).getString("markdown");
    };
    ok(scrivi_open_project(root.c_str(), as.c_str(), "identity-001"));
    ok(scrivi_history_open(root.c_str()));

    // The writer types two sentences into the scene (so history has seeded it), and they are saved.
    ok(scrivi_capture_note_edit(root.c_str(), S.c_str(), "Alpha.", 6, "", 1000));
    ok(scrivi_capture_note_edit(root.c_str(), S.c_str(), "Alpha. Bravo.", 13, "Alpha.", 2000));
    save("Alpha. Bravo.");

    // ⌘V of [none:"one", scene:"two", tail:"three"] at "Alpha. |Bravo." — flush, paste, then the structural node with
    // the texts the paste left (read back from disk) and the target's text before it.
    const std::string frag = R"({"schema":"scrivi.fragment.v1","pieces":[{"opensWith":"none","text":"one","partial":"head"},)"
                             R"({"opensWith":"scene","text":"two"},{"opensWith":"scene","text":"three","partial":"tail"}],)"
                             R"("plainText":"one\n\ntwo\n\nthree"})";
    ok(scrivi_capture_flush(root.c_str(), "flush", ""));
    const JsonDoc pasted = ok(scrivi_fragment_paste(root.c_str(), as.c_str(), pid.c_str(), frag.c_str(), S.c_str(), 7,
                                                    "identity-001", "persona-001", "T"));
    const auto made = pasted.getStringArray("createdSceneIDs");
    REQUIRE(made.size() == 2);
    REQUIRE(textOf(S) == "Alpha. one");
    REQUIRE(textOf(made[1]) == "threeBravo.");
    const std::string ids = "[\"" + made[0] + "\",\"" + made[1] + "\"]";
    JsonDoc payload;
    payload.setString("op", "structuredPaste");
    payload.setString("fragmentJSON", frag);
    payload.setString("caretSceneID", S);
    payload.setInt64("caretByte", 7);
    JsonDoc texts, prior, params;
    texts.setString(S, textOf(S));
    for (const std::string& id : made) texts.setString(id, textOf(id));
    prior.setString(S, "Alpha. Bravo.");
    params.setString("barrierKind", "structuredPaste");
    params.setString("note", "Can't undo past a cross-boundary paste");
    params.setSubDoc("structuralPayload", std::move(payload));
    params.setSubDoc("sceneTexts", std::move(texts));
    params.setSubDoc("sceneTextsBefore", std::move(prior));
    ok(scrivi_capture_record_barrier(root.c_str(), params.dump(-1).c_str()));

    // The writer ends the sentence in the target scene; ⌘Z returns it to what the paste left.
    ok(scrivi_capture_note_edit(root.c_str(), S.c_str(), "Alpha. one.", 11, "Alpha. one", 3000));
    save("Alpha. one.");
    const JsonDoc u1 = ok(scrivi_capture_undo(root.c_str()));
    CHECK(change(u1).getString("newText") == "Alpha. one");
    save(change(u1).getString("newText"));

    // ⌘Z again crosses the paste; the app's inverse op is accepted and the scene is whole again.
    CHECK(ok(scrivi_capture_undo(root.c_str())).getSubDoc("structuralInverse").getString("direction") == "undo");
    ok(scrivi_fragment_uncut_paste(root.c_str(), frag.c_str(), S.c_str(), ("{\"sceneIDs\":" + ids + "}").c_str()));
    CHECK(textOf(S) == "Alpha. Bravo.");

    scrivi_free(scrivi_history_close(root.c_str()));
    scrivi_free(scrivi_close_project(root.c_str()));
    std::error_code ec;
    fs::remove_all(base, ec);
}

// [I-0296] — at an open, history compares each scene's file with the last bytes SCRIVI wrote (the save path reports them,
// [I-0104]). A structured paste, and its undo (`fragment_uncut_paste`), are written by the CORE, and nothing reported those
// bytes: the next open took the pasted-into scene for an outside edit, and undo stopped there. Now the two endpoints report
// what they wrote while the project's history is open.
TEST_CASE("capture ABI: a reopen after a structured paste, or its undo, takes no scene for an outside edit",
          "[capture-abi][I-0296]") {
    namespace fs = std::filesystem;
    const fs::path base = fs::temp_directory_path() /
                          ("scrivi-i0296-" + std::to_string(std::chrono::steady_clock::now().time_since_epoch().count()));
    fs::remove_all(base);
    fs::create_directories(base / "as");
    const std::string root = (base / "p.scrivi").string(), as = (base / "as").string();
    const JsonDoc created = ok(scrivi_create_project(root.c_str(), as.c_str(), "I-0296", "i0296", "identity-001",
                                                     "persona-001", "T"));
    const std::string pid = created.getString("projectID");
    const JsonDoc first = created.getSubDoc("firstScene");
    const std::string S = first.getString("sceneID");
    auto textOf = [&](const std::string& sceneID) {
        return ok(scrivi_read_scene_texts(root.c_str(), ("[\"" + sceneID + "\"]").c_str())).arrayItem("scenes", 0).getString("markdown");
    };
    // Reopen as a platform does: close (the checkpoint), open, and validate every scene against its file.
    auto flaggedOnReopen = [&](const std::vector<std::string>& scenes) {
        scrivi_free(scrivi_history_close(root.c_str()));
        ok(scrivi_history_open(root.c_str()));
        std::vector<std::string> flagged;
        for (const std::string& id : scenes) {
            if (ok(scrivi_history_validate_scene(root.c_str(), id.c_str(), textOf(id).c_str())).getBool("externalChange")) {
                flagged.push_back(id == S ? "target" : id);
            }
        }
        return flagged;
    };
    ok(scrivi_open_project(root.c_str(), as.c_str(), "identity-001"));
    ok(scrivi_history_open(root.c_str()));

    // The writer's text, saved — and reported, as the save path does.
    ok(scrivi_capture_note_edit(root.c_str(), S.c_str(), "Alpha. Bravo.", 13, "", 1000));
    ok(scrivi_save_scene(pid.c_str(), root.c_str(), as.c_str(), S.c_str(), first.getString("metadataPath").c_str(),
                         first.getString("contentPath").c_str(), "Alpha. Bravo.", 0, 0, 0.0, "identity-001", "persona-001", "T"));
    ok(scrivi_history_note_scene_persisted(root.c_str(), S.c_str(), "Alpha. Bravo."));
    CHECK(flaggedOnReopen({S}).empty());

    const std::string frag = R"({"schema":"scrivi.fragment.v1","pieces":[{"opensWith":"none","text":"one","partial":"head"},)"
                             R"({"opensWith":"scene","text":"two","partial":"tail"}],"plainText":"one\n\ntwo"})";
    ok(scrivi_capture_flush(root.c_str(), "flush", ""));
    const JsonDoc pasted = ok(scrivi_fragment_paste(root.c_str(), as.c_str(), pid.c_str(), frag.c_str(), S.c_str(), 7,
                                                    "identity-001", "persona-001", "T"));
    const std::string made = pasted.getStringArray("createdSceneIDs").at(0);
    // [I-0297]: the paste hands back what it wrote, byte for byte — the platform records it without re-reading.
    CHECK(pasted.getSubDoc("sceneTexts").getString(S) == textOf(S));
    CHECK(pasted.getSubDoc("sceneTexts").getString(made) == textOf(made));
    JsonDoc texts, prior, payload, params;
    texts.setString(S, textOf(S));
    texts.setString(made, textOf(made));
    prior.setString(S, "Alpha. Bravo.");
    payload.setString("op", "structuredPaste");
    params.setString("barrierKind", "structuredPaste");
    params.setSubDoc("structuralPayload", std::move(payload));
    params.setSubDoc("sceneTexts", std::move(texts));
    params.setSubDoc("sceneTextsBefore", std::move(prior));
    ok(scrivi_capture_record_barrier(root.c_str(), params.dump(-1).c_str()));
    CHECK(flaggedOnReopen({S, made}).empty());   // ⛔ was {"target"}

    // Its undo: the app uncuts the paste (the core writes the target again).
    CHECK(ok(scrivi_capture_undo(root.c_str())).getSubDoc("structuralInverse").getString("direction") == "undo");
    const JsonDoc uncut =
        ok(scrivi_fragment_uncut_paste(root.c_str(), frag.c_str(), S.c_str(), ("{\"sceneIDs\":[\"" + made + "\"]}").c_str()));
    REQUIRE(textOf(S) == "Alpha. Bravo.");
    CHECK(uncut.getSubDoc("sceneTexts").getString(S) == "Alpha. Bravo.");   // [I-0297]
    CHECK(flaggedOnReopen({S}).empty());

    // EP-029's collapsing cut (`fragment_cut`, no longer called by either platform, but in old histories) writes too.
    const std::string again = ok(scrivi_fragment_paste(root.c_str(), as.c_str(), pid.c_str(), frag.c_str(), S.c_str(), 7,
                                                       "identity-001", "persona-001", "T")).getStringArray("createdSceneIDs").at(0);
    REQUIRE(textOf(S) == "Alpha. one");
    const std::string spans = R"({"spans":[{"sceneID":")" + S + R"(","startByte":7,"endByte":10},{"sceneID":")" + again +
                              R"(","startByte":0,"endByte":3}]})";
    ok(scrivi_fragment_cut(root.c_str(), spans.c_str()));
    REQUIRE(textOf(S) == "Alpha. Bravo.");
    CHECK(flaggedOnReopen({S}).empty());

    scrivi_free(scrivi_history_close(root.c_str()));
    scrivi_free(scrivi_close_project(root.c_str()));
    std::error_code ec;
    fs::remove_all(base, ec);
}

TEST_CASE("capture ABI: not open → an error, never a crash", "[capture-abi]") {
    const char* raw = scrivi_capture_state("/nonexistent/never-opened.scrivi");
    auto parsed = parseJson(raw);
    scrivi_free(raw);
    REQUIRE(parsed.ok());
    CHECK_FALSE(parsed.value().getBool("ok"));
}

// ---------------------------------------------------------------------------
// SP-177 Q3 — the PARITY FIXTURE: 203 sessions run through APPLE'S SWIFT (`HistoryCapture` + the text view's rules) and
// through `scrivi_capture_*` side by side (interop `UndoCaptureParityTests`), BEFORE the Swift was removed; every session
// agreed, and Apple's answers were recorded. Replayed here so they stay checked. Each line: the steps (with their times)
// and the history they must leave — the primary spine and the undo walk back from the end.
// ⚠️ Modelled exactly as the generator did (neither is a capture rule): the idle TIMER fires before a step once the
// deadline has passed; after an undo/redo the caret rests in the earliest scene changed (scenes in name order).
// ---------------------------------------------------------------------------
#include <fstream>

namespace {

JsonDoc parse(const std::string& text) {
    auto parsed = parseJson(text);
    REQUIRE(parsed.ok());
    return std::move(parsed.value());
}

std::string str(const JsonDoc& d, std::string_view key) { return d.getString(key, ""); }

// After an applied undo/redo: the caret rests in the earliest scene the step changed.
void applied(const Root& r, const JsonDoc& step) {
    if (!step.getBool("moved") || step.arraySize("changes") == 0) { return; }
    std::size_t earliest = 0;
    for (std::size_t i = 1; i < step.arraySize("changes"); ++i) {
        if (str(step.arrayItem("changes", i), "sceneID") < str(step.arrayItem("changes", earliest), "sceneID")) { earliest = i; }
    }
    const JsonDoc first = step.arrayItem("changes", earliest);
    ok(scrivi_capture_note_applied(r.c(), str(first, "sceneID").c_str(), str(first, "newText").c_str()));
}

void run(const Root& r, const JsonDoc& s) {
    const long long t = s.getInt64("t", 0);
    const JsonDoc st = ok(scrivi_capture_state(r.c()));
    if (st.getInt64("idleDeadlineMs") >= 0 && t >= st.getInt64("idleDeadlineMs")) { ok(scrivi_capture_idle(r.c(), t)); }
    const std::string op = str(s, "op");
    auto edit = [&]() {
        ok(scrivi_capture_note_edit(r.c(), str(s, "scene").c_str(), str(s, "text").c_str(), int(s.getInt("caret", 0)),
                                    str(s, "before").c_str(), t));
    };
    if (op == "edit") { edit(); }
    else if (op == "move") {
        ok(scrivi_capture_caret_moved(r.c(), str(s, "scene").c_str(), s.getBool("editInFlight") ? 1 : 0, 0));
    } else if (op == "paste" || op == "cut") {
        ok(scrivi_capture_begin(r.c(), op.c_str(), str(s, "buffer").c_str()));
        edit();
        ok(scrivi_capture_flush(r.c(), op.c_str(), op.c_str()));
    } else if (op == "save") { ok(scrivi_capture_before_save(r.c(), t)); }
    else if (op == "group") {
        std::string json = "[";
        for (std::size_t i = 0; i < s.arraySize("edits"); ++i) {
            const JsonDoc e = s.arrayItem("edits", i);
            JsonDoc g;
            g.setString("sceneID", str(e, "scene"));
            g.setString("textBefore", str(e, "before"));
            g.setString("textAfter", str(e, "after"));
            g.setInt("caret", int(e.getInt("caret", 0)));
            json += (i ? "," : "") + g.dump(-1);
        }
        json += "]";
        ok(scrivi_capture_grouped_edit(r.c(), json.c_str(), str(s, "kind").c_str()));
    } else if (op == "barrier") {
        JsonDoc p;
        p.setString("barrierKind", str(s, "kind"));
        p.setString("note", "n");
        if (s.contains("scene")) { p.setString("sceneID", str(s, "scene")); }
        ok(scrivi_capture_record_barrier(r.c(), p.dump(-1).c_str()));
    } else if (op == "undo") { applied(r, ok(scrivi_capture_undo(r.c()))); }
    else if (op == "redo") { applied(r, ok(scrivi_capture_redo(r.c()))); }
    else { FAIL("unknown op " << op); }
}

// The history a session left, in the generator's shape.
std::string observe(const Root& r) {
    JsonDoc out;
    const JsonDoc tree = ok(scrivi_history_get_tree(r.c(), R"({"maxNodes":100000})"));
    std::map<std::string, JsonDoc> byID;
    for (std::size_t i = 0; i < tree.arraySize("nodes"); ++i) {
        JsonDoc n = tree.arrayItem("nodes", i);
        std::string key = str(n, "eventID");
        byID.emplace(std::move(key), std::move(n));
    }
    std::string spine = "[";
    std::string id = byID.count(str(tree, "rootID")) ? str(byID.at(str(tree, "rootID")), "primaryChildID") : "";
    for (bool firstNode = true; byID.count(id); firstNode = false) {
        const JsonDoc& n = byID.at(id);
        JsonDoc e;
        e.setString("kind", str(n, "kind"));
        e.setString("scene", str(n, "sceneID"));
        e.setString("buffer", str(n, "bufferID"));
        e.setString("barrier", str(n, "barrierKind"));
        e.setString("preview", str(n, "preview"));
        spine += (firstNode ? "" : ",") + e.dump(-1);
        id = str(n, "primaryChildID");
    }
    spine += "]";
    std::string walk = "[";
    for (int i = 0; i < 400; ++i) {
        const JsonDoc step = ok(scrivi_capture_undo(r.c()));
        applied(r, step);
        JsonDoc w;
        w.setBool("moved", step.getBool("moved"));
        w.setString("barrier", step.contains("stoppedAtBarrier") ? str(step.getSubDoc("stoppedAtBarrier"), "kind") : "");
        std::string changes = "[";
        for (std::size_t k = 0; k < step.arraySize("changes"); ++k) {
            const JsonDoc c = step.arrayItem("changes", k);
            JsonDoc cc;
            cc.setString("scene", str(c, "sceneID"));
            cc.setString("text", str(c, "newText"));
            cc.setInt64("cursor", c.getInt64("cursorAfter", 0));
            changes += (k ? "," : "") + cc.dump(-1);
        }
        changes += "]";
        JsonDoc wc = parse(w.dump(-1).substr(0, w.dump(-1).size() - 1) + ",\"changes\":" + changes + "}");
        walk += (i ? "," : "") + wc.dump(-1);
        if (!step.getBool("moved")) { break; }
    }
    walk += "]";
    return parse("{\"spine\":" + spine + ",\"walk\":" + walk + "}").dump(-1);
}

}   // namespace

TEST_CASE("capture ABI: every session of Apple's parity fixture leaves the same undo history", "[capture-abi]") {
    std::ifstream in(std::string(SCRIVI_TEST_FIXTURES_DIR) + "/history_capture_parity.jsonl");
    REQUIRE(in.good());
    std::string text;
    int sessions = 0, wrong = 0;
    while (std::getline(in, text)) {
        if (text.empty()) { continue; }
        ++sessions;
        const JsonDoc line = parse(text);
        Root r;
        for (std::size_t i = 0; i < line.arraySize("steps"); ++i) { run(r, line.arrayItem("steps", i)); }
        const std::string expected = line.getSubDoc("expected").dump(-1), actual = observe(r);
        if (expected != actual && ++wrong <= 3) {
            INFO("session " << line.getInt("session", -1));
            CHECK(actual == expected);
        }
    }
    CHECK(sessions == 203);
    CHECK(wrong == 0);
}
