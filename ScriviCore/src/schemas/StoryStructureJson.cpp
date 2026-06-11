#include "StoryStructureJson.hpp"
#include "SchemaUtils.hpp"

namespace scrivi::schemas {

static util::JsonDoc serializeBand(const BandLayout& b) {
    util::JsonDoc doc;
    doc.setString("bandID",     b.bandID);
    doc.setString("label",      b.label);
    doc.setString("color",      b.color);
    doc.setDouble("proportion", b.proportion);
    return doc;
}

static BandLayout parseBand(const util::JsonDoc& doc) {
    BandLayout b;
    b.bandID     = doc.getString("bandID");
    b.label      = doc.getString("label");
    b.color      = doc.getString("color");
    b.proportion = doc.getDouble("proportion", 0.0);
    return b;
}

std::string serializeStoryStructure(const StoryStructureData& d) {
    util::JsonDoc doc;
    doc.setString("schema",            "scrivi.storyStructure.v1");
    doc.setString("activeStructureID", d.activeStructureID);

    for (const auto& b : d.bandLayout) {
        doc.appendToArray("bandLayout", serializeBand(b));
    }
    for (const auto& b : d.customBands) {
        doc.appendToArray("customBands", serializeBand(b));
    }
    return doc.dump();
}

Result<StoryStructureData> parseStoryStructure(std::string_view json) {
    auto r = parseAndValidateSchema(json, "scrivi.storyStructure.v1");
    if (!r.ok()) { return Result<StoryStructureData>::failure(r.error()); }
    auto& doc = r.value();

    for (const auto* key : {"activeStructureID", "bandLayout"}) {
        auto v = requireField(doc, key);
        if (!v.ok()) { return Result<StoryStructureData>::failure(v.error()); }
    }

    StoryStructureData data;
    data.activeStructureID = doc.getString("activeStructureID");

    const auto bandCount = doc.arraySize("bandLayout");
    data.bandLayout.reserve(bandCount);
    for (std::size_t i = 0; i < bandCount; ++i) {
        data.bandLayout.push_back(parseBand(doc.arrayItem("bandLayout", i)));
    }

    const auto customCount = doc.arraySize("customBands");
    data.customBands.reserve(customCount);
    for (std::size_t i = 0; i < customCount; ++i) {
        data.customBands.push_back(parseBand(doc.arrayItem("customBands", i)));
    }

    return Result<StoryStructureData>::success(std::move(data));
}

} // namespace scrivi::schemas
