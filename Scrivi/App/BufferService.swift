import Foundation

// BufferService — the Swift-side driver for the multiple copy buffers (EP-019,
// SP-056, T-0214). One instance per open project, owned by ProjectSession.
//
// Model (user ruling 2026-07-25, refining design §9):
//   Ten buffers, 0–9. Buffer 0 IS the system pasteboard — ordinary ⌘C/⌘V, never
//   touched here. Buffers 1–9 are the ScriviCore-persisted slots (history/buffers.json,
//   scrivi.buffers.v1), mirrored in memory for display. The writer's existing ⌘C/⌘V
//   muscle memory is unchanged; ⌘1–9 are a single-chord, context-sensitive extension:
//     • ⌘N with a selection → copy the selection into slot N (copy-only; no text
//       change ⇒ NO history event, Trade T3).
//     • ⌘N with no selection → paste slot N at the caret (an ordinary paste event;
//       empty slot ⇒ silent no-op).
//     • ⇧⌘N with a selection → cut the selection into slot N (mutates text ⇒ a `cut`
//       history event, bufferID-tagged, Trade T3).
//
// This type owns NO text mutation: the editor coordinator performs the actual
// insert/remove against the NSTextView so it flows through the existing history
// capture + auto-save path. BufferService only talks to ScriviCore (load/get/clear)
// and holds the display mirror. Persistence lives entirely in C++ (BufferStore).
//
// Concurrency: @MainActor — created and used entirely on the main actor (owned by the
// @MainActor ProjectSession, driven by the @MainActor text coordinator and the
// SwiftUI palette). engine.buffers* calls are synchronous and thread-safe in C++.
@Observable @MainActor
final class BufferService {

    private let engine: ScriviEngine
    private let projectRootPath: String

    // The v1 persistent slot IDs. Buffer 0 (the system pasteboard) is deliberately
    // excluded — it is not a ScriviCore slot and never enters buffers.json.
    static let slotIDs = ["1", "2", "3", "4", "5", "6", "7", "8", "9"]

    // In-memory mirror of the non-empty persistent slots, ascending by bufferID.
    // Refreshed from ScriviCore after every load/clear so the palette stays live.
    private(set) var slots: [BufferSlot] = []

    // Editor-action bridges installed by the text coordinator (mirroring the loader's
    // takeFocusHandler pattern). The palette and Edit menu call these so the actual
    // text mutation runs on the NSTextView through the history + auto-save path. nil
    // until an editor coordinator is live (e.g. before the manuscript view appears).
    var pasteFromBufferHandler: ((_ bufferID: String) -> Void)?
    var loadSelectionHandler:   ((_ bufferID: String) -> Void)?
    var cutIntoBufferHandler:   ((_ bufferID: String) -> Void)?

    init(engine: ScriviEngine, projectRootPath: String) {
        self.engine = engine
        self.projectRootPath = projectRootPath
        refresh()
    }

    // MARK: — Display mirror

    // Reloads the slot mirror from ScriviCore. Best-effort — a read failure leaves
    // the previous mirror in place (the buffers are a convenience, never blocking).
    func refresh() {
        guard let listed = try? engine.buffersList(projectRootPath: projectRootPath) else { return }
        slots = listed.buffers
    }

    // The text currently in slot `bufferID` ("1"–"9"), or nil if the slot is empty.
    // Reads through the mirror so the palette and paste path agree.
    func text(inSlot bufferID: String) -> String? {
        slots.first(where: { $0.bufferID == bufferID })?.text
    }

    // The structured fragment in slot `bufferID`, or nil when the slot is empty or holds
    // only plain text (T-0355 / AC4). A non-nil result drives structured paste reconstruction.
    func fragment(inSlot bufferID: String) -> FragmentResult? {
        slots.first(where: { $0.bufferID == bufferID })?.fragment
    }

    // A short single-line preview for slot `bufferID`, or nil when empty. Used by the
    // palette rows.
    func preview(forSlot bufferID: String) -> String? {
        text(inSlot: bufferID).map { Self.previewLine($0) }
    }

    // Collapses `text` to a single trimmed line for display in the palette.
    static func previewLine(_ text: String) -> String {
        let firstLine = text
            .components(separatedBy: .newlines)
            .first { !$0.trimmingCharacters(in: .whitespaces).isEmpty } ?? ""
        return firstLine.trimmingCharacters(in: .whitespaces)
    }

    // MARK: — Load (copy-into-buffer)

    // Loads `text` into slot `bufferID` (create-or-replace) and refreshes the mirror.
    // The caller decides whether the text is a copy (no history event) or a cut (the
    // caller records the `cut` event separately, tagged with this bufferID). Returns
    // false on a store failure. No-op for empty text is the caller's call — an empty
    // selection never reaches here.
    @discardableResult
    func load(_ text: String, intoSlot bufferID: String, fragmentJSON: String = "") -> Bool {
        // fragmentJSON (a serialized scrivi.fragment.v1) is optional — a cross-boundary
        // copy/cut passes it so the slot can later reconstruct structure (T-0355 / AC4);
        // a single-scene copy passes "" and the slot is plain text (AC5). A load always
        // replaces both, so re-copying plain text into a structured slot clears the fragment.
        do {
            try engine.buffersLoad(projectRootPath: projectRootPath, bufferID: bufferID,
                                   text: text, fragmentJSON: fragmentJSON)
            refresh()
            return true
        } catch {
            print("[Scrivi] buffersLoad failed: \(error)")
            return false
        }
    }

    // MARK: — Clear

    // Clears slot `bufferID` and refreshes the mirror. Returns whether a slot was removed.
    @discardableResult
    func clear(slot bufferID: String) -> Bool {
        do {
            let r = try engine.buffersClear(projectRootPath: projectRootPath, bufferID: bufferID)
            refresh()
            return r.cleared
        } catch {
            print("[Scrivi] buffersClear failed: \(error)")
            return false
        }
    }
}
