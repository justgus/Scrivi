// FragmentCutTests.cpp
// EP-029 SP-088 (T-0353): cut-with-merge — extract a manuscript range into a fragment, then delete
// the spanned text and collapse the spanned scenes/chapters into one continuous scene
// (delete-and-fold, design §4.3). Exercises FragmentCutter directly against a real temp project.
//
// Fixture: chapter 1 (S1a "Alpha one two three.", S1b "Bravo four five six."),
//          chapter 2 (S2a "Charlie seven eight nine.", S2b "Delta ten eleven.").
//
// Design: docs/Scrivi_Structured_CutCopyPaste_Design_v0_1.md §4.3.

#include <catch2/catch_test_macros.hpp>

#include "scrivi/ScriviCore.hpp"
#include "scrivi/Requests.hpp"

#include "mocks/DeterministicUUIDProvider.hpp"
#include "mocks/FixedClock.hpp"
#include "mocks/MockGitProvider.hpp"
#include "mocks/MockSecureStore.hpp"

#include "platform/LocalFileSystem.hpp"
#include "manuscript/FragmentCutter.hpp"
#include "manuscript/ManuscriptOrderResolver.hpp"
#include "manuscript/SceneReader.hpp"

#include <chrono>
#include <filesystem>
#include <string>

namespace fs = std::filesystem;
namespace ms = scrivi::manuscript;

struct TempDir {
    fs::path path;
    TempDir() {
        path = fs::temp_directory_path() / ("scrivi-cut-" + std::to_string(
            std::chrono::steady_clock::now().time_since_epoch().count()));
        fs::create_directories(path);
    }
    ~TempDir() { std::error_code ec; fs::remove_all(path, ec); }
    std::string str() const { return path.string(); }
};

static scrivi::CoreServices makeServices(
    scrivi::platform::LocalFileSystem& lfs, scrivi::mocks::DeterministicUUIDProvider& uuids,
    scrivi::mocks::FixedClock& clock, scrivi::mocks::MockSecureStore& store,
    scrivi::mocks::MockGitProvider& git)
{
    scrivi::CoreServices s;
    s.fileSystem = &lfs; s.uuidProvider = &uuids; s.clock = &clock;
    s.secureStore = &store; s.gitProvider = &git;
    return s;
}

static scrivi::AuthorshipRef testAuthor()
{
    return { scrivi::IdentityID{"identity-001"}, scrivi::PersonaID{"persona-001"}, "Test Author" };
}

static const std::string S1A = "Alpha one two three.";
static const std::string S1B = "Bravo four five six.";
static const std::string S2A = "Charlie seven eight nine.";
static const std::string S2B = "Delta ten eleven.";

struct CutProject {
    TempDir projectDir, appSupportDir;
    scrivi::platform::LocalFileSystem        lfs;
    scrivi::mocks::DeterministicUUIDProvider uuids;
    scrivi::mocks::FixedClock                clock{"2026-07-27T00:00:00Z"};
    scrivi::mocks::MockSecureStore           store;
    scrivi::mocks::MockGitProvider           git;
    scrivi::CoreServices                     services;
    scrivi::ScriviCore                       core;

    scrivi::ProjectID projectID;
    scrivi::ChapterID ch1ID, ch2ID;
    scrivi::SceneID   s1aID, s1bID, s2aID, s2bID;

    CutProject() : services(makeServices(lfs, uuids, clock, store, git)), core(services) {
        scrivi::CreateProjectRequest req;
        req.projectRootPath = projectDir.str(); req.appSupportRoot = appSupportDir.str();
        req.title = "Cut Test Novel"; req.slug = "cut-test"; req.author = testAuthor();
        auto created = core.createProject(req);
        REQUIRE(created.ok());
        projectID = created.value().project.projectID;
        ch1ID = created.value().firstChapterID;
        s1aID = created.value().firstSceneID;

        auto save = [&](const scrivi::SceneID& id, const scrivi::RelativePath& meta,
                        const scrivi::RelativePath& content, const std::string& body) {
            scrivi::SaveSceneRequest sv;
            sv.projectID = projectID; sv.projectRootPath = projectDir.str();
            sv.appSupportRoot = appSupportDir.str();
            sv.sceneID = id; sv.sceneMetadataPath = meta; sv.sceneContentPath = content;
            sv.markdown = body; sv.author = testAuthor();
            REQUIRE(core.saveScene(sv).ok());
        };

        save(s1aID, created.value().firstSceneMetadataPath,
             created.value().firstSceneContentPath, S1A);

        auto addScene = [&](const scrivi::ChapterID& ch, const scrivi::SceneID& after,
                            const std::string& body) -> scrivi::SceneID {
            scrivi::CreateSceneRequest sc;
            sc.projectRootPath = projectDir.str(); sc.appSupportRoot = appSupportDir.str();
            sc.projectID = projectID; sc.chapterID = ch; sc.afterSceneID = after;
            sc.author = testAuthor();
            auto r = core.createScene(sc);
            REQUIRE(r.ok());
            save(r.value().sceneID, r.value().metadataPath, r.value().contentPath, body);
            return r.value().sceneID;
        };

        s1bID = addScene(ch1ID, s1aID, S1B);

        scrivi::CreateChapterRequest chReq;
        chReq.projectRootPath = projectDir.str(); chReq.appSupportRoot = appSupportDir.str();
        chReq.projectID = projectID; chReq.author = testAuthor();
        auto ch2 = core.createChapter(chReq);
        REQUIRE(ch2.ok());
        ch2ID = ch2.value().chapterID;
        s2aID = ch2.value().firstSceneID;
        save(s2aID, ch2.value().firstSceneMetadataPath, ch2.value().firstSceneContentPath, S2A);
        s2bID = addScene(ch2ID, s2aID, S2B);
    }

    ms::FragmentCutter cutter() { return ms::FragmentCutter{services}; }

    std::vector<ms::ResolvedScene> resolved() {
        ms::ManuscriptOrderResolver r{services};
        auto res = r.resolve(projectDir.str());
        REQUIRE(res.ok());
        return res.value();
    }
    std::string bodyOf(const ms::ResolvedScene& s) {
        ms::SceneReader reader{services};
        auto r = reader.readContent(projectDir.str(), s.contentPath);
        REQUIRE(r.ok());
        return r.value();
    }
    bool has(const std::vector<ms::ResolvedScene>& v, const scrivi::SceneID& id) {
        for (auto& s : v) if (s.sceneID.value == id.value) return true;
        return false;
    }
};

static ms::FragmentSpan whole(const scrivi::SceneID& id, const std::string& body) {
    return ms::FragmentSpan{id, 0, body.size()};
}

// ---------------------------------------------------------------------------
// Same-chapter cut: two scenes collapse to one; the tail suffix folds onto the head
// ---------------------------------------------------------------------------

TEST_CASE("cut - across two scenes in one chapter collapses to one", "[integration][T-0353]") {
    CutProject p;
    // Cut from S1a byte 6 ("one...") through S1b byte 6 ("...four five six.") — head keeps
    // "Alpha ", tail keeps "four five six.".
    std::vector<ms::FragmentSpan> spans = {
        ms::FragmentSpan{p.s1aID, 6, S1A.size()},
        ms::FragmentSpan{p.s1bID, 0, 6},
    };
    auto r = p.cutter().cut(p.projectDir.str(), spans);
    REQUIRE(r.ok());
    CHECK(r.value().survivingSceneID.value == p.s1aID.value);
    REQUIRE(r.value().removedSceneIDs.size() == 1);
    CHECK(r.value().removedSceneIDs[0].value == p.s1bID.value);
    CHECK(r.value().removedChapterIDs.empty());

    auto scenes = p.resolved();
    // ch1 now has one scene (S1a); ch2 still has S2a, S2b → 3 total.
    REQUIRE(scenes.size() == 3);
    CHECK_FALSE(p.has(scenes, p.s1bID));
    CHECK(p.bodyOf(scenes[0]) == "Alpha four five six.");   // headPrefix + tailSuffix, gap closed
    // The extracted fragment carries the removed content across the boundary.
    CHECK(r.value().fragment.pieces.size() == 2);
}

// ---------------------------------------------------------------------------
// Cross-chapter cut that empties chapter 2's first scene but leaves S2b
// ---------------------------------------------------------------------------

TEST_CASE("cut - across a chapter boundary promotes the tail chapter's survivors into the head (T-0357)",
          "[integration][T-0357]") {
    CutProject p;
    // Cut from mid-S1b (ch1) into the head of S2a (ch2). S2b is AFTER the span and survives, but
    // its chapter's heading region was consumed → under T-0357 S2b is PROMOTED into ch1 and ch2 is
    // removed (no duplicate-named remnant), rather than left behind as its own chapter.
    // S2A = "Charlie seven eight nine." — cutting 0..14 leaves the suffix "eight nine." as the tail.
    std::vector<ms::FragmentSpan> spans = {
        ms::FragmentSpan{p.s1bID, 6, S1B.size()},   // "Bravo " kept as head prefix
        ms::FragmentSpan{p.s2aID, 0, 14},           // "eight nine." kept as tail suffix
    };
    auto r = p.cutter().cut(p.projectDir.str(), spans);
    REQUIRE(r.ok());
    CHECK(r.value().survivingSceneID.value == p.s1bID.value);
    REQUIRE(r.value().removedSceneIDs.size() == 1);
    CHECK(r.value().removedSceneIDs[0].value == p.s2aID.value);
    // ch2 is now emptied of its own identity (S2a folded, S2b promoted) → removed by the merge.
    REQUIRE(r.value().removedChapterIDs.size() == 1);
    CHECK(r.value().removedChapterIDs[0].value == p.ch2ID.value);

    auto scenes = p.resolved();
    // S1a, S1b(survivor), S2b → 3 scenes, now ALL in ch1 (one continuous chapter).
    REQUIRE(scenes.size() == 3);
    CHECK(p.has(scenes, p.s2bID));         // survivor promoted, not lost
    CHECK_FALSE(p.has(scenes, p.s2aID));   // the cut tail scene gone
    for (auto& s : scenes)
        CHECK(s.chapterID.value == p.ch1ID.value);   // every survivor is in the head chapter
    // survivor S1b = "Bravo " + S2a suffix "eight nine." (gap closed, no seam)
    for (auto& s : scenes)
        if (s.sceneID.value == p.s1bID.value)
            CHECK(p.bodyOf(s) == "Bravo eight nine.");
    // The fragment captured ch2's title so a paste can restore it (T-0357).
    REQUIRE(r.value().fragment.pieces.size() == 2);
    CHECK(r.value().fragment.pieces[1].opensWith == ms::OpensWith::Chapter);
}

// ---------------------------------------------------------------------------
// Cross-chapter cut that empties chapter 2 entirely → chapter removed
// ---------------------------------------------------------------------------

TEST_CASE("cut - empties a whole chapter → chapter removed", "[integration][T-0353]") {
    CutProject p;
    // Cut from S1a mid through the WHOLE of ch2 (S2a + S2b). ch2 loses all scenes → removed.
    std::vector<ms::FragmentSpan> spans = {
        ms::FragmentSpan{p.s1aID, 6, S1A.size()},   // head keeps "Alpha "
        whole(p.s1bID, S1B),
        whole(p.s2aID, S2A),
        ms::FragmentSpan{p.s2bID, 0, S2B.size() - 1},  // tail keeps "." (last byte)
    };
    auto r = p.cutter().cut(p.projectDir.str(), spans);
    REQUIRE(r.ok());
    CHECK(r.value().survivingSceneID.value == p.s1aID.value);
    CHECK(r.value().removedSceneIDs.size() == 3);           // S1b, S2a, S2b
    REQUIRE(r.value().removedChapterIDs.size() == 1);
    CHECK(r.value().removedChapterIDs[0].value == p.ch2ID.value);

    auto scenes = p.resolved();
    REQUIRE(scenes.size() == 1);                            // only the survivor remains
    CHECK(scenes[0].sceneID.value == p.s1aID.value);
    CHECK(scenes[0].chapterID.value == p.ch1ID.value);
    CHECK(p.bodyOf(scenes[0]) == "Alpha .");               // headPrefix "Alpha " + tail "." (gap closed)
}

// ---------------------------------------------------------------------------
// Single-scene cut (head==tail): a within-scene deletion, no scenes removed
// ---------------------------------------------------------------------------

TEST_CASE("cut - single scene deletes the span in place", "[integration][T-0353]") {
    CutProject p;
    // Delete "one two three" from S1a (bytes 6..19), keeping "Alpha " + ".".
    auto r = p.cutter().cut(p.projectDir.str(), { ms::FragmentSpan{p.s1aID, 6, 19} });
    REQUIRE(r.ok());
    CHECK(r.value().survivingSceneID.value == p.s1aID.value);
    CHECK(r.value().removedSceneIDs.empty());
    CHECK(r.value().removedChapterIDs.empty());

    auto scenes = p.resolved();
    REQUIRE(scenes.size() == 4);   // nothing structural removed
    CHECK(p.bodyOf(scenes[0]) == "Alpha .");   // headPrefix "Alpha " + tail "." (gap closed)
}

// ---------------------------------------------------------------------------
// Validation
// ---------------------------------------------------------------------------

TEST_CASE("cut - empty spans rejected", "[integration][T-0353]") {
    CutProject p;
    auto r = p.cutter().cut(p.projectDir.str(), {});
    REQUIRE_FALSE(r.ok());
    CHECK(r.error().code == scrivi::ErrorCode::invalidArgument);
}

TEST_CASE("cut - unknown scene rejected", "[integration][T-0353]") {
    CutProject p;
    auto r = p.cutter().cut(p.projectDir.str(), { ms::FragmentSpan{scrivi::SceneID{"nope"}, 0, 1} });
    REQUIRE_FALSE(r.ok());
    CHECK(r.error().code == scrivi::ErrorCode::invalidArgument);
}
