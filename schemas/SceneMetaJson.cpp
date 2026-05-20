#include "SceneMetaJson.hpp"
#include "SchemaUtils.hpp"

namespace scrivi::schemas {

std::string serializeSceneMeta(const SceneMetaData& d) {
    util::JsonDoc createdBy;
    createdBy.setString("identityID",            d.createdByIdentityID);
    createdBy.setString("personaID",             d.createdByPersonaID);
    createdBy.setString("displayNameAtCreation", d.createdByDisplayName);

    util::JsonDoc modifiedBy;
    modifiedBy.setString("identityID",               d.modifiedByIdentityID);
    modifiedBy.setString("personaID",                d.modifiedByPersonaID);
    modifiedBy.setString("displayNameAtModification",d.modifiedByDisplayName);

    util::JsonDoc content;
    content.setString("path",       d.contentPath);
    content.setString("format",     "markdown");
    content.setString("encoding",   "utf-8");
    content.setString("encryption", "none");

    util::JsonDoc stats;
    stats.setInt("wordCount",      static_cast<int>(d.wordCount));
    stats.setInt("characterCount", static_cast<int>(d.characterCount));

    util::JsonDoc doc;
    doc.setString("schema",     "scrivi.scene.v1");
    doc.setString("sceneID",    d.sceneID.value);
    doc.setString("title",      d.title);
    doc.setString("slug",       d.slug);
    doc.setString("status",     d.status);
    doc.setString("createdAt",  d.createdAt);
    doc.setString("modifiedAt", d.modifiedAt);
    doc.setSubDoc("createdBy",  std::move(createdBy));
    doc.setSubDoc("modifiedBy", std::move(modifiedBy));
    doc.setSubDoc("content",    std::move(content));
    doc.setSubDoc("stats",      std::move(stats));

    return doc.dump();
}

Result<SceneMetaData> parseSceneMeta(std::string_view json) {
    auto r = parseAndValidateSchema(json, "scrivi.scene.v1");
    if (!r.ok()) return Result<SceneMetaData>::failure(r.error());
    auto& doc = r.value();

    for (auto key : {"sceneID", "title", "status"}) {
        auto v = requireField(doc, key);
        if (!v.ok()) return Result<SceneMetaData>::failure(v.error());
    }

    SceneMetaData data;
    data.sceneID.value = doc.getString("sceneID");
    data.title         = doc.getString("title");
    data.slug          = doc.getString("slug");
    data.status        = doc.getString("status");
    data.createdAt     = doc.getString("createdAt");
    data.modifiedAt    = doc.getString("modifiedAt");

    auto createdBy = doc.getSubDoc("createdBy");
    data.createdByIdentityID  = createdBy.getString("identityID");
    data.createdByPersonaID   = createdBy.getString("personaID");
    data.createdByDisplayName = createdBy.getString("displayNameAtCreation");

    auto modifiedBy = doc.getSubDoc("modifiedBy");
    data.modifiedByIdentityID  = modifiedBy.getString("identityID");
    data.modifiedByPersonaID   = modifiedBy.getString("personaID");
    data.modifiedByDisplayName = modifiedBy.getString("displayNameAtModification");

    auto content = doc.getSubDoc("content");
    data.contentPath = content.getString("path");

    auto stats = doc.getSubDoc("stats");
    data.wordCount      = static_cast<std::size_t>(stats.getInt("wordCount"));
    data.characterCount = static_cast<std::size_t>(stats.getInt("characterCount"));

    return Result<SceneMetaData>::success(std::move(data));
}

} // namespace scrivi::schemas
