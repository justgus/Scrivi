//
//  ImportExtension.swift
//  ScriviSpotlightImporter
//
//  Spotlight importer for `.scrivi` project packages (EP-017 Layer 2 / SP-046).
//
//  The OS launches this extension to index a `.scrivi` package on disk even when Scrivi is
//  not running. Per the architecture boundary (Option A), the extension does NOT parse the
//  package's JSON in Swift — it calls the same ScriviCore facade the in-app donor uses
//  (`scrivi_extract_searchable_text`) and emits Spotlight attributes from that result.
//
//  CSImportExtension indexes the package as a SINGLE searchable item. The facade returns many
//  records (project + each scene + each object); we fold their text into one attribute set so a
//  search for any scene/object content surfaces the project. Per-item deep-linking is the in-app
//  CSSearchableIndex donor's job (Layer 1); this on-disk layer makes the content findable.
//

import CoreSpotlight
import UniformTypeIdentifiers
import ScriviCore

class ImportExtension: CSImportExtension {

    override func update(_ attributes: CSSearchableItemAttributeSet, forFileAt url: URL) throws {
        // Ask ScriviCore for the indexable content of this .scrivi package. The C function
        // returns a malloc'd JSON C string owned by us; free it unconditionally after copying.
        let json: String = try url.path.withCString { cPath -> String in
            guard let raw = scrivi_extract_searchable_text(cPath) else {
                throw ImporterError.facadeReturnedNull
            }
            defer { scrivi_free(raw) }
            return String(cString: raw)
        }

        guard let data = json.data(using: .utf8) else {
            throw ImporterError.invalidEncoding
        }

        let envelope = try JSONDecoder().decode(Envelope.self, from: data)
        guard envelope.ok, let result = envelope.result else {
            throw ImporterError.extractionFailed(envelope.error?.message ?? "unknown error")
        }

        // The project record (kind == "project") supplies the headline attributes; everything
        // else contributes searchable body text. Fall back gracefully if it's absent.
        let project = result.items.first { $0.kind == "project" }

        attributes.contentType = UTType.folder.identifier
        attributes.displayName = project?.displayName.nonEmpty
            ?? project?.title.nonEmpty
            ?? url.deletingPathExtension().lastPathComponent

        // Fold every record's title + description into one searchable text blob so a query for
        // any scene/character/location/etc. content matches this package.
        var parts: [String] = []
        for item in result.items {
            if let title = item.title.nonEmpty ?? item.displayName.nonEmpty {
                parts.append(title)
            }
            if !item.contentDescription.isEmpty {
                parts.append(item.contentDescription)
            }
        }
        let body = parts.joined(separator: "\n")
        if !body.isEmpty { attributes.textContent = body }

        // Union of all keywords across records (deduped, order-stable).
        var seen = Set<String>()
        let keywords = result.items.flatMap(\.keywords).filter { seen.insert($0).inserted }
        if !keywords.isEmpty { attributes.keywords = keywords }
    }

    // MARK: - Errors

    private enum ImporterError: LocalizedError {
        case facadeReturnedNull
        case invalidEncoding
        case extractionFailed(String)

        var errorDescription: String? {
            switch self {
            case .facadeReturnedNull:      return "ScriviCore returned no searchable content."
            case .invalidEncoding:         return "Searchable content was not valid UTF-8."
            case .extractionFailed(let m): return "Searchable-content extraction failed: \(m)."
            }
        }
    }

    // MARK: - Facade envelope (mirrors scrivi.searchableContent.v1, nested under `result`)
    //
    // This decodes the *facade's* JSON envelope — NOT the .scrivi package's own JSON — so the
    // Option A boundary holds (the package is read only by ScriviCore). Shapes match
    // ScriviEngine.SearchableContentResult / SearchableItemResult in the app.

    private struct Envelope: Decodable {
        let ok: Bool
        let result: SearchableContent?
        let error: APIError?
    }

    private struct APIError: Decodable {
        let code: Int?
        let message: String?
    }

    private struct SearchableContent: Decodable {
        let domainIdentifier: String
        let items: [SearchableItem]

        enum CodingKeys: String, CodingKey { case domainIdentifier, items }

        init(from decoder: Decoder) throws {
            let c = try decoder.container(keyedBy: CodingKeys.self)
            domainIdentifier = try c.decodeIfPresent(String.self, forKey: .domainIdentifier) ?? ""
            items = try c.decodeIfPresent([SearchableItem].self, forKey: .items) ?? []
        }
    }

    private struct SearchableItem: Decodable {
        let kind: String
        let title: String
        let displayName: String
        let contentDescription: String
        let keywords: [String]

        enum CodingKeys: String, CodingKey {
            case kind, title, displayName, contentDescription, keywords
        }

        init(from decoder: Decoder) throws {
            let c = try decoder.container(keyedBy: CodingKeys.self)
            kind               = try c.decodeIfPresent(String.self, forKey: .kind) ?? ""
            title              = try c.decodeIfPresent(String.self, forKey: .title) ?? ""
            displayName        = try c.decodeIfPresent(String.self, forKey: .displayName) ?? ""
            contentDescription = try c.decodeIfPresent(String.self, forKey: .contentDescription) ?? ""
            keywords           = try c.decodeIfPresent([String].self, forKey: .keywords) ?? []
        }
    }
}

private extension String {
    /// nil when empty/whitespace-only, otherwise self — lets us chain `??` fallbacks.
    var nonEmpty: String? {
        trimmingCharacters(in: .whitespacesAndNewlines).isEmpty ? nil : self
    }
}
