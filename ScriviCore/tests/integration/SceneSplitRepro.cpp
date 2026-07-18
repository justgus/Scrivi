// SceneSplitRepro.cpp
// Reproduce the Linux "Split chapter at caret" (Ctrl+Shift+Return) orchestration on the
// EP-027 P6 scene model, to pin the on-disk corruption reported for "The Sentinel of
// Cenuri" (B3/B4). The split, for a caret in scene S of chapter C with followers F*, is now:
//   createChapter(afterChapterID = C)  → K born IN PLACE (final folder name, no reorder)
//   → for each follower f: reorderScene(f, C, K, afterID)  [afterID chains]
// We assert the chapter is born with its final key (no intermediate chapter-w), files end
// up in the RIGHT folders with NO filename collision, and every scene resolves. The last
// test proves createChapter(afterChapterID) itself never creates-then-renames.

#include <catch2/catch_test_macros.hpp>

#include "scrivi/ScriviCore.hpp"
#include "scrivi/Requests.hpp"

#include "mocks/DeterministicUUIDProvider.hpp"
#include "mocks/FixedClock.hpp"
#include "mocks/MockGitProvider.hpp"
#include "mocks/MockSecureStore.hpp"

#include "platform/LocalFileSystem.hpp"
#include "manuscript/ChapterIndex.hpp"
#include "manuscript/SceneIndex.hpp"
#include "manuscript/ManuscriptOrderResolver.hpp"
#include "schemas/ChapterMetaJson.hpp"

#include <algorithm>
#include <chrono>
#include <filesystem>
#include <string>
#include <vector>

namespace fs = std::filesystem;

namespace {
struct TempDir {
    fs::path path;
    TempDir() {
        path = fs::temp_directory_path() / ("scrivi-split-" + std::to_string(
            std::chrono::steady_clock::now().time_since_epoch().count()));
        fs::create_directories(path);
    }
    ~TempDir() { std::error_code ec; fs::remove_all(path, ec); }
    std::string str() const { return path.string(); }
};
scrivi::CoreServices makeServices(
    scrivi::platform::LocalFileSystem& lfs, scrivi::mocks::DeterministicUUIDProvider& u,
    scrivi::mocks::FixedClock& c, scrivi::mocks::MockSecureStore& s,
    scrivi::mocks::MockGitProvider& g) {
    scrivi::CoreServices sv; sv.fileSystem=&lfs; sv.uuidProvider=&u; sv.clock=&c;
    sv.secureStore=&s; sv.gitProvider=&g; return sv;
}
scrivi::AuthorshipRef author() {
    return { scrivi::IdentityID{"identity-001"}, scrivi::PersonaID{"persona-001"}, "A" };
}
} // namespace

TEST_CASE("split repro - createChapter(after C) + move-followers keeps files sane",
          "[integration][EP-027][scenes][split]")
{
    TempDir proj, apps;
    scrivi::platform::LocalFileSystem lfs;
    scrivi::mocks::DeterministicUUIDProvider uuids;
    scrivi::mocks::FixedClock clock{"2026-07-17T00:00:00Z"};
    scrivi::mocks::MockSecureStore store;
    scrivi::mocks::MockGitProvider git;
    auto services = makeServices(lfs, uuids, clock, store, git);
    scrivi::ScriviCore core{services};

    // A project: Chapter C (=chapter-001) with THREE scenes s1,s2,s3.
    scrivi::CreateProjectRequest pr;
    pr.projectRootPath=proj.str(); pr.appSupportRoot=apps.str();
    pr.title="Novel"; pr.slug="novel"; pr.author=author();
    auto created = core.createProject(pr); REQUIRE(created.ok());
    const auto projectID = created.value().project.projectID;
    const auto chapterC  = created.value().firstChapterID;
    const auto s1        = created.value().firstSceneID;

    auto addScene = [&](const scrivi::ChapterID& ch, const scrivi::SceneID& after) {
        scrivi::CreateSceneRequest r;
        r.projectRootPath=proj.str(); r.appSupportRoot=apps.str();
        r.projectID=projectID; r.chapterID=ch; r.afterSceneID=after; r.author=author();
        auto rr = core.createScene(r); REQUIRE(rr.ok()); return rr.value().sceneID;
    };
    const auto s2 = addScene(chapterC, s1);
    const auto s3 = addScene(chapterC, s2);

    scrivi::manuscript::ManuscriptOrderResolver resolver{services};
    REQUIRE(resolver.resolve(proj.str()).value().size() == 3);

    // --- Simulate the split with the caret in s1 (S=s1), followers = [s2, s3]. ---
    // 1. createChapter K IN PLACE right after C (EP-027 P6 create-in-place — no reorder,
    //    no intermediate folder rename). K0 is its blank first scene. This mirrors the new
    //    single-call the app makes; the K0 paths it returns stay valid (no rename).
    scrivi::CreateChapterRequest cr;
    cr.projectRootPath=proj.str(); cr.appSupportRoot=apps.str();
    cr.projectID=projectID; cr.author=author();
    cr.afterChapterID=chapterC;
    auto kR = core.createChapter(cr); REQUIRE(kR.ok());
    const auto chapterK = kR.value().chapterID;
    const auto k0       = kR.value().firstSceneID;

    // The chapter was born with its FINAL folder name (chapter-c, between chapter-001's key
    // and the next) — an intermediate chapter-w (keyAfter the last) must NEVER have existed.
    CHECK_FALSE(fs::exists(fs::path(proj.str()) / "manuscript/chapter-w"));
    // K0's returned metadata path resolves (create-in-place → no stale path).
    CHECK(fs::exists(fs::path(proj.str()) /
          kR.value().firstSceneMetadataPath));

    // 2. Move followers into K after K0, chaining afterID (exactly as EditorShell does).
    scrivi::SceneID afterID = k0;
    for (const auto& f : {s2, s3}) {
        scrivi::ReorderSceneRequest r;
        r.projectRootPath=proj.str();
        r.sceneID=f; r.sourceChapterID=chapterC; r.targetChapterID=chapterK;
        r.afterSceneID=afterID;
        auto rr = core.reorderScene(r);
        INFO("reorderScene f=" << f.value << " ok=" << rr.ok()
             << " err=" << (rr.ok()? std::string() : rr.error().message));
        REQUIRE(rr.ok());
        afterID = f;
    }

    // --- Assertions: on-disk sanity. ---
    // C keeps s1 only; K holds [k0, s2, s3] in order.
    auto chapters = scrivi::manuscript::listChaptersByOrder(lfs, proj.str());
    REQUIRE(chapters.ok());
    for (auto& ch : chapters.value()) {
        auto scenes = scrivi::manuscript::listScenesByOrder(
            lfs, proj.str(), ch.chapterMetadataRelPath);
        REQUIRE(scenes.ok());
        if (ch.chapterID.value == chapterC.value) {
            CHECK(scenes.value().size() == 1);
            CHECK(scenes.value()[0].sceneID.value == s1.value);
        } else if (ch.chapterID.value == chapterK.value) {
            REQUIRE(scenes.value().size() == 3);
            CHECK(scenes.value()[0].sceneID.value == k0.value);
            CHECK(scenes.value()[1].sceneID.value == s2.value);
            CHECK(scenes.value()[2].sceneID.value == s3.value);
        }
    }

    // No two scene files anywhere share a filename (the collision you saw across folders).
    std::vector<std::string> allSceneFiles;
    for (auto& ch : chapters.value()) {
        for (auto& e : scrivi::manuscript::listScenesByOrder(
                 lfs, proj.str(), ch.chapterMetadataRelPath).value()) {
            allSceneFiles.push_back(e.metadataFilename);
        }
    }
    std::sort(allSceneFiles.begin(), allSceneFiles.end());
    CHECK(std::adjacent_find(allSceneFiles.begin(), allSceneFiles.end())
          == allSceneFiles.end());   // no duplicate filenames

    // Full manuscript still has exactly the 4 scenes (s1, k0, s2, s3), all resolvable.
    auto finalR = resolver.resolve(proj.str());
    REQUIRE(finalR.ok());
    CHECK(finalR.value().size() == 4);
}

TEST_CASE("split repro - moving a scene whose order-key filename already exists in the "
          "destination must not clobber or fail silently",
          "[integration][EP-027][scenes][split]")
{
    TempDir proj, apps;
    scrivi::platform::LocalFileSystem lfs;
    scrivi::mocks::DeterministicUUIDProvider uuids;
    scrivi::mocks::FixedClock clock{"2026-07-17T00:00:00Z"};
    scrivi::mocks::MockSecureStore store;
    scrivi::mocks::MockGitProvider git;
    auto services = makeServices(lfs, uuids, clock, store, git);
    scrivi::ScriviCore core{services};

    scrivi::CreateProjectRequest pr;
    pr.projectRootPath=proj.str(); pr.appSupportRoot=apps.str();
    pr.title="Novel"; pr.slug="novel"; pr.author=author();
    auto created = core.createProject(pr); REQUIRE(created.ok());
    const auto projectID = created.value().project.projectID;
    const auto chapterC  = created.value().firstChapterID;
    const auto s1        = created.value().firstSceneID;

    auto addScene = [&](const scrivi::ChapterID& ch, const scrivi::SceneID& after) {
        scrivi::CreateSceneRequest r;
        r.projectRootPath=proj.str(); r.appSupportRoot=apps.str();
        r.projectID=projectID; r.chapterID=ch; r.afterSceneID=after; r.author=author();
        auto rr = core.createScene(r); REQUIRE(rr.ok()); return rr.value().sceneID;
    };
    // C: [s1, s2] where s2's key == keyAfter("001") == "V" → file "V-scene.*".
    const auto s2 = addScene(chapterC, s1);

    // Create chapter K; its first scene k0 is "001-scene". Add kS after k0 → key "V" too,
    // so K already has a "V-scene.*" file.
    scrivi::CreateChapterRequest cr;
    cr.projectRootPath=proj.str(); cr.appSupportRoot=apps.str();
    cr.projectID=projectID; cr.author=author();
    auto kR = core.createChapter(cr); REQUIRE(kR.ok());
    const auto chapterK = kR.value().chapterID;
    const auto k0       = kR.value().firstSceneID;
    const auto kS       = addScene(chapterK, k0);   // K: [k0(001), kS(V)]

    // Now move s2 (file "V-scene") from C to the END of K. The naive dest key would be
    // keyAfter(kS="V") == "k" — but if the reorderer instead reused "V" it would COLLIDE
    // with kS's existing "V-scene" file. Assert the move succeeds and no filename collides.
    scrivi::ReorderSceneRequest r;
    r.projectRootPath=proj.str();
    r.sceneID=s2; r.sourceChapterID=chapterC; r.targetChapterID=chapterK;
    r.afterSceneID=kS;   // after the last scene in K
    auto rr = core.reorderScene(r);
    INFO("err=" << (rr.ok()? std::string() : rr.error().message));
    REQUIRE(rr.ok());

    // K now has 3 scenes; no two share a filename; all resolvable.
    auto scenes = scrivi::manuscript::listScenesByOrder(
        lfs, proj.str(),
        scrivi::manuscript::findChapterByID(lfs, proj.str(), chapterK).value()
            .chapterMetadataRelPath);
    REQUIRE(scenes.ok());
    CHECK(scenes.value().size() == 3);
    std::vector<std::string> names;
    for (auto& e : scenes.value()) names.push_back(e.metadataFilename);
    std::sort(names.begin(), names.end());
    CHECK(std::adjacent_find(names.begin(), names.end()) == names.end());

    scrivi::manuscript::ManuscriptOrderResolver resolver{services};
    CHECK(resolver.resolve(proj.str()).value().size() == 4);   // s1 in C + k0,kS,s2 in K
}

TEST_CASE("createChapter(afterChapterID) - born in place; no intermediate folder, no rename",
          "[integration][EP-027][chapters][create-in-place]")
{
    TempDir proj, apps;
    scrivi::platform::LocalFileSystem lfs;
    scrivi::mocks::DeterministicUUIDProvider uuids;
    scrivi::mocks::FixedClock clock{"2026-07-17T00:00:00Z"};
    scrivi::mocks::MockSecureStore store;
    scrivi::mocks::MockGitProvider git;
    auto services = makeServices(lfs, uuids, clock, store, git);
    scrivi::ScriviCore core{services};

    scrivi::CreateProjectRequest pr;
    pr.projectRootPath=proj.str(); pr.appSupportRoot=apps.str();
    pr.title="Novel"; pr.slug="novel"; pr.author=author();
    auto created = core.createProject(pr); REQUIRE(created.ok());
    const auto projectID = created.value().project.projectID;
    const auto ch1 = created.value().firstChapterID;   // chapter-001

    auto appendChapter = [&]() {
        scrivi::CreateChapterRequest r;
        r.projectRootPath=proj.str(); r.appSupportRoot=apps.str();
        r.projectID=projectID; r.author=author();   // afterChapterID empty → append
        auto rr = core.createChapter(r); REQUIRE(rr.ok()); return rr.value().chapterID;
    };
    // Build the exact "Sentinel of Cenuri" shape: chapters 001, V, k, s.
    const auto ch2 = appendChapter();   // chapter-V
    appendChapter();                    // chapter-k
    appendChapter();                    // chapter-s

    auto folderNames = [&]() {
        std::vector<std::string> names;
        for (auto& e : fs::directory_iterator(fs::path(proj.str()) / "manuscript"))
            if (e.is_directory()) names.push_back(e.path().filename().string());
        std::sort(names.begin(), names.end());
        return names;
    };
    REQUIRE(folderNames() == std::vector<std::string>{
        "chapter-001", "chapter-V", "chapter-k", "chapter-s"});

    // Now create a chapter AFTER ch2 (=chapter-V). Its key must be keyBetween("V","k")="c",
    // so the folder is born as chapter-c. The append-key ("w", keyAfter "s") must NEVER
    // appear — no create-then-rename.
    scrivi::CreateChapterRequest inPlace;
    inPlace.projectRootPath=proj.str(); inPlace.appSupportRoot=apps.str();
    inPlace.projectID=projectID; inPlace.author=author();
    inPlace.afterChapterID=ch2;
    auto kR = core.createChapter(inPlace); REQUIRE(kR.ok());

    const auto names = folderNames();
    CHECK(std::find(names.begin(), names.end(), "chapter-c") != names.end());
    CHECK(std::find(names.begin(), names.end(), "chapter-w") == names.end());

    // Manuscript order: the new chapter sits right after ch2.
    auto ordered = scrivi::manuscript::listChaptersByOrder(lfs, proj.str());
    REQUIRE(ordered.ok());
    std::vector<std::string> ids;
    for (auto& e : ordered.value()) ids.push_back(e.chapterID.value);
    // Expect: ch1, ch2, K, then the rest (k, s).
    REQUIRE(ids.size() == 5);
    CHECK(ids[0] == ch1.value);
    CHECK(ids[1] == ch2.value);
    CHECK(ids[2] == kR.value().chapterID.value);

    // K0's returned metadata path is valid on disk (no stale path).
    CHECK(fs::exists(fs::path(proj.str()) / kR.value().firstSceneMetadataPath));
}

// Regression for the "Centauri" corruption: a mid-scene split in Ch2 (each chapter has
// scenes with the SAME order-key filenames 001/V/k, since keys restart per chapter) must
// NOT drag an identically-NAMED scene out of a DIFFERENT chapter during open-time
// orphan-repair. The bug: migrateScenes matched orphans by filename, and k-scene exists in
// every chapter, so it relocated the wrong one.
TEST_CASE("split repro (Centauri) - open after split does not steal a same-named scene "
          "from another chapter", "[integration][EP-027][scenes][split][I-0074]")
{
    TempDir proj, apps;
    scrivi::platform::LocalFileSystem lfs;
    scrivi::mocks::DeterministicUUIDProvider uuids;
    scrivi::mocks::FixedClock clock{"2026-07-18T00:00:00Z"};
    scrivi::mocks::MockSecureStore store;
    scrivi::mocks::MockGitProvider git;
    auto services = makeServices(lfs, uuids, clock, store, git);
    scrivi::ScriviCore core{services};

    scrivi::CreateProjectRequest pr;
    pr.projectRootPath=proj.str(); pr.appSupportRoot=apps.str();
    pr.title="Novel"; pr.slug="novel"; pr.author=author();
    auto created = core.createProject(pr); REQUIRE(created.ok());
    const auto projectID = created.value().project.projectID;
    const auto ch1 = created.value().firstChapterID;
    const auto ch1s1 = created.value().firstSceneID;

    auto addScene = [&](const scrivi::ChapterID& ch, const scrivi::SceneID& after) {
        scrivi::CreateSceneRequest r;
        r.projectRootPath=proj.str(); r.appSupportRoot=apps.str();
        r.projectID=projectID; r.chapterID=ch; r.afterSceneID=after; r.author=author();
        auto rr = core.createScene(r); REQUIRE(rr.ok()); return rr.value().sceneID;
    };
    auto addChapter = [&](const scrivi::ChapterID& after) {
        scrivi::CreateChapterRequest r;
        r.projectRootPath=proj.str(); r.appSupportRoot=apps.str();
        r.projectID=projectID; r.author=author(); r.afterChapterID=after;
        auto rr = core.createChapter(r); REQUIRE(rr.ok()); return rr.value();
    };

    // Ch1: 3 scenes (001, V, k). The 3rd (ch1s3) is the one the buggy repair used to steal.
    const auto ch1s2 = addScene(ch1, ch1s1);
    const auto ch1s3 = addScene(ch1, ch1s2);   // Ch1's k-scene

    // Ch2: 3 scenes. Split will happen in ch2s2; ch2s3 is the single follower.
    auto ch2r = addChapter(ch1); const auto ch2 = ch2r.chapterID; const auto ch2s1 = ch2r.firstSceneID;
    const auto ch2s2 = addScene(ch2, ch2s1);
    const auto ch2s3 = addScene(ch2, ch2s2);   // Ch2's k-scene (the follower)

    // Simulate the split: createChapter(after Ch2) → move follower ch2s3 into K.
    auto kr = addChapter(ch2); const auto chK = kr.chapterID; const auto k0 = kr.firstSceneID;
    {
        scrivi::ReorderSceneRequest r;
        r.projectRootPath=proj.str();
        r.sceneID=ch2s3; r.sourceChapterID=ch2; r.targetChapterID=chK; r.afterSceneID=k0;
        REQUIRE(core.reorderScene(r).ok());
    }

    // Now OPEN the project (this runs migrateScenes / self-heal — where the bug fired).
    scrivi::OpenProjectRequest openReq;
    openReq.projectRootPath=proj.str(); openReq.appSupportRoot=apps.str();
    auto opened = core.openProject(openReq);
    REQUIRE(opened.ok());
    // It must open cleanly — NOT repairRequired.
    CHECK(opened.value().mode != scrivi::OpenMode::repairRequired);

    // Ch1 must STILL own all three of its scenes (ch1s3 must NOT have been stolen).
    auto ch1Dir = scrivi::manuscript::findChapterByID(lfs, proj.str(), ch1).value()
                      .chapterMetadataRelPath;
    auto ch1Scenes = scrivi::manuscript::listScenesByOrder(lfs, proj.str(), ch1Dir);
    REQUIRE(ch1Scenes.ok());
    std::vector<std::string> ch1ids;
    for (auto& e : ch1Scenes.value()) ch1ids.push_back(e.sceneID.value);
    CHECK(std::find(ch1ids.begin(), ch1ids.end(), ch1s3.value) != ch1ids.end());
    CHECK(ch1Scenes.value().size() == 3);

    // Every scene resolves; total count is intact (7 originals + K0 = 8).
    scrivi::manuscript::ManuscriptOrderResolver resolver{services};
    auto all = resolver.resolve(proj.str());
    REQUIRE(all.ok());
    CHECK(all.value().size() == 7);   // Ch1:3 + Ch2:2 (one moved out) + K:2 (K0 + moved)
}
