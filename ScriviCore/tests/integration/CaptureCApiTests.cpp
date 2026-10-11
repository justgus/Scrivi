// SP-177 / T-0604 (EP-026 S1) — WHEN AN UNDO STEP ENDS, tested THROUGH `scrivi_capture_*` (a facade test cannot see the
// boundary). The rules were Apple's Swift (`HistoryCapture.swift`); these pin them in the core. The parity fixture
// (Apple's own answers, recorded before the Swift was removed) is replayed below.

#include <catch2/catch_test_macros.hpp>

#include "scrivi/scrivi.h"
#include "util/Json.hpp"

#include <atomic>
#include <filesystem>
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
