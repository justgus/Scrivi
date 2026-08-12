#include "objects/EndpointResolver.hpp"

#include "manuscript/ManuscriptOrderResolver.hpp"
#include "objects/ObjectIndex.hpp"

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

    // 2. Scenes — the EP-027 filesystem-authoritative identity path. Scenes are
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

    // 3. Neither. Not an error — the caller decides what an unresolved endpoint
    //    means in its context (create refuses; list reports what it has).
    return out;
}

} // namespace scrivi::objects
