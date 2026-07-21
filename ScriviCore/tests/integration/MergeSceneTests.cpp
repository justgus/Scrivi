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
#include <fstream>
#include <string>

namespace fs = std::filesystem;

// ---------------------------------------------------------------------------
// Helpers
// ---------------------------------------------------------------------------

struct TempDir {
    fs::path path;

    TempDir() {
        path = fs::temp_directory_path() / ("scrivi-merge-" + std::to_string(
            std::chrono::steady_clock::now().time_since_epoch().count()));
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
    // scrivi_merge_chapter endpoint (T-0300), whose own test below proves the scenes SURVIVE.
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

// ---------------------------------------------------------------------------
// T-0299 — scrivi_merge_scene endpoint (same-chapter join into predecessor)
// ---------------------------------------------------------------------------

// A single-chapter project with two scenes, each carrying distinctive body text.
struct TwoSceneOneChapterProject {
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
    scrivi::ChapterID chapterID;
    scrivi::SceneID   scene1ID;   // predecessor / survivor
    scrivi::SceneID   scene2ID;   // merged away

    TwoSceneOneChapterProject()
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
        projectID = created.value().project.projectID;
        chapterID = created.value().firstChapterID;
        scene1ID  = created.value().firstSceneID;

        // Give scene 1 a body.
        scrivi::SaveSceneRequest s1Save;
        s1Save.projectID         = projectID;
        s1Save.projectRootPath   = projectDir.str();
        s1Save.appSupportRoot    = appSupportDir.str();
        s1Save.sceneID           = scene1ID;
        s1Save.sceneMetadataPath = created.value().firstSceneMetadataPath;
        s1Save.sceneContentPath  = created.value().firstSceneContentPath;
        s1Save.markdown          = "SCENE-ONE-BODY";
        s1Save.author            = testAuthor();
        REQUIRE(core.saveScene(s1Save).ok());

        // Add scene 2 after scene 1 in the same chapter, with its own body.
        scrivi::CreateSceneRequest s2Req;
        s2Req.projectRootPath = projectDir.str();
        s2Req.appSupportRoot  = appSupportDir.str();
        s2Req.projectID       = projectID;
        s2Req.chapterID       = chapterID;
        s2Req.afterSceneID    = scene1ID;
        s2Req.author          = testAuthor();
        auto s2 = core.createScene(s2Req);
        REQUIRE(s2.ok());
        scene2ID = s2.value().sceneID;

        scrivi::SaveSceneRequest s2Save;
        s2Save.projectID         = projectID;
        s2Save.projectRootPath   = projectDir.str();
        s2Save.appSupportRoot    = appSupportDir.str();
        s2Save.sceneID           = scene2ID;
        s2Save.sceneMetadataPath = s2.value().metadataPath;
        s2Save.sceneContentPath  = s2.value().contentPath;
        s2Save.markdown          = "SCENE-TWO-BODY";
        s2Save.author            = testAuthor();
        REQUIRE(core.saveScene(s2Save).ok());
    }
};

TEST_CASE("mergeScene - joins a scene into its predecessor; both bodies survive reopen",
          "[integration][T-0299]")
{
    TwoSceneOneChapterProject p;

    scrivi::MergeSceneRequest req;
    req.projectRootPath = p.projectDir.str();
    req.sceneID         = p.scene2ID;   // merge scene 2 INTO scene 1

    auto r = p.core.mergeScene(req);
    REQUIRE(r.ok());
    CHECK(r.value().merged);
    CHECK(r.value().survivorSceneID.value == p.scene1ID.value);
    CHECK(r.value().mergedSceneID.value   == p.scene2ID.value);
    CHECK(r.value().chapterID.value       == p.chapterID.value);

    // Reopen: exactly one scene remains — scene 1 (the survivor keeps its own key) — and its
    // on-disk body now holds BOTH scenes' text, in order.
    scrivi::manuscript::ManuscriptOrderResolver resolver{p.services};
    auto after = resolver.resolve(p.projectDir.str());
    REQUIRE(after.ok());
    REQUIRE(after.value().size() == 1);
    CHECK(after.value()[0].sceneID.value == p.scene1ID.value);

    const std::string body = readFileAt(p.projectDir.str(), after.value()[0].contentPath);
    const auto one = body.find("SCENE-ONE-BODY");
    const auto two = body.find("SCENE-TWO-BODY");
    CHECK(one != std::string::npos);
    CHECK(two != std::string::npos);
    CHECK(one < two);   // survivor body first, merged body appended after

    // The survivor path reported in the result matches where the body actually lives.
    CHECK(r.value().survivorContentPath == after.value()[0].contentPath);
}

TEST_CASE("mergeScene - merging an empty scene preserves the survivor body (no stray blanks)",
          "[integration][T-0299]")
{
    // scene 1 has a body; scene 2 is created but never given content (empty/absent `.md`).
    // The join must elide the separator so the survivor gains no trailing blank lines, and
    // the absent-content branch must not error.
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

    {
        scrivi::SaveSceneRequest save;
        save.projectID         = projectID;
        save.projectRootPath   = projectDir.str();
        save.appSupportRoot    = appSupportDir.str();
        save.sceneID           = scene1ID;
        save.sceneMetadataPath = created.value().firstSceneMetadataPath;
        save.sceneContentPath  = created.value().firstSceneContentPath;
        save.markdown          = "ONLY-BODY";
        save.author            = testAuthor();
        REQUIRE(core.saveScene(save).ok());
    }

    // Second scene, no saveScene → its content is empty/default.
    scrivi::CreateSceneRequest s2Req;
    s2Req.projectRootPath = projectDir.str();
    s2Req.appSupportRoot  = appSupportDir.str();
    s2Req.projectID       = projectID;
    s2Req.chapterID       = ch1ID;
    s2Req.afterSceneID    = scene1ID;
    s2Req.author          = testAuthor();
    auto s2 = core.createScene(s2Req);
    REQUIRE(s2.ok());

    scrivi::MergeSceneRequest mReq;
    mReq.projectRootPath = projectDir.str();
    mReq.sceneID         = s2.value().sceneID;
    auto r = core.mergeScene(mReq);
    REQUIRE(r.ok());
    CHECK(r.value().merged);

    scrivi::manuscript::ManuscriptOrderResolver resolver{services};
    auto after = resolver.resolve(projectDir.str());
    REQUIRE(after.ok());
    REQUIRE(after.value().size() == 1);

    const std::string body = readFileAt(projectDir.str(), after.value()[0].contentPath);
    CHECK(body.find("ONLY-BODY") != std::string::npos);
    // Empty merged body elided: the survivor body is unchanged (no appended blank lines).
    CHECK(body == "ONLY-BODY");
}

TEST_CASE("mergeScene - fails when the scene is first in its chapter (no predecessor)",
          "[integration][T-0299]")
{
    TwoSceneOneChapterProject p;

    scrivi::MergeSceneRequest req;
    req.projectRootPath = p.projectDir.str();
    req.sceneID         = p.scene1ID;   // first scene — nothing before it in the chapter

    auto r = p.core.mergeScene(req);
    CHECK_FALSE(r.ok());
    CHECK(r.error().code == scrivi::ErrorCode::invalidArgument);
}

TEST_CASE("mergeScene - fails for empty sceneID", "[integration][T-0299]")
{
    TwoSceneOneChapterProject p;

    scrivi::MergeSceneRequest req;
    req.projectRootPath = p.projectDir.str();
    req.sceneID         = scrivi::SceneID{""};

    auto r = p.core.mergeScene(req);
    CHECK_FALSE(r.ok());
    CHECK(r.error().code == scrivi::ErrorCode::invalidArgument);
}

TEST_CASE("mergeScene - fails for unknown sceneID", "[integration][T-0299]")
{
    TwoSceneOneChapterProject p;

    scrivi::MergeSceneRequest req;
    req.projectRootPath = p.projectDir.str();
    req.sceneID         = scrivi::SceneID{"scene-does-not-exist"};

    auto r = p.core.mergeScene(req);
    CHECK_FALSE(r.ok());
    CHECK(r.error().code == scrivi::ErrorCode::invalidArgument);
}

// ---------------------------------------------------------------------------
// T-0300 — scrivi_merge_chapter endpoint (whole-chapter relocation; fixes I-0083)
// ---------------------------------------------------------------------------

// Append a scene with `body` to `chapterID` (after the chapter's current last scene) and
// return its sceneID.
static scrivi::SceneID addSceneWithBody(
    scrivi::ScriviCore& core,
    const std::string& projectDir, const std::string& appSupportDir,
    const scrivi::ProjectID& projectID, const scrivi::ChapterID& chapterID,
    const std::string& body)
{
    scrivi::CreateSceneRequest sReq;
    sReq.projectRootPath = projectDir;
    sReq.appSupportRoot  = appSupportDir;
    sReq.projectID       = projectID;
    sReq.chapterID       = chapterID;
    sReq.author          = testAuthor();   // afterSceneID empty = append to end of chapter
    auto s = core.createScene(sReq);
    REQUIRE(s.ok());

    scrivi::SaveSceneRequest save;
    save.projectID         = projectID;
    save.projectRootPath   = projectDir;
    save.appSupportRoot    = appSupportDir;
    save.sceneID           = s.value().sceneID;
    save.sceneMetadataPath = s.value().metadataPath;
    save.sceneContentPath  = s.value().contentPath;
    save.markdown          = body;
    save.author            = testAuthor();
    REQUIRE(core.saveScene(save).ok());
    return s.value().sceneID;
}

TEST_CASE("mergeChapter - relocates all scenes into predecessor; nothing lost (I-0083 fixed)",
          "[integration][T-0300][I-0083]")
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
    const auto ch1s1ID   = created.value().firstSceneID;

    // Chapter 1: scene 1 (existing) gets a body, plus a second scene.
    {
        scrivi::SaveSceneRequest save;
        save.projectID         = projectID;
        save.projectRootPath   = projectDir.str();
        save.appSupportRoot    = appSupportDir.str();
        save.sceneID           = ch1s1ID;
        save.sceneMetadataPath = created.value().firstSceneMetadataPath;
        save.sceneContentPath  = created.value().firstSceneContentPath;
        save.markdown          = "CH1-SCENE-1";
        save.author            = testAuthor();
        REQUIRE(core.saveScene(save).ok());
    }
    const auto ch1s2ID = addSceneWithBody(
        core, projectDir.str(), appSupportDir.str(), projectID, ch1ID, "CH1-SCENE-2");

    // Chapter 2 with two scenes of its own.
    scrivi::CreateChapterRequest chReq;
    chReq.projectRootPath = projectDir.str();
    chReq.appSupportRoot  = appSupportDir.str();
    chReq.projectID       = projectID;
    chReq.author          = testAuthor();
    auto ch2 = core.createChapter(chReq);
    REQUIRE(ch2.ok());
    const auto ch2ID       = ch2.value().chapterID;
    const auto ch2s1ID     = ch2.value().firstSceneID;
    const auto ch2MetaPath = ch2.value().chapterMetadataPath;
    {
        scrivi::SaveSceneRequest save;
        save.projectID         = projectID;
        save.projectRootPath   = projectDir.str();
        save.appSupportRoot    = appSupportDir.str();
        save.sceneID           = ch2s1ID;
        save.sceneMetadataPath = ch2.value().firstSceneMetadataPath;
        save.sceneContentPath  = ch2.value().firstSceneContentPath;
        save.markdown          = "CH2-SCENE-1";
        save.author            = testAuthor();
        REQUIRE(core.saveScene(save).ok());
    }
    const auto ch2s2ID = addSceneWithBody(
        core, projectDir.str(), appSupportDir.str(), projectID, ch2ID, "CH2-SCENE-2");

    scrivi::manuscript::ManuscriptOrderResolver resolver{services};
    auto before = resolver.resolve(projectDir.str());
    REQUIRE(before.ok());
    REQUIRE(before.value().size() == 4);   // 2 + 2

    // Merge chapter 2 into chapter 1.
    scrivi::MergeChapterRequest mReq;
    mReq.projectRootPath = projectDir.str();
    mReq.chapterID       = ch2ID;
    auto r = core.mergeChapter(mReq);
    REQUIRE(r.ok());
    CHECK(r.value().merged);
    CHECK(r.value().survivorChapterID.value == ch1ID.value);
    CHECK(r.value().mergedChapterID.value   == ch2ID.value);
    CHECK(r.value().scenesRelocated         == 2);

    // Chapter 2's folder is gone.
    CHECK_FALSE(projectDir.exists(ch2MetaPath));

    // Reopen: ALL FOUR scenes survive, now all in chapter 1, in manuscript order, each with
    // its body intact on disk. This is the I-0083 fix — no scene deleted.
    auto after = resolver.resolve(projectDir.str());
    REQUIRE(after.ok());
    REQUIRE(after.value().size() == 4);
    for (const auto& s : after.value()) {
        CHECK(s.chapterID.value == ch1ID.value);
    }

    std::vector<std::string> ids;
    std::vector<std::string> bodies;
    for (const auto& s : after.value()) {
        ids.push_back(s.sceneID.value);
        bodies.push_back(readFileAt(projectDir.str(), s.contentPath));
    }
    // Order: ch1's own scenes first (unchanged keys), then ch2's scenes appended in order.
    CHECK(ids[0] == ch1s1ID.value);
    CHECK(ids[1] == ch1s2ID.value);
    CHECK(ids[2] == ch2s1ID.value);
    CHECK(ids[3] == ch2s2ID.value);
    CHECK(bodies[0].find("CH1-SCENE-1") != std::string::npos);
    CHECK(bodies[1].find("CH1-SCENE-2") != std::string::npos);
    CHECK(bodies[2].find("CH2-SCENE-1") != std::string::npos);
    CHECK(bodies[3].find("CH2-SCENE-2") != std::string::npos);
}

TEST_CASE("mergeChapter - fails when the chapter is first in the manuscript (no predecessor)",
          "[integration][T-0300]")
{
    TwoChapterProject p;   // ch1 (first) + ch2

    scrivi::MergeChapterRequest req;
    req.projectRootPath = p.projectDir.str();
    req.chapterID       = p.ch1ID;   // first chapter — nothing before it

    auto r = p.core.mergeChapter(req);
    CHECK_FALSE(r.ok());
    CHECK(r.error().code == scrivi::ErrorCode::invalidArgument);
}

TEST_CASE("mergeChapter - fails for empty chapterID", "[integration][T-0300]")
{
    TwoChapterProject p;

    scrivi::MergeChapterRequest req;
    req.projectRootPath = p.projectDir.str();
    req.chapterID       = scrivi::ChapterID{""};

    auto r = p.core.mergeChapter(req);
    CHECK_FALSE(r.ok());
    CHECK(r.error().code == scrivi::ErrorCode::invalidArgument);
}

TEST_CASE("mergeChapter - fails for unknown chapterID", "[integration][T-0300]")
{
    TwoChapterProject p;

    scrivi::MergeChapterRequest req;
    req.projectRootPath = p.projectDir.str();
    req.chapterID       = scrivi::ChapterID{"chapter-does-not-exist"};

    auto r = p.core.mergeChapter(req);
    CHECK_FALSE(r.ok());
    CHECK(r.error().code == scrivi::ErrorCode::invalidArgument);
}
