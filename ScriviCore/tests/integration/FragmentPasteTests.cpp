// FragmentPasteTests.cpp
// EP-029 SP-087 (T-0352): paste-splice — insert a scrivi.fragment.v1 at a caret, reconstructing
// carried scene/chapter boundaries as if inserting into one continuous flat document. These
// tests exercise the FragmentPaster primitive directly against a real on-disk temp project and
// verify the resulting structure via ManuscriptOrderResolver + on-disk scene bodies.
//
// Design: docs/Scrivi_Structured_CutCopyPaste_Design_v0_1.md §4.2.

#include <catch2/catch_test_macros.hpp>

#include "scrivi/ScriviCore.hpp"
#include "scrivi/Requests.hpp"

#include "mocks/DeterministicUUIDProvider.hpp"
#include "mocks/FixedClock.hpp"
#include "mocks/MockGitProvider.hpp"
#include "mocks/MockSecureStore.hpp"

#include "platform/LocalFileSystem.hpp"
#include "manuscript/FragmentPaster.hpp"
#include "manuscript/ManuscriptOrderResolver.hpp"
#include "manuscript/SceneReader.hpp"

#include <chrono>
#include <filesystem>
#include <string>

namespace fs = std::filesystem;
namespace ms = scrivi::manuscript;

// ---------------------------------------------------------------------------
// Helpers
// ---------------------------------------------------------------------------

struct TempDir {
    fs::path path;
    TempDir() {
        path = fs::temp_directory_path() / ("scrivi-paste-" + std::to_string(
            std::chrono::steady_clock::now().time_since_epoch().count()));
        fs::create_directories(path);
    }
    ~TempDir() { std::error_code ec; fs::remove_all(path, ec); }
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
    s.fileSystem = &lfs; s.uuidProvider = &uuids; s.clock = &clock;
    s.secureStore = &store; s.gitProvider = &git;
    return s;
}

static scrivi::AuthorshipRef testAuthor()
{
    return { scrivi::IdentityID{"identity-001"}, scrivi::PersonaID{"persona-001"}, "Test Author" };
}

// A project with a single chapter and a single scene whose body we set.
struct OneSceneProject {
    TempDir projectDir, appSupportDir;
    scrivi::platform::LocalFileSystem        lfs;
    scrivi::mocks::DeterministicUUIDProvider uuids;
    scrivi::mocks::FixedClock                clock{"2026-07-27T00:00:00Z"};
    scrivi::mocks::MockSecureStore           store;
    scrivi::mocks::MockGitProvider           git;
    scrivi::CoreServices                     services;
    scrivi::ScriviCore                       core;

    scrivi::ProjectID projectID;
    scrivi::ChapterID ch1ID;
    scrivi::SceneID   s1ID;

    explicit OneSceneProject(const std::string& body)
        : services(makeServices(lfs, uuids, clock, store, git)), core(services)
    {
        scrivi::CreateProjectRequest req;
        req.projectRootPath = projectDir.str();
        req.appSupportRoot  = appSupportDir.str();
        req.title = "Paste Test Novel"; req.slug = "paste-test"; req.author = testAuthor();
        auto created = core.createProject(req);
        REQUIRE(created.ok());
        projectID = created.value().project.projectID;
        ch1ID     = created.value().firstChapterID;
        s1ID      = created.value().firstSceneID;

        scrivi::SaveSceneRequest sv;
        sv.projectID = projectID; sv.projectRootPath = projectDir.str();
        sv.appSupportRoot = appSupportDir.str();
        sv.sceneID = s1ID;
        sv.sceneMetadataPath = created.value().firstSceneMetadataPath;
        sv.sceneContentPath  = created.value().firstSceneContentPath;
        sv.markdown = body; sv.author = testAuthor();
        REQUIRE(core.saveScene(sv).ok());
    }

    ms::FragmentPaster paster() { return ms::FragmentPaster{services}; }

    std::vector<ms::ResolvedScene> resolved() {
        ms::ManuscriptOrderResolver r{services};
        auto res = r.resolve(projectDir.str());
        REQUIRE(res.ok());
        return res.value();
    }

    std::string bodyOf(const ms::ResolvedScene& scene) {
        ms::SceneReader reader{services};
        auto r = reader.readContent(projectDir.str(), scene.contentPath);
        REQUIRE(r.ok());
        return r.value();
    }
};

static ms::FragmentPiece piece(ms::OpensWith o, const std::string& text,
                               ms::Partial p = ms::Partial::None, const std::string& chTitle = "",
                               const std::string& scTitle = "") {
    ms::FragmentPiece fp; fp.opensWith = o; fp.text = text; fp.partial = p;
    fp.chapterTitle = chTitle; fp.sceneTitle = scTitle;
    return fp;
}

static ms::PasteFragmentRequest req(OneSceneProject& p, ms::Fragment frag,
                                    const scrivi::SceneID& caret, std::size_t off) {
    ms::PasteFragmentRequest r;
    r.projectRootPath = p.projectDir.str();
    r.appSupportRoot  = p.appSupportDir.str();
    r.projectID = p.projectID; r.author = testAuthor();
    r.fragment = std::move(frag); r.caretSceneID = caret; r.caretByteOffset = off;
    return r;
}

// ---------------------------------------------------------------------------
// One-piece fragment (no boundaries): a plain mid-scene text insert
// ---------------------------------------------------------------------------

TEST_CASE("paste - single-piece fragment is a plain insert", "[integration][T-0352]") {
    OneSceneProject p("AAABBB");
    ms::Fragment frag; frag.pieces = { piece(ms::OpensWith::None, "XXX", ms::Partial::Head) };
    auto r = p.paster().paste(req(p, std::move(frag), p.s1ID, 3));  // caret between AAA|BBB
    REQUIRE(r.ok());
    CHECK(r.value().createdSceneIDs.empty());
    CHECK(r.value().createdChapterIDs.empty());

    auto scenes = p.resolved();
    REQUIRE(scenes.size() == 1);
    CHECK(p.bodyOf(scenes[0]) == "AAAXXXBBB");   // head + piece + tail, no boundaries
}

// ---------------------------------------------------------------------------
// Scene boundary: mid-scene paste creates a trailing scene; tail follows the run
// ---------------------------------------------------------------------------

TEST_CASE("paste - scene-piece splits target and tail follows the pasted run",
          "[integration][T-0352]") {
    OneSceneProject p("AAABBB");
    // fragment: [none "one", scene "two", tail "three"] pasted at AAA|BBB.
    ms::Fragment frag;
    frag.pieces = {
        piece(ms::OpensWith::None,  "one",   ms::Partial::Head),
        piece(ms::OpensWith::Scene, "two"),
        piece(ms::OpensWith::Scene, "three", ms::Partial::Tail),
    };
    auto r = p.paster().paste(req(p, std::move(frag), p.s1ID, 3));
    REQUIRE(r.ok());
    // Two new scenes minted (the "two" and "three" pieces), no new chapters.
    CHECK(r.value().createdSceneIDs.size() == 2);
    CHECK(r.value().createdChapterIDs.empty());

    auto scenes = p.resolved();
    REQUIRE(scenes.size() == 3);
    // Reading order flows continuously: AAA one | two | three BBB.
    CHECK(p.bodyOf(scenes[0]) == "AAAone");
    CHECK(p.bodyOf(scenes[1]) == "two");
    CHECK(p.bodyOf(scenes[2]) == "threeBBB");   // tail-piece + original tail (flat-doc model)
    // All three scenes are in the same chapter.
    CHECK(scenes[0].chapterID.value == scenes[1].chapterID.value);
    CHECK(scenes[1].chapterID.value == scenes[2].chapterID.value);
}

// ---------------------------------------------------------------------------
// Chapter boundary: a chapter-piece creates a new chapter + first scene
// ---------------------------------------------------------------------------

TEST_CASE("paste - chapter-piece creates a new chapter", "[integration][T-0352]") {
    OneSceneProject p("AAABBB");
    // fragment: [none "one", chapter "two"] — the second piece opens a new chapter.
    ms::Fragment frag;
    frag.pieces = {
        piece(ms::OpensWith::None,    "one", ms::Partial::Head),
        piece(ms::OpensWith::Chapter, "two", ms::Partial::Tail, "Chapter 2"),
    };
    auto r = p.paster().paste(req(p, std::move(frag), p.s1ID, 3));
    REQUIRE(r.ok());
    CHECK(r.value().createdChapterIDs.size() == 1);
    CHECK(r.value().createdSceneIDs.size() == 1);   // the new chapter's first scene

    auto scenes = p.resolved();
    REQUIRE(scenes.size() == 2);
    CHECK(p.bodyOf(scenes[0]) == "AAAone");
    CHECK(p.bodyOf(scenes[1]) == "twoBBB");                 // tail follows the run
    CHECK(scenes[0].chapterID.value != scenes[1].chapterID.value);  // distinct chapters
}

// ---------------------------------------------------------------------------
// Title-capture restore (T-0357): a chapter/scene piece carrying captured titles
// re-creates the boundary WITH those titles, not a default "Chapter N" / untitled scene.
// ---------------------------------------------------------------------------

TEST_CASE("paste - restores captured chapter + scene titles (T-0357)", "[integration][T-0357]") {
    OneSceneProject p("AAABBB");
    ms::Fragment frag;
    frag.pieces = {
        piece(ms::OpensWith::None,    "one", ms::Partial::Head),
        // chapter piece carries BOTH the chapter title and its (first) scene's title.
        piece(ms::OpensWith::Chapter, "two", ms::Partial::Tail,
              "The Thing in the Black Casket", "Lady Orra"),
    };
    auto r = p.paster().paste(req(p, std::move(frag), p.s1ID, 3));
    REQUIRE(r.ok());
    REQUIRE(r.value().createdChapterIDs.size() == 1);
    REQUIRE(r.value().createdSceneIDs.size() == 1);

    auto scenes = p.resolved();
    REQUIRE(scenes.size() == 2);
    // The created chapter (scene[1]) wears the captured title, not a positional default.
    CHECK(scenes[1].chapterTitle == "The Thing in the Black Casket");
    CHECK(scenes[1].title        == "Lady Orra");
    // The original target scene/chapter is untouched.
    CHECK(scenes[0].chapterID.value != scenes[1].chapterID.value);
}

// ---------------------------------------------------------------------------
// Caret at scene start / end (no split on one side)
// ---------------------------------------------------------------------------

TEST_CASE("paste - caret at scene start (head empty)", "[integration][T-0352]") {
    OneSceneProject p("BODY");
    ms::Fragment frag;
    frag.pieces = { piece(ms::OpensWith::None, "one"), piece(ms::OpensWith::Scene, "two", ms::Partial::Tail) };
    auto r = p.paster().paste(req(p, std::move(frag), p.s1ID, 0));  // caret at very start
    REQUIRE(r.ok());
    auto scenes = p.resolved();
    REQUIRE(scenes.size() == 2);
    CHECK(p.bodyOf(scenes[0]) == "one");        // empty head + "one"
    CHECK(p.bodyOf(scenes[1]) == "twoBODY");    // tail (the whole original body) follows the run
}

TEST_CASE("paste - caret at scene end (tail empty)", "[integration][T-0352]") {
    OneSceneProject p("BODY");
    ms::Fragment frag;
    frag.pieces = { piece(ms::OpensWith::None, "one"), piece(ms::OpensWith::Scene, "two") };
    auto r = p.paster().paste(req(p, std::move(frag), p.s1ID, 4));  // caret at end
    REQUIRE(r.ok());
    auto scenes = p.resolved();
    REQUIRE(scenes.size() == 2);
    CHECK(p.bodyOf(scenes[0]) == "BODYone");    // head (whole body) + "one"
    CHECK(p.bodyOf(scenes[1]) == "two");        // empty tail
}

// ---------------------------------------------------------------------------
// Validation
// ---------------------------------------------------------------------------

TEST_CASE("paste - empty fragment rejected", "[integration][T-0352]") {
    OneSceneProject p("BODY");
    ms::Fragment frag;  // no pieces
    auto r = p.paster().paste(req(p, std::move(frag), p.s1ID, 0));
    REQUIRE_FALSE(r.ok());
    CHECK(r.error().code == scrivi::ErrorCode::invalidArgument);
}

TEST_CASE("paste - unknown caret scene rejected", "[integration][T-0352]") {
    OneSceneProject p("BODY");
    ms::Fragment frag; frag.pieces = { piece(ms::OpensWith::None, "x") };
    auto r = p.paster().paste(req(p, std::move(frag), scrivi::SceneID{"nope"}, 0));
    REQUIRE_FALSE(r.ok());
    CHECK(r.error().code == scrivi::ErrorCode::invalidArgument);
}

TEST_CASE("paste - caret past end of body rejected", "[integration][T-0352]") {
    OneSceneProject p("BODY");
    ms::Fragment frag; frag.pieces = { piece(ms::OpensWith::None, "x") };
    auto r = p.paster().paste(req(p, std::move(frag), p.s1ID, 999));
    REQUIRE_FALSE(r.ok());
    CHECK(r.error().code == scrivi::ErrorCode::invalidArgument);
}

// ---------------------------------------------------------------------------
// uncutPaste — the exact inverse of paste (EP-029 AC6 / T-0356). paste then uncut must
// restore the ORIGINAL manuscript byte-for-byte: one scene, original body, no leftovers.
// ---------------------------------------------------------------------------

static ms::Fragment sceneBoundaryFragment() {
    ms::Fragment frag;
    frag.pieces = {
        piece(ms::OpensWith::None,  "one",   ms::Partial::Head),
        piece(ms::OpensWith::Scene, "two"),
        piece(ms::OpensWith::Scene, "three", ms::Partial::Tail),
    };
    return frag;
}

TEST_CASE("uncutPaste - inverts a scene-boundary paste back to the original scene (T-0356)",
          "[integration][T-0356]") {
    OneSceneProject p("AAABBB");
    ms::Fragment frag = sceneBoundaryFragment();
    ms::Fragment fragCopy = frag;   // uncut needs the same fragment (piece lengths)

    auto pr = p.paster().paste(req(p, std::move(frag), p.s1ID, 3));   // AAA|BBB
    REQUIRE(pr.ok());
    REQUIRE(p.resolved().size() == 3);                               // split into 3 scenes

    ms::UncutPasteRequest ur;
    ur.projectRootPath   = p.projectDir.str();
    ur.fragment          = std::move(fragCopy);
    ur.targetSceneID     = pr.value().targetSceneID;
    ur.createdSceneIDs   = pr.value().createdSceneIDs;
    ur.createdChapterIDs = pr.value().createdChapterIDs;
    auto ur_r = p.paster().uncutPaste(ur);
    REQUIRE(ur_r.ok());

    auto scenes = p.resolved();
    REQUIRE(scenes.size() == 1);                                     // back to one scene
    CHECK(scenes[0].sceneID.value == p.s1ID.value);                 // the same original scene
    CHECK(p.bodyOf(scenes[0]) == "AAABBB");                         // original body restored
    CHECK(ur_r.value().survivingSceneID.value == p.s1ID.value);
}

TEST_CASE("uncutPaste - inverts a chapter-boundary paste, removing the created chapter (T-0356)",
          "[integration][T-0356]") {
    OneSceneProject p("AAABBB");
    ms::Fragment frag;
    frag.pieces = {
        piece(ms::OpensWith::None,    "one", ms::Partial::Head),
        piece(ms::OpensWith::Chapter, "two", ms::Partial::Tail, "Chapter 2"),
    };
    ms::Fragment fragCopy = frag;

    auto pr = p.paster().paste(req(p, std::move(frag), p.s1ID, 3));
    REQUIRE(pr.ok());
    REQUIRE(pr.value().createdChapterIDs.size() == 1);
    REQUIRE(p.resolved().size() == 2);

    ms::UncutPasteRequest ur;
    ur.projectRootPath   = p.projectDir.str();
    ur.fragment          = std::move(fragCopy);
    ur.targetSceneID     = pr.value().targetSceneID;
    ur.createdSceneIDs   = pr.value().createdSceneIDs;
    ur.createdChapterIDs = pr.value().createdChapterIDs;
    auto ur_r = p.paster().uncutPaste(ur);
    REQUIRE(ur_r.ok());

    auto scenes = p.resolved();
    REQUIRE(scenes.size() == 1);                     // the created chapter + its scene are gone
    CHECK(scenes[0].sceneID.value == p.s1ID.value);
    CHECK(p.bodyOf(scenes[0]) == "AAABBB");          // original body restored
    CHECK(scenes[0].chapterID.value == p.ch1ID.value);
}

TEST_CASE("uncutPaste - rejects a body that the fragment's paste could not have produced (T-0356)",
          "[integration][T-0356]") {
    OneSceneProject p("AAABBB");
    // The target does not end with the head piece's text "one" (never pasted), so uncut must
    // refuse rather than corrupt the scene.
    ms::Fragment frag = sceneBoundaryFragment();
    ms::UncutPasteRequest ur;
    ur.projectRootPath = p.projectDir.str();
    ur.fragment        = std::move(frag);
    ur.targetSceneID   = p.s1ID;                     // body is "AAABBB", not "…one"
    auto r = p.paster().uncutPaste(ur);
    REQUIRE_FALSE(r.ok());
    CHECK(r.error().code == scrivi::ErrorCode::invalidArgument);
}
