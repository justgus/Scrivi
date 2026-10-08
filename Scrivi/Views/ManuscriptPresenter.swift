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

    /// ✅ EP-047 S2 (SP-168, AC4): the manuscript's TYPE — the ONE stored value. The text view reads it through this
    /// presenter (its `textContentStorage` delegate) for every body run it writes; the presenter draws headings, emphasis
    /// and list lines from it. ⛔ The old static `bodyFont` / `headingFont` are GONE, so nothing can keep a second source.
    var typography: ManuscriptTypography = .default

    /// A hidden character: near-zero and transparent (measured: no visible width, no glyph).
    static var hiddenAttributes: [NSAttributedString.Key: Any] {
        [.font: NSFont.systemFont(ofSize: 0.01), .foregroundColor: NSColor.clear]   // type-source-ok: hides a character; not manuscript type
    }

    /// ✅ SP-161 Q2 (ruled 2026-10-05): a REVEALED prefix — dimmed, in the body font.
    var revealedPrefixAttributes: [NSAttributedString.Key: Any] {
        [.font: typography.bodyFont, .foregroundColor: NSColor.tertiaryLabelColor]
    }

    /// A REVEALED inline marker (Q-E2-1 span half; [SP-162] Q2 attributes as for the prefix).
    var revealedMarkerAttributes: [NSAttributedString.Key: Any] { revealedPrefixAttributes }

    /// What one block hides and renders. It depends on the block's TEXT alone, so it is cached by text.
    struct BlockInfo {
        /// Block-relative offsets of hidden escape backslashes (E1's map, unchanged).
        var escapes: Set<Int> = []
        var analysis = MarkdownBlocks.Analysis()
        var headings: [MarkdownBlocks.Heading] { analysis.headings }
        var isEmpty: Bool {
            escapes.isEmpty && analysis.headings.isEmpty && analysis.markers.isEmpty && analysis.listItems.isEmpty
        }
    }

    private var cache: [String: BlockInfo] = [:]

    /// The storage lines whose heading prefixes are SHOWN — the lines holding the selection's ends
    /// (Q-E2-1, line half). Kept as ranges so the lines that stop being revealed can be re-presented.
    private(set) var revealedLines: [NSRange] = []
    /// The inline spans whose markers are SHOWN — those the selection's ends are inside (Q-E2-1, span half).
    private(set) var revealedSpans: [NSRange] = []

    /// ✅ [SP-163] Q1 (user, 2026-10-05): ⌘B / ⌘I with the caret BETWEEN words shows *"the start and end hints smashed
    /// together with the caret between them"*. ⚠️ Markdown cannot STORE an empty `****` (it reads as four literal
    /// asterisks), so the pair is PENDING: it is in the text view's storage ONLY — inserted and removed without
    /// `didChangeText`, so history, autosave and the scene's text never see it. Typing into it makes it a real span
    /// (an ordinary, recorded edit); leaving it removes it, with nothing to undo (user: *"it should be removed"*).
    struct PendingPair: Equatable {
        var range: NSRange          // the marker characters, opener + closer
        let markerLength: Int       // 2 for `**`, 1 for `*`
        var caret: Int { range.location + markerLength }
    }
    var pending: PendingPair?
    /// True while the presenter itself inserts or removes the pending pair.
    var insertingPending = false

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

    // MARK: — EP-047 S3: the first-line indent (P3, P10)

    /// A stored blank line of SCENE text (not a divider's or a chapter title's line).
    private func isGapLine(_ ts: NSAttributedString, _ ns: NSString, _ range: NSRange) -> Bool {
        guard let line = Self.sceneLine(ts, ns, containing: range.location), line.line.length > 0 else { return false }
        return Self.isBlank(ns, line.line)
    }

    /// True when the block is BODY TEXT — not a heading, a list item, a block quote, a table or a fenced block.
    func isBody(_ ns: NSString, block b: NSRange, info: BlockInfo) -> Bool {
        if info.analysis.headings.contains(where: { $0.line.location == 0 }) { return false }
        if info.analysis.listItems.contains(where: { $0.line.location == 0 }) { return false }
        let text = ns.substring(with: b).drop { $0 == " " }
        return !(text.hasPrefix(">") || text.hasPrefix("|") || text.hasPrefix("```") || text.hasPrefix("~~~"))
    }

    /// The first-line indent this block's first line gets, in points (0 = none).
    /// ✅ P10 book rule: indent only when the PREVIOUS block in the SAME scene is body text — not at a scene's start, not
    /// after a heading, a list, a quote or a chapter title.
    func firstLineIndent(_ ts: NSAttributedString, _ ns: NSString, block b: NSRange, info: BlockInfo) -> CGFloat {
        guard typography.indent != .none, isBody(ns, block: b, info: info) else { return 0 }
        if typography.indent == .every { return typography.firstLineIndent }
        guard let prev = previousBlock(in: ts, ns, before: b.location) else { return 0 }
        return isBody(ns, block: prev, info: self.info(ns.substring(with: prev))) ? typography.firstLineIndent : 0
    }

    /// The block before `loc` in the SAME scene, skipping blank lines; nil at a scene's start or after a chapter title.
    func previousBlock(in ts: NSAttributedString, _ ns: NSString, before loc: Int) -> NSRange? {
        var at = loc - 1
        while at >= 0 {
            guard let line = Self.sceneLine(ts, ns, containing: at), !line.endsScene || at < NSMaxRange(line.line) else { return nil }
            if line.line.length == 0 { return nil }
            if !Self.isBlank(ns, line.line) { return block(in: ts, at: line.line.location) }
            at = line.line.location - 1
        }
        return nil
    }

    /// The block after `loc` in the same scene, skipping blank lines.
    func nextBlock(in ts: NSAttributedString, _ ns: NSString, after loc: Int) -> NSRange? {
        var at = loc
        while at < ns.length {
            guard let line = Self.sceneLine(ts, ns, containing: at), line.line.length > 0 else { return nil }
            if !Self.isBlank(ns, line.line) { return block(in: ts, at: line.line.location) }
            if line.endsScene { return nil }
            at = NSMaxRange(line.line)
        }
        return nil
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
    func blocks(in ts: NSAttributedString, from a: Int, to b: Int) -> [(range: NSRange, info: BlockInfo)] {
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

    /// The font for a heading level (nil = body) with italic / bold bits — `ManuscriptTypography`'s rule (bold 700; bold
    /// inside a heading = the face's heaviest weight; the face's own italic file).
    private func font(level: Int?, style: UInt8) -> NSFont {
        typography.font(level: level, style: style)
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

    enum StopKind { case escape, opener, closer, prefix, listPrefix }

    /// The run of characters the caret never rests inside, and where its HOME is:
    /// ✅ an escape backslash → before it (E1); ✅ an opening emphasis marker or a heading prefix → AFTER it,
    /// so the hint shows to the caret's LEFT ([SP-162] Q1, *"headers too"*); ✅ a closing marker → before
    /// it (the end of a bold word continues the bold — [T-0589] rule 3).
    /// ⚠️ Independent of the reveal: a revealed marker is still not a place to type into.
    struct Stop {
        let range: NSRange
        let kind: StopKind
        var homeAfter: Bool { kind == .opener || kind == .prefix || kind == .listPrefix }
    }

    /// A lookup for "which stop run holds storage unit `i`?", memoised per block.
    func stopTest(in ts: NSAttributedString) -> (Int) -> Stop? {
        let ns = ts.string as NSString
        var memo: (block: NSRange, info: BlockInfo)?
        return { [self] i in
            guard i >= 0, i < ns.length else { return nil }
            // The pending pair: its opener's home is after it, its closer's before it — the same spot, between them.
            if let p = pending, NSLocationInRange(i, p.range) {
                return i < p.caret ? Stop(range: NSRange(location: p.range.location, length: p.markerLength), kind: .opener)
                                   : Stop(range: NSRange(location: p.caret, length: p.markerLength), kind: .closer)
            }
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
            for li in info.analysis.listItems where NSLocationInRange(rel, li.prefix) {
                return Stop(range: NSRange(location: b.location + li.prefix.location, length: li.prefix.length), kind: .listPrefix)
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
        if let p = pending, NSLocationInRange(i, p.range) { return false }   // shown, dimmed
        guard let stop = stopTest(in: ts)(i) else { return false }
        switch stop.kind {
        case .escape: return true
        case .listPrefix: return false     // [SP-163] Q7: always visible, dimmed
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
        guard let ts = textContentStorage.textStorage, range.length > 0, NSMaxRange(range) <= ts.length else { return nil }
        let ns = ts.string as NSString
        guard let b = block(in: ts, at: range.location) else {
            // ✅ EP-047 S3 (P10): with an indent on, a stored BLANK line of scene text draws as a small gap.
            if typography.indent != .none, isGapLine(ts, ns, range) {
                let out = NSMutableAttributedString(attributedString: ts.attributedSubstring(from: range))
                out.addAttribute(.paragraphStyle, value: typography.gapParagraphStyle, range: NSRange(location: 0, length: out.length))
                return NSTextParagraph(attributedString: out)
            }
            return nil
        }
        let info = info(ns.substring(with: b))
        // ✅ EP-047 S3: a body block's FIRST line gets the first-line indent — so a PLAIN paragraph (no markup) is presented
        // too whenever an indent is on. Presentation only: ⛔ zero characters in storage, so the `.md` cannot change.
        let indent = range.location == b.location ? firstLineIndent(ts, ns, block: b, info: info) : 0
        guard !info.isEmpty || indent > 0 || pending.map({ NSIntersectionRange($0.range, range).length > 0 }) == true else { return nil }
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
        // 1b. ✅ EP-047 P9: a heading line sits on ITS OWN line height (1.25 × its size) — the storage paragraph style is the
        // body's 1.45 × body size, whose FIXED maximum would clip a larger heading.
        for h in a.headings {
            if let r = local(h.line) {
                out.addAttribute(.paragraphStyle, value: typography.paragraphStyle(
                    lineFor: typography.headingSize(level: h.level), heading: true), range: r)
            }
        }
        // 2. Heading prefixes: hidden, or shown dimmed on the caret's line (Q2).
        for h in a.headings {
            guard let p = local(h.prefix) else { continue }
            let shown = Self.isRevealed(lineAt: b.location + h.line.location, by: revealedLines)
            out.addAttributes(shown ? revealedPrefixAttributes : Self.hiddenAttributes, range: p)
        }
        // 3. Inline markers: hidden, or shown dimmed in the span the caret is in.
        for m in a.markers {
            guard let r = local(m.range) else { continue }
            let g = NSRange(location: b.location + m.range.location, length: m.range.length)
            let shown = revealedSpans.contains { NSIntersectionRange($0, g).length == g.length }
            out.addAttributes(shown ? revealedMarkerAttributes : Self.hiddenAttributes, range: r)
        }
        // 4. Escape backslashes: always hidden.
        for e in info.escapes {
            if let r = local(NSRange(location: e, length: 1)) { out.addAttributes(Self.hiddenAttributes, range: r) }
        }
        // 5. ✅ [SP-163] lists: the prefix stays VISIBLE, dimmed (Q7), with a HANGING indent so wrapped lines align
        // under the text (design §4.2). ⛔ TextKit's own `NSTextList` bullet draws invisible here (SP-159 S1).
        for li in a.listItems {
            guard let line = local(li.line), let p = local(li.prefix) else { continue }
            out.addAttributes(revealedPrefixAttributes, range: p)
            let width = (ns.substring(with: NSRange(location: b.location + li.prefix.location, length: li.prefix.length)) as NSString)
                .size(withAttributes: [.font: typography.bodyFont]).width
            // ✅ EP-047: from THE paragraph-style builder, so a list line keeps the body line height.
            out.addAttribute(.paragraphStyle, value: typography.paragraphStyle(headIndent: width), range: line)
        }
        // 5b. ✅ EP-047 S3: the first-line indent, from THE paragraph-style builder (body line height kept).
        if indent > 0 {
            // `indent > 0` only when `range` starts the block, so `range` IS the block's first line.
            out.addAttribute(.paragraphStyle, value: typography.paragraphStyle(firstLineIndent: indent),
                             range: NSRange(location: 0, length: out.length))
        }
        // 6. The PENDING pair (Q1): shown as revealed hints, the caret between them.
        if let pp = pending, let r = local(pp.range) {
            out.addAttributes(revealedMarkerAttributes, range: r)
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
    /// `keepsOwnFormatting`: Scrivi's own copy being pasted keeps ITS styles instead of taking the caret's ([SP-163]
    /// live pass) — so it is rewritten even when it carries no marker, if it lands inside a span.
    func balancedEdit(in ts: NSAttributedString, replacing range: NSRange,
                      with replacement: String, keepsOwnFormatting: Bool = false) -> (range: NSRange, replacement: String, caret: Int)? {
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
        // Only a PARAGRAPH break (a blank line) splits a span — ⚠️ a single newline (Option-Return's hard break,
        // [T-0584]) may sit inside bold, and closing before it would put the closer after the `\` and escape it.
        let splitsSpan = replacement.contains("\n\n") && ((styleAt[a] ?? 0) != 0 || (styleAt[a - 1] ?? 0) != 0)
        let insideSpan = (styleAt[a] ?? 0) != 0 || (styleAt[a - 1] ?? 0) != 0
        let ownInsideSpan = keepsOwnFormatting && !replacement.isEmpty && insideSpan
        // ✅ [SP-163] live pass (user, 2026-10-05): *"I thought we were going to push or pull the spaces to outside the
        // attributed sections."* ⛔ A space typed at a span's FIRST or LAST character took the plain path and left
        // `** bold**` — not emphasis, so both markers showed. ✅ Any edit AT a span's edge (just after an opener, just
        // before a closer) goes through the rewrite, whose normaliser moves edge whitespace outside.
        let atSpanEdge = touched.contains { blk, info in
            info.analysis.markers.contains { m in
                let g = NSRange(location: blk.location + m.range.location, length: m.range.length)
                return m.opens ? NSMaxRange(g) == a : g.location == b
            }
        }
        guard fragment || crossesMarker || emptiesSpan || splitsSpan || ownInsideSpan || atSpanEdge else { return nil }

        let insertStyle = keepsOwnFormatting ? 0 : insertionStyle(in: ts, at: a, selected: tokens(in: ts, range))
        let inserted = MarkdownEmphasis.tokens(of: replacement).map {
            // A prefix written by the edit (a list's `- ` on Return) never carries emphasis.
            MarkdownEmphasis.Token(unit: $0.unit, style: $0.isPrefix ? 0 : $0.style | insertStyle, isPrefix: $0.isPrefix)
        }
        guard let r = rewrite(in: ts, replacing: range, with: inserted, spans: spans, refuseIfUnbalanced: false) else { return nil }
        return (r.range, r.text, r.range.location + r.caret)
    }

    /// The style new text takes when it replaces `selected` at `a`: that of the first selected character, or —
    /// for an insertion — the caret's: just after an OPENING marker the span's ([SP-162] Q1), otherwise the
    /// text's before it.
    func insertionStyle(in ts: NSAttributedString, at a: Int, selected: [MarkdownEmphasis.Token]) -> UInt8 {
        if let first = selected.first { return first.style }
        let touched = blocks(in: ts, from: a, to: a)
        let afterOpener = touched.contains { blk, info in
            info.analysis.markers.contains { $0.opens && blk.location + NSMaxRange($0.range) == a }
        }
        if afterOpener { return tokens(in: ts, NSRange(location: a, length: min(1, (ts.string as NSString).length - a))).first?.style ?? 0 }
        guard a > 0 else { return 0 }
        // The nearest character before `a` that is not a marker.
        var k = a - 1
        let stops = stopTest(in: ts)
        while k > 0, let st = stops(k), st.kind == .opener || st.kind == .closer { k -= 1 }
        return tokens(in: ts, NSRange(location: k, length: 1)).first?.style ?? 0
    }

    /// The emphasis spans (storage ranges) of the blocks overlapping the closed range [a, b].
    func spans(in ts: NSAttributedString, from a: Int, to b: Int) -> [NSRange] {
        blocks(in: ts, from: a, to: b).flatMap { blk, info in
            info.analysis.spans.map { NSRange(location: blk.location + $0.location, length: $0.length) }
        }
    }

    /// THE REWRITE CORE (E2-S2, shared by edits and the Format commands): replace `range` by `inserted` tokens and
    /// write the stretch — the edit plus every span it touches — back with the fewest markers.
    /// ✅ Checked by the parser IN CONTEXT before it is applied. If it would not read back as intended: an EDIT is
    /// written without emphasis there (formatting lost, but no stray marker shows; logged); a COMMAND
    /// (`refuseIfUnbalanced`) is refused instead — a command must never destroy formatting.
    /// `caret` is the output offset just after the last inserted token; `starts[i]` is where output token `i` begins.
    func rewrite(in ts: NSAttributedString, replacing range: NSRange, with inserted: [MarkdownEmphasis.Token],
                 spans: [NSRange], refuseIfUnbalanced: Bool) -> (range: NSRange, text: String, caret: Int, ends: [Int], leftCount: Int)? {
        let ns = ts.string as NSString
        let a = range.location, b = NSMaxRange(range)
        var w0 = a, w1 = b
        for s in spans where s.location <= b && NSMaxRange(s) >= a {
            w0 = min(w0, s.location); w1 = max(w1, NSMaxRange(s))
        }
        let left = tokens(in: ts, NSRange(location: w0, length: a - w0))
        let right = tokens(in: ts, NSRange(location: b, length: w1 - b))
        let touched = blocks(in: ts, from: w0, to: w1)
        let r0 = touched.first.map { min($0.range.location, w0) } ?? w0
        let r1 = touched.last.map { max(NSMaxRange($0.range), w1) } ?? w1
        let before = tokens(in: ts, NSRange(location: r0, length: w0 - r0))
        let after = tokens(in: ts, NSRange(location: w1, length: r1 - w1))
        // ⚠️ Normalise WITH one token of context each side: whether a span edge can sit on punctuation depends on
        // the character just outside the stretch (`d\'Artagnan`: the `'` follows a letter). Measured [SP-163]:
        // without it, 6.9% of random selections were refused. The context itself is never rewritten.
        let ctxL = Array(before.suffix(1)), ctxR = Array(after.prefix(1))
        var all = Array(MarkdownEmphasis.normalize(ctxL + left + inserted + right + ctxR).dropFirst(ctxL.count).dropLast(ctxR.count))
        var (text, ends) = MarkdownEmphasis.serialize(all)
        let prefix = ns.substring(with: NSRange(location: r0, length: w0 - r0))
        let suffix = ns.substring(with: NSRange(location: w1, length: r1 - w1))
        let expected = before + all + after
        if !MarkdownEmphasis.sameRendering(expected, MarkdownEmphasis.tokens(of: prefix + text + suffix)) {
            if refuseIfUnbalanced { return nil }
            NSLog("[SCRIVI-EDIT] emphasis could not be balanced at %d — written without emphasis", w0)
            all = all.map { MarkdownEmphasis.Token(unit: $0.unit, style: 0, isPrefix: $0.isPrefix) }
            (text, ends) = MarkdownEmphasis.serialize(all)
        }
        let lastInserted = left.count + inserted.count - 1
        let caret = lastInserted >= 0 && !ends.isEmpty ? ends[lastInserted] : 0
        return (NSRange(location: w0, length: w1 - w0), text, caret, ends, left.count)
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
        // The pending pair: typing INTO it makes it a real span (no longer pending); an edit before it moves it; any
        // other edit that touches it ends it.
        if var p = pending, !insertingPending {
            let oldEnd = NSMaxRange(editedRange) - delta
            if editedRange.location == p.caret, oldEnd == p.caret, delta > 0 {
                pending = nil
            } else if oldEnd <= p.range.location {
                p.range.location += delta
                pending = p
            } else if editedRange.location >= NSMaxRange(p.range) {
                // after it: unaffected
            } else {
                pending = nil
            }
        }
        let ns = textStorage.string as NSString
        guard ns.length > 0 else { return }
        let loc = min(editedRange.location, ns.length)
        var span = ns.paragraphRange(for: NSRange(location: loc, length: min(editedRange.length, ns.length - loc)))
        if span.location > 0 {
            span = NSUnionRange(span, ns.paragraphRange(for: NSRange(location: span.location - 1, length: 0)))
        }
        if let b = block(in: textStorage, at: span.location) { span = NSUnionRange(span, b) }
        if span.length > 0, let b = block(in: textStorage, at: NSMaxRange(span) - 1) { span = NSUnionRange(span, b) }
        // ✅ EP-047 S3: the FOLLOWING block too — under the book rule its indent depends on this one (body ⇄ heading).
        // ⚠️ Measured (SP-169 mutation M3): TextKit 2 already re-asks for paragraphs AFTER an edit (their ranges shift), so
        // removing this changed no test. Kept on purpose: that re-asking is observed behaviour, not API.
        if typography.indent == .book, let next = nextBlock(in: textStorage, ns, after: NSMaxRange(span)) {
            span = NSUnionRange(span, next)
        }
        if span != editedRange { textStorage.edited(.editedAttributes, range: span, changeInLength: 0) }
        // EP-046 AC8: the presenter's own per-edit cost (logged only above 0.5 ms, as E1's was).
        let ms = Date().timeIntervalSince(t0) * 1000
        if ms > 0.5 { NSLog(String(format: "[SCRIVI-EDIT] present=%.1f ms (range=%d)", ms, editedRange.length)) }
    }
}
#endif
