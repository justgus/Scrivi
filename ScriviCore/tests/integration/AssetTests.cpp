#include <catch2/catch_test_macros.hpp>

#include "scrivi/ScriviCore.hpp"
#include "scrivi/AssetTypes.hpp"
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

struct AssetFixture {
    fs::path projectDir;
    fs::path appSupportDir;

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

    AssetFixture()
        : projectDir(fs::temp_directory_path() /
                     ("scrivi-asset-test-" + std::to_string(
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
        req.title           = "Asset Test Project";
        req.slug            = "asset-test-project";
        req.author          = author;
        (void)core.createProject(req);
    }

    ~AssetFixture() { fs::remove_all(projectDir); }

    // Write a small synthetic file and return its absolute path.
    fs::path writeSyntheticFile(const std::string& name,
                                 const std::string& content = "FAKE_BYTES") const
    {
        auto path = projectDir / name;
        std::ofstream f(path, std::ios::binary);
        f << content;
        return path;
    }

    scrivi::ImportAssetRequest makeImportReq(const fs::path& sourcePath,
                                              scrivi::AssetCategory cat,
                                              const std::string& title = "Test Asset") const
    {
        scrivi::ImportAssetRequest req;
        req.projectRootPath = projectDir.string();
        req.sourcePath      = sourcePath.string();
        req.category        = cat;
        req.title           = title;
        req.author          = author;
        return req;
    }
};

// ---------------------------------------------------------------------------
// T-0042: importAsset - file and sidecar written
// ---------------------------------------------------------------------------

TEST_CASE_METHOD(AssetFixture, "importAsset writes binary file and sidecar to assets/<category>/",
                 "[integration][T-0042]")
{
    auto src = writeSyntheticFile("map.png");

    auto result = core.importAsset(makeImportReq(src, scrivi::AssetCategory::image, "Map of the Realm"));
    REQUIRE(result.ok());
    REQUIRE_FALSE(result.value().assetID.empty());

    // Binary file exists.
    REQUIRE(fs::exists(result.value().assetPath));

    // Sidecar exists.
    REQUIRE(fs::exists(result.value().sidecarPath));

    // Asset is in assets/images/
    auto expectedDir = (fs::path(projectDir) / "assets" / "images").string();
    REQUIRE(result.value().assetPath.find("assets/images") != std::string::npos);

    // Sidecar has the .meta.json suffix.
    REQUIRE(result.value().sidecarPath.ends_with(".meta.json"));
}

// ---------------------------------------------------------------------------
// T-0042: listAssets - imported assets are returned
// ---------------------------------------------------------------------------

TEST_CASE_METHOD(AssetFixture, "listAssets returns all imported assets",
                 "[integration][T-0042]")
{
    auto src1 = writeSyntheticFile("map.png",   "FAKE_PNG");
    auto src2 = writeSyntheticFile("theme.mp3", "FAKE_MP3");

    auto r1 = core.importAsset(makeImportReq(src1, scrivi::AssetCategory::image, "Map"));
    auto r2 = core.importAsset(makeImportReq(src2, scrivi::AssetCategory::audio, "Theme Music"));
    REQUIRE(r1.ok());
    REQUIRE(r2.ok());

    // List all assets.
    scrivi::ListAssetsRequest listReq;
    listReq.projectRootPath = projectDir.string();

    auto listResult = core.listAssets(listReq);
    REQUIRE(listResult.ok());
    REQUIRE(listResult.value().assets.size() == 2);

    // Both assetIDs are present.
    std::vector<std::string> ids;
    for (const auto& a : listResult.value().assets) ids.push_back(a.assetID);
    REQUIRE(std::find(ids.begin(), ids.end(), r1.value().assetID) != ids.end());
    REQUIRE(std::find(ids.begin(), ids.end(), r2.value().assetID) != ids.end());
}

TEST_CASE_METHOD(AssetFixture, "listAssets with category filter returns only matching assets",
                 "[integration][T-0042]")
{
    auto src1 = writeSyntheticFile("map.png",   "FAKE_PNG");
    auto src2 = writeSyntheticFile("theme.mp3", "FAKE_MP3");

    REQUIRE(core.importAsset(makeImportReq(src1, scrivi::AssetCategory::image)).ok());
    REQUIRE(core.importAsset(makeImportReq(src2, scrivi::AssetCategory::audio)).ok());

    scrivi::ListAssetsRequest listReq;
    listReq.projectRootPath = projectDir.string();
    listReq.category        = scrivi::AssetCategory::image;

    auto listResult = core.listAssets(listReq);
    REQUIRE(listResult.ok());
    REQUIRE(listResult.value().assets.size() == 1);
    REQUIRE(listResult.value().assets[0].category == scrivi::AssetCategory::image);
}

// ---------------------------------------------------------------------------
// T-0042: removeAsset - both binary and sidecar deleted
// ---------------------------------------------------------------------------

TEST_CASE_METHOD(AssetFixture, "removeAsset deletes binary file and sidecar",
                 "[integration][T-0042]")
{
    auto src = writeSyntheticFile("map.png");

    auto importResult = core.importAsset(makeImportReq(src, scrivi::AssetCategory::image));
    REQUIRE(importResult.ok());

    auto assetPath   = importResult.value().assetPath;
    auto sidecarPath = importResult.value().sidecarPath;
    REQUIRE(fs::exists(assetPath));
    REQUIRE(fs::exists(sidecarPath));

    scrivi::RemoveAssetRequest removeReq;
    removeReq.projectRootPath = projectDir.string();
    removeReq.assetID         = importResult.value().assetID;

    auto removeResult = core.removeAsset(removeReq);
    REQUIRE(removeResult.ok());
    REQUIRE(removeResult.value().deleted == true);

    REQUIRE_FALSE(fs::exists(assetPath));
    REQUIRE_FALSE(fs::exists(sidecarPath));

    // List should now be empty.
    scrivi::ListAssetsRequest listReq;
    listReq.projectRootPath = projectDir.string();
    auto listResult = core.listAssets(listReq);
    REQUIRE(listResult.ok());
    REQUIRE(listResult.value().assets.empty());
}
