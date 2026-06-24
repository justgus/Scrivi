#include <catch2/catch_test_macros.hpp>

#include "scrivi/ScriviCore.hpp"
#include "scrivi/Requests.hpp"
#include "scrivi/ObjectTypes.hpp"

#include "mocks/DeterministicUUIDProvider.hpp"
#include "mocks/FixedClock.hpp"
#include "mocks/MockGitProvider.hpp"
#include "mocks/MockSecureStore.hpp"
#include "platform/LocalFileSystem.hpp"

#include <algorithm>
#include <filesystem>
#include <fstream>
#include <string>

namespace fs = std::filesystem;

// ---------------------------------------------------------------------------
// Fixture — a real on-disk project the facade reads back.
// ---------------------------------------------------------------------------

namespace {

struct SearchFixture {
    fs::path projectDir;
    fs::path appSupportDir;

    scrivi::platform::LocalFileSystem        fileSystem;
    scrivi::mocks::DeterministicUUIDProvider uuidProvider;
    scrivi::mocks::FixedClock                clock{"2026-06-23T00:00:00Z"};
    scrivi::mocks::MockGitProvider           gitProvider;
    scrivi::mocks::MockSecureStore           secureStore;
    scrivi::ScriviCore                       core;

    scrivi::CreateProjectResult created;

    const scrivi::AuthorshipRef author{
        scrivi::IdentityID{"identity-001"},
        scrivi::PersonaID {"persona-001"},
        "Test Author"
    };

    SearchFixture()
        : projectDir(fs::temp_directory_path() /
                     ("scrivi-search-test-" + std::to_string(
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
        req.title           = "My Novel";
        req.slug            = "my-novel";
        req.author          = author;
        auto r = core.createProject(req);
        REQUIRE(r.ok());
        created = r.value();
    }

    ~SearchFixture() { std::error_code ec; fs::remove_all(projectDir, ec); }

    std::string sub(const std::string& rel) const {
        return (projectDir / rel).string();
    }

    scrivi::ExtractSearchableTextResult extract() {
        scrivi::ExtractSearchableTextRequest req;
        req.projectRootPath = projectDir.string();
        auto r = core.extractSearchableText(req);
        REQUIRE(r.ok());
        return r.value();
    }

    // Creates a world object then saves it with notes + tags populated, so the
    // facade has contentDescription/keywords to surface.
    scrivi::ObjectID makeObject(scrivi::ObjectKind kind,
                                const std::string& displayName,
                                const std::string& notes,
                                std::vector<std::string> tags) {
        scrivi::CreateObjectRequest creq;
        creq.projectRootPath = projectDir.string();
        creq.objectKind      = kind;
        creq.displayName     = displayName;
        creq.author          = author;
        auto cr = core.createObject(creq);
        REQUIRE(cr.ok());

        scrivi::OpenObjectRequest oreq;
        oreq.projectRootPath = projectDir.string();
        oreq.objectKind      = kind;
        oreq.objectID        = cr.value().objectID;
        auto orr = core.openObject(oreq);
        REQUIRE(orr.ok());

        scrivi::WorldObject obj = orr.value().object;
        std::visit([&](auto& o) { o.notes = notes; o.tags = tags; }, obj);

        scrivi::SaveObjectRequest sreq;
        sreq.projectRootPath = projectDir.string();
        sreq.object          = obj;
        sreq.author          = author;
        REQUIRE(core.saveObject(sreq).ok());

        return cr.value().objectID;
    }
};

const scrivi::SearchableItem* find(
    const scrivi::ExtractSearchableTextResult& r, const std::string& uid) {
    for (const auto& it : r.items) {
        if (it.uniqueIdentifier == uid) { return &it; }
    }
    return nullptr;
}

std::size_t countKind(const scrivi::ExtractSearchableTextResult& r, const std::string& kind) {
    return static_cast<std::size_t>(
        std::count_if(r.items.begin(), r.items.end(),
                      [&](const scrivi::SearchableItem& it){ return it.kind == kind; }));
}

} // namespace

// ---------------------------------------------------------------------------
// Tests
// ---------------------------------------------------------------------------

TEST_CASE("extractSearchableText - schema, domainIdentifier, and project record",
          "[integration][EP-017][T-0179]") {
    SearchFixture fix;
    auto r = fix.extract();

    CHECK(r.schema == "scrivi.searchableContent.v1");
    // domainIdentifier is the project's own projectID (delete-by-domain key),
    // NOT the per-machine identity_ id. (With the real provider this is a
    // "project_<uuid>"; the deterministic test provider yields "proj-001".)
    CHECK(r.domainIdentifier == fix.created.project.projectID.value);
    CHECK(r.projectRootPath == fix.projectDir.string());

    const auto* proj = find(r, "project:" + fix.created.project.projectID.value);
    REQUIRE(proj != nullptr);
    CHECK(proj->kind == "project");
    CHECK(proj->title == "My Novel");
    CHECK(proj->displayName == "My Novel");
    CHECK(proj->deepLink ==
          "scrivi://open?project=" + fix.created.project.projectID.value
          + "&item=project:" + fix.created.project.projectID.value);
}

TEST_CASE("extractSearchableText - scene record carries chapter container and stripped body",
          "[integration][EP-017][T-0179]") {
    SearchFixture fix;

    // Write Markdown directly into the opening scene's content file.
    {
        std::ofstream f(fix.sub("manuscript/chapter-001/001-opening-scene.md"), std::ios::binary);
        f << "# The Beginning\n\nIt was a **dark** and *stormy* [night](http://x).\n";
    }

    auto r = fix.extract();

    const auto* scene = find(r, "scene:" + fix.created.firstSceneID.value);
    REQUIRE(scene != nullptr);
    CHECK(scene->kind == "scene");
    CHECK(scene->containerTitle == "Chapter 1");
    // Markdown markup stripped to plain text.
    CHECK(scene->contentDescription == "The Beginning\nIt was a dark and stormy night.");
    CHECK(scene->deepLink ==
          "scrivi://open?project=" + fix.created.project.projectID.value
          + "&item=scene:" + fix.created.firstSceneID.value);
}

TEST_CASE("extractSearchableText - world objects map name/notes/tags",
          "[integration][EP-017][T-0179]") {
    SearchFixture fix;

    auto charID = fix.makeObject(scrivi::ObjectKind::character, "Khaz'tul Miner",
                                 "Digs the silver mines.", {"dwarf", "miner"});
    fix.makeObject(scrivi::ObjectKind::location, "Silver Mines", "Deep underground.", {});
    fix.makeObject(scrivi::ObjectKind::item,     "Pickaxe",      "", {});
    fix.makeObject(scrivi::ObjectKind::rule,     "Mining Law",   "", {});
    fix.makeObject(scrivi::ObjectKind::timeline,  "Mine History", "", {});

    auto r = fix.extract();

    CHECK(countKind(r, "character") == 1);
    CHECK(countKind(r, "location")  == 1);
    CHECK(countKind(r, "item")      == 1);
    CHECK(countKind(r, "rule")      == 1);
    CHECK(countKind(r, "timeline")  == 1);

    const auto* ch = find(r, "character:" + charID.value);
    REQUIRE(ch != nullptr);
    CHECK(ch->title == "Khaz'tul Miner");
    CHECK(ch->displayName == "Khaz'tul Miner");
    CHECK(ch->contentDescription == "Digs the silver mines.");
    REQUIRE(ch->keywords.size() == 2);
    CHECK(ch->keywords[0] == "dwarf");
    CHECK(ch->keywords[1] == "miner");
    CHECK(ch->deepLink ==
          "scrivi://open?project=" + fix.created.project.projectID.value
          + "&item=character:" + charID.value);
}

TEST_CASE("extractSearchableText - identifiers are stable across runs",
          "[integration][EP-017][T-0179]") {
    SearchFixture fix;
    auto a = fix.extract();
    auto b = fix.extract();

    REQUIRE(a.items.size() == b.items.size());
    for (std::size_t i = 0; i < a.items.size(); ++i) {
        CHECK(a.items[i].uniqueIdentifier == b.items[i].uniqueIdentifier);
    }
}

TEST_CASE("extractSearchableText - degenerate project yields only the project record",
          "[integration][EP-017][T-0179]") {
    SearchFixture fix;
    // A freshly created project has one chapter/scene but no world objects.
    auto r = fix.extract();
    CHECK(countKind(r, "project")   == 1);
    CHECK(countKind(r, "character") == 0);
    CHECK(countKind(r, "location")  == 0);
    CHECK(countKind(r, "item")      == 0);
    CHECK(countKind(r, "rule")      == 0);
    CHECK(countKind(r, "timeline")  == 0);
}

TEST_CASE("extractSearchableText - a malformed object file is skipped, not fatal",
          "[integration][EP-017][T-0179]") {
    SearchFixture fix;
    fix.makeObject(scrivi::ObjectKind::character, "Valid Char", "", {});

    // Drop a garbage .json into the characters dir.
    fs::create_directories(fix.sub("objects/characters"));
    {
        std::ofstream f(fix.sub("objects/characters/broken.json"), std::ios::binary);
        f << "not valid json {{{";
    }

    auto r = fix.extract();
    // The valid character still surfaces; the broken file is silently skipped.
    CHECK(countKind(r, "character") == 1);
}

TEST_CASE("extractSearchableText - invalid project path returns ok:false",
          "[integration][EP-017][T-0179]") {
    scrivi::platform::LocalFileSystem        fileSystem;
    scrivi::mocks::DeterministicUUIDProvider uuidProvider;
    scrivi::mocks::FixedClock                clock{"2026-06-23T00:00:00Z"};
    scrivi::mocks::MockGitProvider           gitProvider;
    scrivi::mocks::MockSecureStore           secureStore;

    scrivi::CoreServices svc;
    svc.fileSystem   = &fileSystem;
    svc.uuidProvider = &uuidProvider;
    svc.clock        = &clock;
    svc.gitProvider  = &gitProvider;
    svc.secureStore  = &secureStore;
    scrivi::ScriviCore core{svc};

    scrivi::ExtractSearchableTextRequest req;
    req.projectRootPath =
        (fs::temp_directory_path() / "scrivi-does-not-exist.scrivi").string();
    auto r = core.extractSearchableText(req);
    CHECK_FALSE(r.ok());
}
