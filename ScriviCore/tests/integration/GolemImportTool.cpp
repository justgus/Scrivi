// ⚠️ A ONE-OFF IMPORT TOOL, not a test of Scrivi.
//
// Builds `the-lone-golem` from THE_LONE_GOLEM.md by driving the SAME public API
// the app drives — createProject / createChapter / createScene / saveScene /
// renameScene / renameChapter — so every ID, order key, sidecar and index entry
// is produced by ScriviCore rather than hand-written JSON.
//
// ⚠️ Tagged `[.golem-import]` — hidden by default, so `ctest` never runs it.
// It is inert unless BOTH env vars are set:
//     GOLEM_SOURCE_JSON  parsed chapters/scenes (see parse_golem.py)
//     GOLEM_DEST         destination .scrivi path (must NOT already exist)

#include <catch2/catch_test_macros.hpp>

#include "scrivi/Requests.hpp"
#include "scrivi/ScriviCore.hpp"
#include "util/Json.hpp"

#include "mocks/MockGitProvider.hpp"
#include "mocks/MockSecureStore.hpp"
#include "platform/LocalFileSystem.hpp"
#include "platform/SystemUUIDProvider.hpp"

#include <cstdlib>
#include <filesystem>
#include <fstream>
#include <iostream>
#include <iterator>
#include <string>

namespace fs = std::filesystem;
using namespace scrivi;

namespace {

// Real wall-clock, so imported scenes carry honest timestamps rather than a
// fixture's frozen one.
class RealClock final : public Clock {
public:
    ISO8601Timestamp nowUTC() override {
        const auto t  = std::chrono::system_clock::now();
        const auto tt = std::chrono::system_clock::to_time_t(t);
        std::tm tm{};
        gmtime_r(&tt, &tm);
        char buf[32];
        std::strftime(buf, sizeof buf, "%Y-%m-%dT%H:%M:%SZ", &tm);
        return ISO8601Timestamp{std::string(buf)};
    }
};

} // namespace

TEST_CASE("import THE LONE GOLEM", "[.golem-import]") {
    const char* srcEnv  = std::getenv("GOLEM_SOURCE_JSON");
    const char* destEnv = std::getenv("GOLEM_DEST");
    if (srcEnv == nullptr || destEnv == nullptr) {
        SUCCEED("GOLEM_SOURCE_JSON / GOLEM_DEST unset — import skipped");
        return;
    }

    const fs::path dest{destEnv};
    // ⚠️ Never write into an existing package: this tool creates, it does not merge.
    REQUIRE_FALSE(fs::exists(dest));

    std::ifstream in(srcEnv);
    REQUIRE(in.good());
    const std::string raw{std::istreambuf_iterator<char>(in),
                          std::istreambuf_iterator<char>()};
    auto docR = util::parseJson(raw);
    REQUIRE(docR.ok());
    const auto& doc = docR.value();

    const auto chapterCount = doc.arraySize("chapters");
    REQUIRE(chapterCount > 0);

    platform::LocalFileSystem    fileSystem;
    platform::SystemUUIDProvider uuidProvider;
    RealClock                    clock;
    mocks::MockGitProvider       gitProvider;
    mocks::MockSecureStore       secureStore;

    CoreServices svc;
    svc.fileSystem   = &fileSystem;
    svc.uuidProvider = &uuidProvider;
    svc.clock        = &clock;
    svc.gitProvider  = &gitProvider;
    svc.secureStore  = &secureStore;
    svc.logger       = nullptr;

    ScriviCore core{svc};

    const fs::path appSupport = dest.parent_path() / ".golem-import-appsupport";
    fs::create_directories(appSupport);
    fs::create_directories(dest);

    auto identR = core.ensureLocalIdentity(
        EnsureIdentityRequest{.requestedDisplayName = "justgus",
                              .appSupportRoot       = appSupport.string()});
    REQUIRE(identR.ok());
    const AuthorshipRef author{IdentityID{identR.value().identityID},
                               PersonaID{identR.value().defaultPersonaID},
                               identR.value().displayName};

    // --- Chapter 1 + its first scene come free with the project ---------------
    auto ch0 = doc.arrayItem("chapters", 0);
    auto sc0 = ch0.arrayItem("scenes", 0);

    CreateProjectRequest cp;
    cp.projectRootPath     = dest.string();
    cp.appSupportRoot      = appSupport.string();
    cp.title               = "The Lone Golem";
    cp.slug                = "the-lone-golem";
    cp.author              = author;
    cp.initialChapterTitle = ch0.getString("title");
    cp.initialSceneTitle   = sc0.getString("title");
    auto projR = core.createProject(cp);
    REQUIRE(projR.ok());

    const auto projectID = projR.value().project.projectID;

    auto writeScene = [&](const SceneID& sceneID,
                          const std::string& metaPath,
                          const std::string& contentPath,
                          const std::string& body) {
        SaveSceneRequest sv;
        sv.projectID         = projectID;
        sv.projectRootPath   = dest.string();
        sv.appSupportRoot    = appSupport.string();
        sv.sceneID           = sceneID;
        sv.sceneMetadataPath = metaPath;
        sv.sceneContentPath  = contentPath;
        sv.markdown          = body;
        sv.author            = author;
        auto r = core.saveScene(sv);
        INFO("saveScene " << metaPath << ": "
             << (r.ok() ? "ok" : r.error().message));
        REQUIRE(r.ok());
    };

    writeScene(projR.value().firstSceneID,
               projR.value().firstSceneMetadataPath,
               projR.value().firstSceneContentPath,
               sc0.getString("body"));

    ChapterID prevChapter = projR.value().firstChapterID;
    SceneID   prevScene   = projR.value().firstSceneID;
    int       scenesDone  = 1;

    // Remaining scenes of chapter 1, then every later chapter in order.
    for (std::size_t ci = 0; ci < chapterCount; ++ci) {
        auto chapter = doc.arrayItem("chapters", ci);

        if (ci > 0) {
            CreateChapterRequest cc;
            cc.projectRootPath = dest.string();
            cc.appSupportRoot  = appSupport.string();
            cc.projectID       = projectID;
            cc.author          = author;
            cc.afterChapterID  = prevChapter;   // append in reading order
            auto r = core.createChapter(cc);
            REQUIRE(r.ok());

            RenameChapterRequest rc;
            rc.projectRootPath = dest.string();
            rc.metadataPath    = r.value().chapterMetadataPath;
            rc.newTitle        = chapter.getString("title");
            REQUIRE(core.renameChapter(rc).ok());

            prevChapter = r.value().chapterID;

            // A new chapter is born with one scene — that is scene 1.
            auto s0 = chapter.arrayItem("scenes", 0);
            RenameSceneRequest rs;
            rs.projectRootPath = dest.string();
            rs.metadataPath    = r.value().firstSceneMetadataPath;
            rs.newTitle        = s0.getString("title");
            REQUIRE(core.renameScene(rs).ok());

            writeScene(r.value().firstSceneID,
                       r.value().firstSceneMetadataPath,
                       r.value().firstSceneContentPath,
                       s0.getString("body"));
            prevScene = r.value().firstSceneID;
            ++scenesDone;
        }

        for (std::size_t si = 1; si < chapter.arraySize("scenes"); ++si) {
            auto scene = chapter.arrayItem("scenes", si);

            CreateSceneRequest cs;
            cs.projectRootPath = dest.string();
            cs.appSupportRoot  = appSupport.string();
            cs.projectID       = projectID;
            cs.chapterID       = prevChapter;
            cs.afterSceneID    = prevScene;
            cs.author          = author;
            auto r = core.createScene(cs);
            INFO("createScene ch" << ci + 1 << " sc" << si + 1);
            REQUIRE(r.ok());

            RenameSceneRequest rs;
            rs.projectRootPath = dest.string();
            rs.metadataPath    = r.value().metadataPath;
            rs.newTitle        = scene.getString("title");
            REQUIRE(core.renameScene(rs).ok());

            writeScene(r.value().sceneID, r.value().metadataPath,
                       r.value().contentPath, scene.getString("body"));
            prevScene = r.value().sceneID;
            ++scenesDone;
        }
    }

    // ⚠️ Prove the package OPENS through the same path the app uses — an import
    // that produces files the core then refuses is worse than no import.
    OpenProjectRequest op;
    op.projectRootPath = dest.string();
    op.appSupportRoot  = appSupport.string();
    auto opened = core.openProject(op);
    INFO("openProject: " << (opened.ok() ? "ok" : opened.error().message));
    REQUIRE(opened.ok());
    REQUIRE(opened.value().mode != OpenMode::repairRequired);

    std::cout << "\nIMPORTED " << chapterCount << " chapters, "
              << scenesDone << " scenes -> " << dest.string() << "\n"
              << "openProject: OK, no repair required\n";

    fs::remove_all(appSupport);
}
