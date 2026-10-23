// RangeOperationCApiTests.cpp — EP-053 S1 (SP-186, T-0614): the range operations, tested THROUGH `scrivi_range_*`
// (a facade test cannot see a boundary gap — that is how I-0113 shipped green).
//
// Each write is checked three ways: the manuscript reads as the design says (§4.3, §4.4, [I-0311], Q2); the delta it
// returned is what is on disk; and `scrivi_scan_for_external_changes` finds nothing — the chapter sidecars' scene lists
// were kept current (the budget ruling of 2026-10-20). The inverses (AC5) restore the manuscript exactly, scene and chapter
// IDs included (the identity ruling of 2026-10-20).

#include <catch2/catch_test_macros.hpp>

#include "manuscript/ProjectIndex.hpp"
#include "platform/LocalFileSystem.hpp"
#include "schemas/ChapterMetaJson.hpp"
#include "schemas/SceneMetaJson.hpp"
#include "scrivi/Services.hpp"
#include "scrivi/scrivi.h"
#include "util/Json.hpp"
#include "util/OrderKey.hpp"

#include <algorithm>
#include <atomic>
#include <cstdint>
#include <filesystem>
#include <fstream>
#include <map>
#include <sstream>
#include <string>
#include <vector>
#include <unistd.h>   // getpid: ctest runs each test case in its own process

using scrivi::util::JsonDoc;
using scrivi::util::parseJson;

namespace {

namespace fs = std::filesystem;

constexpr const char* kIdentity = "identity-001";
constexpr const char* kPersona  = "persona-001";
constexpr const char* kName     = "Test Author";

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

bool refused(const char* raw) { return !envelope(raw).getBool("ok"); }

std::string readFile(const fs::path& p) {
    std::ifstream in(p, std::ios::binary);
    std::stringstream ss;
    ss << in.rdbuf();
    return ss.str();
}

struct Row {
    std::string chapterID, chapterTitle, sceneID, sceneTitle, text;
    bool operator==(const Row&) const = default;
};

// A project of `chapters` chapters of `scenes` scenes each; scene c<i>s<j> holds "C<i>S<j> body text."
struct Project {
    fs::path root, appSupport;
    std::string projectID;
    std::vector<std::vector<std::string>> sceneIDs;   // [chapter][scene], reading order

    Project(int chapters, int scenes) {
        static std::atomic<int> counter{0};
        const auto base = fs::temp_directory_path() / ("scrivi-ep053-capi-" + std::to_string(::getpid()) + "-" + std::to_string(counter.fetch_add(1)) + "-" +
                                                      std::to_string(reinterpret_cast<std::uintptr_t>(this)));
        fs::remove_all(base);
        root = base / "Novel.scrivi";
        appSupport = base / "support";
        fs::create_directories(appSupport);
        const JsonDoc created = ok(scrivi_create_project(root.c_str(), appSupport.c_str(), "Range Novel", "range-novel",
                                                         kIdentity, kPersona, kName));
        projectID = created.getString("projectID");
        for (int c = 2; c <= chapters; ++c) {
            ok(scrivi_create_chapter(root.c_str(), appSupport.c_str(), projectID.c_str(), kIdentity, kPersona, kName, ""));
        }
        std::vector<fs::path> dirs;
        for (const auto& e : fs::directory_iterator(root / "manuscript")) {
            if (e.is_directory()) { dirs.push_back(e.path()); }
        }
        std::sort(dirs.begin(), dirs.end());   // folder order IS reading order (EP-027 B3)
        for (std::size_t c = 0; c < dirs.size(); ++c) {
            std::vector<std::string> ids;
            // The core made each chapter's first scene (`001-…`), empty: give it text and keep its ID.
            fs::path firstMeta;
            for (const auto& e : fs::directory_iterator(dirs[c])) {
                const std::string name = e.path().filename().string();
                if (name.starts_with("001-") && name.ends_with(".meta.json")) { firstMeta = e.path(); }
            }
            auto meta = scrivi::schemas::parseSceneMeta(readFile(firstMeta));
            REQUIRE(meta.ok());
            ids.push_back(meta.value().sceneID.value);
            std::ofstream(dirs[c] / meta.value().contentPath, std::ios::binary) << body(int(c) + 1, 1);
            for (int s = 2; s <= scenes; ++s) {
                // ⚠️ Generated-shape keys ("B", "C", …), as the app makes them — not legacy "002": see [I-0312].
                const std::string key(1, char('A' + s - 1));
                const std::string id = "scene-c" + std::to_string(c + 1) + "s" + std::to_string(s);
                writeScene(dirs[c], key, id, body(int(c) + 1, s));
                ids.push_back(id);
            }
            sceneIDs.push_back(std::move(ids));
        }
        // The chapter sidecars' lists, as the core keeps them (the scenes above were written behind its back).
        ok(scrivi_open_project(root.c_str(), appSupport.c_str(), kIdentity));
        REQUIRE(clean());
    }
    ~Project() {
        scrivi_free(scrivi_close_project(root.c_str()));
        std::error_code ec;
        fs::remove_all(root.parent_path(), ec);
    }
    Project(const Project&)            = delete;
    Project& operator=(const Project&) = delete;

    static std::string body(int c, int s) { return "C" + std::to_string(c) + "S" + std::to_string(s) + " body text."; }

    static void writeScene(const fs::path& dir, const std::string& key, const std::string& id, const std::string& text) {
        scrivi::schemas::SceneMetaData meta;
        meta.sceneID.value = id;
        meta.title         = "Title " + id;
        meta.slug          = key + "-scene";
        meta.status        = "draft";
        meta.createdAt     = "2026-06-01T00:00:00Z";
        meta.modifiedAt    = "2026-06-01T00:00:00Z";
        meta.contentPath   = key + "-scene.md";
        std::ofstream(dir / (key + "-scene.meta.json"), std::ios::binary) << scrivi::schemas::serializeSceneMeta(meta);
        std::ofstream(dir / (key + "-scene.md"), std::ios::binary) << text;
    }

    const char* r() const { return root.c_str(); }
    const std::string& id(int c, int s) const { return sceneIDs.at(std::size_t(c - 1)).at(std::size_t(s - 1)); }

    // The manuscript as the DISK has it, from a fresh index (never the cached one).
    std::vector<Row> rows() const {
        scrivi::platform::LocalFileSystem lfs;
        scrivi::CoreServices services;
        services.fileSystem = &lfs;
        auto index = scrivi::manuscript::ProjectIndex::build(root.string(), services);
        REQUIRE(index.ok());
        std::vector<Row> out;
        for (const auto& s : index.value().scenesInOrder()) {
            out.push_back({s.chapterID.value, s.chapterTitle, s.sceneID.value, s.title, readFile(root / s.contentPath)});
        }
        return out;
    }
    scrivi::manuscript::SceneLocation locationOf(const std::string& sceneID) const {
        scrivi::platform::LocalFileSystem lfs;
        scrivi::CoreServices services;
        services.fileSystem = &lfs;
        auto index = scrivi::manuscript::ProjectIndex::build(root.string(), services);
        REQUIRE(index.ok());
        const auto loc = index.value().findScene(scrivi::SceneID{sceneID});
        REQUIRE(loc);
        return *loc;
    }
    scrivi::schemas::SceneMetaData sidecar(const std::string& sceneID) const {
        auto meta = scrivi::schemas::parseSceneMeta(readFile(root / locationOf(sceneID).metadataPath));
        REQUIRE(meta.ok());
        return meta.value();
    }
    void writeSidecar(const std::string& sceneID, const scrivi::schemas::SceneMetaData& meta) const {
        std::ofstream(root / locationOf(sceneID).metadataPath, std::ios::binary) << scrivi::schemas::serializeSceneMeta(meta);
    }
    scrivi::schemas::ChapterMetaData chapterSidecar(const std::string& sceneID) const {
        auto meta = scrivi::schemas::parseChapterMeta(readFile(root / locationOf(sceneID).chapterMetadataPath));
        REQUIRE(meta.ok());
        return meta.value();
    }
    fs::path contentOf(const std::string& sceneID) const {
        scrivi::platform::LocalFileSystem lfs;
        scrivi::CoreServices services;
        services.fileSystem = &lfs;
        auto index = scrivi::manuscript::ProjectIndex::build(root.string(), services);
        REQUIRE(index.ok());
        const auto loc = index.value().findScene(scrivi::SceneID{sceneID});
        REQUIRE(loc);
        return root / loc->contentPath;
    }
    std::string text(const std::string& sceneID) const {
        for (const auto& row : rows()) {
            if (row.sceneID == sceneID) { return row.text; }
        }
        FAIL("no scene " << sceneID);
        return {};
    }
    std::size_t chapterCount() const {
        std::vector<std::string> ids;
        for (const auto& row : rows()) {
            if (ids.empty() || ids.back() != row.chapterID) { ids.push_back(row.chapterID); }
        }
        return ids.size();
    }

    // ✅ The validator and the external-change scan read the chapter sidecars' lists: nothing to report.
    bool clean() const {
        const JsonDoc scan = ok(scrivi_scan_for_external_changes(r(), appSupport.c_str(), 0));
        INFO("scan: " << scan.dump());
        return scan.arraySize("repairIssues") == 0;
    }

    JsonDoc read(const std::string& a, long long ab, const std::string& b, long long bb) const {
        return ok(scrivi_range_read(r(), a.c_str(), ab, b.c_str(), bb)).getSubDoc("range");
    }
    JsonDoc paste(const JsonDoc& range, const std::string& caret, long long byte) const {
        const std::string json = range.dump(-1);
        return ok(scrivi_range_paste(r(), json.c_str(), caret.c_str(), byte, text(caret).c_str(), kIdentity, kPersona, kName));
    }
    JsonDoc cut(const JsonDoc& range) const {
        const std::string json = range.dump(-1);
        return ok(scrivi_range_cut(r(), json.c_str(), kIdentity, kPersona, kName));
    }
    JsonDoc relabel(const JsonDoc& range, const std::string& target) const {
        const std::string json = range.dump(-1);
        return ok(scrivi_range_relabel(r(), json.c_str(), target.c_str(), kIdentity, kPersona, kName));
    }
    // A chapter's scenes, whole and without their text — what an editor sends for a chapter merge.
    JsonDoc chapterRun(int c) const {
        JsonDoc run;
        run.setString("schema", "scrivi.range.v1");
        for (const auto& id : sceneIDs.at(std::size_t(c - 1))) {
            const auto loc = locationOf(id);
            JsonDoc e;
            e.setString("sceneID", id);
            e.setString("chapterID", loc.chapterID.value);
            e.setString("chapterTitle", loc.chapterTitle);
            e.setString("sceneTitle", loc.title);
            e.setInt64("start", 0);
            e.setInt64("length", 0);
            e.setInt64("sceneLength", 0);
            e.setString("text", "");
            run.appendToArray("scenes", std::move(e));
        }
        return run;
    }
    // An empty two-entry Range — a split (Q1). `newChapter` gives the second entry a different (empty) chapter ID.
    static JsonDoc breakRange(const std::string& caretChapterID, bool newChapter) {
        JsonDoc range;
        range.setString("schema", "scrivi.range.v1");
        for (int i = 0; i < 2; ++i) {
            JsonDoc e;
            e.setString("sceneID", "");
            e.setString("chapterID", i == 1 && newChapter ? "" : caretChapterID);
            e.setString("chapterTitle", "");
            e.setString("sceneTitle", "");
            e.setInt64("start", 0);
            e.setInt64("length", 0);
            e.setInt64("sceneLength", 0);
            e.setString("text", "");
            range.appendToArray("scenes", std::move(e));
        }
        return range;
    }
};

std::string flat(const std::vector<Row>& rows) {
    std::string out;
    for (const auto& row : rows) { out += row.text; }
    return out;
}

// Every scene the delta wrote is on disk as it says, and every scene it names is where it says.
void deltaMatchesDisk(const Project& p, const JsonDoc& delta) {
    for (std::size_t i = 0; i < delta.arraySize("scenes"); ++i) {
        const JsonDoc s = delta.arrayItem("scenes", i);
        INFO("delta scene: " << s.dump());
        CHECK(fs::exists(p.root / s.getString("metadataPath")));
        if (s.getString("change") != "moved") {
            CHECK(readFile(p.root / s.getString("contentPath")) == s.getString("text"));
        }
    }
    const auto rows = p.rows();
    for (const auto& id : delta.getStringArray("removedSceneIDs")) {
        CHECK(std::none_of(rows.begin(), rows.end(), [&](const Row& r) { return r.sceneID == id; }));
    }
}

}  // namespace

TEST_CASE("[EP-053] scrivi_range_read reads the range in reading order, and refuses one that is not", "[ep053][range][capi]") {
    Project p(3, 3);
    const JsonDoc range = p.read(p.id(1, 3), 3, p.id(2, 1), 4);
    REQUIRE(range.getString("schema") == "scrivi.range.v1");
    REQUIRE(range.arraySize("scenes") == 2);
    CHECK(range.arrayItem("scenes", 0).getString("text") == Project::body(1, 3).substr(3));
    CHECK(range.arrayItem("scenes", 0).getInt64("start") == 3);
    CHECK(range.arrayItem("scenes", 1).getString("text") == "C2S1");
    CHECK(range.arrayItem("scenes", 0).getString("chapterID") != range.arrayItem("scenes", 1).getString("chapterID"));

    CHECK(refused(scrivi_range_read(p.r(), p.id(2, 1).c_str(), 0, p.id(1, 3).c_str(), 0)));   // ends before it starts
    CHECK(refused(scrivi_range_read(p.r(), p.id(1, 1).c_str(), 0, p.id(1, 1).c_str(), 999)));  // past the scene's end
    CHECK(refused(scrivi_range_read(p.r(), "no-such-scene", 0, p.id(1, 1).c_str(), 1)));
}

TEST_CASE("[EP-053] a paste within a chapter splits the caret scene and creates the scenes after it", "[ep053][range][capi]") {
    Project p(2, 3);
    const auto before = p.rows();
    const JsonDoc copied = p.read(p.id(1, 2), 4, p.id(1, 3), 4);   // "S2 body text." | "C1S3"
    const JsonDoc r = p.paste(copied, p.id(2, 2), 2);               // into "C2|S2 body text."

    const JsonDoc delta = r.getSubDoc("delta");
    REQUIRE(delta.arraySize("scenes") == 2);
    CHECK(delta.arrayItem("scenes", 0).getString("change") == "rewritten");
    CHECK(delta.arrayItem("scenes", 1).getString("change") == "created");
    CHECK(delta.arrayItem("scenes", 1).getString("afterSceneID") == p.id(2, 2));
    deltaMatchesDisk(p, delta);

    const auto after = p.rows();
    REQUIRE(after.size() == before.size() + 1);
    CHECK(after[4].sceneID == p.id(2, 2));
    CHECK(after[4].text == "C2" + Project::body(1, 2).substr(4));
    CHECK(after[5].text == "C1S3" + Project::body(2, 2).substr(2));
    CHECK(after[5].chapterID == after[4].chapterID);
    CHECK(after[5].sceneID != p.id(1, 3));   // copied: the original is still there, so a fresh ID
    CHECK(p.clean());
}

TEST_CASE("[EP-053] a paste that opens a chapter reads as a flat insert [I-0311]", "[ep053][range][capi]") {
    Project p(3, 4);
    const auto before = p.rows();
    const JsonDoc copied = p.read(p.id(1, 4), 5, p.id(2, 1), 4);   // the end of chapter 1, the start of chapter 2
    const JsonDoc r = p.paste(copied, p.id(3, 2), 3);

    const auto after = p.rows();
    // ✅ Flat: the whole manuscript's text is the old text with the range's inserted at the caret.
    std::size_t caretAbs = 0;
    for (const auto& row : before) {
        if (row.sceneID == p.id(3, 2)) { break; }
        caretAbs += row.text.size();
    }
    caretAbs += 3;
    const std::string inserted = Project::body(1, 4).substr(5) + "C2S1";
    CHECK(flat(after) == flat(before).insert(caretAbs, inserted));

    // Chapter 3 now ends at the caret scene; the new chapter holds the pasted scene, then chapter 3's later scenes.
    REQUIRE(after.size() == before.size() + 1);
    const std::string chapter3 = before[8].chapterID;
    CHECK(after[9].sceneID == p.id(3, 2));
    CHECK(after[9].chapterID == chapter3);
    CHECK(after[10].chapterID != chapter3);
    CHECK(after[11].sceneID == p.id(3, 3));
    CHECK(after[12].sceneID == p.id(3, 4));
    CHECK(after[11].chapterID == after[10].chapterID);
    CHECK(p.chapterCount() == 4);

    const JsonDoc delta = r.getSubDoc("delta");
    CHECK(delta.arraySize("createdChapters") == 1);
    CHECK(delta.arrayItem("createdChapters", 0).getString("afterChapterID") == chapter3);
    deltaMatchesDisk(p, delta);
    CHECK(p.clean());
}

TEST_CASE("[EP-053] a paste from a stale editor is refused and writes nothing", "[ep053][range][capi]") {
    Project p(2, 2);
    const auto before = p.rows();
    const std::string json = p.read(p.id(1, 1), 0, p.id(1, 2), 3).dump(-1);
    CHECK(refused(scrivi_range_paste(p.r(), json.c_str(), p.id(2, 1).c_str(), 1, "not what is on disk",
                                     kIdentity, kPersona, kName)));
    CHECK(p.rows() == before);
}

TEST_CASE("[EP-053] a cut within a chapter joins head to tail and deletes the scenes between", "[ep053][range][capi]") {
    Project p(2, 5);
    const JsonDoc r = p.cut(p.read(p.id(1, 2), 3, p.id(1, 4), 2));
    const auto after = p.rows();
    REQUIRE(after.size() == 8);
    CHECK(after[1].sceneID == p.id(1, 2));
    CHECK(after[1].text == "C1S" + Project::body(1, 4).substr(2));   // ✅ joined DIRECTLY
    CHECK(after[2].sceneID == p.id(1, 5));
    const JsonDoc delta = r.getSubDoc("delta");
    CHECK(delta.getStringArray("removedSceneIDs") == std::vector<std::string>{p.id(1, 3), p.id(1, 4)});
    CHECK(r.getSubDoc("range").arraySize("scenes") == 3);
    deltaMatchesDisk(p, delta);
    CHECK(p.clean());
}

TEST_CASE("[EP-053] a cut across chapters removes the emptied chapters and promotes the end chapter's rest (Q2)",
          "[ep053][range][capi]") {
    Project p(4, 4);
    const auto before = p.rows();
    const JsonDoc r = p.cut(p.read(p.id(1, 3), 2, p.id(3, 2), 1));
    const auto after = p.rows();

    CHECK(p.chapterCount() == 2);   // chapters 2 and 3 are gone
    REQUIRE(after.size() == 3 + 2 + 4);
    CHECK(after[2].sceneID == p.id(1, 3));
    CHECK(after[2].text == "C1" + Project::body(3, 2).substr(1));
    CHECK(after[3].sceneID == p.id(3, 3));   // Q2: into the start scene's chapter
    CHECK(after[4].sceneID == p.id(3, 4));
    CHECK(after[3].chapterID == before[0].chapterID);
    CHECK(after[5].chapterID == before[12].chapterID);   // chapter 4 untouched

    const JsonDoc delta = r.getSubDoc("delta");
    CHECK(delta.getStringArray("removedChapterIDs") == std::vector<std::string>{before[4].chapterID, before[8].chapterID});
    deltaMatchesDisk(p, delta);
    CHECK(p.clean());
}

TEST_CASE("[EP-053] a cut, then a paste of its Range at the cut's start, restores the manuscript — IDs included",
          "[ep053][range][capi]") {
    Project p(4, 3);
    const auto before = p.rows();
    const JsonDoc cut = p.cut(p.read(p.id(1, 2), 4, p.id(3, 2), 3));
    REQUIRE(p.rows() != before);
    p.paste(cut.getSubDoc("range"), p.id(1, 2), 4);
    CHECK(p.rows() == before);   // ✅ chapter IDs and titles, scene IDs and titles, every text
    CHECK(p.clean());
}

TEST_CASE("[EP-053] a paste, then a cut of the Range it inserted, restores the manuscript", "[ep053][range][capi]") {
    Project p(3, 4);
    const auto before = p.rows();
    const JsonDoc pasted = p.paste(p.read(p.id(1, 3), 2, p.id(2, 2), 5), p.id(3, 2), 6);   // opens a chapter: [I-0311]
    REQUIRE(p.rows() != before);
    p.cut(pasted.getSubDoc("range"));
    CHECK(p.rows() == before);
    CHECK(p.clean());
}

TEST_CASE("[EP-053] a cut refuses a Range that is not what the manuscript holds", "[ep053][range][capi]") {
    Project p(2, 4);
    const auto before = p.rows();
    // A gap: scene 2 then scene 4.
    JsonDoc gap = p.read(p.id(1, 2), 0, p.id(1, 3), 3);
    JsonDoc skipped;
    skipped.setString("schema", "scrivi.range.v1");
    skipped.appendToArray("scenes", gap.arrayItem("scenes", 0));
    JsonDoc fourth = p.read(p.id(1, 4), 0, p.id(1, 4), 3).arrayItem("scenes", 0);
    skipped.appendToArray("scenes", std::move(fourth));
    const std::string skippedJson = skipped.dump(-1);
    CHECK(refused(scrivi_range_cut(p.r(), skippedJson.c_str(), kIdentity, kPersona, kName)));
    CHECK(p.rows() == before);

    // Text that is not on disk any more: scene 3 was edited outside the range's knowledge.
    const JsonDoc stale = p.read(p.id(1, 2), 0, p.id(1, 3), 3);
    // ⚠️ The SAME length, so it is the text check that refuses it, not the length check.
    std::string edited3 = Project::body(1, 3);
    edited3[0] = 'X';
    std::ofstream(p.contentOf(p.id(1, 3)), std::ios::binary) << edited3;
    const auto edited = p.rows();
    const std::string staleJson = stale.dump(-1);
    CHECK(refused(scrivi_range_cut(p.r(), staleJson.c_str(), kIdentity, kPersona, kName)));
    CHECK(p.rows() == edited);
    CHECK(edited != before);
}

TEST_CASE("[EP-053] the patched index answers as a fresh one would", "[ep053][range][capi]") {
    Project p(3, 3);
    // ✅ Every read below is served by the index the writes PATCHED (R7) — compared with a fresh build from disk.
    auto sameAsDisk = [&](const std::string& from, const std::string& to) {
        const JsonDoc served = p.read(from, 0, to, 0);
        const auto rows = p.rows();
        std::vector<std::string> expected;
        bool in = false;
        for (const auto& row : rows) {
            if (row.sceneID == from) { in = true; }
            if (in) { expected.push_back(row.sceneID + "|" + row.chapterID + "|" + row.sceneTitle); }
            if (row.sceneID == to) { break; }
        }
        std::vector<std::string> got;
        for (std::size_t i = 0; i < served.arraySize("scenes"); ++i) {
            const JsonDoc e = served.arrayItem("scenes", i);
            got.push_back(e.getString("sceneID") + "|" + e.getString("chapterID") + "|" + e.getString("sceneTitle"));
        }
        CHECK(got == expected);
    };
    p.paste(p.read(p.id(1, 2), 3, p.id(2, 2), 2), p.id(3, 1), 4);   // opens a chapter, moves chapter 3's rest [I-0311]
    sameAsDisk(p.id(1, 1), p.id(3, 3));
    p.cut(p.read(p.id(1, 3), 1, p.id(3, 2), 1));                      // across chapters: removes, promotes (Q2)
    const auto rows = p.rows();
    sameAsDisk(rows.front().sceneID, rows.back().sceneID);
}

// ── [SP-191] ─────────────────────────────────────────────────────────────────────────────────────────────────────────────

namespace {

// What Q3b says an undo must bring back — everything but where the files live (slug, contentPath) and the counts.
// ⚠️ `restored` — a scene removed and brought back keeps even its modification stamp; a scene that SURVIVED an operation
// that rewrote its text is stamped as modified, as every save stamps it.
void sameSidecar(const scrivi::schemas::SceneMetaData& a, const scrivi::schemas::SceneMetaData& b, bool restored = true) {
    CHECK(a.sceneID.value == b.sceneID.value);
    CHECK(a.title == b.title);
    CHECK(a.status == b.status);
    CHECK(a.createdAt == b.createdAt);
    if (restored) { CHECK(a.modifiedAt == b.modifiedAt); }
    CHECK(a.storyTime.offsetMs == b.storyTime.offsetMs);
    CHECK(a.storyTime.durationMs == b.storyTime.durationMs);
    CHECK(a.storyTime.bandID == b.storyTime.bandID);
    CHECK(a.tags == b.tags);
    CHECK(a.outline == b.outline);
    REQUIRE(a.todo.size() == b.todo.size());
    for (std::size_t i = 0; i < a.todo.size(); ++i) { CHECK(a.todo[i].text == b.todo[i].text); }
}

// Gives a scene a sidecar worth restoring: status, story time, band, tags, outline, a to-do.
void decorate(const Project& p, const std::string& sceneID, int n) {
    auto meta = p.sidecar(sceneID);
    meta.status               = "revised";
    meta.storyTime.offsetMs   = 1000 * n;
    meta.storyTime.durationMs = 60 * n;
    meta.storyTime.bandID     = "band-" + std::to_string(n);
    meta.tags                 = {"tag-" + std::to_string(n), "shared"};
    meta.outline              = "Outline " + std::to_string(n);
    meta.todo.push_back({});
    meta.todo.back().text     = "To do " + std::to_string(n);
    meta.modifiedAt           = "2026-07-0" + std::to_string(n) + "T00:00:00Z";
    p.writeSidecar(sceneID, meta);
}

// Gives a scene's chapter a sidecar a re-created chapter would not have (its own creation stamp and status).
void decorateChapter(const Project& p, const std::string& sceneID) {
    const fs::path path = p.root / p.locationOf(sceneID).chapterMetadataPath;
    auto meta = scrivi::schemas::parseChapterMeta(readFile(path));
    REQUIRE(meta.ok());
    meta.value().status    = "revised";
    meta.value().createdAt = "2026-05-01T00:00:00Z";
    std::ofstream(path, std::ios::binary) << scrivi::schemas::serializeChapterMeta(meta.value());
}

void sameChapterSidecar(const scrivi::schemas::ChapterMetaData& a, const scrivi::schemas::ChapterMetaData& b) {
    CHECK(a.chapterID.value == b.chapterID.value);
    CHECK(a.title == b.title);
    CHECK(a.status == b.status);
    CHECK(a.createdAt == b.createdAt);
}

// The scenes whose story time is set, as the timeline asks for them — served by the index a write PATCHED.
std::vector<std::string> storyTimed(const Project& p) {
    const JsonDoc r = ok(scrivi_list_story_times(p.r()));
    std::vector<std::string> out;
    for (std::size_t i = 0; i < r.arraySize("storyTimes"); ++i) {
        out.push_back(r.arrayItem("storyTimes", i).getString("sceneID") + "|" +
                      r.arrayItem("storyTimes", i).getString("bandID"));
    }
    return out;
}

}  // namespace

TEST_CASE("[SP-191] Q3b: a cut's undo brings back each removed scene's sidecar exactly", "[sp191][range][capi]") {
    Project p(3, 4);
    decorate(p, p.id(1, 3), 3);
    decorate(p, p.id(2, 1), 5);   // opens chapter 2, which the cut removes
    decorateChapter(p, p.id(2, 1));
    // The sidecars were written behind the core's back: drop the index it holds, as a reopen would.
    scrivi_free(scrivi_close_project(p.r()));
    ok(scrivi_open_project(p.r(), p.appSupport.c_str(), kIdentity));
    const auto timed = storyTimed(p);
    REQUIRE(timed.size() == 2);
    const auto s13 = p.sidecar(p.id(1, 3));
    const auto s21 = p.sidecar(p.id(2, 1));
    const auto c2  = p.chapterSidecar(p.id(2, 1));
    const auto before = p.rows();

    const JsonDoc cut = p.cut(p.read(p.id(1, 2), 4, p.id(2, 2), 3));
    const JsonDoc removed = cut.getSubDoc("range");
    CHECK(removed.arrayItem("scenes", 0).getString("sceneMeta").empty());   // the start scene survives the cut
    CHECK(!removed.arrayItem("scenes", 1).getString("sceneMeta").empty());
    CHECK(removed.arrayItem("scenes", 2).getString("chapterMeta").empty());    // scene 1.4 opens nothing
    CHECK(!removed.arrayItem("scenes", 3).getString("chapterMeta").empty());   // scene 2.1 opens chapter 2

    CHECK(storyTimed(p).empty());

    p.paste(removed, p.id(1, 2), 4);
    CHECK(p.rows() == before);
    sameSidecar(p.sidecar(p.id(1, 3)), s13);
    sameSidecar(p.sidecar(p.id(2, 1)), s21);
    sameChapterSidecar(p.chapterSidecar(p.id(2, 1)), c2);
    CHECK(storyTimed(p) == timed);   // ✅ the timeline sees the restored story times at once
    CHECK(p.clean());
}

TEST_CASE("[SP-191] Q3b: a paste of a COPY starts fresh sidecars, whatever the Range carries", "[sp191][range][capi]") {
    Project p(2, 3);
    decorate(p, p.id(1, 2), 2);
    // The Range a cut would return — sidecars and all — pasted while its scenes are still in the manuscript.
    const JsonDoc copied = p.read(p.id(1, 1), 3, p.id(1, 2), 4);
    JsonDoc carrying;
    carrying.setString("schema", "scrivi.range.v1");
    carrying.appendToArray("scenes", copied.arrayItem("scenes", 0));
    JsonDoc second = copied.arrayItem("scenes", 1);
    second.setString("sceneMeta", scrivi::schemas::serializeSceneMeta(p.sidecar(p.id(1, 2))));
    carrying.appendToArray("scenes", std::move(second));
    const JsonDoc r = p.paste(carrying, p.id(2, 2), 2);
    const JsonDoc delta = r.getSubDoc("delta");
    const std::string created = delta.arrayItem("scenes", 1).getString("sceneID");
    CHECK(created != p.id(1, 2));
    const auto fresh = p.sidecar(created);
    CHECK(fresh.tags.empty());
    CHECK(fresh.outline.empty());
    CHECK(fresh.status == "draft");
    CHECK(p.clean());
}

TEST_CASE("[SP-191] Q3e: a scene merge is a cut of the zero-length break — joined directly, and undone exactly",
          "[sp191][range][capi]") {
    Project p(2, 3);
    decorate(p, p.id(1, 3), 4);
    const auto s13 = p.sidecar(p.id(1, 3));
    const auto before = p.rows();
    const long long end = static_cast<long long>(Project::body(1, 2).size());

    const JsonDoc merged = p.cut(p.read(p.id(1, 2), end, p.id(1, 3), 0));
    const auto after = p.rows();
    REQUIRE(after.size() == before.size() - 1);
    CHECK(after[1].sceneID == p.id(1, 2));
    CHECK(after[1].text == Project::body(1, 2) + Project::body(1, 3));   // ✅ Q3a: no blank line
    CHECK(merged.getSubDoc("delta").getStringArray("removedSceneIDs") == std::vector<std::string>{p.id(1, 3)});
    CHECK(p.clean());

    p.paste(merged.getSubDoc("range"), p.id(1, 2), end);   // the undo: a paste of the break
    CHECK(p.rows() == before);
    sameSidecar(p.sidecar(p.id(1, 3)), s13);
    CHECK(p.clean());
}

TEST_CASE("[SP-191] Q3e: the zero-length cut at a CHAPTER break joins the scenes, as every cut does", "[sp191][range][capi]") {
    Project p(3, 2);
    const auto before = p.rows();
    const long long end = static_cast<long long>(Project::body(1, 2).size());
    const JsonDoc merged = p.cut(p.read(p.id(1, 2), end, p.id(2, 1), 0));
    const auto after = p.rows();
    CHECK(p.chapterCount() == 2);
    CHECK(after[1].text == Project::body(1, 2) + Project::body(2, 1));
    CHECK(after[2].sceneID == p.id(2, 2));
    CHECK(after[2].chapterID == before[0].chapterID);   // chapter 2's rest joins chapter 1 (Q2)
    CHECK(p.clean());
    p.paste(merged.getSubDoc("range"), p.id(1, 2), end);
    CHECK(p.rows() == before);
    CHECK(p.clean());
}

TEST_CASE("[SP-191] Q1: a split is a paste of an empty two-entry Range; its undo is the cut of what it inserted",
          "[sp191][range][capi]") {
    Project p(2, 3);
    const auto before = p.rows();
    const JsonDoc split = p.paste(Project::breakRange(before[1].chapterID, false), p.id(1, 2), 3);
    const auto after = p.rows();
    REQUIRE(after.size() == before.size() + 1);
    CHECK(after[1].text == Project::body(1, 2).substr(0, 3));
    CHECK(after[2].text == Project::body(1, 2).substr(3));
    CHECK(after[2].chapterID == before[1].chapterID);
    CHECK(after[3].sceneID == p.id(1, 3));
    CHECK(p.clean());
    p.cut(split.getSubDoc("range"));
    CHECK(p.rows() == before);
    CHECK(p.clean());
}

TEST_CASE("[SP-191] Q1 + [I-0314]: a chapter split puts the new chapter IN PLACE on disk, the following scenes in it",
          "[sp191][range][capi]") {
    Project p(3, 4);
    const auto before = p.rows();
    const JsonDoc split = p.paste(Project::breakRange(before[4].chapterID, true), p.id(2, 2), 4);
    const auto after = p.rows();   // ✅ from a FRESH index: the disk's order, not the editor's
    REQUIRE(after.size() == before.size() + 1);
    CHECK(flat(after) == flat(before));
    CHECK(p.chapterCount() == 4);
    CHECK(after[5].sceneID == p.id(2, 2));
    CHECK(after[5].chapterID == before[4].chapterID);
    CHECK(after[6].chapterID != before[4].chapterID);   // the tail opens the new chapter
    CHECK(after[6].text == Project::body(2, 2).substr(4));
    CHECK(after[7].sceneID == p.id(2, 3));
    CHECK(after[8].sceneID == p.id(2, 4));
    CHECK(after[7].chapterID == after[6].chapterID);
    CHECK(after[9].chapterID == before[8].chapterID);   // chapter 3 stays after it
    CHECK(p.clean());
    p.cut(split.getSubDoc("range"));
    CHECK(p.rows() == before);
    CHECK(p.clean());
}

TEST_CASE("[SP-191] Q3e: a chapter merge is a relabel — the files move, no text changes — and its undo restores the chapter",
          "[sp191][range][capi]") {
    Project p(3, 3);
    decorate(p, p.id(2, 2), 6);
    decorateChapter(p, p.id(2, 1));
    const auto s22 = p.sidecar(p.id(2, 2));
    const auto c2  = p.chapterSidecar(p.id(2, 1));
    const fs::path c2Dir = (p.root / p.locationOf(p.id(2, 1)).chapterMetadataPath).parent_path();
    const auto before = p.rows();

    const JsonDoc merged = p.relabel(p.chapterRun(2), before[0].chapterID);
    const auto after = p.rows();
    CHECK(p.chapterCount() == 2);
    REQUIRE(after.size() == before.size());
    for (std::size_t i = 0; i < after.size(); ++i) {
        CHECK(after[i].sceneID == before[i].sceneID);   // the reading order never changes
        CHECK(after[i].text == before[i].text);         // nor any text
    }
    for (std::size_t i = 3; i < 6; ++i) { CHECK(after[i].chapterID == before[0].chapterID); }
    CHECK(fs::exists(p.root / p.locationOf(p.id(2, 1)).contentPath));
    CHECK(!fs::exists(c2Dir));   // the emptied chapter is gone, folder and all
    sameSidecar(p.sidecar(p.id(2, 2)), s22);   // a moved sidecar keeps everything
    CHECK(merged.getString("fromChapterID") == before[3].chapterID);
    const JsonDoc delta = merged.getSubDoc("delta");
    CHECK(delta.getStringArray("removedChapterIDs") == std::vector<std::string>{before[3].chapterID});
    CHECK(delta.arraySize("scenes") == 3);
    CHECK(delta.arrayItem("scenes", 0).getString("change") == "moved");
    CHECK(delta.arrayItem("scenes", 0).getString("afterSceneID") == p.id(1, 3));
    deltaMatchesDisk(p, delta);
    CHECK(p.clean());

    const JsonDoc undone = p.relabel(merged.getSubDoc("range"), merged.getString("fromChapterID"));
    CHECK(p.rows() == before);
    sameChapterSidecar(p.chapterSidecar(p.id(2, 1)), c2);
    sameSidecar(p.sidecar(p.id(2, 2)), s22);
    CHECK(undone.getSubDoc("delta").arraySize("createdChapters") == 1);
    CHECK(p.clean());

    p.relabel(undone.getSubDoc("range"), undone.getString("fromChapterID"));   // redo
    CHECK(p.rows() == after);
    CHECK(p.clean());
}

TEST_CASE("[SP-191] a relabel refuses a run that cannot move where it is sent", "[sp191][range][capi]") {
    Project p(3, 3);
    const auto before = p.rows();
    auto refusedRelabel = [&](const JsonDoc& run, const std::string& target) {
        const std::string json = run.dump(-1);
        return refused(scrivi_range_relabel(p.r(), json.c_str(), target.c_str(), kIdentity, kPersona, kName));
    };
    JsonDoc tail;   // chapter 2's last two scenes: they do not open it
    tail.setString("schema", "scrivi.range.v1");
    const JsonDoc whole = p.chapterRun(2);
    tail.appendToArray("scenes", whole.arrayItem("scenes", 1));
    tail.appendToArray("scenes", whole.arrayItem("scenes", 2));
    CHECK(refusedRelabel(tail, before[0].chapterID));         // into the chapter before: must open its chapter
    CHECK(refusedRelabel(p.chapterRun(1), before[6].chapterID));   // not a neighbour
    CHECK(refusedRelabel(p.chapterRun(2), before[3].chapterID));   // already there
    CHECK(p.rows() == before);
}


// ── [SP-191] Q7: at a scene's very start, the scene whose text survives keeps its identity ────────────────────────────────

TEST_CASE("[SP-191] Q7: a split at a scene's start puts the new scene BEFORE it; the scene keeps its text and identity",
          "[sp191][range][capi]") {
    Project p(2, 3);
    decorate(p, p.id(1, 2), 2);
    const auto s12 = p.sidecar(p.id(1, 2));
    const auto before = p.rows();
    const JsonDoc split = p.paste(Project::breakRange(before[1].chapterID, false), p.id(1, 2), 0);
    const auto after = p.rows();
    REQUIRE(after.size() == before.size() + 1);
    CHECK(after[1].text.empty());                       // the new, empty scene
    CHECK(after[1].sceneID != p.id(1, 2));
    CHECK(after[2].sceneID == p.id(1, 2));               // ✅ I-0096: the scene keeps its text, title, sidecar
    CHECK(after[2].text == Project::body(1, 2));
    sameSidecar(p.sidecar(p.id(1, 2)), s12, false);
    CHECK(p.clean());
    const JsonDoc undone = p.cut(split.getSubDoc("range"));   // begins at the new scene's start: the END scene survives
    CHECK(p.rows() == before);
    sameSidecar(p.sidecar(p.id(1, 2)), s12, false);
    CHECK(undone.getString("undoCaretSceneID") == p.id(1, 2));
    CHECK(undone.getInt64("undoCaretByte") == 0);
    CHECK(p.clean());
}

TEST_CASE("[SP-191] Q7: a paste at a scene's start creates the scenes before it and joins the last part to its start",
          "[sp191][range][capi]") {
    Project p(2, 4);
    const auto before = p.rows();
    const JsonDoc copied = p.read(p.id(2, 1), 3, p.id(2, 2), 4);   // "S1 body text." | "C2S2"
    const JsonDoc r = p.paste(copied, p.id(1, 3), 0);
    const auto after = p.rows();
    REQUIRE(after.size() == before.size() + 1);
    CHECK(after[2].text == Project::body(2, 1).substr(3));          // a new scene before the caret scene
    CHECK(after[2].chapterID == before[2].chapterID);
    CHECK(after[3].sceneID == p.id(1, 3));                            // the caret scene keeps its identity…
    CHECK(after[3].text == "C2S2" + Project::body(1, 3));             // …and takes the last part in front of its text
    std::size_t caretAbs = 0;
    for (std::size_t i = 0; i < 2; ++i) { caretAbs += before[i].text.size(); }
    CHECK(flat(after) == flat(before).insert(caretAbs, Project::body(2, 1).substr(3) + "C2S2"));
    deltaMatchesDisk(p, r.getSubDoc("delta"));
    CHECK(p.clean());
    p.cut(r.getSubDoc("range"));
    CHECK(p.rows() == before);
    CHECK(p.clean());
}

TEST_CASE("[SP-191] Q7: a paste at a scene's start that opens a chapter moves the scene and the rest of its chapter",
          "[sp191][range][capi]") {
    Project p(3, 4);
    const auto before = p.rows();
    const JsonDoc copied = p.read(p.id(1, 4), 5, p.id(2, 1), 4);    // the end of chapter 1, the start of chapter 2
    const JsonDoc r = p.paste(copied, p.id(3, 2), 0);
    const auto after = p.rows();
    REQUIRE(after.size() == before.size() + 1);
    const std::string chapter3 = before[8].chapterID;
    CHECK(after[9].chapterID == chapter3);
    CHECK(after[9].text == Project::body(1, 4).substr(5));           // created before, still in chapter 3
    CHECK(after[10].sceneID == p.id(3, 2));                           // the caret scene opens the new chapter
    CHECK(after[10].chapterID != chapter3);
    CHECK(after[10].text == "C2S1" + Project::body(3, 2));
    CHECK(after[11].sceneID == p.id(3, 3));
    CHECK(after[11].chapterID == after[10].chapterID);
    CHECK(p.chapterCount() == 4);
    deltaMatchesDisk(p, r.getSubDoc("delta"));
    CHECK(p.clean());
    p.cut(r.getSubDoc("range"));
    CHECK(p.rows() == before);
    CHECK(p.clean());
}

TEST_CASE("[SP-191] Q7: a paste at the very start of the manuscript, and of a chapter", "[sp191][range][capi]") {
    Project p(2, 2);
    const auto before = p.rows();
    const JsonDoc copied = p.read(p.id(2, 1), 2, p.id(2, 2), 3);
    const JsonDoc first = p.paste(copied, p.id(1, 1), 0);
    auto after = p.rows();
    {   // ✅ the index the paste PATCHED (it inserted before the first scene) answers as the disk does
        const JsonDoc served = p.read(after.front().sceneID, 0, after.back().sceneID, 0);
        REQUIRE(served.arraySize("scenes") == after.size());
        for (std::size_t i = 0; i < after.size(); ++i) {
            CHECK(served.arrayItem("scenes", i).getString("sceneID") == after[i].sceneID);
        }
    }
    CHECK(after[0].text == Project::body(2, 1).substr(2));
    CHECK(after[1].sceneID == p.id(1, 1));
    CHECK(after[1].text == "C2S" + Project::body(1, 1));
    CHECK(p.clean());
    p.cut(first.getSubDoc("range"));
    CHECK(p.rows() == before);

    const JsonDoc second = p.paste(copied, p.id(2, 1), 0);   // a chapter's first scene: the new scene opens the chapter
    after = p.rows();
    CHECK(after[2].chapterID == before[2].chapterID);
    CHECK(after[2].text == Project::body(2, 1).substr(2));
    CHECK(after[3].sceneID == p.id(2, 1));
    CHECK(p.clean());
    p.cut(second.getSubDoc("range"));
    CHECK(p.rows() == before);
    CHECK(p.clean());
}

TEST_CASE("[SP-191] Q7: a cut from a scene's first character keeps the END scene; its undo pastes before it",
          "[sp191][range][capi]") {
    Project p(3, 4);
    decorate(p, p.id(1, 2), 2);
    decorate(p, p.id(2, 2), 7);
    const auto s12 = p.sidecar(p.id(1, 2));
    const auto s22 = p.sidecar(p.id(2, 2));
    const auto before = p.rows();

    // Within a chapter.
    JsonDoc cut = p.cut(p.read(p.id(1, 2), 0, p.id(1, 3), 4));
    auto after = p.rows();
    CHECK(after[1].sceneID == p.id(1, 3));                  // the end scene survives, with its tail
    CHECK(after[1].text == Project::body(1, 3).substr(4));
    CHECK(cut.getSubDoc("delta").getStringArray("removedSceneIDs") == std::vector<std::string>{p.id(1, 2)});
    CHECK(p.clean());
    p.paste(cut.getSubDoc("range"), cut.getString("undoCaretSceneID"), cut.getInt64("undoCaretByte"));
    CHECK(p.rows() == before);
    sameSidecar(p.sidecar(p.id(1, 2)), s12);

    // Across chapters: the end scene survives and joins the start scene's chapter, with the rest of its own.
    cut = p.cut(p.read(p.id(1, 4), 0, p.id(2, 2), 3));
    after = p.rows();
    CHECK(after[3].sceneID == p.id(2, 2));
    CHECK(after[3].chapterID == before[0].chapterID);
    CHECK(after[3].text == Project::body(2, 2).substr(3));
    CHECK(after[4].sceneID == p.id(2, 3));
    CHECK(after[4].chapterID == before[0].chapterID);
    CHECK(p.chapterCount() == 2);
    sameSidecar(p.sidecar(p.id(2, 2)), s22, false);           // a survivor keeps its sidecar
    deltaMatchesDisk(p, cut.getSubDoc("delta"));
    CHECK(p.clean());
    p.paste(cut.getSubDoc("range"), cut.getString("undoCaretSceneID"), cut.getInt64("undoCaretByte"));
    CHECK(p.rows() == before);
    CHECK(p.clean());
}

TEST_CASE("[SP-191] Q7: merging an EMPTY scene into the next keeps the next scene's identity", "[sp191][range][capi]") {
    Project p(1, 3);
    std::ofstream(p.contentOf(p.id(1, 2)), std::ios::binary) << "";
    scrivi_free(scrivi_close_project(p.r()));
    ok(scrivi_open_project(p.r(), p.appSupport.c_str(), kIdentity));
    const auto before = p.rows();
    const JsonDoc merged = p.cut(p.read(p.id(1, 2), 0, p.id(1, 3), 0));
    const auto after = p.rows();
    REQUIRE(after.size() == 2);
    CHECK(after[1].sceneID == p.id(1, 3));
    CHECK(after[1].text == Project::body(1, 3));
    p.paste(merged.getSubDoc("range"), merged.getString("undoCaretSceneID"), merged.getInt64("undoCaretByte"));
    CHECK(p.rows() == before);
    CHECK(p.clean());
}

// ── [SP-191] MOVE THE SMALLER SIDE (user, 2026-10-22) ────────────────────────────────────────────────────────────────────
// A chapter split, a chapter merge and their undos move the FEWER scenes: over a share each move is a rename, and renames
// there do not overlap. Each case below is built so the smaller side is the one the plain shape would NOT move, and checks
// the same things as the plain shape (order, text, chapter IDs and sidecars, a clean scan) plus what only this shape can
// get wrong: a scene whose FOLDER changed without being in the delta, and an index that no longer answers as the disk.

namespace {

std::map<std::string, scrivi::manuscript::SceneLocation> locations(const Project& p) {
    scrivi::platform::LocalFileSystem lfs;
    scrivi::CoreServices services;
    services.fileSystem = &lfs;
    auto index = scrivi::manuscript::ProjectIndex::build(p.root.string(), services);
    REQUIRE(index.ok());
    std::map<std::string, scrivi::manuscript::SceneLocation> out;
    for (const auto& s : index.value().scenesInOrder()) { out[s.sceneID.value] = s; }
    return out;
}

// ✅ Both apps keep each scene's paths AND its chapter's sidecar path (Linux renames a chapter through it), and update
// them only from the delta: every scene whose files or chapter folder changed must be in it, as the disk now has it.
void deltaNamesEveryRelocatedScene(const Project& p, const std::map<std::string, scrivi::manuscript::SceneLocation>& was,
                                   const JsonDoc& delta) {
    std::map<std::string, std::size_t> named;   // sceneID → its place in the delta
    for (std::size_t i = 0; i < delta.arraySize("scenes"); ++i) { named[delta.arrayItem("scenes", i).getString("sceneID")] = i; }
    for (const auto& [id, now] : locations(p)) {
        const auto it = was.find(id);
        if (it == was.end()) { continue; }   // created: the delta's own check covers it
        const auto& old = it->second;
        if (old.metadataPath == now.metadataPath && old.chapterMetadataPath == now.chapterMetadataPath &&
            old.chapterID.value == now.chapterID.value) {
            continue;
        }
        INFO("relocated scene " << id << ": " << old.chapterMetadataPath << " → " << now.chapterMetadataPath);
        REQUIRE(named.count(id) == 1);
        const JsonDoc d = delta.arrayItem("scenes", named.at(id));
        CHECK(d.getString("metadataPath") == now.metadataPath);
        CHECK(d.getString("contentPath") == now.contentPath);
        CHECK(d.getString("chapterMetadataPath") == now.chapterMetadataPath);
        CHECK(d.getString("chapterID") == now.chapterID.value);
    }
}

// ✅ The index the write PATCHED answers as a fresh build from disk does (R7).
void indexServesDisk(const Project& p) {
    const auto rows = p.rows();
    const JsonDoc served = p.read(rows.front().sceneID, 0, rows.back().sceneID, 0);
    REQUIRE(served.arraySize("scenes") == rows.size());
    for (std::size_t i = 0; i < rows.size(); ++i) {
        const JsonDoc e = served.arrayItem("scenes", i);
        CHECK(e.getString("sceneID") == rows[i].sceneID);
        CHECK(e.getString("chapterID") == rows[i].chapterID);
    }
}

// Whole scenes without their text — a run, as an editor sends it for a relabel.
JsonDoc runOf(const Project& p, const std::vector<std::string>& ids) {
    JsonDoc run;
    run.setString("schema", "scrivi.range.v1");
    for (const auto& id : ids) {
        const auto loc = p.locationOf(id);
        JsonDoc e;
        e.setString("sceneID", id);
        e.setString("chapterID", loc.chapterID.value);
        e.setString("chapterTitle", loc.chapterTitle);
        e.setString("sceneTitle", loc.title);
        e.setInt64("start", 0);
        e.setInt64("length", 0);
        e.setInt64("sceneLength", 0);
        e.setString("text", "");
        run.appendToArray("scenes", std::move(e));
    }
    return run;
}

// ✅ Every chapter folder is whole: it has a sidecar whose `slug` is the folder's name (`ChapterIndex.cpp` keeps that
// invariant), no chapter ID is in two folders, and there is no folder beyond the chapters — an emptied folder left behind
// would hold a second copy of a chapter's sidecar, which neither the scan nor the index sees.
void chaptersAreWhole(const Project& p) {
    std::vector<std::string> ids;
    for (const auto& e : fs::directory_iterator(p.root / "manuscript")) {
        if (!e.is_directory()) { continue; }
        INFO("chapter folder " << e.path().filename());
        REQUIRE(fs::exists(e.path() / "chapter.meta.json"));
        auto meta = scrivi::schemas::parseChapterMeta(readFile(e.path() / "chapter.meta.json"));
        REQUIRE(meta.ok());
        CHECK(meta.value().slug == e.path().filename().string());
        ids.push_back(meta.value().chapterID.value);
    }
    std::sort(ids.begin(), ids.end());
    CHECK(std::adjacent_find(ids.begin(), ids.end()) == ids.end());
    CHECK(ids.size() == p.chapterCount());
}

}  // namespace

TEST_CASE("[SP-191] a chapter split near a chapter's start moves the scenes BEFORE the caret; its undo moves them back",
          "[sp191][range][capi][smaller]") {
    Project p(3, 6);
    decorateChapter(p, p.id(2, 1));
    decorate(p, p.id(2, 1), 4);
    const auto c2  = p.chapterSidecar(p.id(2, 1));
    const auto s21 = p.sidecar(p.id(2, 1));
    const auto before = p.rows();
    const auto was = locations(p);
    const std::string c2Meta = was.at(p.id(2, 1)).chapterMetadataPath;

    // In scene 2 of 6: the two scenes up to the caret are the fewer, so they move; the four after it stay in the folder.
    const JsonDoc split = p.paste(Project::breakRange(before[6].chapterID, true), p.id(2, 2), 4);
    const auto after = p.rows();
    REQUIRE(after.size() == before.size() + 1);
    CHECK(flat(after) == flat(before));
    CHECK(p.chapterCount() == 4);
    CHECK(after[6].sceneID == p.id(2, 1));
    CHECK(after[7].sceneID == p.id(2, 2));
    CHECK(after[6].chapterID == before[6].chapterID);   // chapter 2 keeps the scenes up to the caret, and its ID
    CHECK(after[7].chapterID == before[6].chapterID);
    CHECK(after[8].chapterID != before[6].chapterID);   // the tail opens the new chapter…
    CHECK(after[8].text == Project::body(2, 2).substr(4));
    for (std::size_t i = 9; i < 13; ++i) {
        CHECK(after[i].sceneID == before[i - 1].sceneID);
        CHECK(after[i].chapterID == after[8].chapterID);   // …with the rest of chapter 2
    }
    CHECK(after[13].chapterID == before[12].chapterID);
    // ✅ The smaller side moved: the two scenes up to the caret left the folder; the four after it stayed there.
    const auto now = locations(p);
    CHECK(now.at(p.id(2, 1)).chapterMetadataPath != c2Meta);
    CHECK(now.at(p.id(2, 2)).chapterMetadataPath == now.at(p.id(2, 1)).chapterMetadataPath);
    CHECK(now.at(p.id(2, 3)).chapterMetadataPath == c2Meta);
    CHECK(now.at(p.id(2, 3)).metadataPath == was.at(p.id(2, 3)).metadataPath);
    sameChapterSidecar(p.chapterSidecar(p.id(2, 1)), c2);   // chapter 2's sidecar followed its scenes
    CHECK(p.chapterSidecar(p.id(2, 3)).title.empty());      // the new chapter is untitled (Q10)
    sameSidecar(p.sidecar(p.id(2, 1)), s21);
    deltaMatchesDisk(p, split.getSubDoc("delta"));
    deltaNamesEveryRelocatedScene(p, was, split.getSubDoc("delta"));
    indexServesDisk(p);
    chaptersAreWhole(p);
    CHECK(p.clean());

    const auto wasSplit = locations(p);
    const JsonDoc undone = p.cut(split.getSubDoc("range"));
    CHECK(p.rows() == before);
    CHECK(locations(p).at(p.id(2, 1)).chapterMetadataPath == c2Meta);   // back in its own folder: the two scenes moved
    sameChapterSidecar(p.chapterSidecar(p.id(2, 1)), c2);
    sameSidecar(p.sidecar(p.id(2, 1)), s21);
    deltaMatchesDisk(p, undone.getSubDoc("delta"));
    deltaNamesEveryRelocatedScene(p, wasSplit, undone.getSubDoc("delta"));
    indexServesDisk(p);
    chaptersAreWhole(p);
    CHECK(p.clean());

    p.paste(undone.getSubDoc("range"), undone.getString("undoCaretSceneID"), undone.getInt64("undoCaretByte"));   // redo
    CHECK(p.rows() == after);
    CHECK(p.clean());
}

TEST_CASE("[SP-191] a run into a new chapter moves the fewer scenes left behind; the merge back folds the smaller chapter",
          "[sp191][range][capi][smaller]") {
    Project p(3, 4);
    decorateChapter(p, p.id(1, 1));
    decorate(p, p.id(1, 1), 1);
    decorate(p, p.id(1, 3), 3);
    const auto c1  = p.chapterSidecar(p.id(1, 1));
    const auto s11 = p.sidecar(p.id(1, 1));
    const auto s13 = p.sidecar(p.id(1, 3));
    const auto before = p.rows();
    const auto was = locations(p);
    const std::string c1Meta = was.at(p.id(1, 1)).chapterMetadataPath;

    // A chapter split at scene 2's start (Q8): scenes 2–4 go into a new chapter; scene 1, the fewer, is what moves.
    const JsonDoc split = p.relabel(runOf(p, {p.id(1, 2), p.id(1, 3), p.id(1, 4)}), "");
    const auto after = p.rows();
    CHECK(p.chapterCount() == 4);
    CHECK(after[0].chapterID == before[0].chapterID);
    for (std::size_t i = 1; i < 4; ++i) { CHECK(after[i].chapterID == after[1].chapterID); }
    CHECK(after[1].chapterID != before[0].chapterID);
    CHECK(after[4].chapterID == before[4].chapterID);
    CHECK(flat(after) == flat(before));
    const auto now = locations(p);
    CHECK(now.at(p.id(1, 1)).chapterMetadataPath != c1Meta);
    CHECK(now.at(p.id(1, 2)).chapterMetadataPath == c1Meta);
    sameChapterSidecar(p.chapterSidecar(p.id(1, 1)), c1);
    sameSidecar(p.sidecar(p.id(1, 1)), s11);
    deltaMatchesDisk(p, split.getSubDoc("delta"));
    deltaNamesEveryRelocatedScene(p, was, split.getSubDoc("delta"));
    CHECK(split.getSubDoc("delta").arraySize("createdChapters") == 1);
    indexServesDisk(p);
    chaptersAreWhole(p);
    CHECK(p.clean());

    // Its undo, a merge: chapter 1 (one scene) folds into the new chapter's folder, below the three already there.
    const auto wasSplit = locations(p);
    const JsonDoc merged = p.relabel(split.getSubDoc("range"), split.getString("fromChapterID"));
    CHECK(p.rows() == before);
    CHECK(locations(p).at(p.id(1, 1)).chapterMetadataPath == c1Meta);
    sameChapterSidecar(p.chapterSidecar(p.id(1, 1)), c1);
    sameSidecar(p.sidecar(p.id(1, 1)), s11);
    sameSidecar(p.sidecar(p.id(1, 3)), s13);
    CHECK(merged.getSubDoc("delta").getStringArray("removedChapterIDs") == std::vector<std::string>{after[1].chapterID});
    deltaMatchesDisk(p, merged.getSubDoc("delta"));
    deltaNamesEveryRelocatedScene(p, wasSplit, merged.getSubDoc("delta"));
    indexServesDisk(p);
    chaptersAreWhole(p);
    CHECK(p.clean());

    p.relabel(merged.getSubDoc("range"), merged.getString("fromChapterID"));   // redo: the split again
    CHECK(p.rows() == after);
    CHECK(p.clean());
}

TEST_CASE("[SP-191] a chapter merge whose earlier chapter is smaller re-keys its scenes below the later chapter's",
          "[sp191][range][capi][smaller]") {
    Project p(3, 4);
    // Chapter 1's last two scenes go to chapter 2 (re-keyed before its first), so chapter 1 holds 2 scenes, chapter 2 six.
    p.relabel(runOf(p, {p.id(1, 3), p.id(1, 4)}), p.locationOf(p.id(2, 1)).chapterID.value);
    decorate(p, p.id(1, 2), 2);
    decorateChapter(p, p.id(1, 1));
    const auto c1  = p.chapterSidecar(p.id(1, 1));
    const auto s12 = p.sidecar(p.id(1, 2));
    const auto before = p.rows();
    REQUIRE(before[1].chapterID == before[0].chapterID);
    REQUIRE(before[2].chapterID != before[0].chapterID);
    const auto was = locations(p);
    std::vector<std::string> chapter2;
    for (const auto& row : before) {
        if (row.chapterID == before[2].chapterID) { chapter2.push_back(row.sceneID); }
    }
    REQUIRE(chapter2.size() == 6);
    // ⚠️ The case this test exists for: chapter 1's keys (001, B) do NOT sort below chapter 2's first, so they are re-keyed.
    CHECK(was.at(p.id(1, 2)).metadataPath.substr(was.at(p.id(1, 2)).metadataPath.rfind('/') + 1) >=
          was.at(chapter2.front()).metadataPath.substr(was.at(chapter2.front()).metadataPath.rfind('/') + 1));

    const JsonDoc merged = p.relabel(runOf(p, chapter2), before[0].chapterID);
    const auto after = p.rows();
    CHECK(p.chapterCount() == 2);
    REQUIRE(after.size() == before.size());
    for (std::size_t i = 0; i < after.size(); ++i) {
        CHECK(after[i].sceneID == before[i].sceneID);
        CHECK(after[i].text == before[i].text);
    }
    for (std::size_t i = 0; i < 8; ++i) { CHECK(after[i].chapterID == before[0].chapterID); }
    CHECK(locations(p).at(p.id(1, 1)).chapterMetadataPath == was.at(chapter2.front()).chapterMetadataPath);
    sameChapterSidecar(p.chapterSidecar(p.id(1, 1)), c1);
    sameSidecar(p.sidecar(p.id(1, 2)), s12);   // re-keyed: its files renamed, its sidecar kept
    deltaMatchesDisk(p, merged.getSubDoc("delta"));
    deltaNamesEveryRelocatedScene(p, was, merged.getSubDoc("delta"));
    indexServesDisk(p);
    chaptersAreWhole(p);
    CHECK(p.clean());

    p.relabel(merged.getSubDoc("range"), merged.getString("fromChapterID"));   // undo
    CHECK(p.rows() == before);
    sameChapterSidecar(p.chapterSidecar(p.id(1, 1)), c1);
    indexServesDisk(p);
    chaptersAreWhole(p);
    CHECK(p.clean());
}

TEST_CASE("[SP-191] a scene merge across a chapter break moves the smaller start chapter, re-keyed below the end chapter's",
          "[sp191][range][capi][smaller]") {
    Project p(3, 4);
    // Chapter 1 down to 2 scenes (its last two go to chapter 2, re-keyed before its first): chapter 2 holds 6.
    p.relabel(runOf(p, {p.id(1, 3), p.id(1, 4)}), p.locationOf(p.id(2, 1)).chapterID.value);
    decorateChapter(p, p.id(1, 1));
    const auto c1 = p.chapterSidecar(p.id(1, 1));
    const auto before = p.rows();
    const auto was = locations(p);
    // ⌫ at chapter 2's first scene's start: the zero-length cut across the break (Q3e). Chapter 2 is emptied of that one
    // scene and joins chapter 1; chapter 1's two scenes are the fewer, and their keys (001, B) do NOT sort below chapter 2's
    // next scene's, so they are re-keyed there.
    const std::string last = p.id(1, 2), next = before[2].sceneID;
    const long long lastLen = static_cast<long long>(p.text(last).size());
    const JsonDoc merged = p.cut(p.read(last, lastLen, next, 0));
    const auto after = p.rows();
    REQUIRE(after.size() == before.size() - 1);
    CHECK(flat(after) == flat(before));
    CHECK(p.chapterCount() == 2);
    for (std::size_t i = 0; i < 6; ++i) { CHECK(after[i].chapterID == before[0].chapterID); }
    CHECK(locations(p).at(p.id(1, 1)).chapterMetadataPath == was.at(before[3].sceneID).chapterMetadataPath);
    sameChapterSidecar(p.chapterSidecar(p.id(1, 1)), c1);
    deltaMatchesDisk(p, merged.getSubDoc("delta"));
    deltaNamesEveryRelocatedScene(p, was, merged.getSubDoc("delta"));
    indexServesDisk(p);
    chaptersAreWhole(p);
    CHECK(p.clean());

    p.paste(merged.getSubDoc("range"), merged.getString("undoCaretSceneID"), merged.getInt64("undoCaretByte"));   // undo
    CHECK(p.rows() == before);
    sameChapterSidecar(p.chapterSidecar(p.id(1, 1)), c1);
    indexServesDisk(p);
    chaptersAreWhole(p);
    CHECK(p.clean());
}

TEST_CASE("[SP-191] when the scenes before the caret move, they leave before the new scene takes one of their file names",
          "[sp191][range][capi][smaller]") {
    Project p(2, 6);
    // Chapter 2's scene 2 renamed to the very key a chapter split at it gives the new tail scene: the one between nothing
    // and scene 3's "C" (keys are only unique within a chapter, and the tail opens a chapter in this folder).
    const std::string key = scrivi::util::keyBetween("", "C");
    REQUIRE(key > "001");
    REQUIRE(key < "C");
    const auto loc = p.locationOf(p.id(2, 2));
    const fs::path dir = (p.root / loc.metadataPath).parent_path();
    auto meta = p.sidecar(p.id(2, 2));
    meta.slug        = key + "-scene";
    meta.contentPath = key + "-scene.md";
    fs::remove(p.root / loc.metadataPath);
    fs::rename(p.root / loc.contentPath, dir / (key + "-scene.md"));
    std::ofstream(dir / (key + "-scene.meta.json"), std::ios::binary) << scrivi::schemas::serializeSceneMeta(meta);
    scrivi_free(scrivi_close_project(p.r()));
    ok(scrivi_open_project(p.r(), p.appSupport.c_str(), kIdentity));
    REQUIRE(p.clean());
    const auto before = p.rows();

    const JsonDoc split = p.paste(Project::breakRange(before[6].chapterID, true), p.id(2, 2), 4);
    const auto after = p.rows();
    REQUIRE(after.size() == before.size() + 1);
    CHECK(flat(after) == flat(before));
    CHECK(after[7].sceneID == p.id(2, 2));
    CHECK(after[7].text == Project::body(2, 2).substr(0, 4));
    CHECK(after[8].text == Project::body(2, 2).substr(4));
    chaptersAreWhole(p);
    CHECK(p.clean());
    p.cut(split.getSubDoc("range"));
    CHECK(p.rows() == before);
    chaptersAreWhole(p);
    CHECK(p.clean());
}

// ── [SP-188] Q3: the cut BALANCES its join, and its undo is exact ─────────────────────────────────────────────────────────

namespace {

// Give two scenes their own text, behind the core's back, and reopen (as an editor's save would leave it).
void setTexts(Project& p, const std::vector<std::pair<std::string, std::string>>& texts) {
    for (const auto& [id, text] : texts) { std::ofstream(p.contentOf(id), std::ios::binary) << text; }
    scrivi_free(scrivi_close_project(p.r()));
    ok(scrivi_open_project(p.r(), p.appSupport.c_str(), kIdentity));
}

std::size_t count(const std::string& s, const std::string& needle) {
    std::size_t n = 0;
    for (auto at = s.find(needle); at != std::string::npos; at = s.find(needle, at + needle.size())) { ++n; }
    return n;
}

}  // namespace

TEST_CASE("[SP-188] Q3: a cut that begins inside bold closes it in the survivor; undo and redo are exact",
          "[sp188][range][capi]") {
    Project p(2, 3);
    setTexts(p, {{p.id(1, 2), "**Alpha beta.**"}, {p.id(1, 3), "Gamma delta."}});
    const auto before = p.rows();
    const JsonDoc range = p.read(p.id(1, 2), 10, p.id(1, 3), 6);   // from inside `be|ta` to `Gamma |delta.`
    const JsonDoc cut = p.cut(range);
    const std::string survivor = p.text(p.id(1, 2));
    CHECK(survivor == "**Alpha be**delta.");   // ⛔ the plain join would be `**Alpha bedelta.` — bold left open
    const JsonDoc removed = cut.getSubDoc("range");
    REQUIRE(removed.contains("join"));
    CHECK(removed.getSubDoc("join").getString("sceneID") == p.id(1, 2));
    deltaMatchesDisk(p, cut.getSubDoc("delta"));

    // Undo: the paste at the survivor's seam (in the PLAIN text) puts the plain join back first.
    CHECK(cut.getString("undoCaretSceneID") == p.id(1, 2));
    p.paste(removed, cut.getString("undoCaretSceneID"), cut.getInt64("undoCaretByte"));
    CHECK(p.rows() == before);
    CHECK(p.clean());

    // Redo: the cut of the node's Range — the join it carries is ignored and recomputed the same.
    const JsonDoc again = p.cut(removed);
    CHECK(p.text(p.id(1, 2)) == survivor);
    CHECK(again.getSubDoc("range").getSubDoc("join").dump(-1) == removed.getSubDoc("join").dump(-1));
    CHECK(p.clean());
}

TEST_CASE("[SP-188] Q3: a cut that ends inside bold re-opens it in the survivor; undo is exact", "[sp188][range][capi]") {
    Project p(2, 3);
    setTexts(p, {{p.id(1, 2), "Alpha beta."}, {p.id(1, 3), "**Gamma delta.**"}});
    const auto before = p.rows();
    const JsonDoc cut = p.cut(p.read(p.id(1, 2), 6, p.id(1, 3), 5));   // `Alpha |beta.` to `**Gam|ma`
    const std::string survivor = p.text(p.id(1, 2));
    CHECK(count(survivor, "**") == 2);   // ⛔ the plain join `Alpha ma delta.**` has ONE
    CHECK(survivor.starts_with("Alpha "));
    CHECK(survivor.ends_with("ma delta.**"));
    p.paste(cut.getSubDoc("range"), cut.getString("undoCaretSceneID"), cut.getInt64("undoCaretByte"));
    CHECK(p.rows() == before);
    CHECK(p.clean());
}

TEST_CASE("[SP-188] Q3: a cut across a CHAPTER from a scene's first character balances, and undoes exactly",
          "[sp188][range][capi]") {
    Project p(2, 2);
    setTexts(p, {{p.id(1, 2), "**Alpha beta.**"}, {p.id(2, 1), "**Gamma** delta."}});
    const auto before = p.rows();
    const JsonDoc cut = p.cut(p.read(p.id(1, 2), 0, p.id(2, 1), 4));   // the whole of 1.2 (Q7: the END scene survives)
    CHECK(p.chapterCount() == 1);
    for (const auto& row : p.rows()) { CHECK(count(row.text, "**") % 2 == 0); }
    p.paste(cut.getSubDoc("range"), cut.getString("undoCaretSceneID"), cut.getInt64("undoCaretByte"));
    CHECK(p.rows() == before);
    CHECK(p.clean());
}

TEST_CASE("[SP-188] Q3: a plain cut carries no join; a join is refused anywhere but at its survivor",
          "[sp188][range][capi]") {
    Project p(2, 3);
    const JsonDoc plain = p.cut(p.read(p.id(1, 1), 3, p.id(1, 2), 2));
    CHECK_FALSE(plain.getSubDoc("range").contains("join"));

    setTexts(p, {{p.id(2, 2), "**Alpha beta.**"}, {p.id(2, 3), "Gamma delta."}});
    const JsonDoc cut = p.cut(p.read(p.id(2, 2), 10, p.id(2, 3), 6));
    const JsonDoc removed = cut.getSubDoc("range");
    REQUIRE(removed.contains("join"));
    const auto before = p.rows();
    const std::string json = removed.dump(-1);
    // A paste of the undo record at another scene: refused, nothing written.
    CHECK(refused(scrivi_range_paste(p.r(), json.c_str(), p.id(2, 1).c_str(), 0, p.text(p.id(2, 1)).c_str(),
                                     kIdentity, kPersona, kName)));
    // At its survivor, after the writer changed the join: refused too.
    std::ofstream(p.contentOf(p.id(2, 2)), std::ios::binary) << "**Alpha beXYZ delta.**";   // no `**` at the join
    scrivi_free(scrivi_close_project(p.r()));
    ok(scrivi_open_project(p.r(), p.appSupport.c_str(), kIdentity));
    CHECK(refused(scrivi_range_paste(p.r(), json.c_str(), p.id(2, 2).c_str(), 10, p.text(p.id(2, 2)).c_str(),
                                     kIdentity, kPersona, kName)));
    CHECK(p.text(p.id(2, 2)) == "**Alpha beXYZ delta.**");
    CHECK(p.rows().size() == before.size());
}
