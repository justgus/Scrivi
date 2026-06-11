#include "TimelineMetaJson.hpp"
#include "SchemaUtils.hpp"

namespace scrivi::schemas {

std::string serializeTimelineMeta(const TimelineMetaData& d) {
    util::JsonDoc doc;
    doc.setString("schema",      "scrivi.timeline.v1");
    doc.setString("timelineID",  d.timelineID.value);
    doc.setString("projectID",   d.projectID.value);
    doc.setString("createdAt",   d.createdAt);
    doc.setString("epochLabel",  d.epochLabel);
    doc.setString("notes",       d.notes);
    return doc.dump();
}

Result<TimelineMetaData> parseTimelineMeta(std::string_view json) {
    auto r = parseAndValidateSchema(json, "scrivi.timeline.v1");
    if (!r.ok()) { return Result<TimelineMetaData>::failure(r.error()); }
    auto& doc = r.value();

    for (const auto* key : {"timelineID", "projectID", "createdAt", "epochLabel"}) {
        auto v = requireField(doc, key);
        if (!v.ok()) { return Result<TimelineMetaData>::failure(v.error()); }
    }

    TimelineMetaData data;
    data.timelineID.value = doc.getString("timelineID");
    data.projectID.value  = doc.getString("projectID");
    data.createdAt        = doc.getString("createdAt");
    data.epochLabel       = doc.getString("epochLabel");
    data.notes            = doc.getString("notes");
    return Result<TimelineMetaData>::success(std::move(data));
}

} // namespace scrivi::schemas
