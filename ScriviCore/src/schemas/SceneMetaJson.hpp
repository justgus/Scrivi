#pragma once

#include "scrivi/Result.hpp"
#include "scrivi/IDs.hpp"
#include "scrivi/Types.hpp"

#include <cstddef>
#include <string>

namespace scrivi::schemas {

struct SceneMetaData {
    SceneID     sceneID;
    std::string title;
    Slug        slug;
    std::string status;
    ISO8601Timestamp createdAt;
    std::string createdByIdentityID;
    std::string createdByPersonaID;
    std::string createdByDisplayName;
    ISO8601Timestamp modifiedAt;
    std::string modifiedByIdentityID;
    std::string modifiedByPersonaID;
    std::string modifiedByDisplayName;
    std::string contentPath;
    std::size_t wordCount      = 0;
    std::size_t characterCount = 0;
};

std::string serializeSceneMeta(const SceneMetaData& data);
Result<SceneMetaData> parseSceneMeta(std::string_view json);

} // namespace scrivi::schemas
