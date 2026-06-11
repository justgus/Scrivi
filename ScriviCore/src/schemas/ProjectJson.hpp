#pragma once

#include "scrivi/Result.hpp"
#include "scrivi/Types.hpp"
#include "scrivi/IDs.hpp"

#include <string>

namespace scrivi::schemas {

struct ProjectJsonData {
    ProjectID   projectID;
    std::string title;
    Slug        slug;
    ISO8601Timestamp createdAt;

    std::string createdByIdentityID;
    std::string createdByPersonaID;
    std::string createdByDisplayName;

    std::string manuscriptPath;
    std::string membersPath;
    std::string personasPath;

    bool    gitSnapshotsEnabled      = false;

    // Timeline defaults (optional — absent in old projects, runtime uses hardcoded defaults)
    int64_t defaultSceneDurationMs   = 3'600'000;   // 1 hour
    std::string dragPositioningMode  = "proportional";
};

std::string serializeProject(const ProjectJsonData& data);
Result<ProjectJsonData> parseProject(std::string_view json);

} // namespace scrivi::schemas
