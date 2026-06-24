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

            let item = CSSearchableItem(
                uniqueIdentifier: record.uniqueIdentifier,
                domainIdentifier: content.domainIdentifier,   // projectID — delete-by-domain key
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
    static func deleteProject(domainIdentifier: String) {
        guard !domainIdentifier.isEmpty else { return }
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
