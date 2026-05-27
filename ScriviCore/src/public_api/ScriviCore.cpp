#include "scrivi/ScriviCore.hpp"

#include "git/SnapshotService.hpp"
#include "identity/IdentityService.hpp"
#include "manuscript/SceneWriter.hpp"
#include "project_package/ProjectCreator.hpp"
#include "project_package/ProjectOpener.hpp"
#include "repair/ExternalChangeScanner.hpp"
#include "repair/RepairDispatcher.hpp"
#include "objects/ObjectStore.hpp"
#include "platform/AppSupportLayout.hpp"

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

} // namespace scrivi
