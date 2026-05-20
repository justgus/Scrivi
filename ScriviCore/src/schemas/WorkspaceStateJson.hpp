#pragma once

#include "scrivi/Result.hpp"
#include "scrivi/IDs.hpp"
#include "scrivi/Types.hpp"

#include <optional>
#include <string>

namespace scrivi::schemas {

struct WorkspaceStateData {
    ProjectID  projectID;
    std::string deviceID;
    std::string identityID;
    std::string activePersonaID;
    ISO8601Timestamp lastOpenedAt;

    // optional last writing surface
    bool        hasLastWritingSurface = false;
    std::string lastSceneID;
    std::string lastContentPath;
    std::size_t cursorAnchor   = 0;
    std::size_t cursorFocus    = 0;
    double      scrollPosition = 0.0;
};

std::string serializeWorkspaceState(const WorkspaceStateData& data);
Result<WorkspaceStateData> parseWorkspaceState(std::string_view json);

} // namespace scrivi::schemas
