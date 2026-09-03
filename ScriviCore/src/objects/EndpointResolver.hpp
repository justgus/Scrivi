#pragma once

// EndpointResolver — answers "what is this edge endpoint?" (EP-031, SP-096, T-0402).
//
// ⚠️ WHY THIS EXISTS. Worldbuilding Object Model v0.2 §5.2 originally said the
// loader distinguishes scene endpoints from object endpoints "by ID prefix
// (`scene_…`)". That rule is BROKEN and was replaced at SP-096 planning:
//
//   • SystemUUIDProvider::newObjectID() mints "character_<uuid>" for EVERY object
//     kind, so a `location`'s ID literally begins `character_`. The prefix names
//     the generator, not the kind.
//   • DeterministicUUIDProvider (tests) mints "obj-1" / "scene-1" — a different
//     separator AND a different stem, so tests would take a different branch from
//     production.
//
// Both failure modes are SILENT: an edge would resolve against the wrong
// subsystem and report "not found" rather than erroring. So endpoint kind is
// resolved by LOOKUP, never by string inspection:
//
//   1. objects/index.json  — authoritative for ID→kind since SP-095 (§4.2), which
//                            is exactly why promotion cannot stale an endpoint (§3.1)
//   2. the EP-027 scene identity path
//   3. neither → unresolved
//
// SP-098's pending-vs-dangling distinction (the Epic's highest-risk task) hangs
// off this one resolver rather than a prefix test scattered across call sites.

#include "scrivi/ObjectTypes.hpp"
#include "scrivi/Result.hpp"
#include "scrivi/Services.hpp"
#include "worlds/WorldTypes.hpp"

#include <string>

namespace scrivi::objects {

// ⚠️ THREE STATES, NOT TWO (SP-098 T-0380; Doc 1 §5.5, Doc 3 §4.6).
//
//   found                       — resolved; the endpoint is right there
//   !found && pending           — its WORLD is unavailable; unknowable, NOT gone
//   !found && !pending          — dangling; the world (or project) is present
//                                 and the endpoint genuinely is not
//
// Collapsing the middle case into the third is the one failure in EP-031 that is
// silent AND unrecoverable: a prune pass reading "world unmounted" as "object
// deleted" destroys every relationship into that world, errors nothing, and may
// go unnoticed for weeks. Callers must therefore branch on `pending` explicitly
// and must never infer state from `found` alone.
struct ResolvedEndpoint {
    bool        found   = false;
    bool        isScene = false;
    ObjectKind  kind    = ObjectKind::character;  // meaningful only when !isScene
    std::string displayName;                      // object displayName or scene title
    std::string slug;                             // objects only

    // --- pending (T-0380) ----------------------------------------------------
    // Set when the endpoint is NAMED BY an unavailable world's cached index.
    // `displayName` is populated from that cache in this case (AC-A7) — the
    // whole reason the cache exists: a writer asked whether to clear references
    // cannot decide blind.
    bool                worldPending = false;
    // The world holding this object, when it is world-scoped. ⚠️ Set for BOTH a
    // reachable world object and a pending one (I-0142) — it answers "which
    // world does this belong to", which is a fact about the object, not about
    // whether its world happens to be reachable right now. Empty for a
    // project-scoped object or a scene endpoint.
    std::string         worldID;
    worlds::WorldStatus worldStatus = worlds::WorldStatus::available;

    // ⚠️ I-0183 — set when a bound world reported itself AVAILABLE but its object
    // set could NOT be positively established (the index was unreadable and the
    // directory scan found nothing). ⚠️ An empty scan is INDISTINGUISHABLE from
    // "this world has no objects", so it is NOT evidence of absence.
    //
    // ⚠️ This is the hole that made `available` unsafe. The three states below
    // assumed a reachable world always yields a readable object set — true for a
    // local disk, FALSE over a network mount whose small files (`world.json`)
    // still read from cache while larger ones (`index.json`) fail EBADF. The
    // world answered "available", the object set came back empty, and every edge
    // into it was classified `dangling` and tombstoned.
    bool worldIndeterminate = false;

    // True when this endpoint must be treated as "unknowable, hold" rather than
    // "gone, prune". The graph is frozen toward it in BOTH directions.
    // ⚠️ I-0183: an indeterminate world is held too — "I could not read it" and
    // "it is away" are the same instruction to a prune pass.
    [[nodiscard]] bool pending() const {
        return !found && (worldPending || worldIndeterminate);
    }

    // True only when the endpoint is positively absent: nothing resolved it, no
    // unavailable world claims it, ⚠️ AND every bound world's object set was
    // actually READ. This — and only this — licenses a prune.
    [[nodiscard]] bool dangling() const {
        return !found && !worldPending && !worldIndeterminate;
    }
};

class EndpointResolver {
public:
    explicit EndpointResolver(CoreServices& services);

    // Never fails on an unknown ID — returns {found = false} so callers can
    // distinguish "no such endpoint" from an I/O error, and `pending()` /
    // `dangling()` to distinguish the two ways of not resolving.
    [[nodiscard]] ResolvedEndpoint resolve(const AbsolutePath& projectRoot,
                                           const std::string& endpointID) const;

private:
    CoreServices& services_;
};

} // namespace scrivi::objects
