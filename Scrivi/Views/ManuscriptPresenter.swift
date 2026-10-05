import Foundation
#if os(macOS)
import AppKit

// EP-046 E2-S1 (T-0588) + E2-S2 (T-0590) — THE PRESENTER: route (a′), Q-E2-6 ruled 2026-10-05.
// Design: `docs/Scrivi_Manuscript_Renderer_E2_Design_v0_1.md` §2–§3.
//
// ✅ STORAGE STAYS PLAIN MARKDOWN (EP-046 AC1). Nothing here writes an attribute into the text
// storage. TextKit 2 asks this delegate for each paragraph as it LAYS IT OUT, and gets the SAME
// characters with styled attributes: heading, bold and italic fonts, and hidden characters (markers,
// prefixes, escape backslashes) at 0.01 pt + clear. It also decides where the caret may rest
// (`stopTest`) and how an edit keeps emphasis balanced (`balancedEdit`, AC12).
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

    /// A REVEALED inline marker (Q-E2-1 span half; [SP-162] Q2 attributes as for the prefix).
    static var revealedMarkerAttributes: [NSAttributedString.Key: Any] { revealedPrefixAttributes }

    /// What one block hides and renders. It depends on the block's TEXT alone, so it is cached by text.
    struct BlockInfo {
        /// Block-relative offsets of hidden escape backslashes (E1's map, unchanged).
        var escapes: Set<Int> = []
        var analysis = MarkdownBlocks.Analysis()
        var headings: [MarkdownBlocks.Heading] { analysis.headings }
        var isEmpty: Bool { escapes.isEmpty && analysis.headings.isEmpty && analysis.markers.isEmpty }
    }

    private var cache: [String: BlockInfo] = [:]
    private var fonts: [Int: NSFont] = [:]

    /// The storage lines whose heading prefixes are SHOWN — the lines holding the selection's ends
    /// (Q-E2-1, line half). Kept as ranges so the lines that stop being revealed can be re-presented.
    private(set) var revealedLines: [NSRange] = []
    /// The inline spans whose markers are SHOWN — those the selection's ends are inside (Q-E2-1, span half).
    private(set) var revealedSpans: [NSRange] = []

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
        info.analysis = MarkdownBlocks.analyze(text)
        if cache.count > 4096 { cache.removeAll(keepingCapacity: true) }
        cache[text] = info
        return info
    }

    /// The blocks overlapping the CLOSED range [a, b], each with its info.
    private func blocks(in ts: NSAttributedString, from a: Int, to b: Int) -> [(range: NSRange, info: BlockInfo)] {
        let ns = ts.string as NSString
        var out: [(NSRange, BlockInfo)] = []
        var p = a
        while p <= b, p <= ns.length {
            if let blk = block(in: ts, at: p), NSMaxRange(blk) >= p {
                if out.last?.0 != blk { out.append((blk, info(ns.substring(with: blk)))) }
                p = max(NSMaxRange(blk), p + 1)
            } else {
                p += 1
            }
        }
        return out
    }

    // MARK: — Fonts

    /// The font for a heading level (nil = body) with italic / bold bits.
    private func font(level: Int?, style: UInt8) -> NSFont {
        let key = (level ?? 0) * 4 + Int(style)
        if let f = fonts[key] { return f }
        var f = level.map(Self.headingFont(level:)) ?? Self.bodyFont
        // ✅ [SP-162] live pass (user, 2026-10-05: *"I find the BOLD text to be too subtle in both Light and Dark
        // modes … a fine adjustment on how bold is bold"*): ⛔ `NSFontManager`'s bold trait gives the monospaced
        // system font SEMIBOLD (weight 0.30, measured). ✅ Bold is one real weight step above the text: body →
        // Bold (0.40); a heading (already bold) → Heavy (0.56), so bold inside a heading still shows.
        if style & MarkdownBlocks.bold != 0 {
            f = NSFont.monospacedSystemFont(ofSize: f.pointSize, weight: level == nil ? .bold : .heavy)
        }
        if style & MarkdownBlocks.italic != 0 { f = NSFontManager.shared.convert(f, toHaveTrait: .italicFontMask) }
        fonts[key] = f
        return f
    }

    // MARK: — The reveal (Q-E2-1: line half for prefixes, span half for inline markers)

    /// The lines a selection reveals: those holding its two ends.
    static func revealLines(for selection: NSRange, in ns: NSString) -> [NSRange] {
        guard ns.length > 0 else { return [] }
        let a = ns.paragraphRange(for: NSRange(location: min(selection.location, ns.length), length: 0))
        let b = ns.paragraphRange(for: NSRange(location: min(NSMaxRange(selection), ns.length), length: 0))
        return a == b ? [a] : [a, b]
    }

    /// The inline spans a selection reveals: those with one of its ends at the FIRST or LAST character of
    /// the span's text — right after an opening marker or right before a closing one.
    /// ✅ [SP-162] live pass (user, 2026-10-05: *"I was expecting the Markup Hints to go away once the cursor was
    /// no longer at the first or last character of the section"*). ⛔ It first revealed them anywhere inside.
    func revealSpans(for selection: NSRange, in ts: NSAttributedString) -> [NSRange] {
        var out: [NSRange] = []
        for c in Set([selection.location, NSMaxRange(selection)]) {
            for (b, info) in blocks(in: ts, from: c, to: c) {
                let rel = c - b.location
                let atEdge = info.analysis.markers.contains {
                    $0.opens ? NSMaxRange($0.range) == rel : $0.range.location == rel
                }
                guard atEdge else { continue }
                for s in info.analysis.spans {
                    let g = NSRange(location: b.location + s.location, length: s.length)
                    if g.location < c, c < NSMaxRange(g), !out.contains(g) { out.append(g) }
                }
            }
        }
        return out
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
        let spans = revealSpans(for: selection, in: ts)
        guard lines != revealedLines || spans != revealedSpans else { return }
        let previousLines = revealedLines, previousSpans = revealedSpans
        revealedLines = lines
        revealedSpans = spans
        // Only lines that carry a heading, and spans, need presenting again. ⚠️ `previous…` may
        // predate an edit; re-presenting text that has since moved is harmless.
        let whole = NSRange(location: 0, length: ns.length)
        var touched: [NSRange] = []
        for r in previousLines + lines {
            let r = NSIntersectionRange(r, whole)
            guard r.length > 0, let b = block(in: ts, at: r.location), !info(ns.substring(with: b)).headings.isEmpty else { continue }
            touched.append(r)
        }
        for r in previousSpans + spans {
            let r = NSIntersectionRange(r, whole)
            if r.length > 0 { touched.append(r) }
        }
        guard !touched.isEmpty else { return }
        ts.beginEditing()
        for r in touched { ts.edited(.editedAttributes, range: r, changeInLength: 0) }
        ts.endEditing()
    }

    // MARK: — Where the caret may rest (the snap reads ONLY this)

    enum StopKind { case escape, opener, closer, prefix }

    /// The run of characters the caret never rests inside, and where its HOME is:
    /// ✅ an escape backslash → before it (E1); ✅ an opening emphasis marker or a heading prefix → AFTER it,
    /// so the hint shows to the caret's LEFT ([SP-162] Q1, *"headers too"*); ✅ a closing marker → before
    /// it (the end of a bold word continues the bold — [T-0589] rule 3).
    /// ⚠️ Independent of the reveal: a revealed marker is still not a place to type into.
    struct Stop {
        let range: NSRange
        let kind: StopKind
        var homeAfter: Bool { kind == .opener || kind == .prefix }
    }

    /// A lookup for "which stop run holds storage unit `i`?", memoised per block.
    func stopTest(in ts: NSAttributedString) -> (Int) -> Stop? {
        let ns = ts.string as NSString
        var memo: (block: NSRange, info: BlockInfo)?
        return { [self] i in
            guard i >= 0, i < ns.length else { return nil }
            if memo == nil || !NSLocationInRange(i, memo!.block) {
                guard let b = block(in: ts, at: i) else { return nil }
                memo = (b, info(ns.substring(with: b)))
            }
            let (b, info) = memo!
            let rel = i - b.location
            if info.escapes.contains(rel) { return Stop(range: NSRange(location: i, length: 1), kind: .escape) }
            for h in info.headings where NSLocationInRange(rel, h.prefix) {
                return Stop(range: NSRange(location: b.location + h.prefix.location, length: h.prefix.length), kind: .prefix)
            }
            if let m = info.analysis.marker(containing: rel) {
                return Stop(range: NSRange(location: b.location + m.range.location, length: m.range.length),
                            kind: m.opens ? .opener : .closer)
            }
            return nil
        }
    }

    /// True when storage unit `i` is HIDDEN on screen under the reveal `selection` would produce.
    func isHidden(_ i: Int, in ts: NSAttributedString, revealing selection: NSRange) -> Bool {
        guard let stop = stopTest(in: ts)(i) else { return false }
        switch stop.kind {
        case .escape: return true
        case .prefix:
            let ns = ts.string as NSString
            let line = ns.paragraphRange(for: NSRange(location: i, length: 0))
            return !Self.isRevealed(lineAt: line.location, by: Self.revealLines(for: selection, in: ns))
        case .opener, .closer:
            return !revealSpans(for: selection, in: ts).contains { NSLocationInRange(i, $0) }
        }
    }

    // MARK: — NSTextContentStorageDelegate: present one paragraph

    func textContentStorage(_ textContentStorage: NSTextContentStorage,
                            textParagraphWith range: NSRange) -> NSTextParagraph? {
        guard let ts = textContentStorage.textStorage, range.length > 0, NSMaxRange(range) <= ts.length,
              let b = block(in: ts, at: range.location) else { return nil }
        let ns = ts.string as NSString
        let info = info(ns.substring(with: b))
        guard !info.isEmpty else { return nil }
        let a = info.analysis

        // Block-relative → paragraph-relative, clipped to this paragraph.
        func local(_ r: NSRange) -> NSRange? {
            let x = NSIntersectionRange(NSRange(location: b.location + r.location, length: r.length), range)
            return x.length > 0 ? NSRange(location: x.location - range.location, length: x.length) : nil
        }
        let out = NSMutableAttributedString(attributedString: ts.attributedSubstring(from: range))
        // 1. Fonts: heading level × inline style, in runs.
        let first = max(range.location, b.location) - b.location
        let last = min(NSMaxRange(range), NSMaxRange(b)) - b.location
        if first < last, !a.headings.isEmpty || !a.styles.isEmpty {
            func level(_ k: Int) -> Int? { a.headings.first { NSLocationInRange(k, $0.line) }?.level }
            var k = first
            while k < last {
                let lv = level(k), st = a.style(at: k)
                var j = k + 1
                while j < last, level(j) == lv, a.style(at: j) == st { j += 1 }
                if lv != nil || st != 0, let r = local(NSRange(location: k, length: j - k)) {
                    out.addAttribute(.font, value: font(level: lv, style: st), range: r)
                }
                k = j
            }
        }
        // 2. Heading prefixes: hidden, or shown dimmed on the caret's line (Q2).
        for h in a.headings {
            guard let p = local(h.prefix) else { continue }
            let shown = Self.isRevealed(lineAt: b.location + h.line.location, by: revealedLines)
            out.addAttributes(shown ? Self.revealedPrefixAttributes : Self.hiddenAttributes, range: p)
        }
        // 3. Inline markers: hidden, or shown dimmed in the span the caret is in.
        for m in a.markers {
            guard let r = local(m.range) else { continue }
            let g = NSRange(location: b.location + m.range.location, length: m.range.length)
            let shown = revealedSpans.contains { NSIntersectionRange($0, g).length == g.length }
            out.addAttributes(shown ? Self.revealedMarkerAttributes : Self.hiddenAttributes, range: r)
        }
        // 4. Escape backslashes: always hidden.
        for e in info.escapes {
            if let r = local(NSRange(location: e, length: 1)) { out.addAttributes(Self.hiddenAttributes, range: r) }
        }
        // ✅ SAME LENGTH as `range` — Apple's contract; only attributes differ.
        return NSTextParagraph(attributedString: out)
    }

    // MARK: — EP-046 AC12 / Q3: edits keep emphasis BALANCED

    /// Tokens (`MarkdownEmphasis`) of a storage range: every unit except emphasis markers, with the
    /// style its block gives it. Units outside any block (blank lines) carry no style.
    func tokens(in ts: NSAttributedString, _ r: NSRange) -> [MarkdownEmphasis.Token] {
        let ns = ts.string as NSString
        var out: [MarkdownEmphasis.Token] = []
        var p = r.location
        let end = NSMaxRange(r)
        while p < end {
            if let b = block(in: ts, at: p), NSLocationInRange(p, b) {
                let units = Array(ns.substring(with: b).utf16)
                let stop = min(end, NSMaxRange(b))
                out += MarkdownEmphasis.tokens(of: units[...], analysis: info(ns.substring(with: b)).analysis,
                                               from: p - b.location, to: stop - b.location)
                p = stop
            } else {
                out.append(MarkdownEmphasis.Token(unit: ns.character(at: p), style: 0))
                p += 1
            }
        }
        return out
    }

    /// The edit that keeps emphasis balanced when `range` is replaced by `replacement` (already in
    /// stored form), or nil when the plain edit is already balanced — ✅ the common case (typing inside
    /// or outside a span) costs one scan of the touched blocks' markers.
    /// ✅ Rewrites ONLY the stretch the edit touches: the spans it crosses, plus the edit itself.
    func balancedEdit(in ts: NSAttributedString, replacing range: NSRange,
                      with replacement: String) -> (range: NSRange, replacement: String, caret: Int)? {
        let ns = ts.string as NSString
        let a = range.location, b = NSMaxRange(range)
        let touched = blocks(in: ts, from: a, to: b)
        var spans: [NSRange] = []
        var markers: [NSRange] = []
        var styleAt: [Int: UInt8] = [:]
        for (blk, info) in touched {
            spans += info.analysis.spans.map { NSRange(location: blk.location + $0.location, length: $0.length) }
            markers += info.analysis.markers.map { NSRange(location: blk.location + $0.range.location, length: $0.range.length) }
            for k in [a - 1, a] where NSLocationInRange(k, blk) { styleAt[k] = info.analysis.style(at: k - blk.location) }
        }
        let fragment = MarkdownEmphasis.mayHaveMarkers(replacement)
        let crossesMarker = range.length > 0 && markers.contains { $0.location < b && NSMaxRange($0) > a }
        let markerUnits = Set(markers.flatMap { $0.location..<NSMaxRange($0) })
        let emptiesSpan = range.length > 0 && replacement.isEmpty && spans.contains { s in
            (s.location..<NSMaxRange(s)).allSatisfy { markerUnits.contains($0) || ($0 >= a && $0 < b) }
        }
        let splitsSpan = replacement.utf16.contains(0x0A) && ((styleAt[a] ?? 0) != 0 || (styleAt[a - 1] ?? 0) != 0)
        guard fragment || crossesMarker || emptiesSpan || splitsSpan else { return nil }

        // The stretch to rewrite: the edit plus every span it touches (closed interval).
        var w0 = a, w1 = b
        for s in spans where s.location <= b && NSMaxRange(s) >= a {
            w0 = min(w0, s.location); w1 = max(w1, NSMaxRange(s))
        }
        let left = tokens(in: ts, NSRange(location: w0, length: a - w0))
        let selected = tokens(in: ts, range)
        let right = tokens(in: ts, NSRange(location: b, length: w1 - b))
        // The style the new text takes: that of the first selected character, or — for an insertion —
        // the caret's: just after an OPENING marker the span's (Q1), otherwise the text's before it.
        let afterOpener = touched.contains { blk, info in
            info.analysis.markers.contains { $0.opens && blk.location + NSMaxRange($0.range) == a }
        }
        let insertStyle: UInt8 = selected.first?.style ?? (afterOpener ? (right.first?.style ?? 0) : (left.last?.style ?? 0))
        let inserted = MarkdownEmphasis.tokens(of: replacement).map {
            MarkdownEmphasis.Token(unit: $0.unit, style: $0.style | insertStyle, isPrefix: $0.isPrefix)
        }
        var all = MarkdownEmphasis.normalize(left + inserted + right)
        var (text, ends) = MarkdownEmphasis.serialize(all)

        // ✅ Checked by the parser, IN CONTEXT, before it is applied. ⚠️ If it would not read back as
        // intended, the stretch is written with no emphasis: formatting is lost there, but no stray
        // marker ever shows. Measured in [SP-162]: 5,999/6,000 realistic edits read back exactly.
        let r0 = touched.first.map { min($0.range.location, w0) } ?? w0
        let r1 = touched.last.map { max(NSMaxRange($0.range), w1) } ?? w1
        let prefix = ns.substring(with: NSRange(location: r0, length: w0 - r0))
        let suffix = ns.substring(with: NSRange(location: w1, length: r1 - w1))
        let expected = tokens(in: ts, NSRange(location: r0, length: w0 - r0)) + all
            + tokens(in: ts, NSRange(location: w1, length: r1 - w1))
        if !MarkdownEmphasis.sameRendering(expected, MarkdownEmphasis.tokens(of: prefix + text + suffix)) {
            NSLog("[SCRIVI-EDIT] emphasis could not be balanced at %d — written without emphasis", w0)
            all = all.map { MarkdownEmphasis.Token(unit: $0.unit, style: 0, isPrefix: $0.isPrefix) }
            (text, ends) = MarkdownEmphasis.serialize(all)
        }
        let lastInserted = left.count + inserted.count - 1
        let caret = lastInserted >= 0 && !ends.isEmpty ? ends[lastInserted] : 0
        return (NSRange(location: w0, length: w1 - w0), text, w0 + caret)
    }

    /// What a copy of `selection` puts on the pasteboards: the BALANCED source (Scrivi's own paste keeps the
    /// formatting — AC12) and the PRESENTED text, markers removed ([SP-162] Q2).
    func balancedCopy(in ts: NSAttributedString, _ selection: NSRange) -> (source: String, presented: String) {
        var toks = MarkdownEmphasis.normalize(tokens(in: ts, selection))
        var source = MarkdownEmphasis.serialize(toks).text
        if !MarkdownEmphasis.sameRendering(toks, MarkdownEmphasis.tokens(of: source)) {
            toks = toks.map { MarkdownEmphasis.Token(unit: $0.unit, style: 0, isPrefix: $0.isPrefix) }
            source = MarkdownEmphasis.serialize(toks).text
        }
        return (source, MarkdownEmphasis.presented(toks))
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
