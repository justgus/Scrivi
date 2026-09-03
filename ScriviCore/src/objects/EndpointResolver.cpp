#include "objects/EndpointResolver.hpp"

#include "manuscript/ManuscriptOrderResolver.hpp"
#include "objects/ObjectIndex.hpp"
#include "worlds/WorldStore.hpp"

namespace scrivi::objects {

EndpointResolver::EndpointResolver(CoreServices& services)
    : services_(services) {}

ResolvedEndpoint EndpointResolver::resolve(const AbsolutePath& projectRoot,
                                            const std::string& endpointID) const {
    ResolvedEndpoint out;
    if (endpointID.empty()) { return out; }

    // 1. Objects — the index is authoritative for ID→kind (SP-095 §4.2).
    ObjectIndex index{services_};
    if (auto hit = index.find(projectRoot, ObjectID{endpointID}); hit.ok()) {
        const auto& e = hit.value();
        out.found       = true;
        out.isScene     = false;
        out.kind        = e.kind;
        out.displayName = e.displayName;
        out.slug        = e.slug;
        return out;
    }

    // 2. World objects — the object index spans BOTH partitions (Doc 1 §4.2,
    //    Doc 3 §6.3), so a cross-partition edge (project character ↔ world
    //    artifact) must resolve identically to a same-partition one (AC10).
    //    The live world index wins when the world is reachable; the binding's
    //    cache names the object when it is not, which is what lets a pending
    //    entry read "⟨Midgard: Sword of Dawn⟩" instead of a bare UUID.
    {
        worlds::WorldStore ws{services_};
        if (auto ids = ws.listBoundWorldIDs(projectRoot); ids.ok()) {
            for (const auto& worldID : ids.value()) {
                auto res = ws.resolve(projectRoot, worldID);
                if (res.status == worlds::WorldStatus::available) {
                    // ⚠️ I-0183 — "available" is NOT a promise the object set is
                    // readable. `resolve` establishes availability from
                    // `world.json`, a SMALL file that can still be served from a
                    // client cache while a LARGER `index.json` on the same mount
                    // fails EBADF. That asymmetry is not hypothetical: it
                    // destroyed 10 of 12 relationships in a real project
                    // (the-lone-golem, 2026-09-01) when a pulled SMB volume left
                    // the mount serving a phantom listing.
                    bool indeterminate = false;
                    if (auto entries = index.loadWorldIndex(res.packagePath, &indeterminate);
                        entries.ok() && !indeterminate) {
                        for (const auto& e : entries.value()) {
                            if (e.objectID.value == endpointID) {
                                out.found       = true;
                                out.kind        = e.kind;
                                out.displayName = e.displayName;
                                out.slug        = e.slug;
                                // ⚠️ I-0142: report the world for a REACHABLE
                                // world object too, not only a pending one. The
                                // field was previously set on the pending branch
                                // alone, so a healthy world object resolved with
                                // no world attributed — and the app could not
                                // show which world it belongs to.
                                out.worldID     = worldID;
                                return out;
                            }
                        }
                    }
                    // ⚠️ I-0183 — the world SAID available but its objects could
                    // not be read. Fall through to the cached index below: a
                    // cache hit yields `pending` (hold), and a miss yields
                    // `worldIndeterminate` (also hold). ⚠️ NEVER `dangling`,
                    // which would license a prune on evidence we do not have.
                    if (!indeterminate) { continue; }
                }
                // World unavailable, OR available-but-unreadable (I-0183) — fall
                // back to the cached names.
                if (auto b = ws.loadBinding(projectRoot, worldID); b.ok()) {
                    // ⚠️ I-0183: a world we could not READ cannot license a
                    // prune, whether or not its cache happens to name this
                    // endpoint. Mark it before the lookup so a cache MISS is held
                    // too — the cache is a display aid, never a census.
                    if (res.status == worlds::WorldStatus::available) {
                        out.worldIndeterminate = true;
                        out.worldID            = worldID;
                    }
                    for (const auto& e : b.value().cachedIndex) {
                        if (e.objectID == endpointID) {
                            // ⚠️ NOT `found` — and NOT dangling either. The object
                            // cannot be VERIFIED while its world is away, but an
                            // unavailable world naming it is positive evidence it
                            // exists. This is the *pending* state (T-0380): hold,
                            // never prune, never modify. The cached displayName
                            // travels with it so the writer sees a name rather
                            // than a bare UUID (AC-A7).
                            out.worldPending = true;
                            out.worldID      = worldID;
                            out.worldStatus  = res.status;
                            out.displayName  = e.displayName;
                            if (auto k = objectKindFromName(e.kind)) { out.kind = *k; }
                            return out;
                        }
                    }
                }
            }
        }
    }

    // 3. Scenes — the EP-027 filesystem-authoritative identity path. Scenes are
    //    never objects/ files (§8), so they are resolved by walking the
    //    manuscript rather than through the object index.
    manuscript::ManuscriptOrderResolver resolver{services_};
    if (auto scenes = resolver.resolve(projectRoot); scenes.ok()) {
        for (const auto& s : scenes.value()) {
            if (s.sceneID.value == endpointID) {
                out.found       = true;
                out.isScene     = true;
                out.displayName = s.title;
                return out;
            }
        }
    }

    // 4. Neither. Not an error — the caller decides what an unresolved endpoint
    //    means in its context (create refuses; list reports what it has).
    return out;
}

} // namespace scrivi::objects
