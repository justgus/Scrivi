#pragma once

#include "scrivi/IDs.hpp"
#include "scrivi/Result.hpp"
#include "scrivi/Types.hpp"

#include <cstdint>
#include <string>
#include <string_view>
#include <vector>

namespace scrivi::schemas {

struct HistoricalEventData {
    HistoricalEventID        eventID;
    std::string              title;
    Slug                     slug;
    int64_t                  offsetMs     = 0;
    std::string              offsetSource = "manual";
    std::string              description;
    std::vector<std::string> tags;
    ISO8601Timestamp         createdAt;
    ISO8601Timestamp         modifiedAt;
};

std::string serializeHistoricalEvent(const HistoricalEventData& data);
Result<HistoricalEventData> parseHistoricalEvent(std::string_view json);

} // namespace scrivi::schemas
