#include "ChapterMetaJson.hpp"
#include "SchemaUtils.hpp"
#include "util/PathUtils.hpp"

namespace scrivi::schemas {

std::string serializeChapterMeta(const ChapterMetaData& d) {
    util::JsonDoc createdBy;
    createdBy.setString("identityID",            d.createdByIdentityID);
    createdBy.setString("personaID",             d.createdByPersonaID);
    createdBy.setString("displayNameAtCreation", d.createdByDisplayName);

    util::JsonDoc doc;
    doc.setString("schema",       "scrivi.chapter.v1");
    doc.setString("chapterID",    d.chapterID.value);
    doc.setString("title",        d.title);
    doc.setString("slug",         d.slug);
    doc.setString("displayLabel", d.displayLabel);
    doc.setString("status",       d.status);
    doc.setString("createdAt",    d.createdAt);
    doc.setSubDoc("createdBy",    std::move(createdBy));

    for (const auto& s : d.scenes) {
        util::JsonDoc ref;
        // EP-027 §8.1: filename-only scene reference (no sceneID, no folder prefix).
        ref.setString("metadataFilename", s.metadataFilename);
        doc.appendToArray("scenes", std::move(ref));
    }

    return doc.dump();
}

Result<ChapterMetaData> parseChapterMeta(std::string_view json) {
    auto r = parseAndValidateSchema(json, "scrivi.chapter.v1");
    if (!r.ok()) { return Result<ChapterMetaData>::failure(r.error()); }
    auto& doc = r.value();

    for (const auto* key : {"chapterID", "title", "displayLabel", "status"}) {
        auto v = requireField(doc, key);
        if (!v.ok()) { return Result<ChapterMetaData>::failure(v.error()); }
    }

    ChapterMetaData data;
    data.chapterID.value  = doc.getString("chapterID");
    data.title            = doc.getString("title");
    data.slug             = doc.getString("slug");
    data.displayLabel     = doc.getString("displayLabel");
    data.status           = doc.getString("status");
    data.createdAt        = doc.getString("createdAt");

    auto createdBy = doc.getSubDoc("createdBy");
    data.createdByIdentityID  = createdBy.getString("identityID");
    data.createdByPersonaID   = createdBy.getString("personaID");
    data.createdByDisplayName = createdBy.getString("displayNameAtCreation");

    auto n = doc.arraySize("scenes");
    for (std::size_t i = 0; i < n; ++i) {
        auto item = doc.arrayItem("scenes", i);
        SceneRef ref;
        // Dual-scheme read (EP-027 §8.2). New scheme: `metadataFilename` (bare filename).
        // Legacy scheme: `metadataPath` (a root-relative path that embedded the chapter
        // folder). For a legacy ref we keep only the filename so it resolves against the
        // chapter's own folder — this is what lets a not-yet-migrated project still open,
        // and it is idempotent (filename of a filename is itself). Scene migration then
        // rewrites the sidecar into the new shape and repairs any true orphans.
        std::string filename = item.getString("metadataFilename");
        if (filename.empty()) {
            const std::string legacyPath = item.getString("metadataPath");
            filename = util::filename(legacyPath);   // strip any dir prefix; no-op if bare
        }
        ref.metadataFilename = std::move(filename);
        data.scenes.push_back(std::move(ref));
    }

    return Result<ChapterMetaData>::success(std::move(data));
}

} // namespace scrivi::schemas
