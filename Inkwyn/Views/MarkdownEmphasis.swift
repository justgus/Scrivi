import Foundation

// EP-046 E2-S2 (T-0590) — reading a manuscript's emphasis as TOKENS: every UTF-16 unit EXCEPT emphasis markers, each
// carrying its style (italic / bold bits, from `MarkdownBlocks` — Apple's parser, what the screen renders).
//
// ✅ [SP-174] (T-0601): the WRITING half — normalise, serialise with the fewest markers, the balanced edits — moved into
// ScriviCore (`markdown/Emphasis`, `markdown/Editing`), shared with Linux. What stays here reads: the Navigator's search
// text (`searchable`), and the tokens Apple's tests read results back with.
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
        for li in a.listItems { for k in li.prefix.location..<NSMaxRange(li.prefix) { prefixUnits.insert(k) } }
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

    /// A whole scene as the writer SEES it, for SEARCH — the Navigator's filter ([SP-164] live pass: a query with a period
    /// found nothing, the stored text being `Mr\.`): no emphasis markers, no heading prefixes, no escape backslashes.
    /// ⚠️ A list prefix is VISIBLE, so it stays (as in the find bar's `PresentedText`). Text with none of `\ * _ #` is
    /// already what the writer sees and is returned as is.
    static func searchable(_ source: String) -> String {
        guard source.utf16.contains(where: { $0 == 0x5C || $0 == 0x2A || $0 == 0x5F || $0 == 0x23 }) else { return source }
        let u = Array(source.utf16)
        var out: [UInt16] = []
        out.reserveCapacity(u.count)
        var p = 0
        for b in blocks(of: u) {
            out += u[p..<b.location]
            let a = MarkdownBlocks.analyze(String(utf16CodeUnits: Array(u[b.location..<NSMaxRange(b)]), count: b.length))
            var hidden = Set<Int>()
            for m in a.markers { hidden.formUnion(m.range.location..<NSMaxRange(m.range)) }
            for h in a.headings { hidden.formUnion(h.prefix.location..<NSMaxRange(h.prefix)) }
            for k in 0..<b.length where !hidden.contains(k) { out.append(u[b.location + k]) }
            p = NSMaxRange(b)
        }
        out += u[p...]
        return MarkdownEscapes.map(String(utf16CodeUnits: out, count: out.count)).presented
    }
}

/// EP-046 E2-S3 ([SP-163]) — the Format menu's commands (Q-E2-2). Cross-platform: the menu and the session
/// carry it; the macOS manuscript view performs it.
enum ManuscriptFormat: Equatable, Sendable {
    case bold, italic
    case heading(Int)      // 1…3
    case body
    case bulletList, numberedList
}
