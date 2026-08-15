#include <catch2/catch_test_macros.hpp>

#include "scrivi/ScriviCore.hpp"
#include "scrivi/Requests.hpp"
#include "scrivi/ObjectTypes.hpp"
#include "worlds/WorldStore.hpp"

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

        // ⚠️ SP-104: since T-0409 every worldbuilding kind is world-scoped, so a
        // world must exist before any object can be created. This fixture
        // predates the ruling and created objects with no world at all.
        scrivi::CoreServices svc;
        svc.fileSystem   = &fileSystem;
        svc.uuidProvider = &uuidProvider;
        svc.clock        = &clock;
        svc.gitProvider  = &gitProvider;
        svc.secureStore  = &secureStore;
        svc.logger       = nullptr;
        scrivi::worlds::WorldStore ws{svc};
        auto w = ws.createWorld(projectDir.string(),
                                (projectDir / "Search.scrivworld").string(),
                                "Search World", "");
        REQUIRE(w.ok());
        worldID = w.value().worldID;
    }

    ~SearchFixture() { std::error_code ec; fs::remove_all(projectDir, ec); }

    std::string worldID;

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
        if (scrivi::objectKindIsWorldScoped(kind)) { creq.worldID = worldID; }
        auto cr = core.createObject(creq);
        REQUIRE(cr.ok());

        scrivi::OpenObjectRequest oreq;
        oreq.projectRootPath = projectDir.string();
        oreq.objectKind      = kind;
        oreq.objectID        = cr.value().objectID;
        if (scrivi::objectKindIsWorldScoped(kind)) { oreq.worldID = worldID; }
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
    fix.makeObject(scrivi::ObjectKind::building, "Guild Hall",   "", {});
    fix.makeObject(scrivi::ObjectKind::vehicle,  "Ore Cart",     "", {});
    fix.makeObject(scrivi::ObjectKind::map,      "Tunnel Map",   "", {});

    auto r = fix.extract();

    // ⚠️ I-0118 (ruled 2026-08-14): world package contents ARE indexed now, so
    // every world-scoped kind is searchable again. This test previously asserted
    // zero for all of them — the regression-in-reach guard — and was written to
    // fail loudly the moment indexing landed. It did.
    CHECK(countKind(r, "character") == 1);
    CHECK(countKind(r, "location")  == 1);
    CHECK(countKind(r, "item")      == 1);
    CHECK(countKind(r, "building")  == 1);
    CHECK(countKind(r, "vehicle")   == 1);
    CHECK(countKind(r, "map")       == 1);

    // The retired `timeline` kind indexes nothing — the project timeline is
    // not a world object (SP-095 T-0370).
    CHECK(countKind(r, "timeline")  == 0);

    const auto* ch = find(r, "character:" + charID.value);
    REQUIRE(ch != nullptr);
    CHECK(ch->title == "Khaz'tul Miner");
    CHECK(ch->displayName == "Khaz'tul Miner");
    CHECK(ch->contentDescription == "Digs the silver mines.");
    REQUIRE(ch->keywords.size() == 2);
    CHECK(ch->keywords[0] == "dwarf");
    CHECK(ch->keywords[1] == "miner");

    // ⚠️ Q1 — a world item belongs to the WORLD's domain, never the project's.
    // This is the assertion that keeps one project's teardown from wiping a
    // shared world's entries.
    CHECK(ch->domainIdentifier == fix.worldID);
    CHECK(ch->domainIdentifier != r.domainIdentifier);

    // ⚠️ Q2 — the deep link is WORLD-scoped, because a character bound by
    // several projects has no single owning project.
    CHECK(ch->deepLink ==
          "scrivi://open?world=" + fix.worldID
          + "&item=character:" + charID.value);

    // The world's domain is advertised so the donor can index it separately.
    REQUIRE(r.worldDomainIdentifiers.size() == 1);
    CHECK(r.worldDomainIdentifiers[0] == fix.worldID);

    // `source` stays project-scoped: project domain, project-scoped deep link.
    // The control proving the two halves did not collapse into one.
    auto srcID = fix.makeObject(scrivi::ObjectKind::source, "Mining Digest",
                                "A cited work.", {"reference"});
    auto r2 = fix.extract();
    CHECK(countKind(r2, "source") == 1);
    const auto* src = find(r2, "source:" + srcID.value);
    REQUIRE(src != nullptr);
    CHECK(src->domainIdentifier.empty());   // empty ⇒ the result's project domain
    CHECK(src->deepLink ==
          "scrivi://open?project=" + fix.created.project.projectID.value
          + "&item=source:" + srcID.value);
}

TEST_CASE("extractSearchableText - an UNAVAILABLE world is skipped, never pruned",
          "[integration][I-0118]") {
    SearchFixture fix;
    fix.makeObject(scrivi::ObjectKind::character, "Khaz'tul Miner", "", {});

    // Present while the world is reachable...
    REQUIRE(countKind(fix.extract(), "character") == 1);

    // ...and gone from THIS extraction once it is not. ⚠️ Q3: that is a skip,
    // not a prune — the caller must not read an empty result as "delete these".
    // Previously donated entries stay in Spotlight, because a disconnected
    // volume must never make a writer's cast vanish from search (the I-0115
    // principle: never treat unreadable as absent).
    fs::rename(fix.projectDir / "Search.scrivworld",
               fix.projectDir / "Search.scrivworld-detached");

    auto r = fix.extract();
    CHECK(countKind(r, "character") == 0);
    // The world contributes NO domain, so the donor is never even told about a
    // domain it might otherwise be tempted to clear.
    CHECK(r.worldDomainIdentifiers.empty());
    // The project half is unaffected — an away world costs its own records only.
    CHECK(countKind(r, "project") == 1);
}

TEST_CASE("extractSearchableText - the whole world package is indexed, not only object kinds",
          "[integration][I-0118]") {
    SearchFixture fix;
    const auto pkg = fix.projectDir / "Search.scrivworld";

    // ⚠️ Q4 — `historical-events`, `historical-timelines` and `assets` are not
    // ObjectKinds and have no subdir mapping, so they are indexed explicitly.
    {
        std::ofstream f(pkg / "historical-events" / "sundering.json", std::ios::binary);
        f << R"({"eventID":"hev-001","displayName":"The First Sundering",)"
             R"("notes":"The continent split."})";
    }
    {
        std::ofstream f(pkg / "historical-timelines" / "ages.json", std::ios::binary);
        f << R"({"timelineID":"htl-001","displayName":"The Four Ages"})";
    }

    auto r = fix.extract();

    const auto* ev = find(r, "historical-event:hev-001");
    REQUIRE(ev != nullptr);
    CHECK(ev->title == "The First Sundering");
    CHECK(ev->contentDescription == "The continent split.");
    CHECK(ev->domainIdentifier == fix.worldID);

    const auto* tl = find(r, "historical-timeline:htl-001");
    REQUIRE(tl != nullptr);
    CHECK(tl->title == "The Four Ages");

    // Best-effort, as everywhere else: one unreadable file costs only itself.
    {
        std::ofstream f(pkg / "historical-events" / "broken.json", std::ios::binary);
        f << "not json {{{";
    }
    CHECK(find(fix.extract(), "historical-event:hev-001") != nullptr);
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
    // ⚠️ SP-104: retargeted from `character` to `source`. The point of this test
    // is that ONE unparseable file does not cost the whole extraction — that
    // requires a kind the extractor actually indexes. Since T-0409 `source` is
    // the only project-scoped kind, so asserting against `character` would have
    // asserted 0 == 0 and passed vacuously, proving nothing (see I-0118).
    fix.makeObject(scrivi::ObjectKind::source, "Valid Source", "", {});

    // Drop a garbage .json beside it.
    fs::create_directories(fix.sub("objects/sources"));
    {
        std::ofstream f(fix.sub("objects/sources/broken.json"), std::ios::binary);
        f << "not valid json {{{";
    }

    auto r = fix.extract();
    // The valid source still surfaces; the broken file is silently skipped.
    CHECK(countKind(r, "source") == 1);
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
