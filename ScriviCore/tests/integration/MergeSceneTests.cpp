// MergeSceneTests.cpp
// T-0298 (SP-074, EP-028): reproduce the chapter-merge data-loss bug (I-0083) as a
// RED integration test, and confirm the same-chapter scene-merge path survives reopen.
//
// I-0083: under EP-027 the app composes a chapter-merge in Swift by (a) saving the merged
// text into the predecessor scene and (b) calling deleteChapter on the emptied chapter.
// Because scene identity + location are filesystem-authoritative (EP-027 §8.1), the merged
// scenes' files still physically live in the deleted chapter's folder, so deleteChapter
// wipes them on disk. On reopen the scenes are LOST. These tests document that loss (the
// chapter-merge asserts data is lost — it will be inverted once scrivi_merge_chapter lands
// in T-0300) and prove the simpler same-chapter scene-merge is already coherent.
//
// ✅ [SP-191] (Q3e): `scrivi_merge_scene` and `scrivi_merge_chapter` RETIRED — a scene merge is a range cut of the break
// and a chapter merge a range relabel; their tests are `RangeOperationCApiTests.cpp`'s `[sp191]` cases. The two tests below
// stay: they record the I-0083 composition, which still describes `deleteChapter`.

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

#include <atomic>
#include <chrono>
#include <filesystem>
#include <fstream>
#include <string>

#include <unistd.h>

namespace fs = std::filesystem;

// ---------------------------------------------------------------------------
// Helpers
// ---------------------------------------------------------------------------

struct TempDir {
    fs::path path;

    // ⚠️ [SP-191] (seen once on Linux, `ctest -j6`): the clock alone named two directories alike — the two TempDirs of one
    // test, or two tests in parallel processes — so one test's project landed in the other's. The process and a counter too.
    TempDir() {
        static std::atomic<int> counter{0};
        path = fs::temp_directory_path() / ("scrivi-merge-" + std::to_string(::getpid()) + "-" +
                                            std::to_string(counter.fetch_add(1)) + "-" +
                                            std::to_string(std::chrono::steady_clock::now().time_since_epoch().count()));
        fs::create_directories(path);
    }

    ~TempDir() {
        std::error_code ec;
        fs::remove_all(path, ec);
    }

    std::string str() const { return path.string(); }
    bool exists(const std::string& rel) const { return fs::exists(path / rel); }
};

static std::string readFileAt(const std::string& root, const std::string& rel) {
    std::ifstream f(fs::path(root) / rel);
    return {std::istreambuf_iterator<char>(f), {}};
}

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

// A project with two chapters, one scene each. Chapter 1 = predecessor; Chapter 2 = the
// chapter that will be merged into it (its scene carries distinctive text we track).
struct TwoChapterProject {
    TempDir projectDir;
    TempDir appSupportDir;
    scrivi::platform::LocalFileSystem        lfs;
    scrivi::mocks::DeterministicUUIDProvider uuids;
    scrivi::mocks::FixedClock                clock{"2026-07-20T00:00:00Z"};
    scrivi::mocks::MockSecureStore           store;
    scrivi::mocks::MockGitProvider           git;
    scrivi::CoreServices                     services;
    scrivi::ScriviCore                       core;

    scrivi::ProjectID projectID;

    scrivi::ChapterID    ch1ID;
    scrivi::SceneID      scene1ID;
    scrivi::RelativePath scene1MetaPath;
    scrivi::RelativePath scene1ContentPath;

    scrivi::ChapterID    ch2ID;
    scrivi::SceneID      scene2ID;
    scrivi::RelativePath ch2MetaPath;

    TwoChapterProject()
        : services(makeServices(lfs, uuids, clock, store, git))
        , core(services)
    {
        scrivi::CreateProjectRequest req;
        req.projectRootPath = projectDir.str();
        req.appSupportRoot  = appSupportDir.str();
        req.title           = "Merge Test Novel";
        req.slug            = "merge-test";
        req.author          = testAuthor();
        auto created = core.createProject(req);
        REQUIRE(created.ok());

        projectID         = created.value().project.projectID;
        ch1ID             = created.value().firstChapterID;
        scene1ID          = created.value().firstSceneID;
        scene1MetaPath    = created.value().firstSceneMetadataPath;
        scene1ContentPath = created.value().firstSceneContentPath;

        scrivi::CreateChapterRequest chReq;
        chReq.projectRootPath = projectDir.str();
        chReq.appSupportRoot  = appSupportDir.str();
        chReq.projectID       = projectID;
        chReq.author          = testAuthor();
        auto ch2 = core.createChapter(chReq);
        REQUIRE(ch2.ok());
        ch2ID       = ch2.value().chapterID;
        scene2ID    = ch2.value().firstSceneID;
        ch2MetaPath = ch2.value().chapterMetadataPath;

        // Give chapter 2's scene distinctive text so we can detect its loss on reopen.
        scrivi::SaveSceneRequest s2Save;
        s2Save.projectID         = projectID;
        s2Save.projectRootPath   = projectDir.str();
        s2Save.appSupportRoot    = appSupportDir.str();
        s2Save.sceneID           = scene2ID;
        s2Save.sceneMetadataPath = ch2.value().firstSceneMetadataPath;
        s2Save.sceneContentPath  = ch2.value().firstSceneContentPath;
        s2Save.markdown          = "SCENE-TWO-BODY chapter two content";
        s2Save.author            = testAuthor();
        REQUIRE(core.saveScene(s2Save).ok());
    }
};

// ---------------------------------------------------------------------------
// I-0083 — chapter-merge composed in Swift loses scenes on reopen (RED)
// ---------------------------------------------------------------------------

TEST_CASE("chapter-merge via saveScene+deleteChapter LOSES scenes on reopen (I-0083)",
          "[integration][T-0298][I-0083]")
{
    TwoChapterProject p;

    scrivi::manuscript::ManuscriptOrderResolver resolver{p.services};
    auto before = resolver.resolve(p.projectDir.str());
    REQUIRE(before.ok());
    REQUIRE(before.value().size() == 2);

    // The app's composed chapter-merge is scene-PRESERVING: chapter 2's scene is meant to
    // become a distinct trailing scene of chapter 1 (the merge concatenates chapters, not
    // scene bodies). The Swift code reassigns chapter 2's scenes to chapter 1 in memory and
    // then calls deleteChapter(ch2) to drop the now-"empty" chapter. It never relocates the
    // scene FILES, which physically remain under chapter 2's folder.
    //
    // We record scene 2's distinctive body on disk before the merge...
    const std::string scene2Content = before.value()[1].contentPath;
    const std::string scene2BodyBefore = readFileAt(p.projectDir.str(), scene2Content);
    REQUIRE(scene2BodyBefore.find("SCENE-TWO-BODY") != std::string::npos);

    // ...then replay the lossy step: deleteChapter(ch2) after the in-memory reassign.
    scrivi::DeleteChapterRequest delReq;
    delReq.projectRootPath = p.projectDir.str();
    delReq.chapterID       = p.ch2ID;
    REQUIRE(p.core.deleteChapter(delReq).ok());

    // I-0083: on reopen, chapter 2's scene is GONE. deleteChapter removed chapter 2's folder
    // wholesale — including the scene file the merge intended to keep — so the manuscript
    // loses a scene and its body vanishes from disk. This documents WHY the Swift-composed
    // merge is lossy and guards against re-adopting it. The correct path is the atomic
    // relocation — ✅ [SP-191] `scrivi_range_relabel` (RangeOperationCApiTests' `[sp191]` cases prove the scenes SURVIVE).
    auto after = resolver.resolve(p.projectDir.str());
    REQUIRE(after.ok());
    CHECK(after.value().size() == 1);                    // was 2 — scene 2 lost
    CHECK(after.value()[0].chapterID.value == p.ch1ID.value);
    CHECK_FALSE(p.projectDir.exists(p.ch2MetaPath));     // whole folder gone
    CHECK_FALSE(p.projectDir.exists(scene2Content));     // scene 2's body deleted on disk
}

// ---------------------------------------------------------------------------
// Same-chapter scene-merge is already coherent (GREEN)
// ---------------------------------------------------------------------------

TEST_CASE("same-chapter scene-merge via saveScene+deleteScene SURVIVES reopen",
          "[integration][T-0298]")
{
    TempDir projectDir;
    TempDir appSupportDir;

    scrivi::platform::LocalFileSystem        lfs;
    scrivi::mocks::DeterministicUUIDProvider uuids;
    scrivi::mocks::FixedClock                clock{"2026-07-20T00:00:00Z"};
    scrivi::mocks::MockSecureStore           store;
    scrivi::mocks::MockGitProvider           git;
    auto services = makeServices(lfs, uuids, clock, store, git);
    scrivi::ScriviCore core{services};

    scrivi::CreateProjectRequest req;
    req.projectRootPath = projectDir.str();
    req.appSupportRoot  = appSupportDir.str();
    req.title           = "Merge Test Novel";
    req.slug            = "merge-test";
    req.author          = testAuthor();
    auto created = core.createProject(req);
    REQUIRE(created.ok());

    const auto projectID = created.value().project.projectID;
    const auto ch1ID     = created.value().firstChapterID;
    const auto scene1ID  = created.value().firstSceneID;

    // Add a second scene in the SAME chapter.
    scrivi::CreateSceneRequest s2Req;
    s2Req.projectRootPath = projectDir.str();
    s2Req.appSupportRoot  = appSupportDir.str();
    s2Req.projectID       = projectID;
    s2Req.chapterID       = ch1ID;
    s2Req.afterSceneID    = scene1ID;
    s2Req.author          = testAuthor();
    auto s2 = core.createScene(s2Req);
    REQUIRE(s2.ok());
    const auto scene2ID = s2.value().sceneID;

    scrivi::manuscript::ManuscriptOrderResolver resolver{services};
    auto before = resolver.resolve(projectDir.str());
    REQUIRE(before.ok());
    REQUIRE(before.value().size() == 2);

    // Compose a same-chapter merge: append scene 2's body into scene 1 (the predecessor),
    // then delete scene 2. Because both scenes live in the same chapter folder, deleteScene
    // removes only scene 2's files — scene 1 (now holding the merged body) is untouched.
    scrivi::SaveSceneRequest mergeSave;
    mergeSave.projectID         = projectID;
    mergeSave.projectRootPath   = projectDir.str();
    mergeSave.appSupportRoot    = appSupportDir.str();
    mergeSave.sceneID           = scene1ID;
    mergeSave.sceneMetadataPath = created.value().firstSceneMetadataPath;
    mergeSave.sceneContentPath  = created.value().firstSceneContentPath;
    mergeSave.markdown          = "SCENE-ONE-BODY\n\nSCENE-TWO-BODY same chapter";
    mergeSave.author            = testAuthor();
    REQUIRE(core.saveScene(mergeSave).ok());

    scrivi::DeleteSceneRequest delReq;
    delReq.projectRootPath = projectDir.str();
    delReq.sceneID         = scene2ID;
    REQUIRE(core.deleteScene(delReq).ok());

    // Reopen: exactly one scene (the survivor) with BOTH bodies intact on disk.
    auto after = resolver.resolve(projectDir.str());
    REQUIRE(after.ok());
    REQUIRE(after.value().size() == 1);
    CHECK(after.value()[0].sceneID.value == scene1ID.value);

    const std::string mergedBody =
        readFileAt(projectDir.str(), after.value()[0].contentPath);
    CHECK(mergedBody.find("SCENE-ONE-BODY") != std::string::npos);
    CHECK(mergedBody.find("SCENE-TWO-BODY") != std::string::npos);
}
