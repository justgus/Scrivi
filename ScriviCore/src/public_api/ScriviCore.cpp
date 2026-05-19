#include "scrivi/ScriviCore.hpp"

namespace scrivi {

ScriviCore::ScriviCore(CoreServices services)
    : services_(services) {}

Result<EnsureIdentityResult> ScriviCore::ensureLocalIdentity(
    const EnsureIdentityRequest&) {
    return Result<EnsureIdentityResult>::failure(
        {ErrorCode::internalError, "not implemented"});
}

Result<CreateProjectResult> ScriviCore::createProject(
    const CreateProjectRequest&) {
    return Result<CreateProjectResult>::failure(
        {ErrorCode::internalError, "not implemented"});
}

Result<OpenProjectResult> ScriviCore::openProject(
    const OpenProjectRequest&) {
    return Result<OpenProjectResult>::failure(
        {ErrorCode::internalError, "not implemented"});
}

Result<SaveSceneResult> ScriviCore::saveScene(
    const SaveSceneRequest&) {
    return Result<SaveSceneResult>::failure(
        {ErrorCode::internalError, "not implemented"});
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
