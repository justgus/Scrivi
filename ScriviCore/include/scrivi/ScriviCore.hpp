#pragma once

#include "scrivi/Requests.hpp"
#include "scrivi/Result.hpp"
#include "scrivi/Results.hpp"
#include "scrivi/Services.hpp"

namespace scrivi {

class ScriviCore {
public:
    explicit ScriviCore(CoreServices services);

    Result<EnsureIdentityResult>    ensureLocalIdentity(const EnsureIdentityRequest& request);
    Result<CreateProjectResult>     createProject(const CreateProjectRequest& request);
    Result<OpenProjectResult>       openProject(const OpenProjectRequest& request);
    Result<OpenSceneResult>         openScene(const OpenSceneRequest& request);
    Result<SaveSceneResult>         saveScene(const SaveSceneRequest& request);
    Result<ExternalChangeScanResult> scanForExternalChanges(const ExternalChangeScanRequest& request);
    Result<EnableGitResult>         enableGitSnapshots(const EnableGitRequest& request);
    Result<CreateSnapshotResult>    createSnapshot(const CreateSnapshotRequest& request);
    Result<ApplyRepairResult>       applyRepair(const ApplyRepairRequest& request);

    Result<CreateObjectResult>      createObject(const CreateObjectRequest& request);
    Result<OpenObjectResult>        openObject(const OpenObjectRequest& request);
    Result<SaveObjectResult>        saveObject(const SaveObjectRequest& request);
    Result<DeleteObjectResult>      deleteObject(const DeleteObjectRequest& request);

    Result<ImportAssetResult>       importAsset(const ImportAssetRequest& request);
    Result<ListAssetsResult>        listAssets(const ListAssetsRequest& request);
    Result<RemoveAssetResult>       removeAsset(const RemoveAssetRequest& request);

    Result<AddCommentResult>        addComment(const AddCommentRequest& request);
    Result<ListCommentsResult>      listComments(const ListCommentsRequest& request);
    Result<ResolveCommentResult>    resolveComment(const ResolveCommentRequest& request);

    Result<ListInboxResult>         listInbox(const ListInboxRequest& request);
    Result<ImportFromInboxResult>   importFromInbox(const ImportFromInboxRequest& request);

    Result<CreateSceneResult>       createScene(const CreateSceneRequest& request);
    Result<CreateChapterResult>     createChapter(const CreateChapterRequest& request);

    Result<DeleteSceneResult>       deleteScene(const DeleteSceneRequest& request);
    Result<DeleteChapterResult>     deleteChapter(const DeleteChapterRequest& request);

    Result<RenameSceneResult>       renameScene(const RenameSceneRequest& request);
    Result<RenameChapterResult>     renameChapter(const RenameChapterRequest& request);

    Result<ReorderSceneResult>      reorderScene(const ReorderSceneRequest& request);
    Result<ReorderChapterResult>    reorderChapter(const ReorderChapterRequest& request);

    // Merge (EP-028 SP-074)
    Result<MergeSceneResult>        mergeScene(const MergeSceneRequest& request);
    Result<MergeChapterResult>      mergeChapter(const MergeChapterRequest& request);

    // Timeline (EP-016 SP-039)
    Result<GetTimelineResult>             getTimeline(const GetTimelineRequest& request);
    Result<SetTimelineEpochLabelResult>   setTimelineEpochLabel(const SetTimelineEpochLabelRequest& request);
    Result<SetSceneStoryTimeResult>       setSceneStoryTime(const SetSceneStoryTimeRequest& request);
    Result<GetSceneStoryTimeResult>       getSceneStoryTime(const GetSceneStoryTimeRequest& request);
    Result<ClearSceneStoryTimeResult>     clearSceneStoryTime(const ClearSceneStoryTimeRequest& request);
    Result<AssignSceneToBandResult>       assignSceneToBand(const AssignSceneToBandRequest& request);
    Result<UnassignSceneFromBandResult>   unassignSceneFromBand(const UnassignSceneFromBandRequest& request);
    Result<GetStoryStructureResult>       getStoryStructure(const GetStoryStructureRequest& request);
    Result<SetStoryStructureResult>       setStoryStructure(const SetStoryStructureRequest& request);
    Result<UpdateBandLayoutResult>        updateBandLayout(const UpdateBandLayoutRequest& request);
    Result<RemoveStoryStructureResult>    removeStoryStructure(const RemoveStoryStructureRequest& request);
    Result<CreateHistoricalEventResult>   createHistoricalEvent(const CreateHistoricalEventRequest& request);
    Result<UpdateHistoricalEventResult>   updateHistoricalEvent(const UpdateHistoricalEventRequest& request);
    Result<DeleteHistoricalEventResult>   deleteHistoricalEvent(const DeleteHistoricalEventRequest& request);
    Result<ListHistoricalEventsResult>    listHistoricalEvents(const ListHistoricalEventsRequest& request);
    Result<ImportExternalTimelineResult>        importExternalTimeline(const ImportExternalTimelineRequest& request);
    Result<UpdateImportedTimelineOffsetResult>  updateImportedTimelineOffset(const UpdateImportedTimelineOffsetRequest& request);
    Result<SetImportedTimelineVisibleResult>    setImportedTimelineVisible(const SetImportedTimelineVisibleRequest& request);
    Result<ListImportedTimelinesResult>         listImportedTimelines(const ListImportedTimelinesRequest& request);
    Result<RemoveImportedTimelineResult>        removeImportedTimeline(const RemoveImportedTimelineRequest& request);
    Result<ExportProjectTimelineResult>         exportProjectTimeline(const ExportProjectTimelineRequest& request);

    // Searchable content (EP-017 SP-044 — Spotlight indexing facade).
    // Read-only: composes existing schema readers and returns one record per
    // indexable item (project, scenes, world objects). Single source of indexing
    // truth for both Layer 1 (in-app donations) and Layer 2 (importer extension).
    Result<ExtractSearchableTextResult>         extractSearchableText(const ExtractSearchableTextRequest& request);

private:
    CoreServices services_;
};

} // namespace scrivi
