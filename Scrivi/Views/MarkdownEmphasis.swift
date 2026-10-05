import Foundation

// EP-046 E2-S2 (T-0590) — EMPHASIS STAYS BALANCED THROUGH EDITS (EP-046 AC12; [SP-162] plan 5a, Q3).
//
// ✅ User, 2026-10-05: *"a cut there cuts the text and splits the bold section … terminate the bold
// section at the cut point and insert a bold begin at the beginning of the cut text"* — and (Q3) the same
// for every edit that replaces a selection.
//
// ✅ The model: a stretch of source becomes TOKENS — every UTF-16 unit EXCEPT emphasis markers, each
// carrying its style (italic / bold bits, from `MarkdownBlocks`). An edit is applied to tokens, and the
// stretch is written back with the FEWEST markers that give those styles. So a cut that crosses a span
// edge closes and re-opens the span, a span left empty disappears, and a paste of bold text into bold
// text merges instead of toggling.
// ⚠️ Only the stretch an edit touches is rewritten (the caller picks it: the spans the edit crosses);
// markup elsewhere keeps its bytes (EP-045 AC9).
enum MarkdownEmphasis {

    struct Token: Equatable {
        var unit: UInt16
        var style: UInt8
        /// A heading prefix unit — kept in the source, dropped from what the writer SEES (Q2).
        var isPrefix = false
    }

    // MARK: — Reading

    /// The blocks of a standalone fragment: maximal runs of non-blank lines.
    static func blocks(of u: [UInt16]) -> [NSRange] {
        var out: [NSRange] = []
        var i = 0
        let n = u.count
        var blockStart: Int?
        while i < n {
            var j = i
            while j < n, u[j] != 0x0A { j += 1 }
            let blank = u[i..<j].allSatisfy { $0 == 0x20 || $0 == 0x09 }
            let lineEnd = min(j + 1, n)
            if blank {
                if let s = blockStart { out.append(NSRange(location: s, length: i - s)); blockStart = nil }
            } else if blockStart == nil {
                blockStart = i
            }
            i = lineEnd
        }
        if let s = blockStart { out.append(NSRange(location: s, length: n - s)) }
        return out
    }

    /// Tokens of one block, given its analysis (offsets relative to the block).
    static func tokens(of u: ArraySlice<UInt16>, analysis a: MarkdownBlocks.Analysis, from: Int, to: Int) -> [Token] {
        let base = u.startIndex
        var markerUnits = Set<Int>()
        for m in a.markers { for k in m.range.location..<NSMaxRange(m.range) { markerUnits.insert(k) } }
        var prefixUnits = Set<Int>()
        for h in a.headings { for k in h.prefix.location..<NSMaxRange(h.prefix) { prefixUnits.insert(k) } }
        var out: [Token] = []
        for k in from..<to where !markerUnits.contains(k) {
            out.append(Token(unit: u[base + k], style: a.style(at: k), isPrefix: prefixUnits.contains(k)))
        }
        return out
    }

    /// Tokens of a STANDALONE fragment (a paste, a typed run): read block by block.
    static func tokens(of source: String) -> [Token] {
        let u = Array(source.utf16)
        var out: [Token] = []
        var p = 0
        for b in blocks(of: u) {
            while p < b.location { out.append(Token(unit: u[p], style: 0)); p += 1 }
            let text = String(utf16CodeUnits: Array(u[b.location..<NSMaxRange(b)]), count: b.length)
            out += tokens(of: u[b.location..<NSMaxRange(b)], analysis: MarkdownBlocks.analyze(text), from: 0, to: b.length)
            p = NSMaxRange(b)
        }
        while p < u.count { out.append(Token(unit: u[p], style: 0)); p += 1 }
        return out
    }

    /// True when `source` may carry emphasis markers: an `*` or `_` that is not escaped.
    static func mayHaveMarkers(_ source: String) -> Bool {
        var backslashes = 0
        for c in source.utf16 {
            if (c == 0x2A || c == 0x5F), backslashes % 2 == 0 { return true }
            backslashes = c == 0x5C ? backslashes + 1 : 0
        }
        return false
    }

    // MARK: — Writing

    private static func isSpace(_ c: UInt16) -> Bool { c == 0x20 || c == 0x09 || c == 0x0A }

    /// ✅ A formatted span always begins and ends with a VISIBLE character (user rule, 2026-10-05) — and
    /// CommonMark needs it: `** x**` is not emphasis. Whitespace takes the style BOTH neighbours share, so
    /// span edges land on visible characters. ⚠️ A newline never carries a style: a span is closed before a
    /// line break and re-opened after (a Return inside bold splits it into two balanced spans).
    static func normalize(_ tokens: [Token]) -> [Token] {
        var t = tokens
        var i = 0
        while i < t.count {
            guard isSpace(t[i].unit) else { i += 1; continue }
            var j = i
            while j < t.count, isSpace(t[j].unit) { j += 1 }
            let left: UInt8 = i > 0 ? t[i - 1].style : 0
            let right: UInt8 = j < t.count ? t[j].style : 0
            let hasNewline = t[i..<j].contains { $0.unit == 0x0A }
            for k in i..<j { t[k].style = hasNewline ? 0 : (left & right) }
            i = j
        }
        // ⚠️ CommonMark cannot open a span on PUNCTUATION that follows a letter (`x*,*` is not emphasis),
        // nor close one on punctuation that precedes a letter. There the span edge moves past that one
        // punctuation mark (with its escape backslash), which loses its style — the only representable
        // result. Measured: the cases a random edit produces in [SP-162]'s corpus.
        var changed = true
        while changed {
            changed = false
            for k in 1..<max(1, t.count) {
                let before = t[k - 1], here = t[k]
                let opening = here.style & ~before.style, closing = before.style & ~here.style
                if opening != 0, isWordUnit(before.unit), isPunct(here.unit) {
                    var e = k + (here.unit == 0x5C && k + 1 < t.count ? 2 : 1)
                    e = min(e, t.count)
                    for m in k..<e { t[m].style &= ~opening }
                    changed = true
                }
                if closing != 0, isPunct(before.unit), isWordUnit(here.unit) {
                    var s = k - 1
                    if s > 0, t[s - 1].unit == 0x5C { s -= 1 }
                    for m in s..<k { t[m].style &= ~closing }
                    changed = true
                }
            }
        }
        return t
    }

    private static func isPunct(_ c: UInt16) -> Bool {
        (0x21...0x2F).contains(c) || (0x3A...0x40).contains(c) || (0x5B...0x60).contains(c) || (0x7B...0x7E).contains(c)
            || (0x2010...0x205E).contains(c)   // general punctuation (dashes, quotes, …)
    }
    private static func isWordUnit(_ c: UInt16) -> Bool { !isSpace(c) && !isPunct(c) }

    private static let boldMarker = Array("**".utf16), italicMarker = Array("*".utf16)

    /// Write tokens back with the fewest markers. `ends[i]` is the output offset just after token `i`.
    /// ✅ Spans nest properly: a style that ends inside another closes the inner ones first and re-opens
    /// them. ✅ When two styles open together, the one that LASTS LONGER opens outside, so the shorter one
    /// can close without closing (and re-opening) the other — `***a* b**`, not `***a****b**`.
    /// ✅ Italic is written `*`, never `_` (Q-E2-2: `_` cannot open inside a word — measured 286/2000).
    static func serialize(_ tokens: [Token]) -> (text: String, ends: [Int]) {
        var out: [UInt16] = []
        out.reserveCapacity(tokens.count + 8)
        var ends: [Int] = []
        var stack: [UInt8] = []
        func persists(_ bit: UInt8, from i: Int) -> Int {
            var j = i
            while j < tokens.count, tokens[j].style & bit != 0 { j += 1 }
            return j - i
        }
        func move(to next: UInt8, at i: Int, opens: Bool = true) {
            if let k = stack.firstIndex(where: { next & $0 == 0 }) {
                for bit in stack[k...].reversed() { out += bit == MarkdownBlocks.bold ? boldMarker : italicMarker }
                stack.removeSubrange(k...)
            }
            guard opens else { return }
            let opening = [MarkdownBlocks.bold, MarkdownBlocks.italic]
                .filter { next & $0 != 0 && !stack.contains($0) }
                .sorted { persists($0, from: i) > persists($1, from: i) }
            for bit in opening {
                out += bit == MarkdownBlocks.bold ? boldMarker : italicMarker
                stack.append(bit)
            }
        }
        for (i, t) in tokens.enumerated() {
            // ⚠️ A span never OPENS on whitespace (`** x` is not emphasis): a style that has to re-open
            // after an inner span closes waits for the next visible character. Whitespace renders the
            // same either way.
            move(to: t.style, at: i, opens: !isSpace(t.unit))
            out.append(t.unit)
            ends.append(out.count)
        }
        move(to: 0, at: tokens.count)
        return (String(utf16CodeUnits: out, count: out.count), ends)
    }

    /// What the writer SEES of these tokens: no markers, no heading prefix, no escape backslashes (Q2).
    static func presented(_ tokens: [Token]) -> String {
        let u = tokens.filter { !$0.isPrefix }.map(\.unit)
        return MarkdownEscapes.map(String(utf16CodeUnits: u, count: u.count)).presented
    }

    /// Token lists agree when their units match and every VISIBLE unit has the same style.
    static func sameRendering(_ a: [Token], _ b: [Token]) -> Bool {
        guard a.count == b.count else { return false }
        for (x, y) in zip(a, b) {
            if x.unit != y.unit { return false }
            if !isSpace(x.unit), x.style != y.style { return false }
        }
        return true
    }
}
