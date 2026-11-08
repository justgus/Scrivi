// EP-054 S1 (SP-205, T-0664) — the project archive, through the C ABI only.
//
// Every assertion goes through scrivi_*: a facade test cannot see a boundary gap ([I-0113]).
// What is proved: the round trip is byte-identical and the history replays (AC3); Q9's contents (AC2); Q4's
// Cancel / Rename / Replace with the ID kept (AC4); Q8's worlds (AC5); every rejection leaves nothing (AC6); the
// queued writes land before the archive is written (AC2).

#include <catch2/catch_test_macros.hpp>

#include "scrivi/scrivi.h"
#include "util/Json.hpp"

#include "miniz.h"

#include <atomic>
#include <filesystem>
#include <fstream>
#include <map>
#include <sstream>
#include <string>
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

JsonDoc errorOf(const char* raw) {
    JsonDoc env = envelope(raw);
    INFO("envelope: " << env.dump());
    REQUIRE_FALSE(env.getBool("ok"));
    return env.getSubDoc("error");
}

std::string readFile(const fs::path& p) {
    std::ifstream in(p, std::ios::binary);
    std::stringstream s;
    s << in.rdbuf();
    return s.str();
}

void writeFile(const fs::path& p, const std::string& bytes) {
    fs::create_directories(p.parent_path());
    std::ofstream(p, std::ios::binary | std::ios::trunc) << bytes;
}

// Every regular file under `root`, relative path → bytes.
std::map<std::string, std::string> tree(const fs::path& root) {
    std::map<std::string, std::string> out;
    for (const auto& e : fs::recursive_directory_iterator(root)) {
        if (e.is_regular_file()) { out[e.path().lexically_relative(root).generic_string()] = readFile(e.path()); }
    }
    return out;
}

std::size_t entriesIn(const fs::path& dir) {
    return static_cast<std::size_t>(std::distance(fs::directory_iterator(dir), fs::directory_iterator{}));
}

std::string options(const std::string& projectFolderName = "", bool replace = false,
                    const std::string& worldID = "", const std::string& worldName = "") {
    JsonDoc o;
    if (!projectFolderName.empty()) { o.setString("projectFolderName", projectFolderName); }
    if (replace) { o.setBool("replaceProject", true); }
    if (!worldID.empty()) {
        JsonDoc names;
        names.setString(worldID, worldName);
        o.setSubDoc("worldFolderNames", std::move(names));
    }
    return o.dump(-1);
}

// Q9's left-out names, planted in the package: none may reach the archive.
const char* const kJunk[] = {".DS_Store", "._project.json", "indexes/search.index", ".cache/thumb.bin",
                             "manuscript/scene.tmp", "stale.lock", "Thumbs.db", "~$draft.md"};

struct ArchiveFixture {
    fs::path base, root, appSupport, worlds, dest, archivePath;
    std::string projectID, sceneID;

    explicit ArchiveFixture(const std::string& name = "Novel") {
        static std::atomic<int> counter{0};
        base = fs::temp_directory_path() / ("scrivi-archive-" + std::to_string(::getpid()) + "-" +
                                            std::to_string(counter.fetch_add(1)));
        fs::remove_all(base);
        root = base / "src" / (name + ".scrivi");
        archivePath = base / (name + ".scrivarchive");
        appSupport = base / "support";
        worlds = base / "worlds";
        dest = base / "dest";
        fs::create_directories(root);
        fs::create_directories(appSupport);
        fs::create_directories(worlds);
        fs::create_directories(dest);
        projectID = ok(scrivi_create_project(r(), as(), "Novel", "novel", "identity-001", "persona-001", "T"))
                        .getString("projectID");
        sceneID = ok(scrivi_open_project(r(), as(), "")).getSubDoc("activeScene").getString("sceneID");
        scrivi_free(scrivi_close_project(r()));
    }
    ~ArchiveFixture() {
        scrivi_free(scrivi_history_close(r()));
        scrivi_free(scrivi_close_project(r()));
        std::error_code ec;
        fs::remove_all(base, ec);
    }
    [[nodiscard]] const char* r() const { return root.c_str(); }
    [[nodiscard]] const char* as() const { return appSupport.c_str(); }
    [[nodiscard]] std::string archive() const { return archivePath.string(); }

    // Two typing events on the scene, so the history has something to undo.
    void recordHistory() {
        ok(scrivi_history_open(r()));
        ok(scrivi_history_record_event(r(), sceneID.c_str(), "Hello",
                                       R"({"kind":"typing","cursorBefore":0,"cursorAfter":5})"));
        ok(scrivi_history_record_event(r(), sceneID.c_str(), "Hello world",
                                       R"({"kind":"typing","cursorBefore":5,"cursorAfter":11})"));
        ok(scrivi_history_close(r()));
    }

    std::string makeWorld(const std::string& name) {
        return ok(scrivi_create_world(r(), (worlds / (name + ".scrivworld")).c_str(), name.c_str(), "The First Age"))
            .getString("worldID");
    }

    JsonDoc exportArchive(bool includeWorlds) {
        return ok(scrivi_export_archive(r(), archive().c_str(), includeWorlds ? 1 : 0));
    }
    const char* importRaw(const std::string& opts = "") {
        return scrivi_import_archive(archive().c_str(), dest.c_str(), opts.c_str());
    }
};

} // namespace

TEST_CASE("Archive: the round trip is byte-identical, keeps the ID, and the history replays (AC2, AC3)",
          "[archive][capi]") {
    ArchiveFixture f;
    f.recordHistory();
    std::string everyByte;
    for (int i = 0; i < 256; ++i) { everyByte.push_back(static_cast<char>(i)); }
    writeFile(f.root / "assets" / "blob.bin", everyByte);
    fs::create_directories(f.root / "inbox" / "empty-folder");
    const auto before = tree(f.root);
    for (const char* junk : kJunk) { writeFile(f.root / junk, "junk"); }

    auto exported = f.exportArchive(false);
    CHECK(exported.getString("archivePath") == f.archive());
    CHECK(exported.getInt64("files") == static_cast<int64_t>(before.size()));

    auto imported = ok(f.importRaw());
    REQUIRE(imported.getBool("imported"));
    const fs::path copy = imported.getString("projectRoot");
    CHECK(copy == f.dest / "Novel.scrivi");
    CHECK(imported.getString("projectID") == f.projectID);

    CHECK(tree(copy) == before);                               // every content file, byte for byte; no junk
    CHECK(fs::is_directory(copy / "inbox" / "empty-folder"));  // empty folders travel too
    CHECK(entriesIn(f.dest) == 1);                             // no staging folder left behind

    auto opened = ok(scrivi_open_project(copy.c_str(), f.as(), ""));
    CHECK(opened.getString("projectID") == f.projectID);
    scrivi_free(scrivi_close_project(copy.c_str()));

    ok(scrivi_history_open(copy.c_str()));
    auto undone = ok(scrivi_history_undo(copy.c_str()));
    REQUIRE(undone.arraySize("changes") == 1);
    CHECK(undone.arrayItem("changes", 0).getString("newText") == "Hello");
    auto redone = ok(scrivi_history_redo(copy.c_str()));
    CHECK(redone.arrayItem("changes", 0).getString("newText") == "Hello world");
    scrivi_free(scrivi_history_close(copy.c_str()));
}

TEST_CASE("Archive: a project name outside ASCII survives the round trip", "[archive][capi]") {
    ArchiveFixture f("Les Misérables — 第一部");
    const auto before = tree(f.root);
    f.exportArchive(false);
    auto imported = ok(f.importRaw());
    REQUIRE(imported.getBool("imported"));
    CHECK(fs::path(imported.getString("projectRoot")) == f.dest / "Les Misérables — 第一部.scrivi");
    CHECK(tree(imported.getString("projectRoot")) == before);
}

TEST_CASE("Archive: the queued writes land before the archive is written (AC2, Q9)", "[archive][capi]") {
    ArchiveFixture f;
    auto opened = ok(scrivi_open_project(f.r(), f.as(), ""));
    const auto scene = opened.getSubDoc("activeScene");
    JsonDoc req;
    req.setString("projectID", f.projectID);
    req.setString("appSupportRoot", f.appSupport.string());
    req.setString("identityID", "identity-001");
    req.setString("personaID", "persona-001");
    req.setString("displayName", "T");
    JsonDoc e;
    e.setString("sceneID", f.sceneID);
    e.setString("metadataPath", scene.getString("metadataPath"));
    e.setString("contentPath", scene.getString("contentPath"));
    e.setString("markdown", "Queued, not yet written.");
    req.appendToArray("scenes", std::move(e));
    ok(scrivi_enqueue_save_scenes(f.r(), req.dump(-1).c_str()));

    f.exportArchive(false);
    auto imported = ok(f.importRaw());
    CHECK(readFile(fs::path(imported.getString("projectRoot")) / scene.getString("contentPath")) ==
          "Queued, not yet written.");
}

TEST_CASE("Archive: behind a write that cannot land, the export refuses rather than archive a half-written state "
          "(AC2, Q9)", "[archive][capi]") {
    ArchiveFixture f;
    ok(scrivi_open_project(f.r(), f.as(), ""));
    JsonDoc req;
    req.setString("projectID", f.projectID);
    req.setString("appSupportRoot", f.appSupport.string());
    req.setString("identityID", "identity-001");
    req.setString("personaID", "persona-001");
    req.setString("displayName", "T");
    JsonDoc e;
    e.setString("sceneID", f.sceneID);
    e.setString("metadataPath", "gone/scene.meta.json");   // a folder that is not there: the save is HELD (EP-052)
    e.setString("contentPath", "gone/scene.md");
    e.setString("markdown", "Never lands.");
    req.appendToArray("scenes", std::move(e));
    ok(scrivi_enqueue_save_scenes(f.r(), req.dump(-1).c_str()));

    auto err = errorOf(scrivi_export_archive(f.r(), f.archive().c_str(), 0));
    CHECK(err.getString("detail") == "writeQueueHeld");
    CHECK_FALSE(fs::exists(f.archive()));
}

TEST_CASE("Archive: a project name already in the folder is Cancel / Rename / Replace, the ID kept (AC4, Q4)",
          "[archive][capi]") {
    ArchiveFixture f;
    f.exportArchive(false);
    ok(f.importRaw());
    const fs::path existing = f.dest / "Novel.scrivi";
    writeFile(existing / "written-after-import.md", "the writer's later work");

    SECTION("asked: nothing is extracted, the conflict is named") {
        auto r = ok(f.importRaw());
        CHECK_FALSE(r.getBool("imported"));
        CHECK(r.getSubDoc("conflict").getString("kind") == "project");
        CHECK(r.getSubDoc("conflict").getString("folderName") == "Novel.scrivi");
        CHECK(entriesIn(f.dest) == 1);
        CHECK(fs::exists(existing / "written-after-import.md"));
    }
    SECTION("Rename: unpacked under the new name, the same project ID") {
        auto r = ok(f.importRaw(options("Novel-2")));
        REQUIRE(r.getBool("imported"));
        CHECK(fs::path(r.getString("projectRoot")) == f.dest / "Novel-2.scrivi");
        CHECK(r.getString("projectID") == f.projectID);
        CHECK(fs::exists(existing / "written-after-import.md"));
    }
    SECTION("Replace: the old project is gone, the archive's is in its place") {
        auto r = ok(f.importRaw(options("", true)));
        REQUIRE(r.getBool("imported"));
        CHECK_FALSE(fs::exists(existing / "written-after-import.md"));
        CHECK(fs::exists(existing / "project.json"));
        CHECK(entriesIn(f.dest) == 1);   // nothing set aside or staged is left
    }
    SECTION("Replace is refused while that project is open") {
        ok(scrivi_history_open(existing.c_str()));
        auto err = errorOf(f.importRaw(options("", true)));
        CHECK(err.getString("detail") == "projectOpen");
        CHECK(fs::exists(existing / "written-after-import.md"));
        scrivi_free(scrivi_history_close(existing.c_str()));
    }
    SECTION("Replace aimed at a folder that is not a project is refused") {
        fs::remove_all(existing);
        writeFile(existing / "notes.txt", "not a project");
        auto err = errorOf(f.importRaw(options("", true)));
        CHECK(err.getString("detail") == "notAProject");
        CHECK(fs::exists(existing / "notes.txt"));
    }
}

TEST_CASE("Archive: worlds travel, and a world already here is bound, not copied (AC5, Q8)", "[archive][capi]") {
    ArchiveFixture f;
    const auto worldID = f.makeWorld("Eskandar");
    const fs::path original = f.worlds / "Eskandar.scrivworld";

    SECTION("without worlds: none is carried") {
        auto exported = f.exportArchive(false);
        CHECK(exported.arraySize("worlds") == 0);
    }
    SECTION("the world is still where the binding saw it: bound there, nothing unpacked") {
        auto exported = f.exportArchive(true);
        REQUIRE(exported.arraySize("worlds") == 1);
        CHECK(exported.arrayItem("worlds", 0).getString("folder") == "Eskandar.scrivworld");
        auto r = ok(f.importRaw());
        REQUIRE(r.arraySize("worlds") == 1);
        CHECK_FALSE(r.arrayItem("worlds", 0).getBool("unpacked"));
        CHECK(fs::path(r.arrayItem("worlds", 0).getString("packagePath")) == original);
        CHECK_FALSE(fs::exists(f.dest / "Eskandar.scrivworld"));
    }
    SECTION("the world is not on this machine: unpacked beside the project and bound") {
        f.exportArchive(true);
        fs::rename(original, f.base / "elsewhere.scrivworld");   // gone from where the binding saw it
        auto r = ok(f.importRaw());
        REQUIRE(r.arraySize("worlds") == 1);
        CHECK(r.arrayItem("worlds", 0).getBool("unpacked"));
        const auto copy = r.getString("projectRoot");
        auto status = ok(scrivi_get_world_status(copy.c_str(), worldID.c_str()));
        CHECK(status.getString("status") == "available");
        CHECK(fs::canonical(status.getString("packagePath")) == fs::canonical(f.dest / "Eskandar.scrivworld"));
        CHECK_FALSE(fs::exists(f.dest / "Eskandar.scrivworld" / ".lock"));
    }
    SECTION("a DIFFERENT world holds the name: asked (no Replace), then Rename") {
        f.exportArchive(true);
        fs::rename(original, f.base / "elsewhere.scrivworld");
        ArchiveFixture other;   // a world of the same name, another world ID, in the destination folder
        ok(scrivi_create_world(other.r(), (f.dest / "Eskandar.scrivworld").c_str(), "Eskandar", "Another Age"));

        auto asked = ok(f.importRaw());
        CHECK_FALSE(asked.getBool("imported"));
        CHECK(asked.getSubDoc("conflict").getString("kind") == "world");
        CHECK(asked.getSubDoc("conflict").getString("worldID") == worldID);
        CHECK(entriesIn(f.dest) == 1);   // nothing extracted, not even the project

        auto r = ok(f.importRaw(options("", false, worldID, "Eskandar (Novel)")));
        REQUIRE(r.getBool("imported"));
        CHECK(fs::path(r.arrayItem("worlds", 0).getString("packagePath")) ==
              f.dest / "Eskandar (Novel).scrivworld");
        auto status = ok(scrivi_get_world_status(r.getString("projectRoot").c_str(), worldID.c_str()));
        CHECK(status.getString("status") == "available");
    }
}

TEST_CASE("Archive: the Mac's trailing slash, and a removed world's leftover folder, export cleanly (SP-206 live pass)",
          "[archive][capi]") {
    ArchiveFixture f;
    f.makeWorld("Eskandar");
    fs::create_directories(f.root / "worlds" / "world_character_removed");   // Remove Reference keeps the folder

    const auto slashed = f.root.string() + "/";
    auto exported = ok(scrivi_export_archive(slashed.c_str(), f.archive().c_str(), 1));
    REQUIRE(exported.arraySize("worlds") == 1);
    CHECK(exported.arrayItem("worlds", 0).getString("folder") == "Eskandar.scrivworld");
    CHECK(exported.arraySize("skippedWorlds") == 0);

    auto imported = ok(f.importRaw());
    REQUIRE(imported.getBool("imported"));
    CHECK(fs::path(imported.getString("projectRoot")) == f.dest / "Novel.scrivi");
}

TEST_CASE("Archive: every rejection leaves nothing behind (AC6)", "[archive][capi]") {
    ArchiveFixture f;

    SECTION("not a zip") {
        writeFile(f.archive(), "plain text");
        CHECK(errorOf(f.importRaw()).getString("detail") == "notAnArchive");
    }
    SECTION("a zip with no table of contents") {
        mz_zip_archive z{};
        REQUIRE(mz_zip_writer_init_file(&z, f.archive().c_str(), 0));
        REQUIRE(mz_zip_writer_add_mem(&z, "Novel.scrivi/project.json", "{}", 2, 0));
        REQUIRE(mz_zip_writer_finalize_archive(&z));
        mz_zip_writer_end(&z);
        CHECK(errorOf(f.importRaw()).getString("detail") == "notAnArchive");
    }
    SECTION("an entry that would leave the folder") {
        f.exportArchive(false);
        // Rewrite the archive with one hostile entry added.
        mz_zip_archive in{}, out{};
        const auto hostile = (f.base / "hostile.scrivarchive").string();
        REQUIRE(mz_zip_reader_init_file(&in, f.archive().c_str(), 0));
        REQUIRE(mz_zip_writer_init_file_v2(&out, hostile.c_str(), 0, MZ_ZIP_FLAG_WRITE_ZIP64));
        for (mz_uint i = 0; i < mz_zip_reader_get_num_files(&in); ++i) {
            REQUIRE(mz_zip_writer_add_from_zip_reader(&out, &in, i));
        }
        REQUIRE(mz_zip_writer_add_mem(&out, "Novel.scrivi/../../escaped.txt", "x", 1, 0));
        REQUIRE(mz_zip_writer_finalize_archive(&out));
        mz_zip_writer_end(&out);
        mz_zip_reader_end(&in);
        fs::rename(hostile, f.archive());
        CHECK(errorOf(f.importRaw()).getString("detail") == "unsafeEntry");
        CHECK_FALSE(fs::exists(f.base / "escaped.txt"));
    }
    SECTION("a Rename that is not a folder name") {
        f.exportArchive(false);
        CHECK(errorOf(f.importRaw(options("../Novel"))).getString("detail") == "badFolderName");
    }
    CHECK(entriesIn(f.dest) == 0);
}

TEST_CASE("Archive: a failed export leaves no file (AC2)", "[archive][capi]") {
    ArchiveFixture f;
    SECTION("the destination folder does not exist") {
        const auto target = f.base / "no-such-folder" / "Novel.scrivarchive";
        errorOf(scrivi_export_archive(f.r(), target.c_str(), 0));
        CHECK_FALSE(fs::exists(target.parent_path()));
    }
    SECTION("the destination is inside the project") {
        const auto target = f.root / "Novel.scrivarchive";
        errorOf(scrivi_export_archive(f.r(), target.c_str(), 0));
        CHECK_FALSE(fs::exists(target));
    }
}
