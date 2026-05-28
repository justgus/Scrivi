#include "AssetMetaJson.hpp"
#include "SchemaUtils.hpp"

namespace scrivi::schemas {

static constexpr std::string_view kAssetMetaSchema = "scrivi.asset.meta.v1";

std::string serializeAssetMeta(const AssetMeta& meta) {
    util::JsonDoc importedBy;
    importedBy.setString("identityID",   meta.importedByIdentityID);
    importedBy.setString("personaID",    meta.importedByPersonaID);
    importedBy.setString("displayName",  meta.importedByDisplayName);

    util::JsonDoc root;
    root.setString("schema",     std::string(kAssetMetaSchema));
    root.setString("assetID",    meta.assetID);
    root.setString("slug",       meta.slug);
    root.setString("filename",   meta.filename);
    root.setString("category",   assetCategoryString(meta.category));
    root.setString("mimeType",   meta.mimeType);
    root.setString("importedAt", meta.importedAt);
    root.setSubDoc("importedBy", std::move(importedBy));
    root.setString("title",      meta.title);
    root.setString("notes",      meta.notes);

    for (const auto& tag : meta.tags) {
        util::JsonDoc tagDoc;
        tagDoc.setString("v", tag);
        root.appendToArray("tags", std::move(tagDoc));
    }

    return root.dump();
}

Result<AssetMeta> parseAssetMeta(std::string_view json) {
    auto r = parseAndValidateSchema(json, kAssetMetaSchema);
    if (!r.ok()) return Result<AssetMeta>::failure(r.error());
    auto& doc = r.value();

    AssetMeta meta;
    meta.assetID = doc.getString("assetID");
    meta.slug          = doc.getString("slug");
    meta.filename      = doc.getString("filename");
    meta.category      = assetCategoryFromString(doc.getString("category"));
    meta.mimeType      = doc.getString("mimeType");
    meta.importedAt    = doc.getString("importedAt");
    meta.title         = doc.getString("title");
    meta.notes         = doc.getString("notes");

    auto importedBy = doc.getSubDoc("importedBy");
    meta.importedByIdentityID   = importedBy.getString("identityID");
    meta.importedByPersonaID    = importedBy.getString("personaID");
    meta.importedByDisplayName  = importedBy.getString("displayName");

    const auto tagCount = doc.arraySize("tags");
    meta.tags.reserve(tagCount);
    for (std::size_t i = 0; i < tagCount; ++i)
        meta.tags.push_back(doc.arrayItem("tags", i).getString("v"));

    return Result<AssetMeta>::success(std::move(meta));
}

} // namespace scrivi::schemas
