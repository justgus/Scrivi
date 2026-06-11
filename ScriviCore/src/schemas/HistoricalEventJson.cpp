#include "HistoricalEventJson.hpp"
#include "SchemaUtils.hpp"

namespace scrivi::schemas {

std::string serializeHistoricalEvent(const HistoricalEventData& d) {
    util::JsonDoc doc;
    doc.setString("schema",       "scrivi.historicalEvent.v1");
    doc.setString("eventID",      d.eventID.value);
    doc.setString("title",        d.title);
    doc.setString("slug",         d.slug);
    doc.setInt64("offsetMs",      d.offsetMs);
    doc.setString("offsetSource", d.offsetSource);
    doc.setString("description",  d.description);
    doc.setString("createdAt",    d.createdAt);
    doc.setString("modifiedAt",   d.modifiedAt);
    for (const auto& tag : d.tags) {
        doc.appendStringToArray("tags", tag);
    }
    return doc.dump();
}

Result<HistoricalEventData> parseHistoricalEvent(std::string_view json) {
    auto r = parseAndValidateSchema(json, "scrivi.historicalEvent.v1");
    if (!r.ok()) { return Result<HistoricalEventData>::failure(r.error()); }
    auto& doc = r.value();

    for (const auto* key : {"eventID", "title", "offsetMs", "offsetSource"}) {
        auto v = requireField(doc, key);
        if (!v.ok()) { return Result<HistoricalEventData>::failure(v.error()); }
    }

    HistoricalEventData data;
    data.eventID.value = doc.getString("eventID");
    data.title         = doc.getString("title");
    data.slug          = doc.getString("slug");
    data.offsetMs      = doc.getInt64("offsetMs", 0);
    data.offsetSource  = doc.getString("offsetSource", "manual");
    data.description   = doc.getString("description");
    data.createdAt     = doc.getString("createdAt");
    data.modifiedAt    = doc.getString("modifiedAt");
    data.tags          = doc.getStringArray("tags");
    return Result<HistoricalEventData>::success(std::move(data));
}

} // namespace scrivi::schemas
