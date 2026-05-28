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

private:
    CoreServices services_;
};

} // namespace scrivi
