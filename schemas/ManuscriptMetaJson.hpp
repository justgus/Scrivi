#pragma once

#include "scrivi/Result.hpp"
#include "scrivi/IDs.hpp"
#include "scrivi/Types.hpp"

#include <string>
#include <vector>

namespace scrivi::schemas {

struct ChapterRef {
    ChapterID   chapterID;
    std::string path;
};

struct ManuscriptMetaData {
    ManuscriptID manuscriptID;
    std::string  title;
    ISO8601Timestamp createdAt;
    std::string  createdByIdentityID;
    std::string  createdByPersonaID;
    std::string  createdByDisplayName;
    std::vector<ChapterRef> chapters;
};

std::string serializeManuscriptMeta(const ManuscriptMetaData& data);
Result<ManuscriptMetaData> parseManuscriptMeta(std::string_view json);

} // namespace scrivi::schemas
