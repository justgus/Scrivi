#pragma once

#include "scrivi/Result.hpp"
#include "scrivi/Types.hpp"

#include <cstdint>
#include <string>
#include <string_view>
#include <vector>

namespace scrivi::schemas {

struct ExternalTimelineEvent {
    std::string eventID;
    std::string title;
    int64_t     offsetMs = 0;
    std::string kind;    // "scene" | "historical"
    std::string notes;
};

struct ExternalTimelineData {
    std::string                        timelineID;
    std::string                        sourceProjectTitle;
    std::string                        sourceProjectID;
    ISO8601Timestamp                   exportedAt;
    std::string                        epochLabel;
    std::vector<ExternalTimelineEvent> events;

    // Import-time additions (absent in fresh exports, present in stored files)
    int64_t     epochOffsetMs     = 0;
    bool        visible           = true;
    std::string assignedGreyShade;
};

std::string serializeExternalTimeline(const ExternalTimelineData& data);
Result<ExternalTimelineData> parseExternalTimeline(std::string_view json);

} // namespace scrivi::schemas
