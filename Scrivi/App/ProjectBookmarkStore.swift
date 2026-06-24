import Foundation
import os

// Maps a projectID → a security-scoped bookmark of its on-disk .scrivi path,
// so a deep link (scrivi://open?project=<projectID>) can reopen a project the
// user previously granted access to — even under the App Sandbox, and even on a
// cold launch (EP-017 T-0184).
//
// Why bookmarks: the sandbox only grants access to a path the user explicitly
// selected (via NSOpenPanel). A security-scoped bookmark persists that grant so
// a later launch can re-acquire access without a fresh panel. A deep link for a
// project that was never opened in this app cannot be resolved — that is an
// inherent sandbox limitation, surfaced to the caller as a nil resolve.
//
// Bookmarks are stored in UserDefaults keyed by projectID. The stored payload is
// the bookmark Data; resolution returns the URL plus whether access was started
// (the caller must balance startAccessingSecurityScopedResource with a stop).
enum ProjectBookmarkStore {

    private static let key = "scrivi.projectBookmarks.v1"
    private static let log = Logger(subsystem: "com.caposoft.scrivi", category: "Bookmarks")

    // Records (or refreshes) the bookmark for a project the user just opened.
    // `url` is the directory the user picked; `projectID` is the opened project's id.
    static func record(projectID: String, url: URL) {
        guard !projectID.isEmpty else { return }
        #if os(macOS)
        do {
            let data = try url.bookmarkData(
                options: .withSecurityScope,
                includingResourceValuesForKeys: nil,
                relativeTo: nil
            )
            var map = bookmarks()
            map[projectID] = data
            persist(map)
            log.notice("recorded bookmark for \(projectID, privacy: .public)")
        } catch {
            log.error("failed to record bookmark for \(projectID, privacy: .public): \(String(describing: error), privacy: .public)")
        }
        #endif
    }

    // Resolves a projectID to its path, re-acquiring security-scoped access.
    // Returns nil when no bookmark exists (project never opened here) or it can't
    // be resolved. On success the caller owns the access scope and MUST call
    // url.stopAccessingSecurityScopedResource() when done with the project.
    static func resolve(projectID: String) -> (url: URL, didStartAccess: Bool)? {
        guard !projectID.isEmpty, let data = bookmarks()[projectID] else { return nil }
        #if os(macOS)
        var isStale = false
        do {
            let url = try URL(
                resolvingBookmarkData: data,
                options: .withSecurityScope,
                relativeTo: nil,
                bookmarkDataIsStale: &isStale
            )
            let started = url.startAccessingSecurityScopedResource()
            if isStale {
                // Refresh the stored bookmark while we still hold access.
                record(projectID: projectID, url: url)
            }
            log.notice("resolved bookmark for \(projectID, privacy: .public) (started=\(started, privacy: .public), stale=\(isStale, privacy: .public))")
            return (url, started)
        } catch {
            log.error("failed to resolve bookmark for \(projectID, privacy: .public): \(String(describing: error), privacy: .public)")
            return nil
        }
        #else
        return nil
        #endif
    }

    static func hasBookmark(projectID: String) -> Bool {
        !projectID.isEmpty && bookmarks()[projectID] != nil
    }

    // MARK: — storage

    private static func bookmarks() -> [String: Data] {
        guard let raw = UserDefaults.standard.dictionary(forKey: key) as? [String: Data] else { return [:] }
        return raw
    }

    private static func persist(_ map: [String: Data]) {
        UserDefaults.standard.set(map, forKey: key)
    }
}
