#include "scrivi/ScriviCore.hpp"

#include "git/SnapshotService.hpp"
#include "manuscript/SceneWriter.hpp"
#include "project_package/ProjectCreator.hpp"
#include "project_package/ProjectOpener.hpp"
#include "repair/ExternalChangeScanner.hpp"

namespace scrivi {

ScriviCore::ScriviCore(CoreServices services)
    : services_(services) {}

Result<EnsureIdentityResult> ScriviCore::ensureLocalIdentity(
    const EnsureIdentityRequest&) {
    return Result<EnsureIdentityResult>::failure(
        {ErrorCode::internalError, "not implemented"});
}

Result<CreateProjectResult> ScriviCore::createProject(
    const CreateProjectRequest& request) {
    project_package::ProjectCreator creator{services_};
    return creator.create(request);
}

Result<OpenProjectResult> ScriviCore::openProject(
    const OpenProjectRequest& request) {
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

} // namespace scrivi
