#include "ExternalTimelineJson.hpp"
#include "SchemaUtils.hpp"

namespace scrivi::schemas {

std::string serializeExternalTimeline(const ExternalTimelineData& d) {
    util::JsonDoc doc;
    doc.setString("schema",             "scrivi.externalTimeline.v1");
    doc.setString("timelineID",         d.timelineID);
    doc.setString("sourceProjectTitle", d.sourceProjectTitle);
    doc.setString("sourceProjectID",    d.sourceProjectID);
    doc.setString("exportedAt",         d.exportedAt);
    doc.setString("epochLabel",         d.epochLabel);

    for (const auto& ev : d.events) {
        util::JsonDoc evDoc;
        evDoc.setString("eventID",  ev.eventID);
        evDoc.setString("title",    ev.title);
        evDoc.setInt64("offsetMs",  ev.offsetMs);
        evDoc.setString("kind",     ev.kind);
        evDoc.setString("notes",    ev.notes);
        doc.appendToArray("events", std::move(evDoc));
    }

    // Import-time fields (only written when non-default)
    doc.setInt64("epochOffsetMs",      d.epochOffsetMs);
    doc.setBool("visible",             d.visible);
    doc.setString("assignedGreyShade", d.assignedGreyShade);

    return doc.dump();
}

Result<ExternalTimelineData> parseExternalTimeline(std::string_view json) {
    auto r = parseAndValidateSchema(json, "scrivi.externalTimeline.v1");
    if (!r.ok()) { return Result<ExternalTimelineData>::failure(r.error()); }
    auto& doc = r.value();

    for (const auto* key : {"timelineID", "events"}) {
        auto v = requireField(doc, key);
        if (!v.ok()) { return Result<ExternalTimelineData>::failure(v.error()); }
    }

    ExternalTimelineData data;
    data.timelineID         = doc.getString("timelineID");
    data.sourceProjectTitle = doc.getString("sourceProjectTitle");
    data.sourceProjectID    = doc.getString("sourceProjectID");
    data.exportedAt         = doc.getString("exportedAt");
    data.epochLabel         = doc.getString("epochLabel");
    data.epochOffsetMs      = doc.getInt64("epochOffsetMs", 0);
    data.visible            = doc.getBool("visible", true);
    data.assignedGreyShade  = doc.getString("assignedGreyShade");

    const auto count = doc.arraySize("events");
    data.events.reserve(count);
    for (std::size_t i = 0; i < count; ++i) {
        auto ev = doc.arrayItem("events", i);
        ExternalTimelineEvent e;
        e.eventID  = ev.getString("eventID");
        e.title    = ev.getString("title");
        e.offsetMs = ev.getInt64("offsetMs", 0);
        e.kind     = ev.getString("kind");
        e.notes    = ev.getString("notes");
        data.events.push_back(std::move(e));
    }

    return Result<ExternalTimelineData>::success(std::move(data));
}

} // namespace scrivi::schemas
