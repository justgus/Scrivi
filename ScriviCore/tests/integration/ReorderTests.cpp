// ReorderTests.cpp
// Integration tests for T-0101/T-0102: reorderScene and reorderChapter (EP-010 SP-029).

#include <catch2/catch_test_macros.hpp>

#include "scrivi/ScriviCore.hpp"
#include "scrivi/Requests.hpp"

#include "mocks/DeterministicUUIDProvider.hpp"
#include "mocks/FixedClock.hpp"
#include "mocks/MockGitProvider.hpp"
#include "mocks/MockSecureStore.hpp"

#include "platform/LocalFileSystem.hpp"
#include "manuscript/ManuscriptOrderResolver.hpp"
#include "util/PathUtils.hpp"

#include <chrono>
#include <filesystem>
#include <string>

namespace fs = std::filesystem;

// ---------------------------------------------------------------------------
// Helpers
// ---------------------------------------------------------------------------

struct TempDir {
    fs::path path;

    TempDir() {
        path = fs::temp_directory_path() / ("scrivi-reorder-" + std::to_string(
            std::chrono::steady_clock::now().time_since_epoch().count()));
        fs::create_directories(path);
    }

    ~TempDir() {
        std::error_code ec;
        fs::remove_all(path, ec);
    }

    std::string str() const { return path.string(); }
};

static scrivi::CoreServices makeServices(
    scrivi::platform::LocalFileSystem&        lfs,
    scrivi::mocks::DeterministicUUIDProvider& uuids,
    scrivi::mocks::FixedClock&                clock,
    scrivi::mocks::MockSecureStore&           store,
    scrivi::mocks::MockGitProvider&           git)
{
    scrivi::CoreServices s;
    s.fileSystem   = &lfs;
    s.uuidProvider = &uuids;
    s.clock        = &clock;
    s.secureStore  = &store;
    s.gitProvider  = &git;
    return s;
}

static scrivi::AuthorshipRef testAuthor()
{
    return {
        scrivi::IdentityID{"identity-001"},
        scrivi::PersonaID {"persona-001"},
        "Test Author"
    };
}

// Builds a project with two chapters, each containing two scenes.
// ch1: [s1, s2]   ch2: [s3, s4]
struct FourSceneProject {
    TempDir projectDir;
    TempDir appSupportDir;
    scrivi::platform::LocalFileSystem        lfs;
    scrivi::mocks::DeterministicUUIDProvider uuids;
    scrivi::mocks::FixedClock                clock{"2026-06-04T00:00:00Z"};
    scrivi::mocks::MockSecureStore           store;
    scrivi::mocks::MockGitProvider           git;
    scrivi::CoreServices                     services;
    scrivi::ScriviCore                       core;

    scrivi::ProjectID  projectID;
    scrivi::ChapterID  ch1ID, ch2ID;
    scrivi::SceneID    s1ID, s2ID, s3ID, s4ID;

    FourSceneProject()
        : services(makeServices(lfs, uuids, clock, store, git))
        , core(services)
    {
        scrivi::CreateProjectRequest req;
        req.projectRootPath = projectDir.str();
        req.appSupportRoot  = appSupportDir.str();
        req.title           = "Novel";
        req.slug            = "novel";
        req.author          = testAuthor();
        auto created = core.createProject(req);
        REQUIRE(created.ok());

        projectID = created.value().project.projectID;
        ch1ID     = created.value().firstChapterID;
        s1ID      = created.value().firstSceneID;

        // Add s2 to ch1
        scrivi::CreateSceneRequest sReq;
        sReq.projectRootPath = projectDir.str();
        sReq.appSupportRoot  = appSupportDir.str();
        sReq.projectID       = projectID;
        sReq.chapterID       = ch1ID;
        sReq.afterSceneID    = s1ID;
        sReq.author          = testAuthor();
        auto s2r = core.createScene(sReq);
        REQUIRE(s2r.ok());
        s2ID = s2r.value().sceneID;

        // Create ch2 (auto-creates s3)
        scrivi::CreateChapterRequest chReq;
        chReq.projectRootPath = projectDir.str();
        chReq.appSupportRoot  = appSupportDir.str();
        chReq.projectID       = projectID;
        chReq.author          = testAuthor();
        auto ch2r = core.createChapter(chReq);
        REQUIRE(ch2r.ok());
        ch2ID = ch2r.value().chapterID;
        s3ID  = ch2r.value().firstSceneID;

        // Add s4 to ch2
        sReq.chapterID    = ch2ID;
        sReq.afterSceneID = s3ID;
        auto s4r = core.createScene(sReq);
        REQUIRE(s4r.ok());
        s4ID = s4r.value().sceneID;
    }

    // Returns scene IDs in full manuscript order.
    std::vector<std::string> resolvedOrder() {
        scrivi::manuscript::ManuscriptOrderResolver resolver{services};
        auto r = resolver.resolve(projectDir.str());
        REQUIRE(r.ok());
        std::vector<std::string> ids;
        for (const auto& s : r.value()) ids.push_back(s.sceneID.value);
        return ids;
    }

    // Returns chapter IDs in manuscript order.
    std::vector<std::string> resolvedChapterOrder() {
        scrivi::manuscript::ManuscriptOrderResolver resolver{services};
        auto r = resolver.resolve(projectDir.str());
        REQUIRE(r.ok());
        std::vector<std::string> seen;
        std::vector<std::string> order;
        for (const auto& s : r.value()) {
            if (seen.empty() || seen.back() != s.chapterID.value) {
                seen.push_back(s.chapterID.value);
                order.push_back(s.chapterID.value);
            }
        }
        return order;
    }
};

// ---------------------------------------------------------------------------
// reorderScene — within chapter (T-0101)
// ---------------------------------------------------------------------------

TEST_CASE("reorderScene - move scene to beginning of same chapter",
          "[integration][T-0101]")
{
    FourSceneProject p;
    // Initial: [s1, s2, s3, s4]
    REQUIRE(p.resolvedOrder() == std::vector<std::string>{
        p.s1ID.value, p.s2ID.value, p.s3ID.value, p.s4ID.value});

    // Move s2 to before s1 (afterSceneID empty = beginning)
    scrivi::ReorderSceneRequest req;
    req.projectRootPath = p.projectDir.str();
    req.sceneID         = p.s2ID;
    req.sourceChapterID = p.ch1ID;
    req.targetChapterID = p.ch1ID;
    req.afterSceneID    = scrivi::SceneID{""};

    auto r = p.core.reorderScene(req);
    REQUIRE(r.ok());
    CHECK(r.value().reordered);

    // Expected: [s2, s1, s3, s4]
    CHECK(p.resolvedOrder() == std::vector<std::string>{
        p.s2ID.value, p.s1ID.value, p.s3ID.value, p.s4ID.value});
}

TEST_CASE("reorderScene - move scene after specific scene in same chapter",
          "[integration][T-0101]")
{
    FourSceneProject p;

    // ch2 starts as [s3, s4]. Move s3 after s4.
    scrivi::ReorderSceneRequest req;
    req.projectRootPath = p.projectDir.str();
    req.sceneID         = p.s3ID;
    req.sourceChapterID = p.ch2ID;
    req.targetChapterID = p.ch2ID;
    req.afterSceneID    = p.s4ID;

    auto r = p.core.reorderScene(req);
    REQUIRE(r.ok());
    CHECK(r.value().reordered);

    // Expected: [s1, s2, s4, s3]
    CHECK(p.resolvedOrder() == std::vector<std::string>{
        p.s1ID.value, p.s2ID.value, p.s4ID.value, p.s3ID.value});
}

// ---------------------------------------------------------------------------
// reorderScene — cross-chapter (T-0101)
// ---------------------------------------------------------------------------

TEST_CASE("reorderScene - move scene from ch1 to beginning of ch2",
          "[integration][T-0101]")
{
    FourSceneProject p;

    // Move s2 from ch1 to start of ch2
    scrivi::ReorderSceneRequest req;
    req.projectRootPath = p.projectDir.str();
    req.sceneID         = p.s2ID;
    req.sourceChapterID = p.ch1ID;
    req.targetChapterID = p.ch2ID;
    req.afterSceneID    = scrivi::SceneID{""};

    auto r = p.core.reorderScene(req);
    REQUIRE(r.ok());
    CHECK(r.value().reordered);
    CHECK(r.value().sourceChapterID.value == p.ch1ID.value);
    CHECK(r.value().targetChapterID.value == p.ch2ID.value);

    // Expected: [s1, s2, s3, s4] → [s1, s2, s3, s4] becomes [s1, s2, s3, s4]
    // ch1:[s1]  ch2:[s2, s3, s4]
    CHECK(p.resolvedOrder() == std::vector<std::string>{
        p.s1ID.value, p.s2ID.value, p.s3ID.value, p.s4ID.value});
}

TEST_CASE("reorderScene - move scene from ch2 to after s1 in ch1",
          "[integration][T-0101]")
{
    FourSceneProject p;

    // Move s3 from ch2 to after s1 in ch1
    scrivi::ReorderSceneRequest req;
    req.projectRootPath = p.projectDir.str();
    req.sceneID         = p.s3ID;
    req.sourceChapterID = p.ch2ID;
    req.targetChapterID = p.ch1ID;
    req.afterSceneID    = p.s1ID;

    auto r = p.core.reorderScene(req);
    REQUIRE(r.ok());
    CHECK(r.value().reordered);

    // ch1:[s1, s3, s2]  ch2:[s4]
    CHECK(p.resolvedOrder() == std::vector<std::string>{
        p.s1ID.value, p.s3ID.value, p.s2ID.value, p.s4ID.value});
}

// ---------------------------------------------------------------------------
// reorderScene — error cases (T-0101)
// ---------------------------------------------------------------------------

TEST_CASE("reorderScene - returns error for empty sceneID",
          "[integration][T-0101]")
{
    FourSceneProject p;

    scrivi::ReorderSceneRequest req;
    req.projectRootPath = p.projectDir.str();
    req.sceneID         = scrivi::SceneID{""};
    req.sourceChapterID = p.ch1ID;
    req.targetChapterID = p.ch1ID;

    auto r = p.core.reorderScene(req);
    CHECK_FALSE(r.ok());
    CHECK(r.error().code == scrivi::ErrorCode::invalidArgument);
}

TEST_CASE("reorderScene - returns error for unknown sourceChapterID",
          "[integration][T-0101]")
{
    FourSceneProject p;

    scrivi::ReorderSceneRequest req;
    req.projectRootPath = p.projectDir.str();
    req.sceneID         = p.s1ID;
    req.sourceChapterID = scrivi::ChapterID{"chapter-does-not-exist"};
    req.targetChapterID = p.ch1ID;

    auto r = p.core.reorderScene(req);
    CHECK_FALSE(r.ok());
    CHECK(r.error().code == scrivi::ErrorCode::invalidArgument);
}

TEST_CASE("reorderScene - returns error when sceneID not in sourceChapter",
          "[integration][T-0101]")
{
    FourSceneProject p;

    // s3 is in ch2, but we claim it's in ch1
    scrivi::ReorderSceneRequest req;
    req.projectRootPath = p.projectDir.str();
    req.sceneID         = p.s3ID;
    req.sourceChapterID = p.ch1ID;
    req.targetChapterID = p.ch1ID;

    auto r = p.core.reorderScene(req);
    CHECK_FALSE(r.ok());
    CHECK(r.error().code == scrivi::ErrorCode::invalidArgument);
}

// ---------------------------------------------------------------------------
// reorderChapter (T-0102)
// ---------------------------------------------------------------------------

TEST_CASE("reorderChapter - move ch2 to beginning (before ch1)",
          "[integration][T-0102]")
{
    FourSceneProject p;
    // Initial chapter order: [ch1, ch2]
    REQUIRE(p.resolvedChapterOrder() == std::vector<std::string>{
        p.ch1ID.value, p.ch2ID.value});

    scrivi::ReorderChapterRequest req;
    req.projectRootPath = p.projectDir.str();
    req.chapterID       = p.ch2ID;
    req.afterChapterID  = scrivi::ChapterID{""};  // beginning

    auto r = p.core.reorderChapter(req);
    REQUIRE(r.ok());
    CHECK(r.value().reordered);

    // Expected chapter order: [ch2, ch1]
    CHECK(p.resolvedChapterOrder() == std::vector<std::string>{
        p.ch2ID.value, p.ch1ID.value});

    // Full scene order: [s3, s4, s1, s2]
    CHECK(p.resolvedOrder() == std::vector<std::string>{
        p.s3ID.value, p.s4ID.value, p.s1ID.value, p.s2ID.value});
}

TEST_CASE("reorderChapter - move ch1 after ch2",
          "[integration][T-0102]")
{
    FourSceneProject p;

    scrivi::ReorderChapterRequest req;
    req.projectRootPath = p.projectDir.str();
    req.chapterID       = p.ch1ID;
    req.afterChapterID  = p.ch2ID;

    auto r = p.core.reorderChapter(req);
    REQUIRE(r.ok());
    CHECK(r.value().reordered);

    CHECK(p.resolvedChapterOrder() == std::vector<std::string>{
        p.ch2ID.value, p.ch1ID.value});
}

TEST_CASE("reorderChapter - returns error for empty chapterID",
          "[integration][T-0102]")
{
    FourSceneProject p;

    scrivi::ReorderChapterRequest req;
    req.projectRootPath = p.projectDir.str();
    req.chapterID       = scrivi::ChapterID{""};

    auto r = p.core.reorderChapter(req);
    CHECK_FALSE(r.ok());
    CHECK(r.error().code == scrivi::ErrorCode::invalidArgument);
}

TEST_CASE("reorderChapter - returns error for unknown chapterID",
          "[integration][T-0102]")
{
    FourSceneProject p;

    scrivi::ReorderChapterRequest req;
    req.projectRootPath = p.projectDir.str();
    req.chapterID       = scrivi::ChapterID{"chapter-does-not-exist"};

    auto r = p.core.reorderChapter(req);
    CHECK_FALSE(r.ok());
    CHECK(r.error().code == scrivi::ErrorCode::invalidArgument);
}
