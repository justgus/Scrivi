// FragmentExtractTests.cpp
// EP-029 SP-086 (T-0351): extract-fragment — turn a manuscript range (ordered scene-local
// byte spans) into a scrivi.fragment.v1 structured fragment. These tests exercise the
// FragmentExtractor primitive directly against a real on-disk temp project.
//
// Fixture: two chapters. Chapter 1 has two scenes (S1a, S1b); Chapter 2 has one scene (S2).
// Each scene body is distinctive so we can assert exactly which bytes land in each piece:
//   S1a = "Alpha one two three."          (chapter 1, scene 1)
//   S1b = "Bravo four five six."          (chapter 1, scene 2)
//   S2  = "Charlie seven eight nine."     (chapter 2, scene 1)
//
// Design: docs/Scrivi_Structured_CutCopyPaste_Design_v0_1.md §3, §4.1.

#include <catch2/catch_test_macros.hpp>

#include "scrivi/ScriviCore.hpp"
#include "scrivi/Requests.hpp"

#include "mocks/DeterministicUUIDProvider.hpp"
#include "mocks/FixedClock.hpp"
#include "mocks/MockGitProvider.hpp"
#include "mocks/MockSecureStore.hpp"

#include "platform/LocalFileSystem.hpp"
#include "manuscript/FragmentExtractor.hpp"
#include "manuscript/ManuscriptOrderResolver.hpp"

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
        path = fs::temp_directory_path() / ("scrivi-fragment-" + std::to_string(
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
    s.fileSystem   = &lfs;
    s.uuidProvider = &uuids;
    s.clock        = &clock;
    s.secureStore  = &store;
    s.gitProvider  = &git;
    return s;
}

static scrivi::AuthorshipRef testAuthor()
{
    return { scrivi::IdentityID{"identity-001"}, scrivi::PersonaID{"persona-001"}, "Test Author" };
}

static const std::string S1A = "Alpha one two three.";
static const std::string S1B = "Bravo four five six.";
static const std::string S2  = "Charlie seven eight nine.";

// A project with chapter 1 (scenes S1a, S1b) and chapter 2 (scene S2).
struct FragProject {
    TempDir projectDir;
    TempDir appSupportDir;
    scrivi::platform::LocalFileSystem        lfs;
    scrivi::mocks::DeterministicUUIDProvider uuids;
    scrivi::mocks::FixedClock                clock{"2026-07-27T00:00:00Z"};
    scrivi::mocks::MockSecureStore           store;
    scrivi::mocks::MockGitProvider           git;
    scrivi::CoreServices                     services;
    scrivi::ScriviCore                       core;

    scrivi::ProjectID projectID;
    scrivi::ChapterID ch1ID, ch2ID;
    scrivi::SceneID   s1aID, s1bID, s2ID;

    FragProject()
        : services(makeServices(lfs, uuids, clock, store, git))
        , core(services)
    {
        scrivi::CreateProjectRequest req;
        req.projectRootPath = projectDir.str();
        req.appSupportRoot  = appSupportDir.str();
        req.title           = "Fragment Test Novel";
        req.slug            = "fragment-test";
        req.author          = testAuthor();
        auto created = core.createProject(req);
        REQUIRE(created.ok());
        projectID = created.value().project.projectID;
        ch1ID     = created.value().firstChapterID;
        s1aID     = created.value().firstSceneID;

        auto save = [&](const scrivi::SceneID& id, const scrivi::RelativePath& meta,
                        const scrivi::RelativePath& content, const std::string& body) {
            scrivi::SaveSceneRequest sv;
            sv.projectID         = projectID;
            sv.projectRootPath   = projectDir.str();
            sv.appSupportRoot    = appSupportDir.str();
            sv.sceneID           = id;
            sv.sceneMetadataPath = meta;
            sv.sceneContentPath  = content;
            sv.markdown          = body;
            sv.author            = testAuthor();
            REQUIRE(core.saveScene(sv).ok());
        };

        // S1a body.
        save(s1aID, created.value().firstSceneMetadataPath,
             created.value().firstSceneContentPath, S1A);

        // S1b — a second scene appended to chapter 1.
        scrivi::CreateSceneRequest s1bReq;
        s1bReq.projectRootPath = projectDir.str();
        s1bReq.appSupportRoot  = appSupportDir.str();
        s1bReq.projectID       = projectID;
        s1bReq.chapterID       = ch1ID;
        s1bReq.afterSceneID    = s1aID;
        s1bReq.author          = testAuthor();
        auto s1b = core.createScene(s1bReq);
        REQUIRE(s1b.ok());
        s1bID = s1b.value().sceneID;
        save(s1bID, s1b.value().metadataPath, s1b.value().contentPath, S1B);

        // Chapter 2 + its scene S2.
        scrivi::CreateChapterRequest chReq;
        chReq.projectRootPath = projectDir.str();
        chReq.appSupportRoot  = appSupportDir.str();
        chReq.projectID       = projectID;
        chReq.author          = testAuthor();
        auto ch2 = core.createChapter(chReq);
        REQUIRE(ch2.ok());
        ch2ID = ch2.value().chapterID;
        s2ID  = ch2.value().firstSceneID;
        save(s2ID, ch2.value().firstSceneMetadataPath, ch2.value().firstSceneContentPath, S2);
    }

    ms::FragmentExtractor extractor() { return ms::FragmentExtractor{services}; }
};

// A span covering a whole scene, given its full body length.
static ms::FragmentSpan whole(const scrivi::SceneID& id, const std::string& body) {
    return ms::FragmentSpan{id, 0, body.size()};
}

// ---------------------------------------------------------------------------
// Single-scene: one-piece fragment (opensWith none)
// ---------------------------------------------------------------------------

TEST_CASE("extract - single whole scene is a one-piece fragment", "[integration][T-0351]") {
    FragProject p;
    auto ex = p.extractor();
    auto r = ex.extract(p.projectDir.str(), { whole(p.s1aID, S1A) });
    REQUIRE(r.ok());
    const auto& f = r.value();
    REQUIRE(f.pieces.size() == 1);
    CHECK(f.pieces[0].opensWith == ms::OpensWith::None);
    CHECK(f.pieces[0].text == S1A);
    CHECK(f.pieces[0].partial == ms::Partial::None);   // whole scene, exact bounds
    CHECK(f.plainText == S1A);
}

TEST_CASE("extract - single partial scene marks head/tail", "[integration][T-0351]") {
    FragProject p;
    auto ex = p.extractor();
    // "Alpha one two three." — bytes 6..13 = "one two"
    auto r = ex.extract(p.projectDir.str(), { ms::FragmentSpan{p.s1aID, 6, 13} });
    REQUIRE(r.ok());
    const auto& f = r.value();
    REQUIRE(f.pieces.size() == 1);
    CHECK(f.pieces[0].text == "one two");
    // Started past 0 → head; a single partial piece records head (model stays total).
    CHECK(f.pieces[0].partial == ms::Partial::Head);
    CHECK(f.plainText == "one two");
}

// ---------------------------------------------------------------------------
// Cross-scene, same chapter: two pieces (opensWith scene)
// ---------------------------------------------------------------------------

TEST_CASE("extract - across two scenes in one chapter → opensWith scene", "[integration][T-0351]") {
    FragProject p;
    auto ex = p.extractor();
    auto r = ex.extract(p.projectDir.str(), { whole(p.s1aID, S1A), whole(p.s1bID, S1B) });
    REQUIRE(r.ok());
    const auto& f = r.value();
    REQUIRE(f.pieces.size() == 2);
    CHECK(f.pieces[0].opensWith == ms::OpensWith::None);
    CHECK(f.pieces[0].text == S1A);
    CHECK(f.pieces[1].opensWith == ms::OpensWith::Scene);   // same chapter → scene boundary
    CHECK(f.pieces[1].chapterTitle.empty());
    CHECK(f.pieces[1].text == S1B);
    CHECK(f.plainText == S1A + "\n\n" + S1B);               // blank-line seam
}

// ---------------------------------------------------------------------------
// Cross-chapter: opensWith chapter + chapterTitle
// ---------------------------------------------------------------------------

TEST_CASE("extract - across a chapter boundary → opensWith chapter + title", "[integration][T-0351]") {
    FragProject p;
    // Confirm the resolver reports a chapter title for chapter 2 so we know what to expect.
    ms::ManuscriptOrderResolver resolver{p.services};
    auto resolved = resolver.resolve(p.projectDir.str());
    REQUIRE(resolved.ok());
    REQUIRE(resolved.value().size() == 3);
    const std::string ch2Title = resolved.value()[2].chapterTitle;

    auto ex = p.extractor();
    // Span S1b (ch1) → S2 (ch2): the second piece opens a new chapter.
    auto r = ex.extract(p.projectDir.str(), { whole(p.s1bID, S1B), whole(p.s2ID, S2) });
    REQUIRE(r.ok());
    const auto& f = r.value();
    REQUIRE(f.pieces.size() == 2);
    CHECK(f.pieces[0].opensWith == ms::OpensWith::None);
    CHECK(f.pieces[1].opensWith == ms::OpensWith::Chapter);
    CHECK(f.pieces[1].chapterTitle == ch2Title);
    CHECK(f.pieces[1].text == S2);
}

// ---------------------------------------------------------------------------
// Three pieces: head + whole interior + tail, spanning all three scenes/two chapters
// ---------------------------------------------------------------------------

TEST_CASE("extract - head/interior/tail across three scenes", "[integration][T-0351]") {
    FragProject p;
    auto ex = p.extractor();
    // S1a bytes 6..end ("one two three."), whole S1b, S2 bytes 0..7 ("Charlie").
    std::vector<ms::FragmentSpan> spans = {
        ms::FragmentSpan{p.s1aID, 6, S1A.size()},
        whole(p.s1bID, S1B),
        ms::FragmentSpan{p.s2ID, 0, 7},
    };
    auto r = ex.extract(p.projectDir.str(), spans);
    REQUIRE(r.ok());
    const auto& f = r.value();
    REQUIRE(f.pieces.size() == 3);

    CHECK(f.pieces[0].opensWith == ms::OpensWith::None);
    CHECK(f.pieces[0].partial   == ms::Partial::Head);       // started past byte 0
    CHECK(f.pieces[0].text == "one two three.");

    CHECK(f.pieces[1].opensWith == ms::OpensWith::Scene);    // same chapter as S1a
    CHECK(f.pieces[1].partial   == ms::Partial::None);       // whole interior scene
    CHECK(f.pieces[1].text == S1B);

    CHECK(f.pieces[2].opensWith == ms::OpensWith::Chapter);  // S2 is a new chapter
    CHECK(f.pieces[2].partial   == ms::Partial::Tail);       // ended before end of S2
    CHECK(f.pieces[2].text == "Charlie");

    CHECK(f.plainText == "one two three." "\n\n" + S1B + "\n\n" "Charlie");
}

// ---------------------------------------------------------------------------
// Empty piece: a boundary crossed with zero characters selected from the next scene
// ---------------------------------------------------------------------------

TEST_CASE("extract - zero-length second span still produces a piece", "[integration][T-0351]") {
    FragProject p;
    auto ex = p.extractor();
    // Whole S1a, then a 0..0 span on S1b (selection ended exactly at the boundary).
    auto r = ex.extract(p.projectDir.str(), { whole(p.s1aID, S1A), ms::FragmentSpan{p.s1bID, 0, 0} });
    REQUIRE(r.ok());
    const auto& f = r.value();
    REQUIRE(f.pieces.size() == 2);
    CHECK(f.pieces[1].opensWith == ms::OpensWith::Scene);
    CHECK(f.pieces[1].text.empty());
    // Ended before end of S1b → tail partial.
    CHECK(f.pieces[1].partial == ms::Partial::Tail);
}

// ---------------------------------------------------------------------------
// Validation: rejects malformed spans
// ---------------------------------------------------------------------------

TEST_CASE("extract - empty spans rejected", "[integration][T-0351]") {
    FragProject p;
    auto r = p.extractor().extract(p.projectDir.str(), {});
    REQUIRE_FALSE(r.ok());
    CHECK(r.error().code == scrivi::ErrorCode::invalidArgument);
}

TEST_CASE("extract - unknown scene rejected", "[integration][T-0351]") {
    FragProject p;
    auto r = p.extractor().extract(p.projectDir.str(), { ms::FragmentSpan{scrivi::SceneID{"nope"}, 0, 1} });
    REQUIRE_FALSE(r.ok());
    CHECK(r.error().code == scrivi::ErrorCode::invalidArgument);
}

TEST_CASE("extract - startByte > endByte rejected", "[integration][T-0351]") {
    FragProject p;
    auto r = p.extractor().extract(p.projectDir.str(), { ms::FragmentSpan{p.s1aID, 5, 2} });
    REQUIRE_FALSE(r.ok());
    CHECK(r.error().code == scrivi::ErrorCode::invalidArgument);
}

TEST_CASE("extract - span past end of scene body rejected", "[integration][T-0351]") {
    FragProject p;
    auto r = p.extractor().extract(p.projectDir.str(),
                                   { ms::FragmentSpan{p.s1aID, 0, S1A.size() + 100} });
    REQUIRE_FALSE(r.ok());
    CHECK(r.error().code == scrivi::ErrorCode::invalidArgument);
}

TEST_CASE("extract - out-of-order spans rejected", "[integration][T-0351]") {
    FragProject p;
    // S1b then S1a — reverse reading order.
    auto r = p.extractor().extract(p.projectDir.str(), { whole(p.s1bID, S1B), whole(p.s1aID, S1A) });
    REQUIRE_FALSE(r.ok());
    CHECK(r.error().code == scrivi::ErrorCode::invalidArgument);
}
