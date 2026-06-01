#pragma once

#include "scrivi/IDs.hpp"
#include "scrivi/Types.hpp"

#include <cstdint>
#include <map>
#include <string>
#include <variant>
#include <vector>

namespace scrivi {

// -----------------------------------------------------------------------
// ObjectKind — discriminant for all world-object types
// -----------------------------------------------------------------------

enum class ObjectKind : std::uint8_t {
    character,
    location,
    item,
    rule,
    timeline,
};

// Returns the subdirectory name for a given kind (e.g. "characters").
inline std::string objectKindSubdir(ObjectKind kind) {
    switch (kind) {
        case ObjectKind::character: return "characters";
        case ObjectKind::location:  return "locations";
        case ObjectKind::item:      return "items";
        case ObjectKind::rule:      return "rules";
        case ObjectKind::timeline:  return "timelines";
    }
    return "characters";
}

// -----------------------------------------------------------------------
// Shared field block — every world object has the same shape
// -----------------------------------------------------------------------

struct WorldObjectFields {
    ObjectID         objectID;
    Slug             slug;
    std::string      displayName;
    std::string      status;          // "active" | "archived"

    ISO8601Timestamp createdAt;
    std::string      createdByIdentityID;
    std::string      createdByPersonaID;
    std::string      createdByDisplayName;

    ISO8601Timestamp modifiedAt;
    std::string      modifiedByIdentityID;
    std::string      modifiedByPersonaID;
    std::string      modifiedByDisplayName;

    std::string      notes;
    std::vector<std::string>           tags;
    std::map<std::string, std::string> attributes;  // freeform key/value pairs
};

// -----------------------------------------------------------------------
// Concrete object types — all identical in v1; typed wrappers allow
// distinct schema tags and future divergence.
// -----------------------------------------------------------------------

struct CharacterObject : WorldObjectFields {};
struct LocationObject  : WorldObjectFields {};
struct ItemObject      : WorldObjectFields {};
struct RuleObject      : WorldObjectFields {};
struct TimelineObject  : WorldObjectFields {};

// -----------------------------------------------------------------------
// WorldObject variant — used in OpenObjectResult and SaveObjectRequest
// -----------------------------------------------------------------------

using WorldObject = std::variant<
    CharacterObject,
    LocationObject,
    ItemObject,
    RuleObject,
    TimelineObject
>;

// Retrieve the shared fields from any WorldObject variant.
inline const WorldObjectFields& worldObjectFields(const WorldObject& obj) {
    return std::visit([]<typename T>(const T& o) -> const WorldObjectFields& {
        return static_cast<const WorldObjectFields&>(o);
    }, obj);
}

} // namespace scrivi
