// ApplyRepairTests.cpp
// Integration tests for ScriviCore::applyRepair() - one test per action kind
// (8 manuscript handlers) plus 2 rename-detection tests via scanForExternalChanges.
// All tests use real filesystem (TempDir) + mock services.

#include <catch2/catch_test_macros.hpp>

#include "scrivi/ScriviCore.hpp"
#include "scrivi/Requests.hpp"

#include "mocks/DeterministicUUIDProvider.hpp"
#include "mocks/FixedClock.hpp"
#include "mocks/MockGitProvider.hpp"
#include "mocks/MockSecureStore.hpp"

#include "platform/LocalFileSystem.hpp"
#include "repair/RepairHandlers.hpp"   // for direct handler call in test 6

#include <chrono>
#include <filesystem>
#include <fstream>
#include <sstream>
#include <string>

namespace fs = std::filesystem;

// ---------------------------------------------------------------------------
// Shared test infrastructure
// ---------------------------------------------------------------------------

struct TempDir {
    fs::path path;
    TempDir() {
        path = fs::temp_directory_path() /
               ("scrivi-repair-test-" + std::to_string(
                   std::chrono::steady_clock::now().time_since_epoch().count()));
        fs::create_directories(path);
    }
    ~TempDir() { std::error_code ec; fs::remove_all(path, ec); }
    std::string str()                       const { return path.string(); }
    std::string sub(const std::string& rel) const { return (path / rel).string(); }
};

struct RepairFixture {
    TempDir projectDir;
    TempDir appSupportDir;

    scrivi::platform::LocalFileSystem        lfs;
    scrivi::mocks::DeterministicUUIDProvider uuids;
    scrivi::mocks::FixedClock                clock{"2026-05-27T10:00:00Z"};
    scrivi::mocks::MockSecureStore           store;
    scrivi::mocks::MockGitProvider           git;

    scrivi::CoreServices services;
    scrivi::ScriviCore   core;

    scrivi::AuthorshipRef author{
        scrivi::IdentityID{"identity-001"},
        scrivi::PersonaID {"persona-001"},
        "Test Author"
    };

    // Paths set after createProject
    std::string firstSceneMetaPath;
    std::string firstSceneContentPath;
    std::string firstSceneID;

    explicit RepairFixture() : core([&]{
        services.fileSystem   = &lfs;
        services.uuidProvider = &uuids;
        services.clock        = &clock;
        services.secureStore  = &store;
        services.gitProvider  = &git;
        return services;
    }()) {
        scrivi::CreateProjectRequest req;
        req.projectRootPath = projectDir.str();
        req.appSupportRoot  = appSupportDir.str();
        req.title           = "Repair Test";
        req.slug            = "repair-test";
        req.author          = author;
        auto r = core.createProject(req);
        REQUIRE(r.ok());
        firstSceneMetaPath    = r.value().firstSceneMetadataPath;
        firstSceneContentPath = r.value().firstSceneContentPath;
        firstSceneID          = r.value().firstSceneID.value;
    }

    // Return the first issue with a matching category from a fresh scan.
    scrivi::RepairIssue scanForIssue(scrivi::RepairCategory cat) {
        scrivi::ExternalChangeScanRequest req;
        req.projectRootPath  = projectDir.str();
        req.appSupportRoot   = appSupportDir.str();
        req.includeGitStatus = false;
        auto r = core.scanForExternalChanges(req);
        REQUIRE(r.ok());
        for (auto& i : r.value().repairIssues)
            if (i.category == cat) return i;
        FAIL("No issue with requested category found in scan");
        return {};
    }

    // Convenience: run a full scan and verify zero issues remain.
    void checkClean() {
        scrivi::ExternalChangeScanRequest req;
        req.projectRootPath  = projectDir.str();
        req.appSupportRoot   = appSupportDir.str();
        req.includeGitStatus = false;
        auto r = core.scanForExternalChanges(req);
        REQUIRE(r.ok());
        CHECK(r.value().repairIssues.empty());
    }

    scrivi::ApplyRepairRequest makeRepairReq(
        const std::string&       issueID,
        scrivi::RepairActionKind kind,
        const std::string&       targetPath = {}) const
    {
        scrivi::ApplyRepairRequest req;
        req.issueID         = issueID;
        req.projectRootPath = projectDir.str();
        req.appSupportRoot  = appSupportDir.str();
        req.actionKind      = kind;
        req.targetPath      = targetPath;
        req.author          = author;
        return req;
    }

    // Read a file to string.
    static std::string readFile(const std::string& path) {
        std::ifstream in(path);
        std::ostringstream ss;
        ss << in.rdbuf();
        return ss.str();
    }
};

// ---------------------------------------------------------------------------
// 1. relinkToFile
// ---------------------------------------------------------------------------
TEST_CASE("applyRepair - relinkToFile updates contentPath in scene metadata",
          "[integration][EP-004][T-0032]")
{
    RepairFixture f;

    // Create a new content file; remove the original
    const std::string newContentAbs =
        f.projectDir.sub("manuscript/chapter-001/renamed-scene.md");
    { std::ofstream out(newContentAbs, std::ios::binary); out << "# Renamed\n\nSome content."; }
    fs::remove(f.projectDir.sub("manuscript/chapter-001/001-opening-scene.md"));

    auto issue = f.scanForIssue(scrivi::RepairCategory::missingContent);
    CHECK(issue.sceneID.value == f.firstSceneID);

    auto result = f.core.applyRepair(
        f.makeRepairReq(issue.issueID, scrivi::RepairActionKind::relinkToFile, newContentAbs));
    REQUIRE(result.ok());
    CHECK(result.value().resolved == true);
    CHECK(result.value().actionApplied == scrivi::RepairActionKind::relinkToFile);

    f.checkClean();
}

// ---------------------------------------------------------------------------
// 2. createEmptyContentFile
// ---------------------------------------------------------------------------
TEST_CASE("applyRepair - createEmptyContentFile creates a .md at missing path",
          "[integration][EP-004][T-0032]")
{
    RepairFixture f;

    const std::string contentAbs =
        f.projectDir.sub("manuscript/chapter-001/001-opening-scene.md");
    fs::remove(contentAbs);

    auto issue = f.scanForIssue(scrivi::RepairCategory::missingContent);

    auto result = f.core.applyRepair(
        f.makeRepairReq(issue.issueID, scrivi::RepairActionKind::createEmptyContentFile));
    REQUIRE(result.ok());
    CHECK(result.value().resolved == true);
    CHECK(fs::exists(contentAbs));

    f.checkClean();
}

// ---------------------------------------------------------------------------
// 3. markMissing
// ---------------------------------------------------------------------------
TEST_CASE("applyRepair - markMissing sets scene status to 'missing'",
          "[integration][EP-004][T-0032]")
{
    RepairFixture f;

    fs::remove(f.projectDir.sub("manuscript/chapter-001/001-opening-scene.md"));
    auto issue = f.scanForIssue(scrivi::RepairCategory::missingContent);

    auto result = f.core.applyRepair(
        f.makeRepairReq(issue.issueID, scrivi::RepairActionKind::markMissing));
    REQUIRE(result.ok());
    CHECK(result.value().resolved == true);

    const std::string metaAbs =
        f.projectDir.sub("manuscript/chapter-001/001-opening-scene.meta.json");
    CHECK(RepairFixture::readFile(metaAbs).find("\"missing\"") != std::string::npos);
    CHECK(fs::exists(metaAbs + ".bak"));
}

// ---------------------------------------------------------------------------
// 4. removeFromProject
// ---------------------------------------------------------------------------
TEST_CASE("applyRepair - removeFromProject removes scene from chapter index",
          "[integration][EP-004][T-0032]")
{
    RepairFixture f;

    fs::remove(f.projectDir.sub("manuscript/chapter-001/001-opening-scene.md"));
    auto issue = f.scanForIssue(scrivi::RepairCategory::missingContent);

    auto result = f.core.applyRepair(
        f.makeRepairReq(issue.issueID, scrivi::RepairActionKind::removeFromProject));
    REQUIRE(result.ok());
    CHECK(result.value().resolved == true);

    const std::string chMetaAbs =
        f.projectDir.sub("manuscript/chapter-001/chapter.meta.json");
    // sceneID no longer referenced
    CHECK(RepairFixture::readFile(chMetaAbs).find(f.firstSceneID) == std::string::npos);
    // .bak created
    CHECK(fs::exists(chMetaAbs + ".bak"));
    // metadata file preserved on disk
    CHECK(fs::exists(f.projectDir.sub("manuscript/chapter-001/001-opening-scene.meta.json")));
}

// ---------------------------------------------------------------------------
// 5. moveToInbox
// ---------------------------------------------------------------------------
TEST_CASE("applyRepair - moveToInbox moves unregistered file to inbox/dropped-files/",
          "[integration][EP-004][T-0032]")
{
    RepairFixture f;

    const std::string orphanAbs =
        f.projectDir.sub("manuscript/chapter-001/orphan-scene.md");
    { std::ofstream out(orphanAbs, std::ios::binary); out << "An orphaned scene."; }

    auto issue = f.scanForIssue(scrivi::RepairCategory::unregisteredManuscriptFile);

    auto result = f.core.applyRepair(
        f.makeRepairReq(issue.issueID, scrivi::RepairActionKind::moveToInbox));
    REQUIRE(result.ok());
    CHECK(result.value().resolved == true);

    CHECK_FALSE(fs::exists(orphanAbs));
    CHECK(fs::exists(f.projectDir.sub("inbox/dropped-files/orphan-scene.md")));
}

// ---------------------------------------------------------------------------
// 6. reloadExternalVersion
// ---------------------------------------------------------------------------
// Tested via direct handler call: the handler is a free function in
// RepairHandlers.hpp (included above). No dispatcher-level suggestedActions
// validation is needed; we verify the observable contract: no write, content
// returned in detail.
TEST_CASE("applyRepair - reloadExternalVersion returns on-disk content unchanged",
          "[integration][EP-004][T-0032]")
{
    RepairFixture f;

    const std::string contentAbs =
        f.projectDir.sub("manuscript/chapter-001/001-opening-scene.md");
    const std::string expectedContent = "# Reloaded\n\nExternal content.";
    { std::ofstream out(contentAbs, std::ios::binary); out << expectedContent; }

    // Build a fake issue pointing at the content file
    scrivi::RepairIssue fakeIssue;
    fakeIssue.issueID  = "test-reload";
    fakeIssue.path     = contentAbs;
    fakeIssue.sceneID  = scrivi::SceneID{f.firstSceneID};
    fakeIssue.category = scrivi::RepairCategory::missingContent;

    scrivi::ApplyRepairRequest fakeReq;
    fakeReq.issueID         = "test-reload";
    fakeReq.projectRootPath = f.projectDir.str();
    fakeReq.appSupportRoot  = f.appSupportDir.str();
    fakeReq.actionKind      = scrivi::RepairActionKind::reloadExternalVersion;
    fakeReq.author          = f.author;

    scrivi::repair::HandlerContext hctx{fakeReq, fakeIssue, f.services};
    auto handlerResult = scrivi::repair::handleReloadExternalVersion(hctx);
    REQUIRE(handlerResult.ok());
    CHECK(handlerResult.value().resolved == true);
    CHECK(handlerResult.value().detail.find("External content") != std::string::npos);

    // Verify: file was NOT modified
    CHECK(RepairFixture::readFile(contentAbs) == expectedContent);
}

// ---------------------------------------------------------------------------
// 7. regenerateMetadata (scene)
// ---------------------------------------------------------------------------
TEST_CASE("applyRepair - regenerateMetadata (scene) creates new .meta.json",
          "[integration][EP-004][T-0032]")
{
    RepairFixture f;

    const std::string metaAbs =
        f.projectDir.sub("manuscript/chapter-001/001-opening-scene.meta.json");
    fs::remove(metaAbs);

    auto issue = f.scanForIssue(scrivi::RepairCategory::missingMetadata);
    CHECK(issue.sceneID.value == f.firstSceneID);

    auto result = f.core.applyRepair(
        f.makeRepairReq(issue.issueID, scrivi::RepairActionKind::regenerateMetadata));
    REQUIRE(result.ok());
    CHECK(result.value().resolved == true);
    CHECK(fs::exists(metaAbs));

    const auto content = RepairFixture::readFile(metaAbs);
    CHECK(content.find("scrivi.scene.v1") != std::string::npos);
    CHECK(content.find("scene-") != std::string::npos);
}

// ---------------------------------------------------------------------------
// 8. regenerateMetadata (chapter)
// ---------------------------------------------------------------------------
TEST_CASE("applyRepair - regenerateMetadata (chapter) creates new chapter.meta.json",
          "[integration][EP-004][T-0032]")
{
    RepairFixture f;

    const std::string chMetaAbs =
        f.projectDir.sub("manuscript/chapter-001/chapter.meta.json");
    fs::remove(chMetaAbs);

    // Chapter-level missingMetadata has empty sceneID
    auto issue = f.scanForIssue(scrivi::RepairCategory::missingMetadata);
    CHECK(issue.sceneID.value.empty());

    auto result = f.core.applyRepair(
        f.makeRepairReq(issue.issueID, scrivi::RepairActionKind::regenerateMetadata));
    REQUIRE(result.ok());
    CHECK(result.value().resolved == true);
    CHECK(fs::exists(chMetaAbs));

    const auto content = RepairFixture::readFile(chMetaAbs);
    CHECK(content.find("scrivi.chapter.v1") != std::string::npos);
    CHECK(content.find("ch-") != std::string::npos);
}

// ---------------------------------------------------------------------------
// 9. Rename detection - unambiguous metadata rename is auto-applied
// T-0031: scanner auto-applies when sceneID matches + single candidate in dir
// ---------------------------------------------------------------------------
TEST_CASE("scanForExternalChanges - unambiguous metadata rename is auto-applied",
          "[integration][EP-004][T-0031][T-0032]")
{
    RepairFixture f;

    const std::string origMeta =
        f.projectDir.sub("manuscript/chapter-001/001-opening-scene.meta.json");
    const std::string renamedMeta =
        f.projectDir.sub("manuscript/chapter-001/001-renamed-scene.meta.json");
    fs::rename(origMeta, renamedMeta);

    // After rename, scan should auto-apply and produce no missingMetadata issue
    // for this scene
    scrivi::ExternalChangeScanRequest scanReq;
    scanReq.projectRootPath  = f.projectDir.str();
    scanReq.appSupportRoot   = f.appSupportDir.str();
    scanReq.includeGitStatus = false;
    auto scanR = f.core.scanForExternalChanges(scanReq);
    REQUIRE(scanR.ok());

    for (auto& issue : scanR.value().repairIssues) {
        if (issue.category == scrivi::RepairCategory::missingMetadata &&
            issue.sceneID.value == f.firstSceneID)
        {
            FAIL("missingMetadata for renamed scene should have been auto-resolved");
        }
    }

    // chapter.meta.json now references the renamed path
    const std::string chMetaAbs =
        f.projectDir.sub("manuscript/chapter-001/chapter.meta.json");
    CHECK(RepairFixture::readFile(chMetaAbs).find("001-renamed-scene.meta.json") !=
          std::string::npos);
    CHECK(fs::exists(chMetaAbs + ".bak"));
}

// ---------------------------------------------------------------------------
// 10. Rename detection - ambiguous metadata rename stages possibleRename issue
// T-0031: when two candidates share the same sceneID, issue is staged
// ---------------------------------------------------------------------------
TEST_CASE("scanForExternalChanges - ambiguous metadata rename stages possibleRename",
          "[integration][EP-004][T-0031][T-0032]")
{
    RepairFixture f;

    const std::string origMeta =
        f.projectDir.sub("manuscript/chapter-001/001-opening-scene.meta.json");
    const std::string candidateA =
        f.projectDir.sub("manuscript/chapter-001/001-candidate-a.meta.json");
    const std::string candidateB =
        f.projectDir.sub("manuscript/chapter-001/001-candidate-b.meta.json");

    // Rename original to candidate A; copy to candidate B (same sceneID, two candidates)
    fs::rename(origMeta, candidateA);
    { std::ifstream src(candidateA, std::ios::binary); std::ofstream dst(candidateB, std::ios::binary); dst << src.rdbuf(); }

    scrivi::ExternalChangeScanRequest scanReq;
    scanReq.projectRootPath  = f.projectDir.str();
    scanReq.appSupportRoot   = f.appSupportDir.str();
    scanReq.includeGitStatus = false;
    auto scanR = f.core.scanForExternalChanges(scanReq);
    REQUIRE(scanR.ok());

    bool foundPossibleRename = false;
    for (auto& issue : scanR.value().repairIssues) {
        if (issue.category == scrivi::RepairCategory::possibleRename &&
            issue.sceneID.value == f.firstSceneID)
        {
            foundPossibleRename = true;
            CHECK(issue.severity == scrivi::RepairSeverity::warning);
            CHECK_FALSE(issue.suggestedActions.empty());
        }
    }
    CHECK(foundPossibleRename);

    // Chapter meta must NOT have been rewritten (no auto-apply on ambiguous)
    const std::string chMetaAbs =
        f.projectDir.sub("manuscript/chapter-001/chapter.meta.json");
    CHECK_FALSE(fs::exists(chMetaAbs + ".bak"));
}
