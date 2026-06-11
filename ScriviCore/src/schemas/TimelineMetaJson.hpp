#pragma once

#include "scrivi/IDs.hpp"
#include "scrivi/Result.hpp"
#include "scrivi/Types.hpp"

#include <string>
#include <string_view>

namespace scrivi::schemas {

struct TimelineMetaData {
    TimelineID          timelineID;
    ProjectID           projectID;
    ISO8601Timestamp    createdAt;
    std::string         epochLabel;   // default: "Story Open"
    std::string         notes;        // may be empty
};

std::string serializeTimelineMeta(const TimelineMetaData& data);
Result<TimelineMetaData> parseTimelineMeta(std::string_view json);

} // namespace scrivi::schemas
