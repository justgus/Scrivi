#include "WorkspaceStateJson.hpp"
#include "SchemaUtils.hpp"

namespace scrivi::schemas {

std::string serializeWorkspaceState(const WorkspaceStateData& d) {
    util::JsonDoc doc;
    doc.setString("schema",          "scrivi.workspaceState.v1");
    doc.setString("projectID",       d.projectID.value);
    doc.setString("deviceID",        d.deviceID);
    doc.setString("identityID",      d.identityID);
    doc.setString("activePersonaID", d.activePersonaID);
    doc.setString("lastOpenedAt",    d.lastOpenedAt);

    if (d.hasLastWritingSurface) {
        util::JsonDoc cursor;
        cursor.setInt("anchor", static_cast<int>(d.cursorAnchor));
        cursor.setInt("focus",  static_cast<int>(d.cursorFocus));

        util::JsonDoc lws;
        lws.setString("sceneID",        d.lastSceneID);
        lws.setString("contentPath",    d.lastContentPath);
        lws.setSubDoc("cursor",         std::move(cursor));
        lws.setDouble("scrollPosition", d.scrollPosition);

        doc.setSubDoc("lastWritingSurface", std::move(lws));
    }

    return doc.dump();
}

Result<WorkspaceStateData> parseWorkspaceState(std::string_view json) {
    auto r = parseAndValidateSchema(json, "scrivi.workspaceState.v1");
    if (!r.ok()) return Result<WorkspaceStateData>::failure(r.error());
    auto& doc = r.value();

    for (auto key : {"projectID", "deviceID", "identityID"}) {
        auto v = requireField(doc, key);
        if (!v.ok()) return Result<WorkspaceStateData>::failure(v.error());
    }

    WorkspaceStateData data;
    data.projectID.value  = doc.getString("projectID");
    data.deviceID         = doc.getString("deviceID");
    data.identityID       = doc.getString("identityID");
    data.activePersonaID  = doc.getString("activePersonaID");
    data.lastOpenedAt     = doc.getString("lastOpenedAt");

    if (doc.contains("lastWritingSurface")) {
        auto lws = doc.getSubDoc("lastWritingSurface");
        data.hasLastWritingSurface = true;
        data.lastSceneID     = lws.getString("sceneID");
        data.lastContentPath = lws.getString("contentPath");
        data.scrollPosition  = lws.getDouble("scrollPosition");
        auto cursor = lws.getSubDoc("cursor");
        data.cursorAnchor = static_cast<std::size_t>(cursor.getInt("anchor"));
        data.cursorFocus  = static_cast<std::size_t>(cursor.getInt("focus"));
    }

    return Result<WorkspaceStateData>::success(std::move(data));
}

} // namespace scrivi::schemas
