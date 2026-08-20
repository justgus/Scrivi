#pragma once

// World types (EP-031 SP-097). Design: World Data Separation v0.1 §6.1–§6.5.
//
// A world is a SEPARATE `.scrivworld` package referenced by a project (W6=A),
// not a folder inside `.scrivi`. Only `binding.json` is project-local. The
// package is self-contained — it carries its own object index and assets, so it
// can be moved, copied, or opened by another project with nothing left behind.

#include "scrivi/Types.hpp"

#include <cstdint>
#include <string>
#include <vector>

namespace scrivi::worlds {

// world.json — the world's own identity and intrinsic epoch (Doc 1 §7.0 layer 2).
struct WorldRecord {
    std::string worldID;        // stable identity; the ONLY durable way to name this world
    std::string displayName;
    std::string epochLabel;     // intrinsic; travels with the world when shared
    std::string createdAt;
    std::string modifiedAt;
    int         formatVersion = 1;

    // ⚠️ The highest world.json formatVersion this build understands (T-0420,
    // I-0136). A package declaring a HIGHER version is refused rather than
    // parsed as if current — see parseWorld.
    //
    // Forward compatibility is the one property a shared, sync-carried package
    // format cannot retrofit: by the time a newer file exists in the wild, the
    // old readers that silently mis-parsed it have already shipped. A world
    // package is shared between projects and carried across machines, which is
    // exactly where version skew occurs.
    //
    // ⚠️ RAISE THIS ONLY when this build can actually READ the newer shape.
    static constexpr int kSupportedFormatVersion = 1;
};

// One cached world-object entry, mirrored into the project's binding (§6.3) so a
// pending edge can be shown BY NAME while the world is unavailable.
struct CachedWorldObject {
    std::string objectID;
    std::string kind;
    std::string displayName;
};

// Where a world was last seen. Every path is a HINT — `worldID` is the identity
// (§6.4). Deliberately platform-neutral (§4.4.1): no security-scoped bookmarks,
// no SAF handles. A platform layer may hold access tokens per device, keyed by
// worldID, but they are never part of this reference.
struct WorldReference {
    std::string lastKnownPath;           // RELATIVE to the project package where possible
    std::string lastKnownAbsolutePath;
    std::string volumeLabel;             // hint only; never authoritative
};

// worlds/<worldID>/binding.json — THIS project's translation of the world.
struct WorldBindingRecord {
    std::string    worldID;
    std::string    displayName;          // cached for display while unavailable
    std::int64_t   epochOffsetMs = 0;    // world epoch → project timeline (Doc 1 §7.0 layer 3)
    WorldReference reference;
    std::vector<CachedWorldObject> cachedIndex;
};

// Availability (§4.6). The PENDING BEHAVIOUR IS IDENTICAL in every unavailable
// state; only the reported status differs, so the writer knows what to do.
//
// ⚠️ `missing` is reported ONLY when positively established — the containing
// folder is present and readable and the package is absent. Everything else
// falls back to `unavailable`. Doc 3: "a wrong 'missing' is materially worse
// than an honest 'unavailable'", because only `missing` invites destructive
// remedies (clearing references, restoring from backup) when the world may be
// perfectly intact on an unreachable volume. NEVER GUESS.
enum class WorldStatus {
    available,
    missing,        // container present + readable, package absent — positively established
    offline,        // remote/network volume unreachable   (platform-layer refinement)
    unmounted,      // volume not mounted                  (platform-layer refinement)
    unavailable,    // cause undetermined — the honest default
};

inline std::string worldStatusName(WorldStatus s) {
    switch (s) {
        case WorldStatus::available:   return "available";
        case WorldStatus::missing:     return "missing";
        case WorldStatus::offline:     return "offline";
        case WorldStatus::unmounted:   return "unmounted";
        case WorldStatus::unavailable: return "unavailable";
    }
    return "unavailable";
}

} // namespace scrivi::worlds
