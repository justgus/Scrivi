#pragma once

#include "scrivi/Result.hpp"
#include "scrivi/IDs.hpp"
#include "scrivi/Types.hpp"

#include <string>
#include <vector>

namespace scrivi::schemas {

struct SceneRef {
    SceneID     sceneID;
    std::string metadataPath;
};

struct ChapterMetaData {
    ChapterID   chapterID;
    std::string title;
    Slug        slug;
    std::string displayLabel;
    std::string status;
    ISO8601Timestamp createdAt;
    std::string createdByIdentityID;
    std::string createdByPersonaID;
    std::string createdByDisplayName;
    std::vector<SceneRef> scenes;
};

std::string serializeChapterMeta(const ChapterMetaData& data);
Result<ChapterMetaData> parseChapterMeta(std::string_view json);

} // namespace scrivi::schemas
