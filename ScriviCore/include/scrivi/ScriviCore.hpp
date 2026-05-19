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

private:
    CoreServices services_;
};

} // namespace scrivi
