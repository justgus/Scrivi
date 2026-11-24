import Foundation

// OpenProjectRegistry — the authoritative record of which projects are currently open
// (EP-018 / T-0193).
//
// Keyed by projectID → the live ProjectSession for that project. This is the
// *authoritative* guard for R3 non-reentrancy: callers MUST check `session(for:)` and
// focus/reuse the existing session before opening a new one. The T-0191 spike showed
// macOS 26 WindowGroup(for:) de-dup is NOT race-safe (two rapid same-value opens both
// create windows), so we never rely on native de-dup against a race — restore-all
// (T-0195) and deep links (T-0196) open windows concurrently. Native WindowGroup(for:)
// de-dup remains only a steady-state backstop.
//
// Until T-0194 the app is single-window, so at most one session is registered at a time;
// the registry is still the single source of truth for "is this project open?". T-0194
// makes sessions per-window and the registry spans all open windows.
@Observable @MainActor final class OpenProjectRegistry {

    // projectID → live session. Insertion order is not significant.
    private(set) var sessions: [String: ProjectSession] = [:]

    var isEmpty: Bool { sessions.isEmpty }

    var openProjectIDs: [String] { Array(sessions.keys) }

    // The live session for a projectID, if that project is currently open.
    func session(for projectID: String) -> ProjectSession? {
        sessions[projectID]
    }

    // True if a project is already open (R3 check).
    func isOpen(projectID: String) -> Bool {
        sessions[projectID] != nil
    }

    // Registers a freshly-opened session under its projectID. The session must already
    // be loaded (it carries its projectID via openProjectResult).
    func register(_ session: ProjectSession) {
        guard let projectID = session.openProjectResult?.projectID else { return }
        sessions[projectID] = session
    }

    // Removes a session from the registry by projectID. Safe if absent.
    func deregister(projectID: String) {
        sessions.removeValue(forKey: projectID)
    }

    // Removes a session by identity (used on close when the caller holds the session but
    // not necessarily a valid projectID, e.g. after teardown).
    func deregister(_ session: ProjectSession) {
        if let projectID = session.openProjectResult?.projectID {
            sessions.removeValue(forKey: projectID)
            return
        }
        // Fall back to identity match if the projectID is already cleared.
        if let key = sessions.first(where: { $0.value === session })?.key {
            sessions.removeValue(forKey: key)
        }
    }
}
