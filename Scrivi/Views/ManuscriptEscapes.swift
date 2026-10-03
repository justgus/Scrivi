import Foundation

// EP-045 AC3 — the two coordinate spaces of one manuscript fragment.
//
// ✅ SOURCE    = every stored character, escape backslashes included. The save path,
//               `byteOffset(charOffset:in:)`, EP-019 undo and ScriviCore all live here.
// ✅ PRESENTED = what the writer sees and lands on: the fragment with each ESCAPING
//               backslash removed (`\*` presents as `*`).
//
// ⚠️ OFFSETS ARE UTF-16 CODE UNITS, because the caret and every selection are `NSRange`s.
// ⛔ The design's sketch (§4.2) indexed by `Character`, which disagrees with `NSRange` the
// moment a fragment holds a multi-unit character (👋 is ONE Character, TWO units). ✅ Every
// escapable mark and the backslash are single UTF-16 units, so scanning units is exact.
//
// ⛔ PER FRAGMENT, NEVER DOCUMENT-WIDE (design §4.2): a table over a 1.85 MB manuscript rebuilt
// per keystroke is [I-0196]'s hang class.
enum MarkdownEscapes {

    /// ✅ EP-045 AC8 — THE ONE parsing mode for the manuscript, CONFIRMED by the user 2026-10-03
    /// (*"confirm .full for AC8"*). ⛔ Every parse of manuscript Markdown uses this; ⛔ the study
    /// had been quoting two modes that disagree on whitespace (§4C.2).
    /// ✅ `.full` because R2 makes existing `##` lines INTENDED headings and AC7 suppresses
    /// BLOCK intents — the inline-only modes see neither. ⚠️ `.full` collapses whitespace, ✅ but
    /// under R3 = (c) the screen shows STORAGE and the parser only decides attributes, so the
    /// parser's string is never displayed (design §4.5, measured).
    static let interpretedSyntax: AttributedString.MarkdownParsingOptions.InterpretedSyntax = .full

    /// ✅ The ruled set — all 32 ASCII punctuation marks (study §4B.4, user: *"use the READ
    /// set"*). ⚠️ ONE list serves BOTH directions: what typing escapes is exactly what
    /// presentation un-escapes, so the two can never drift.
    static let escapable: Set<UInt16> = Set(##"!"#$%&'()*+,-./:;<=>?@[\]^_`{|}~"##.utf16)

    private static let backslash: UInt16 = 0x5C
    private static let newline: UInt16 = 0x0A

    /// True when the backslash at `i` is HIDDEN when presented: it escapes one of the 32, or
    /// it is a CommonMark HARD LINE BREAK (a backslash immediately before a newline).
    /// ⚠️ The second case was found by the AC3 oracle (2026-10-03): Apple's parser presents
    /// `foo\⏎bar` as `foo⏎bar`. Typing never produces it (a typed backslash is itself escaped),
    /// but existing scene files may (R2: no escape pass), so the map agrees with the parser there.
    private static func hidesBackslash(_ u: [UInt16], at i: Int) -> Bool {
        u[i] == backslash && i + 1 < u.count && (escapable.contains(u[i + 1]) || u[i + 1] == newline)
    }

    /// Boundary maps for one fragment. Indices are caret BOUNDARIES (0...length), not
    /// characters, so both arrays carry a final end-of-fragment entry.
    struct Map: Equatable {
        /// `presentedToSource[p]` = the source boundary for presented boundary `p`.
        /// ⚠️ For an escaped character this is BEFORE ITS BACKSLASH, so text inserted at the
        /// caret never lands between `\` and the mark it escapes.
        let presentedToSource: [Int]
        /// `sourceToPresented[s]` = the presented boundary for source boundary `s`. ⚠️ The
        /// boundary BETWEEN `\` and its mark (unreachable by the caret) snaps to before the mark.
        let sourceToPresented: [Int]
        /// The fragment as the writer sees it.
        let presented: String

        var isIdentity: Bool { presentedToSource.count == sourceToPresented.count }
    }

    /// Scan one SOURCE fragment.
    static func map(_ source: String) -> Map {
        let u = Array(source.utf16)
        let n = u.count
        var p2s: [Int] = []
        p2s.reserveCapacity(n + 1)
        var s2p = [Int](repeating: 0, count: n + 1)
        var shown: [UInt16] = []
        shown.reserveCapacity(n)
        var i = 0
        while i < n {
            let p = p2s.count
            if hidesBackslash(u, at: i) {
                p2s.append(i)          // the caret sits before the backslash
                s2p[i] = p
                s2p[i + 1] = p         // between `\` and its mark → before the mark
                shown.append(u[i + 1]) // the mark shows; the backslash does not
                i += 2
            } else {
                p2s.append(i)
                s2p[i] = p
                shown.append(u[i])
                i += 1
            }
        }
        p2s.append(n)
        s2p[n] = p2s.count - 1
        return Map(presentedToSource: p2s, sourceToPresented: s2p,
                   presented: String(utf16CodeUnits: shown, count: shown.count))
    }

    /// The WRITE half: what typed text becomes in storage — every escapable mark gets a
    /// backslash. ⚠️ Not yet wired to typing: that is AC4, blocked on ruling R3 (how the
    /// backslash is hidden). `map(escape(t)).presented == t` for every `t`.
    static func escape(_ typed: String) -> String {
        var out: [UInt16] = []
        out.reserveCapacity(typed.utf16.count)
        for c in typed.utf16 {
            if escapable.contains(c) { out.append(backslash) }
            out.append(c)
        }
        return String(utf16CodeUnits: out, count: out.count)
    }

    // MARK: — R3 = (c): the caret never rests inside a hidden escape (ruled 2026-10-03)

    /// Marks a backslash that is HIDDEN by storage attributes (R3 = (c)). ✅ The ONE test for
    /// "is this backslash hidden?" — the snap below reads only this key, never the styling.
    /// ⚠️ Set by the hiding styler, which lands with AC4's wiring; until then no character
    /// carries it and every snap below is a no-op.
    static let hiddenKey = NSAttributedString.Key("scrivi.hiddenEscape")

    /// True when boundary `s` sits between a hidden backslash and the mark it escapes — a spot
    /// that LOOKS identical to the boundary before the backslash (measured: x 35.32 vs 35.33).
    static func isUnreachable(_ s: Int, in storage: NSAttributedString) -> Bool {
        s > 0 && s < storage.length && storage.attribute(hiddenKey, at: s - 1, effectiveRange: nil) != nil
    }

    /// Where a CARET proposed at `loc` (coming from `previous`) must land instead, or nil to
    /// leave it. ✅ A forward STEP from just before the backslash (→) continues past the mark;
    /// ✅ everything else — ←, a click, a jump — lands BEFORE the backslash, the visually
    /// identical spot. ⚠️ Measured 2026-10-03: a click there hit-tests to the unreachable
    /// boundary, and snapping it forward put the caret one character right of the click.
    static func snapCaret(_ loc: Int, from previous: Int, in storage: NSAttributedString) -> Int? {
        guard isUnreachable(loc, in: storage) else { return nil }
        return previous == loc - 1 ? loc + 1 : loc - 1
    }

    /// A SELECTION never splits a hidden backslash from its mark: its start moves before the
    /// backslash, its end after the mark. nil when neither end needs moving.
    static func snapSelection(_ r: NSRange, in storage: NSAttributedString) -> NSRange? {
        var start = r.location, end = r.location + r.length
        if isUnreachable(start, in: storage) { start -= 1 }
        if isUnreachable(end, in: storage) { end += 1 }
        guard start != r.location || end != r.location + r.length else { return nil }
        return NSRange(location: start, length: end - start)
    }
}
