import Foundation
import os

// Maps a worldID → a security-scoped bookmark of its .scrivworld package, so a
// world stays readable across launches under the App Sandbox (SP-104).
//
// ⚠️ WHY THIS EXISTS. A world is DESIGNED to live outside the .scrivi package —
// it is a movable, shareable package that several projects may bind. The project
// bookmark therefore never covers it. Without a bookmark of its own, the sandbox
// grant a world got from its NSSavePanel/NSOpenPanel died with the process, and
// the next launch could not read it at all.
//
// The consequence was not subtle: ScriviCore could not read the package, and
// reported the world as `missing` — so a perfectly intact world looked deleted.
// Since T-0409 made every worldbuilding kind world-scoped (only `source` is
// project-scoped), an unreadable world means characters, locations and items
// cannot be created or listed. That is why this is a blocker, not a nicety.
//
// Difference from ProjectBookmarkStore: a project's access is scoped to its
// window, but a world may be referenced by ANY open project, so access is
// acquired once and deliberately held for the process lifetime — there is no
// balanced stop. `stopAll` exists for teardown/tests.
enum WorldBookmarkStore {

    private static let key = "scrivi.worldBookmarks.v1"
    private static let log = Logger(subsystem: "com.caposoft.scrivi", category: "WorldBookmarks")

    // URLs whose access scope this process currently holds, keyed by worldID, so
    // a repeated resolve does not start a second scope for the same world.
    nonisolated(unsafe) private static var active: [String: URL] = [:]
    private static let lock = NSLock()

    // Records the bookmark for a world the user just created or bound. `url` must
    // be the package URL the user actually picked in a panel — that is the grant
    // the bookmark captures.
    static func record(worldID: String, url: URL) {
        guard !worldID.isEmpty else { return }
        #if os(macOS)
        do {
            let data = try url.bookmarkData(
                options: .withSecurityScope,
                includingResourceValuesForKeys: nil,
                relativeTo: nil
            )
            lock.lock()
            var map = bookmarks()
            map[worldID] = data
            persist(map)
            lock.unlock()
            log.notice("recorded world bookmark for \(worldID, privacy: .public)")
        } catch {
            log.error("failed to record world bookmark for \(worldID, privacy: .public): \(String(describing: error), privacy: .public)")
        }
        #endif
    }

    // Re-acquires access to a single world. Returns true when this process holds
    // access afterwards (including when it already did).
    @discardableResult
    static func activate(worldID: String) -> Bool {
        guard !worldID.isEmpty else { return false }
        #if os(macOS)
        lock.lock()
        if active[worldID] != nil {
            lock.unlock()
            return true
        }
        let data = bookmarks()[worldID]
        lock.unlock()

        guard let data else { return false }

        var isStale = false
        do {
            let url = try URL(
                resolvingBookmarkData: data,
                options: .withSecurityScope,
                relativeTo: nil,
                bookmarkDataIsStale: &isStale
            )
            let started = url.startAccessingSecurityScopedResource()
            if started {
                lock.lock()
                active[worldID] = url
                lock.unlock()
            }
            // Refresh while access is held — a moved world would otherwise decay
            // into an unresolvable bookmark and read as missing again.
            if isStale {
                record(worldID: worldID, url: url)
            }
            log.notice("activated world \(worldID, privacy: .public) (started=\(started, privacy: .public), stale=\(isStale, privacy: .public))")
            return started
        } catch {
            log.error("failed to resolve world bookmark for \(worldID, privacy: .public): \(String(describing: error), privacy: .public)")
            return false
        }
        #else
        return false
        #endif
    }

    // Re-acquires access to every world this project binds. Called BEFORE the
    // project's worlds are first resolved, so ScriviCore's very first read
    // already sees a readable package and reports `available` rather than
    // `missing`.
    //
    // Best-effort by design: a world with no bookmark (bound on another machine,
    // or before this store existed) simply stays unavailable, which the Worlds
    // manager surfaces honestly with a "Locate…" repair.
    static func activateAll(worldIDs: [String]) {
        for id in worldIDs { activate(worldID: id) }
    }

    static func hasBookmark(worldID: String) -> Bool {
        lock.lock()
        defer { lock.unlock() }
        return !worldID.isEmpty && bookmarks()[worldID] != nil
    }

    // Drops a world's stored grant. Called when the project's reference is
    // removed so a stale grant does not linger for a world nothing binds.
    static func forget(worldID: String) {
        guard !worldID.isEmpty else { return }
        lock.lock()
        if let url = active.removeValue(forKey: worldID) {
            url.stopAccessingSecurityScopedResource()
        }
        var map = bookmarks()
        map[worldID] = nil
        persist(map)
        lock.unlock()
    }

    // Releases every held scope. For teardown and tests.
    static func stopAll() {
        lock.lock()
        for (_, url) in active { url.stopAccessingSecurityScopedResource() }
        active.removeAll()
        lock.unlock()
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
