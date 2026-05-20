#include "scrivi/ScriviCore.hpp"

#include "manuscript/SceneWriter.hpp"
#include "project_package/ProjectCreator.hpp"
#include "project_package/ProjectOpener.hpp"

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
    const ExternalChangeScanRequest&) {
    return Result<ExternalChangeScanResult>::failure(
        {ErrorCode::internalError, "not implemented"});
}

Result<EnableGitResult> ScriviCore::enableGitSnapshots(
    const EnableGitRequest&) {
    return Result<EnableGitResult>::failure(
        {ErrorCode::internalError, "not implemented"});
}

Result<CreateSnapshotResult> ScriviCore::createSnapshot(
    const CreateSnapshotRequest&) {
    return Result<CreateSnapshotResult>::failure(
        {ErrorCode::internalError, "not implemented"});
}

} // namespace scrivi
