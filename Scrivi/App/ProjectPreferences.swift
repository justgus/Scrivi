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

    // True when this project had no persisted preferences yet (first open on this machine).
    // ProjectSession uses it to decide whether to seed the display title from project.json.
    let hadNoStoredPreferences: Bool

    init(projectID: String) {
        self.projectID = projectID
        let key = Self.key(for: projectID)
        if let data = defaults.data(forKey: key),
           let stored = try? JSONDecoder().decode(Stored.self, from: data) {
            showChapterTitles = stored.showChapterTitles
            projectTitle      = stored.projectTitle
            projectSubtitle   = stored.projectSubtitle
            hadNoStoredPreferences = false
        } else {
            showChapterTitles = false
            projectTitle      = ""
            projectSubtitle   = ""
            hadNoStoredPreferences = true
        }
    }

    // Seeds the display title from the on-disk project.json title (I-0093) when the writer has
    // not already set one on this machine — so a project shows its real name instead of "Untitled".
    // Only applies when there were no stored preferences and the incoming title is non-empty; an
    // explicit later rename in Project Settings (persisted to UserDefaults) always wins.
    func seedTitleFromSchemaIfUnset(_ schemaTitle: String) {
        guard hadNoStoredPreferences,
              projectTitle.trimmingCharacters(in: .whitespaces).isEmpty,
              !schemaTitle.trimmingCharacters(in: .whitespaces).isEmpty else { return }
        projectTitle = schemaTitle
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
