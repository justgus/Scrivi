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

    [[nodiscard]] Result<void> writeGitignore(const AbsolutePath& root) const;
    [[nodiscard]] Result<void> writeSnapshotMetadata(const AbsolutePath& root) const;
};

} // namespace scrivi::project_package
