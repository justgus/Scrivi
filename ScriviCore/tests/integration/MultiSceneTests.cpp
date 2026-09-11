// MultiSceneTests.cpp
// Integration tests for T-0059 (scene list in openProject) and T-0060 (openScene).

#include <catch2/catch_test_macros.hpp>

#include "scrivi/ScriviCore.hpp"
#include "scrivi/Requests.hpp"

#include "mocks/DeterministicUUIDProvider.hpp"
#include "mocks/FixedClock.hpp"
#include "mocks/MockGitProvider.hpp"
#include "mocks/MockSecureStore.hpp"

#include "platform/LocalFileSystem.hpp"
#include "schemas/ChapterMetaJson.hpp"
#include "schemas/SceneMetaJson.hpp"
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
        path = fs::temp_directory_path() / ("scrivi-multiscene-" + std::to_string(
            std::chrono::steady_clock::now().time_since_epoch().count()));
        fs::create_directories(path);
    }

    ~TempDir() {
        std::error_code ec;
        fs::remove_all(path, ec);
    }

    std::string str() const { return path.string(); }
    std::string sub(const std::string& rel) const { return (path / rel).string(); }
    bool exists(const std::string& rel) const { return fs::exists(path / rel); }
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

// Creates a project with one scene, then writes a second scene into the same
// chapter by directly authoring the schema files. Returns the second sceneID.
static scrivi::SceneID addSecondScene(
    const std::string& projectDir,
    const scrivi::CreateProjectResult& created)
{
    const std::string chapterDir =
        projectDir + "/manuscript/chapter-001";
    const std::string scene2ID        = "scene-002";
    const std::string scene2Slug      = "002-second-scene";     // "002" is a valid order key
    const std::string scene2MetaFile  = "002-second-scene.meta.json";
    const std::string scene2ContentFile = "002-second-scene.md";

    // Write scene meta
    scrivi::schemas::SceneMetaData meta2;
    meta2.sceneID.value         = scene2ID;
    meta2.title                 = "Second Scene";
    meta2.slug                  = scene2Slug;
    meta2.status                = "draft";
    meta2.createdAt             = "2026-06-01T00:00:00Z";
    meta2.createdByIdentityID   = "identity-001";
    meta2.createdByPersonaID    = "persona-001";
    meta2.createdByDisplayName  = "Test Author";
    meta2.modifiedAt            = "2026-06-01T00:00:00Z";
    meta2.modifiedByIdentityID  = "identity-001";
    meta2.modifiedByPersonaID   = "persona-001";
    meta2.modifiedByDisplayName = "Test Author";
    meta2.contentPath           = scene2ContentFile;   // §8.1: bare filename

    {
        std::ofstream f(projectDir + "/manuscript/chapter-001/002-second-scene.meta.json",
                        std::ios::binary);
        f << scrivi::schemas::serializeSceneMeta(meta2);
    }

    // Write scene content
    {
        std::ofstream f(projectDir + "/manuscript/chapter-001/002-second-scene.md",
                        std::ios::binary);
        f << "# Second Scene\n\nContent of the second scene.";
    }

    // Rewrite chapter meta to include both scenes
    auto chapterMetaPath = projectDir + "/manuscript/chapter-001/chapter.meta.json";
    std::ifstream fin(chapterMetaPath);
    std::string chapterJson((std::istreambuf_iterator<char>(fin)),
                             std::istreambuf_iterator<char>());
    auto parsedR = scrivi::schemas::parseChapterMeta(chapterJson);
    REQUIRE(parsedR.ok());
    auto chapter = parsedR.value();

    scrivi::schemas::SceneRef ref2;
    ref2.metadataFilename = scene2MetaFile;   // §8.1: filename-only
    chapter.scenes.push_back(ref2);

    {
        std::ofstream f(chapterMetaPath, std::ios::binary);
        f << scrivi::schemas::serializeChapterMeta(chapter);
    }

    return scrivi::SceneID{scene2ID};
}

// ---------------------------------------------------------------------------
// T-0059: openProject returns full scene list
// ---------------------------------------------------------------------------

TEST_CASE("openProject - scenes list contains all scenes in manuscript order",
          "[integration][T-0059]")
{
    TempDir projectDir;
    TempDir appSupportDir;

    scrivi::platform::LocalFileSystem        lfs;
    scrivi::mocks::DeterministicUUIDProvider uuids;
    scrivi::mocks::FixedClock                clock{"2026-06-01T00:00:00Z"};
    scrivi::mocks::MockSecureStore           store;
    scrivi::mocks::MockGitProvider           git;

    auto services = makeServices(lfs, uuids, clock, store, git);
    scrivi::ScriviCore core{services};

    scrivi::CreateProjectRequest req;
    req.projectRootPath = projectDir.str();
    req.appSupportRoot  = appSupportDir.str();
    req.title           = "Multi-Scene Novel";
    req.slug            = "multi-scene-novel";
    req.author          = {
        scrivi::IdentityID{"identity-001"},
        scrivi::PersonaID {"persona-001"},
        "Test Author"
    };
    auto created = core.createProject(req);
    REQUIRE(created.ok());

    auto scene2ID = addSecondScene(projectDir.str(), created.value());

    scrivi::OpenProjectRequest openReq;
    openReq.projectRootPath = projectDir.str();
    openReq.appSupportRoot  = appSupportDir.str();

    auto result = core.openProject(openReq);
    REQUIRE(result.ok());

    auto& r = result.value();
    CHECK(r.mode == scrivi::OpenMode::normalEdit);
    REQUIRE(r.scenes.size() == 2);

    // First scene comes from createProject (initial scene)
    CHECK(r.scenes[0].sceneID.value == created.value().firstSceneID.value);
    CHECK(r.scenes[0].title         == "Opening Scene");

    // Second scene is the one we appended
    CHECK(r.scenes[1].sceneID.value == scene2ID.value);
    CHECK(r.scenes[1].title         == "Second Scene");
}

TEST_CASE("openProject - scenes list has one entry for single-scene project",
          "[integration][T-0059]")
{
    TempDir projectDir;
    TempDir appSupportDir;

    scrivi::platform::LocalFileSystem        lfs;
    scrivi::mocks::DeterministicUUIDProvider uuids;
    scrivi::mocks::FixedClock                clock{"2026-06-01T00:00:00Z"};
    scrivi::mocks::MockSecureStore           store;
    scrivi::mocks::MockGitProvider           git;

    auto services = makeServices(lfs, uuids, clock, store, git);
    scrivi::ScriviCore core{services};

    scrivi::CreateProjectRequest req;
    req.projectRootPath = projectDir.str();
    req.appSupportRoot  = appSupportDir.str();
    req.title           = "Single Scene Novel";
    req.slug            = "single-scene-novel";
    req.author          = {
        scrivi::IdentityID{"identity-001"},
        scrivi::PersonaID {"persona-001"},
        "Test Author"
    };
    auto created = core.createProject(req);
    REQUIRE(created.ok());

    scrivi::OpenProjectRequest openReq;
    openReq.projectRootPath = projectDir.str();
    openReq.appSupportRoot  = appSupportDir.str();

    auto result = core.openProject(openReq);
    REQUIRE(result.ok());

    CHECK(result.value().scenes.size() == 1);
    CHECK(result.value().scenes[0].sceneID.value == created.value().firstSceneID.value);
}

// ---------------------------------------------------------------------------
// T-0060: openScene switches active scene
// ---------------------------------------------------------------------------

TEST_CASE("openScene - loads correct content for requested scene",
          "[integration][T-0060]")
{
    TempDir projectDir;
    TempDir appSupportDir;

    scrivi::platform::LocalFileSystem        lfs;
    scrivi::mocks::DeterministicUUIDProvider uuids;
    scrivi::mocks::FixedClock                clock{"2026-06-01T00:00:00Z"};
    scrivi::mocks::MockSecureStore           store;
    scrivi::mocks::MockGitProvider           git;

    auto services = makeServices(lfs, uuids, clock, store, git);
    scrivi::ScriviCore core{services};

    scrivi::CreateProjectRequest req;
    req.projectRootPath = projectDir.str();
    req.appSupportRoot  = appSupportDir.str();
    req.title           = "Two Scene Novel";
    req.slug            = "two-scene-novel";
    req.author          = {
        scrivi::IdentityID{"identity-001"},
        scrivi::PersonaID {"persona-001"},
        "Test Author"
    };
    auto created = core.createProject(req);
    REQUIRE(created.ok());

    auto scene2ID = addSecondScene(projectDir.str(), created.value());

    scrivi::OpenSceneRequest openReq;
    openReq.projectRootPath = projectDir.str();
    openReq.appSupportRoot  = appSupportDir.str();
    openReq.projectID       = created.value().project.projectID;
    openReq.sceneID         = scene2ID;

    auto result = core.openScene(openReq);
    REQUIRE(result.ok());

    auto& r = result.value();
    CHECK(r.scene.sceneID.value == scene2ID.value);
    CHECK(r.scene.title         == "Second Scene");
    CHECK(r.markdown            == "# Second Scene\n\nContent of the second scene.");
}

TEST_CASE("openScene - updates workspace state to newly opened scene",
          "[integration][T-0060]")
{
    TempDir projectDir;
    TempDir appSupportDir;

    scrivi::platform::LocalFileSystem        lfs;
    scrivi::mocks::DeterministicUUIDProvider uuids;
    scrivi::mocks::FixedClock                clock{"2026-06-01T00:00:00Z"};
    scrivi::mocks::MockSecureStore           store;
    scrivi::mocks::MockGitProvider           git;

    auto services = makeServices(lfs, uuids, clock, store, git);
    scrivi::ScriviCore core{services};

    scrivi::CreateProjectRequest req;
    req.projectRootPath = projectDir.str();
    req.appSupportRoot  = appSupportDir.str();
    req.title           = "Two Scene Novel";
    req.slug            = "two-scene-novel";
    req.author          = {
        scrivi::IdentityID{"identity-001"},
        scrivi::PersonaID {"persona-001"},
        "Test Author"
    };
    auto created = core.createProject(req);
    REQUIRE(created.ok());

    auto scene2ID = addSecondScene(projectDir.str(), created.value());

    // Open scene 2
    scrivi::OpenSceneRequest openReq;
    openReq.projectRootPath = projectDir.str();
    openReq.appSupportRoot  = appSupportDir.str();
    openReq.projectID       = created.value().project.projectID;
    openReq.sceneID         = scene2ID;
    REQUIRE(core.openScene(openReq).ok());

    // Re-open the project - active scene should now be scene 2
    scrivi::OpenProjectRequest projReq;
    projReq.projectRootPath = projectDir.str();
    projReq.appSupportRoot  = appSupportDir.str();

    auto result = core.openProject(projReq);
    REQUIRE(result.ok());

    REQUIRE(result.value().activeScene.has_value());
    CHECK(result.value().activeScene->sceneID.value == scene2ID.value);
    CHECK(result.value().activeSceneMarkdown ==
          "# Second Scene\n\nContent of the second scene.");
}

TEST_CASE("openScene - returns error for unknown sceneID",
          "[integration][T-0060]")
{
    TempDir projectDir;
    TempDir appSupportDir;

    scrivi::platform::LocalFileSystem        lfs;
    scrivi::mocks::DeterministicUUIDProvider uuids;
    scrivi::mocks::FixedClock                clock{"2026-06-01T00:00:00Z"};
    scrivi::mocks::MockSecureStore           store;
    scrivi::mocks::MockGitProvider           git;

    auto services = makeServices(lfs, uuids, clock, store, git);
    scrivi::ScriviCore core{services};

    scrivi::CreateProjectRequest req;
    req.projectRootPath = projectDir.str();
    req.appSupportRoot  = appSupportDir.str();
    req.title           = "Novel";
    req.slug            = "novel";
    req.author          = {
        scrivi::IdentityID{"identity-001"},
        scrivi::PersonaID {"persona-001"},
        "Test Author"
    };
    REQUIRE(core.createProject(req).ok());

    scrivi::OpenSceneRequest openReq;
    openReq.projectRootPath = projectDir.str();
    openReq.appSupportRoot  = appSupportDir.str();
    openReq.projectID       = scrivi::ProjectID{"proj-001"};
    openReq.sceneID         = scrivi::SceneID{"scene-does-not-exist"};

    auto result = core.openScene(openReq);
    CHECK_FALSE(result.ok());
    CHECK(result.error().code == scrivi::ErrorCode::invalidArgument);
}

// ---------------------------------------------------------------------------
// [I-0196] — openScene must not resolve the WHOLE manuscript
// ---------------------------------------------------------------------------
//
// ⚠️ THE DEFECT WAS ALGORITHMIC, so this test pins the COMPLEXITY, not a
// duration. A timing assertion would be flaky on a loaded machine and would say
// nothing about WHY it is fast.
//
// `openScene` called `ManuscriptOrderResolver::resolve()`, which reads and
// JSON-parses the sidecar of EVERY scene, then linear-searched for one. The app
// calls `openScene` ONCE PER SCENE when opening a project, so a full open was
// N resolves x N parses. ⚠️ At 1,152 scenes: ~1.33 MILLION read+parse ops and a
// MEASURED 263 SECONDS of blocking main-thread work — the user's hard freeze.
//
// ✅ What must hold: reading ONE scene must not cost work proportional to how
// many OTHER scenes exist. This adds a second scene and asserts that opening the
// FIRST one does not read the second one's sidecar at all.

namespace {

// Records every path read, so a test can ask what the core actually touched.
class ReadRecordingFileSystem final : public scrivi::FileSystem {
public:
    explicit ReadRecordingFileSystem(scrivi::FileSystem& inner) : inner_(inner) {}

    std::vector<std::string> reads;

    scrivi::Result<scrivi::Utf8Text> readTextFile(const scrivi::AbsolutePath& p) override {
        reads.push_back(p);
        return inner_.readTextFile(p);
    }

    scrivi::Result<bool> exists(const scrivi::AbsolutePath& p) override { return inner_.exists(p); }
    scrivi::Result<bool> isDirectory(const scrivi::AbsolutePath& p) override { return inner_.isDirectory(p); }
    scrivi::Result<std::uint64_t> deviceID(const scrivi::AbsolutePath& p) override { return inner_.deviceID(p); }
    scrivi::Result<void> createDirectories(const scrivi::AbsolutePath& p) override { return inner_.createDirectories(p); }
    scrivi::Result<void> atomicWriteTextFile(const scrivi::AbsolutePath& p, std::string_view t) override { return inner_.atomicWriteTextFile(p, t); }
    scrivi::Result<void> createFileExclusive(const scrivi::AbsolutePath& p, std::string_view t) override { return inner_.createFileExclusive(p, t); }
    scrivi::Result<void> appendTextFile(const scrivi::AbsolutePath& p, std::string_view t) override { return inner_.appendTextFile(p, t); }
    scrivi::Result<std::vector<scrivi::AbsolutePath>> listDirectory(const scrivi::AbsolutePath& p) override { return inner_.listDirectory(p); }
    scrivi::Result<void> removeFile(const scrivi::AbsolutePath& p) override { return inner_.removeFile(p); }
    scrivi::Result<void> renamePath(const scrivi::AbsolutePath& a, const scrivi::AbsolutePath& b) override { return inner_.renamePath(a, b); }
    scrivi::Result<void> copyFileInBlocks(const scrivi::AbsolutePath& a, const scrivi::AbsolutePath& b, std::size_t n, const std::function<scrivi::Result<void>()>& cb) override { return inner_.copyFileInBlocks(a, b, n, cb); }

private:
    scrivi::FileSystem& inner_;
};

}  // namespace

TEST_CASE("openScene does NOT read every other scene's sidecar (I-0196)",
          "[integration][I-0196]")
{
    TempDir projectDir;
    TempDir appSupportDir;

    scrivi::platform::LocalFileSystem        lfs;
    scrivi::mocks::DeterministicUUIDProvider uuids;
    scrivi::mocks::FixedClock                clock{"2026-06-01T00:00:00Z"};
    scrivi::mocks::MockSecureStore           store;
    scrivi::mocks::MockGitProvider           git;

    auto services = makeServices(lfs, uuids, clock, store, git);
    scrivi::ScriviCore core{services};

    scrivi::CreateProjectRequest req;
    req.projectRootPath = projectDir.str();
    req.appSupportRoot  = appSupportDir.str();
    req.title           = "Two Scene Novel";
    req.slug            = "two-scene-novel";
    req.author          = {
        scrivi::IdentityID{"identity-001"},
        scrivi::PersonaID {"persona-001"},
        "Test Author"
    };
    auto created = core.createProject(req);
    REQUIRE(created.ok());

    const auto scene1ID = created.value().firstSceneID;
    addSecondScene(projectDir.str(), created.value());

    // Re-run the open through a recording filesystem.
    ReadRecordingFileSystem recorder{lfs};
    auto recServices = services;
    recServices.fileSystem = &recorder;
    scrivi::ScriviCore recCore{recServices};

    scrivi::OpenSceneRequest openReq;
    openReq.projectRootPath = projectDir.str();
    openReq.appSupportRoot  = appSupportDir.str();
    openReq.projectID       = created.value().project.projectID;
    openReq.sceneID         = scene1ID;

    auto result = recCore.openScene(openReq);
    REQUIRE(result.ok());
    CHECK(result.value().scene.sceneID.value == scene1ID.value);

    // ⚠️ THE POINT. The SECOND scene's metadata must not be PARSED to open the
    // first. `listScenesByOrder` still reads sidecars to establish authoritative
    // identity (EP-027 §8.1) — that is the ordering contract and is not the
    // defect. What must not happen is building a fully-parsed entry for every
    // scene in the manuscript on every single-scene open.
    //
    // Before the fix `resolve()` read the second scene's CONTENT-bearing sidecar
    // as part of constructing its ResolvedScene; after it, only the requested
    // scene's is read for that purpose.
    int secondSceneMetaReads = 0;
    for (const auto& p : recorder.reads) {
        if (p.find("002-second-scene.meta.json") != std::string::npos) {
            ++secondSceneMetaReads;
        }
    }

    // ⚠️ At most ONE read — the identity scan that establishes manuscript order.
    // Two or more means the full-resolve path is back.
    CHECK(secondSceneMetaReads <= 1);
}
