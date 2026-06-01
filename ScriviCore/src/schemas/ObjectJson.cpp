#include "ObjectJson.hpp"
#include "SchemaUtils.hpp"

namespace scrivi::schemas {

static constexpr std::string_view kCharacterSchema = "scrivi.object.character.v1";
static constexpr std::string_view kLocationSchema  = "scrivi.object.location.v1";
static constexpr std::string_view kItemSchema      = "scrivi.object.item.v1";
static constexpr std::string_view kRuleSchema      = "scrivi.object.rule.v1";
static constexpr std::string_view kTimelineSchema  = "scrivi.object.timeline.v1";

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

    return Result<WorldObjectFields>::success(std::move(obj));
}

// ---------------------------------------------------------------------------
// Character
// ---------------------------------------------------------------------------

std::string serializeCharacter(const CharacterObject& obj) {
    return serializeFields(obj, kCharacterSchema);
}

Result<CharacterObject> parseCharacter(std::string_view json) {
    auto r = parseFields(json, kCharacterSchema);
    if (!r.ok()) { return Result<CharacterObject>::failure(r.error()); }
    CharacterObject obj;
    static_cast<WorldObjectFields&>(obj) = std::move(r.value());
    return Result<CharacterObject>::success(std::move(obj));
}

// ---------------------------------------------------------------------------
// Location
// ---------------------------------------------------------------------------

std::string serializeLocation(const LocationObject& obj) {
    return serializeFields(obj, kLocationSchema);
}

Result<LocationObject> parseLocation(std::string_view json) {
    auto r = parseFields(json, kLocationSchema);
    if (!r.ok()) { return Result<LocationObject>::failure(r.error()); }
    LocationObject obj;
    static_cast<WorldObjectFields&>(obj) = std::move(r.value());
    return Result<LocationObject>::success(std::move(obj));
}

// ---------------------------------------------------------------------------
// Item
// ---------------------------------------------------------------------------

std::string serializeItem(const ItemObject& obj) {
    return serializeFields(obj, kItemSchema);
}

Result<ItemObject> parseItem(std::string_view json) {
    auto r = parseFields(json, kItemSchema);
    if (!r.ok()) { return Result<ItemObject>::failure(r.error()); }
    ItemObject obj;
    static_cast<WorldObjectFields&>(obj) = std::move(r.value());
    return Result<ItemObject>::success(std::move(obj));
}

// ---------------------------------------------------------------------------
// Rule
// ---------------------------------------------------------------------------

std::string serializeRule(const RuleObject& obj) {
    return serializeFields(obj, kRuleSchema);
}

Result<RuleObject> parseRule(std::string_view json) {
    auto r = parseFields(json, kRuleSchema);
    if (!r.ok()) { return Result<RuleObject>::failure(r.error()); }
    RuleObject obj;
    static_cast<WorldObjectFields&>(obj) = std::move(r.value());
    return Result<RuleObject>::success(std::move(obj));
}

// ---------------------------------------------------------------------------
// Timeline
// ---------------------------------------------------------------------------

std::string serializeTimeline(const TimelineObject& obj) {
    return serializeFields(obj, kTimelineSchema);
}

Result<TimelineObject> parseTimeline(std::string_view json) {
    auto r = parseFields(json, kTimelineSchema);
    if (!r.ok()) { return Result<TimelineObject>::failure(r.error()); }
    TimelineObject obj;
    static_cast<WorldObjectFields&>(obj) = std::move(r.value());
    return Result<TimelineObject>::success(std::move(obj));
}

// ---------------------------------------------------------------------------
// WorldObject variant helpers
// ---------------------------------------------------------------------------

std::string serializeWorldObject(const WorldObject& obj) {
    if (const auto* p = std::get_if<CharacterObject>(&obj)) { return serializeCharacter(*p); }
    if (const auto* p = std::get_if<LocationObject>(&obj))  { return serializeLocation(*p); }
    if (const auto* p = std::get_if<ItemObject>(&obj))      { return serializeItem(*p); }
    if (const auto* p = std::get_if<RuleObject>(&obj))      { return serializeRule(*p); }
    if (const auto* p = std::get_if<TimelineObject>(&obj))  { return serializeTimeline(*p); }
    return "";
}

Result<WorldObject> parseWorldObject(std::string_view json, ObjectKind kind) {
    switch (kind) {
        case ObjectKind::character: {
            auto r = parseCharacter(json);
            if (!r.ok()) { return Result<WorldObject>::failure(r.error()); }
            return Result<WorldObject>::success(std::move(r.value()));
        }
        case ObjectKind::location: {
            auto r = parseLocation(json);
            if (!r.ok()) { return Result<WorldObject>::failure(r.error()); }
            return Result<WorldObject>::success(std::move(r.value()));
        }
        case ObjectKind::item: {
            auto r = parseItem(json);
            if (!r.ok()) { return Result<WorldObject>::failure(r.error()); }
            return Result<WorldObject>::success(std::move(r.value()));
        }
        case ObjectKind::rule: {
            auto r = parseRule(json);
            if (!r.ok()) { return Result<WorldObject>::failure(r.error()); }
            return Result<WorldObject>::success(std::move(r.value()));
        }
        case ObjectKind::timeline: {
            auto r = parseTimeline(json);
            if (!r.ok()) { return Result<WorldObject>::failure(r.error()); }
            return Result<WorldObject>::success(std::move(r.value()));
        }
    }
    return Result<WorldObject>::failure({.code = ErrorCode::invalidArgument, .message = "unknown ObjectKind"});
}

} // namespace scrivi::schemas
