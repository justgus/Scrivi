#include "ObjectJson.hpp"
#include "SchemaUtils.hpp"

namespace scrivi::schemas {

std::string objectSchemaTag(ObjectKind kind) {
    return "scrivi.object." + objectKindName(kind) + ".v1";
}

// ---------------------------------------------------------------------------
// Shared helpers
// ---------------------------------------------------------------------------

static std::string serializeFields(const WorldObjectFields& obj,
                                    std::string_view schemaTag)
{
    util::JsonDoc createdBy;
    createdBy.setString("identityID",            obj.createdByIdentityID);
    createdBy.setString("personaID",             obj.createdByPersonaID);
    createdBy.setString("displayNameAtCreation", obj.createdByDisplayName);

    util::JsonDoc modifiedBy;
    modifiedBy.setString("identityID",                 obj.modifiedByIdentityID);
    modifiedBy.setString("personaID",                  obj.modifiedByPersonaID);
    modifiedBy.setString("displayNameAtModification",  obj.modifiedByDisplayName);

    util::JsonDoc root;
    root.setString("schema",      std::string(schemaTag));
    root.setString("objectID",    obj.objectID.value);
    root.setString("slug",        obj.slug);
    root.setString("displayName", obj.displayName);
    root.setString("subtitle",    obj.subtitle);
    root.setString("status",      obj.status);
    root.setString("createdAt",   obj.createdAt);
    root.setSubDoc("createdBy",   std::move(createdBy));
    root.setString("modifiedAt",  obj.modifiedAt);
    root.setSubDoc("modifiedBy",  std::move(modifiedBy));
    root.setString("notes",       obj.notes);

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

    // Optional blocks are written only when populated, so an object that
    // carries neither round-trips byte-identically to the pre-SP-095 shape.
    if (!obj.image.empty()) {
        util::JsonDoc imageDoc;
        imageDoc.setString("assetID", obj.image.assetID);
        if (!obj.image.thumbnailAssetID.empty()) {
            imageDoc.setString("thumbnailAssetID", obj.image.thumbnailAssetID);
        }
        root.setSubDoc("image", std::move(imageDoc));
    }

    if (!obj.worldID.empty()) {
        root.setString("worldID", obj.worldID);
    }

    return root.dump();
}

static Result<WorldObjectFields> parseFields(std::string_view json,
                                              std::string_view expectedSchema)
{
    auto r = parseAndValidateSchema(json, expectedSchema);
    if (!r.ok()) { return Result<WorldObjectFields>::failure(r.error()); }
    auto& doc = r.value();

    WorldObjectFields obj;
    obj.objectID.value = doc.getString("objectID");
    obj.slug           = doc.getString("slug");
    obj.displayName    = doc.getString("displayName");
    obj.subtitle       = doc.getString("subtitle");
    obj.status         = doc.getString("status");
    obj.createdAt      = doc.getString("createdAt");
    obj.modifiedAt     = doc.getString("modifiedAt");
    obj.notes          = doc.getString("notes");

    auto createdBy = doc.getSubDoc("createdBy");
    obj.createdByIdentityID  = createdBy.getString("identityID");
    obj.createdByPersonaID   = createdBy.getString("personaID");
    obj.createdByDisplayName = createdBy.getString("displayNameAtCreation");

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

    // Absent optional keys default empty — this is what lets a legacy 5-kind
    // file written before SP-095 parse unchanged.
    auto imageDoc = doc.getSubDoc("image");
    obj.image.assetID          = imageDoc.getString("assetID");
    obj.image.thumbnailAssetID = imageDoc.getString("thumbnailAssetID");

    obj.worldID = doc.getString("worldID");

    return Result<WorldObjectFields>::success(std::move(obj));
}

// ---------------------------------------------------------------------------
// WorldObject variant
// ---------------------------------------------------------------------------

std::string serializeWorldObject(const WorldObject& obj) {
    return serializeFields(worldObjectFields(obj),
                           objectSchemaTag(worldObjectKind(obj)));
}

WorldObject makeWorldObject(ObjectKind kind, WorldObjectFields fields) {
    auto as = []<typename T>(WorldObjectFields f) -> WorldObject {
        T t;
        static_cast<WorldObjectFields&>(t) = std::move(f);
        return t;
    };

    switch (kind) {
        case ObjectKind::character: return as.template operator()<CharacterObject>(std::move(fields));
        case ObjectKind::location:  return as.template operator()<LocationObject>(std::move(fields));
        case ObjectKind::item:      return as.template operator()<ItemObject>(std::move(fields));
        case ObjectKind::building:  return as.template operator()<BuildingObject>(std::move(fields));
        case ObjectKind::vehicle:   return as.template operator()<VehicleObject>(std::move(fields));
        case ObjectKind::map:       return as.template operator()<MapObject>(std::move(fields));
        case ObjectKind::rule:      return as.template operator()<RuleObject>(std::move(fields));
        case ObjectKind::artifact:  return as.template operator()<ArtifactObject>(std::move(fields));
        case ObjectKind::chronicle: return as.template operator()<ChronicleObject>(std::move(fields));
        case ObjectKind::faction:   return as.template operator()<FactionObject>(std::move(fields));
        case ObjectKind::world:     return as.template operator()<WorldContainerObject>(std::move(fields));
    }
    return as.template operator()<CharacterObject>(std::move(fields));
}

Result<WorldObject> parseWorldObject(std::string_view json, ObjectKind kind) {
    auto r = parseFields(json, objectSchemaTag(kind));
    if (!r.ok()) { return Result<WorldObject>::failure(r.error()); }
    return Result<WorldObject>::success(makeWorldObject(kind, std::move(r.value())));
}

} // namespace scrivi::schemas
