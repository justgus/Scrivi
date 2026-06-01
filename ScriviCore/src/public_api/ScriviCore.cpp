#include "scrivi/ScriviCore.hpp"

#include "git/SnapshotService.hpp"
#include "identity/IdentityService.hpp"
#include "manuscript/ManuscriptOrderResolver.hpp"
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

namespace scrivi {

ScriviCore::ScriviCore(CoreServices services)
    : services_(services) {}

Result<EnsureIdentityResult> ScriviCore::ensureLocalIdentity(
    const EnsureIdentityRequest& request) {
    if (auto r = util::bootstrapAppSupport(request.appSupportRoot, *services_.fileSystem); !r.ok())
        return Result<EnsureIdentityResult>::failure(r.error());
    identity::IdentityService svc{services_};
    return svc.ensureLocalIdentity(request);
}

Result<CreateProjectResult> ScriviCore::createProject(
    const CreateProjectRequest& request) {
    if (auto r = util::bootstrapAppSupport(request.appSupportRoot, *services_.fileSystem); !r.ok())
        return Result<CreateProjectResult>::failure(r.error());
    project_package::ProjectCreator creator{services_};
    return creator.create(request);
}

Result<OpenProjectResult> ScriviCore::openProject(
    const OpenProjectRequest& request) {
    if (auto r = util::bootstrapAppSupport(request.appSupportRoot, *services_.fileSystem); !r.ok())
        return Result<OpenProjectResult>::failure(r.error());
    project_package::ProjectOpener opener{services_};
    return opener.open(request);
}

Result<OpenSceneResult> ScriviCore::openScene(
    const OpenSceneRequest& request) {
    if (auto r = util::bootstrapAppSupport(request.appSupportRoot, *services_.fileSystem); !r.ok())
        return Result<OpenSceneResult>::failure(r.error());

    // 1. Resolve manuscript order to find the requested scene
    manuscript::ManuscriptOrderResolver resolver{services_};
    auto scenesR = resolver.resolve(request.projectRootPath);
    if (!scenesR.ok()) return Result<OpenSceneResult>::failure(scenesR.error());

    const manuscript::ResolvedScene* found = nullptr;
    for (auto& s : scenesR.value()) {
        if (s.sceneID.value == request.sceneID.value) {
            found = &s;
            break;
        }
    }
    if (!found) {
        return Result<OpenSceneResult>::failure(
            Error{ErrorCode::invalidArgument, "Scene not found: " + request.sceneID.value});
    }

    // 2. Read scene content
    manuscript::SceneReader reader{services_};
    auto mdR = reader.readContent(request.projectRootPath, found->contentPath);
    if (!mdR.ok()) return Result<OpenSceneResult>::failure(mdR.error());

    // 3. Load workspace state to restore cursor/scroll, then update active scene
    workspace::WorkspaceStateService wsService{services_};
    auto wsR = wsService.load(request.appSupportRoot, request.projectID);
    if (!wsR.ok()) return Result<OpenSceneResult>::failure(wsR.error());

    TextSelection  restoredSelection;
    ScrollPosition restoredScroll;

    WorkspaceState ws;
    if (wsR.value().has_value()) {
        ws = *wsR.value();
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

    if (auto r = wsService.save(request.appSupportRoot, ws); !r.ok())
        return Result<OpenSceneResult>::failure(r.error());

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

} // namespace scrivi
