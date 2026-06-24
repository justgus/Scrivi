import Foundation

// OpenSessionManifest — persists the set of currently-open projectIDs so the app can
// restore all previously-open project windows on the next launch (EP-018 / T-0195, R4).
//
// Stored in UserDefaults as a plain [String] of projectIDs. Paths are NOT stored here —
// each projectID resolves to its on-disk path via ProjectBookmarkStore, the same
// mechanism that powers deep-link reopen. The manifest is rewritten whenever a project
// opens or closes, so it always reflects the live set.
enum OpenSessionManifest {

    private static let key = "scrivi.openSessionManifest.v1"

    // Records the full set of currently-open projectIDs (call after any open/close).
    static func save(projectIDs: [String]) {
        UserDefaults.standard.set(projectIDs, forKey: key)
    }

    // The projectIDs that were open at last save — the restore set for launch.
    static func load() -> [String] {
        UserDefaults.standard.stringArray(forKey: key) ?? []
    }

    static func clear() {
        UserDefaults.standard.removeObject(forKey: key)
    }
}
