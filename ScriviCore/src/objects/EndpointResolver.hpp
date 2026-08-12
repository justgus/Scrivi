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
// SP-097's pending-vs-dangling distinction (the Epic's highest-risk task) hangs
// off this one resolver rather than a prefix test scattered across call sites.

#include "scrivi/ObjectTypes.hpp"
#include "scrivi/Result.hpp"
#include "scrivi/Services.hpp"

#include <string>

namespace scrivi::objects {

struct ResolvedEndpoint {
    bool        found   = false;
    bool        isScene = false;
    ObjectKind  kind    = ObjectKind::character;  // meaningful only when !isScene
    std::string displayName;                      // object displayName or scene title
    std::string slug;                             // objects only
};

class EndpointResolver {
public:
    explicit EndpointResolver(CoreServices& services);

    // Never fails on an unknown ID — returns {found = false} so callers can
    // distinguish "no such endpoint" from an I/O error. SP-097 will extend the
    // unresolved case to separate *pending* (world unavailable) from *dangling*
    // (genuinely gone); SP-096 treats both as simply unresolved.
    [[nodiscard]] ResolvedEndpoint resolve(const AbsolutePath& projectRoot,
                                           const std::string& endpointID) const;

private:
    CoreServices& services_;
};

} // namespace scrivi::objects
