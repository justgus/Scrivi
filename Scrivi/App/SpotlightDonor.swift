import Foundation
import os

#if canImport(CoreSpotlight) && os(macOS)
import CoreSpotlight
import UniformTypeIdentifiers

private let spotlightLog = Logger(subsystem: "com.caposoft.scrivi", category: "Spotlight")

// Donates a project's indexable content to Core Spotlight (Layer 1, EP-017).
//
// The single source of indexing truth is ScriviCore's facade
// (`scrivi_extract_searchable_text`, via ScriviEngine) — this type only maps the
// returned records to CSSearchableItems and talks to CSSearchableIndex. It owns
// no project I/O.
//
// Granularity is open/save/close (per design §8 — no per-keystroke reindex):
//   - donate(...)            on project open and on save
//   - deleteProject(...)     on project close/remove, keyed by domainIdentifier
//
// All operations are best-effort: failures are logged, never thrown, so indexing
// can never block or crash the writing flow.
enum SpotlightDonor {

    // Re-donates every searchable item for a project. Indexing the same
    // uniqueIdentifier again updates the existing entry, so this doubles as the
    // "update on save" path.
    static func donate(_ content: SearchableContentResult) {
        guard !content.items.isEmpty else { return }

        let items = content.items.map { record -> CSSearchableItem in
            let attrs = CSSearchableItemAttributeSet(contentType: contentType(for: record.kind))
            attrs.title = record.title.isEmpty ? record.displayName : record.title
            attrs.displayName = record.displayName.isEmpty ? record.title : record.displayName
            if !record.contentDescription.isEmpty {
                attrs.contentDescription = record.contentDescription
            }
            if !record.keywords.isEmpty {
                attrs.keywords = record.keywords
            }
            if !record.deepLink.isEmpty {
                attrs.relatedUniqueIdentifier = record.deepLink
            }

            // ⚠️ I-0118: an item's domain is its OWN when it names one — a world
            // item carries "world_<worldID>" — and the project's otherwise.
            // Donating a world's objects under the project domain would mean
            // closing one project deletes a shared world's entries out from
            // under every other project that binds it.
            let domain = record.domainIdentifier.isEmpty
                ? content.domainIdentifier
                : record.domainIdentifier

            let item = CSSearchableItem(
                uniqueIdentifier: record.uniqueIdentifier,
                domainIdentifier: domain,
                attributeSet: attrs
            )
            return item
        }

        // Capture only the count (a Sendable Int), not the non-Sendable
        // [CSSearchableItem] array, inside the @Sendable completion handler.
        let itemCount = items.count
        spotlightLog.notice("donate: indexing \(itemCount, privacy: .public) items, domain=\(content.domainIdentifier, privacy: .public)")
        // NOTE: this completion handler reporting error==nil does NOT guarantee the
        // donation reached the index — the underlying async CSInlineDonation can still
        // fail later (observed: SetStoreUpdateService -4099 on dev-signed sandboxed
        // builds; see Task-verified-0182). Treat this as "accepted for processing".
        CSSearchableIndex.default().indexSearchableItems(items) { error in
            if let error {
                spotlightLog.error("donate FAILED (\(itemCount, privacy: .public) items): \(error.localizedDescription, privacy: .public)")
            } else {
                spotlightLog.notice("donate accepted: \(itemCount, privacy: .public) items submitted")
            }
        }
    }

    // Removes every item for a project by its domain identifier (the projectID).
    //
    // ⚠️ **PROJECT DOMAINS ONLY — never a world's** (I-0118 Q1, user-ruled
    // 2026-08-14). A world outlives every project that binds it and may be bound
    // by several at once, so its entries must survive a project closing, being
    // deleted, or unbinding the world. They are removed *only* on explicit
    // instruction — an affordance that does not exist yet and is deferred to
    // EP-033 (in-app view vs. dedicated world-management app).
    //
    // The guard below is not defensive noise: this is called on every project
    // close, so a world domain reaching it would silently destroy shared search
    // data with no user action and no way to notice.
    static func deleteProject(domainIdentifier: String) {
        guard !domainIdentifier.isEmpty else { return }
        guard !domainIdentifier.hasPrefix("world_") else {
            spotlightLog.error(
                "REFUSED to delete a WORLD domain via deleteProject: \(domainIdentifier, privacy: .public) — worlds are never deleted as a side effect (I-0118)")
            return
        }
        spotlightLog.notice("delete: domain=\(domainIdentifier, privacy: .public)")
        CSSearchableIndex.default()
            .deleteSearchableItems(withDomainIdentifiers: [domainIdentifier]) { error in
                if let error {
                    spotlightLog.error("delete FAILED: \(error.localizedDescription, privacy: .public)")
                } else {
                    spotlightLog.notice("delete OK: domain=\(domainIdentifier, privacy: .public)")
                }
            }
    }

    // Maps a record kind to the Spotlight content type. Scenes are text;
    // worldbuilding objects and the project use generic content/item types.
    private static func contentType(for kind: String) -> UTType {
        switch kind {
        case "scene":   return .text
        case "project": return .folder
        default:        return .content
        }
    }
}

#else

// Non-macOS / no CoreSpotlight: donations are a no-op so call sites need no guards.
enum SpotlightDonor {
    static func donate(_ content: SearchableContentResult) {}
    static func deleteProject(domainIdentifier: String) {}
}

#endif
