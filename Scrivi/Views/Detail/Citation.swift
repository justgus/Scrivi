import Foundation

/// The citation field vocabulary — the ONE place SP-120 names the keys it stores
/// in an object's `attributes` map (EP-034 SP-120, T-0457; S11 §5.2).
///
/// ## ⚠️ Why a shared type rather than string literals at each site
///
/// Three surfaces read or write these keys: the citation editor on the documented
/// object, `CitationPopover`, and (through the popup) the aggregate `sources`
/// card. ⚠️ **A key spelled `"accessed"` in one and `"accessedDate"` in another
/// would store the writer's typing where nothing reads it** — the field would be
/// written, round-tripped, and invisible.
///
/// That is the restated-list defect this project has paid for repeatedly, in the
/// one form the standing `ObjectKind` rule does not cover: these keys are not an
/// enum the core owns, so **there is nothing to derive them from.** The next best
/// thing is to state them exactly once.
///
/// ## ⚠️ Why `attributes` and not typed fields on the core
///
/// A `source` object carries `displayName`, `subtitle`, `notes`, `tags` and a
/// freeform `attributes` map — and nothing else (`ObjectTypes.hpp:208-224`).
/// `attributes` already crosses the ABI and already round-trips
/// (`ObjectJson.cpp:46-50`), so citation data needs **no schema change, no ABI
/// change and no version decision** — which is what keeps SP-120 `[Apple]`-only
/// (user ruling, 2026-08-24).
enum CitationField: String, CaseIterable, Sendable {
    case author
    case year
    case publisher
    case page
    case url
    case accessed

    /// The label the writer sees.
    var label: String {
        switch self {
        case .author:    "Author"
        case .year:      "Year"
        case .publisher: "Publisher"
        case .page:      "Page"
        case .url:       "URL"
        case .accessed:  "Accessed"
        }
    }

    /// Placeholder text. ⚠️ `year` and `accessed` are deliberately FREE TEXT, not
    /// dates: "c. 1387", "n.d." and "forthcoming" are all real citation years and
    /// a date picker would refuse every one of them (S11 §5.2).
    var placeholder: String {
        switch self {
        case .author:    "Ursula K. Le Guin"
        case .year:      "1968 — or “n.d.”, “c. 1387”"
        case .publisher: "Parnassus Press"
        case .page:      "pp. 12–18"
        case .url:       "https://…"
        case .accessed:  "12 March 2026"
        }
    }

    /// The `attributes` key. Explicit rather than implied by `rawValue` so a label
    /// change can never silently repoint stored data.
    var storageKey: String {
        switch self {
        case .author:    "author"
        case .year:      "year"
        case .publisher: "publisher"
        case .page:      "page"
        case .url:       "url"
        case .accessed:  "accessed"
        }
    }
}

/// A source's citation fields, in display order, skipping the empty ones.
///
/// ⚠️ **An unfilled citation field is ABSENT, not present-and-blank** — so a book
/// with only an author does not render five empty rows.
struct CitationFields: Sendable, Equatable {
    /// Ordered `(label, value)` pairs, empties already dropped.
    let entries: [(label: String, value: String)]

    var isEmpty: Bool { entries.isEmpty }

    init(attributes: [String: String]) {
        entries = CitationField.allCases.compactMap { field in
            guard let raw = attributes[field.storageKey] else { return nil }
            let value = raw.trimmingCharacters(in: .whitespacesAndNewlines)
            return value.isEmpty ? nil : (label: field.label, value: value)
        }
    }

    static func == (a: CitationFields, b: CitationFields) -> Bool {
        a.entries.count == b.entries.count
        && zip(a.entries, b.entries).allSatisfy { $0.label == $1.label && $0.value == $1.value }
    }
}
