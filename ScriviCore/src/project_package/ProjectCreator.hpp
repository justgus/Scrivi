#pragma once

#include "scrivi/Requests.hpp"
#include "scrivi/Results.hpp"
#include "scrivi/Services.hpp"

namespace scrivi::project_package {

class ProjectCreator {
public:
    explicit ProjectCreator(CoreServices& services);

    Result<CreateProjectResult> create(const CreateProjectRequest& request);

private:
    CoreServices& services_;

    Result<void> writeGitignore(const AbsolutePath& root);
    Result<void> writeSnapshotMetadata(const AbsolutePath& root);
};

} // namespace scrivi::project_package
