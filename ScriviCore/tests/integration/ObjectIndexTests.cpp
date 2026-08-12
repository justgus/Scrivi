// SP-095 T-0401 — objects/index.json rebuild + corruption coverage.
//
// Worldbuilding Object Model v0.2 §4.2 calls the rebuild path "a correctness
// requirement, not a convenience": the index is a DERIVED CACHE and the
// <slug>.json files are the truth. Every test here asserts the same invariant
// from a different angle — disk wins, and the project always opens.

#include <catch2/catch_test_macros.hpp>

#include "scrivi/ObjectTypes.hpp"
#include "scrivi/Requests.hpp"
#include "scrivi/ScriviCore.hpp"

#include "mocks/DeterministicUUIDProvider.hpp"
#include "mocks/FixedClock.hpp"
#include "mocks/MockGitProvider.hpp"
#include "mocks/MockSecureStore.hpp"
#include "platform/LocalFileSystem.hpp"

#include <chrono>
#include <filesystem>
#include <fstream>
#include <iterator>
#include <string>

namespace fs = std::filesystem;

namespace {

// Counts listDirectory calls into objects/<kind>/ so a test can prove that the
// index REPLACED the directory scan rather than merely sitting alongside it.
// Everything else forwards to the real filesystem.
class ScanCountingFileSystem final : public scrivi::FileSystem {
public:
    int objectDirScans = 0;

    scrivi::Result<bool> exists(const scrivi::AbsolutePath& p) override { return inner_.exists(p); }
    scrivi::Result<bool> isDirectory(const scrivi::AbsolutePath& p) override { return inner_.isDirectory(p); }
    scrivi::Result<void> createDirectories(const scrivi::AbsolutePath& p) override { return inner_.createDirectories(p); }
    scrivi::Result<scrivi::Utf8Text> readTextFile(const scrivi::AbsolutePath& p) override { return inner_.readTextFile(p); }
    scrivi::Result<void> atomicWriteTextFile(const scrivi::AbsolutePath& p, std::string_view t) override { return inner_.atomicWriteTextFile(p, t); }
    scrivi::Result<void> appendTextFile(const scrivi::AbsolutePath& p, std::string_view t) override { return inner_.appendTextFile(p, t); }
    scrivi::Result<void> removeFile(const scrivi::AbsolutePath& p) override { return inner_.removeFile(p); }
    scrivi::Result<void> renamePath(const scrivi::AbsolutePath& a, const scrivi::AbsolutePath& b) override { return inner_.renamePath(a, b); }

    scrivi::Result<std::vector<scrivi::AbsolutePath>>
    listDirectory(const scrivi::AbsolutePath& p) override {
        if (p.find("/objects/") != std::string::npos) { ++objectDirScans; }
        return inner_.listDirectory(p);
    }

private:
    scrivi::platform::LocalFileSystem inner_;
};

struct IndexFixture {
    fs::path projectDir;
    fs::path appSupportDir;

    ScanCountingFileSystem                   fileSystem;
    scrivi::mocks::DeterministicUUIDProvider uuidProvider;
    scrivi::mocks::FixedClock                clock{"2026-08-12T00:00:00Z"};
    scrivi::mocks::MockGitProvider           gitProvider;
    scrivi::mocks::MockSecureStore           secureStore;
    scrivi::ScriviCore                       core;

    const scrivi::AuthorshipRef author{
        scrivi::IdentityID{"identity-001"},
        scrivi::PersonaID {"persona-001"},
        "Test Author"
    };

    IndexFixture()
        : projectDir(fs::temp_directory_path() /
                     ("scrivi-objindex-" + std::to_string(
                         std::chrono::steady_clock::now().time_since_epoch().count())))
        , appSupportDir(projectDir / "appsupport")
        , core([&]{
            scrivi::CoreServices svc;
            svc.fileSystem   = &fileSystem;
            svc.uuidProvider = &uuidProvider;
            svc.clock        = &clock;
            svc.gitProvider  = &gitProvider;
            svc.secureStore  = &secureStore;
            svc.logger       = nullptr;
            return svc;
          }())
    {
        fs::create_directories(projectDir);
        fs::create_directories(appSupportDir);

        scrivi::CreateProjectRequest req;
        req.projectRootPath = projectDir.string();
        req.appSupportRoot  = appSupportDir.string();
        req.title           = "Index Test Project";
        req.slug            = "index-test";
        req.author          = author;
        auto r = core.createProject(req);
        REQUIRE(r.ok());
    }

    ~IndexFixture() {
        std::error_code ec;
        fs::remove_all(projectDir, ec);
    }

    [[nodiscard]] fs::path indexPath() const {
        return projectDir / "objects" / "index.json";
    }

    [[nodiscard]] std::string readIndex() const {
        std::ifstream in(indexPath());
        return {std::istreambuf_iterator<char>(in), std::istreambuf_iterator<char>()};
    }

    void writeIndex(std::string_view body) const {
        fs::create_directories(indexPath().parent_path());
        std::ofstream out(indexPath(), std::ios::trunc);
        out << body;
    }

    scrivi::ObjectID makeObject(scrivi::ObjectKind kind,
                                const std::string& displayName,
                                const std::string& slug) {
        scrivi::CreateObjectRequest req;
        req.projectRootPath = projectDir.string();
        req.objectKind      = kind;
        req.displayName     = displayName;
        req.slug            = slug;
        req.author          = author;
        auto r = core.createObject(req);
        REQUIRE(r.ok());
        return r.value().objectID;
    }

    // Opening through the object CRUD path is what exercises findByID, which
    // is the whole point of the index.
    [[nodiscard]] bool canResolve(scrivi::ObjectKind kind, const scrivi::ObjectID& id) {
        scrivi::OpenObjectRequest req;
        req.projectRootPath = projectDir.string();
        req.objectKind      = kind;
        req.objectID        = id;
        return core.openObject(req).ok();
    }
};

} // namespace

// ---------------------------------------------------------------------------
// Build + maintenance
// ---------------------------------------------------------------------------

TEST_CASE("creating an object writes an index entry", "[integration][T-0372]") {
    IndexFixture fix;
    auto id = fix.makeObject(scrivi::ObjectKind::character, "Ada", "ada");

    const auto body = fix.readIndex();
    REQUIRE(body.find("scrivi.object-index.v1") != std::string::npos);
    REQUIRE(body.find(id.value) != std::string::npos);
    REQUIRE(body.find("\"ada\"") != std::string::npos);
    REQUIRE(body.find("character") != std::string::npos);
}

TEST_CASE("saving an object refreshes its index entry", "[integration][T-0372]") {
    IndexFixture fix;
    auto id = fix.makeObject(scrivi::ObjectKind::character, "Ada", "ada");

    scrivi::OpenObjectRequest openReq;
    openReq.projectRootPath = fix.projectDir.string();
    openReq.objectKind      = scrivi::ObjectKind::character;
    openReq.objectID        = id;
    auto opened = fix.core.openObject(openReq);
    REQUIRE(opened.ok());

    auto obj = opened.value().object;
    std::get<scrivi::CharacterObject>(obj).displayName = "Ada Thornwood";
    std::get<scrivi::CharacterObject>(obj).worldID     = "world_01MIDGARD";

    scrivi::SaveObjectRequest saveReq;
    saveReq.projectRootPath = fix.projectDir.string();
    saveReq.object          = obj;
    saveReq.author          = fix.author;
    REQUIRE(fix.core.saveObject(saveReq).ok());

    const auto body = fix.readIndex();
    REQUIRE(body.find("Ada Thornwood")   != std::string::npos);
    REQUIRE(body.find("world_01MIDGARD") != std::string::npos);
}

TEST_CASE("deleting an object removes its index entry", "[integration][T-0372]") {
    IndexFixture fix;
    auto keep = fix.makeObject(scrivi::ObjectKind::character, "Ada",  "ada");
    auto drop = fix.makeObject(scrivi::ObjectKind::character, "Bram", "bram");

    scrivi::DeleteObjectRequest req;
    req.projectRootPath = fix.projectDir.string();
    req.objectKind      = scrivi::ObjectKind::character;
    req.objectID        = drop;
    REQUIRE(fix.core.deleteObject(req).ok());

    const auto body = fix.readIndex();
    REQUIRE(body.find(drop.value) == std::string::npos);
    REQUIRE(body.find(keep.value) != std::string::npos);
}

// ---------------------------------------------------------------------------
// Rebuild triggers (Doc 1 AC2) — missing / corrupt / stale
// ---------------------------------------------------------------------------

TEST_CASE("a MISSING index is rebuilt from a directory scan",
          "[integration][T-0401]") {
    IndexFixture fix;
    auto ada  = fix.makeObject(scrivi::ObjectKind::character, "Ada",   "ada");
    auto keep = fix.makeObject(scrivi::ObjectKind::location,  "Vault", "vault");

    fs::remove(fix.indexPath());
    REQUIRE_FALSE(fs::exists(fix.indexPath()));

    // Resolution still succeeds, and the index is back afterwards.
    REQUIRE(fix.canResolve(scrivi::ObjectKind::character, ada));
    REQUIRE(fs::exists(fix.indexPath()));

    const auto body = fix.readIndex();
    REQUIRE(body.find(ada.value)  != std::string::npos);
    REQUIRE(body.find(keep.value) != std::string::npos);
}

TEST_CASE("a CORRUPT index is rebuilt rather than failing the open",
          "[integration][T-0401]") {
    IndexFixture fix;
    auto ada = fix.makeObject(scrivi::ObjectKind::character, "Ada", "ada");

    SECTION("malformed JSON") {
        fix.writeIndex("{ this is not json ");
    }
    SECTION("valid JSON, wrong schema tag") {
        fix.writeIndex(R"({"schema":"scrivi.something-else.v1","entries":[]})");
    }
    SECTION("valid JSON, entry naming an unknown kind") {
        fix.writeIndex(R"({"schema":"scrivi.object-index.v1","entries":[
            {"objectID":"x","kind":"sasquatch","slug":"x","displayName":"X","worldID":""}]})");
    }
    SECTION("valid JSON, entry with no objectID") {
        fix.writeIndex(R"({"schema":"scrivi.object-index.v1","entries":[
            {"objectID":"","kind":"character","slug":"x","displayName":"X","worldID":""}]})");
    }
    SECTION("truncated mid-document") {
        fix.writeIndex(R"({"schema":"scrivi.object-index.v1","entries":[{"objectID":)");
    }

    // The project must open and the object must still resolve, in every case.
    REQUIRE(fix.canResolve(scrivi::ObjectKind::character, ada));
    REQUIRE(fix.readIndex().find(ada.value) != std::string::npos);
}

TEST_CASE("a STALE index loses to disk", "[integration][T-0401]") {
    IndexFixture fix;
    auto ada = fix.makeObject(scrivi::ObjectKind::character, "Ada", "ada");

    SECTION("hand-edited displayName in the object file") {
        // Edit the FILE, leaving the index advertising the old name.
        const auto objPath = fix.projectDir / "objects" / "characters" / "ada.json";
        std::string body;
        {
            std::ifstream in(objPath);
            body.assign(std::istreambuf_iterator<char>(in),
                        std::istreambuf_iterator<char>());
        }
        const auto pos = body.find("\"displayName\": \"Ada\"");
        REQUIRE(pos != std::string::npos);
        body.replace(pos, std::string("\"displayName\": \"Ada\"").size(),
                     "\"displayName\": \"Renamed On Disk\"");
        {
            std::ofstream out(objPath, std::ios::trunc);
            out << body;
        }

        // Force a rebuild by removing the index, then confirm disk won.
        fs::remove(fix.indexPath());
        REQUIRE(fix.canResolve(scrivi::ObjectKind::character, ada));
        REQUIRE(fix.readIndex().find("Renamed On Disk") != std::string::npos);
        REQUIRE(fix.readIndex().find("\"Ada\"")         == std::string::npos);
    }

    SECTION("index points at a slug that no longer exists") {
        // The index still names ada.json; the real file is now renamed.json.
        const auto dir = fix.projectDir / "objects" / "characters";
        fs::rename(dir / "ada.json", dir / "renamed.json");

        // findByID must not return the stale path — it falls through to the
        // scan and still resolves the object.
        REQUIRE(fix.canResolve(scrivi::ObjectKind::character, ada));
    }
}

TEST_CASE("rebuilding twice yields identical content", "[integration][T-0401]") {
    IndexFixture fix;
    auto ada = fix.makeObject(scrivi::ObjectKind::character, "Ada",   "ada");
    fix.makeObject(scrivi::ObjectKind::location,  "Vault", "vault");
    fix.makeObject(scrivi::ObjectKind::item,      "Key",   "key");

    // Each resolve after a delete forces a fresh scan. Rebuild output must not
    // depend on the order the filesystem hands back directory entries, or the
    // index would churn on every open and defeat its own purpose.
    fs::remove(fix.indexPath());
    REQUIRE(fix.canResolve(scrivi::ObjectKind::character, ada));
    const auto first = fix.readIndex();

    fs::remove(fix.indexPath());
    REQUIRE(fix.canResolve(scrivi::ObjectKind::character, ada));
    const auto second = fix.readIndex();

    REQUIRE_FALSE(first.empty());
    REQUIRE(first == second);

    // All three objects are present in the rebuilt index, not just the one
    // that triggered the rebuild.
    REQUIRE(first.find("\"ada\"")   != std::string::npos);
    REQUIRE(first.find("\"vault\"") != std::string::npos);
    REQUIRE(first.find("\"key\"")   != std::string::npos);
}

TEST_CASE("one unparseable object file does not cost the whole index",
          "[integration][T-0401]") {
    IndexFixture fix;
    auto ada  = fix.makeObject(scrivi::ObjectKind::character, "Ada",  "ada");
    auto bram = fix.makeObject(scrivi::ObjectKind::character, "Bram", "bram");

    // Corrupt ONE object file, mirroring collectObjects' best-effort posture.
    {
        std::ofstream out(fix.projectDir / "objects" / "characters" / "bram.json",
                          std::ios::trunc);
        out << "{ not a valid object ";
    }

    fs::remove(fix.indexPath());
    REQUIRE(fix.canResolve(scrivi::ObjectKind::character, ada));

    const auto body = fix.readIndex();
    REQUIRE(body.find(ada.value)  != std::string::npos);
    REQUIRE(body.find(bram.value) == std::string::npos);
}

TEST_CASE("findByID performs NO directory scan on the happy path",
          "[integration][T-0372]") {
    IndexFixture fix;

    // A populated project: without the index, resolving one object would list
    // a kind directory and parse every file in it.
    auto ada = fix.makeObject(scrivi::ObjectKind::character, "Ada",   "ada");
    fix.makeObject(scrivi::ObjectKind::character, "Bram",  "bram");
    fix.makeObject(scrivi::ObjectKind::character, "Cora",  "cora");
    fix.makeObject(scrivi::ObjectKind::location,  "Vault", "vault");

    // Resolve through a healthy index and count scans over just that call.
    fix.fileSystem.objectDirScans = 0;
    REQUIRE(fix.canResolve(scrivi::ObjectKind::character, ada));
    REQUIRE(fix.fileSystem.objectDirScans == 0);

    // The scan is still there as the repair path: destroy the index and the
    // same lookup falls back to it rather than failing.
    fs::remove(fix.indexPath());
    fix.fileSystem.objectDirScans = 0;
    REQUIRE(fix.canResolve(scrivi::ObjectKind::character, ada));
    REQUIRE(fix.fileSystem.objectDirScans > 0);
}

TEST_CASE("the index survives a create/reopen cycle", "[integration][T-0372]") {
    IndexFixture fix;
    auto ada = fix.makeObject(scrivi::ObjectKind::character, "Ada", "ada");

    const auto before = fix.readIndex();
    REQUIRE(fix.canResolve(scrivi::ObjectKind::character, ada));

    // Resolution through a healthy index must not rewrite it.
    REQUIRE(fix.readIndex() == before);
}
