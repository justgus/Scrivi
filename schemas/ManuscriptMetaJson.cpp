#include "ManuscriptMetaJson.hpp"
#include "SchemaUtils.hpp"

namespace scrivi::schemas {

std::string serializeManuscriptMeta(const ManuscriptMetaData& d) {
    util::JsonDoc createdBy;
    createdBy.setString("identityID",            d.createdByIdentityID);
    createdBy.setString("personaID",             d.createdByPersonaID);
    createdBy.setString("displayNameAtCreation", d.createdByDisplayName);

    util::JsonDoc structure;
    for (auto& ch : d.chapters) {
        util::JsonDoc ref;
        ref.setString("chapterID", ch.chapterID.value);
        ref.setString("path",      ch.path);
        structure.appendToArray("chapters", std::move(ref));
    }

    util::JsonDoc doc;
    doc.setString("schema",       "scrivi.manuscript.v1");
    doc.setString("manuscriptID", d.manuscriptID.value);
    doc.setString("title",        d.title);
    doc.setString("createdAt",    d.createdAt);
    doc.setSubDoc("createdBy",    std::move(createdBy));
    doc.setSubDoc("structure",    std::move(structure));

    return doc.dump();
}

Result<ManuscriptMetaData> parseManuscriptMeta(std::string_view json) {
    auto r = parseAndValidateSchema(json, "scrivi.manuscript.v1");
    if (!r.ok()) return Result<ManuscriptMetaData>::failure(r.error());
    auto& doc = r.value();

    for (auto key : {"manuscriptID", "title"}) {
        auto v = requireField(doc, key);
        if (!v.ok()) return Result<ManuscriptMetaData>::failure(v.error());
    }

    ManuscriptMetaData data;
    data.manuscriptID.value   = doc.getString("manuscriptID");
    data.title                = doc.getString("title");
    data.createdAt            = doc.getString("createdAt");

    auto createdBy = doc.getSubDoc("createdBy");
    data.createdByIdentityID  = createdBy.getString("identityID");
    data.createdByPersonaID   = createdBy.getString("personaID");
    data.createdByDisplayName = createdBy.getString("displayNameAtCreation");

    auto structure = doc.getSubDoc("structure");
    auto n = structure.arraySize("chapters");
    for (std::size_t i = 0; i < n; ++i) {
        auto item = structure.arrayItem("chapters", i);
        ChapterRef ref;
        ref.chapterID.value = item.getString("chapterID");
        ref.path            = item.getString("path");
        data.chapters.push_back(std::move(ref));
    }

    return Result<ManuscriptMetaData>::success(std::move(data));
}

} // namespace scrivi::schemas
