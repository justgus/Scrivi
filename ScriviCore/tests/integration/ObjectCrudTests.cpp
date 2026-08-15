#include <catch2/catch_test_macros.hpp>

#include "scrivi/ScriviCore.hpp"
#include "scrivi/Requests.hpp"
#include "scrivi/ObjectTypes.hpp"

#include "mocks/DeterministicUUIDProvider.hpp"
#include "mocks/FixedClock.hpp"
#include "mocks/MockGitProvider.hpp"
#include "mocks/MockSecureStore.hpp"
#include "platform/LocalFileSystem.hpp"
#include "worlds/WorldStore.hpp"

#include <filesystem>
#include <fstream>
#include <iterator>
#include <string>

namespace fs = std::filesystem;

// ---------------------------------------------------------------------------
// Fixture
// ---------------------------------------------------------------------------

struct ObjectFixture {
    fs::path projectDir;
    fs::path appSupportDir;

    scrivi::platform::LocalFileSystem        fileSystem;
    scrivi::mocks::DeterministicUUIDProvider uuidProvider;
    scrivi::mocks::FixedClock                clock{"2026-05-28T00:00:00Z"};
    scrivi::mocks::MockGitProvider           gitProvider;
    scrivi::mocks::MockSecureStore          secureStore;
    // Declared before `core` so the member init order matches the ctor list.
    scrivi::CoreServices                    services;
    scrivi::ScriviCore                      core;

    // SP-103: the world every worldbuilding kind now lives in.
    std::string worldID;

    const scrivi::AuthorshipRef author{
        scrivi::IdentityID{"identity-001"},
        scrivi::PersonaID {"persona-001"},
        "Test Author"
    };

    ObjectFixture()
        : projectDir(fs::temp_directory_path() /
                     ("scrivi-obj-test-" + std::to_string(
                         std::chrono::steady_clock::now().time_since_epoch().count())))
        , appSupportDir(projectDir / "appsupport")
        , services([&]{
            scrivi::CoreServices svc;
            svc.fileSystem   = &fileSystem;
            svc.uuidProvider = &uuidProvider;
            svc.clock        = &clock;
            svc.gitProvider  = &gitProvider;
            svc.secureStore  = &secureStore;
            svc.logger       = nullptr;
            return svc;
          }())
        , core(services)
    {
        fs::create_directories(projectDir);
        fs::create_directories(appSupportDir);

        scrivi::CreateProjectRequest req;
        req.projectRootPath = projectDir.string();
        req.appSupportRoot  = appSupportDir.string();
        req.title           = "Object Test Project";
        req.slug            = "object-test-project";
        req.author          = author;
        (void)core.createProject(req);

        // SP-103: every worldbuilding kind is world-scoped now.
        scrivi::worlds::WorldStore ws{services};
        auto w = ws.createWorld(projectDir.string(),
                                (projectDir / "Obj.scrivworld").string(),
                                "Object World", "");
        REQUIRE(w.ok());
        worldID = w.value().worldID;
    }

    ~ObjectFixture() { fs::remove_all(projectDir); }

    scrivi::CreateObjectRequest makeCreateReq(
        scrivi::ObjectKind kind,
        const std::string& displayName,
        const std::string& slug = "") const
    {
        scrivi::CreateObjectRequest req;
        req.projectRootPath = projectDir.string();
        req.objectKind      = kind;
        req.displayName     = displayName;
        req.slug            = slug;
        req.author          = author;
        if (scrivi::objectKindIsWorldScoped(kind)) { req.worldID = worldID; }
        return req;
    }

    // SP-103: read/write requests need the worldID too, since the file now lives
    // in the world package. Helpers keep that in ONE place rather than at every
    // call site.
    scrivi::OpenObjectRequest makeOpenReq(scrivi::ObjectKind kind,
                                          const scrivi::ObjectID& id) const {
        scrivi::OpenObjectRequest req;
        req.projectRootPath = projectDir.string();
        req.objectKind      = kind;
        req.objectID        = id;
        if (scrivi::objectKindIsWorldScoped(kind)) { req.worldID = worldID; }
        return req;
    }

    scrivi::DeleteObjectRequest makeDeleteReq(scrivi::ObjectKind kind,
                                              const scrivi::ObjectID& id) const {
        scrivi::DeleteObjectRequest req;
        req.projectRootPath = projectDir.string();
        req.objectKind      = kind;
        req.objectID        = id;
        if (scrivi::objectKindIsWorldScoped(kind)) { req.worldID = worldID; }
        return req;
    }

    // Convenience overload defaulting to character (preserves existing call sites)
    scrivi::CreateObjectRequest makeCreateReq(
        const std::string& displayName,
        const std::string& slug = "") const
    {
        return makeCreateReq(scrivi::ObjectKind::character, displayName, slug);
    }
};

// ---------------------------------------------------------------------------
// Character tests (T-0036 - unchanged behaviour)
// ---------------------------------------------------------------------------

TEST_CASE("createObject succeeds and file exists at expected path",
          "[integration][EP-005][T-0036]")
{
    ObjectFixture fix;

    auto result = fix.core.createObject(fix.makeCreateReq("Ada Thornwood"));
    REQUIRE(result.ok());
    REQUIRE(!result.value().objectID.value.empty());
    REQUIRE(result.value().slug == "ada-thornwood");

    const auto& path = result.value().path;
    REQUIRE(fs::exists(path));
    REQUIRE(path.ends_with("ada-thornwood.json"));
}

TEST_CASE("createObject with explicit slug uses that slug",
          "[integration][EP-005][T-0036]")
{
    ObjectFixture fix;

    auto result = fix.core.createObject(fix.makeCreateReq("Ada Thornwood", "ada"));
    REQUIRE(result.ok());
    REQUIRE(result.value().slug == "ada");
    REQUIRE(result.value().path.ends_with("ada.json"));
}

TEST_CASE("openObject returns the created character with correct fields",
          "[integration][EP-005][T-0036]")
{
    ObjectFixture fix;

    auto created = fix.core.createObject(fix.makeCreateReq("Thomas Belacroix", "thomas"));
    REQUIRE(created.ok());

    auto req = fix.makeOpenReq(scrivi::ObjectKind::character, created.value().objectID);

    auto opened = fix.core.openObject(req);
    REQUIRE(opened.ok());

    auto& ch = std::get<scrivi::CharacterObject>(opened.value().object);
    REQUIRE(ch.displayName      == "Thomas Belacroix");
    REQUIRE(ch.slug             == "thomas");
    REQUIRE(ch.status           == "active");
    REQUIRE(ch.objectID.value   == created.value().objectID.value);
}

TEST_CASE("saveObject updates displayName and modifiedAt on disk",
          "[integration][EP-005][T-0036]")
{
    ObjectFixture fix;

    auto created = fix.core.createObject(fix.makeCreateReq("Old Name", "old-name"));
    REQUIRE(created.ok());

    auto openReq = fix.makeOpenReq(scrivi::ObjectKind::character, created.value().objectID);
    auto opened = fix.core.openObject(openReq);
    REQUIRE(opened.ok());

    auto updated = std::get<scrivi::CharacterObject>(opened.value().object);
    updated.displayName = "New Name";
    updated.notes       = "Updated notes.";
    updated.tags        = {"protagonist"};

    scrivi::SaveObjectRequest saveReq;
    saveReq.projectRootPath = fix.projectDir.string();
    saveReq.object          = updated;
    saveReq.author          = fix.author;

    auto saved = fix.core.saveObject(saveReq);
    REQUIRE(saved.ok());
    REQUIRE(saved.value().saved == true);

    auto reopened = fix.core.openObject(openReq);
    REQUIRE(reopened.ok());
    auto& ch = std::get<scrivi::CharacterObject>(reopened.value().object);
    REQUIRE(ch.displayName     == "New Name");
    REQUIRE(ch.notes           == "Updated notes.");
    REQUIRE(ch.tags.size()     == 1);
    REQUIRE(ch.tags[0]         == "protagonist");
}

TEST_CASE("deleteObject removes the file; subsequent openObject fails",
          "[integration][EP-005][T-0036]")
{
    ObjectFixture fix;

    auto created = fix.core.createObject(fix.makeCreateReq("To Be Deleted", "to-be-deleted"));
    REQUIRE(created.ok());
    REQUIRE(fs::exists(created.value().path));

    auto delReq = fix.makeDeleteReq(scrivi::ObjectKind::character, created.value().objectID);

    auto deleted = fix.core.deleteObject(delReq);
    REQUIRE(deleted.ok());
    REQUIRE(deleted.value().deleted == true);
    REQUIRE_FALSE(fs::exists(created.value().path));

    auto openReq = fix.makeOpenReq(scrivi::ObjectKind::character, created.value().objectID);
    auto reopened = fix.core.openObject(openReq);
    REQUIRE_FALSE(reopened.ok());
}

TEST_CASE("createObject with duplicate slug returns failure",
          "[integration][EP-005][T-0036]")
{
    ObjectFixture fix;

    auto first = fix.core.createObject(fix.makeCreateReq("Ada Thornwood", "ada"));
    REQUIRE(first.ok());

    auto second = fix.core.createObject(fix.makeCreateReq("Another Ada", "ada"));
    REQUIRE_FALSE(second.ok());
    REQUIRE(second.error().code == scrivi::ErrorCode::invalidArgument);
}

// ---------------------------------------------------------------------------
// Remaining object types - T-0039
// ---------------------------------------------------------------------------

template<typename T>
static void runCrudCycle(ObjectFixture& fix,
                          scrivi::ObjectKind kind,
                          const std::string& displayName,
                          const std::string& slug,
                          const std::string& subdir)
{
    // create
    auto created = fix.core.createObject(fix.makeCreateReq(kind, displayName, slug));
    REQUIRE(created.ok());
    REQUIRE(created.value().slug == slug);
    INFO("path: " << created.value().path);
    REQUIRE(fs::exists(created.value().path));
    REQUIRE(created.value().path.find(subdir) != std::string::npos);

    // open
    auto openReq = fix.makeOpenReq(kind, created.value().objectID);

    auto opened = fix.core.openObject(openReq);
    REQUIRE(opened.ok());
    auto& obj = std::get<T>(opened.value().object);
    REQUIRE(obj.displayName    == displayName);
    REQUIRE(obj.slug           == slug);
    REQUIRE(obj.status         == "active");
    REQUIRE(obj.objectID.value == created.value().objectID.value);

    // save
    auto updated       = obj;
    updated.notes      = "updated notes";
    updated.tags       = {"test-tag"};

    scrivi::SaveObjectRequest saveReq;
    saveReq.projectRootPath = fix.projectDir.string();
    saveReq.object          = updated;
    saveReq.author          = fix.author;

    auto saved = fix.core.saveObject(saveReq);
    REQUIRE(saved.ok());
    REQUIRE(saved.value().saved == true);

    auto reopened = fix.core.openObject(openReq);
    REQUIRE(reopened.ok());
    auto& rObj = std::get<T>(reopened.value().object);
    REQUIRE(rObj.notes        == "updated notes");
    REQUIRE(rObj.tags.size()  == 1);
    REQUIRE(rObj.tags[0]      == "test-tag");

    // delete
    auto delReq = fix.makeDeleteReq(kind, created.value().objectID);

    auto deleted = fix.core.deleteObject(delReq);
    REQUIRE(deleted.ok());
    REQUIRE(deleted.value().deleted == true);
    REQUIRE_FALSE(fs::exists(created.value().path));

    auto afterDelete = fix.core.openObject(openReq);
    REQUIRE_FALSE(afterDelete.ok());
}

TEST_CASE("LocationObject full CRUD cycle",
          "[integration][EP-005][T-0039]")
{
    ObjectFixture fix;
    runCrudCycle<scrivi::LocationObject>(
        fix, scrivi::ObjectKind::location,
        "Old Watchtower", "old-watchtower", "locations");
}

TEST_CASE("ItemObject full CRUD cycle",
          "[integration][EP-005][T-0039]")
{
    ObjectFixture fix;
    runCrudCycle<scrivi::ItemObject>(
        fix, scrivi::ObjectKind::item,
        "Brass Key", "brass-key", "items");
}

// NB: `rule` had a project-scoped CRUD cycle here until SP-097 (T-0404) moved
// it to world scope. Its round-trip now lives in WorldTests.cpp alongside the
// other world-scoped kinds, since it needs a bound world to exist in.

// --- SP-095 T-0370: the new project-scoped kinds ----------------------------

TEST_CASE("BuildingObject full CRUD cycle", "[integration][T-0370]")
{
    ObjectFixture fix;
    runCrudCycle<scrivi::BuildingObject>(
        fix, scrivi::ObjectKind::building,
        "Old Watchtower", "old-watchtower", "buildings");
}

TEST_CASE("VehicleObject full CRUD cycle", "[integration][T-0370]")
{
    ObjectFixture fix;
    runCrudCycle<scrivi::VehicleObject>(
        fix, scrivi::ObjectKind::vehicle,
        "The Kestrel", "the-kestrel", "vehicles");
}

TEST_CASE("MapObject full CRUD cycle", "[integration][T-0370]")
{
    ObjectFixture fix;
    runCrudCycle<scrivi::MapObject>(
        fix, scrivi::ObjectKind::map,
        "Northern Reaches", "northern-reaches", "maps");
}

TEST_CASE("world-scoped kinds are refused when NO world is supplied",
          "[integration][T-0370][T-0385][T-0409]")
{
    ObjectFixture fix;

    // ⚠️ WIDENED for SP-103 / T-0409: this used to cover only the four
    // originally-world-scoped kinds. ALL TEN worldbuilding kinds are now
    // world-scoped (Doc 1 §3.0), so every one of them must refuse without a
    // world — and the refusal must stay explicit, because World Data Separation
    // v0.1 §7 still writes NO migration code. An object created under objects/
    // could never be moved into world scope later, so a silent fallback would
    // strand it permanently.
    //
    // The request is built by hand rather than via makeCreateReq(), which now
    // supplies the fixture's world; the point here is the ABSENCE of one.
    for (auto kind : {scrivi::ObjectKind::artifact,
                      scrivi::ObjectKind::building,
                      scrivi::ObjectKind::character,
                      scrivi::ObjectKind::chronicle,
                      scrivi::ObjectKind::faction,
                      scrivi::ObjectKind::item,
                      scrivi::ObjectKind::location,
                      scrivi::ObjectKind::map,
                      scrivi::ObjectKind::rule,
                      scrivi::ObjectKind::vehicle}) {
        CAPTURE(scrivi::objectKindName(kind));

        scrivi::CreateObjectRequest req;
        req.projectRootPath = fix.projectDir.string();
        req.objectKind      = kind;
        req.displayName     = "Thing";
        req.slug            = "thing";
        req.author          = fix.author;
        // req.worldID deliberately left EMPTY.

        auto r = fix.core.createObject(req);
        REQUIRE_FALSE(r.ok());
        REQUIRE(r.error().code == scrivi::ErrorCode::invalidArgument);
        // The message must name the reason, not just fail.
        REQUIRE(r.error().message.find("world") != std::string::npos);
        // And it must carry the machine-readable discriminator the app reads to
        // offer "create a world" rather than showing a generic error (T-0410).
        REQUIRE(r.error().detail == "worldRequired");

        // Nothing was written on the way to the refusal.
        REQUIRE_FALSE(fs::exists(fix.projectDir / "objects" /
                                 scrivi::objectKindSubdir(kind)));
    }
}

TEST_CASE("`source` is the sole kind creatable with NO world",
          "[integration][T-0409]")
{
    ObjectFixture fix;

    // The other half of the ruling: a citation documents a real-world
    // publication supporting THIS manuscript, so it stays project-scoped and
    // must NOT require a world.
    scrivi::CreateObjectRequest req;
    req.projectRootPath = fix.projectDir.string();
    req.objectKind      = scrivi::ObjectKind::source;
    req.displayName     = "On the Origin of Species";
    req.slug            = "origin";
    req.author          = fix.author;

    auto r = fix.core.createObject(req);
    REQUIRE(r.ok());
    REQUIRE(fs::exists(fix.projectDir / "objects" / "sources"));
}

TEST_CASE("the world container kind is not creatable as an object",
          "[integration][T-0370]")
{
    ObjectFixture fix;

    // Worlds are created by scrivi_create_world (SP-098), never through the
    // object CRUD path.
    auto r = fix.core.createObject(
        fix.makeCreateReq(scrivi::ObjectKind::world, "Midgard", "midgard"));
    REQUIRE_FALSE(r.ok());
    REQUIRE(r.error().code == scrivi::ErrorCode::invalidArgument);
}

TEST_CASE("retiring the timeline kind leaves the project timeline intact",
          "[integration][T-0370]")
{
    ObjectFixture fix;

    // objects/timelines/ is SHARED: ObjectKind::timeline (retired in SP-095)
    // and the Timeline Panel's timeline.meta.json (very much alive) occupied
    // the same directory. Retiring the kind must not disturb the panel's file,
    // which ProjectCreator seeds into every new project.
    const auto metaPath = fix.projectDir / "objects" / "timelines" / "timeline.meta.json";
    REQUIRE(fs::exists(metaPath));

    // And it is still the Timeline Panel's schema, not an object schema.
    std::ifstream in(metaPath);
    std::string   body((std::istreambuf_iterator<char>(in)),
                        std::istreambuf_iterator<char>());
    REQUIRE(body.find("scrivi.timeline") != std::string::npos);
}

TEST_CASE("Objects of different kinds with same slug do not conflict",
          "[integration][EP-005][T-0039]")
{
    ObjectFixture fix;

    auto ch = fix.core.createObject(fix.makeCreateReq(scrivi::ObjectKind::character, "Shared Name", "shared"));
    auto lo = fix.core.createObject(fix.makeCreateReq(scrivi::ObjectKind::location,  "Shared Name", "shared"));
    REQUIRE(ch.ok());
    REQUIRE(lo.ok());
    REQUIRE(ch.value().path != lo.value().path);
}
