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
#include "schemas/SceneMetaJson.hpp"
#include "scrivi/Services.hpp"
#include "scrivi/scrivi.h"
#include "util/Json.hpp"

#include <algorithm>
#include <atomic>
#include <cstdint>
#include <filesystem>
#include <fstream>
#include <sstream>
#include <string>
#include <vector>

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
        const auto base = fs::temp_directory_path() / ("scrivi-ep053-capi-" + std::to_string(counter.fetch_add(1)) + "-" +
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
