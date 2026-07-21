#include "scrivi/ScriviCore.hpp"

#include "git/SnapshotService.hpp"
#include "identity/IdentityService.hpp"
#include "manuscript/ManuscriptOrderResolver.hpp"
#include "manuscript/ChapterCreator.hpp"
#include "manuscript/ChapterDeleter.hpp"
#include "manuscript/ChapterRenamer.hpp"
#include "manuscript/ChapterReorderer.hpp"
#include "manuscript/SceneCreator.hpp"
#include "manuscript/SceneDeleter.hpp"
#include "manuscript/SceneRenamer.hpp"
#include "manuscript/SceneReorderer.hpp"
#include "manuscript/SceneMerger.hpp"
#include "manuscript/ChapterMerger.hpp"
#include "manuscript/SceneReader.hpp"
#include "manuscript/SceneWriter.hpp"
#include "project_package/ProjectCreator.hpp"
#include "project_package/ProjectOpener.hpp"
#include "repair/ExternalChangeScanner.hpp"
#include "repair/RepairDispatcher.hpp"
#include "objects/ObjectStore.hpp"
#include "assets/AssetStore.hpp"
#include "comments/CommentStore.hpp"
#include "inbox/InboxStore.hpp"
#include "platform/AppSupportLayout.hpp"
#include "workspace/WorkspaceStateService.hpp"
#include "schemas/ExternalTimelineJson.hpp"
#include "schemas/HistoricalEventJson.hpp"
#include "schemas/SceneMetaJson.hpp"
#include "schemas/StoryStructureJson.hpp"
#include "schemas/TimelineMetaJson.hpp"
#include "schemas/ObjectJson.hpp"
#include "schemas/ProjectJson.hpp"
#include "domain/Slug.hpp"
#include "util/Json.hpp"
#include "util/MarkdownStrip.hpp"
#include "util/PathUtils.hpp"

namespace scrivi {

ScriviCore::ScriviCore(CoreServices services)
    : services_(services) {}

Result<EnsureIdentityResult> ScriviCore::ensureLocalIdentity(
    const EnsureIdentityRequest& request) {
    if (auto r = util::bootstrapAppSupport(request.appSupportRoot, *services_.fileSystem); !r.ok()) {
        return Result<EnsureIdentityResult>::failure(r.error());
    }
    identity::IdentityService svc{services_};
    return svc.ensureLocalIdentity(request);
}

Result<CreateProjectResult> ScriviCore::createProject(
    const CreateProjectRequest& request) {
    if (auto r = util::bootstrapAppSupport(request.appSupportRoot, *services_.fileSystem); !r.ok()) {
        return Result<CreateProjectResult>::failure(r.error());
    }
    project_package::ProjectCreator creator{services_};
    return creator.create(request);
}

Result<OpenProjectResult> ScriviCore::openProject(
    const OpenProjectRequest& request) {
    if (auto r = util::bootstrapAppSupport(request.appSupportRoot, *services_.fileSystem); !r.ok()) {
        return Result<OpenProjectResult>::failure(r.error());
    }
    project_package::ProjectOpener opener{services_};
    return opener.open(request);
}

Result<OpenSceneResult> ScriviCore::openScene(
    const OpenSceneRequest& request) {
    if (auto r = util::bootstrapAppSupport(request.appSupportRoot, *services_.fileSystem); !r.ok()) {
        return Result<OpenSceneResult>::failure(r.error());
    }

    // 1. Resolve manuscript order to find the requested scene
    manuscript::ManuscriptOrderResolver resolver{services_};
    auto scenesR = resolver.resolve(request.projectRootPath);
    if (!scenesR.ok()) { return Result<OpenSceneResult>::failure(scenesR.error()); }

    const manuscript::ResolvedScene* found = nullptr;
    for (auto& s : scenesR.value()) {
        if (s.sceneID.value == request.sceneID.value) {
            found = &s;
            break;
        }
    }
    if (found == nullptr) {
        return Result<OpenSceneResult>::failure(
            Error{.code = ErrorCode::invalidArgument,
                  .message = "Scene not found: " + request.sceneID.value});
    }

    // 2. Read scene content
    manuscript::SceneReader reader{services_};
    auto mdR = reader.readContent(request.projectRootPath, found->contentPath);
    if (!mdR.ok()) { return Result<OpenSceneResult>::failure(mdR.error()); }

    // 3. Load workspace state to restore cursor/scroll, then update active scene
    workspace::WorkspaceStateService wsService{services_};
    auto wsR = wsService.load(request.appSupportRoot, request.projectID);
    if (!wsR.ok()) { return Result<OpenSceneResult>::failure(wsR.error()); }

    TextSelection  restoredSelection;
    ScrollPosition restoredScroll;

    WorkspaceState ws;
    const auto& maybeWs = wsR.value();
    if (maybeWs.has_value()) {
        ws = *maybeWs;
        // Restore cursor/scroll only if the last surface matches this scene
        if (ws.lastWritingSurface.has_value() &&
            ws.lastWritingSurface->sceneID.value == request.sceneID.value) {
            restoredSelection = ws.lastWritingSurface->selection;
            restoredScroll    = ws.lastWritingSurface->scroll;
        }
    }

    // Update last writing surface to the newly opened scene
    LastWritingSurface lws;
    lws.sceneID     = found->sceneID;
    lws.contentPath = found->contentPath;
    lws.selection   = restoredSelection;
    lws.scroll      = restoredScroll;
    ws.projectID    = request.projectID;
    ws.lastWritingSurface = lws;

    if (auto r = wsService.save(request.appSupportRoot, ws); !r.ok()) {
        return Result<OpenSceneResult>::failure(r.error());
    }

    // 4. Assemble result
    SceneSummary summary;
    summary.sceneID      = found->sceneID;
    summary.chapterID    = found->chapterID;
    summary.title        = found->title;
    summary.slug         = found->slug;
    summary.metadataPath = found->metadataPath;
    summary.contentPath  = found->contentPath;

    OpenSceneResult result;
    result.scene             = std::move(summary);
    result.markdown          = std::move(mdR.value());
    result.restoredSelection = restoredSelection;
    result.restoredScroll    = restoredScroll;
    return Result<OpenSceneResult>::success(std::move(result));
}

Result<SaveSceneResult> ScriviCore::saveScene(
    const SaveSceneRequest& request) {
    manuscript::SceneWriter writer{services_};
    return writer.save(request);
}

Result<ExternalChangeScanResult> ScriviCore::scanForExternalChanges(
    const ExternalChangeScanRequest& request) {
    repair::ExternalChangeScanner scanner{services_};
    return scanner.scan(request);
}

Result<EnableGitResult> ScriviCore::enableGitSnapshots(
    const EnableGitRequest& request) {
    git::SnapshotService service{services_};
    return service.enable(request);
}

Result<CreateSnapshotResult> ScriviCore::createSnapshot(
    const CreateSnapshotRequest& request) {
    git::SnapshotService service{services_};
    return service.createSnapshot(request);
}

Result<ApplyRepairResult> ScriviCore::applyRepair(
    const ApplyRepairRequest& request) {
    repair::RepairDispatcher dispatcher{services_};
    return dispatcher.apply(request);
}

Result<CreateObjectResult> ScriviCore::createObject(
    const CreateObjectRequest& request) {
    objects::ObjectStore store{services_};
    return store.create(request);
}

Result<OpenObjectResult> ScriviCore::openObject(
    const OpenObjectRequest& request) {
    objects::ObjectStore store{services_};
    return store.open(request);
}

Result<SaveObjectResult> ScriviCore::saveObject(
    const SaveObjectRequest& request) {
    objects::ObjectStore store{services_};
    return store.save(request);
}

Result<DeleteObjectResult> ScriviCore::deleteObject(
    const DeleteObjectRequest& request) {
    objects::ObjectStore store{services_};
    return store.remove(request);
}

Result<ImportAssetResult> ScriviCore::importAsset(
    const ImportAssetRequest& request) {
    assets::AssetStore store{services_};
    return store.import(request);
}

Result<ListAssetsResult> ScriviCore::listAssets(
    const ListAssetsRequest& request) {
    assets::AssetStore store{services_};
    return store.list(request);
}

Result<RemoveAssetResult> ScriviCore::removeAsset(
    const RemoveAssetRequest& request) {
    assets::AssetStore store{services_};
    return store.remove(request);
}

Result<AddCommentResult> ScriviCore::addComment(
    const AddCommentRequest& request) {
    comments::CommentStore store{services_};
    return store.add(request);
}

Result<ListCommentsResult> ScriviCore::listComments(
    const ListCommentsRequest& request) {
    comments::CommentStore store{services_};
    return store.list(request);
}

Result<ResolveCommentResult> ScriviCore::resolveComment(
    const ResolveCommentRequest& request) {
    comments::CommentStore store{services_};
    return store.resolve(request);
}

Result<ListInboxResult> ScriviCore::listInbox(
    const ListInboxRequest& request) {
    inbox::InboxStore store{services_};
    return store.list(request);
}

Result<ImportFromInboxResult> ScriviCore::importFromInbox(
    const ImportFromInboxRequest& request) {
    inbox::InboxStore store{services_};
    return store.importFromInbox(request);
}

Result<CreateSceneResult> ScriviCore::createScene(
    const CreateSceneRequest& request) {
    manuscript::SceneCreator creator{services_};
    return creator.create(request);
}

Result<CreateChapterResult> ScriviCore::createChapter(
    const CreateChapterRequest& request) {
    manuscript::ChapterCreator creator{services_};
    return creator.create(request);
}

Result<DeleteSceneResult> ScriviCore::deleteScene(
    const DeleteSceneRequest& request) {
    manuscript::SceneDeleter deleter{services_};
    return deleter.remove(request);
}

Result<DeleteChapterResult> ScriviCore::deleteChapter(
    const DeleteChapterRequest& request) {
    manuscript::ChapterDeleter deleter{services_};
    return deleter.remove(request);
}

Result<RenameSceneResult> ScriviCore::renameScene(
    const RenameSceneRequest& request) {
    manuscript::SceneRenamer renamer{services_};
    return renamer.rename(request);
}

Result<RenameChapterResult> ScriviCore::renameChapter(
    const RenameChapterRequest& request) {
    manuscript::ChapterRenamer renamer{services_};
    return renamer.rename(request);
}

Result<ReorderSceneResult> ScriviCore::reorderScene(
    const ReorderSceneRequest& request) {
    manuscript::SceneReorderer reorderer{services_};
    return reorderer.reorder(request);
}

Result<ReorderChapterResult> ScriviCore::reorderChapter(
    const ReorderChapterRequest& request) {
    manuscript::ChapterReorderer reorderer{services_};
    return reorderer.reorder(request);
}

Result<MergeSceneResult> ScriviCore::mergeScene(
    const MergeSceneRequest& request) {
    manuscript::SceneMerger merger{services_};
    return merger.merge(request);
}

Result<MergeChapterResult> ScriviCore::mergeChapter(
    const MergeChapterRequest& request) {
    manuscript::ChapterMerger merger{services_};
    return merger.merge(request);
}

// ---------------------------------------------------------------------------
// Timeline — EP-016 SP-039
// ---------------------------------------------------------------------------

namespace {
    constexpr std::string_view kTimelineMetaPath = "objects/timelines/timeline.meta.json";
    constexpr std::string_view kStoryStructurePath = "objects/story-structures/story-structure.json";
    constexpr std::string_view kHistoricalEventsDir = "objects/historical-events";
    constexpr std::string_view kImportedTimelinesDir = "objects/imported-timelines";

    // Built-in band definitions per structure ID.
    // Returns empty vector if structureID is "custom" or unknown.
    std::vector<schemas::BandLayout> builtInBands(std::string_view structureID) {
        if (structureID == "three-act") {
            return {
                {"act-i",   "Act I",   "#5B8DD9", 1.0/3},
                {"act-ii",  "Act II",  "#D9A05B", 1.0/3},
                {"act-iii", "Act III", "#5BD98D", 1.0/3},
            };
        }
        if (structureID == "five-act") {
            return {
                {"exposition",   "Exposition",    "#5B8DD9", 0.2},
                {"rising-action","Rising Action",  "#8D5BD9", 0.2},
                {"climax",       "Climax",         "#D95B5B", 0.2},
                {"falling-action","Falling Action","#D9A05B", 0.2},
                {"denouement",   "Denouement",     "#5BD98D", 0.2},
            };
        }
        if (structureID == "heroes-journey") {
            const double w = 1.0 / 12;
            return {
                {"ordinary-world",      "Ordinary World",         "#5B8DD9", w},
                {"call-to-adventure",   "Call to Adventure",      "#7B8DD9", w},
                {"refusal",             "Refusal of the Call",    "#9B8DD9", w},
                {"meeting-mentor",      "Meeting the Mentor",     "#BD8DD9", w},
                {"crossing-threshold",  "Crossing the Threshold", "#D98DBD", w},
                {"tests-allies-enemies","Tests, Allies, Enemies", "#D98D9B", w},
                {"approach",            "Approach",               "#D9A05B", w},
                {"ordeal",              "Ordeal",                 "#D9735B", w},
                {"reward",              "Reward",                 "#D95B5B", w},
                {"road-back",           "The Road Back",          "#A05BD9", w},
                {"resurrection",        "Resurrection",           "#7B5BD9", w},
                {"return",              "Return with the Elixir", "#5B8DD9", w},
            };
        }
        if (structureID == "save-the-cat") {
            const double w = 1.0 / 15;
            return {
                {"opening-image",   "Opening Image",          "#5B8DD9", w},
                {"theme-stated",    "Theme Stated",           "#6B8DD9", w},
                {"set-up",          "Set-Up",                 "#7B8DD9", w},
                {"catalyst",        "Catalyst",               "#8B8DD9", w},
                {"debate",          "Debate",                 "#9B8DD9", w},
                {"break-into-two",  "Break Into Two",         "#AB8DD9", w},
                {"b-story",         "B Story",                "#BD8DD9", w},
                {"fun-and-games",   "Fun and Games",          "#D98DBD", w},
                {"midpoint",        "Midpoint",               "#D98D9B", w},
                {"bad-guys-close-in","Bad Guys Close In",     "#D9735B", w},
                {"all-is-lost",     "All Is Lost",            "#D95B5B", w},
                {"dark-night",      "Dark Night of the Soul", "#C05B5B", w},
                {"break-into-three","Break Into Three",       "#A05BD9", w},
                {"finale",          "Finale",                 "#7B5BD9", w},
                {"final-image",     "Final Image",            "#5B8DD9", w},
            };
        }
        if (structureID == "freytag") {
            return {
                {"introduction",  "Introduction",  "#5B8DD9", 0.2},
                {"rising-action", "Rising Action", "#8D5BD9", 0.2},
                {"climax",        "Climax",        "#D95B5B", 0.2},
                {"falling-action","Falling Action","#D9A05B", 0.2},
                {"catastrophe",   "Catastrophe",   "#5BD98D", 0.2},
            };
        }
        if (structureID == "kishotenketsu") {
            return {
                {"ki",    "Ki — Introduction", "#5B8DD9", 0.25},
                {"sho",   "Sho — Development", "#8D5BD9", 0.25},
                {"ten",   "Ten — Twist",        "#D95B5B", 0.25},
                {"ketsu", "Ketsu — Conclusion", "#5BD98D", 0.25},
            };
        }
        if (structureID == "in-medias-res") {
            return {
                {"action-open",  "Action Open",  "#D95B5B", 0.25},
                {"backstory",    "Backstory",     "#5B8DD9", 0.25},
                {"rising-action","Rising Action", "#8D5BD9", 0.25},
                {"resolution",   "Resolution",   "#5BD98D", 0.25},
            };
        }
        return {};
    }
} // anonymous namespace

Result<GetTimelineResult> ScriviCore::getTimeline(const GetTimelineRequest& request) {
    auto path = util::join(request.projectRootPath, kTimelineMetaPath);
    auto textR = services_.fileSystem->readTextFile(path);
    if (!textR.ok()) { return Result<GetTimelineResult>::failure(textR.error()); }
    auto parseR = schemas::parseTimelineMeta(textR.value());
    if (!parseR.ok()) { return Result<GetTimelineResult>::failure(parseR.error()); }
    const auto& d = parseR.value();
    GetTimelineResult result;
    result.timelineID = d.timelineID.value;
    result.epochLabel = d.epochLabel;
    result.projectID  = d.projectID.value;
    result.createdAt  = d.createdAt;
    return Result<GetTimelineResult>::success(std::move(result));
}

Result<SetTimelineEpochLabelResult> ScriviCore::setTimelineEpochLabel(
    const SetTimelineEpochLabelRequest& request) {
    auto path = util::join(request.projectRootPath, kTimelineMetaPath);
    auto textR = services_.fileSystem->readTextFile(path);
    if (!textR.ok()) { return Result<SetTimelineEpochLabelResult>::failure(textR.error()); }
    auto parseR = schemas::parseTimelineMeta(textR.value());
    if (!parseR.ok()) { return Result<SetTimelineEpochLabelResult>::failure(parseR.error()); }
    auto data = std::move(parseR.value());
    data.epochLabel = request.label;
    auto r = services_.fileSystem->atomicWriteTextFile(path, schemas::serializeTimelineMeta(data));
    if (!r.ok()) { return Result<SetTimelineEpochLabelResult>::failure(r.error()); }
    return Result<SetTimelineEpochLabelResult>::success({.updated = true});
}

// Helper: find scene metadata path by sceneID using ManuscriptOrderResolver.
static Result<std::string> findSceneMetaPath(
    const AbsolutePath& projectRoot, const SceneID& sceneID, CoreServices& services) {
    manuscript::ManuscriptOrderResolver resolver{services};
    auto scenesR = resolver.resolve(projectRoot);
    if (!scenesR.ok()) { return Result<std::string>::failure(scenesR.error()); }
    for (const auto& s : scenesR.value()) {
        if (s.sceneID.value == sceneID.value) {
            return Result<std::string>::success(util::join(projectRoot, s.metadataPath));
        }
    }
    return Result<std::string>::failure(
        {.code = ErrorCode::invalidArgument, .message = "Scene not found: " + sceneID.value});
}

Result<SetSceneStoryTimeResult> ScriviCore::setSceneStoryTime(
    const SetSceneStoryTimeRequest& request) {
    auto pathR = findSceneMetaPath(request.projectRootPath, request.sceneID, services_);
    if (!pathR.ok()) { return Result<SetSceneStoryTimeResult>::failure(pathR.error()); }
    auto textR = services_.fileSystem->readTextFile(pathR.value());
    if (!textR.ok()) { return Result<SetSceneStoryTimeResult>::failure(textR.error()); }
    auto parseR = schemas::parseSceneMeta(textR.value());
    if (!parseR.ok()) { return Result<SetSceneStoryTimeResult>::failure(parseR.error()); }
    auto data = std::move(parseR.value());
    data.storyTime.offsetMs       = request.offsetMs;
    data.storyTime.offsetSource   = request.source.empty() ? "manual" : request.source;
    data.storyTime.gapMs          = request.gapMs;
    data.storyTime.durationMs     = request.durationMs > 0 ? request.durationMs : data.storyTime.durationMs;
    data.storyTime.durationSource = request.durationSource.empty() ? "default" : request.durationSource;
    auto r = services_.fileSystem->atomicWriteTextFile(pathR.value(), schemas::serializeSceneMeta(data));
    if (!r.ok()) { return Result<SetSceneStoryTimeResult>::failure(r.error()); }
    return Result<SetSceneStoryTimeResult>::success({.sceneID = request.sceneID, .updated = true});
}

Result<GetSceneStoryTimeResult> ScriviCore::getSceneStoryTime(
    const GetSceneStoryTimeRequest& request) {
    auto pathR = findSceneMetaPath(request.projectRootPath, request.sceneID, services_);
    if (!pathR.ok()) { return Result<GetSceneStoryTimeResult>::failure(pathR.error()); }
    auto textR = services_.fileSystem->readTextFile(pathR.value());
    if (!textR.ok()) { return Result<GetSceneStoryTimeResult>::failure(textR.error()); }
    auto parseR = schemas::parseSceneMeta(textR.value());
    if (!parseR.ok()) { return Result<GetSceneStoryTimeResult>::failure(parseR.error()); }
    const auto& st = parseR.value().storyTime;
    GetSceneStoryTimeResult result;
    result.sceneID              = request.sceneID;
    result.offsetMs             = st.offsetMs;
    result.offsetSource         = st.offsetSource;
    result.gapMs                = st.gapMs;
    result.durationMs           = st.durationMs;
    result.durationSource       = st.durationSource;
    result.inferenceHint        = st.inferenceHint;
    result.inferenceConfidence  = st.inferenceConfidence;
    result.bandID               = st.bandID;
    result.bandAssignedAt       = st.bandAssignedAt;
    return Result<GetSceneStoryTimeResult>::success(std::move(result));
}

Result<ClearSceneStoryTimeResult> ScriviCore::clearSceneStoryTime(
    const ClearSceneStoryTimeRequest& request) {
    auto pathR = findSceneMetaPath(request.projectRootPath, request.sceneID, services_);
    if (!pathR.ok()) { return Result<ClearSceneStoryTimeResult>::failure(pathR.error()); }
    auto textR = services_.fileSystem->readTextFile(pathR.value());
    if (!textR.ok()) { return Result<ClearSceneStoryTimeResult>::failure(textR.error()); }
    auto parseR = schemas::parseSceneMeta(textR.value());
    if (!parseR.ok()) { return Result<ClearSceneStoryTimeResult>::failure(parseR.error()); }
    auto data = std::move(parseR.value());
    data.storyTime = schemas::SceneStoryTime{};  // reset to defaults
    auto r = services_.fileSystem->atomicWriteTextFile(pathR.value(), schemas::serializeSceneMeta(data));
    if (!r.ok()) { return Result<ClearSceneStoryTimeResult>::failure(r.error()); }
    return Result<ClearSceneStoryTimeResult>::success({.sceneID = request.sceneID, .cleared = true});
}

Result<AssignSceneToBandResult> ScriviCore::assignSceneToBand(
    const AssignSceneToBandRequest& request) {
    auto pathR = findSceneMetaPath(request.projectRootPath, request.sceneID, services_);
    if (!pathR.ok()) { return Result<AssignSceneToBandResult>::failure(pathR.error()); }
    auto textR = services_.fileSystem->readTextFile(pathR.value());
    if (!textR.ok()) { return Result<AssignSceneToBandResult>::failure(textR.error()); }
    auto parseR = schemas::parseSceneMeta(textR.value());
    if (!parseR.ok()) { return Result<AssignSceneToBandResult>::failure(parseR.error()); }
    auto data = std::move(parseR.value());
    data.storyTime.bandID         = request.bandID;
    data.storyTime.bandAssignedAt = services_.clock->nowUTC();
    auto r = services_.fileSystem->atomicWriteTextFile(pathR.value(), schemas::serializeSceneMeta(data));
    if (!r.ok()) { return Result<AssignSceneToBandResult>::failure(r.error()); }
    return Result<AssignSceneToBandResult>::success({.sceneID = request.sceneID, .assigned = true});
}

Result<UnassignSceneFromBandResult> ScriviCore::unassignSceneFromBand(
    const UnassignSceneFromBandRequest& request) {
    auto pathR = findSceneMetaPath(request.projectRootPath, request.sceneID, services_);
    if (!pathR.ok()) { return Result<UnassignSceneFromBandResult>::failure(pathR.error()); }
    auto textR = services_.fileSystem->readTextFile(pathR.value());
    if (!textR.ok()) { return Result<UnassignSceneFromBandResult>::failure(textR.error()); }
    auto parseR = schemas::parseSceneMeta(textR.value());
    if (!parseR.ok()) { return Result<UnassignSceneFromBandResult>::failure(parseR.error()); }
    auto data = std::move(parseR.value());
    data.storyTime.bandID.clear();
    data.storyTime.bandAssignedAt.clear();
    auto r = services_.fileSystem->atomicWriteTextFile(pathR.value(), schemas::serializeSceneMeta(data));
    if (!r.ok()) { return Result<UnassignSceneFromBandResult>::failure(r.error()); }
    return Result<UnassignSceneFromBandResult>::success({.sceneID = request.sceneID, .unassigned = true});
}

Result<GetStoryStructureResult> ScriviCore::getStoryStructure(
    const GetStoryStructureRequest& request) {
    auto path = util::join(request.projectRootPath, kStoryStructurePath);
    auto existsR = services_.fileSystem->exists(path);
    if (!existsR.ok()) { return Result<GetStoryStructureResult>::failure(existsR.error()); }
    if (!existsR.value()) {
        return Result<GetStoryStructureResult>::success({.hasStructure = false});
    }
    auto textR = services_.fileSystem->readTextFile(path);
    if (!textR.ok()) { return Result<GetStoryStructureResult>::failure(textR.error()); }
    auto parseR = schemas::parseStoryStructure(textR.value());
    if (!parseR.ok()) { return Result<GetStoryStructureResult>::failure(parseR.error()); }
    const auto& d = parseR.value();
    // Re-serialize just the bandLayout array as JSON string for the Swift layer
    util::JsonDoc bandDoc;
    for (const auto& b : d.bandLayout) {
        util::JsonDoc item;
        item.setString("bandID",     b.bandID);
        item.setString("label",      b.label);
        item.setString("color",      b.color);
        item.setDouble("proportion", b.proportion);
        bandDoc.appendToArray("bands", std::move(item));
    }
    GetStoryStructureResult result;
    result.hasStructure   = true;
    result.structureID    = d.activeStructureID;
    result.bandLayoutJSON = bandDoc.dump();
    return Result<GetStoryStructureResult>::success(std::move(result));
}

Result<SetStoryStructureResult> ScriviCore::setStoryStructure(
    const SetStoryStructureRequest& request) {
    auto dir = util::join(request.projectRootPath, "objects/story-structures");
    if (auto r = services_.fileSystem->createDirectories(dir); !r.ok()) {
        return Result<SetStoryStructureResult>::failure(r.error());
    }
    schemas::StoryStructureData data;
    data.activeStructureID = request.structureID;
    if (request.bandLayoutJSON.empty() || request.bandLayoutJSON == "[]") {
        data.bandLayout = builtInBands(request.structureID);
    } else {
        // Parse the provided band layout JSON (bands array wrapped in an object)
        auto parseR = util::parseJson(request.bandLayoutJSON);
        if (parseR.ok()) {
            const auto& doc = parseR.value();
            const auto count = doc.arraySize("bands");
            data.bandLayout.reserve(count);
            for (std::size_t i = 0; i < count; ++i) {
                auto item = doc.arrayItem("bands", i);
                schemas::BandLayout b;
                b.bandID     = item.getString("bandID");
                b.label      = item.getString("label");
                b.color      = item.getString("color");
                b.proportion = item.getDouble("proportion", 0.0);
                data.bandLayout.push_back(std::move(b));
            }
        }
    }
    auto path = util::join(request.projectRootPath, kStoryStructurePath);
    auto r = services_.fileSystem->atomicWriteTextFile(path, schemas::serializeStoryStructure(data));
    if (!r.ok()) { return Result<SetStoryStructureResult>::failure(r.error()); }
    return Result<SetStoryStructureResult>::success({.set = true});
}

Result<UpdateBandLayoutResult> ScriviCore::updateBandLayout(
    const UpdateBandLayoutRequest& request) {
    auto path = util::join(request.projectRootPath, kStoryStructurePath);
    auto textR = services_.fileSystem->readTextFile(path);
    if (!textR.ok()) { return Result<UpdateBandLayoutResult>::failure(textR.error()); }
    auto parseR = schemas::parseStoryStructure(textR.value());
    if (!parseR.ok()) { return Result<UpdateBandLayoutResult>::failure(parseR.error()); }
    auto data = std::move(parseR.value());
    // Parse new band layout from JSON
    auto jsonR = util::parseJson(request.bandLayoutJSON);
    if (!jsonR.ok()) { return Result<UpdateBandLayoutResult>::failure(jsonR.error()); }
    const auto& doc = jsonR.value();
    const auto count = doc.arraySize("bands");
    data.bandLayout.clear();
    data.bandLayout.reserve(count);
    for (std::size_t i = 0; i < count; ++i) {
        auto item = doc.arrayItem("bands", i);
        schemas::BandLayout b;
        b.bandID     = item.getString("bandID");
        b.label      = item.getString("label");
        b.color      = item.getString("color");
        b.proportion = item.getDouble("proportion", 0.0);
        data.bandLayout.push_back(std::move(b));
    }
    auto r = services_.fileSystem->atomicWriteTextFile(path, schemas::serializeStoryStructure(data));
    if (!r.ok()) { return Result<UpdateBandLayoutResult>::failure(r.error()); }
    return Result<UpdateBandLayoutResult>::success({.updated = true});
}

Result<RemoveStoryStructureResult> ScriviCore::removeStoryStructure(
    const RemoveStoryStructureRequest& request) {
    auto path = util::join(request.projectRootPath, kStoryStructurePath);
    auto existsR = services_.fileSystem->exists(path);
    if (!existsR.ok()) { return Result<RemoveStoryStructureResult>::failure(existsR.error()); }
    if (!existsR.value()) {
        return Result<RemoveStoryStructureResult>::success({.removed = false});
    }
    auto r = services_.fileSystem->removeFile(path);
    if (!r.ok()) { return Result<RemoveStoryStructureResult>::failure(r.error()); }
    return Result<RemoveStoryStructureResult>::success({.removed = true});
}

Result<CreateHistoricalEventResult> ScriviCore::createHistoricalEvent(
    const CreateHistoricalEventRequest& request) {
    auto dir = util::join(request.projectRootPath, kHistoricalEventsDir);
    if (auto r = services_.fileSystem->createDirectories(dir); !r.ok()) {
        return Result<CreateHistoricalEventResult>::failure(r.error());
    }
    schemas::HistoricalEventData data;
    data.eventID.value = services_.uuidProvider->newHistoricalEventID().value;
    data.title         = request.title;
    data.slug          = util::makeSlug(request.title);
    data.offsetMs      = request.offsetMs;
    data.offsetSource  = "manual";
    data.description   = request.description;
    data.tags          = request.tags;
    data.createdAt     = services_.clock->nowUTC();
    data.modifiedAt    = data.createdAt;
    auto filename = data.eventID.value + "-" + data.slug + ".json";
    auto path = util::join(dir, filename);
    auto r = services_.fileSystem->atomicWriteTextFile(path, schemas::serializeHistoricalEvent(data));
    if (!r.ok()) { return Result<CreateHistoricalEventResult>::failure(r.error()); }
    return Result<CreateHistoricalEventResult>::success({.eventID = data.eventID.value, .slug = data.slug});
}

Result<UpdateHistoricalEventResult> ScriviCore::updateHistoricalEvent(
    const UpdateHistoricalEventRequest& request) {
    auto dir = util::join(request.projectRootPath, kHistoricalEventsDir);
    auto entriesR = services_.fileSystem->listDirectory(dir);
    if (!entriesR.ok()) { return Result<UpdateHistoricalEventResult>::failure(entriesR.error()); }
    for (const auto& p : entriesR.value()) {
        auto textR = services_.fileSystem->readTextFile(p);
        if (!textR.ok()) { continue; }
        auto parseR = schemas::parseHistoricalEvent(textR.value());
        if (!parseR.ok()) { continue; }
        if (parseR.value().eventID.value != request.eventID) { continue; }
        auto data = std::move(parseR.value());
        data.title       = request.title;
        data.offsetMs    = request.offsetMs;
        data.description = request.description;
        data.tags        = request.tags;
        data.modifiedAt  = services_.clock->nowUTC();
        auto r = services_.fileSystem->atomicWriteTextFile(p, schemas::serializeHistoricalEvent(data));
        if (!r.ok()) { return Result<UpdateHistoricalEventResult>::failure(r.error()); }
        return Result<UpdateHistoricalEventResult>::success({.eventID = request.eventID, .updated = true});
    }
    return Result<UpdateHistoricalEventResult>::failure(
        {.code = ErrorCode::invalidArgument, .message = "Historical event not found: " + request.eventID});
}

Result<DeleteHistoricalEventResult> ScriviCore::deleteHistoricalEvent(
    const DeleteHistoricalEventRequest& request) {
    auto dir = util::join(request.projectRootPath, kHistoricalEventsDir);
    auto entriesR = services_.fileSystem->listDirectory(dir);
    if (!entriesR.ok()) { return Result<DeleteHistoricalEventResult>::failure(entriesR.error()); }
    for (const auto& p : entriesR.value()) {
        auto textR = services_.fileSystem->readTextFile(p);
        if (!textR.ok()) { continue; }
        auto parseR = schemas::parseHistoricalEvent(textR.value());
        if (!parseR.ok() || parseR.value().eventID.value != request.eventID) { continue; }
        auto r = services_.fileSystem->removeFile(p);
        if (!r.ok()) { return Result<DeleteHistoricalEventResult>::failure(r.error()); }
        return Result<DeleteHistoricalEventResult>::success({.eventID = request.eventID, .deleted = true});
    }
    return Result<DeleteHistoricalEventResult>::failure(
        {.code = ErrorCode::invalidArgument, .message = "Historical event not found: " + request.eventID});
}

Result<ListHistoricalEventsResult> ScriviCore::listHistoricalEvents(
    const ListHistoricalEventsRequest& request) {
    auto dir = util::join(request.projectRootPath, kHistoricalEventsDir);
    auto existsR = services_.fileSystem->exists(dir);
    if (!existsR.ok()) { return Result<ListHistoricalEventsResult>::failure(existsR.error()); }
    util::JsonDoc root;
    int count = 0;
    if (existsR.value()) {
        auto entriesR = services_.fileSystem->listDirectory(dir);
        if (!entriesR.ok()) { return Result<ListHistoricalEventsResult>::failure(entriesR.error()); }
        for (const auto& p : entriesR.value()) {
            auto textR = services_.fileSystem->readTextFile(p);
            if (!textR.ok()) { continue; }
            auto parseR = schemas::parseHistoricalEvent(textR.value());
            if (!parseR.ok()) { continue; }
            const auto& d = parseR.value();
            util::JsonDoc item;
            item.setString("eventID",     d.eventID.value);
            item.setString("title",       d.title);
            item.setInt64("offsetMs",     d.offsetMs);
            item.setString("offsetSource",d.offsetSource);
            item.setString("description", d.description);
            item.setString("createdAt",   d.createdAt);
            item.setString("modifiedAt",  d.modifiedAt);
            root.appendToArray("events", std::move(item));
            ++count;
        }
    }
    ListHistoricalEventsResult result;
    result.eventsJSON = root.dump();
    result.count      = count;
    return Result<ListHistoricalEventsResult>::success(std::move(result));
}

Result<ImportExternalTimelineResult> ScriviCore::importExternalTimeline(
    const ImportExternalTimelineRequest& request) {
    auto parseR = schemas::parseExternalTimeline(request.timelineJSON);
    if (!parseR.ok()) { return Result<ImportExternalTimelineResult>::failure(parseR.error()); }
    auto data = std::move(parseR.value());
    data.epochOffsetMs     = request.epochOffsetMs;
    data.visible           = true;
    data.assignedGreyShade = request.assignedGreyShade;
    auto dir = util::join(request.projectRootPath, kImportedTimelinesDir);
    if (auto r = services_.fileSystem->createDirectories(dir); !r.ok()) {
        return Result<ImportExternalTimelineResult>::failure(r.error());
    }
    auto slug = util::makeSlug(data.sourceProjectTitle.empty() ? data.timelineID : data.sourceProjectTitle);
    auto filename = data.timelineID + "-" + slug + ".scrivi-timeline.json";
    auto path = util::join(dir, filename);
    auto r = services_.fileSystem->atomicWriteTextFile(path, schemas::serializeExternalTimeline(data));
    if (!r.ok()) { return Result<ImportExternalTimelineResult>::failure(r.error()); }
    return Result<ImportExternalTimelineResult>::success({.timelineID = data.timelineID, .imported = true});
}

Result<UpdateImportedTimelineOffsetResult> ScriviCore::updateImportedTimelineOffset(
    const UpdateImportedTimelineOffsetRequest& request) {
    auto dir = util::join(request.projectRootPath, kImportedTimelinesDir);
    auto entriesR = services_.fileSystem->listDirectory(dir);
    if (!entriesR.ok()) { return Result<UpdateImportedTimelineOffsetResult>::failure(entriesR.error()); }
    for (const auto& p : entriesR.value()) {
        auto textR = services_.fileSystem->readTextFile(p);
        if (!textR.ok()) { continue; }
        auto parseR = schemas::parseExternalTimeline(textR.value());
        if (!parseR.ok() || parseR.value().timelineID != request.timelineID) { continue; }
        auto data = std::move(parseR.value());
        data.epochOffsetMs = request.epochOffsetMs;
        auto r = services_.fileSystem->atomicWriteTextFile(p, schemas::serializeExternalTimeline(data));
        if (!r.ok()) { return Result<UpdateImportedTimelineOffsetResult>::failure(r.error()); }
        return Result<UpdateImportedTimelineOffsetResult>::success({.timelineID = request.timelineID, .updated = true});
    }
    return Result<UpdateImportedTimelineOffsetResult>::failure(
        {.code = ErrorCode::invalidArgument, .message = "Imported timeline not found: " + request.timelineID});
}

Result<SetImportedTimelineVisibleResult> ScriviCore::setImportedTimelineVisible(
    const SetImportedTimelineVisibleRequest& request) {
    auto dir = util::join(request.projectRootPath, kImportedTimelinesDir);
    auto entriesR = services_.fileSystem->listDirectory(dir);
    if (!entriesR.ok()) { return Result<SetImportedTimelineVisibleResult>::failure(entriesR.error()); }
    for (const auto& p : entriesR.value()) {
        auto textR = services_.fileSystem->readTextFile(p);
        if (!textR.ok()) { continue; }
        auto parseR = schemas::parseExternalTimeline(textR.value());
        if (!parseR.ok() || parseR.value().timelineID != request.timelineID) { continue; }
        auto data = std::move(parseR.value());
        data.visible = request.visible;
        auto r = services_.fileSystem->atomicWriteTextFile(p, schemas::serializeExternalTimeline(data));
        if (!r.ok()) { return Result<SetImportedTimelineVisibleResult>::failure(r.error()); }
        return Result<SetImportedTimelineVisibleResult>::success({.timelineID = request.timelineID, .updated = true});
    }
    return Result<SetImportedTimelineVisibleResult>::failure(
        {.code = ErrorCode::invalidArgument, .message = "Imported timeline not found: " + request.timelineID});
}

Result<ListImportedTimelinesResult> ScriviCore::listImportedTimelines(
    const ListImportedTimelinesRequest& request) {
    auto dir = util::join(request.projectRootPath, kImportedTimelinesDir);
    auto existsR = services_.fileSystem->exists(dir);
    if (!existsR.ok()) { return Result<ListImportedTimelinesResult>::failure(existsR.error()); }
    util::JsonDoc root;
    int count = 0;
    if (existsR.value()) {
        auto entriesR = services_.fileSystem->listDirectory(dir);
        if (!entriesR.ok()) { return Result<ListImportedTimelinesResult>::failure(entriesR.error()); }
        for (const auto& p : entriesR.value()) {
            auto textR = services_.fileSystem->readTextFile(p);
            if (!textR.ok()) { continue; }
            auto parseR = schemas::parseExternalTimeline(textR.value());
            if (!parseR.ok()) { continue; }
            const auto& d = parseR.value();
            util::JsonDoc item;
            item.setString("timelineID",         d.timelineID);
            item.setString("sourceProjectTitle",  d.sourceProjectTitle);
            item.setString("epochLabel",          d.epochLabel);
            item.setInt64("epochOffsetMs",         d.epochOffsetMs);
            item.setBool("visible",               d.visible);
            item.setString("assignedGreyShade",   d.assignedGreyShade);
            item.setInt("eventCount",             static_cast<int>(d.events.size()));
            root.appendToArray("timelines", std::move(item));
            ++count;
        }
    }
    ListImportedTimelinesResult result;
    result.timelinesJSON = root.dump();
    result.count         = count;
    return Result<ListImportedTimelinesResult>::success(std::move(result));
}

Result<RemoveImportedTimelineResult> ScriviCore::removeImportedTimeline(
    const RemoveImportedTimelineRequest& request) {
    auto dir = util::join(request.projectRootPath, kImportedTimelinesDir);
    auto entriesR = services_.fileSystem->listDirectory(dir);
    if (!entriesR.ok()) { return Result<RemoveImportedTimelineResult>::failure(entriesR.error()); }
    for (const auto& p : entriesR.value()) {
        auto textR = services_.fileSystem->readTextFile(p);
        if (!textR.ok()) { continue; }
        auto parseR = schemas::parseExternalTimeline(textR.value());
        if (!parseR.ok() || parseR.value().timelineID != request.timelineID) { continue; }
        auto r = services_.fileSystem->removeFile(p);
        if (!r.ok()) { return Result<RemoveImportedTimelineResult>::failure(r.error()); }
        return Result<RemoveImportedTimelineResult>::success({.timelineID = request.timelineID, .removed = true});
    }
    return Result<RemoveImportedTimelineResult>::failure(
        {.code = ErrorCode::invalidArgument, .message = "Imported timeline not found: " + request.timelineID});
}

Result<ExportProjectTimelineResult> ScriviCore::exportProjectTimeline(
    const ExportProjectTimelineRequest& request) {
    // Collect timeline meta for epoch label
    auto tlPath = util::join(request.projectRootPath, kTimelineMetaPath);
    auto tlTextR = services_.fileSystem->readTextFile(tlPath);
    std::string epochLabel = "Story Open";
    std::string projectID;
    if (tlTextR.ok()) {
        auto parseR = schemas::parseTimelineMeta(tlTextR.value());
        if (parseR.ok()) {
            epochLabel = parseR.value().epochLabel;
            projectID  = parseR.value().projectID.value;
        }
    }

    schemas::ExternalTimelineData exportData;
    exportData.timelineID         = "export-" + services_.clock->nowUTC();
    exportData.sourceProjectTitle = "";  // caller can set this; not available here
    exportData.sourceProjectID    = projectID;
    exportData.exportedAt         = services_.clock->nowUTC();
    exportData.epochLabel         = epochLabel;

    // Scenes in manuscript order
    manuscript::ManuscriptOrderResolver resolver{services_};
    auto scenesR = resolver.resolve(request.projectRootPath);
    if (scenesR.ok()) {
        for (const auto& s : scenesR.value()) {
            auto metaPath = util::join(request.projectRootPath, s.metadataPath);
            auto textR = services_.fileSystem->readTextFile(metaPath);
            if (!textR.ok()) { continue; }
            auto parseR = schemas::parseSceneMeta(textR.value());
            if (!parseR.ok()) { continue; }
            schemas::ExternalTimelineEvent ev;
            ev.eventID  = s.sceneID.value;
            ev.title    = s.title;
            ev.offsetMs = parseR.value().storyTime.offsetMs;
            ev.kind     = "scene";
            exportData.events.push_back(std::move(ev));
        }
    }

    // Historical events
    auto hDir = util::join(request.projectRootPath, kHistoricalEventsDir);
    auto hExistsR = services_.fileSystem->exists(hDir);
    if (hExistsR.ok() && hExistsR.value()) {
        auto entriesR = services_.fileSystem->listDirectory(hDir);
        if (entriesR.ok()) {
            for (const auto& p : entriesR.value()) {
                auto textR = services_.fileSystem->readTextFile(p);
                if (!textR.ok()) { continue; }
                auto parseR = schemas::parseHistoricalEvent(textR.value());
                if (!parseR.ok()) { continue; }
                const auto& d = parseR.value();
                schemas::ExternalTimelineEvent ev;
                ev.eventID  = d.eventID.value;
                ev.title    = d.title;
                ev.offsetMs = d.offsetMs;
                ev.kind     = "historical";
                ev.notes    = d.description;
                exportData.events.push_back(std::move(ev));
            }
        }
    }

    ExportProjectTimelineResult result;
    result.timelineJSON = schemas::serializeExternalTimeline(exportData);
    return Result<ExportProjectTimelineResult>::success(std::move(result));
}

// ---------------------------------------------------------------------------
// Searchable content (EP-017 SP-044 — Spotlight indexing facade)
// ---------------------------------------------------------------------------

namespace {

// Singular kind name used in uniqueIdentifier / the "kind" field
// (objectKindSubdir returns the plural directory name).
std::string objectKindName(ObjectKind kind) {
    switch (kind) {
        case ObjectKind::character: return "character";
        case ObjectKind::location:  return "location";
        case ObjectKind::item:      return "item";
        case ObjectKind::rule:      return "rule";
        case ObjectKind::timeline:  return "timeline";
    }
    return "character";
}

// Appends one record per *.json world object in objects/<subdir>/ to `items`.
// Best-effort: unparseable files are skipped (a malformed object should not
// fail the whole index extraction).
void collectObjects(CoreServices& services,
                    const AbsolutePath& projectRoot,
                    const std::string& projectID,
                    ObjectKind kind,
                    std::vector<SearchableItem>& items) {
    auto& fs  = *services.fileSystem;
    auto  dir = util::join(util::join(projectRoot, "objects"), objectKindSubdir(kind));

    auto existsR = fs.exists(dir);
    if (!existsR.ok() || !existsR.value()) { return; }

    auto listR = fs.listDirectory(dir);
    if (!listR.ok()) { return; }

    const std::string kindStr = objectKindName(kind);

    for (const auto& entry : listR.value()) {
        if (util::extension(entry) != ".json") { continue; }
        auto textR = fs.readTextFile(entry);
        if (!textR.ok()) { continue; }
        auto parseR = schemas::parseWorldObject(textR.value(), kind);
        if (!parseR.ok()) { continue; }

        const auto& f = worldObjectFields(parseR.value());
        SearchableItem item;
        item.uniqueIdentifier   = kindStr + ":" + f.objectID.value;
        item.kind               = kindStr;
        item.title              = f.displayName;
        item.displayName        = f.displayName;
        item.contentDescription = f.notes;
        item.keywords           = f.tags;
        item.deepLink           = "scrivi://open?project=" + projectID
                                + "&item=" + item.uniqueIdentifier;
        items.push_back(std::move(item));
    }
}

} // namespace

Result<ExtractSearchableTextResult> ScriviCore::extractSearchableText(
    const ExtractSearchableTextRequest& request) {
    auto& fs = *services_.fileSystem;

    // 1. Project record — read project.json for identity (domain key) + title.
    auto projTextR = fs.readTextFile(util::join(request.projectRootPath, "project.json"));
    if (!projTextR.ok()) { return Result<ExtractSearchableTextResult>::failure(projTextR.error()); }

    auto projParsed = schemas::parseProject(projTextR.value());
    if (!projParsed.ok()) { return Result<ExtractSearchableTextResult>::failure(projParsed.error()); }

    const auto& proj = projParsed.value();
    const std::string projectID = proj.projectID.value;

    ExtractSearchableTextResult out;
    out.domainIdentifier = projectID;          // per-project delete-by-domain key
    out.projectRootPath  = request.projectRootPath;

    {
        SearchableItem item;
        item.uniqueIdentifier = "project:" + projectID;
        item.kind             = "project";
        item.title            = proj.title;
        item.displayName      = proj.title;     // project.json has no separate displayName/summary
        item.deepLink         = "scrivi://open?project=" + projectID
                              + "&item=" + item.uniqueIdentifier;
        out.items.push_back(std::move(item));
    }

    // 2. Scene records — manuscript order, body Markdown stripped to plain text.
    manuscript::ManuscriptOrderResolver resolver{services_};
    auto scenesR = resolver.resolve(request.projectRootPath);
    if (scenesR.ok()) {
        manuscript::SceneReader reader{services_};
        for (const auto& s : scenesR.value()) {
            SearchableItem item;
            item.uniqueIdentifier = "scene:" + s.sceneID.value;
            item.kind             = "scene";
            item.title            = s.title;
            item.displayName      = s.title;
            item.containerTitle   = s.chapterTitle;

            auto mdR = reader.readContent(request.projectRootPath, s.contentPath);
            if (mdR.ok()) {
                item.contentDescription = util::stripMarkdown(mdR.value());
            }
            item.deepLink = "scrivi://open?project=" + projectID
                          + "&item=" + item.uniqueIdentifier;
            out.items.push_back(std::move(item));
        }
    }
    // A missing/empty manuscript is not fatal for indexing — degenerate projects
    // still yield the project record (and any world objects).

    // 3. World-object records — character/location/item/rule/timeline.
    for (auto kind : {ObjectKind::character, ObjectKind::location, ObjectKind::item,
                      ObjectKind::rule, ObjectKind::timeline}) {
        collectObjects(services_, request.projectRootPath, projectID, kind, out.items);
    }

    return Result<ExtractSearchableTextResult>::success(std::move(out));
}

} // namespace scrivi
