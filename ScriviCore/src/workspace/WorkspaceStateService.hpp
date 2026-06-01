#pragma once

#include "scrivi/Result.hpp"
#include "scrivi/Services.hpp"
#include "scrivi/Types.hpp"

#include <optional>
#include <string>

namespace scrivi::workspace {

class WorkspaceStateService {
public:
    explicit WorkspaceStateService(CoreServices& services);

    // Read the workspace state for a project. Returns nullopt if no state exists.
    [[nodiscard]] Result<std::optional<WorkspaceState>> load(
        const AbsolutePath& appSupportRoot,
        const ProjectID&    projectID) const;

    // Write workspace state for a project, creating directories as needed.
    [[nodiscard]] Result<void> save(
        const AbsolutePath& appSupportRoot,
        const WorkspaceState& state) const;

private:
    CoreServices& services_;

    [[nodiscard]] static AbsolutePath stateFilePath(const AbsolutePath& appSupportRoot,
                                                    const ProjectID& projectID);
};

} // namespace scrivi::workspace
