import Foundation
#if os(macOS)
import AppKit

// EP-046 E2-S1 (T-0588) — THE PRESENTER: route (a′), Q-E2-6 ruled 2026-10-05.
// Design: `docs/Scrivi_Manuscript_Renderer_E2_Design_v0_1.md` §2–§3.
//
// ✅ STORAGE STAYS PLAIN MARKDOWN (EP-046 AC1). Nothing here writes an attribute into the text
// storage. TextKit 2 asks this delegate for each paragraph as it LAYS IT OUT, and gets the SAME
// characters with styled attributes: heading fonts, and hidden characters at 0.01 pt + clear.
// ⚠️ Apple's contract for that delegate: *"The attributed string for a custom text paragraph must
// have `range.length`."* E1's route (a) REMOVED the backslash, broke that, and the caret jumped
// (0 → 8). This presenter never changes a character, so offsets stay SOURCE offsets everywhere.
// ✅ Measured (SP-159 S4b): a 1.85 MB rebuild styles only the laid-out paragraphs (0.45 ms +
// 6.8 ms viewport layout), where E1's storage styler generalised costs 201–682 ms.
// ⚠️ It REPLACES `EscapeHidingStyler` (EP-046 AC11): R3 = (c)'s hidden backslash looks the same;
// its attributes moved from storage to the presented paragraph.
final class ManuscriptPresenter: NSObject, NSTextContentStorageDelegate, NSTextStorageDelegate {

    // Computed, not stored: Swift 6 rejects a static `NSFont` / attribute dictionary as not
    // concurrency-safe, and both are cheap to build.
    static var bodyFont: NSFont { NSFont.monospacedSystemFont(ofSize: NSFont.systemFontSize, weight: .regular) }

    /// ✅ SP-161 Q1 (ruled 2026-10-05): H1 22 pt, H2 18 pt, H3 16 pt, H4–H6 the body size — all bold.
    static func headingFont(level: Int) -> NSFont {
        let size: CGFloat = switch level {
        case 1: 22
        case 2: 18
        case 3: 16
        default: NSFont.systemFontSize
        }
        return NSFont.monospacedSystemFont(ofSize: size, weight: .bold)
    }

    /// A hidden character: near-zero and transparent (measured: no visible width, no glyph).
    static var hiddenAttributes: [NSAttributedString.Key: Any] {
        [.font: NSFont.systemFont(ofSize: 0.01), .foregroundColor: NSColor.clear]
    }

    /// ✅ SP-161 Q2 (ruled 2026-10-05): a REVEALED prefix — dimmed, in the body font.
    static var revealedPrefixAttributes: [NSAttributedString.Key: Any] {
        [.font: bodyFont, .foregroundColor: NSColor.tertiaryLabelColor]
    }

    /// What one block hides and renders. It depends on the block's TEXT alone, so it is cached by text.
    struct BlockInfo {
        /// Block-relative offsets of hidden escape backslashes (E1's map, unchanged).
        var escapes: Set<Int> = []
        var headings: [MarkdownBlocks.Heading] = []
        var isEmpty: Bool { escapes.isEmpty && headings.isEmpty }
    }

    private var cache: [String: BlockInfo] = [:]

    /// The storage lines whose heading prefixes are SHOWN — the lines holding the selection's ends
    /// (Q-E2-1, line half). Kept as ranges so the lines that stop being revealed can be re-presented.
    private(set) var revealedLines: [NSRange] = []

    // MARK: — Blocks

    /// The scene-text part of the line holding `loc`: from the line's start up to its first divider
    /// or chapter-heading character. ⚠️ A scene file with no final newline puts the DIVIDER on its
    /// last line (`testr￼⏎`, four such files in the dumas fixture — SP-161 step 1), so a block ends
    /// at the divider CHARACTER, not at a divider line. nil when the line is not scene text at all.
    private static func sceneLine(_ ts: NSAttributedString, _ ns: NSString, containing loc: Int) -> (line: NSRange, endsScene: Bool)? {
        let line = ns.paragraphRange(for: NSRange(location: min(loc, ns.length), length: 0))
        var cut = NSMaxRange(line)
        for key in [NSAttributedString.Key.scriviDivider, .scriviHeading] where line.length > 0 {
            ts.enumerateAttribute(key, in: line, options: []) { value, r, stop in
                if value != nil { cut = min(cut, r.location); stop.pointee = true }
            }
        }
        if cut == line.location, line.length > 0 { return nil }
        return (NSRange(location: line.location, length: cut - line.location), cut < NSMaxRange(line))
    }

    private static func isBlank(_ ns: NSString, _ r: NSRange) -> Bool {
        for i in r.location..<NSMaxRange(r) {
            let c = ns.character(at: i)
            if c != 0x20 && c != 0x09 && c != 0x0A { return false }
        }
        return true
    }

    /// The block (a maximal run of non-blank scene lines) holding `loc`, or nil when `loc` is on a
    /// blank line, a divider or a chapter heading.
    func block(in ts: NSAttributedString, at loc: Int) -> NSRange? {
        let ns = ts.string as NSString
        guard ns.length > 0, let first = Self.sceneLine(ts, ns, containing: loc), !Self.isBlank(ns, first.line) else {
            return nil
        }
        // `loc` past the scene text of a line that ends the scene: it is the divider itself.
        if first.endsScene, loc >= NSMaxRange(first.line) { return nil }
        var start = first.line
        while start.location > 0 {
            guard let prev = Self.sceneLine(ts, ns, containing: start.location - 1), !prev.endsScene,
                  !Self.isBlank(ns, prev.line) else { break }
            start = prev.line
        }
        var end = first
        while !end.endsScene, NSMaxRange(end.line) < ns.length {
            guard let next = Self.sceneLine(ts, ns, containing: NSMaxRange(end.line)), !Self.isBlank(ns, next.line) else { break }
            end = next
        }
        return NSUnionRange(start, end.line)
    }

    func info(_ text: String) -> BlockInfo {
        if let hit = cache[text] { return hit }
        var info = BlockInfo()
        if text.utf16.contains(0x5C) {
            // E1, unchanged: one line at a time; a line-end backslash is a hidden HARD BREAK only when
            // a line of the same block follows it (EP-045 AC6, Q1 = (a)).
            let ns = text as NSString
            var start = 0
            while start < ns.length {
                let line = ns.paragraphRange(for: NSRange(location: start, length: 0))
                let continues = NSMaxRange(line) < ns.length
                for off in MarkdownEscapes.hiddenBackslashes(in: ns.substring(with: line), continues: continues) {
                    info.escapes.insert(line.location + off)
                }
                start = NSMaxRange(line)
            }
        }
        info.headings = MarkdownBlocks.analyze(text).headings
        if cache.count > 4096 { cache.removeAll(keepingCapacity: true) }
        cache[text] = info
        return info
    }

    // MARK: — The reveal (Q-E2-1, line half)

    /// The lines a selection reveals: those holding its two ends.
    static func revealLines(for selection: NSRange, in ns: NSString) -> [NSRange] {
        guard ns.length > 0 else { return [] }
        let a = ns.paragraphRange(for: NSRange(location: min(selection.location, ns.length), length: 0))
        let b = ns.paragraphRange(for: NSRange(location: min(NSMaxRange(selection), ns.length), length: 0))
        return a == b ? [a] : [a, b]
    }

    private static func isRevealed(lineAt loc: Int, by lines: [NSRange]) -> Bool {
        lines.contains { $0.location == loc || NSLocationInRange(loc, $0) }
    }

    /// Move the reveal to `selection`. ✅ Attributes-only: an `.editedAttributes` edit re-asks TextKit
    /// for those paragraphs (measured SP-159 S4b — `invalidateLayout(for:)` does NOT), posts no
    /// `textDidChange`, registers no undo, and the scene-boundary table ignores it (AC4).
    func reveal(for selection: NSRange, in ts: NSTextStorage) {
        let ns = ts.string as NSString
        let lines = Self.revealLines(for: selection, in: ns)
        guard lines != revealedLines else { return }
        let previous = revealedLines
        revealedLines = lines
        // Only lines that carry a heading need presenting again. ⚠️ `previous` may predate an edit;
        // re-presenting a line that has since moved is harmless.
        var touched: [NSRange] = []
        for r in previous + lines {
            let r = NSIntersectionRange(r, NSRange(location: 0, length: ns.length))
            guard r.length > 0, let b = block(in: ts, at: r.location), !info(ns.substring(with: b)).headings.isEmpty else { continue }
            touched.append(r)
        }
        guard !touched.isEmpty else { return }
        ts.beginEditing()
        for r in touched { ts.edited(.editedAttributes, range: r, changeInLength: 0) }
        ts.endEditing()
    }

    // MARK: — What is hidden (the caret snap reads ONLY this)

    /// A test for "is storage character `i` hidden?" under the reveal `selection` would produce.
    /// ⚠️ Storage carries no hiding attribute any more, so the snap and the pair-delete ask here.
    func hiddenTest(in ts: NSAttributedString, revealing selection: NSRange) -> (Int) -> Bool {
        let ns = ts.string as NSString
        let lines = Self.revealLines(for: selection, in: ns)
        var memo: (block: NSRange, info: BlockInfo)?
        return { [self] i in
            guard i >= 0, i < ns.length else { return false }
            if memo == nil || !NSLocationInRange(i, memo!.block) {
                guard let b = block(in: ts, at: i) else { return false }
                memo = (b, info(ns.substring(with: b)))
            }
            let (b, info) = memo!
            let rel = i - b.location
            if info.escapes.contains(rel) { return true }
            for h in info.headings where NSLocationInRange(rel, h.prefix) {
                return !Self.isRevealed(lineAt: b.location + h.line.location, by: lines)
            }
            return false
        }
    }

    func isHidden(_ i: Int, in ts: NSAttributedString, revealing selection: NSRange) -> Bool {
        hiddenTest(in: ts, revealing: selection)(i)
    }

    // MARK: — NSTextContentStorageDelegate: present one paragraph

    func textContentStorage(_ textContentStorage: NSTextContentStorage,
                            textParagraphWith range: NSRange) -> NSTextParagraph? {
        guard let ts = textContentStorage.textStorage, range.length > 0, NSMaxRange(range) <= ts.length,
              let b = block(in: ts, at: range.location) else { return nil }
        let ns = ts.string as NSString
        let info = info(ns.substring(with: b))
        guard !info.isEmpty else { return nil }

        // Block-relative → paragraph-relative, clipped to this paragraph.
        func local(_ r: NSRange) -> NSRange? {
            let x = NSIntersectionRange(NSRange(location: b.location + r.location, length: r.length), range)
            return x.length > 0 ? NSRange(location: x.location - range.location, length: x.length) : nil
        }
        let out = NSMutableAttributedString(attributedString: ts.attributedSubstring(from: range))
        var changed = false
        for h in info.headings {
            guard let line = local(h.line) else { continue }
            out.addAttribute(.font, value: Self.headingFont(level: h.level), range: line)
            if let p = local(h.prefix) {
                let shown = Self.isRevealed(lineAt: b.location + h.line.location, by: revealedLines)
                out.addAttributes(shown ? Self.revealedPrefixAttributes : Self.hiddenAttributes, range: p)
            }
            changed = true
        }
        for e in info.escapes {
            if let r = local(NSRange(location: e, length: 1)) {
                out.addAttributes(Self.hiddenAttributes, range: r)
                changed = true
            }
        }
        // ✅ SAME LENGTH as `range` — Apple's contract; only attributes differ.
        return changed ? NSTextParagraph(attributedString: out) : nil
    }

    // MARK: — NSTextStorageDelegate: a character edit re-presents its whole block

    /// A character edit can change how OTHER lines of its block present — e.g. a ⌫ merge that turns
    /// `end.\⏎⏎next` into a hard break hides the backslash on the line ABOVE (EP-045 AC6). TextKit
    /// re-asks only for the edited paragraphs, so widen the edit to the block (and the line above,
    /// as E1's styler did) with an attributes-only edit.
    func textStorage(_ textStorage: NSTextStorage, didProcessEditing editedMask: NSTextStorageEditActions,
                     range editedRange: NSRange, changeInLength delta: Int) {
        guard editedMask.contains(.editedCharacters) else { return }
        let t0 = Date()
        let ns = textStorage.string as NSString
        guard ns.length > 0 else { return }
        let loc = min(editedRange.location, ns.length)
        var span = ns.paragraphRange(for: NSRange(location: loc, length: min(editedRange.length, ns.length - loc)))
        if span.location > 0 {
            span = NSUnionRange(span, ns.paragraphRange(for: NSRange(location: span.location - 1, length: 0)))
        }
        if let b = block(in: textStorage, at: span.location) { span = NSUnionRange(span, b) }
        if span.length > 0, let b = block(in: textStorage, at: NSMaxRange(span) - 1) { span = NSUnionRange(span, b) }
        if span != editedRange { textStorage.edited(.editedAttributes, range: span, changeInLength: 0) }
        // EP-046 AC8: the presenter's own per-edit cost (logged only above 0.5 ms, as E1's was).
        let ms = Date().timeIntervalSince(t0) * 1000
        if ms > 0.5 { NSLog(String(format: "[SCRIVI-EDIT] present=%.1f ms (range=%d)", ms, editedRange.length)) }
    }
}
#endif
