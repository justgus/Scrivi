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
    private static let space: UInt16 = 0x20
    private static let tab: UInt16 = 0x09

    /// True when the backslash at `i` is HIDDEN when presented: it escapes one of the 32, or
    /// it is a CommonMark HARD LINE BREAK (a backslash immediately before a newline).
    /// ⚠️ The second case was found by the AC3 oracle (2026-10-03): Apple's parser presents
    /// `foo\⏎bar` as `foo⏎bar`. Typing never produces it (a typed backslash is itself escaped),
    /// but existing scene files may (R2: no escape pass), so the map agrees with the parser there.
    /// ✅ EP-045 AC6 / Q1 = (a), MEASURED 2026-10-04 under `.full`: a hard break cannot END a
    /// paragraph, so a backslash before a BLANK line (or the end of the text) stays LITERAL —
    /// `end.\⏎⏎next.` presents `end.\`. AC6's Enter writes exactly that form.
    /// `continues` says whether a non-blank line follows when the `\n` is the LAST character
    /// of `u` (the styler maps one line at a time, so it supplies what comes next).
    private static func hidesBackslash(_ u: [UInt16], at i: Int, continues: Bool) -> Bool {
        guard u[i] == backslash, i + 1 < u.count else { return false }
        if escapable.contains(u[i + 1]) { return true }
        guard u[i + 1] == newline else { return false }
        var j = i + 2
        if j == u.count { return continues }
        while j < u.count, u[j] == space || u[j] == tab { j += 1 }
        return j < u.count && u[j] != newline
    }

    /// True when `line` (one line, with or without its `\n`) holds only spaces and tabs —
    /// a CommonMark BLANK line, which ends a paragraph.
    static func isBlankLine(_ line: String) -> Bool {
        line.utf16.allSatisfy { $0 == space || $0 == tab || $0 == newline }
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

    /// ✅ EP-045 AC7 (study §4D.4(a), Q-AC7 = (a)): UNEXPOSED BLOCK INTENTS ARE PROSE. A line led by
    /// a tab or ≥4 spaces is a `codeBlock` to the parser, which then shows `\*` literally; ✅ this
    /// map ignores indentation and hides the escape anyway — Scrivi reads such text as ordinary
    /// prose. Measured 2026-10-04: 2,000/2,000 against `.full` of the source with its leading
    /// indentation removed (the 42 that differ from plain `.full` are exactly the indented ones).
    /// ⚠️ Drawing those blocks as prose is [EP-046]'s, where a renderer exists.
    ///
    /// Scan one SOURCE fragment. `continues`: whether a non-blank line follows the fragment
    /// (only matters when it ends in `\` + `\n` — see `hidesBackslash`).
    static func map(_ source: String, continues: Bool = false) -> Map {
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
            if hidesBackslash(u, at: i, continues: continues) {
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
    /// backslash. `map(escape(t)).presented == t` for every `t`.
    static func escape(_ typed: String) -> String {
        var out: [UInt16] = []
        out.reserveCapacity(typed.utf16.count)
        for c in typed.utf16 {
            if escapable.contains(c) { out.append(backslash) }
            out.append(c)
        }
        return String(utf16CodeUnits: out, count: out.count)
    }

    // MARK: — R3 = (c): the caret never rests inside a HIDDEN RUN (ruled 2026-10-03; generalised EP-046 AC5)
    //
    // ⚠️ EP-046 E2-S1: a hidden character is no longer marked in STORAGE (route (a′) keeps storage
    // plain), so these take an `isHidden` test — `ManuscriptPresenter.hiddenTest(in:revealing:)` in
    // the app. ✅ A hidden RUN is one or more hidden characters in a row: an escape backslash, or a
    // heading prefix (`## `) on a line the caret is not on.

    /// True when boundary `s` sits just after a hidden character — a spot that LOOKS identical to
    /// the boundary before the hidden run (measured: x 35.32 vs 35.33).
    static func isUnreachable(_ s: Int, length: Int, isHidden: (Int) -> Bool) -> Bool {
        s > 0 && s < length && isHidden(s - 1)
    }

    /// Where a CARET proposed at `loc` (coming from `previous`) must land instead, or nil to leave it.
    /// ✅ A forward STEP from just before the run (→) lands after the first VISIBLE character past it;
    /// ✅ everything else — ←, a click, a jump — lands BEFORE the run, the visually identical spot.
    /// ⚠️ Measured 2026-10-03: a click there hit-tests to the unreachable boundary, and snapping it
    /// forward put the caret one character right of the click.
    static func snapCaret(_ loc: Int, from previous: Int, length: Int, isHidden: (Int) -> Bool) -> Int? {
        guard isUnreachable(loc, length: length, isHidden: isHidden) else { return nil }
        var start = loc - 1
        while start > 0, isHidden(start - 1) { start -= 1 }
        guard previous == loc - 1 else { return start }
        var end = loc
        while end < length, isHidden(end) { end += 1 }
        return min(end + 1, length)
    }

    /// A SELECTION never ends inside a hidden run: its start moves before the run. Its end moves
    /// past the run when the selection grows (an escape backslash keeps its mark with it), and
    /// ⚠️ BEFORE the run when it shrinks — E1 always moved the end forward, so shift-← stalled at a
    /// hidden escape (SP-161 step 1). `previousEnd` is the end before this change (nil: growing).
    /// nil when neither end needs moving.
    static func snapSelection(_ r: NSRange, previousEnd: Int? = nil, in text: NSString,
                              isHidden: (Int) -> Bool) -> NSRange? {
        let n = text.length
        var start = r.location, end = NSMaxRange(r)
        if isUnreachable(start, length: n, isHidden: isHidden) {
            while start > 0, isHidden(start - 1) { start -= 1 }
        }
        if isUnreachable(end, length: n, isHidden: isHidden) {
            if let p = previousEnd, end < p {
                while end > 0, isHidden(end - 1) { end -= 1 }
            } else {
                while end < n, isHidden(end) { end += 1 }
                if end < n, text.character(at: end - 1) == backslash { end += 1 }
            }
        }
        guard start != r.location || end != NSMaxRange(r) else { return nil }
        return NSRange(location: start, length: end - start)
    }

    /// UTF-16 offsets in `source` of every backslash HIDDEN when it is presented (an escape or
    /// a hard line break) — what `ManuscriptPresenter` hides.
    static func hiddenBackslashes(in source: String, continues: Bool = false) -> [Int] {
        let p2s = map(source, continues: continues).presentedToSource
        var out: [Int] = []
        for p in 0..<(p2s.count - 1) where p2s[p + 1] - p2s[p] == 2 { out.append(p2s[p]) }
        return out
    }
}
