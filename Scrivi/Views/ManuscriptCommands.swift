import Foundation
#if os(macOS)
import AppKit

// EP-046 E2-S3 ([SP-163], T-0592 + T-0591) — THE FORMAT COMMANDS, as edits the presenter computes.
//
// ✅ Under the escape ruling (study §3A.0) these are the ONLY way formatting enters a manuscript. Each returns ONE
// edit (range + replacement) that the view applies as one history event, or nil to refuse.
// ✅ Bold / italic are STYLE edits on E2-S2's tokens (`MarkdownEmphasis`), written back balanced — so whitespace at
// the edges is left out (the user's rule 4), `*` is used, never `_`, a selection across paragraphs is formatted per
// paragraph, and toggling part of a span off splits it.
extension ManuscriptPresenter {

    typealias CommandEdit = (range: NSRange, replacement: String, selection: NSRange)

    // MARK: — Words

    static func isWordUnit(_ c: UInt16) -> Bool {
        guard let scalar = Unicode.Scalar(c) else { return true }     // a surrogate half: part of a letter
        return CharacterSet.alphanumerics.contains(scalar)
    }

    private static func isPunctUnit(_ c: UInt16) -> Bool {
        guard let scalar = Unicode.Scalar(c) else { return false }
        return CharacterSet.punctuationCharacters.contains(scalar) || CharacterSet.symbols.contains(scalar)
    }

    /// The word the caret is IN (touching a letter or digit on either side), or nil BETWEEN words ([SP-163] Q1).
    /// ✅ An escaped apostrophe or hyphen inside a word (`don\'t`, `well\-known`) keeps it one word.
    func wordRange(in ts: NSAttributedString, at caret: Int) -> NSRange? {
        let ns = ts.string as NSString
        let n = ns.length
        func word(_ i: Int) -> Bool { i >= 0 && i < n && Self.isWordUnit(ns.character(at: i)) }
        guard word(caret - 1) || word(caret) else { return nil }
        var s = caret, e = caret
        while true {
            if word(s - 1) { s -= 1; continue }
            if s >= 3, Self.isPunctUnit(ns.character(at: s - 1)), ns.character(at: s - 2) == 0x5C, word(s - 3), word(s) { s -= 2; continue }
            break
        }
        while true {
            if word(e) { e += 1; continue }
            if e + 2 < n, ns.character(at: e) == 0x5C, Self.isPunctUnit(ns.character(at: e + 1)), word(e + 2), word(e - 1) { e += 2; continue }
            break
        }
        return NSRange(location: s, length: e - s)
    }

    // MARK: — Bold / Italic

    /// Toggle `bit` (bold or italic) on the visible text of `selection`: SET it when any visible selected character
    /// lacks it, CLEAR it when all have it. ✅ [SP-163] Q3: an edge that cannot carry it (punctuation glued to a
    /// letter) is shrunk away — E2-S2's rule. `caret`: an offset to map into the result (the caret when a WORD was
    /// formatted for it, Q1); the result's selection is then that caret, else the formatted text.
    func emphasisEdit(in ts: NSAttributedString, selection sel: NSRange, bit: UInt8, caret: Int? = nil) -> CommandEdit? {
        guard sel.length > 0 else { return nil }
        let mid = tokens(in: ts, sel)
        let visible = mid.filter { !$0.isPrefix && !Self.isSpaceUnit($0.unit) }
        guard !visible.isEmpty else { return nil }
        let allHave = visible.allSatisfy { $0.style & bit != 0 }
        let changed = mid.map { t -> MarkdownEmphasis.Token in
            var t = t
            if !t.isPrefix { t.style = allHave ? (t.style & ~bit) : (t.style | bit) }
            return t
        }
        let a = sel.location, b = NSMaxRange(sel)
        guard let r = rewrite(in: ts, replacing: sel, with: changed, spans: spans(in: ts, from: a, to: b),
                              refuseIfUnbalanced: true) else { return nil }
        // The selection after: the formatted VISIBLE text, without the edge whitespace the span left out.
        let visibleIdx = changed.indices.filter { !Self.isSpaceUnit(changed[$0].unit) && !changed[$0].isPrefix }
        let first = r.ends[r.leftCount + visibleIdx.first!] - 1
        let last = r.ends[r.leftCount + visibleIdx.last!]
        var selection = NSRange(location: r.range.location + first, length: last - first)
        if let caret {
            // The caret sat after `k` tokens of the word: put it after the same `k` in the result.
            let k = tokens(in: ts, NSRange(location: a, length: max(0, min(caret, b) - a))).count
            let at = k == 0 ? first : r.ends[r.leftCount + k - 1]
            selection = NSRange(location: r.range.location + at, length: 0)
        }
        return (r.range, r.text, selection)
    }

    private static func isSpaceUnit(_ c: UInt16) -> Bool { c == 0x20 || c == 0x09 || c == 0x0A }

    // MARK: — Headings, body, lists (Q5: a paragraph is ONE of body / heading / list item)

    private enum ParagraphKind: Equatable { case body, heading(Int), bullet, numbered }

    /// The paragraph kind of a block (its first line) and that line's prefix (storage range), if any.
    private func kind(of b: NSRange, _ info: BlockInfo) -> (kind: ParagraphKind, prefix: NSRange?) {
        if let h = info.headings.first(where: { $0.line.location == 0 }) {
            return (.heading(h.level), NSRange(location: b.location + h.prefix.location, length: h.prefix.length))
        }
        if let li = info.analysis.listItems.first(where: { $0.line.location == 0 }) {
            return (li.ordered ? .numbered : .bullet, NSRange(location: b.location + li.prefix.location, length: li.prefix.length))
        }
        return (.body, nil)
    }

    /// Set every paragraph the selection touches to `format`'s kind; the same command again on paragraphs that all
    /// have it already returns them to body text. ✅ [SP-163] Q4: numbered items are kept SEQUENTIAL — the items
    /// that follow in the same list are renumbered in the same edit.
    func paragraphEdit(in ts: NSAttributedString, selection sel: NSRange, format: ManuscriptFormat) -> CommandEdit? {
        let target: ParagraphKind
        switch format {
        case .heading(let n): target = .heading(max(1, min(6, n)))
        case .body: target = .body
        case .bulletList: target = .bullet
        case .numberedList: target = .numbered
        case .bold, .italic: return nil
        }
        let ns = ts.string as NSString
        let targets = blocks(in: ts, from: sel.location, to: max(sel.location, NSMaxRange(sel) - (sel.length > 0 ? 1 : 0)))
        guard !targets.isEmpty else { return nil }
        let kinds = targets.map { kind(of: $0.range, $0.info) }
        let newKind: ParagraphKind = kinds.allSatisfy { $0.kind == target } && target != .body ? .body : target

        // The edit runs from the first target to the end of the list that follows it (renumbering), and starts
        // numbering after an ordered item that precedes it.
        var all = targets.map { (range: $0.range, kind: kind(of: $0.range, $0.info), isTarget: true) }
        var p = NSMaxRange(all.last!.range)
        while p < ns.length {
            guard let next = nextBlock(in: ts, after: p), NSMaxRange(next.range) > p else { break }
            let k = kind(of: next.range, next.info)
            guard k.kind == .numbered else { break }
            all.append((next.range, k, false))
            p = NSMaxRange(next.range)
        }
        var counter = 1
        if let prev = previousBlock(in: ts, before: all[0].range.location),
           let li = prev.info.analysis.listItems.first(where: { $0.line.location == 0 }), li.ordered {
            counter = li.number + 1
        }
        let start = all[0].range.location
        let end = NSMaxRange(all.last!.range)
        let out = NSMutableString(string: ns.substring(with: NSRange(location: start, length: end - start)))
        var deltaBefore: [(at: Int, delta: Int)] = []
        // Back to front, so earlier offsets stay valid; numbers are assigned front to back first.
        var numbers: [Int?] = []
        for item in all {
            let k = item.isTarget ? newKind : item.kind.kind
            if k == .numbered { numbers.append(counter); counter += 1 } else { numbers.append(nil); counter = 1 }
        }
        for (i, item) in all.enumerated().reversed() {
            let k = item.isTarget ? newKind : item.kind.kind
            let prefix: String
            switch k {
            case .body: prefix = ""
            case .heading(let n): prefix = String(repeating: "#", count: n) + " "
            case .bullet: prefix = "- "
            case .numbered: prefix = "\(numbers[i] ?? 1). "
            }
            let old = item.kind.prefix ?? NSRange(location: item.range.location, length: 0)
            let local = NSRange(location: old.location - start, length: old.length)
            out.replaceCharacters(in: local, with: prefix)
            deltaBefore.append((old.location, (prefix as NSString).length - old.length))
        }
        // The selection keeps its text: shift it by the prefix changes before (and inside) it.
        func map(_ x: Int) -> Int { x + deltaBefore.filter { $0.at <= x }.map(\.delta).reduce(0, +) }
        let selection = NSRange(location: map(sel.location), length: max(0, map(NSMaxRange(sel)) - map(sel.location)))
        return (NSRange(location: start, length: end - start), out as String, selection)
    }

    private func nextBlock(in ts: NSAttributedString, after p: Int) -> (range: NSRange, info: BlockInfo)? {
        let ns = ts.string as NSString
        var q = p
        while q < ns.length {
            let c = ns.character(at: q)
            if c != 0x0A && c != 0x20 && c != 0x09 { break }
            q += 1
        }
        guard q < ns.length, let b = block(in: ts, at: q) else { return nil }
        return (b, info(ns.substring(with: b)))
    }

    private func previousBlock(in ts: NSAttributedString, before p: Int) -> (range: NSRange, info: BlockInfo)? {
        let ns = ts.string as NSString
        var q = p - 1
        while q >= 0 {
            let c = ns.character(at: q)
            if c != 0x0A && c != 0x20 && c != 0x09 { break }
            q -= 1
        }
        guard q >= 0, let b = block(in: ts, at: q) else { return nil }
        return (b, info(ns.substring(with: b)))
    }

    // MARK: — Return in a list item

    /// Return with the caret in a list item: on an EMPTY item, end the list (remove its prefix); otherwise start the
    /// next item — `\n\n` + the next prefix — and renumber the items that follow (Q4). The text after the caret
    /// moves into the new item; emphasis around the caret is split balanced (E2-S2). nil when not in a list item.
    func listReturnEdit(in ts: NSAttributedString, selection sel: NSRange) -> CommandEdit? {
        guard sel.length == 0, let b = block(in: ts, at: sel.location) else { return nil }
        let ns = ts.string as NSString
        let info = info(ns.substring(with: b))
        guard let li = info.analysis.listItems.first(where: { $0.line.location == 0 }) else { return nil }
        let prefix = NSRange(location: b.location + li.prefix.location, length: li.prefix.length)
        let caret = sel.location
        guard caret >= NSMaxRange(prefix) else { return nil }
        let content = ns.substring(with: NSRange(location: NSMaxRange(prefix), length: NSMaxRange(b) - NSMaxRange(prefix)))
        if content.trimmingCharacters(in: .whitespacesAndNewlines).isEmpty {
            // ✅ Q4: the ordered items after it close the gap — Markdown still reads them as the same list.
            let (tail, end) = li.ordered ? renumbered(in: ts, after: b, from: NSMaxRange(prefix), startingAt: li.number)
                                         : ("", NSMaxRange(prefix))
            return (NSRange(location: prefix.location, length: end - prefix.location), tail,
                    NSRange(location: prefix.location, length: 0))
        }
        let bulletText = ns.substring(with: prefix).trimmingCharacters(in: .whitespaces)
        let nextPrefix = li.ordered ? "\(li.number + 1)\(bulletText.last.map(String.init) ?? ".") " : "\(bulletText) "
        // The break itself, through the balancing rewrite (a Return inside bold closes and re-opens it).
        let breakTokens = Array("\n\n".utf16).map { MarkdownEmphasis.Token(unit: $0, style: 0) }
            + Array(nextPrefix.utf16).map { MarkdownEmphasis.Token(unit: $0, style: 0, isPrefix: true) }
        guard let r = rewrite(in: ts, replacing: NSRange(location: caret, length: 0), with: breakTokens,
                              spans: spans(in: ts, from: caret, to: caret), refuseIfUnbalanced: false) else { return nil }
        var replacement = r.text
        var end = NSMaxRange(r.range)
        // Renumber the ordered items that follow, in the same edit (Q4).
        if li.ordered {
            let (tail, newEnd) = renumbered(in: ts, after: b, from: end, startingAt: li.number + 2)
            replacement += tail
            end = newEnd
        }
        return (NSRange(location: r.range.location, length: end - r.range.location), replacement,
                NSRange(location: r.range.location + r.caret, length: 0))
    }

    /// The text from `from` to the end of the ordered items that follow block `b`, with those items renumbered from
    /// `number` — and where that text ends.
    private func renumbered(in ts: NSAttributedString, after b: NSRange, from: Int, startingAt number: Int) -> (String, Int) {
        let ns = ts.string as NSString
        var number = number
        var end = from
        var p = NSMaxRange(b)
        var tail = ""
        while let next = nextBlock(in: ts, after: p),
              let nli = next.info.analysis.listItems.first(where: { $0.line.location == 0 }), nli.ordered {
            tail += ns.substring(with: NSRange(location: end, length: next.range.location - end))
            let text = NSMutableString(string: ns.substring(with: next.range))
            text.replaceCharacters(in: nli.prefix, with: "\(number). ")
            tail += text as String
            number += 1
            end = NSMaxRange(next.range)
            p = end
        }
        return (tail, end)
    }

    // MARK: — [T-0591] across SCENE boundaries (ruled 2026-10-05: Apple-side)

    /// The style at the caret: what typing there would take (E2-S2's insertion style).
    func styleAtCaret(in ts: NSAttributedString, _ caret: Int) -> UInt8 {
        insertionStyle(in: ts, at: caret, selected: [])
    }

    /// A cross-scene paste lands INSIDE the caret's span, then the scene is split there: the FIRST piece starts with
    /// that span open before it — it is CLOSED where the piece's own styles differ; the LAST piece RE-OPENS it for the
    /// text after the split. ✅ [SP-163] live pass (user, 2026-10-05): the pasted text keeps ITS OWN formatting — *"Only
    /// the first word of the pasted section should have been bold."* (Bold pasted into bold still continues it.)
    static func balancePastePieces(_ texts: [String], caretStyle s: UInt8) -> [String] {
        guard s != 0, texts.count >= 2 else { return texts }
        func styled(_ t: String) -> [MarkdownEmphasis.Token] {
            MarkdownEmphasis.normalize(MarkdownEmphasis.tokens(of: t))
        }
        var out = texts
        out[0] = MarkdownEmphasis.serialize(styled(texts[0]), open: s, leaveOpen: 0).text
        let last = texts.count - 1
        out[last] = MarkdownEmphasis.serialize(styled(texts[last]), open: 0, leaveOpen: s).text
        return out
    }
}
#endif
