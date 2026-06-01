#include <catch2/catch_test_macros.hpp>

#include "scrivi/ScriviCore.hpp"
#include "scrivi/Requests.hpp"
#include "scrivi/Results.hpp"

#include "mocks/DeterministicUUIDProvider.hpp"
#include "mocks/FixedClock.hpp"
#include "mocks/MockGitProvider.hpp"
#include "mocks/MockSecureStore.hpp"
#include "platform/LocalFileSystem.hpp"

#include <chrono>
#include <filesystem>
#include <fstream>
#include <string>

namespace fs = std::filesystem;

// ---------------------------------------------------------------------------
// Fixture
// ---------------------------------------------------------------------------

struct InboxFixture {
    fs::path projectDir;
    fs::path appSupportDir;
    fs::path droppedFilesDir;

    scrivi::platform::LocalFileSystem        fileSystem;
    scrivi::mocks::DeterministicUUIDProvider uuidProvider;
    scrivi::mocks::FixedClock                clock{"2026-05-28T10:00:00Z"};
    scrivi::mocks::MockGitProvider           gitProvider;
    scrivi::mocks::MockSecureStore           secureStore;
    scrivi::ScriviCore                       core;

    const scrivi::AuthorshipRef author{
        scrivi::IdentityID{"identity-001"},
        scrivi::PersonaID {"persona-001"},
        "Test Author"
    };

    InboxFixture()
        : projectDir(fs::temp_directory_path() /
                     ("scrivi-inbox-test-" + std::to_string(
                         std::chrono::steady_clock::now().time_since_epoch().count())))
        , appSupportDir(projectDir / "appsupport")
        , droppedFilesDir(projectDir / "inbox" / "dropped-files")
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
        req.title           = "Inbox Test Project";
        req.slug            = "inbox-test-project";
        req.author          = author;
        (void)core.createProject(req);
    }

    ~InboxFixture() { fs::remove_all(projectDir); }

    // Drop a file into inbox/dropped-files/ and return its filename.
    std::string dropFile(const std::string& filename,
                         const std::string& content = "FAKE_BYTES") const
    {
        std::ofstream f(droppedFilesDir / filename, std::ios::binary);
        f << content;
        return filename;
    }
};

// ---------------------------------------------------------------------------
// T-0046: ProjectCreator creates inbox/dropped-files/
// ---------------------------------------------------------------------------

TEST_CASE_METHOD(InboxFixture, "createProject creates inbox/dropped-files/ directory",
                 "[integration][T-0046]")
{
    REQUIRE(fs::exists(droppedFilesDir));
    REQUIRE(fs::is_directory(droppedFilesDir));
}

// ---------------------------------------------------------------------------
// T-0046: listInbox - empty inbox returns empty list
// ---------------------------------------------------------------------------

TEST_CASE_METHOD(InboxFixture, "listInbox returns empty list when inbox is empty",
                 "[integration][T-0046]")
{
    scrivi::ListInboxRequest req;
    req.projectRootPath = projectDir.string();

    auto result = core.listInbox(req);
    REQUIRE(result.ok());
    REQUIRE(result.value().entries.empty());
}

// ---------------------------------------------------------------------------
// T-0046: listInbox - dropped files are returned
// ---------------------------------------------------------------------------

TEST_CASE_METHOD(InboxFixture, "listInbox returns all files in dropped-files/",
                 "[integration][T-0046]")
{
    dropFile("photo.jpg");
    dropFile("notes.pdf");

    scrivi::ListInboxRequest req;
    req.projectRootPath = projectDir.string();

    auto result = core.listInbox(req);
    REQUIRE(result.ok());
    REQUIRE(result.value().entries.size() == 2);

    std::vector<std::string> names;
    for (const auto& e : result.value().entries) names.push_back(e.filename);
    REQUIRE(std::find(names.begin(), names.end(), "photo.jpg")  != names.end());
    REQUIRE(std::find(names.begin(), names.end(), "notes.pdf") != names.end());
}

// ---------------------------------------------------------------------------
// T-0046: importFromInbox - importAsAsset moves file into assets/ and removes from inbox
// ---------------------------------------------------------------------------

TEST_CASE_METHOD(InboxFixture, "importFromInbox importAsAsset moves file to assets and clears inbox entry",
                 "[integration][T-0046]")
{
    dropFile("hero.png", "PNG_BYTES");

    scrivi::ImportFromInboxRequest req;
    req.projectRootPath = projectDir.string();
    req.filename        = "hero.png";
    req.action          = scrivi::InboxAction::importAsAsset;
    req.assetCategory   = scrivi::AssetCategory::image;
    req.author          = author;

    auto result = core.importFromInbox(req);
    REQUIRE(result.ok());
    REQUIRE(result.value().actionTaken == "importAsAsset");
    REQUIRE_FALSE(result.value().assetID.empty());
    REQUIRE_FALSE(result.value().resultPath.empty());

    // File landed in assets/images/
    REQUIRE(fs::exists(result.value().resultPath));
    REQUIRE(result.value().resultPath.find("assets/images") != std::string::npos);

    // Inbox is now empty.
    scrivi::ListInboxRequest listReq;
    listReq.projectRootPath = projectDir.string();
    auto listResult = core.listInbox(listReq);
    REQUIRE(listResult.ok());
    REQUIRE(listResult.value().entries.empty());
}

// ---------------------------------------------------------------------------
// T-0046: importFromInbox - ignore leaves file in inbox
// ---------------------------------------------------------------------------

TEST_CASE_METHOD(InboxFixture, "importFromInbox ignore leaves file untouched",
                 "[integration][T-0046]")
{
    dropFile("draft.txt");

    scrivi::ImportFromInboxRequest req;
    req.projectRootPath = projectDir.string();
    req.filename        = "draft.txt";
    req.action          = scrivi::InboxAction::ignore;
    req.author          = author;

    auto result = core.importFromInbox(req);
    REQUIRE(result.ok());
    REQUIRE(result.value().actionTaken == "ignored");
    REQUIRE(result.value().resultPath.empty());

    // File still in inbox.
    REQUIRE(fs::exists(droppedFilesDir / "draft.txt"));
}

// ---------------------------------------------------------------------------
// T-0046: importFromInbox - deleteFile removes file from inbox
// ---------------------------------------------------------------------------

TEST_CASE_METHOD(InboxFixture, "importFromInbox deleteFile removes file from dropped-files",
                 "[integration][T-0046]")
{
    dropFile("junk.bin");
    REQUIRE(fs::exists(droppedFilesDir / "junk.bin"));

    scrivi::ImportFromInboxRequest req;
    req.projectRootPath = projectDir.string();
    req.filename        = "junk.bin";
    req.action          = scrivi::InboxAction::deleteFile;
    req.author          = author;

    auto result = core.importFromInbox(req);
    REQUIRE(result.ok());
    REQUIRE(result.value().actionTaken == "deleted");

    REQUIRE_FALSE(fs::exists(droppedFilesDir / "junk.bin"));
}
