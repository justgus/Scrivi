#include "ProjectPersonasJson.hpp"
#include "SchemaUtils.hpp"

namespace scrivi::schemas {

std::string serializeProjectPersonas(const ProjectPersonasData& d) {
    util::JsonDoc doc;
    doc.setString("schema", "scrivi.projectPersonas.v1");

    for (auto& p : d.personas) {
        util::JsonDoc entry;
        entry.setString("personaID",   p.personaID.value);
        entry.setString("displayName", p.displayName);
        entry.setString("personaKind", p.personaKind);
        entry.setString("createdAt",   p.createdAt);
        entry.setString("status",      p.status);
        // controlledBy is an array of identity IDs per schema spec
        util::JsonDoc idDoc;
        idDoc.setString("value", p.controlledByIdentityID);
        entry.appendToArray("controlledBy", std::move(idDoc));
        doc.appendToArray("personas", std::move(entry));
    }

    return doc.dump();
}

Result<ProjectPersonasData> parseProjectPersonas(std::string_view json) {
    auto r = parseAndValidateSchema(json, "scrivi.projectPersonas.v1");
    if (!r.ok()) return Result<ProjectPersonasData>::failure(r.error());
    auto& doc = r.value();

    auto v = requireField(doc, "personas");
    if (!v.ok()) return Result<ProjectPersonasData>::failure(v.error());

    ProjectPersonasData data;
    auto n = doc.arraySize("personas");
    for (std::size_t i = 0; i < n; ++i) {
        auto item = doc.arrayItem("personas", i);
        PersonaEntry p;
        p.personaID.value = item.getString("personaID");
        p.displayName     = item.getString("displayName");
        p.personaKind     = item.getString("personaKind");
        p.createdAt       = item.getString("createdAt");
        p.status          = item.getString("status");
        if (item.arraySize("controlledBy") > 0)
            p.controlledByIdentityID = item.arrayItem("controlledBy", 0).getString("value");
        data.personas.push_back(std::move(p));
    }

    return Result<ProjectPersonasData>::success(std::move(data));
}

} // namespace scrivi::schemas
