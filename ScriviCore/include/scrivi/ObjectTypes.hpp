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
//   WORLD-scoped   — live inside a `.scrivworld` package (SP-097), reached
//                    through the project's binding. Creatable since SP-098
//                    widened the C ABI with `worldID` (T-0405, I-0113); before
//                    that they were storable but unreachable at the boundary.
//                    See objectKindIsWorldScoped.
//
// ObjectKind::timeline was retired in SP-095 (v0.2 §3.2, T4=A) — the Timeline
// Panel owns story-time, not this enum. NOTE: retiring the KIND did not retire
// the objects/timelines/ DIRECTORY, which still holds the project timeline's
// timeline.meta.json (see ProjectCreator / kTimelineMetaPath).
// -----------------------------------------------------------------------

enum class ObjectKind : std::uint8_t {
    // ⚠️ World-scoped as of SP-103 (2026-08-14) — these were project-scoped
    // until the scope ruling in Doc 1 §3.0. They now live in the .scrivworld
    // package. See objectKindIsWorldScoped.
    character,
    location,
    item,
    building,
    vehicle,
    map,

    // Project-scoped — the ONLY one. A citation documents a real-world
    // publication supporting THIS manuscript, not the invented world.
    source,     // a cited work — book, paper, interview (SP-098 T-0406)

    // World-scoped (and always were)
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
// ⚠️ THE canonical list of storable kinds — every ObjectKind except the `world`
// container. Anything that needs to iterate kinds (index rebuilds, package
// scaffolding, scans) MUST use this rather than restating the set.
//
// Restated kind lists are this Epic's recurring defect: I-0113, SP-098's
// `source` table, SP-103's index scan table, and SP-104's world-package
// skeleton — four occurrences, each silently naming a stale partition. The
// standing rule "grep before adding a kind" was not enough, because a list can
// also go wrong when a kind's SCOPE changes and its own text stays untouched.
// Derive from this list and `objectKindIsWorldScoped()`; never restate either.
inline constexpr ObjectKind kAllStorableKinds[] = {
    ObjectKind::character,
    ObjectKind::location,
    ObjectKind::item,
    ObjectKind::building,
    ObjectKind::vehicle,
    ObjectKind::map,
    ObjectKind::source,
    ObjectKind::rule,
    ObjectKind::artifact,
    ObjectKind::chronicle,
    ObjectKind::faction,
};

inline std::string objectKindSubdir(ObjectKind kind) {
    switch (kind) {
        case ObjectKind::character: return "characters";
        case ObjectKind::location:  return "locations";
        case ObjectKind::item:      return "items";
        case ObjectKind::building:  return "buildings";
        case ObjectKind::vehicle:   return "vehicles";
        case ObjectKind::map:       return "maps";
        case ObjectKind::source:    return "sources";
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
        case ObjectKind::source:    return "source";
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
                   ObjectKind::source,    ObjectKind::rule,      ObjectKind::artifact,
                   ObjectKind::chronicle, ObjectKind::faction,   ObjectKind::world}) {
        if (objectKindName(k) == name) { return k; }
    }
    return std::nullopt;
}

// True for kinds that belong to a WORLD rather than the project.
//
// `rule` joined this set in SP-097 (T-0404): rules govern an ENVIRONMENT, not a
// manuscript, so they live at worlds/<worldID>/rules/ (Doc 1 §3, Doc 3 §7.2).
// No migration was written — Scrivi has not shipped, so there is no field data
// at objects/rules/; developer fixtures were regenerated instead.
// ⚠️ SP-103 / T-0409 (user ruling 2026-08-14, Doc 1 §3.0): EVERY worldbuilding
// kind is world-scoped. `source` is the ONLY project-scoped object kind.
//
// Why the list inverted: a writer asked why a character she might reuse in
// another project was stored in the PROJECT, noting she would have to "promote
// her to the world first." She could not — there was no world-scoped character
// kind, so promotion would have turned her into an `artifact`, a category error
// rather than reuse. Cross-project character reuse had NO supported path.
//
// The model now says: a worldbuilding object IS a thing in a world. A project is
// a MANUSCRIPT (scenes, chapters, structure, timeline) that REFERENCES world
// objects through the relationship graph. Reuse falls out of binding the same
// world — no promotion step, no duplication.
//
// `source` stays project-scoped on its own merits: a citation points at a
// REAL-WORLD publication, or is a footnote/authorial comment on the manuscript.
// It documents research supporting THIS manuscript, not a fact about the
// invented world — and keeping it here stops a shared world from dragging one
// project's bibliography into every project that binds it. `cites` edges still
// cross the partition (SP-097 made cross-partition resolution work).
//
// Written as an explicit enumeration rather than `kind != source` so that adding
// a kind forces a deliberate scope decision at this site.
inline bool objectKindIsWorldScoped(ObjectKind kind) {
    switch (kind) {
        case ObjectKind::artifact:
        case ObjectKind::building:
        case ObjectKind::character:
        case ObjectKind::chronicle:
        case ObjectKind::faction:
        case ObjectKind::item:
        case ObjectKind::location:
        case ObjectKind::map:
        case ObjectKind::rule:
        case ObjectKind::vehicle:
            return true;
        case ObjectKind::source:   // the sole project-scoped object kind
        case ObjectKind::world:    // container; created by scrivi_create_world
            return false;
    }
    return false;
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
struct SourceObject    : WorldObjectFields {};
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
    SourceObject,
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
        else if constexpr (std::is_same_v<U, SourceObject>)    { return ObjectKind::source; }
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
