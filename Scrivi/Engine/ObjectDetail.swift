import Foundation

/// A typed read of one worldbuilding object, and a **patching** writer for it
/// (EP-034 SP-117, T-0436 + T-0437).
///
/// ## Why this type exists
///
/// `ScriviEngine.openObject` returns `objectJson: String` — the app has had **no
/// typed object model at all**. `subtitle` and `notes` have existed in the core
/// since SP-095 (`ObjectTypes.hpp:209,222`) and were surfaced **nowhere**, which
/// is `project_capability_without_surface` at its largest scale in EP-034: the
/// whole object-editing capability shipped and was Verified while the app showed
/// a single `TextField("Name")`.
///
/// ## ⚠️ The rule that governs saving
///
/// **Patch the object's own JSON. Never reconstruct it.**
///
/// `ObjectCard.rename()` already works this way and says why: *"so every field
/// this build does not know about survives the round trip."* Reconstructing an
/// object from a Swift struct would silently drop `image`, `thumbnailAssetID`,
/// `attributes`, and anything a later core version adds — and the loss would be
/// invisible until a writer noticed her portrait gone.
///
/// ⚠️ This type therefore **decodes for display only**. `applying(...)` mutates a
/// parsed JSON dictionary and hands back JSON; the struct is never serialized.
struct ObjectDetail: Sendable, Equatable {

    // Identity — never edited here.
    let objectID: String
    let kind: String
    let slug: String
    let worldID: String

    // Editable in the Detail Sheet (T-0437).
    var displayName: String
    var subtitle: String
    var notes: String

    /// ⚠️ Read-only in SP-117. `tags` is a string array with no chip-editor
    /// precedent anywhere in the app, so editing is deferred to **SP-119** by
    /// user ruling — which means **R2 and AC2 do NOT close in SP-117**. Decoded
    /// here so the deferral is a UI decision, not a data gap.
    let tags: [String]

    /// Present when the object has an image. ⚠️ Never written by this sprint —
    /// it exists so a save can be *verified* not to have dropped it, and so
    /// SP-119 has something to build on.
    let imageAssetID: String

    // Authorship, for display.
    let modifiedAt: String

    /// The exact bytes `openObject` returned, kept so a save can patch them.
    /// ⚠️ **This is the field that makes preservation possible** — without the
    /// original document there is nothing to patch, and a save would have to
    /// reconstruct.
    let sourceJson: String

    // MARK: — Decoding

    enum DecodeError: Error, LocalizedError {
        case notAnObject
        var errorDescription: String? { "The object's data could not be read." }
    }

    /// Decodes for DISPLAY. Missing fields become empty strings rather than
    /// errors: an object written by an older core legitimately has no
    /// `subtitle`, and refusing to open it would be worse than showing a blank.
    init(json: String, kind: String) throws {
        guard let root = try JSONSerialization.jsonObject(
            with: Data(json.utf8)) as? [String: Any] else {
            throw DecodeError.notAnObject
        }

        self.sourceJson  = json
        self.kind        = kind
        self.objectID    = root["objectID"]    as? String ?? ""
        self.slug        = root["slug"]        as? String ?? ""
        self.worldID     = root["worldID"]     as? String ?? ""
        self.displayName = root["displayName"] as? String ?? ""
        self.subtitle    = root["subtitle"]    as? String ?? ""
        self.notes       = root["notes"]       as? String ?? ""
        self.modifiedAt  = root["modifiedAt"]  as? String ?? ""

        // ⚠️ Tags serialize as `[{"v": "..."}]`, not `["..."]`
        // (`ObjectJson.cpp:40-44`). Reading them as a plain string array yields
        // an empty list and looks like "this object has no tags".
        if let raw = root["tags"] as? [[String: Any]] {
            self.tags = raw.compactMap { $0["v"] as? String }
        } else {
            self.tags = []
        }

        // `image` is an optional SUB-OBJECT, written only when populated
        // (`ObjectJson.cpp:55-62`).
        if let image = root["image"] as? [String: Any] {
            self.imageAssetID = image["assetID"] as? String ?? ""
        } else {
            self.imageAssetID = ""
        }
    }

    // MARK: — Saving, by patch

    /// Returns `sourceJson` with only the edited fields replaced.
    ///
    /// ⚠️ **Every key not named here is carried through untouched** — that is the
    /// entire point. `image`, `attributes`, `createdBy`, `schema` and any field a
    /// future core adds survive, because this never builds a document from
    /// scratch.
    ///
    /// ⚠️ It also does not write `modifiedAt`/`modifiedBy`: **ScriviCore stamps
    /// those on save** (`ObjectStore::save`). Writing them here would either be
    /// overwritten or, worse, disagree with the core's clock.
    func applyingEdits(displayName newName: String,
                       subtitle newSubtitle: String,
                       notes newNotes: String,
                       tags newTags: [String]? = nil) throws -> String {
        guard var root = try JSONSerialization.jsonObject(
            with: Data(sourceJson.utf8)) as? [String: Any] else {
            throw DecodeError.notAnObject
        }

        root["displayName"] = newName.trimmingCharacters(in: .whitespacesAndNewlines)
        root["subtitle"]    = newSubtitle
        root["notes"]       = newNotes

        // ⚠️ T-0449: tags round-trip as `[{"v": "..."}]`, NOT `["..."]`
        // (`ObjectJson.cpp:40-44`). Writing a plain string array would parse back
        // as an EMPTY list — every tag silently dropped on the next read, with
        // the object file looking perfectly reasonable to a human.
        //
        // ⚠️ `nil` means "not edited" and leaves the key untouched, so a caller
        // that does not handle tags cannot erase them.
        if let newTags {
            let cleaned = newTags
                .map { $0.trimmingCharacters(in: .whitespacesAndNewlines) }
                .filter { !$0.isEmpty }
            if cleaned.isEmpty {
                // Absent rather than an empty array — matches how the core writes
                // an object that has never had tags.
                root.removeValue(forKey: "tags")
            } else {
                root["tags"] = cleaned.map { ["v": $0] }
            }
        }

        let data = try JSONSerialization.data(withJSONObject: root)
        return String(decoding: data, as: UTF8.self)
    }
}
