#include <catch2/catch_test_macros.hpp>

#include "scrivi/ScriviCore.hpp"
#include "scrivi/Requests.hpp"

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

    scrivi::platform::LocalFileSystem    fileSystem;
    scrivi::mocks::DeterministicUUIDProvider uuidProvider;
    scrivi::mocks::FixedClock            clock;
    scrivi::mocks::MockGitProvider       gitProvider;
    scrivi::mocks::MockSecureStore       secureStore;
    scrivi::ScriviCore                   core;

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

        // Bootstrap a minimal project so the root exists
        scrivi::CreateProjectRequest req;
        req.projectRootPath = projectDir.string();
        req.appSupportRoot  = appSupportDir.string();
        req.title           = "Object Test Project";
        req.slug            = "object-test-project";
        req.author          = author;
        (void)core.createProject(req);
    }

    ~ObjectFixture() {
        fs::remove_all(projectDir);
    }

    scrivi::CreateObjectRequest makeCreateReq(
        const std::string& displayName,
        const std::string& slug = "") const
    {
        scrivi::CreateObjectRequest req;
        req.projectRootPath = projectDir.string();
        req.displayName     = displayName;
        req.slug            = slug;
        req.author          = author;
        return req;
    }
};

// ---------------------------------------------------------------------------
// Tests
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
    req.objectID        = created.value().objectID;

    auto opened = fix.core.openObject(req);
    REQUIRE(opened.ok());
    REQUIRE(opened.value().object.displayName == "Thomas Belacroix");
    REQUIRE(opened.value().object.slug        == "thomas");
    REQUIRE(opened.value().object.status      == "active");
    REQUIRE(opened.value().object.objectID.value == created.value().objectID.value);
}

TEST_CASE("saveObject updates displayName and modifiedAt on disk",
          "[integration][EP-005][T-0036]")
{
    ObjectFixture fix;

    auto created = fix.core.createObject(fix.makeCreateReq("Old Name", "old-name"));
    REQUIRE(created.ok());

    // Open to get the full object
    scrivi::OpenObjectRequest openReq;
    openReq.projectRootPath = fix.projectDir.string();
    openReq.objectID        = created.value().objectID;
    auto opened = fix.core.openObject(openReq);
    REQUIRE(opened.ok());

    // Mutate and save
    scrivi::CharacterObject updated  = opened.value().object;
    updated.displayName              = "New Name";
    updated.notes                    = "Updated notes.";
    updated.tags                     = {"protagonist"};

    scrivi::SaveObjectRequest saveReq;
    saveReq.projectRootPath = fix.projectDir.string();
    saveReq.object          = updated;
    saveReq.author          = fix.author;

    auto saved = fix.core.saveObject(saveReq);
    REQUIRE(saved.ok());
    REQUIRE(saved.value().saved == true);

    // Re-open and verify
    auto reopened = fix.core.openObject(openReq);
    REQUIRE(reopened.ok());
    REQUIRE(reopened.value().object.displayName == "New Name");
    REQUIRE(reopened.value().object.notes       == "Updated notes.");
    REQUIRE(reopened.value().object.tags.size() == 1);
    REQUIRE(reopened.value().object.tags[0]     == "protagonist");
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
    delReq.objectID        = created.value().objectID;

    auto deleted = fix.core.deleteObject(delReq);
    REQUIRE(deleted.ok());
    REQUIRE(deleted.value().deleted == true);
    REQUIRE_FALSE(fs::exists(created.value().path));

    // openObject on a deleted character must fail
    scrivi::OpenObjectRequest openReq;
    openReq.projectRootPath = fix.projectDir.string();
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
