import Foundation

// Parses and builds Scrivi deep links. Two ownership forms exist:
//
//   scrivi://open?project=<projectID>&item=<kind>:<id>   — project-owned records
//   scrivi://open?world=<worldID>&item=<kind>:<id>       — world-owned records
//
// This is the same URL the indexing facade emits as each record's deepLink
// (EP-017) and that Core Spotlight carries as relatedUniqueIdentifier.
//
// ⚠️ The `world=` form was added by I-0118 (ruled 2026-08-14). A worldbuilding
// object may be bound by several projects at once and outlives all of them, so
// there is no single owning project to name — a project-scoped link would have
// to pick one arbitrarily and would break when that project was deleted.
struct ScriviDeepLink: Equatable {
    /// Empty for a world-owned link.
    let projectID: String
    /// Empty for a project-owned link.
    let worldID: String
    let itemID: String     // "<kind>:<id>", e.g. "scene:scene_..." or "project:project_..."

    /// True when this link names a world rather than a project.
    var isWorldScoped: Bool { !worldID.isEmpty }

    // The scene/object id without the kind prefix, or nil for the project record.
    var targetSceneID: String? {
        guard itemID.hasPrefix("scene:") else { return nil }
        return String(itemID.dropFirst("scene:".count))
    }

    // Parses either form; returns nil for any other URL. A link naming neither a
    // project nor a world is rejected rather than half-resolved.
    init?(url: URL) {
        guard url.scheme == "scrivi",
              let comps = URLComponents(url: url, resolvingAgainstBaseURL: false),
              comps.host == "open" else { return nil }
        let items = comps.queryItems ?? []

        let project = items.first(where: { $0.name == "project" })?.value ?? ""
        let world   = items.first(where: { $0.name == "world" })?.value ?? ""
        guard !project.isEmpty || !world.isEmpty else { return nil }

        projectID = project
        worldID   = world
        itemID    = items.first(where: { $0.name == "item" })?.value ?? ""
    }

    init(projectID: String, itemID: String) {
        self.projectID = projectID
        self.worldID = ""
        self.itemID = itemID
    }

    init(worldID: String, itemID: String) {
        self.projectID = ""
        self.worldID = worldID
        self.itemID = itemID
    }
}
