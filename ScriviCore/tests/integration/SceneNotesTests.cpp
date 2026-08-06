#include <catch2/catch_test_macros.hpp>

#include "scrivi/ScriviCore.hpp"
#include "scrivi/Requests.hpp"

#include "mocks/DeterministicUUIDProvider.hpp"
#include "mocks/FixedClock.hpp"
#include "mocks/MockGitProvider.hpp"
#include "mocks/MockSecureStore.hpp"

#include "platform/LocalFileSystem.hpp"
#include "schemas/SceneMetaJson.hpp"

#include <atomic>
#include <cstdint>
#include <filesystem>
#include <fstream>
#include <sstream>
#include <string>

// Scene writing-tool card content — tags / outline / todo (EP-030 SP-091, T-0393).
// Exercises the facade endpoints that back the inspector's Writing-tab cards, plus the
// additive-compatibility guarantee that a pre-SP-091 sidecar still opens.

namespace fs = std::filesystem;

namespace {

struct TempDir {
    fs::path path;
    TempDir() {
        static std::atomic<int> counter{0};
        path = fs::temp_directory_path() /
               ("scrivi-scene-notes-" + std::to_string(counter.fetch_add(1)) + "-" +
                std::to_string(reinterpret_cast<std::uintptr_t>(this)));
        fs::create_directories(path);
    }
    ~TempDir() { std::error_code ec; fs::remove_all(path, ec); }
    std::string str() const { return path.string(); }
    fs::path sub(const std::string& rel) const { return path / rel; }
};

scrivi::ScriviCore makeCore(scrivi::platform::LocalFileSystem& fs_,
                            scrivi::mocks::DeterministicUUIDProvider& uuids,
                            scrivi::mocks::FixedClock& clock,
                            scrivi::mocks::MockSecureStore& store,
                            scrivi::mocks::MockGitProvider& git) {
    scrivi::CoreServices svc;
    svc.fileSystem   = &fs_;
    svc.uuidProvider = &uuids;
    svc.clock        = &clock;
    svc.secureStore  = &store;
    svc.gitProvider  = &git;
    return scrivi::ScriviCore{svc};
}

scrivi::CreateProjectResult createProject(scrivi::ScriviCore& core,
                                          const std::string& root,
                                          const std::string& appSupport) {
    scrivi::CreateProjectRequest req;
    req.projectRootPath = root;
    req.appSupportRoot  = appSupport;
    req.title  = "The Shattered Veil";
    req.slug   = "the-shattered-veil";
    req.author = {scrivi::IdentityID{"identity-001"}, scrivi::PersonaID{"persona-001"}, "Author"};
    auto r = core.createProject(req);
    REQUIRE(r.ok());
    return r.value();
}

} // namespace

#define SETUP_PROJECT()                                                   \
    TempDir proj, app;                                                    \
    scrivi::platform::LocalFileSystem        fs_;                         \
    scrivi::mocks::DeterministicUUIDProvider uuids;                       \
    scrivi::mocks::FixedClock                clock{"2026-08-05T00:00:00Z"}; \
    scrivi::mocks::MockSecureStore           store;                       \
    scrivi::mocks::MockGitProvider           git;                         \
    auto core = makeCore(fs_, uuids, clock, store, git);                  \
    auto p = createProject(core, proj.str(), app.str())

TEST_CASE("setSceneTags then getSceneNotes round-trips", "[integration][T-0393]") {
    SETUP_PROJECT();

    scrivi::SetSceneTagsRequest sreq;
    sreq.projectRootPath = proj.str();
    sreq.sceneID         = p.firstSceneID;
    sreq.tags            = {"battle", "ada-pov"};
    auto sr = core.setSceneTags(sreq);
    REQUIRE(sr.ok());
    CHECK(sr.value().updated);

    scrivi::GetSceneNotesRequest greq;
    greq.projectRootPath = proj.str();
    greq.sceneID         = p.firstSceneID;
    auto gr = core.getSceneNotes(greq);
    REQUIRE(gr.ok());
    REQUIRE(gr.value().tags.size() == 2);
    CHECK(gr.value().tags[0] == "battle");
    CHECK(gr.value().tags[1] == "ada-pov");   // writer's order preserved
}

TEST_CASE("setSceneTags de-duplicates while preserving order", "[integration][T-0393]") {
    SETUP_PROJECT();

    scrivi::SetSceneTagsRequest sreq;
    sreq.projectRootPath = proj.str();
    sreq.sceneID         = p.firstSceneID;
    sreq.tags            = {"battle", "ada-pov", "battle", "", "ada-pov"};
    REQUIRE(core.setSceneTags(sreq).ok());

    scrivi::GetSceneNotesRequest greq;
    greq.projectRootPath = proj.str();
    greq.sceneID         = p.firstSceneID;
    auto gr = core.getSceneNotes(greq);
    REQUIRE(gr.ok());
    REQUIRE(gr.value().tags.size() == 2);     // duplicates and the empty string dropped
    CHECK(gr.value().tags[0] == "battle");
    CHECK(gr.value().tags[1] == "ada-pov");
}

TEST_CASE("setSceneOutline round-trips and can be cleared", "[integration][T-0393]") {
    SETUP_PROJECT();

    scrivi::SetSceneOutlineRequest sreq;
    sreq.projectRootPath = proj.str();
    sreq.sceneID         = p.firstSceneID;
    sreq.outline         = "Ada confronts the Warden at the gate.";
    REQUIRE(core.setSceneOutline(sreq).ok());

    scrivi::GetSceneNotesRequest greq;
    greq.projectRootPath = proj.str();
    greq.sceneID         = p.firstSceneID;
    CHECK(core.getSceneNotes(greq).value().outline == "Ada confronts the Warden at the gate.");

    sreq.outline = "";
    REQUIRE(core.setSceneOutline(sreq).ok());
    CHECK(core.getSceneNotes(greq).value().outline.empty());
}

TEST_CASE("setSceneTodo round-trips done flags and drops blank rows",
          "[integration][T-0393]") {
    SETUP_PROJECT();

    scrivi::SetSceneTodoRequest sreq;
    sreq.projectRootPath = proj.str();
    sreq.sceneID         = p.firstSceneID;
    sreq.todo            = { {"check the timeline", false},
                             {"", false},                    // blank — never persisted
                             {"name the Warden", true} };
    REQUIRE(core.setSceneTodo(sreq).ok());

    scrivi::GetSceneNotesRequest greq;
    greq.projectRootPath = proj.str();
    greq.sceneID         = p.firstSceneID;
    auto gr = core.getSceneNotes(greq);
    REQUIRE(gr.ok());
    REQUIRE(gr.value().todo.size() == 2);
    CHECK(gr.value().todo[0].text == "check the timeline");
    CHECK(gr.value().todo[0].done == false);
    CHECK(gr.value().todo[1].text == "name the Warden");
    CHECK(gr.value().todo[1].done == true);
}

// The three cards are independent fields on one file: writing one must not clobber the
// others, since each card saves on its own schedule.
TEST_CASE("scene notes fields are independent of one another", "[integration][T-0393]") {
    SETUP_PROJECT();

    scrivi::SetSceneTagsRequest tags;
    tags.projectRootPath = proj.str();
    tags.sceneID         = p.firstSceneID;
    tags.tags            = {"battle"};
    REQUIRE(core.setSceneTags(tags).ok());

    scrivi::SetSceneOutlineRequest outline;
    outline.projectRootPath = proj.str();
    outline.sceneID         = p.firstSceneID;
    outline.outline         = "The gate falls.";
    REQUIRE(core.setSceneOutline(outline).ok());

    scrivi::SetSceneTodoRequest todo;
    todo.projectRootPath = proj.str();
    todo.sceneID         = p.firstSceneID;
    todo.todo            = { {"name the Warden", false} };
    REQUIRE(core.setSceneTodo(todo).ok());

    scrivi::GetSceneNotesRequest greq;
    greq.projectRootPath = proj.str();
    greq.sceneID         = p.firstSceneID;
    auto gr = core.getSceneNotes(greq);
    REQUIRE(gr.ok());
    CHECK(gr.value().tags.size() == 1);       // survived the outline + todo writes
    CHECK(gr.value().outline == "The gate falls.");
    CHECK(gr.value().todo.size() == 1);
}

// Writing card content must not disturb the scene's identity, authorship, or story-time
// data — the setters do a full parse/serialize round-trip of the sidecar.
TEST_CASE("setting scene notes preserves existing sidecar fields",
          "[integration][T-0393]") {
    SETUP_PROJECT();

    scrivi::SetSceneStoryTimeRequest st;
    st.projectRootPath = proj.str();
    st.sceneID         = p.firstSceneID;
    st.offsetMs        = 90'000;
    st.source          = "manual";
    st.durationMs      = 7'200'000;
    REQUIRE(core.setSceneStoryTime(st).ok());

    scrivi::SetSceneTagsRequest tags;
    tags.projectRootPath = proj.str();
    tags.sceneID         = p.firstSceneID;
    tags.tags            = {"battle"};
    REQUIRE(core.setSceneTags(tags).ok());

    scrivi::GetSceneStoryTimeRequest gst;
    gst.projectRootPath = proj.str();
    gst.sceneID         = p.firstSceneID;
    auto got = core.getSceneStoryTime(gst);
    REQUIRE(got.ok());
    CHECK(got.value().offsetMs     == 90'000);        // untouched by the tags write
    CHECK(got.value().offsetSource == "manual");
    CHECK(got.value().durationMs   == 7'200'000);
}

TEST_CASE("getSceneNotes on an untouched scene returns empty fields",
          "[integration][T-0393]") {
    SETUP_PROJECT();

    scrivi::GetSceneNotesRequest greq;
    greq.projectRootPath = proj.str();
    greq.sceneID         = p.firstSceneID;
    auto gr = core.getSceneNotes(greq);
    REQUIRE(gr.ok());
    CHECK(gr.value().tags.empty());
    CHECK(gr.value().outline.empty());
    CHECK(gr.value().todo.empty());
}

TEST_CASE("scene notes endpoints reject an unknown sceneID", "[integration][T-0393]") {
    SETUP_PROJECT();

    scrivi::GetSceneNotesRequest greq;
    greq.projectRootPath = proj.str();
    greq.sceneID.value   = "scene-does-not-exist";
    CHECK_FALSE(core.getSceneNotes(greq).ok());

    scrivi::SetSceneTagsRequest sreq;
    sreq.projectRootPath = proj.str();
    sreq.sceneID.value   = "scene-does-not-exist";
    sreq.tags            = {"battle"};
    CHECK_FALSE(core.setSceneTags(sreq).ok());
}
