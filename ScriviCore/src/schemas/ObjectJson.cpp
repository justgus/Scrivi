#include "ObjectJson.hpp"
#include "SchemaUtils.hpp"

namespace scrivi::schemas {

static constexpr std::string_view kCharacterSchema = "scrivi.object.character.v1";

// ---------------------------------------------------------------------------
// Serialize
// ---------------------------------------------------------------------------

std::string serializeCharacter(const CharacterObject& obj)
{
    util::JsonDoc createdBy;
    createdBy.setString("identityID",           obj.createdByIdentityID);
    createdBy.setString("personaID",            obj.createdByPersonaID);
    createdBy.setString("displayNameAtCreation", obj.createdByDisplayName);

    util::JsonDoc modifiedBy;
    modifiedBy.setString("identityID",                obj.modifiedByIdentityID);
    modifiedBy.setString("personaID",                 obj.modifiedByPersonaID);
    modifiedBy.setString("displayNameAtModification", obj.modifiedByDisplayName);

    // tags array
    util::JsonDoc tagsDoc;
    // appendToArray expects sub-docs, but tags are strings — we'll embed them
    // as single-field docs and extract on parse. Instead, rely on raw string
    // embedding via a temporary JsonDoc per tag.
    // NOTE: JsonDoc has no setStringArray; we build via appendToArray with
    // single-value sub-docs is awkward. Instead we embed tags as a JSON
    // array by serializing a wrapper doc then extracting in parse.
    // Simpler: store tags as comma-separated in a single string field for v1,
    // or encode each tag as a JsonDoc with a "v" key. We use the "v" key
    // approach to stay within the existing JsonDoc API.
    util::JsonDoc root;
    root.setString("schema",     std::string(kCharacterSchema));
    root.setString("objectID",   obj.objectID.value);
    root.setString("slug",       obj.slug);
    root.setString("displayName", obj.displayName);
    root.setString("status",     obj.status);
    root.setString("createdAt",  obj.createdAt);
    root.setSubDoc("createdBy",  std::move(createdBy));
    root.setString("modifiedAt", obj.modifiedAt);
    root.setSubDoc("modifiedBy", std::move(modifiedBy));
    root.setString("notes",      obj.notes);

    for (const auto& tag : obj.tags) {
        util::JsonDoc tagDoc;
        tagDoc.setString("v", tag);
        root.appendToArray("tags", std::move(tagDoc));
    }

    for (const auto& [k, v] : obj.attributes) {
        util::JsonDoc attrDoc;
        attrDoc.setString("k", k);
        attrDoc.setString("v", v);
        root.appendToArray("attributes", std::move(attrDoc));
    }

    return root.dump();
}

// ---------------------------------------------------------------------------
// Parse
// ---------------------------------------------------------------------------

Result<CharacterObject> parseCharacter(std::string_view json)
{
    auto r = parseAndValidateSchema(json, kCharacterSchema);
    if (!r.ok()) return Result<CharacterObject>::failure(r.error());
    auto& doc = r.value();

    CharacterObject obj;
    obj.objectID.value = doc.getString("objectID");
    obj.slug           = doc.getString("slug");
    obj.displayName    = doc.getString("displayName");
    obj.status         = doc.getString("status");
    obj.createdAt      = doc.getString("createdAt");
    obj.modifiedAt     = doc.getString("modifiedAt");
    obj.notes          = doc.getString("notes");

    auto createdBy = doc.getSubDoc("createdBy");
    obj.createdByIdentityID   = createdBy.getString("identityID");
    obj.createdByPersonaID    = createdBy.getString("personaID");
    obj.createdByDisplayName  = createdBy.getString("displayNameAtCreation");

    auto modifiedBy = doc.getSubDoc("modifiedBy");
    obj.modifiedByIdentityID  = modifiedBy.getString("identityID");
    obj.modifiedByPersonaID   = modifiedBy.getString("personaID");
    obj.modifiedByDisplayName = modifiedBy.getString("displayNameAtModification");

    const auto tagCount = doc.arraySize("tags");
    obj.tags.reserve(tagCount);
    for (std::size_t i = 0; i < tagCount; ++i) {
        obj.tags.push_back(doc.arrayItem("tags", i).getString("v"));
    }

    const auto attrCount = doc.arraySize("attributes");
    for (std::size_t i = 0; i < attrCount; ++i) {
        auto attrDoc = doc.arrayItem("attributes", i);
        obj.attributes[attrDoc.getString("k")] = attrDoc.getString("v");
    }

    return Result<CharacterObject>::success(std::move(obj));
}

} // namespace scrivi::schemas
