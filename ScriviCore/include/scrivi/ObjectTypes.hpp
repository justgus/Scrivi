#pragma once

#include "scrivi/IDs.hpp"
#include "scrivi/Types.hpp"

#include <cstdint>
#include <map>
#include <optional>
#include <string>
#include <string_view>
#include <variant>
#include <vector>

namespace scrivi {

// -----------------------------------------------------------------------
// ObjectKind — discriminant for all world-object types
//
// Kinds are split by OWNERSHIP (Worldbuilding Object Model v0.2 §3):
//
//   PROJECT-scoped — live under objects/<subdir>/ in the .scrivi package.
//   WORLD-scoped   — live under a world package (worlds/<worldID>/<subdir>/).
//                    Declared here so the enum and the object index know the
//                    complete kind set, but NOT creatable until EP-031 SP-098
//                    supplies world packages. See objectKindIsWorldScoped.
//
// ObjectKind::timeline was retired in SP-095 (v0.2 §3.2, T4=A) — the Timeline
// Panel owns story-time, not this enum. NOTE: retiring the KIND did not retire
// the objects/timelines/ DIRECTORY, which still holds the project timeline's
// timeline.meta.json (see ProjectCreator / kTimelineMetaPath).
// -----------------------------------------------------------------------

enum class ObjectKind : std::uint8_t {
    // Project-scoped
    character,
    location,
    item,
    building,
    vehicle,
    map,

    // World-scoped (declared; gated until SP-098)
    rule,
    artifact,
    chronicle,
    faction,

    // Container kind — created by scrivi_create_world (SP-098), never by
    // scrivi_create_object.
    world,
};

// Returns the subdirectory name for a given kind (e.g. "characters").
// For world-scoped kinds this is the name WITHIN the world package; it is
// never joined to objects/ (ObjectStore rejects those kinds first).
inline std::string objectKindSubdir(ObjectKind kind) {
    switch (kind) {
        case ObjectKind::character: return "characters";
        case ObjectKind::location:  return "locations";
        case ObjectKind::item:      return "items";
        case ObjectKind::building:  return "buildings";
        case ObjectKind::vehicle:   return "vehicles";
        case ObjectKind::map:       return "maps";
        // TODO(SP-098): `rule` still resolves under objects/rules/ because that
        // is where it ships today. World Data Separation v0.1 §7.2 relocates it
        // to worlds/<worldID>/rules/ together with the world package that will
        // hold it; Package Structure v0.1 §11 is corrected in the same step.
        case ObjectKind::rule:      return "rules";
        case ObjectKind::artifact:  return "artifacts";
        case ObjectKind::chronicle: return "chronicles";
        case ObjectKind::faction:   return "factions";
        case ObjectKind::world:     return "worlds";
    }
    return "characters";
}

// Singular kind name — the boundary spelling used by the C ABI, the object
// index, and Spotlight identifiers.
inline std::string objectKindName(ObjectKind kind) {
    switch (kind) {
        case ObjectKind::character: return "character";
        case ObjectKind::location:  return "location";
        case ObjectKind::item:      return "item";
        case ObjectKind::building:  return "building";
        case ObjectKind::vehicle:   return "vehicle";
        case ObjectKind::map:       return "map";
        case ObjectKind::rule:      return "rule";
        case ObjectKind::artifact:  return "artifact";
        case ObjectKind::chronicle: return "chronicle";
        case ObjectKind::faction:   return "faction";
        case ObjectKind::world:     return "world";
    }
    return "character";
}

// Parses a singular kind name (the boundary spelling). Returns nullopt for
// anything unrecognised — callers must never silently substitute a default.
inline std::optional<ObjectKind> objectKindFromName(std::string_view name) {
    for (auto k : {ObjectKind::character, ObjectKind::location,  ObjectKind::item,
                   ObjectKind::building,  ObjectKind::vehicle,   ObjectKind::map,
                   ObjectKind::rule,      ObjectKind::artifact,  ObjectKind::chronicle,
                   ObjectKind::faction,   ObjectKind::world}) {
        if (objectKindName(k) == name) { return k; }
    }
    return std::nullopt;
}

// True for kinds that belong to a WORLD rather than the project.
//
// `rule` is deliberately NOT listed: it ships project-scoped at objects/rules/
// and stays there until SP-098 moves it with the rest of the world package.
// Reporting it as world-scoped now would break every existing rule object.
inline bool objectKindIsWorldScoped(ObjectKind kind) {
    switch (kind) {
        case ObjectKind::artifact:
        case ObjectKind::chronicle:
        case ObjectKind::faction:
            return true;
        default:
            return false;
    }
}

// -----------------------------------------------------------------------
// ObjectImageRef — an object's image, stored BY REFERENCE into assets/.
// Never inline bytes (v0.2 §4.1): object files stay small and Git-friendly,
// and imports reuse scrivi_import_asset.
// -----------------------------------------------------------------------

struct ObjectImageRef {
    std::string assetID;
    std::string thumbnailAssetID;   // optional

    [[nodiscard]] bool empty() const {
        return assetID.empty() && thumbnailAssetID.empty();
    }
};

// -----------------------------------------------------------------------
// Shared field block — every world object has the same shape
// -----------------------------------------------------------------------

struct WorldObjectFields {
    ObjectID         objectID;
    Slug             slug;
    std::string      displayName;
    std::string      subtitle;        // one-line descriptor
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

    ObjectImageRef   image;
    std::string      worldID;         // empty for project-scoped objects
};

// -----------------------------------------------------------------------
// Concrete object types — all identical in v1; typed wrappers allow
// distinct schema tags and future divergence.
// -----------------------------------------------------------------------

struct CharacterObject : WorldObjectFields {};
struct LocationObject  : WorldObjectFields {};
struct ItemObject      : WorldObjectFields {};
struct BuildingObject  : WorldObjectFields {};
struct VehicleObject   : WorldObjectFields {};
struct MapObject       : WorldObjectFields {};
struct RuleObject      : WorldObjectFields {};
struct ArtifactObject  : WorldObjectFields {};
struct ChronicleObject : WorldObjectFields {};
struct FactionObject   : WorldObjectFields {};
struct WorldContainerObject : WorldObjectFields {};

// -----------------------------------------------------------------------
// WorldObject variant — used in OpenObjectResult and SaveObjectRequest
// -----------------------------------------------------------------------

using WorldObject = std::variant<
    CharacterObject,
    LocationObject,
    ItemObject,
    BuildingObject,
    VehicleObject,
    MapObject,
    RuleObject,
    ArtifactObject,
    ChronicleObject,
    FactionObject,
    WorldContainerObject
>;

// Retrieve the shared fields from any WorldObject variant.
inline const WorldObjectFields& worldObjectFields(const WorldObject& obj) {
    return std::visit([]<typename T>(const T& o) -> const WorldObjectFields& {
        return static_cast<const WorldObjectFields&>(o);
    }, obj);
}

// The ObjectKind matching a WorldObject's active alternative.
//
// Written as an exhaustive if-constexpr chain with NO positional fallback:
// every alternative names itself. The pre-SP-095 version used `timeline` as a
// terminal `else`, so removing that alternative would have silently mis-typed
// whichever kind happened to land last.
inline ObjectKind worldObjectKind(const WorldObject& obj) {
    return std::visit([]<typename T>(const T&) -> ObjectKind {
        using U = std::decay_t<T>;
        if constexpr (std::is_same_v<U, CharacterObject>)      { return ObjectKind::character; }
        else if constexpr (std::is_same_v<U, LocationObject>)  { return ObjectKind::location; }
        else if constexpr (std::is_same_v<U, ItemObject>)      { return ObjectKind::item; }
        else if constexpr (std::is_same_v<U, BuildingObject>)  { return ObjectKind::building; }
        else if constexpr (std::is_same_v<U, VehicleObject>)   { return ObjectKind::vehicle; }
        else if constexpr (std::is_same_v<U, MapObject>)       { return ObjectKind::map; }
        else if constexpr (std::is_same_v<U, RuleObject>)      { return ObjectKind::rule; }
        else if constexpr (std::is_same_v<U, ArtifactObject>)  { return ObjectKind::artifact; }
        else if constexpr (std::is_same_v<U, ChronicleObject>) { return ObjectKind::chronicle; }
        else if constexpr (std::is_same_v<U, FactionObject>)   { return ObjectKind::faction; }
        else {
            static_assert(std::is_same_v<U, WorldContainerObject>,
                          "worldObjectKind: unhandled WorldObject alternative");
            return ObjectKind::world;
        }
    }, obj);
}

} // namespace scrivi
