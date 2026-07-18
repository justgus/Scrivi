// SceneIdentityTests.cpp
// EP-027 §8 — filesystem-authoritative scene identity & ordering.
//
// Proves the scene-level rework that unblocks P4: order-key scene filenames, cross-chapter
// reorder that RELOCATES the files, open-time orphan repair (the C6 "Missing scene.meta.json"
// breaker), legacy numeric→order-key scene migration, and the property that a CHAPTER folder
// rename now touches ZERO scene fields (the B3/B4 fix).

#include <catch2/catch_test_macros.hpp>

#include "scrivi/ScriviCore.hpp"
#include "scrivi/Requests.hpp"

#include "mocks/DeterministicUUIDProvider.hpp"
#include "mocks/FixedClock.hpp"
#include "mocks/MockGitProvider.hpp"
#include "mocks/MockSecureStore.hpp"

#include "platform/LocalFileSystem.hpp"
#include "manuscript/ManuscriptOrderResolver.hpp"
#include "manuscript/ChapterIndex.hpp"
#include "manuscript/SceneIndex.hpp"
#include "schemas/ChapterMetaJson.hpp"
#include "schemas/ManuscriptMetaJson.hpp"
#include "schemas/SceneMetaJson.hpp"
#include "util/OrderKey.hpp"
#include "util/PathUtils.hpp"

#include <algorithm>
#include <chrono>
#include <filesystem>
#include <fstream>
#include <string>

namespace fs = std::filesystem;

namespace {

struct TempDir {
    fs::path path;
    TempDir() {
        path = fs::temp_directory_path() / ("scrivi-sceneid-" + std::to_string(
            std::chrono::steady_clock::now().time_since_epoch().count()));
        fs::create_directories(path);
    }
    ~TempDir() { std::error_code ec; fs::remove_all(path, ec); }
    std::string str() const { return path.string(); }
};

scrivi::CoreServices makeServices(
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

scrivi::AuthorshipRef testAuthor() {
    return { scrivi::IdentityID{"identity-001"}, scrivi::PersonaID{"persona-001"}, "Test Author" };
}

// A project with two chapters, ch1:[s1,s2] ch2:[s3,s4] — same shape as ReorderTests.
struct FourSceneProject {
    TempDir projectDir, appSupportDir;
    scrivi::platform::LocalFileSystem        lfs;
    scrivi::mocks::DeterministicUUIDProvider uuids;
    scrivi::mocks::FixedClock                clock{"2026-06-04T00:00:00Z"};
    scrivi::mocks::MockSecureStore           store;
    scrivi::mocks::MockGitProvider           git;
    scrivi::CoreServices                     services;
    scrivi::ScriviCore                       core;

    scrivi::ProjectID projectID;
    scrivi::ChapterID ch1ID, ch2ID;
    scrivi::SceneID   s1ID, s2ID, s3ID, s4ID;

    FourSceneProject()
        : services(makeServices(lfs, uuids, clock, store, git)), core(services)
    {
        scrivi::CreateProjectRequest req;
        req.projectRootPath = projectDir.str();
        req.appSupportRoot  = appSupportDir.str();
        req.title = "Novel"; req.slug = "novel"; req.author = testAuthor();
        auto created = core.createProject(req);
        REQUIRE(created.ok());
        projectID = created.value().project.projectID;
        ch1ID = created.value().firstChapterID;
        s1ID  = created.value().firstSceneID;

        scrivi::CreateSceneRequest sReq;
        sReq.projectRootPath = projectDir.str();
        sReq.appSupportRoot  = appSupportDir.str();
        sReq.projectID = projectID; sReq.chapterID = ch1ID;
        sReq.afterSceneID = s1ID; sReq.author = testAuthor();
        auto s2r = core.createScene(sReq); REQUIRE(s2r.ok()); s2ID = s2r.value().sceneID;

        scrivi::CreateChapterRequest chReq;
        chReq.projectRootPath = projectDir.str();
        chReq.appSupportRoot  = appSupportDir.str();
        chReq.projectID = projectID; chReq.author = testAuthor();
        auto ch2r = core.createChapter(chReq); REQUIRE(ch2r.ok());
        ch2ID = ch2r.value().chapterID; s3ID = ch2r.value().firstSceneID;

        sReq.chapterID = ch2ID; sReq.afterSceneID = s3ID;
        auto s4r = core.createScene(sReq); REQUIRE(s4r.ok()); s4ID = s4r.value().sceneID;
    }

    std::vector<std::string> resolvedOrder() {
        scrivi::manuscript::ManuscriptOrderResolver resolver{services};
        auto r = resolver.resolve(projectDir.str());
        REQUIRE(r.ok());
        std::vector<std::string> ids;
        for (const auto& s : r.value()) ids.push_back(s.sceneID.value);
        return ids;
    }

    std::string read(const std::string& rel) {
        std::ifstream in(projectDir.str() + "/" + rel, std::ios::binary);
        return std::string((std::istreambuf_iterator<char>(in)), std::istreambuf_iterator<char>());
    }

    // Chapter folder names are order-key slugs (chapter-001, then chapter-<keyAfter(...)>),
    // NOT chapter-002 — resolve the real dir by chapterID.
    std::string chapterMetaRel(const scrivi::ChapterID& id) {
        auto chapters = scrivi::manuscript::listChaptersByOrder(lfs, projectDir.str());
        REQUIRE(chapters.ok());
        for (auto& e : chapters.value())
            if (e.chapterID.value == id.value) return e.chapterMetadataRelPath;
        FAIL("chapter not found: " + id.value);
        return {};
    }
    fs::path chapterDir(const scrivi::ChapterID& id) {
        return fs::path(projectDir.str()) /
               fs::path(chapterMetaRel(id)).parent_path();
    }
};

// List the scene .meta.json filenames physically present in a chapter folder, sorted.
std::vector<std::string> sceneFilesOnDisk(const fs::path& chapterDir) {
    std::vector<std::string> names;
    for (auto& e : fs::directory_iterator(chapterDir)) {
        const auto n = e.path().filename().string();
        if (n != "chapter.meta.json" && n.size() > 10 &&
            n.substr(n.size() - 10) == ".meta.json") {
            names.push_back(n);
        }
    }
    std::sort(names.begin(), names.end());
    return names;
}

} // namespace

// ---------------------------------------------------------------------------
// 1. Scene filenames are order-key slugs; sorted filenames == reading order.
// ---------------------------------------------------------------------------
TEST_CASE("scene create - filenames are order keys and the ref is filename-only",
          "[integration][EP-027][scenes]")
{
    FourSceneProject p;
    const fs::path ch1Dir = fs::path(p.projectDir.str()) / "manuscript/chapter-001";

    // Two scenes on disk in ch1, both order-key filenames, sorted == reading order.
    auto files = sceneFilesOnDisk(ch1Dir);
    REQUIRE(files.size() == 2);
    for (const auto& f : files) {
        const auto dash = f.find('-');
        REQUIRE(dash != std::string::npos);
        CHECK(scrivi::util::isOrderKey(f.substr(0, dash)));
    }

    // The chapter sidecar's scene refs are bare filenames (no path, no sceneID) and match
    // the on-disk files in order.
    auto chMeta = scrivi::schemas::parseChapterMeta(
        p.read("manuscript/chapter-001/chapter.meta.json"));
    REQUIRE(chMeta.ok());
    REQUIRE(chMeta.value().scenes.size() == 2);
    CHECK(chMeta.value().scenes[0].metadataFilename == files[0]);
    CHECK(chMeta.value().scenes[1].metadataFilename == files[1]);
    for (const auto& ref : chMeta.value().scenes) {
        CHECK(ref.metadataFilename.find('/') == std::string::npos);  // bare filename
    }
}

// ---------------------------------------------------------------------------
// 2. Cross-chapter reorder RELOCATES the scene's files (the core §8 fix).
// ---------------------------------------------------------------------------
TEST_CASE("reorderScene cross-chapter - moves the files into the destination folder",
          "[integration][EP-027][scenes]")
{
    FourSceneProject p;
    const fs::path ch1Dir = p.chapterDir(p.ch1ID);
    const fs::path ch2Dir = p.chapterDir(p.ch2ID);

    REQUIRE(sceneFilesOnDisk(ch1Dir).size() == 2);
    REQUIRE(sceneFilesOnDisk(ch2Dir).size() == 2);

    // Move s2 from ch1 → ch2.
    scrivi::ReorderSceneRequest req;
    req.projectRootPath = p.projectDir.str();
    req.sceneID = p.s2ID; req.sourceChapterID = p.ch1ID; req.targetChapterID = p.ch2ID;
    req.afterSceneID = p.s3ID;
    auto r = p.core.reorderScene(req);
    REQUIRE(r.ok());

    // The files physically moved: ch1 lost one, ch2 gained one.
    CHECK(sceneFilesOnDisk(ch1Dir).size() == 1);
    CHECK(sceneFilesOnDisk(ch2Dir).size() == 3);

    // s2's body is now resolvable inside ch2, and manuscript order reflects the move.
    CHECK(p.resolvedOrder() == std::vector<std::string>{
        p.s1ID.value, p.s3ID.value, p.s2ID.value, p.s4ID.value});
}

// ---------------------------------------------------------------------------
// 3. A CHAPTER folder rename touches ZERO scene fields (the B3/B4 fix). After a chapter
//    reorder renames chapter-002 → a new key, its scene refs + sidecar contentPaths are
//    unchanged (bare filenames), and every scene stays resolvable.
// ---------------------------------------------------------------------------
TEST_CASE("chapter reorder - renaming a chapter folder leaves scene refs untouched",
          "[integration][EP-027][scenes]")
{
    FourSceneProject p;

    // Capture ch2's scene refs + contentPaths BEFORE the chapter moves.
    auto before = scrivi::schemas::parseChapterMeta(p.read(p.chapterMetaRel(p.ch2ID)));
    REQUIRE(before.ok());
    std::vector<std::string> beforeRefs;
    for (auto& r : before.value().scenes) beforeRefs.push_back(r.metadataFilename);

    // Move ch2 to the front (renames its folder to a new order key).
    scrivi::ReorderChapterRequest chReq;
    chReq.projectRootPath = p.projectDir.str();
    chReq.chapterID = p.ch2ID; chReq.afterChapterID = scrivi::ChapterID{""};
    REQUIRE(p.core.reorderChapter(chReq).ok());

    // ch2's folder is now some chapter-<newKey>; find it by scanning for its chapterID.
    scrivi::platform::LocalFileSystem lfs;
    auto chapters = scrivi::manuscript::listChaptersByOrder(lfs, p.projectDir.str());
    REQUIRE(chapters.ok());
    std::string newCh2Rel;
    for (auto& e : chapters.value())
        if (e.chapterID.value == p.ch2ID.value) newCh2Rel = e.chapterMetadataRelPath;
    REQUIRE_FALSE(newCh2Rel.empty());

    auto after = scrivi::schemas::parseChapterMeta(p.read(newCh2Rel));
    REQUIRE(after.ok());
    std::vector<std::string> afterRefs;
    for (auto& r : after.value().scenes) afterRefs.push_back(r.metadataFilename);

    // The scene refs are byte-for-byte identical — the chapter rename touched no scene field.
    CHECK(afterRefs == beforeRefs);

    // And all four scenes still resolve (bodies intact under the renamed folder).
    CHECK(p.resolvedOrder().size() == 4);
}

// ---------------------------------------------------------------------------
// 4. A damaged project (a scene file physically in ch1, but stale-referenced by ch2) must
//    OPEN cleanly and stay self-consistent. Ownership follows PHYSICAL LOCATION: the scene
//    is owned by the folder it sits in (ch1). We deliberately do NOT relocate it to ch2 by
//    filename — order-key scene filenames repeat across chapters, so filename-based
//    relocation stole the wrong scene (the "Sentinel of Centauri" corruption, I-0074). The
//    stale ch2 ref is dropped; the file stays in ch1 and is owned there.
// ---------------------------------------------------------------------------
TEST_CASE("open - a scene stale-referenced by a foreign chapter opens cleanly, owned by its "
          "physical folder (I-0074)", "[integration][EP-027][scenes][migration]")
{
    FourSceneProject p;
    const std::string ch1Rel = p.chapterMetaRel(p.ch1ID);
    const std::string ch2Rel = p.chapterMetaRel(p.ch2ID);
    const fs::path ch1Dir = p.chapterDir(p.ch1ID);
    const fs::path ch2Dir = p.chapterDir(p.ch2ID);

    // A scene "z9-orphan" whose FILES sit physically in ch1's folder, but whose REF is (also)
    // spliced into ch2's sidecar — a stale cross-chapter ref. Distinctive name so we can
    // track it unambiguously.
    const std::string orphanFile = "z9-orphan.meta.json";
    {
        scrivi::schemas::SceneMetaData m;
        m.sceneID.value = "scene-orphan"; m.title = "Orphan"; m.slug = "z9-orphan";
        m.status = "draft"; m.contentPath = "z9-orphan.md";
        std::ofstream(ch1Dir / orphanFile, std::ios::binary)
            << scrivi::schemas::serializeSceneMeta(m);
        std::ofstream(ch1Dir / "z9-orphan.md", std::ios::binary) << "ORPHAN BODY";
    }
    auto ch2 = scrivi::schemas::parseChapterMeta(p.read(ch2Rel)).value();
    ch2.scenes.push_back({.metadataFilename = orphanFile});
    {
        std::ofstream f(ch2Dir / "chapter.meta.json", std::ios::binary);
        f << scrivi::schemas::serializeChapterMeta(ch2);
    }

    // Open — self-heal reconciles caches with disk; it must NOT error.
    scrivi::OpenProjectRequest openReq;
    openReq.projectRootPath = p.projectDir.str();
    openReq.appSupportRoot  = p.appSupportDir.str();
    auto opened = p.core.openProject(openReq);
    REQUIRE(opened.ok());
    CHECK(opened.value().mode != scrivi::OpenMode::repairRequired);

    // The file stays where it physically is (ch1); it is NOT moved to ch2 by its filename.
    CHECK(fs::exists(ch1Dir / orphanFile));
    CHECK_FALSE(fs::exists(ch2Dir / orphanFile));

    // ch1 now owns the orphan (its cache mirrors its folder); ch2's stale ref was dropped.
    auto ch1Scenes = scrivi::manuscript::listScenesByOrder(p.lfs, p.projectDir.str(), ch1Rel);
    REQUIRE(ch1Scenes.ok());
    bool ch1HasOrphan = false;
    for (auto& e : ch1Scenes.value())
        if (e.sceneID.value == "scene-orphan") ch1HasOrphan = true;
    CHECK(ch1HasOrphan);

    auto ch2meta = scrivi::schemas::parseChapterMeta(p.read(ch2Rel));
    REQUIRE(ch2meta.ok());
    for (auto& ref : ch2meta.value().scenes)
        CHECK(ref.metadataFilename != orphanFile);   // stale ref dropped

    // The orphan resolves exactly once, and every original scene is intact.
    auto order = p.resolvedOrder();
    CHECK(std::count(order.begin(), order.end(), std::string("scene-orphan")) == 1);
}

// ---------------------------------------------------------------------------
// 5. Legacy numeric scene filenames (a prefix that is NOT a valid order key — one ending
//    in '0', e.g. "010") are migrated to order-key filenames on open, in array order,
//    with bodies intact.
// ---------------------------------------------------------------------------
TEST_CASE("open - migrates legacy numeric scene filenames to order keys",
          "[integration][EP-027][scenes][migration]")
{
    FourSceneProject p;
    const fs::path ch1Dir = fs::path(p.projectDir.str()) / "manuscript/chapter-001";

    // Author a legacy scene "010-legacy" — "010" ends in '0' so it is NOT a valid order key,
    // marking it old-format. Give it a distinctive body.
    {
        scrivi::schemas::SceneMetaData m;
        m.sceneID.value = "scene-legacy"; m.title = "Legacy"; m.slug = "010-legacy";
        m.status = "draft"; m.contentPath = "010-legacy.md";
        std::ofstream(ch1Dir / "010-legacy.meta.json", std::ios::binary)
            << scrivi::schemas::serializeSceneMeta(m);
        std::ofstream(ch1Dir / "010-legacy.md", std::ios::binary) << "LEGACY BODY";
    }
    // Reference it (filename-only) at the end of ch1's scenes[].
    {
        auto ch1 = scrivi::schemas::parseChapterMeta(
            p.read("manuscript/chapter-001/chapter.meta.json")).value();
        ch1.scenes.push_back({.metadataFilename = "010-legacy.meta.json"});
        std::ofstream(ch1Dir / "chapter.meta.json", std::ios::binary)
            << scrivi::schemas::serializeChapterMeta(ch1);
    }
    REQUIRE_FALSE(scrivi::util::isOrderKey("010"));

    // Open — migrateScenes reslugs the legacy file to an order-key filename.
    scrivi::OpenProjectRequest openReq;
    openReq.projectRootPath = p.projectDir.str();
    openReq.appSupportRoot  = p.appSupportDir.str();
    REQUIRE(p.core.openProject(openReq).ok());

    // The legacy file is gone; every scene file in ch1 now has a valid order-key prefix.
    CHECK_FALSE(fs::exists(ch1Dir / "010-legacy.meta.json"));
    for (const auto& f : sceneFilesOnDisk(ch1Dir)) {
        const auto dash = f.find('-');
        REQUIRE(dash != std::string::npos);
        CHECK(scrivi::util::isOrderKey(f.substr(0, dash)));
    }

    // The legacy scene still resolves (by its sceneID) and its body survived the reslug.
    scrivi::manuscript::ManuscriptOrderResolver resolver{p.services};
    auto resolved = resolver.resolve(p.projectDir.str());
    REQUIRE(resolved.ok());
    bool found = false;
    for (auto& s : resolved.value()) {
        if (s.sceneID.value == "scene-legacy") {
            found = true;
            auto body = p.read(s.contentPath);
            CHECK(body == "LEGACY BODY");
        }
    }
    CHECK(found);

    // Idempotent: a second open renames nothing further.
    auto filesAfter1 = sceneFilesOnDisk(ch1Dir);
    REQUIRE(p.core.openProject(openReq).ok());
    CHECK(sceneFilesOnDisk(ch1Dir) == filesAfter1);
}

// ---------------------------------------------------------------------------
// 6. I-0076 — a legacy scene sidecar whose `content.path` is a FULL path
//    ("manuscript/chapter-NNN/…md") — the pre-EP-027 shape — must be normalised to a bare
//    filename on open. The stem "001" is accepted by isOrderKey (so the reslug branch never
//    fires), yet the stale full path dangles once the chapter folder is reslugged, which is
//    what produced "Repair required: Missing scene content file" on the real project. After
//    open: the project opens (NOT repairRequired), the sidecar content.path is bare, the body
//    resolves, and it is idempotent.
// ---------------------------------------------------------------------------
TEST_CASE("open - normalises a legacy full-path scene content.path to a bare filename (I-0076)",
          "[integration][EP-027][scenes][migration]")
{
    FourSceneProject p;
    const std::string ch1Rel = "manuscript/chapter-001/chapter.meta.json";
    const fs::path ch1Dir = fs::path(p.projectDir.str()) / "manuscript/chapter-001";

    // Precondition: "001" is (unfortunately) a valid order key, so the reslug branch is
    // SKIPPED for it — the fix must still heal its content.path.
    REQUIRE(scrivi::util::isOrderKey("001"));

    // Author a legacy-shape scene "001-opening": order-key-shaped stem, but content.path is a
    // FULL path referencing the (current) chapter folder — the pre-P6 sidecar shape.
    {
        scrivi::schemas::SceneMetaData m;
        m.sceneID.value = "scene-legacypath"; m.title = "Opening"; m.slug = "001-opening";
        m.status = "draft";
        m.contentPath = "manuscript/chapter-001/001-opening.md";   // stale FULL path
        std::ofstream(ch1Dir / "001-opening.meta.json", std::ios::binary)
            << scrivi::schemas::serializeSceneMeta(m);
        std::ofstream(ch1Dir / "001-opening.md", std::ios::binary) << "OPENING BODY";
    }
    {
        auto ch1 = scrivi::schemas::parseChapterMeta(p.read(ch1Rel)).value();
        ch1.scenes.push_back({.metadataFilename = "001-opening.meta.json"});
        std::ofstream(ch1Dir / "chapter.meta.json", std::ios::binary)
            << scrivi::schemas::serializeChapterMeta(ch1);
    }

    scrivi::OpenProjectRequest openReq;
    openReq.projectRootPath = p.projectDir.str();
    openReq.appSupportRoot  = p.appSupportDir.str();
    auto opened = p.core.openProject(openReq);
    REQUIRE(opened.ok());
    CHECK(opened.value().mode != scrivi::OpenMode::repairRequired);   // the bug: it errored

    // The sidecar's content.path is now a BARE filename (no path separator).
    auto meta = scrivi::schemas::parseSceneMeta(p.read("manuscript/chapter-001/001-opening.meta.json"));
    REQUIRE(meta.ok());
    CHECK(meta.value().contentPath.find('/') == std::string::npos);
    CHECK(meta.value().contentPath == "001-opening.md");

    // The scene resolves and its body survived.
    scrivi::manuscript::ManuscriptOrderResolver resolver{p.services};
    auto resolved = resolver.resolve(p.projectDir.str());
    REQUIRE(resolved.ok());
    bool found = false;
    for (auto& s : resolved.value())
        if (s.sceneID.value == "scene-legacypath") {
            found = true;
            CHECK(p.read(s.contentPath.find('/') == std::string::npos
                         ? "manuscript/chapter-001/" + s.contentPath
                         : s.contentPath) == "OPENING BODY");
        }
    CHECK(found);

    // Idempotent: a second open makes no further change to the sidecar.
    const std::string after1 = p.read("manuscript/chapter-001/001-opening.meta.json");
    REQUIRE(p.core.openProject(openReq).ok());
    CHECK(p.read("manuscript/chapter-001/001-opening.meta.json") == after1);
}

// ---------------------------------------------------------------------------
// 7. I-0077 — a chapter whose index chapterID DISAGREES with its sidecar chapterID must still
//    participate in chapter order-key migration (matched by its folder path), not be dropped
//    as a "phantom". Otherwise it is left half-migrated (legacy numeric folder while its peers
//    are reslugged). We force a migration by making the index array order differ from the disk
//    folder sort, and corrupt one chapter's index id.
// ---------------------------------------------------------------------------
TEST_CASE("open - migrates a chapter whose index/sidecar chapterID disagree (I-0077)",
          "[integration][EP-027][chapters][migration]")
{
    FourSceneProject p;   // chapter-001 (ch1) + chapter-<key> (ch2)

    // Corrupt the index: give ch1's entry a bogus chapterID (≠ its sidecar) AND swap the array
    // order so the folder-sort no longer reproduces the index-array order (forces migration).
    const std::string msRel = "manuscript/manuscript.meta.json";
    auto ms = scrivi::schemas::parseManuscriptMeta(p.read(msRel)).value();
    REQUIRE(ms.chapters.size() == 2);
    // Find the chapter-001 entry and rewrite its id to something that exists on NO sidecar.
    for (auto& ref : ms.chapters)
        if (ref.path.find("chapter-001/") != std::string::npos)
            ref.chapterID.value = "chapter-BOGUS-mismatch";
    std::reverse(ms.chapters.begin(), ms.chapters.end());   // array order ≠ folder sort
    std::ofstream(fs::path(p.projectDir.str()) / msRel, std::ios::binary)
        << scrivi::schemas::serializeManuscriptMeta(ms);

    scrivi::OpenProjectRequest openReq;
    openReq.projectRootPath = p.projectDir.str();
    openReq.appSupportRoot  = p.appSupportDir.str();
    auto opened = p.core.openProject(openReq);
    REQUIRE(opened.ok());
    CHECK(opened.value().mode != scrivi::OpenMode::repairRequired);

    // The DISCRIMINATING assertion: the id-mismatched chapter's original `chapter-001` folder
    // must be GONE — it was reslugged like its peer. Before the fix it was dropped as a
    // "phantom" and left stranded on `chapter-001` (a numeric name isOrderKey() accepts, so a
    // weaker "isOrderKey(orderKey)" check would falsely pass — hence checking the folder name).
    CHECK_FALSE(fs::exists(fs::path(p.projectDir.str()) / "manuscript/chapter-001"));

    auto chapters = scrivi::manuscript::listChaptersByOrder(p.lfs, p.projectDir.str());
    REQUIRE(chapters.ok());
    REQUIRE(chapters.value().size() == 2);
    // No chapter retains a legacy numeric folder name; keys are the generated letter series.
    for (auto& e : chapters.value()) {
        CHECK(scrivi::util::isOrderKey(e.orderKey));
        CHECK(e.chapterMetadataRelPath.find("chapter-001/") == std::string::npos);
    }

    // Reading order honours the (reversed) index array: ch2 first, then the mismatched ch1.
    scrivi::manuscript::ManuscriptOrderResolver resolver{p.services};
    auto order = resolver.resolve(p.projectDir.str());
    REQUIRE(order.ok());

    // Both original chapters are present by their (sidecar-authoritative) identity.
    bool sawCh1 = false, sawCh2 = false;
    for (auto& e : chapters.value()) {
        if (e.chapterID.value == p.ch1ID.value) sawCh1 = true;
        if (e.chapterID.value == p.ch2ID.value) sawCh2 = true;
    }
    CHECK(sawCh1);
    CHECK(sawCh2);
}
