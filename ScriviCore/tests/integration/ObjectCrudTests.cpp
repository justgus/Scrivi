#include <catch2/catch_test_macros.hpp>

#include "scrivi/ScriviCore.hpp"
#include "scrivi/Requests.hpp"
#include "scrivi/ObjectTypes.hpp"

#include "mocks/DeterministicUUIDProvider.hpp"
#include "mocks/FixedClock.hpp"
#include "mocks/MockGitProvider.hpp"
#include "mocks/MockSecureStore.hpp"
#include "platform/LocalFileSystem.hpp"

#include <filesystem>
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
    scrivi::ScriviCore                      core;

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
        req.title           = "Object Test Project";
        req.slug            = "object-test-project";
        req.author          = author;
        (void)core.createProject(req);
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
// Character tests (T-0036 — unchanged behaviour)
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

    scrivi::OpenObjectRequest req;
    req.projectRootPath = fix.projectDir.string();
    req.objectKind      = scrivi::ObjectKind::character;
    req.objectID        = created.value().objectID;

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

    scrivi::OpenObjectRequest openReq;
    openReq.projectRootPath = fix.projectDir.string();
    openReq.objectKind      = scrivi::ObjectKind::character;
    openReq.objectID        = created.value().objectID;
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

    scrivi::DeleteObjectRequest delReq;
    delReq.projectRootPath = fix.projectDir.string();
    delReq.objectKind      = scrivi::ObjectKind::character;
    delReq.objectID        = created.value().objectID;

    auto deleted = fix.core.deleteObject(delReq);
    REQUIRE(deleted.ok());
    REQUIRE(deleted.value().deleted == true);
    REQUIRE_FALSE(fs::exists(created.value().path));

    scrivi::OpenObjectRequest openReq;
    openReq.projectRootPath = fix.projectDir.string();
    openReq.objectKind      = scrivi::ObjectKind::character;
    openReq.objectID        = created.value().objectID;
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
// Remaining object types — T-0039
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
    scrivi::OpenObjectRequest openReq;
    openReq.projectRootPath = fix.projectDir.string();
    openReq.objectKind      = kind;
    openReq.objectID        = created.value().objectID;

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
    scrivi::DeleteObjectRequest delReq;
    delReq.projectRootPath = fix.projectDir.string();
    delReq.objectKind      = kind;
    delReq.objectID        = created.value().objectID;

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

TEST_CASE("RuleObject full CRUD cycle",
          "[integration][EP-005][T-0039]")
{
    ObjectFixture fix;
    runCrudCycle<scrivi::RuleObject>(
        fix, scrivi::ObjectKind::rule,
        "Magic System", "magic-system", "rules");
}

TEST_CASE("TimelineObject full CRUD cycle",
          "[integration][EP-005][T-0039]")
{
    ObjectFixture fix;
    runCrudCycle<scrivi::TimelineObject>(
        fix, scrivi::ObjectKind::timeline,
        "Main Timeline", "main-timeline", "timelines");
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
