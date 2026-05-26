#include "SnapshotMetadataJson.hpp"
#include "SchemaUtils.hpp"

namespace scrivi::schemas {

std::string serializeSnapshotMetadata(const SnapshotMetadataData& data) {
    util::JsonDoc doc;
    doc.setString("schema",        "scrivi-snapshots");
    doc.setInt("schemaVersion",    1);

    for (const auto& e : data.snapshots) {
        util::JsonDoc entry;
        entry.setString("snapshotID",             e.snapshotID);
        entry.setString("commitID",               e.commitID);
        entry.setString("label",                  e.label);
        entry.setString("note",                   e.note);
        entry.setString("createdAt",              e.createdAt);
        entry.setString("createdByIdentityID",    e.createdByIdentityID);
        entry.setString("createdByPersonaID",     e.createdByPersonaID);
        entry.setString("createdByDisplayName",   e.createdByDisplayName);
        doc.appendToArray("snapshots", std::move(entry));
    }

    return doc.dump();
}

Result<SnapshotMetadataData> parseSnapshotMetadata(std::string_view json) {
    auto r = util::parseJson(json);
    if (!r.ok()) return Result<SnapshotMetadataData>::failure(r.error());
    auto& doc = r.value();

    // Validate schema tag — this file uses a plain string, not dot-notation
    auto schemaTag = doc.getString("schema");
    if (schemaTag != "scrivi-snapshots")
        return Result<SnapshotMetadataData>::failure(
            {ErrorCode::validationError, "unexpected schema: " + schemaTag});

    SnapshotMetadataData data;
    const auto count = doc.arraySize("snapshots");
    data.snapshots.reserve(count);

    for (std::size_t i = 0; i < count; ++i) {
        auto item = doc.arrayItem("snapshots", i);
        SnapshotEntryData e;
        e.snapshotID             = item.getString("snapshotID");
        e.commitID               = item.getString("commitID");
        e.label                  = item.getString("label");
        e.note                   = item.getString("note");
        e.createdAt              = item.getString("createdAt");
        e.createdByIdentityID    = item.getString("createdByIdentityID");
        e.createdByPersonaID     = item.getString("createdByPersonaID");
        e.createdByDisplayName   = item.getString("createdByDisplayName");
        data.snapshots.push_back(std::move(e));
    }

    return Result<SnapshotMetadataData>::success(std::move(data));
}

} // namespace scrivi::schemas
