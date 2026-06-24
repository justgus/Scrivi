import Foundation

// Parses and builds Scrivi deep links: scrivi://open?project=<projectID>&item=<kind>:<id>
// This is the same URL the indexing facade emits as each record's deepLink
// (EP-017) and that Core Spotlight carries as relatedUniqueIdentifier.
struct ScriviDeepLink: Equatable {
    let projectID: String
    let itemID: String     // "<kind>:<id>", e.g. "scene:scene_..." or "project:project_..."

    // The scene/object id without the kind prefix, or nil for the project record.
    var targetSceneID: String? {
        guard itemID.hasPrefix("scene:") else { return nil }
        return String(itemID.dropFirst("scene:".count))
    }

    // Parses scrivi://open?project=…&item=… ; returns nil for any other URL.
    init?(url: URL) {
        guard url.scheme == "scrivi",
              let comps = URLComponents(url: url, resolvingAgainstBaseURL: false),
              comps.host == "open" else { return nil }
        let items = comps.queryItems ?? []
        guard let project = items.first(where: { $0.name == "project" })?.value,
              !project.isEmpty else { return nil }
        projectID = project
        itemID = items.first(where: { $0.name == "item" })?.value ?? ""
    }

    init(projectID: String, itemID: String) {
        self.projectID = projectID
        self.itemID = itemID
    }
}
