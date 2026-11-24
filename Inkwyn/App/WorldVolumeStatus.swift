//
//  WorldVolumeStatus.swift
//  Scrivi
//
//  EP-031 AC24 (SP-102 / T-0389) — the platform-layer refinement of an unavailable
//  world's status.
//
//  ScriviCore emits only `missing` or `unavailable`: `WorldStatus::offline` and
//  `::unmounted` are declared in `WorldTypes.hpp:67-68` and produced nowhere, because
//  distinguishing them needs volume inspection that varies across Scrivi's seven
//  target platforms. Doc 3 §4.4.1 forbids a platform-specific *model*, so the
//  refinement lives here — in the Apple layer, feeding the SAME neutral enum. The
//  model does not learn a new state; only the diagnostic gets sharper.
//
//  ⚠️ WHY NOT `volumeIsRemovableKey` / `volumeIsEjectableKey`
//
//  Those are the obvious signals and they are WRONG here. Probed against the real
//  verification rig — `/Volumes/Scrivi Worlds`, a 931 GB USB external drive the user
//  physically unplugs:
//
//      volumeIsRemovable : false      ← on a drive that is unplugged by hand
//      volumeIsEjectable : false      ← diskutil agrees: "Removable Media: Fixed"
//      volumeIsLocal     : true
//
//  Keying `unmounted` off those flags would misreport the one world we actually
//  verify against. Worse, a `hdiutil` disk image — the fixture this was nearly tested
//  with — reports `ejectable == true`, so the fixture would have PASSED a rule that
//  fails on real hardware.
//
//  What works, and what this file uses, is **volume-root mount presence**: an
//  unmounted volume fails `exists` on both the package and its volume root, and
//  `resourceValues` fails outright. That separates the cases cleanly.
//
//  ⚠️ THE GOVERNING RULE (Doc 2 §7.2.1, and I-0115, which was this defect shipped):
//  a wrong `missing` is worse than an honest `unavailable`, because only one of them
//  invites the writer to restore from backup or clear references against a world that
//  is perfectly fine. Every inconclusive branch below returns `.unavailable`.
//

import Foundation

/// Refines an unavailable world's status from its package path.
///
/// Pure and side-effect free: it inspects the filesystem and returns a status. It
/// never writes, never mounts, and never modifies a binding.
enum WorldVolumeStatus {

    /// Refines the status ScriviCore reported for a world.
    ///
    /// - Parameters:
    ///   - coreStatus: what the core said. `available` is returned untouched — a
    ///     reachable world is not this function's business.
    ///   - packagePath: the world's `packagePath` from `scrivi_list_worlds`.
    /// - Returns: a more specific status, or `coreStatus` when nothing better can be
    ///   **established**. Never a guess.
    static func refine(coreStatus: WorldStatus, packagePath: String) -> WorldStatus {
        // A world that resolves needs no diagnosis.
        guard coreStatus.isUnavailable else { return coreStatus }
        guard !packagePath.isEmpty else { return coreStatus }

        // ⚠️ The core's `missing` is authoritative and must NOT be softened: it is
        // only ever reported on positive proof of absence — a readable parent plus a
        // definitively absent package, or a worldID mismatch (I-0115's fix). The one
        // refinement allowed on top of it is the unmounted-volume case below, which
        // is *more* specific and points at a different remedy.

        let fm = FileManager.default

        // Is the package on a mounted volume at all?
        if let volumeRoot = volumeRoot(for: packagePath) {
            if !fm.fileExists(atPath: volumeRoot.path) || !isMounted(volumeRoot) {
                // The volume itself is gone: insert or reconnect the drive. This is
                // NOT "missing" — the package is presumed intact on hardware we
                // cannot currently see, and telling the writer it is missing would
                // invite a restore she does not need.
                return .unmounted
            }

            // Volume is mounted. Is it a network volume we cannot reach?
            if let values = try? volumeRoot.resourceValues(forKeys: [.volumeIsLocalKey]),
               values.volumeIsLocal == false {
                // ⚠️ `volumeIsLocal` is the signal that actually describes `offline` —
                // unlike removable/ejectable, which describe something else entirely.
                return .offline
            }
        }

        // Package under /Volumes could not be attributed, or is on the boot volume:
        // whatever the core concluded stands. An honest generic beats a confident
        // wrong answer.
        return coreStatus
    }

    /// The `/Volumes/<name>` root a path sits under, or `nil` if it is not on a
    /// mounted external/secondary volume.
    ///
    /// Derived from the PATH rather than from `binding.json`'s `volumeLabel`, which is
    /// **empty on the real rig** — nothing may depend on that field.
    private static func volumeRoot(for path: String) -> URL? {
        let parts = (path as NSString).pathComponents
        // ["/", "Volumes", "<name>", …]
        guard parts.count > 2, parts[1] == "Volumes" else { return nil }
        return URL(fileURLWithPath: "/Volumes/\(parts[2])")
    }

    /// Whether a volume root is currently mounted.
    ///
    /// Checked against the live mount table rather than existence alone: a stale
    /// `/Volumes/<name>` directory can outlive an unclean unmount, and treating that
    /// leftover as "mounted" would report `missing` for a drive that is merely
    /// unplugged — the exact wrong answer.
    private static func isMounted(_ volumeRoot: URL) -> Bool {
        guard let mounted = FileManager.default.mountedVolumeURLs(
            includingResourceValuesForKeys: nil, options: []) else {
            // Cannot enumerate: fall back to existence rather than claiming absence.
            return FileManager.default.fileExists(atPath: volumeRoot.path)
        }
        let target = volumeRoot.standardizedFileURL.path
        return mounted.contains { $0.standardizedFileURL.path == target }
    }
}
