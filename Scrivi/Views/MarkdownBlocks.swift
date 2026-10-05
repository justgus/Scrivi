import Foundation

// EP-046 E2-S1 (T-0588) — the BLOCK ANALYZER (design `Scrivi_Manuscript_Renderer_E2_Design_v0_1.md` §3.1).
//
// A BLOCK is a maximal run of non-blank lines of scene text. Markdown cannot carry a heading or
// emphasis across a blank line, so a block is the whole context the parser needs.
// ✅ The PARSER decides (study §4A.3: Apple's parser won Q7(b) on correctness). This file only
// turns what it reports into UTF-16 ranges.
//
// ⚠️ E2-S1 renders HEADINGS only. Bold/italic markers are E2-S2's (design §12), so they are not
// computed here yet.
enum MarkdownBlocks {

    /// One ATX heading line. Ranges are UTF-16 offsets RELATIVE TO THE BLOCK.
    struct Heading: Equatable {
        /// The heading's line, without its newline.
        let line: NSRange
        /// The `#…# ` prefix that is hidden unless the caret is on the line (Q-E2-1, line half).
        let prefix: NSRange
        let level: Int
    }

    struct Analysis: Equatable {
        var headings: [Heading] = []
    }

    // AC8 (EP-045): the ONE parsing mode. Source positions give each heading run its LINE.
    private static let options = AttributedString.MarkdownParsingOptions(
        interpretedSyntax: MarkdownEscapes.interpretedSyntax,
        failurePolicy: .returnPartiallyParsedIfPossible,
        appliesSourcePositionAttributes: true)

    /// An ATX prefix: up to 3 spaces of indent, 1–6 `#`, then whitespace or the end of the line.
    private static let atxPrefix = try! NSRegularExpression(pattern: "^ {0,3}#{1,6}(?:[ \\t]+|$)")

    static func analyze(_ block: String) -> Analysis {
        var out = Analysis()
        // Fast path: no `#`, no heading. ⚠️ An escaped `\#` still contains `#`, so it is parsed
        // and the parser rejects it — the fast path never decides anything the parser would not.
        guard block.utf16.contains(0x23), let parsed = try? AttributedString(markdown: block, options: options) else {
            return out
        }
        // 1-based source LINE → heading level. ✅ EP-045 AC7 / Q-AC7 = (a): a heading inside a
        // code block, quote, list or table is NOT rendered — those blocks are drawn as prose.
        var levels: [Int: Int] = [:]
        for run in parsed.runs {
            guard let intent = run.presentationIntent, let pos = run.markdownSourcePosition else { continue }
            var level = 0
            var nested = false
            for c in intent.components {
                switch c.kind {
                case .header(let l): level = l
                case .codeBlock, .blockQuote, .listItem, .orderedList, .unorderedList,
                     .table, .tableHeaderRow, .tableRow, .tableCell: nested = true
                default: break
                }
            }
            if level > 0, !nested { levels[pos.startLine] = level }
        }
        guard !levels.isEmpty else { return out }

        let ns = block as NSString
        var lineStart = 0
        var lineNumber = 1
        while lineStart <= ns.length {
            let nl = ns.range(of: "\n", options: .literal, range: NSRange(location: lineStart, length: ns.length - lineStart))
            let lineEnd = nl.location == NSNotFound ? ns.length : nl.location
            let line = NSRange(location: lineStart, length: lineEnd - lineStart)
            // ⚠️ Only an ATX line: a SETEXT heading (`Title⏎===`) is reported as a header too,
            // but has no prefix to hide, so it is drawn as stored (design §13).
            if let level = levels[lineNumber],
               let m = atxPrefix.firstMatch(in: block, range: line) {
                out.headings.append(Heading(line: line, prefix: m.range, level: level))
            }
            if nl.location == NSNotFound { break }
            lineStart = lineEnd + 1
            lineNumber += 1
        }
        return out
    }
}
