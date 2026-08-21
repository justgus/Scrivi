// EP-034 SP-116 — world-scoped assets, asset path resolution, and the kind-scope
// endpoint AT THE C ABI.
//
// ⚠️ WHY THIS FILE EXISTS, and why AssetTests.cpp could not host these tests.
//
// AssetTests.cpp calls the C++ facade (core.importAsset(...)) directly. That is
// exactly the shape that let I-0113 ship green: SP-097 widened the requests and
// the facade tests passed, while the scrivi_* entry points were never widened,
// so the new field was permanently empty at the boundary and the feature was
// unreachable through the ABI entirely.
//
// Every assertion here therefore goes through scrivi_*. A facade test cannot see
// a boundary gap, by construction (`feedback_boundary_tests_not_facade`).

#include <catch2/catch_test_macros.hpp>

#include "scrivi/scrivi.h"
#include "scrivi/ObjectTypes.hpp"
#include "util/Json.hpp"
#include "worlds/WorldStore.hpp"
#include "platform/LocalFileSystem.hpp"
#include "mocks/DeterministicUUIDProvider.hpp"

#include <algorithm>
#include <atomic>
#include <cstdint>
#include <filesystem>
#include <fstream>
#include <iterator>
#include <chrono>
#include <ctime>
#include <optional>
#include <string>
#include <vector>

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

JsonDoc okResult(const char* raw) {
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

struct AssetCApiFixture {
    fs::path    projectDir;
    fs::path    appSupportDir;
    fs::path    worldsRoot;
    fs::path    sourceDir;      // where synthetic import sources are written
    std::string projectID;

    AssetCApiFixture() {
        static std::atomic<int> counter{0};
        const std::string stem =
            "scrivi-asset-capi-" + std::to_string(counter.fetch_add(1)) + "-" +
            std::to_string(reinterpret_cast<std::uintptr_t>(this));

        projectDir    = fs::temp_directory_path() / (stem + ".scrivi");
        appSupportDir = fs::temp_directory_path() / (stem + "-appsupport");
        worldsRoot    = fs::temp_directory_path() / (stem + "-worlds");
        sourceDir     = fs::temp_directory_path() / (stem + "-src");

        fs::create_directories(projectDir);
        fs::create_directories(appSupportDir);
        fs::create_directories(worldsRoot);
        fs::create_directories(sourceDir);

        auto created = okResult(scrivi_create_project(root(), appSupport(), "Asset C API",
                                                      "asset-capi",
                                                      "identity-001", "persona-001",
                                                      "Test Author"));
        projectID = created.getString("projectID");
    }

    ~AssetCApiFixture() {
        std::error_code ec;
        fs::remove_all(projectDir, ec);
        fs::remove_all(appSupportDir, ec);
        fs::remove_all(worldsRoot, ec);
        fs::remove_all(sourceDir, ec);
    }

    [[nodiscard]] const char* root() const { return projectDir.c_str(); }
    [[nodiscard]] const char* appSupport() const { return appSupportDir.c_str(); }

    [[nodiscard]] std::string pkg(const std::string& name) const {
        return (worldsRoot / (name + ".scrivworld")).string();
    }

    std::string makeWorld(const std::string& name = "Eskandar") {
        auto r = okResult(scrivi_create_world(root(), pkg(name).c_str(),
                                              name.c_str(), "The First Age"));
        auto id = r.getString("worldID");
        REQUIRE_FALSE(id.empty());
        return id;
    }

    // Writes a synthetic file to import. `name` may contain characters that are
    // legal in a filename but hostile to hand-built JSON — that is the point.
    std::string writeSource(const std::string& name, const std::string& bytes) {
        auto p = sourceDir / name;
        std::ofstream out(p, std::ios::binary);
        out << bytes;
        out.close();
        return p.string();
    }

    void detachWorld(const std::string& name = "Eskandar") {
        fs::rename(pkg(name), pkg(name + "-detached"));
    }
};



// The C ABI stamps lock heartbeats with the real system clock; a competing
// writer must do the same or its lock reads as stale and is broken on sight.
struct RealClock final : public scrivi::Clock {
    scrivi::ISO8601Timestamp nowUTC() override {
        auto now = std::chrono::system_clock::now();
        auto tt  = std::chrono::system_clock::to_time_t(now);
        std::tm tm{};
        gmtime_r(&tt, &tm);
        char buf[32];
        std::strftime(buf, sizeof(buf), "%Y-%m-%dT%H:%M:%SZ", &tm);
        return scrivi::ISO8601Timestamp{buf};
    }
};

// Holds the world package lock the way ANOTHER PROCESS would, so a write can be
// observed refusing. Uses its own services — the C ABI's singleton is not
// reachable from here, and that is the point: this is a second writer.
struct CompetingWriter {
    scrivi::platform::LocalFileSystem fs;
    // ⚠️ A REAL clock, not FixedClock. The C ABI stamps locks with the real time,
    // and WorldLock breaks any lock whose heartbeat is more than kStaleSeconds
    // old — so a fixed 2026-08-21T00:00:00Z heartbeat reads as long dead, the
    // ABI legitimately breaks it, and the write succeeds. The first version of
    // this test did exactly that and "failed", which looked like a broken guard
    // and was in fact a broken RIG: the product was behaving correctly.
    RealClock                                clock;
    scrivi::mocks::DeterministicUUIDProvider uuid;
    scrivi::CoreServices              services{};
    std::optional<scrivi::worlds::WorldLock> lock;

    explicit CompetingWriter(const std::string& packagePath) {
        services.fileSystem   = &fs;
        services.clock        = &clock;
        services.uuidProvider = &uuid;
        lock.emplace(services, packagePath);
        REQUIRE(lock->acquire("some-other-project").ok());
    }
};

// Finds the listed asset with this assetID, or a null doc's empty strings.
JsonDoc findAsset(const JsonDoc& listResult, const std::string& assetID) {
    const auto n = listResult.arraySize("assets");
    for (std::size_t i = 0; i < n; ++i) {
        auto item = listResult.arrayItem("assets", i);
        if (item.getString("assetID") == assetID) { return item; }
    }
    FAIL("assetID not found in list envelope: " << listResult.dump());
    return JsonDoc{};
}

} // namespace

// ---------------------------------------------------------------------------
// S1 / S2 — D6: a world's assets live in the WORLD package and travel with it
// ---------------------------------------------------------------------------

TEST_CASE("C ABI: a world-scoped asset is written into the world package, not the project",
          "[integration][SP-116][T-0426][S1]") {
    AssetCApiFixture fix;
    const auto worldID = fix.makeWorld();
    const auto src     = fix.writeSource("mara.png", "FAKE_PNG_BYTES");

    auto imported = okResult(scrivi_import_asset(
        fix.root(), src.c_str(), "image", "Mara's Portrait",
        "identity-001", "persona-001", "Test Author",
        worldID.c_str(), fix.projectID.c_str()));

    const auto assetPath = imported.getString("assetPath");
    REQUIRE_FALSE(assetPath.empty());

    // The bytes are inside the .scrivworld package...
    REQUIRE(assetPath.find(".scrivworld") != std::string::npos);
    REQUIRE(fs::exists(assetPath));

    // ...and NOT under the project. This is the whole point of D6: an image
    // stored in the project cannot travel with a shared world.
    REQUIRE(assetPath.find(fix.projectDir.string()) == std::string::npos);
    REQUIRE_FALSE(fs::exists(fix.projectDir / "assets" / "images" / "mara.png"));
}

TEST_CASE("C ABI: a world's asset is visible from a DIFFERENT project that binds the same world",
          "[integration][SP-116][T-0426][S2]") {
    // ⚠️ THIS is the criterion D6 exists for. S1 alone does not prove it: bytes
    // can sit in the right directory and still be unreachable from the second
    // project if the list path resolves against the wrong root.
    AssetCApiFixture first;
    const auto worldID = first.makeWorld("Shared");
    const auto src     = first.writeSource("map.png", "FAKE_MAP_BYTES");

    auto imported = okResult(scrivi_import_asset(
        first.root(), src.c_str(), "image", "The Sundered Coast",
        "identity-001", "persona-001", "Test Author",
        worldID.c_str(), first.projectID.c_str()));
    const auto assetID = imported.getString("assetID");

    // A second, entirely separate project binds the SAME world package.
    AssetCApiFixture second;
    auto bound = okResult(scrivi_add_world(second.root(), first.pkg("Shared").c_str()));
    const auto secondWorldID = bound.getString("worldID");
    REQUIRE(secondWorldID == worldID);   // identity travels with the package

    auto listed = okResult(scrivi_list_assets(second.root(), "", secondWorldID.c_str()));
    REQUIRE(listed.getInt("count") == 1);

    auto item = findAsset(listed, assetID);
    REQUIRE(item.getString("title") == "The Sundered Coast");

    // And the path it reports from the second project actually resolves.
    const auto path = item.getString("assetPath");
    REQUIRE_FALSE(path.empty());
    REQUIRE(fs::exists(path));
}

TEST_CASE("C ABI: an empty worldID still means the project — pre-D6 callers are unaffected",
          "[integration][SP-116][T-0426]") {
    AssetCApiFixture fix;
    const auto src = fix.writeSource("notes.pdf", "FAKE_PDF");

    auto imported = okResult(scrivi_import_asset(
        fix.root(), src.c_str(), "document", "Research Notes",
        "identity-001", "persona-001", "Test Author",
        "", ""));

    const auto assetPath = imported.getString("assetPath");
    REQUIRE(assetPath.find(fix.projectDir.string()) != std::string::npos);
    REQUIRE(assetPath.find(".scrivworld") == std::string::npos);

    auto listed = okResult(scrivi_list_assets(fix.root(), "", ""));
    REQUIRE(listed.getInt("count") == 1);
}

// ---------------------------------------------------------------------------
// S3 — refusal must be honest, and must write NOTHING
// ---------------------------------------------------------------------------

TEST_CASE("C ABI: importing into an unavailable world refuses with worldUnavailable and writes nothing",
          "[integration][SP-116][T-0426][S3]") {
    AssetCApiFixture fix;
    const auto worldID = fix.makeWorld();
    const auto src     = fix.writeSource("ghost.png", "FAKE_PNG");

    fix.detachWorld();   // the writer ejected the drive / moved the folder

    auto err = errorOf(scrivi_import_asset(
        fix.root(), src.c_str(), "image", "Ghost",
        "identity-001", "persona-001", "Test Author",
        worldID.c_str(), fix.projectID.c_str()));

    // ⚠️ Byte-identical to ObjectStore::kindDirFor's contract, so the app needs
    // ONE handler for both surfaces rather than two that can drift apart.
    const auto detail = err.getString("detail");
    REQUIRE(detail.rfind("worldUnavailable:", 0) == 0);

    // Nothing was written anywhere — no orphaned bytes, no orphaned sidecar.
    REQUIRE_FALSE(fs::exists(fix.projectDir / "assets" / "images" / "ghost.png"));
    REQUIRE_FALSE(fs::exists(fs::path(fix.pkg("Eskandar")) / "assets"));
}

TEST_CASE("C ABI: listing and removing an unavailable world's assets refuse the same way",
          "[integration][SP-116][T-0426][S3]") {
    AssetCApiFixture fix;
    const auto worldID = fix.makeWorld();
    fix.detachWorld();

    auto listErr = errorOf(scrivi_list_assets(fix.root(), "", worldID.c_str()));
    REQUIRE(listErr.getString("detail").rfind("worldUnavailable:", 0) == 0);

    auto rmErr = errorOf(scrivi_remove_asset(fix.root(), "any-asset-id",
                                             worldID.c_str(), fix.projectID.c_str()));
    REQUIRE(rmErr.getString("detail").rfind("worldUnavailable:", 0) == 0);
}

TEST_CASE("C ABI: a world asset can be removed, and the bytes and sidecar both go",
          "[integration][SP-116][T-0426]") {
    AssetCApiFixture fix;
    const auto worldID = fix.makeWorld();
    const auto src     = fix.writeSource("doomed.png", "FAKE_PNG");

    auto imported = okResult(scrivi_import_asset(
        fix.root(), src.c_str(), "image", "Doomed",
        "identity-001", "persona-001", "Test Author",
        worldID.c_str(), fix.projectID.c_str()));
    const auto assetID   = imported.getString("assetID");
    const auto assetPath = imported.getString("assetPath");
    const auto sidecar   = imported.getString("sidecarPath");
    REQUIRE(fs::exists(assetPath));
    REQUIRE(fs::exists(sidecar));

    auto removed = okResult(scrivi_remove_asset(fix.root(), assetID.c_str(),
                                                worldID.c_str(), fix.projectID.c_str()));
    REQUIRE(removed.getBool("deleted"));
    REQUIRE_FALSE(fs::exists(assetPath));
    REQUIRE_FALSE(fs::exists(sidecar));
}

// ---------------------------------------------------------------------------
// S5 / S6 — D7's path, and I-0143's escaping
// ---------------------------------------------------------------------------

TEST_CASE("C ABI: list_assets emits a resolvable assetPath for every asset",
          "[integration][SP-116][T-0427][S5]") {
    AssetCApiFixture fix;
    const auto src = fix.writeSource("cover.png", "FAKE_PNG");

    auto imported = okResult(scrivi_import_asset(
        fix.root(), src.c_str(), "image", "Cover",
        "identity-001", "persona-001", "Test Author", "", ""));
    const auto assetID    = imported.getString("assetID");
    const auto importPath = imported.getString("assetPath");

    auto listed = okResult(scrivi_list_assets(fix.root(), "", ""));
    auto item   = findAsset(listed, assetID);

    // The path was disclosed at import and then never again until T-0427.
    // It must agree with import's, and it must actually exist.
    REQUIRE(item.getString("assetPath") == importPath);
    REQUIRE(fs::exists(item.getString("assetPath")));
}

TEST_CASE("C ABI: a title containing quotes and backslashes survives list_assets intact",
          "[integration][SP-116][T-0428][I-0143][S6]") {
    // ⚠️ I-0143: the assets array was built by string concatenation and escaped
    // nothing, so this exact input produced a malformed envelope. A test using
    // tame ASCII proves nothing here — the whole defect is about the characters
    // JSON reserves.
    AssetCApiFixture fix;
    const auto src = fix.writeSource("quoted.png", "FAKE_PNG");

    const std::string nastyTitle = R"(The "Sundered" Coast \ Vol. 2)";

    auto imported = okResult(scrivi_import_asset(
        fix.root(), src.c_str(), "image", nastyTitle.c_str(),
        "identity-001", "persona-001", "Test Author", "", ""));
    const auto assetID = imported.getString("assetID");

    // If the envelope were malformed, okResult's parse would already have failed.
    auto listed = okResult(scrivi_list_assets(fix.root(), "", ""));
    auto item   = findAsset(listed, assetID);

    // Round-trips byte for byte — not merely "parses".
    REQUIRE(item.getString("title") == nastyTitle);
}

TEST_CASE("C ABI: a filename containing a quote survives list_assets intact",
          "[integration][SP-116][T-0428][I-0143][S6]") {
    // The other half of the same defect. A filename reaches the envelope from
    // the filesystem rather than from a text field, so it is not covered by the
    // title case above.
    AssetCApiFixture fix;
    const std::string nastyName = R"(a"b.png)";
    const auto src = fix.writeSource(nastyName, "FAKE_PNG");

    auto imported = okResult(scrivi_import_asset(
        fix.root(), src.c_str(), "image", "Odd Name",
        "identity-001", "persona-001", "Test Author", "", ""));
    const auto assetID = imported.getString("assetID");

    auto listed = okResult(scrivi_list_assets(fix.root(), "", ""));
    auto item   = findAsset(listed, assetID);
    REQUIRE(item.getString("filename") == nastyName);
    REQUIRE(fs::exists(item.getString("assetPath")));
}

TEST_CASE("C ABI: listing a project with no assets returns an empty list, not a malformed one",
          "[integration][SP-116][T-0427][T-0428]") {
    // ⚠️ The zero case is where an array-building bug hides: `appendToArray` is
    // never called, so the key can be absent rather than empty. A decoder that
    // tolerates the absence would mask it — assert the envelope itself.
    AssetCApiFixture fix;

    auto listed = okResult(scrivi_list_assets(fix.root(), "", ""));
    REQUIRE(listed.getInt("count") == 0);
    REQUIRE(listed.arraySize("assets") == 0);
}

TEST_CASE("C ABI: re-importing the same filename does not destroy the first copy's bytes",
          "[integration][SP-116][T-0426]") {
    // ⚠️ Guards the rollback bug found in self-review. atomicWriteTextFile is
    // temp-write → rename, so the second import OVERWRITES; a rollback that
    // deleted destPath unconditionally would take out bytes the FIRST asset's
    // sidecar still points at. Both sidecars must survive with readable bytes.
    AssetCApiFixture fix;
    const auto src = fix.writeSource("same.png", "FAKE_PNG");

    auto first = okResult(scrivi_import_asset(
        fix.root(), src.c_str(), "image", "First",
        "identity-001", "persona-001", "Test Author", "", ""));
    auto second = okResult(scrivi_import_asset(
        fix.root(), src.c_str(), "image", "Second",
        "identity-001", "persona-001", "Test Author", "", ""));

    REQUIRE(first.getString("assetID") != second.getString("assetID"));
    REQUIRE(fs::exists(first.getString("assetPath")));
    REQUIRE(fs::exists(second.getString("assetPath")));
}

// ---------------------------------------------------------------------------
// S12 — I-0144: world-package writes actually TAKE THE LOCK
//
// ⚠️ These are the tests whose ABSENCE was I-0144. WorldLock was complete,
// correct and unit-tested for three sprints while NOTHING CALLED IT — so a test
// exercising the lock CLASS proves nothing about the product. The only thing
// that does is holding the lock and watching a real write refuse.
// ---------------------------------------------------------------------------

TEST_CASE("C ABI: creating a world object while the package is locked is REFUSED",
          "[integration][SP-116][T-0431][I-0144][S12]") {
    AssetCApiFixture fix;
    const auto worldID = fix.makeWorld();
    CompetingWriter other{fix.pkg("Eskandar")};

    auto err = errorOf(scrivi_create_object(
        fix.root(), "character", "Mara", "",
        "identity-001", "persona-001", "Test Author", worldID.c_str()));
    REQUIRE(err.getString("detail") == "worldLocked");
}

TEST_CASE("C ABI: deleting a world object while the package is locked is REFUSED",
          "[integration][SP-116][T-0431][I-0144][S12]") {
    AssetCApiFixture fix;
    const auto worldID = fix.makeWorld();
    auto created = okResult(scrivi_create_object(
        fix.root(), "character", "Vance", "",
        "identity-001", "persona-001", "Test Author", worldID.c_str()));

    CompetingWriter other{fix.pkg("Eskandar")};

    auto err = errorOf(scrivi_delete_object(
        fix.root(), "character", created.getString("objectID").c_str(),
        worldID.c_str()));
    REQUIRE(err.getString("detail") == "worldLocked");
}

TEST_CASE("C ABI: importing a world ASSET while the package is locked is REFUSED",
          "[integration][SP-116][T-0426][I-0144][S12]") {
    AssetCApiFixture fix;
    const auto worldID = fix.makeWorld();
    const auto src     = fix.writeSource("blocked.png", "FAKE_PNG");
    CompetingWriter other{fix.pkg("Eskandar")};

    auto err = errorOf(scrivi_import_asset(
        fix.root(), src.c_str(), "image", "Blocked",
        "identity-001", "persona-001", "Test Author",
        worldID.c_str(), fix.projectID.c_str()));
    REQUIRE(err.getString("detail") == "worldLocked");

    // ⚠️ And it wrote NOTHING — a refused write must not leave bytes behind.
    REQUIRE_FALSE(fs::exists(fs::path(fix.pkg("Eskandar")) / "assets" / "images" / "blocked.png"));
}

TEST_CASE("C ABI: a PROJECT-scoped write is unaffected by any world lock",
          "[integration][SP-116][T-0431][I-0144][S12]") {
    // ⚠️ The guard must be INERT for project writes. If it were not, holding one
    // world's lock would block writing a `source` — a project object with
    // nothing to do with that world.
    AssetCApiFixture fix;
    (void)fix.makeWorld();
    CompetingWriter other{fix.pkg("Eskandar")};

    auto res = okResult(scrivi_create_object(
        fix.root(), "source", "Field Notes", "",
        "identity-001", "persona-001", "Test Author", ""));
    REQUIRE_FALSE(res.getString("objectID").empty());
}

TEST_CASE("C ABI: the lock is RELEASED after every successful world write",
          "[integration][SP-116][T-0431][I-0144][S12]") {
    // ⚠️ A guard that never released would make the SECOND write of any session
    // fail — worse than the missing lock it replaced. Three writes in a row, then
    // no lock file left behind.
    AssetCApiFixture fix;
    const auto worldID = fix.makeWorld();

    for (int i = 0; i < 3; ++i) {
        auto r = okResult(scrivi_create_object(
            fix.root(), "character", ("Character " + std::to_string(i)).c_str(), "",
            "identity-001", "persona-001", "Test Author", worldID.c_str()));
        REQUIRE_FALSE(r.getString("objectID").empty());
    }

    REQUIRE_FALSE(fs::exists(fs::path(fix.pkg("Eskandar")) / ".lock"));
}

// ---------------------------------------------------------------------------
// S13 — T-0432: block transfer, the per-block watchdog, and partial cleanup
// ---------------------------------------------------------------------------

TEST_CASE("an asset spanning many blocks copies byte-for-byte",
          "[integration][SP-116][T-0432][S13]") {
    // ⚠️ Every other asset test uses a payload smaller than one block, so none
    // of them exercises the loop at all. This one crosses it several times.
    AssetCApiFixture fix;

    std::string big;
    big.reserve(5u << 20);
    for (std::size_t i = 0; i < (5u << 20); ++i) {
        big.push_back(static_cast<char>(i % 251));   // non-repeating, includes NULs
    }
    const auto src = fix.writeSource("big.bin", big);

    auto imported = okResult(scrivi_import_asset(
        fix.root(), src.c_str(), "image", "Big Map",
        "identity-001", "persona-001", "Test Author", "", ""));

    std::ifstream in(imported.getString("assetPath"), std::ios::binary);
    std::string copied((std::istreambuf_iterator<char>(in)),
                        std::istreambuf_iterator<char>());
    REQUIRE(copied.size() == big.size());
    REQUIRE(copied == big);   // ⚠️ byte-exact, NULs and all
}

TEST_CASE("copyFileInBlocks kicks the watchdog once per block",
          "[integration][SP-116][T-0432][S13]") {
    // The callback IS the watchdog kick. If the loop stopped calling it, a long
    // transfer would silently go back to risking its lock — invisible from the
    // outside, which is why this asserts the count directly.
    scrivi::platform::LocalFileSystem fs;

    auto dir = fs::temp_directory_path() / "scrivi-block-kick";
    fs::create_directories(dir);
    const auto from = (dir / "src.bin").string();
    const auto to   = (dir / "dst.bin").string();
    {
        std::ofstream out(from, std::ios::binary);
        out << std::string(4096, 'x');
    }

    int kicks = 0;
    auto r = fs.copyFileInBlocks(from, to, 1024, [&]() -> scrivi::Result<void> {
        ++kicks;
        return scrivi::Result<void>::success();
    });
    REQUIRE(r.ok());
    REQUIRE(kicks == 4);            // 4096 bytes / 1024-byte blocks
    REQUIRE(fs::exists(to));

    std::error_code ec;
    fs::remove_all(dir, ec);
}

TEST_CASE("a transfer aborted by the watchdog leaves NO destination and NO partial",
          "[integration][SP-116][T-0432][S13]") {
    // ⚠️ This is the case the destExisted rollback could never cover: the
    // transfer stops mid-flight because the lock was lost. Nothing may survive
    // — not the destination, and not the .partial temporary, which would
    // otherwise sit in a shared world forever, invisible to list().
    scrivi::platform::LocalFileSystem fs;

    auto dir = fs::temp_directory_path() / "scrivi-block-abort";
    fs::create_directories(dir);
    const auto from = (dir / "src.bin").string();
    const auto to   = (dir / "dst.bin").string();
    {
        std::ofstream out(from, std::ios::binary);
        out << std::string(8192, 'y');
    }

    int kicks = 0;
    auto r = fs.copyFileInBlocks(from, to, 1024, [&]() -> scrivi::Result<void> {
        if (++kicks == 3) {
            return scrivi::Result<void>::failure(
                {.code = scrivi::ErrorCode::ioError, .message = "lock lost"});
        }
        return scrivi::Result<void>::success();
    });

    REQUIRE_FALSE(r.ok());
    REQUIRE(r.error().message == "lock lost");
    REQUIRE_FALSE(fs::exists(to));                    // no destination
    REQUIRE_FALSE(fs::exists(to + ".partial"));       // and no leftover temp

    std::error_code ec;
    fs::remove_all(dir, ec);
}

TEST_CASE("a failed transfer never leaves a readable partial at the destination",
          "[integration][SP-116][T-0432][S13]") {
    // The destination must not exist in a half-written state at ANY point a
    // reader could observe: list() would not show it (no sidecar), but a later
    // import of the same filename would find bytes it did not write.
    scrivi::platform::LocalFileSystem fs;

    auto dir = fs::temp_directory_path() / "scrivi-block-partial";
    fs::create_directories(dir);
    const auto from = (dir / "src.bin").string();
    const auto to   = (dir / "dst.bin").string();
    {
        std::ofstream out(from, std::ios::binary);
        out << std::string(4096, 'z');
    }

    auto r = fs.copyFileInBlocks(from, to, 512, [&]() -> scrivi::Result<void> {
        // Assert mid-flight that the destination has not appeared yet.
        REQUIRE_FALSE(fs::exists(to));
        return scrivi::Result<void>::success();
    });
    REQUIRE(r.ok());
    REQUIRE(fs::exists(to));

    std::error_code ec;
    fs::remove_all(dir, ec);
}

// ---------------------------------------------------------------------------
// S14 — I-0146: abandoned .partial files are swept by the next lock holder
//
// ⚠️ WHY THESE EXIST. The lab test "an aborted transfer leaves no destination
// and no partial" PASSES and always did — because in a lab the filesystem is
// still there to clean up with. On the real rig, a USB drive pulled mid-import
// left 459 MB of `.partial` inside a shared world: the rollback could not run,
// because the failure WAS the filesystem going away.
//
// These reproduce the surviving orphan directly rather than trying to stage a
// vanishing volume, and assert that acquiring the lock reclaims it.
// ---------------------------------------------------------------------------

TEST_CASE("acquiring a world lock sweeps an abandoned .partial",
          "[integration][SP-116][T-0433][I-0146][S14]") {
    AssetCApiFixture fix;
    const auto worldID = fix.makeWorld();
    const auto src     = fix.writeSource("seed.png", "FAKE_PNG");

    // One real import, so the assets/images directory exists as it would in life.
    (void)okResult(scrivi_import_asset(
        fix.root(), src.c_str(), "image", "Seed",
        "identity-001", "persona-001", "Test Author",
        worldID.c_str(), fix.projectID.c_str()));

    // The orphan a dead writer leaves behind.
    const auto orphan = fs::path(fix.pkg("Eskandar")) / "assets" / "images" /
                        "abandoned.png.partial";
    { std::ofstream o(orphan, std::ios::binary); o << std::string(4096, 'p'); }
    REQUIRE(fs::exists(orphan));

    // ANY subsequent world write acquires the lock — and the sweep runs there.
    const auto src2 = fix.writeSource("next.png", "FAKE_PNG");
    (void)okResult(scrivi_import_asset(
        fix.root(), src2.c_str(), "image", "Next",
        "identity-001", "persona-001", "Test Author",
        worldID.c_str(), fix.projectID.c_str()));

    REQUIRE_FALSE(fs::exists(orphan));   // reclaimed
}

TEST_CASE("the sweep reclaims partials across every asset category",
          "[integration][SP-116][T-0433][I-0146][S14]") {
    // ⚠️ Categories are iterated from assetCategorySubdir, not written out, so a
    // new category cannot silently go unswept. This asserts that breadth.
    AssetCApiFixture fix;
    const auto worldID = fix.makeWorld();

    const auto seed = fix.writeSource("seed.png", "FAKE_PNG");
    (void)okResult(scrivi_import_asset(
        fix.root(), seed.c_str(), "image", "Seed",
        "identity-001", "persona-001", "Test Author",
        worldID.c_str(), fix.projectID.c_str()));

    std::vector<fs::path> orphans;
    for (const char* sub : {"images", "audio", "video", "documents", "other"}) {
        auto dir = fs::path(fix.pkg("Eskandar")) / "assets" / sub;
        fs::create_directories(dir);
        auto p = dir / "ghost.bin.partial";
        { std::ofstream o(p, std::ios::binary); o << "junk"; }
        orphans.push_back(p);
    }

    const auto next = fix.writeSource("next2.png", "FAKE_PNG");
    (void)okResult(scrivi_import_asset(
        fix.root(), next.c_str(), "image", "Next",
        "identity-001", "persona-001", "Test Author",
        worldID.c_str(), fix.projectID.c_str()));

    for (const auto& p : orphans) {
        INFO("still present: " << p.string());
        REQUIRE_FALSE(fs::exists(p));
    }
}

TEST_CASE("a FRESH abandoned lock blocks the sweep for kStaleSeconds — accepted (I-0147)",
          "[integration][SP-116][I-0147]") {
    // ⚠️ THIS ASSERTS A LIMITATION, NOT A FIX (user ruling 2026-08-21, option 1).
    //
    // When a volume vanishes mid-import the writer dies HOLDING THE LOCK, so
    // `.lock` survives with a FRESH heartbeat alongside the orphaned `.partial`.
    // A quick reattach then finds the lock not-yet-stale: acquire refuses with
    // worldLocked, and since the sweep only runs after a successful acquire, the
    // orphan is retained until the lock ages out at kStaleSeconds.
    //
    // ⚠️ My earlier hardware test passed only because it staged the orphan
    // WITHOUT the matching fresh lock — the aftermath of the failure, not the
    // failure. This encodes the real state so the behaviour cannot change
    // unnoticed: if someone later makes acquire break fresh locks, this fails
    // and forces the locking-model conversation rather than letting it happen by
    // accident (the lesson of I-0144).
    AssetCApiFixture fix;
    const auto worldID = fix.makeWorld();
    const auto seed    = fix.writeSource("seed.png", "FAKE_PNG");
    (void)okResult(scrivi_import_asset(
        fix.root(), seed.c_str(), "image", "Seed",
        "identity-001", "persona-001", "Test Author",
        worldID.c_str(), fix.projectID.c_str()));

    // The state a real crash leaves: a live-looking lock AND an orphan.
    //
    // ⚠️ ORDER MATTERS, and getting it wrong cost me a false failure. The lock
    // must be taken FIRST: acquiring a lock now runs the sweep (T-0433), so an
    // orphan staged before the lock is reclaimed by the very act of staging the
    // lock. A real crash produces this order naturally — the writer holds the
    // lock, then writes the partial, then dies.
    CompetingWriter dead{fix.pkg("Eskandar")};   // holds a fresh, heartbeating lock

    const auto orphan = fs::path(fix.pkg("Eskandar")) / "assets" / "images" /
                        "crashed.png.partial";
    { std::ofstream o(orphan, std::ios::binary); o << std::string(1024, 'x'); }

    const auto next = fix.writeSource("next3.png", "FAKE_PNG");
    auto err = errorOf(scrivi_import_asset(
        fix.root(), next.c_str(), "image", "Next",
        "identity-001", "persona-001", "Test Author",
        worldID.c_str(), fix.projectID.c_str()));

    REQUIRE(err.getString("detail") == "worldLocked");
    REQUIRE(fs::exists(orphan));   // ⚠️ retained — this is the accepted limitation
}

TEST_CASE("the sweep NEVER touches real assets or sidecars",
          "[integration][SP-116][T-0433][I-0146][S14]") {
    // ⚠️ The dangerous failure mode for a sweep is over-reach. A suffix match
    // that caught "portrait.png" instead of "portrait.png.partial" would delete
    // a writer's images — far worse than the junk it cleans up.
    AssetCApiFixture fix;
    const auto worldID = fix.makeWorld();
    const auto src     = fix.writeSource("keeper.png", "REAL_BYTES");

    auto imported = okResult(scrivi_import_asset(
        fix.root(), src.c_str(), "image", "Keeper",
        "identity-001", "persona-001", "Test Author",
        worldID.c_str(), fix.projectID.c_str()));
    const auto kept    = imported.getString("assetPath");
    const auto sidecar = imported.getString("sidecarPath");

    // A file whose name merely CONTAINS "partial" must also survive.
    const auto decoy = fs::path(fix.pkg("Eskandar")) / "assets" / "images" /
                       "partial-eclipse.png";
    { std::ofstream o(decoy, std::ios::binary); o << "DECOY"; }

    const auto src2 = fix.writeSource("trigger.png", "FAKE_PNG");
    (void)okResult(scrivi_import_asset(
        fix.root(), src2.c_str(), "image", "Trigger",
        "identity-001", "persona-001", "Test Author",
        worldID.c_str(), fix.projectID.c_str()));

    REQUIRE(fs::exists(kept));
    REQUIRE(fs::exists(sidecar));
    REQUIRE(fs::exists(decoy));

    auto listed = okResult(scrivi_list_assets(fix.root(), "", worldID.c_str()));
    REQUIRE(listed.getInt("count") == 2);   // Keeper + Trigger, both intact
}

TEST_CASE("a partial left in a PROJECT (unlocked) import is still cleaned by rollback",
          "[integration][SP-116][T-0432]") {
    // Project writes take no lock, so they get no sweep — their safety net is
    // the in-process rollback, which is sufficient there because a project is
    // not shared between processes the way a world package is.
    AssetCApiFixture fix;
    const auto src = fix.writeSource("proj.png", "FAKE_PNG");

    auto imported = okResult(scrivi_import_asset(
        fix.root(), src.c_str(), "image", "Project Asset",
        "identity-001", "persona-001", "Test Author", "", ""));

    REQUIRE(fs::exists(imported.getString("assetPath")));
    REQUIRE_FALSE(fs::exists(imported.getString("assetPath") + ".partial"));
}

// ---------------------------------------------------------------------------
// S7 — D5: the kind-scope endpoint, and that it stays DERIVED
// ---------------------------------------------------------------------------

TEST_CASE("C ABI: list_object_kinds reports every storable kind with its scope",
          "[integration][SP-116][T-0429][I-0140][S7]") {
    auto res = okResult(scrivi_list_object_kinds());

    const auto n = res.arraySize("kinds");
    REQUIRE(n == std::size(scrivi::kAllStorableKinds));
    REQUIRE(res.getInt("count") == static_cast<int>(n));

    // ⚠️ THE ASSERTION THAT MATTERS: the endpoint must agree with
    // objectKindIsWorldScoped() for EVERY kind, derived the same way the rule
    // itself is. Written as a comparison against the predicate rather than
    // against a hand-written expectation table — a table here would be exactly
    // the restated-kind-list defect this endpoint exists to retire, and would
    // silently desync the day a kind's scope changes.
    std::vector<std::string> seen;
    for (std::size_t i = 0; i < n; ++i) {
        auto item = res.arrayItem("kinds", i);
        const auto name = item.getString("kind");
        seen.push_back(name);

        auto parsed = scrivi::objectKindFromName(name);
        REQUIRE(parsed.has_value());
        INFO("kind: " << name);
        REQUIRE(item.getBool("isWorldScoped") ==
                scrivi::objectKindIsWorldScoped(parsed.value()));
        REQUIRE(item.getString("subdir") == scrivi::objectKindSubdir(parsed.value()));
    }

    // Every storable kind appears exactly once.
    for (auto kind : scrivi::kAllStorableKinds) {
        const auto name = scrivi::objectKindName(kind);
        INFO("expected kind missing from envelope: " << name);
        REQUIRE(std::count(seen.begin(), seen.end(), name) == 1);
    }

    // `world` is a container, not a storable kind — it must not be offered as
    // something a writer could create through the object endpoints.
    REQUIRE(std::count(seen.begin(), seen.end(), "world") == 0);
}

TEST_CASE("C ABI: the kind-scope endpoint agrees with what the object endpoints actually enforce",
          "[integration][SP-116][T-0429][I-0140][S7]") {
    // ⚠️ The endpoint could be internally consistent and still LIE about the
    // behaviour it describes. This checks the claim against the enforcement:
    // a kind the endpoint calls world-scoped must be REFUSED without a worldID,
    // and a kind it calls project-scoped must be ACCEPTED without one.
    AssetCApiFixture fix;
    auto res = okResult(scrivi_list_object_kinds());

    const auto n = res.arraySize("kinds");
    for (std::size_t i = 0; i < n; ++i) {
        auto item = res.arrayItem("kinds", i);
        const auto kind          = item.getString("kind");
        const bool isWorldScoped = item.getBool("isWorldScoped");

        INFO("kind: " << kind << " isWorldScoped=" << isWorldScoped);

        auto raw = scrivi_create_object(fix.root(), kind.c_str(),
                                        ("Probe " + kind).c_str(), "",
                                        "identity-001", "persona-001", "Test Author",
                                        "");   // deliberately no worldID
        JsonDoc env = envelope(raw);

        if (isWorldScoped) {
            REQUIRE_FALSE(env.getBool("ok"));
            REQUIRE(env.getSubDoc("error").getString("detail") == "worldRequired");
        } else {
            REQUIRE(env.getBool("ok"));
        }
    }
}
