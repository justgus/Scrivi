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

// Builds record-event params tagged with a copy-buffer slot (Trade T3 cut-into-buffer).
std::string recordParamsWithBuffer(const char* kind, int64_t before, int64_t after,
                                   const char* bufferID) {
    JsonDoc p;
    p.setString("kind", kind);
    p.setInt64("cursorBefore", before);
    p.setInt64("cursorAfter", after);
    p.setString("bufferID", bufferID);
    return p.dump();
}

// Reads and concatenates every history log segment (log-*.jsonl) under `root`
// (empty if none). The log is the ground truth for what got persisted.
std::string readHistoryLog(const std::string& root) {
    namespace fs = std::filesystem;
    const fs::path dir = fs::path(root) / "history";
    std::error_code ec;
    if (!fs::exists(dir, ec)) return {};
    std::string out;
    for (const auto& entry : fs::directory_iterator(dir, ec)) {
        const std::string name = entry.path().filename().string();
        if (name.rfind("log-", 0) == 0 && entry.path().extension() == ".jsonl") {
            std::ifstream in(entry.path());
            std::stringstream ss; ss << in.rdbuf();
            out += ss.str();
        }
    }
    return out;
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

// Forces a state.json checkpoint without closing. There is no public checkpoint
// endpoint; set_settings performs one as a side effect, which is exactly the
// mid-session ordering that exposed the I-0104 barrier loop.
void forceCheckpoint(const char* root) {
    JsonDoc p;
    p.setInt("capacityEvents", 100000);
    p.setInt("staleBranchDays", 30);
    p.setInt("idleRolloverHours", 8);
    scrivi_free(scrivi_history_set_settings(root, p.dump().c_str()));
}

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

TEST_CASE("C ABI: a structural node is stepped across, returning its inverse payload "
          "and surviving relaunch (T-0356)", "[HistoryCApi][T-0356]") {
    HistoryRoot ROOT_; const char* ROOT = ROOT_.c();

    // --- Session 1: record text, then a REVERSIBLE structural node (payload present) ---
    scrivi_free(scrivi_history_open(ROOT));
    scrivi_free(scrivi_history_record_event(
        ROOT, "scene_a", "before", recordParams("typing", 0, 6).c_str()));

    JsonDoc payload;                       // the app's opaque inverse-op descriptor
    payload.setString("op", "paste");
    payload.setString("fragmentJSON", "F");
    payload.setString("caretSceneID", "scene_a");
    JsonDoc bp;
    bp.setString("barrierKind", "structuredCut");
    bp.setString("note", "Can't undo past a cross-boundary cut");
    bp.setSubDoc("structuralPayload", std::move(payload));
    scrivi_free(scrivi_history_record_barrier(ROOT, bp.dump().c_str()));

    // Undo steps ACROSS the structural node: moved, no text change, inverse payload returned.
    {
        auto r = okResult(scrivi_history_undo(ROOT));
        REQUIRE(r.getBool("moved"));
        REQUIRE(r.arraySize("changes") == 0);           // no SceneChange
        REQUIRE_FALSE(r.contains("stoppedAtBarrier"));
        REQUIRE(r.contains("structuralInverse"));
        JsonDoc si = r.getSubDoc("structuralInverse");
        REQUIRE(si.getString("direction") == "undo");
        JsonDoc p = si.getSubDoc("payload");
        REQUIRE(p.getString("op") == "paste");
        REQUIRE(p.getString("caretSceneID") == "scene_a");
    }
    // Redo re-crosses forward with direction=redo.
    {
        auto r = okResult(scrivi_history_redo(ROOT));
        REQUIRE(r.getBool("moved"));
        REQUIRE(r.arraySize("changes") == 0);
        REQUIRE(r.contains("structuralInverse"));
        REQUIRE(r.getSubDoc("structuralInverse").getString("direction") == "redo");
    }
    scrivi_free(scrivi_history_close(ROOT));

    // --- Session 2: reopen; the structural node + payload replay from the log ---
    scrivi_free(scrivi_history_open(ROOT));
    {
        auto r = okResult(scrivi_history_undo(ROOT));    // across the structural node again
        REQUIRE(r.getBool("moved"));
        REQUIRE(r.contains("structuralInverse"));
        JsonDoc si = r.getSubDoc("structuralInverse");
        REQUIRE(si.getString("direction") == "undo");
        REQUIRE(si.getSubDoc("payload").getString("op") == "paste");   // payload survived reload
    }
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

TEST_CASE("C ABI: cut-into-buffer event persists its bufferID tag (Trade T3)", "[HistoryCApi]") {
    HistoryRoot ROOT_; const char* ROOT = ROOT_.c();

    // Session 1: seed, then record a cut-into-buffer event tagged with slot "3".
    scrivi_free(scrivi_history_open(ROOT));
    scrivi_free(scrivi_history_seed_scene(ROOT, "scene_a", "Base. Cut me."));
    {
        auto r = okResult(scrivi_history_record_event(
            ROOT, "scene_a", "Base. ",
            recordParamsWithBuffer("cut", 12, 6, "3").c_str()));
        REQUIRE_FALSE(r.getBool("noOp"));   // a real deletion → a real event
    }
    scrivi_free(scrivi_history_close(ROOT));

    // The tag reached disk: the persisted event line carries bufferID "3".
    {
        const std::string log = readHistoryLog(ROOT);
        REQUIRE(log.find("\"kind\":\"cut\"") != std::string::npos);
        REQUIRE(log.find("\"bufferID\":\"3\"") != std::string::npos);
    }

    // Session 2: reopen — the tagged cut replays and behaves exactly like a plain
    // cut (undo restores the pre-cut text). The tag is provenance, not behavior.
    scrivi_free(scrivi_history_open(ROOT));
    {
        auto r = okResult(scrivi_history_undo(ROOT));
        REQUIRE(r.getBool("moved"));
        REQUIRE(r.arrayItem("changes", 0).getString("newText") == "Base. Cut me.");
    }
    scrivi_free(scrivi_history_close(ROOT));
}

TEST_CASE("C ABI: an ordinary event writes no bufferID field", "[HistoryCApi]") {
    HistoryRoot ROOT_; const char* ROOT = ROOT_.c();

    // A plain typing event must not gain a bufferID key — the on-disk shape is
    // unchanged for the common case (the tag is written only when non-empty).
    scrivi_free(scrivi_history_open(ROOT));
    scrivi_free(scrivi_history_seed_scene(ROOT, "scene_a", "A"));
    scrivi_free(scrivi_history_record_event(
        ROOT, "scene_a", "AB", recordParams("typing", 1, 2).c_str()));
    scrivi_free(scrivi_history_close(ROOT));

    const std::string log = readHistoryLog(ROOT);
    REQUIRE(log.find("\"kind\":\"typing\"") != std::string::npos);
    REQUIRE(log.find("bufferID") == std::string::npos);
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

// --- I-0104: the head hash must describe the bytes on disk, not the replayed head ---
//
// Before the fix, persistState hashed service_->headTextForScene() while
// validateSceneHead hashed the on-disk text. Those are different artifacts
// whenever a save lands after the last recorded event (the autosave path writes
// the editor's text with its own normalization), so a scene edited only inside
// Scrivi re-flagged as externally changed on EVERY open.

TEST_CASE("C ABI: a scene saved normally raises no externalChange on reopen (I-0104)",
          "[HistoryCApi][I-0104]") {
    HistoryRoot ROOT_; const char* ROOT = ROOT_.c();

    // Session 1: type, then save. The save path writes "AB." — one character more
    // than the last recorded event ("AB") — which is exactly the head-vs-disk
    // divergence that used to fire a spurious barrier.
    scrivi_free(scrivi_history_open(ROOT));
    scrivi_free(scrivi_history_seed_scene(ROOT, "scene_a", "A"));
    scrivi_free(scrivi_history_record_event(
        ROOT, "scene_a", "AB", recordParams("typing", 1, 2).c_str()));
    scrivi_free(scrivi_history_note_scene_persisted(ROOT, "scene_a", "AB."));
    scrivi_free(scrivi_history_close(ROOT));

    // Session 2: validating against the very bytes we wrote must be silent.
    scrivi_free(scrivi_history_open(ROOT));
    {
        auto r = okResult(scrivi_history_validate_scene(ROOT, "scene_a", "AB."));
        REQUIRE_FALSE(r.getBool("externalChange"));
    }
    // Repeated opens must stay silent — the original symptom was that the noise
    // recurred indefinitely rather than settling after one warning.
    scrivi_free(scrivi_history_close(ROOT));
    scrivi_free(scrivi_history_open(ROOT));
    {
        auto r = okResult(scrivi_history_validate_scene(ROOT, "scene_a", "AB."));
        REQUIRE_FALSE(r.getBool("externalChange"));
    }
    scrivi_free(scrivi_history_close(ROOT));
}

TEST_CASE("C ABI: a genuine external edit still raises the barrier after a save (I-0104)",
          "[HistoryCApi][I-0104]") {
    HistoryRoot ROOT_; const char* ROOT = ROOT_.c();

    // The other direction — the fix must not quiet real third-party edits.
    scrivi_free(scrivi_history_open(ROOT));
    scrivi_free(scrivi_history_seed_scene(ROOT, "scene_a", "A"));
    scrivi_free(scrivi_history_record_event(
        ROOT, "scene_a", "AB", recordParams("typing", 1, 2).c_str()));
    scrivi_free(scrivi_history_note_scene_persisted(ROOT, "scene_a", "AB."));
    scrivi_free(scrivi_history_close(ROOT));

    // Something else rewrote the file between sessions.
    scrivi_free(scrivi_history_open(ROOT));
    {
        auto r = okResult(scrivi_history_validate_scene(ROOT, "scene_a", "AB. edited by vim"));
        REQUIRE(r.getBool("externalChange"));
    }
    // And undo stops at that barrier, as §6.b requires.
    {
        auto r = okResult(scrivi_history_undo(ROOT));
        REQUIRE_FALSE(r.getBool("moved"));
        REQUIRE(r.getSubDoc("stoppedAtBarrier").getString("kind") == "externalChange");
    }
    scrivi_free(scrivi_history_close(ROOT));
}

// ⚠️ THE case the first I-0104 fix missed — reported in live verify 2026-08-10 after
// three quit→reopen cycles produced three externalChange barriers on a scene the
// writer only ever edited inside Scrivi.
//
// Shape: the save path records a disk hash, but then MORE text is committed to
// history without a following save (T-0396 defers the save-time commit, and the
// closing flush advances the head after the last autosave already cleared isDirty).
// persistState then fell back to hashing the replayed head, so the next open compared
// head-to-disk — the original bug, via a new route. Recording the final text at close
// is what keeps the persisted hash disk-accurate.
TEST_CASE("C ABI: a commit after the last save does not poison the head hash (I-0104)",
          "[HistoryCApi][I-0104]") {
    HistoryRoot ROOT_; const char* ROOT = ROOT_.c();

    scrivi_free(scrivi_history_open(ROOT));
    scrivi_free(scrivi_history_seed_scene(ROOT, "scene_a", ""));
    scrivi_free(scrivi_history_record_event(
        ROOT, "scene_a", "First sentence.", recordParams("typing", 0, 15).c_str()));
    scrivi_free(scrivi_history_note_scene_persisted(ROOT, "scene_a", "First sentence."));

    // More typing committed at close — disk already holds it (autosave wrote the file),
    // and close() must record that same text so the hash still describes disk.
    scrivi_free(scrivi_history_record_event(
        ROOT, "scene_a", "First sentence. Second.", recordParams("typing", 15, 23).c_str()));
    scrivi_free(scrivi_history_note_scene_persisted(ROOT, "scene_a", "First sentence. Second."));
    scrivi_free(scrivi_history_close(ROOT));

    // Three quit→reopen cycles, exactly as reported: none may raise a barrier.
    for (int i = 0; i < 3; ++i) {
        scrivi_free(scrivi_history_open(ROOT));
        auto r = okResult(scrivi_history_validate_scene(ROOT, "scene_a", "First sentence. Second."));
        REQUIRE_FALSE(r.getBool("externalChange"));
        scrivi_free(scrivi_history_close(ROOT));
    }
}

TEST_CASE("C ABI: a scene never saved this session keeps its prior baseline (I-0104)",
          "[HistoryCApi][I-0104]") {
    HistoryRoot ROOT_; const char* ROOT = ROOT_.c();

    // noteScenePersisted is not called for scene_a in session 2, so its session-1
    // disk hash must carry forward rather than being replaced by a head-derived
    // one — otherwise the fallback would reintroduce the original bug.
    scrivi_free(scrivi_history_open(ROOT));
    scrivi_free(scrivi_history_seed_scene(ROOT, "scene_a", "A"));
    scrivi_free(scrivi_history_note_scene_persisted(ROOT, "scene_a", "A-on-disk"));
    scrivi_free(scrivi_history_close(ROOT));

    scrivi_free(scrivi_history_open(ROOT));       // opened, read, never saved
    scrivi_free(scrivi_history_close(ROOT));

    scrivi_free(scrivi_history_open(ROOT));
    {
        auto r = okResult(scrivi_history_validate_scene(ROOT, "scene_a", "A-on-disk"));
        REQUIRE_FALSE(r.getBool("externalChange"));
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

// I-0104 (3rd investigation, 2026-08-10): does a scene whose baseline is ALREADY
// wrong self-heal? validateSceneHead records a barrier and re-seeds, so the SECOND
// open must be silent. The user's project shows the same 14 scenes re-flagging on
// every open, ~14 barriers per launch, which means the repair is not sticking.
TEST_CASE("C ABI: a mismatching scene stops re-flagging after one repair (I-0104)",
          "[HistoryCApi][I-0104]") {
    HistoryRoot ROOT_; const char* ROOT = ROOT_.c();

    // Session 1: establish a baseline of "A", then close.
    scrivi_free(scrivi_history_open(ROOT));
    scrivi_free(scrivi_history_seed_scene(ROOT, "scene_a", "A"));
    scrivi_free(scrivi_history_note_scene_persisted(ROOT, "scene_a", "A"));
    scrivi_free(scrivi_history_close(ROOT));

    // Session 2: disk now says something else (a genuine external edit).
    scrivi_free(scrivi_history_open(ROOT));
    {
        auto r = okResult(scrivi_history_validate_scene(ROOT, "scene_a", "EDITED OUTSIDE"));
        REQUIRE(r.getBool("externalChange"));   // correct: warn once
    }
    scrivi_free(scrivi_history_close(ROOT));

    // Session 3: the SAME unchanged text must now be accepted — the repair persisted.
    scrivi_free(scrivi_history_open(ROOT));
    {
        auto r = okResult(scrivi_history_validate_scene(ROOT, "scene_a", "EDITED OUTSIDE"));
        REQUIRE_FALSE(r.getBool("externalChange"));   // must NOT re-flag forever
    }
    scrivi_free(scrivi_history_close(ROOT));
}

// I-0104: the user's exact remaining symptom (2026-08-10) — a scene the writer NEVER
// touches re-flags on every quit/relaunch, forever. An untouched scene never saves, so
// noteScenePersisted never runs for it; the ONLY thing that can update its baseline is
// validateSceneHead's repair. If that repair does not survive to the next open, the
// scene re-flags indefinitely. Three opens, one genuine external edit, no saves at all.
TEST_CASE("C ABI: an UNTOUCHED mismatching scene heals without any save (I-0104)",
          "[HistoryCApi][I-0104]") {
    HistoryRoot ROOT_; const char* ROOT = ROOT_.c();

    // Session 1: baseline "A" recorded via the save path, then closed.
    scrivi_free(scrivi_history_open(ROOT));
    scrivi_free(scrivi_history_seed_scene(ROOT, "scene_a", "A"));
    scrivi_free(scrivi_history_note_scene_persisted(ROOT, "scene_a", "A"));
    scrivi_free(scrivi_history_close(ROOT));

    // Session 2: disk differs. Warn once, repair, close. NO save this session.
    scrivi_free(scrivi_history_open(ROOT));
    {
        auto r = okResult(scrivi_history_validate_scene(ROOT, "scene_a", "CHANGED"));
        REQUIRE(r.getBool("externalChange"));
    }
    scrivi_free(scrivi_history_close(ROOT));

    // Sessions 3 and 4: same unchanged text, still no saves. Must be silent BOTH times —
    // one repeat would be the reported "new notice every relaunch".
    for (int i = 0; i < 2; ++i) {
        scrivi_free(scrivi_history_open(ROOT));
        auto r = okResult(scrivi_history_validate_scene(ROOT, "scene_a", "CHANGED"));
        REQUIRE_FALSE(r.getBool("externalChange"));
        scrivi_free(scrivi_history_close(ROOT));
    }
}

// I-0104 — the barrier LOOP (user-reported 2026-08-10, forensics on the real project).
//
// The earlier "heals without any save" test passed while the writer's project still
// re-flagged 14 scenes on every launch, because it never forced a checkpoint between
// the repair and the close. checkpoint() preferred `loadedHeadHashes_` — the value read
// from state.json at OPEN — over the floor the repair had just adopted from disk, so it
// wrote the stale hash straight back. Every open then repeated: warn, repair, persist
// stale, forever. On the real project 13 of 14 stuck scenes had a *superseded* floor's
// hash in state.json while the newest floor matched the file exactly.
TEST_CASE("C ABI: a repaired baseline survives a mid-session checkpoint (I-0104)",
          "[HistoryCApi][I-0104]") {
    HistoryRoot ROOT_; const char* ROOT = ROOT_.c();

    scrivi_free(scrivi_history_open(ROOT));
    scrivi_free(scrivi_history_seed_scene(ROOT, "scene_a", "A"));
    scrivi_free(scrivi_history_note_scene_persisted(ROOT, "scene_a", "A"));
    scrivi_free(scrivi_history_close(ROOT));

    // Session 2: repair, then CHECKPOINT explicitly before closing — the ordering the
    // real app hits (checkpoint also fires every 200 records, not just at close).
    scrivi_free(scrivi_history_open(ROOT));
    {
        auto r = okResult(scrivi_history_validate_scene(ROOT, "scene_a", "CHANGED OUTSIDE"));
        REQUIRE(r.getBool("externalChange"));
    }
    forceCheckpoint(ROOT);
    scrivi_free(scrivi_history_close(ROOT));

    // Sessions 3-5: the repair must hold. Three cycles because the user reported the
    // count climbing by one on EVERY relaunch.
    for (int i = 0; i < 3; ++i) {
        scrivi_free(scrivi_history_open(ROOT));
        auto r = okResult(scrivi_history_validate_scene(ROOT, "scene_a", "CHANGED OUTSIDE"));
        REQUIRE_FALSE(r.getBool("externalChange"));
        forceCheckpoint(ROOT);
        scrivi_free(scrivi_history_close(ROOT));
    }
}

// I-0104 — the stale-hash source, isolated. A scene can be checkpointed WITHOUT being
// validated in that session (the app validates only the scenes it loaded into the
// editor; checkpoint persists every scene in floorTexts()). For those scenes the old
// code wrote `loadedHeadHashes_` — the value read at open — even though the floor had
// since been re-seeded from disk by an earlier repair. state.json therefore kept a
// superseded hash, which is exactly what forensics found on the writer's project:
// 13 of 14 stuck scenes had the hash of an OLD floor while the newest floor matched
// the file byte-for-byte.
TEST_CASE("C ABI: checkpoint persists the CURRENT floor, not the hash read at open (I-0104)",
          "[HistoryCApi][I-0104]") {
    HistoryRoot ROOT_; const char* ROOT = ROOT_.c();

    // Session 1: baseline "ORIGINAL".
    scrivi_free(scrivi_history_open(ROOT));
    scrivi_free(scrivi_history_seed_scene(ROOT, "scene_a", "ORIGINAL"));
    scrivi_free(scrivi_history_note_scene_persisted(ROOT, "scene_a", "ORIGINAL"));
    scrivi_free(scrivi_history_close(ROOT));

    // Session 2: repair to "REPAIRED", then close (state.json should now say REPAIRED).
    scrivi_free(scrivi_history_open(ROOT));
    scrivi_free(scrivi_history_validate_scene(ROOT, "scene_a", "REPAIRED"));
    scrivi_free(scrivi_history_close(ROOT));

    // Session 3: open and checkpoint WITHOUT validating this scene — the case that
    // regressed. If checkpoint writes the open-time hash instead of the current floor,
    // it rewrites the stale value and the next session warns all over again.
    scrivi_free(scrivi_history_open(ROOT));
    forceCheckpoint(ROOT);
    scrivi_free(scrivi_history_close(ROOT));

    // Session 4: the repaired baseline must still hold.
    scrivi_free(scrivi_history_open(ROOT));
    {
        auto r = okResult(scrivi_history_validate_scene(ROOT, "scene_a", "REPAIRED"));
        REQUIRE_FALSE(r.getBool("externalChange"));
    }
    scrivi_free(scrivi_history_close(ROOT));
}

// I-0104 — THE LOOP, finally reproduced (forensics on the writer's project, 2026-08-10).
//
// Two conditions must hold together, which is why earlier attempts kept passing:
//   1. state.json holds a STALE hash (loadedHeadHashes_ is seeded from it at open), and
//   2. the replayed FLOOR already equals the file on disk.
//
// Then every open: validateSceneHead compares disk against the stale loadedHeadHashes_
// -> mismatch -> barrier + reseedSceneFloor (a NO-OP, the floor already equals disk) ->
// checkpoint() writes loadedHeadHashes_ back -> unchanged. Warn, "repair", persist stale,
// forever. On the real project all 14 stuck scenes had exactly this shape: newest floor
// == file byte-for-byte, state.json holding a superseded hash.
//
// This test writes state.json directly to construct the condition, because it is a
// PERSISTED inconsistency — no sequence of clean API calls produces it.
TEST_CASE("C ABI: a stale state.json hash does not re-warn forever (I-0104 loop)",
          "[HistoryCApi][I-0104]") {
    namespace fs = std::filesystem;
    HistoryRoot ROOT_; const char* ROOT = ROOT_.c();
    const std::string TEXT = "the text that is genuinely on disk";

    // Session 1: establish floor == TEXT, closed normally.
    scrivi_free(scrivi_history_open(ROOT));
    scrivi_free(scrivi_history_seed_scene(ROOT, "scene_a", TEXT.c_str()));
    scrivi_free(scrivi_history_note_scene_persisted(ROOT, "scene_a", TEXT.c_str()));
    scrivi_free(scrivi_history_close(ROOT));

    // Corrupt ONLY state.json's head hash, leaving the log's floor correct.
    {
        const fs::path sp = fs::path(ROOT_.path) / "history" / "state.json";
        std::ifstream in(sp); std::stringstream ss; ss << in.rdbuf(); in.close();
        auto parsed = parseJson(ss.str());
        REQUIRE(parsed.ok());
        std::string doc = ss.str();
        const std::string bad = "0000000000000000000000000000000000000000000000000000000000000000";
        // Replace the scene's sha256 with a hash that matches nothing.
        auto pos = doc.find("\"sha256\"");
        REQUIRE(pos != std::string::npos);
        auto q1 = doc.find('"', doc.find(':', pos)); auto q2 = doc.find('"', q1 + 1);
        doc = doc.substr(0, q1 + 1) + bad + doc.substr(q2);
        std::ofstream out(sp); out << doc; out.close();
    }

    // First open after corruption: warning here is CORRECT — as far as history knows,
    // the file no longer matches its recorded baseline.
    scrivi_free(scrivi_history_open(ROOT));
    {
        auto r = okResult(scrivi_history_validate_scene(ROOT, "scene_a", TEXT.c_str()));
        REQUIRE(r.getBool("externalChange"));
    }
    scrivi_free(scrivi_history_close(ROOT));

    // Every subsequent open must be SILENT. This is the actual defect: the repair must
    // persist, so the writer is told once and never again.
    for (int cycle = 2; cycle <= 4; ++cycle) {
        scrivi_free(scrivi_history_open(ROOT));
        auto r = okResult(scrivi_history_validate_scene(ROOT, "scene_a", TEXT.c_str()));
        REQUIRE_FALSE(r.getBool("externalChange"));
        scrivi_free(scrivi_history_close(ROOT));
    }
}

// I-0104 — THE actual barrier loop, root-caused by an instrumented probe on a copy of
// the writer's project (2026-08-11).
//
// The repair was never broken. `validateSceneHead` recorded a barrier and re-seeded the
// floor from disk correctly EVERY time — forensics showed the floor had been right for
// 77 consecutive repairs while `state.json` stayed frozen on floor #2 of 79. What was
// missing is that the app never CLOSED the history on quit, so `checkpoint()` never ran
// and the repaired baseline died with the process. Next launch: same stale hash, same
// barrier, forever, whether or not the writer touched the scene.
//
// This test asserts the persistence contract the app depends on: a repair that is
// followed by a close must survive, and the scene must never warn twice.
TEST_CASE("C ABI: an externalChange repair is lost unless the history is CLOSED (I-0104)",
          "[HistoryCApi][I-0104]") {
    HistoryRoot ROOT_; const char* ROOT = ROOT_.c();
    const std::string DISK = "the text that is really on disk";

    scrivi_free(scrivi_history_open(ROOT));
    scrivi_free(scrivi_history_seed_scene(ROOT, "scene_a", "original"));
    scrivi_free(scrivi_history_note_scene_persisted(ROOT, "scene_a", "original"));
    scrivi_free(scrivi_history_close(ROOT));

    // Session 2: the file differs → warn once, repair, and CLOSE (the fix). Closing is
    // what writes state.json; the app previously skipped it entirely on quit.
    scrivi_free(scrivi_history_open(ROOT));
    {
        auto r = okResult(scrivi_history_validate_scene(ROOT, "scene_a", DISK.c_str()));
        REQUIRE(r.getBool("externalChange"));
    }
    scrivi_free(scrivi_history_close(ROOT));

    // Sessions 3-5: silent. The writer is told once, not on every launch.
    for (int cycle = 3; cycle <= 5; ++cycle) {
        scrivi_free(scrivi_history_open(ROOT));
        auto r = okResult(scrivi_history_validate_scene(ROOT, "scene_a", DISK.c_str()));
        REQUIRE_FALSE(r.getBool("externalChange"));
        scrivi_free(scrivi_history_close(ROOT));
    }

    // And a genuine external edit is still caught afterwards — the repair must not have
    // desensitised the check.
    scrivi_free(scrivi_history_open(ROOT));
    {
        auto r = okResult(scrivi_history_validate_scene(ROOT, "scene_a", "edited again elsewhere"));
        REQUIRE(r.getBool("externalChange"));
    }
    scrivi_free(scrivi_history_close(ROOT));
}

// I-0110 — replaying a purge that contains the current node must not break open().
//
// Reported 2026-08-11 from the writer's real project:
//   [Scrivi] historyOpen failed: ScriviError(code: 13,
//     "unhandled exception: HistoryService: unknown node evt_019ff122-...")
// The project opened with NO undo/redo at all.
//
// Forensics: the log's LAST ctl record was a purge whose branchRootEventID was an
// ANCESTOR of the newest event. On replay, applyLoadedEviction erased that subtree —
// including the node currentNodeID_ pointed at — and nothing walked the pointer back,
// so the next nodeRef() threw and the exception escaped scrivi_history_open.
//
// pruneInconsistentNodes already had this guard; the eviction path did not.
TEST_CASE("C ABI: open survives a purge that contains the current node (I-0110)",
          "[HistoryCApi][I-0110]") {
    HistoryRoot ROOT_; const char* ROOT = ROOT_.c();

    // Build a fork so there is a non-primary branch to purge, then leave the current
    // pointer ON that branch — the shape that made the purge swallow it.
    scrivi_free(scrivi_history_open(ROOT));
    scrivi_free(scrivi_history_seed_scene(ROOT, "scene_a", ""));
    scrivi_free(scrivi_history_record_event(
        ROOT, "scene_a", "base", recordParams("typing", 0, 4).c_str()));
    scrivi_free(scrivi_history_record_event(
        ROOT, "scene_a", "base one", recordParams("typing", 4, 8).c_str()));
    scrivi_free(scrivi_history_undo(ROOT));
    // Fork: this branch becomes primary and the current pointer sits on it.
    std::string branchRoot;
    {
        auto r = okResult(scrivi_history_record_event(
            ROOT, "scene_a", "base two", recordParams("typing", 4, 8).c_str()));
        branchRoot = r.getString("eventID");
    }
    scrivi_free(scrivi_history_record_event(
        ROOT, "scene_a", "base two three", recordParams("typing", 8, 14).c_str()));
    scrivi_free(scrivi_history_close(ROOT));

    // Purge the branch the current pointer is inside. The endpoint refuses to purge a
    // live branch, so append the ctl:purge directly — which is exactly the on-disk
    // state the writer's log was in.
    {
        namespace fs = std::filesystem;
        const fs::path log = fs::path(ROOT_.path) / "history" / "log-000001.jsonl";
        std::ofstream out(log, std::ios::app);
        out << "{\"rec\":\"ctl\",\"seq\":1,\"op\":\"purge\",\"branchRootEventID\":\""
            << branchRoot << "\"}\n";
    }

    // Must open cleanly (pointer walked back), not throw.
    {
        auto env = envelope(scrivi_history_open(ROOT));
        REQUIRE(env.getBool("ok"));
    }
    // And the surviving history is usable rather than merely non-crashing.
    scrivi_free(scrivi_history_record_event(
        ROOT, "scene_a", "recovered text", recordParams("typing", 0, 14).c_str()));
    {
        auto r = okResult(scrivi_history_undo(ROOT));
        REQUIRE(r.getBool("moved"));
    }
    scrivi_free(scrivi_history_close(ROOT));
}

// I-0111 — prune-driven purge records must carry a sane sequence number.
//
// Found while investigating I-0110 (2026-08-11): the writer's log held 13 ctl:purge
// records all stamped **seq 1**, producing 11 sequence regressions in a 3,607-record
// log. Cause: `openOrCreate` ran `pruneInconsistentNodes()` + `persistPurge()` while
// `lastSeq_` was still 0 — the replayed high-water mark was not adopted until ~34
// lines later. persistPurge stamps `++lastSeq_`, so each wrote seq 1.
//
// Replay itself is ordered by file position, not seq, so nothing malfunctioned — but
// the numbers were meaningless and actively misled log forensics. This asserts the
// monotonic-sequence invariant the format implies.
TEST_CASE("C ABI: a prune-driven purge does not reset the sequence counter (I-0111)",
          "[HistoryCApi][I-0111]") {
    namespace fs = std::filesystem;
    HistoryRoot ROOT_; const char* ROOT = ROOT_.c();

    std::string lastEventID;
    scrivi_free(scrivi_history_open(ROOT));
    scrivi_free(scrivi_history_seed_scene(ROOT, "scene_a", ""));
    for (int i = 1; i <= 5; ++i) {
        auto r = okResult(scrivi_history_record_event(
            ROOT, "scene_a", std::string(static_cast<std::size_t>(i), 'x').c_str(),
            recordParams("typing", i - 1, i).c_str()));
        lastEventID = r.getString("eventID");
    }
    scrivi_free(scrivi_history_close(ROOT));

    // Append an event whose diff cannot have come from its scene's text, so the next
    // open's pruneInconsistentNodes() drops it AND calls persistPurge() — the path
    // that used to stamp seq 1 because lastSeq_ had not been adopted yet.
    {
        const fs::path log = fs::path(ROOT_.path) / "history" / "log-000001.jsonl";
        std::ofstream out(log, std::ios::app);
        out << R"({"rec":"event","seq":9000,"eventID":"evt_bogus",)"
            << R"("parentID":")" << lastEventID << R"(","kind":"typing","sceneID":"scene_a",)"
            << R"("diff":{"offsetUtf8":0,"removed":"NOT-THE-TEXT","inserted":"zz"},)"
            << R"("cursorBefore":0,"cursorAfter":2,"timestamp":"2026-08-11T00:00:00Z",)"
            << R"("sessionID":"ses_x"})" << "\n";
    }

    // This open prunes the bogus node and persists a ctl:purge for it.
    scrivi_free(scrivi_history_open(ROOT));
    scrivi_free(scrivi_history_close(ROOT));

    // That purge must carry a seq above the replayed high-water mark (9000), not 1.
    const std::string log = readHistoryLog(ROOT);
    std::istringstream in(log);
    std::string line;
    bool sawPurge = false;
    while (std::getline(in, line)) {
        if (line.empty()) continue;
        auto parsed = parseJson(line);
        if (!parsed.ok()) continue;
        const JsonDoc d = std::move(parsed.value());
        if (d.getString("rec") == "ctl" && d.getString("op") == "purge") {
            sawPurge = true;
            INFO("purge record: " << line);
            REQUIRE(d.getInt64("seq") > 9000);
        }
    }
    REQUIRE(sawPurge);   // the test is meaningless if no purge was written
}
