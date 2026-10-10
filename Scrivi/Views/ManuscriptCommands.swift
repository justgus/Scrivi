import Foundation
#if os(macOS)
import AppKit

// EP-046 E2-S3 ([SP-163], T-0592 + T-0591) — THE FORMAT COMMANDS, as edits the presenter computes.
//
// ✅ Under the escape ruling (study §3A.0) these are the ONLY way formatting enters a manuscript. Each returns ONE
// edit (range + replacement) that the view applies as one history event, or nil to refuse.
// ✅ [SP-174] (T-0601): the edits are computed by ScriviCore (`scrivi_markdown_*`), the same code Linux calls, on the
// scene text holding the selection (`sceneRange`); this file only moves offsets between storage and the scene. A
// selection that reaches into a divider or a chapter title is refused.
extension ManuscriptPresenter {

    typealias CommandEdit = (range: NSRange, replacement: String, selection: NSRange)

    private static func command(_ e: MarkdownEdit?, _ base: Int) -> CommandEdit? {
        e.map { (NSRange(location: base + $0.range.location, length: $0.range.length), $0.replacement,
                 NSRange(location: base + $0.selection.location, length: $0.selection.length)) }
    }

    // MARK: — Words

    /// A letter or digit (Apple's `CharacterSet.alphanumerics`); a surrogate half counts as part of a letter.
    static func isWordUnit(_ c: UInt16) -> Bool {
        guard let scalar = Unicode.Scalar(c) else { return true }
        return CharacterSet.alphanumerics.contains(scalar)
    }

    /// The word the caret is IN (touching a letter or digit on either side), or nil BETWEEN words ([SP-163] Q1).
    /// ✅ An escaped apostrophe or hyphen inside a word (`don\'t`, `well\-known`) keeps it one word.
    func wordRange(in ts: NSAttributedString, at caret: Int) -> NSRange? {
        inScene(ts, NSRange(location: caret, length: 0)) { text, local, base in
            try ScriviEngine.markdownWordRange(scene: text, caret: local.location)
                .map { NSRange(location: base + $0.location, length: $0.length) }
        }
    }

    // MARK: — Bold / Italic

    /// Toggle `bit` (bold or italic) on the visible text of `selection`: SET it when any visible selected character
    /// lacks it, CLEAR it when all have it. ✅ [SP-163] Q3: an edge that cannot carry it is shrunk away. `caret`: when a
    /// WORD was formatted for a caret (Q1), the result's selection is that caret, else the formatted text.
    func emphasisEdit(in ts: NSAttributedString, selection sel: NSRange, bit: UInt8, caret: Int? = nil) -> CommandEdit? {
        inScene(ts, sel) { text, local, base in
            Self.command(try ScriviEngine.markdownEmphasisEdit(scene: text, selection: local, bit: bit,
                                                               caret: caret.map { $0 - base }), base)
        }
    }

    // MARK: — Headings, body, lists (Q5: a paragraph is ONE of body / heading / list item)

    /// Set every paragraph the selection touches to `format`'s kind; the same command again on paragraphs that all
    /// have it already returns them to body text. ✅ [SP-163] Q4: numbered items stay SEQUENTIAL.
    func paragraphEdit(in ts: NSAttributedString, selection sel: NSRange, format: ManuscriptFormat) -> CommandEdit? {
        let name: String
        switch format {
        case .heading(let n): name = "heading\(n)"
        case .body: name = "body"
        case .bulletList: name = "bulletList"
        case .numberedList: name = "numberedList"
        case .bold, .italic: return nil
        }
        return inScene(ts, sel) { text, local, base in
            Self.command(try ScriviEngine.markdownParagraphEdit(scene: text, selection: local, format: name), base)
        }
    }

    // MARK: — Return in a list item

    /// Return with the caret in a list item: on an EMPTY item, end the list; otherwise start the next item and renumber
    /// the items that follow (Q4). nil when not in a list item.
    func listReturnEdit(in ts: NSAttributedString, selection sel: NSRange) -> CommandEdit? {
        guard sel.length == 0 else { return nil }
        return inScene(ts, sel) { text, local, base in
            Self.command(try ScriviEngine.markdownListReturnEdit(scene: text, caret: local.location), base)
        }
    }

    // MARK: — [T-0591] across SCENE boundaries (ruled 2026-10-05: Apple-side)

    /// The style at the caret: what typing there would take.
    func styleAtCaret(in ts: NSAttributedString, _ caret: Int) -> UInt8 {
        inScene(ts, NSRange(location: caret, length: 0)) { text, local, _ in
            try ScriviEngine.markdownStyleAtCaret(scene: text, caret: local.location)
        } ?? 0
    }

    /// A cross-scene paste lands INSIDE the caret's span, then the scene is split there: the FIRST piece starts with
    /// that span open; the LAST piece RE-OPENS it for the text after the split. ✅ [SP-163] live pass (user): the pasted
    /// text keeps ITS OWN formatting.
    static func balancePastePieces(_ texts: [String], caretStyle s: UInt8) -> [String] {
        do {
            return try ScriviEngine.markdownBalancePastePieces(texts, caretStyle: s)
        } catch {
            NSLog("[SCRIVI-EDIT] paste pieces could not be balanced in the core: %@", String(describing: error))
            return texts
        }
    }
}
#endif
