import Foundation

// EP-046 (T-0588, T-0590) — the BLOCK ANALYZER (design `Scrivi_Manuscript_Renderer_E2_Design_v0_1.md` §3.1).
//
// A BLOCK is a maximal run of non-blank lines of scene text. Markdown cannot carry a heading or
// emphasis across a blank line, so a block is the whole context the parser needs.
// ✅ The PARSER decides (study §4A.3: Apple's parser won Q7(b) on correctness). This file only
// turns what it reports into UTF-16 ranges.
enum MarkdownBlocks {

    /// One ATX heading line. Ranges are UTF-16 offsets RELATIVE TO THE BLOCK.
    struct Heading: Equatable {
        /// The heading's line, without its newline.
        let line: NSRange
        /// The `#…# ` prefix that is hidden unless the caret is on the line (Q-E2-1, line half).
        let prefix: NSRange
        let level: Int
    }

    /// An emphasis DELIMITER run (`*`, `**`, `_`, `***`…) the parser used. ✅ E2-S2: classified as
    /// OPENING (the caret's home is AFTER it — [SP-162] Q1) or closing (home BEFORE it).
    struct Marker: Equatable {
        let range: NSRange
        let opens: Bool
    }

    /// Inline style bits, per UTF-16 unit.
    static let italic: UInt8 = 1
    static let bold: UInt8 = 2

    struct Analysis: Equatable {
        var headings: [Heading] = []
        var markers: [Marker] = []
        /// Style bits for every UTF-16 unit of the block; empty when the block has no emphasis.
        /// Marker units carry the style of the text they enclose; never displayed.
        var styles: [UInt8] = []
        /// Maximal stretches of emphasis — content AND its markers (the span reveal, Q-E2-1).
        var spans: [NSRange] = []

        func style(at i: Int) -> UInt8 { styles.isEmpty ? 0 : styles[i] }
        func marker(containing i: Int) -> Marker? { markers.first { NSLocationInRange(i, $0.range) } }
    }

    // AC8 (EP-045): the ONE parsing mode. Source positions give each run its source range.
    private static let options = AttributedString.MarkdownParsingOptions(
        interpretedSyntax: MarkdownEscapes.interpretedSyntax,
        failurePolicy: .returnPartiallyParsedIfPossible,
        appliesSourcePositionAttributes: true)

    /// An ATX prefix: up to 3 spaces of indent, 1–6 `#`, then whitespace or the end of the line.
    private static let atxPrefix = try! NSRegularExpression(pattern: "^ {0,3}#{1,6}(?:[ \\t]+|$)")

    private static let star: UInt16 = 0x2A, underscore: UInt16 = 0x5F, hash: UInt16 = 0x23

    static func analyze(_ block: String) -> Analysis {
        analyze(block, headings: true)
    }

    private static func analyze(_ block: String, headings allowHeadings: Bool) -> Analysis {
        var out = Analysis()
        let u = Array(block.utf16)
        let n = u.count
        // Fast path: no `#`, `*` or `_`, nothing to render. ⚠️ An escaped `\#` or `\*` still contains
        // the character, so it is parsed and the parser rejects it — the fast path decides nothing
        // the parser would not.
        guard u.contains(where: { $0 == hash || $0 == star || $0 == underscore }),
              let parsed = try? AttributedString(markdown: block, options: options) else { return out }

        // UTF-8 (line, column) → UTF-16 offset (Q7(b): columns are UTF-8 bytes, `endColumn` inclusive).
        let lines = block.split(separator: "\n", omittingEmptySubsequences: false).map { Array($0.utf8) }
        var lineStarts: [Int] = []
        var acc = 0
        for l in block.split(separator: "\n", omittingEmptySubsequences: false) {
            lineStarts.append(acc); acc += l.utf16.count + 1
        }
        func offset(_ line: Int, _ column: Int, after: Bool) -> Int {
            let li = max(0, min(line - 1, lines.count - 1))
            let bytes = lines[li]
            var b = max(0, min(column - 1, bytes.count))
            if after, b < bytes.count {
                b += 1
                while b < bytes.count, bytes[b] & 0xC0 == 0x80 { b += 1 }
            }
            return lineStarts[li] + String(decoding: bytes[0..<b], as: UTF8.self).utf16.count
        }

        var covered = [Bool](repeating: false, count: n)
        var styles = [UInt8](repeating: 0, count: n)
        var levels: [Int: Int] = [:]          // 1-based source line → heading level
        var codeBlock = false, table = false
        for run in parsed.runs {
            var level = 0, nested = false
            if let intent = run.presentationIntent {
                for c in intent.components {
                    switch c.kind {
                    case .header(let l): level = l
                    case .codeBlock: codeBlock = true; nested = true
                    case .table, .tableHeaderRow, .tableRow, .tableCell: table = true; nested = true
                    case .blockQuote, .listItem, .orderedList, .unorderedList: nested = true
                    default: break
                    }
                }
            }
            guard let pos = run.markdownSourcePosition else { continue }
            // ✅ EP-045 AC7 / Q-AC7 = (a): a heading inside a code block, quote, list or table is NOT rendered.
            if level > 0, !nested { levels[pos.startLine] = level }
            let s = offset(pos.startLine, pos.startColumn, after: false)
            let e = min(offset(pos.endLine, pos.endColumn, after: true), n)
            guard e > s else { continue }
            var bits: UInt8 = 0
            if let ii = run.inlinePresentationIntent {
                if ii.contains(.emphasized) { bits |= italic }
                if ii.contains(.stronglyEmphasized) { bits |= bold }
            }
            for i in s..<e { covered[i] = true; styles[i] = bits }
        }
        // ✅ AC7: an INDENTED block is prose — re-read with its indentation stripped, inline only.
        if codeBlock { return demoteIndented(block) }
        // ✅ AC7: a TABLE is drawn exactly as stored.
        if table { return out }

        if allowHeadings, !levels.isEmpty {
            for (li, start) in lineStarts.enumerated() {
                guard let level = levels[li + 1] else { continue }
                let len = (li + 1 < lineStarts.count ? lineStarts[li + 1] - 1 : n) - start
                let line = NSRange(location: start, length: len)
                // ⚠️ Only an ATX line: a SETEXT heading has no prefix to hide, so it is drawn as stored.
                if let m = atxPrefix.firstMatch(in: block, range: line) {
                    out.headings.append(Heading(line: line, prefix: m.range, level: level))
                }
            }
        }

        guard styles.contains(where: { $0 != 0 }) else { return out }

        // The style a position sees on each side: the nearest COVERED unit, skipping uncovered
        // non-whitespace (link syntax, a neighbouring delimiter) but stopping at whitespace/newlines.
        func side(_ from: Int, _ step: Int) -> UInt8 {
            var i = from
            while i >= 0, i < n {
                if covered[i] { return styles[i] }
                let c = u[i]
                if c == 0x20 || c == 0x09 || c == 0x0A { return 0 }
                i += step
            }
            return 0
        }
        // Delimiter runs: maximal runs of UNCOVERED `*` / `_`. ✅ A run is a marker only when the style
        // differs across it — an uncovered `*` bullet (`* item`) does not, and stays visible.
        var i = 0
        while i < n {
            guard !covered[i], u[i] == star || u[i] == underscore else { i += 1; continue }
            var j = i
            while j < n, !covered[j], u[j] == star || u[j] == underscore { j += 1 }
            let left = side(i - 1, -1), right = side(j, 1)
            if left != right {
                let opens = (right & ~left) != 0 && (left & ~right) == 0
                out.markers.append(Marker(range: NSRange(location: i, length: j - i), opens: opens))
                for k in i..<j { styles[k] = left | right }
            }
            i = j
        }
        // Uncovered units that are not markers take the style both sides share (a link's `[`, a
        // soft break) — so a link inside bold is bold.
        let markerUnits = Set(out.markers.flatMap { $0.range.location..<NSMaxRange($0.range) })
        for k in 0..<n where !covered[k] && !markerUnits.contains(k) {
            styles[k] = side(k - 1, -1) & side(k + 1, 1)
        }
        out.styles = styles
        // Spans: maximal stretches of (marker ∪ styled), each containing a marker.
        var k = 0
        while k < n {
            guard styles[k] != 0 || markerUnits.contains(k) else { k += 1; continue }
            var j = k
            var hasMarker = false
            while j < n, styles[j] != 0 || markerUnits.contains(j) {
                if markerUnits.contains(j) { hasMarker = true }
                j += 1
            }
            if hasMarker { out.spans.append(NSRange(location: k, length: j - k)) }
            k = j
        }
        return out
    }

    /// AC7: an indented block is prose. Strip each line's indentation, analyse INLINE only (no
    /// heading), and shift the results back.
    private static func demoteIndented(_ block: String) -> Analysis {
        var stripped: [String] = []
        var shifts: [(dst: Int, delta: Int)] = []
        var src = 0, dst = 0
        for line in block.split(separator: "\n", omittingEmptySubsequences: false) {
            let lead = line.utf16.prefix { $0 == 0x20 || $0 == 0x09 }.count
            let rest = String(String(line).utf16.dropFirst(lead))!
            shifts.append((dst, src + lead - dst))
            stripped.append(rest)
            src += line.utf16.count + 1
            dst += rest.utf16.count + 1
        }
        let inner = analyze(stripped.joined(separator: "\n"), headings: false)
        func map(_ i: Int) -> Int { i + (shifts.last { $0.dst <= i }?.delta ?? 0) }
        func map(_ r: NSRange) -> NSRange { NSRange(location: map(r.location), length: r.length) }
        var out = Analysis()
        out.markers = inner.markers.map { Marker(range: map($0.range), opens: $0.opens) }
        out.spans = inner.spans.map(map)
        if !inner.styles.isEmpty {
            out.styles = [UInt8](repeating: 0, count: block.utf16.count)
            for (i, s) in inner.styles.enumerated() where s != 0 { out.styles[map(i)] = s }
        }
        return out
    }
}
