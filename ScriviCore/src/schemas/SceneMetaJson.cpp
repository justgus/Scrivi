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

    util::JsonDoc storyTimeDoc;
    storyTimeDoc.setInt64("offsetMs",       d.storyTime.offsetMs);
    storyTimeDoc.setString("offsetSource",  d.storyTime.offsetSource);
    storyTimeDoc.setInt64("gapMs",          d.storyTime.gapMs);
    storyTimeDoc.setInt64("durationMs",     d.storyTime.durationMs);
    storyTimeDoc.setString("durationSource",d.storyTime.durationSource);
    if (!d.storyTime.inferenceHint.empty()) {
        storyTimeDoc.setString("inferenceHint", d.storyTime.inferenceHint);
    }
    if (d.storyTime.inferenceConfidence >= 0.0) {
        storyTimeDoc.setDouble("inferenceConfidence", d.storyTime.inferenceConfidence);
    }
    util::JsonDoc storyStructureDoc;
    if (!d.storyTime.bandID.empty()) {
        storyStructureDoc.setString("bandID",       d.storyTime.bandID);
        storyStructureDoc.setString("assignedAt",   d.storyTime.bandAssignedAt);
    }
    storyTimeDoc.setSubDoc("storyStructure", std::move(storyStructureDoc));

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
    doc.setSubDoc("storyTime",  std::move(storyTimeDoc));

    return doc.dump();
}

Result<SceneMetaData> parseSceneMeta(std::string_view json) {
    auto r = parseAndValidateSchema(json, "scrivi.scene.v1");
    if (!r.ok()) { return Result<SceneMetaData>::failure(r.error()); }
    auto& doc = r.value();

    for (const auto* key : {"sceneID", "title", "status"}) {
        auto v = requireField(doc, key);
        if (!v.ok()) { return Result<SceneMetaData>::failure(v.error()); }
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

    if (doc.contains("storyTime")) {
        auto st = doc.getSubDoc("storyTime");
        data.storyTime.offsetMs            = st.getInt64("offsetMs", 0);
        data.storyTime.offsetSource        = st.getString("offsetSource", "default");
        data.storyTime.gapMs               = st.getInt64("gapMs", 0);
        data.storyTime.durationMs          = st.getInt64("durationMs", 3'600'000);
        data.storyTime.durationSource      = st.getString("durationSource", "default");
        data.storyTime.inferenceHint       = st.getString("inferenceHint");
        data.storyTime.inferenceConfidence = st.getDouble("inferenceConfidence", -1.0);
        auto ss = st.getSubDoc("storyStructure");
        data.storyTime.bandID         = ss.getString("bandID");
        data.storyTime.bandAssignedAt = ss.getString("assignedAt");
    }

    return Result<SceneMetaData>::success(std::move(data));
}

} // namespace scrivi::schemas
