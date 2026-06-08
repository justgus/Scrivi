import Foundation

// Per-project user preferences, persisted to UserDefaults keyed by projectID.
// Created by AppEnvironment when a project opens; cleared when it closes.
@Observable @MainActor final class ProjectPreferences {

    private let projectID: String
    private let defaults = UserDefaults.standard

    // Writing surface
    var showChapterTitles: Bool {
        didSet { save() }
    }

    // Project identity (display only — not written back to the C++ schema)
    var projectTitle: String {
        didSet { save() }
    }

    var projectSubtitle: String {
        didSet { save() }
    }

    init(projectID: String) {
        self.projectID = projectID
        let key = Self.key(for: projectID)
        if let data = defaults.data(forKey: key),
           let stored = try? JSONDecoder().decode(Stored.self, from: data) {
            showChapterTitles = stored.showChapterTitles
            projectTitle      = stored.projectTitle
            projectSubtitle   = stored.projectSubtitle
        } else {
            showChapterTitles = false
            projectTitle      = ""
            projectSubtitle   = ""
        }
    }

    private func save() {
        let stored = Stored(
            showChapterTitles: showChapterTitles,
            projectTitle: projectTitle,
            projectSubtitle: projectSubtitle
        )
        if let data = try? JSONEncoder().encode(stored) {
            defaults.set(data, forKey: Self.key(for: projectID))
        }
    }

    private static func key(for projectID: String) -> String {
        "scrivi.project.\(projectID).preferences"
    }

    private struct Stored: Codable {
        var showChapterTitles: Bool
        var projectTitle:      String
        var projectSubtitle:   String
    }
}
