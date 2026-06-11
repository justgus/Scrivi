#include <catch2/catch_test_macros.hpp>

#include "scrivi/ScriviCore.hpp"
#include "scrivi/Requests.hpp"

#include "mocks/DeterministicUUIDProvider.hpp"
#include "mocks/FixedClock.hpp"
#include "mocks/MockGitProvider.hpp"
#include "mocks/MockSecureStore.hpp"

#include "platform/LocalFileSystem.hpp"
#include "schemas/TimelineMetaJson.hpp"
#include "schemas/SceneMetaJson.hpp"
#include "schemas/StoryStructureJson.hpp"
#include "schemas/HistoricalEventJson.hpp"
#include "schemas/ExternalTimelineJson.hpp"

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
        path = fs::temp_directory_path() / ("scrivi-tl-test-" + std::to_string(
            std::chrono::steady_clock::now().time_since_epoch().count()));
        fs::create_directories(path);
    }
    ~TempDir() { std::error_code ec; fs::remove_all(path, ec); }
    std::string str() const { return path.string(); }
    std::string sub(const std::string& rel) const { return (path / rel).string(); }
};

static std::string readFile(const std::string& p) {
    std::ifstream f(p);
    return {std::istreambuf_iterator<char>(f), {}};
}

static scrivi::ScriviCore makeCore(
    scrivi::platform::LocalFileSystem& fs_,
    scrivi::mocks::DeterministicUUIDProvider& uuids,
    scrivi::mocks::FixedClock& clock,
    scrivi::mocks::MockSecureStore& store,
    scrivi::mocks::MockGitProvider& git)
{
    scrivi::CoreServices svc;
    svc.fileSystem   = &fs_;
    svc.uuidProvider = &uuids;
    svc.clock        = &clock;
    svc.secureStore  = &store;
    svc.gitProvider  = &git;
    return scrivi::ScriviCore{svc};
}

static scrivi::CreateProjectResult createProject(
    scrivi::ScriviCore& core,
    const std::string& root, const std::string& appSupport)
{
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

// ---------------------------------------------------------------------------
// T-0147 Test 1: timeline.meta.json created on new project
// ---------------------------------------------------------------------------

TEST_CASE("createProject writes timeline.meta.json", "[integration][T-0148]") {
    TempDir proj, app;
    scrivi::platform::LocalFileSystem        fs_;
    scrivi::mocks::DeterministicUUIDProvider uuids;
    scrivi::mocks::FixedClock                clock{"2026-06-11T00:00:00Z"};
    scrivi::mocks::MockSecureStore           store;
    scrivi::mocks::MockGitProvider           git;
    auto core = makeCore(fs_, uuids, clock, store, git);
    auto p = createProject(core, proj.str(), app.str());

    auto tlPath = proj.sub("objects/timelines/timeline.meta.json");
    REQUIRE(fs::exists(tlPath));

    auto json = readFile(tlPath);
    auto parseR = scrivi::schemas::parseTimelineMeta(json);
    REQUIRE(parseR.ok());
    CHECK(parseR.value().projectID.value == p.project.projectID.value);
    CHECK(parseR.value().epochLabel == "Story Open");
    CHECK(!parseR.value().timelineID.value.empty());
}

// ---------------------------------------------------------------------------
// T-0147 Test 2: getTimeline returns timeline meta
// ---------------------------------------------------------------------------

TEST_CASE("getTimeline reads timeline.meta.json", "[integration][T-0147]") {
    TempDir proj, app;
    scrivi::platform::LocalFileSystem        fs_;
    scrivi::mocks::DeterministicUUIDProvider uuids;
    scrivi::mocks::FixedClock                clock{"2026-06-11T00:00:00Z"};
    scrivi::mocks::MockSecureStore           store;
    scrivi::mocks::MockGitProvider           git;
    auto core = makeCore(fs_, uuids, clock, store, git);
    auto p = createProject(core, proj.str(), app.str());

    scrivi::GetTimelineRequest req;
    req.projectRootPath = proj.str();
    auto r = core.getTimeline(req);
    REQUIRE(r.ok());
    CHECK(r.value().epochLabel == "Story Open");
    CHECK(r.value().projectID  == p.project.projectID.value);
}

// ---------------------------------------------------------------------------
// T-0147 Test 3: setTimelineEpochLabel persists
// ---------------------------------------------------------------------------

TEST_CASE("setTimelineEpochLabel updates epoch label", "[integration][T-0147]") {
    TempDir proj, app;
    scrivi::platform::LocalFileSystem        fs_;
    scrivi::mocks::DeterministicUUIDProvider uuids;
    scrivi::mocks::FixedClock                clock{"2026-06-11T00:00:00Z"};
    scrivi::mocks::MockSecureStore           store;
    scrivi::mocks::MockGitProvider           git;
    auto core = makeCore(fs_, uuids, clock, store, git);
    createProject(core, proj.str(), app.str());

    scrivi::SetTimelineEpochLabelRequest req;
    req.projectRootPath = proj.str();
    req.label = "The First Sundering";
    auto r = core.setTimelineEpochLabel(req);
    REQUIRE(r.ok());
    CHECK(r.value().updated);

    scrivi::GetTimelineRequest gr;
    gr.projectRootPath = proj.str();
    auto gr2 = core.getTimeline(gr);
    REQUIRE(gr2.ok());
    CHECK(gr2.value().epochLabel == "The First Sundering");
}

// ---------------------------------------------------------------------------
// T-0147 Test 4: setSceneStoryTime / getSceneStoryTime round-trip
// ---------------------------------------------------------------------------

TEST_CASE("setSceneStoryTime and getSceneStoryTime", "[integration][T-0147]") {
    TempDir proj, app;
    scrivi::platform::LocalFileSystem        fs_;
    scrivi::mocks::DeterministicUUIDProvider uuids;
    scrivi::mocks::FixedClock                clock{"2026-06-11T00:00:00Z"};
    scrivi::mocks::MockSecureStore           store;
    scrivi::mocks::MockGitProvider           git;
    auto core = makeCore(fs_, uuids, clock, store, git);
    auto p = createProject(core, proj.str(), app.str());

    const int64_t kOffset = 3'600'000LL;  // 1 hour in ms

    scrivi::SetSceneStoryTimeRequest sreq;
    sreq.projectRootPath = proj.str();
    sreq.sceneID         = p.firstSceneID;
    sreq.offsetMs        = kOffset;
    sreq.source          = "manual";
    auto sr = core.setSceneStoryTime(sreq);
    REQUIRE(sr.ok());
    CHECK(sr.value().updated);

    scrivi::GetSceneStoryTimeRequest greq;
    greq.projectRootPath = proj.str();
    greq.sceneID         = p.firstSceneID;
    auto gr = core.getSceneStoryTime(greq);
    REQUIRE(gr.ok());
    CHECK(gr.value().offsetMs     == kOffset);
    CHECK(gr.value().offsetSource == "manual");

    // Verify it persisted in scene.meta.json on disk
    auto metaJson = readFile(proj.sub(p.firstSceneMetadataPath));
    auto parseR   = scrivi::schemas::parseSceneMeta(metaJson);
    REQUIRE(parseR.ok());
    CHECK(parseR.value().storyTime.offsetMs     == kOffset);
    CHECK(parseR.value().storyTime.offsetSource == "manual");
}

// ---------------------------------------------------------------------------
// T-0147 Test 5: clearSceneStoryTime resets to default
// ---------------------------------------------------------------------------

TEST_CASE("clearSceneStoryTime resets to default", "[integration][T-0147]") {
    TempDir proj, app;
    scrivi::platform::LocalFileSystem        fs_;
    scrivi::mocks::DeterministicUUIDProvider uuids;
    scrivi::mocks::FixedClock                clock{"2026-06-11T00:00:00Z"};
    scrivi::mocks::MockSecureStore           store;
    scrivi::mocks::MockGitProvider           git;
    auto core = makeCore(fs_, uuids, clock, store, git);
    auto p = createProject(core, proj.str(), app.str());

    scrivi::SetSceneStoryTimeRequest sreq;
    sreq.projectRootPath = proj.str();
    sreq.sceneID = p.firstSceneID;
    sreq.offsetMs = 86'400'000LL;
    sreq.source   = "manual";
    REQUIRE(core.setSceneStoryTime(sreq).ok());

    scrivi::ClearSceneStoryTimeRequest creq;
    creq.projectRootPath = proj.str();
    creq.sceneID = p.firstSceneID;
    auto cr = core.clearSceneStoryTime(creq);
    REQUIRE(cr.ok());
    CHECK(cr.value().cleared);

    scrivi::GetSceneStoryTimeRequest greq;
    greq.projectRootPath = proj.str();
    greq.sceneID = p.firstSceneID;
    auto gr = core.getSceneStoryTime(greq);
    REQUIRE(gr.ok());
    CHECK(gr.value().offsetMs     == 0);
    CHECK(gr.value().offsetSource == "default");
}

// ---------------------------------------------------------------------------
// T-0147 Test 6: assignSceneToBand / unassignSceneFromBand
// ---------------------------------------------------------------------------

TEST_CASE("assignSceneToBand and unassignSceneFromBand", "[integration][T-0147]") {
    TempDir proj, app;
    scrivi::platform::LocalFileSystem        fs_;
    scrivi::mocks::DeterministicUUIDProvider uuids;
    scrivi::mocks::FixedClock                clock{"2026-06-11T00:00:00Z"};
    scrivi::mocks::MockSecureStore           store;
    scrivi::mocks::MockGitProvider           git;
    auto core = makeCore(fs_, uuids, clock, store, git);
    auto p = createProject(core, proj.str(), app.str());

    scrivi::AssignSceneToBandRequest areq;
    areq.projectRootPath = proj.str();
    areq.sceneID         = p.firstSceneID;
    areq.bandID          = "act-ii";
    auto ar = core.assignSceneToBand(areq);
    REQUIRE(ar.ok());
    CHECK(ar.value().assigned);

    scrivi::GetSceneStoryTimeRequest greq;
    greq.projectRootPath = proj.str();
    greq.sceneID = p.firstSceneID;
    auto gr = core.getSceneStoryTime(greq);
    REQUIRE(gr.ok());
    CHECK(gr.value().bandID == "act-ii");
    CHECK(!gr.value().bandAssignedAt.empty());

    scrivi::UnassignSceneFromBandRequest ureq;
    ureq.projectRootPath = proj.str();
    ureq.sceneID = p.firstSceneID;
    auto ur = core.unassignSceneFromBand(ureq);
    REQUIRE(ur.ok());
    CHECK(ur.value().unassigned);

    auto gr2 = core.getSceneStoryTime(greq);
    REQUIRE(gr2.ok());
    CHECK(gr2.value().bandID.empty());
}

// ---------------------------------------------------------------------------
// T-0147 Test 7: Story structure round-trip
// ---------------------------------------------------------------------------

TEST_CASE("setStoryStructure and getStoryStructure", "[integration][T-0147]") {
    TempDir proj, app;
    scrivi::platform::LocalFileSystem        fs_;
    scrivi::mocks::DeterministicUUIDProvider uuids;
    scrivi::mocks::FixedClock                clock{"2026-06-11T00:00:00Z"};
    scrivi::mocks::MockSecureStore           store;
    scrivi::mocks::MockGitProvider           git;
    auto core = makeCore(fs_, uuids, clock, store, git);
    createProject(core, proj.str(), app.str());

    // Initially no structure
    scrivi::GetStoryStructureRequest nreq;
    nreq.projectRootPath = proj.str();
    auto nr = core.getStoryStructure(nreq);
    REQUIRE(nr.ok());
    CHECK(!nr.value().hasStructure);

    // Apply three-act
    scrivi::SetStoryStructureRequest sreq;
    sreq.projectRootPath = proj.str();
    sreq.structureID     = "three-act";
    sreq.bandLayoutJSON  = "";  // auto-divide
    auto sr = core.setStoryStructure(sreq);
    REQUIRE(sr.ok());
    CHECK(sr.value().set);

    // Verify file on disk
    REQUIRE(fs::exists(proj.sub("objects/story-structures/story-structure.json")));
    auto json    = readFile(proj.sub("objects/story-structures/story-structure.json"));
    auto parseR  = scrivi::schemas::parseStoryStructure(json);
    REQUIRE(parseR.ok());
    CHECK(parseR.value().activeStructureID == "three-act");
    CHECK(parseR.value().bandLayout.size() == 3);

    // getStoryStructure
    auto gr = core.getStoryStructure(nreq);
    REQUIRE(gr.ok());
    CHECK(gr.value().hasStructure);
    CHECK(gr.value().structureID == "three-act");

    // removeStoryStructure
    scrivi::RemoveStoryStructureRequest rreq;
    rreq.projectRootPath = proj.str();
    auto rr = core.removeStoryStructure(rreq);
    REQUIRE(rr.ok());
    CHECK(rr.value().removed);
    CHECK(!fs::exists(proj.sub("objects/story-structures/story-structure.json")));
}

// ---------------------------------------------------------------------------
// T-0147 Test 8: Historical event CRUD
// ---------------------------------------------------------------------------

TEST_CASE("createHistoricalEvent / updateHistoricalEvent / deleteHistoricalEvent", "[integration][T-0147]") {
    TempDir proj, app;
    scrivi::platform::LocalFileSystem        fs_;
    scrivi::mocks::DeterministicUUIDProvider uuids;
    scrivi::mocks::FixedClock                clock{"2026-06-11T00:00:00Z"};
    scrivi::mocks::MockSecureStore           store;
    scrivi::mocks::MockGitProvider           git;
    auto core = makeCore(fs_, uuids, clock, store, git);
    createProject(core, proj.str(), app.str());

    scrivi::CreateHistoricalEventRequest creq;
    creq.projectRootPath = proj.str();
    creq.title           = "Battle of the Iron Gate";
    creq.offsetMs        = -31'536'000'000LL;
    creq.description     = "The decisive siege.";
    creq.tags            = {"war", "turning-point"};
    creq.author          = {scrivi::IdentityID{"identity-001"}, scrivi::PersonaID{"persona-001"}, "Author"};
    auto cr = core.createHistoricalEvent(creq);
    REQUIRE(cr.ok());
    const auto eventID = cr.value().eventID;
    CHECK(!eventID.empty());

    // list
    scrivi::ListHistoricalEventsRequest lreq;
    lreq.projectRootPath = proj.str();
    auto lr = core.listHistoricalEvents(lreq);
    REQUIRE(lr.ok());
    CHECK(lr.value().count == 1);

    // update
    scrivi::UpdateHistoricalEventRequest ureq;
    ureq.projectRootPath = proj.str();
    ureq.eventID         = eventID;
    ureq.title           = "Battle of the Iron Gate (Revised)";
    ureq.offsetMs        = -31'536'000'000LL;
    ureq.description     = "Updated description.";
    auto ur = core.updateHistoricalEvent(ureq);
    REQUIRE(ur.ok());
    CHECK(ur.value().updated);

    // delete
    scrivi::DeleteHistoricalEventRequest dreq;
    dreq.projectRootPath = proj.str();
    dreq.eventID         = eventID;
    auto dr = core.deleteHistoricalEvent(dreq);
    REQUIRE(dr.ok());
    CHECK(dr.value().deleted);

    auto lr2 = core.listHistoricalEvents(lreq);
    REQUIRE(lr2.ok());
    CHECK(lr2.value().count == 0);
}

// ---------------------------------------------------------------------------
// T-0147 Test 9: External timeline import / list / remove
// ---------------------------------------------------------------------------

TEST_CASE("importExternalTimeline / listImportedTimelines / removeImportedTimeline", "[integration][T-0147]") {
    TempDir proj, app;
    scrivi::platform::LocalFileSystem        fs_;
    scrivi::mocks::DeterministicUUIDProvider uuids;
    scrivi::mocks::FixedClock                clock{"2026-06-11T00:00:00Z"};
    scrivi::mocks::MockSecureStore           store;
    scrivi::mocks::MockGitProvider           git;
    auto core = makeCore(fs_, uuids, clock, store, git);
    createProject(core, proj.str(), app.str());

    // Build a minimal external timeline JSON
    scrivi::schemas::ExternalTimelineData tl;
    tl.timelineID         = "ext-timeline-001";
    tl.sourceProjectTitle = "The Iron Chronicles";
    tl.sourceProjectID    = "proj-ext-001";
    tl.exportedAt         = "2026-06-11T00:00:00Z";
    tl.epochLabel         = "The First Age";
    scrivi::schemas::ExternalTimelineEvent ev;
    ev.eventID  = "event-001";
    ev.title    = "Founding of the Empire";
    ev.offsetMs = 0;
    ev.kind     = "historical";
    tl.events.push_back(ev);
    auto tlJSON = scrivi::schemas::serializeExternalTimeline(tl);

    scrivi::ImportExternalTimelineRequest ireq;
    ireq.projectRootPath   = proj.str();
    ireq.timelineJSON      = tlJSON;
    ireq.epochOffsetMs     = -94'608'000'000LL;
    ireq.assignedGreyShade = "#8A8A8A";
    auto ir = core.importExternalTimeline(ireq);
    REQUIRE(ir.ok());
    CHECK(ir.value().imported);
    CHECK(ir.value().timelineID == "ext-timeline-001");

    // list
    scrivi::ListImportedTimelinesRequest lreq;
    lreq.projectRootPath = proj.str();
    auto lr = core.listImportedTimelines(lreq);
    REQUIRE(lr.ok());
    CHECK(lr.value().count == 1);

    // remove
    scrivi::RemoveImportedTimelineRequest rreq;
    rreq.projectRootPath = proj.str();
    rreq.timelineID      = "ext-timeline-001";
    auto rr = core.removeImportedTimeline(rreq);
    REQUIRE(rr.ok());
    CHECK(rr.value().removed);

    auto lr2 = core.listImportedTimelines(lreq);
    REQUIRE(lr2.ok());
    CHECK(lr2.value().count == 0);
}

// ---------------------------------------------------------------------------
// T-0147 Test 10: exportProjectTimeline
// ---------------------------------------------------------------------------

TEST_CASE("exportProjectTimeline produces valid scrivi.externalTimeline.v1", "[integration][T-0147]") {
    TempDir proj, app;
    scrivi::platform::LocalFileSystem        fs_;
    scrivi::mocks::DeterministicUUIDProvider uuids;
    scrivi::mocks::FixedClock                clock{"2026-06-11T00:00:00Z"};
    scrivi::mocks::MockSecureStore           store;
    scrivi::mocks::MockGitProvider           git;
    auto core = makeCore(fs_, uuids, clock, store, git);
    auto p = createProject(core, proj.str(), app.str());

    // Set a story-time offset on the scene
    scrivi::SetSceneStoryTimeRequest sreq;
    sreq.projectRootPath = proj.str();
    sreq.sceneID         = p.firstSceneID;
    sreq.offsetMs        = 7'200'000LL;
    sreq.source          = "manual";
    REQUIRE(core.setSceneStoryTime(sreq).ok());

    scrivi::ExportProjectTimelineRequest ereq;
    ereq.projectRootPath = proj.str();
    auto er = core.exportProjectTimeline(ereq);
    REQUIRE(er.ok());

    auto parseR = scrivi::schemas::parseExternalTimeline(er.value().timelineJSON);
    REQUIRE(parseR.ok());
    CHECK(parseR.value().epochLabel == "Story Open");
    REQUIRE(parseR.value().events.size() == 1);
    CHECK(parseR.value().events[0].kind     == "scene");
    CHECK(parseR.value().events[0].offsetMs == 7'200'000LL);
}
