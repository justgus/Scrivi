#if os(macOS)
import AppKit

// EP-050 S1 ([SP-171], T-0599) — THE ONE PRESENTED ⇄ STORAGE MAP.
//
// ✅ Q3 (ruled 2026-10-08): ONE map serves Find ([SP-164]) and accessibility (EP-050 AC1). It is the manuscript AS THE PAGE
// SHOWS IT AT REST (A1): escape backslashes, emphasis markers and heading prefixes hidden whatever the caret or Markup Hints;
// list prefixes kept; ✅ chapter titles INCLUDED (Q1 — they are on the page) and ✅ each divider presented as WORDS ("Scene break",
// "End of chapter" — Q2), both marked `excluded` so Find never matches inside them (its Q4).
// ✅ Measured ([SP-171] Plan 1(c)): re-deriving a block costs ~0.04 ms, but shifting a FLAT offset array costs ~90 ms for an edit
// near the start of 1.7 MB. So the map is held PER SEGMENT (a block, or a run of text with nothing hidden) with segment-local
// hidden offsets: an edit re-derives the blocks it touches and shifts only the segment starts after it.
// ✅ LAZY (Plan 1(b)): built at the first Find or accessibility request, so a writer who uses neither never pays for it.

/// One piece of storage and how it presents.
struct PresentedSegment: Equatable {
    /// Storage start and length.
    var s: Int
    var sLen: Int
    /// Presented start.
    var p: Int
    /// Sorted SEGMENT-LOCAL storage offsets that are not presented.
    var hidden: [Int]
    /// A chapter title or divider: on the page, but never searched (Find's Q4).
    var excluded: Bool
    /// ✅ Q2 (ruled 2026-10-08, after the live pass): a DIVIDER presents as WORDS — one storage character, several presented
    /// ones. ⚠️ VoiceOver ignored an `AXAttachment` label (measured by ear), so the words are in the text it reads.
    var replacement: [UInt16]? = nil
    var pLen: Int { replacement?.count ?? (sLen - hidden.count) }
}

/// The map's lookups — value-typed, so a snapshot for a background reader is a copy.
struct PresentedLayout {
    var segments: [PresentedSegment]
    /// Storage range covered (the whole manuscript, or a scene for the Navigator's jump).
    var storageStart: Int
    var storageEnd: Int
    /// Presented length.
    var length: Int

    /// Index of the last segment starting at or before storage offset `s` (segments are in storage order).
    func segment(atStorage s: Int) -> Int {
        var lo = 0, hi = segments.count - 1
        while lo < hi {
            let mid = (lo + hi + 1) / 2
            if segments[mid].s <= s { lo = mid } else { hi = mid - 1 }
        }
        return lo
    }

    /// Index of the segment holding presented unit `k` (0 ≤ k < length).
    func segment(atPresented k: Int) -> Int {
        var lo = 0, hi = segments.count - 1
        while lo < hi {
            let mid = (lo + hi + 1) / 2
            if segments[mid].p <= k { lo = mid } else { hi = mid - 1 }
        }
        while lo < segments.count - 1, k >= segments[lo].p + segments[lo].pLen { lo += 1 }
        return lo
    }

    /// Presented index of a storage offset: the first presented unit at or after it.
    func presentedIndex(_ s: Int) -> Int {
        guard !segments.isEmpty, s > storageStart else { return 0 }
        guard s < storageEnd else { return length }
        let g = segments[segment(atStorage: s)]
        let local = s - g.s
        if local >= g.sLen { return g.p + g.pLen }
        return g.p + local - Self.countBelow(g.hidden, local)
    }

    /// Storage offset of presented unit `k`; `length` (or beyond) → the storage end. Every unit of a divider's words → the
    /// divider character.
    func storageIndex(_ k: Int) -> Int {
        guard !segments.isEmpty, k < length else { return storageEnd }
        let g = segments[segment(atPresented: max(0, k))]
        if g.replacement != nil { return g.s }
        var j = max(0, k) - g.p
        for h in g.hidden { if h <= j { j += 1 } else { break } }
        return g.s + j
    }

    /// Storage range of a presented range. Its end is just after the last presented character, so hidden characters that
    /// FOLLOW it (a closing marker) are not part of it.
    func storageRange(_ r: NSRange) -> NSRange {
        let lo = min(max(r.location, 0), length)
        let start = storageIndex(lo)
        let stop = r.length == 0 ? start : storageIndex(min(NSMaxRange(r), length) - 1) + 1
        return NSRange(location: start, length: max(0, stop - start))
    }

    func presentedRange(_ r: NSRange) -> NSRange {
        let a = presentedIndex(r.location), b = presentedIndex(NSMaxRange(r))
        return NSRange(location: a, length: max(0, b - a))
    }

    /// The presented characters of `r`, read from `storage`.
    func units(in r: NSRange, from storage: NSString) -> [UInt16] {
        let lo = min(max(r.location, 0), length), hi = min(NSMaxRange(r), length)
        guard lo < hi, !segments.isEmpty else { return [] }
        var out: [UInt16] = []
        out.reserveCapacity(hi - lo)
        var buf = [UInt16](repeating: 0, count: 256)
        var i = segment(atPresented: lo)
        while i < segments.count, segments[i].p < hi {
            let g = segments[i]
            if let words = g.replacement {
                let a = max(g.p, lo), b = min(g.p + words.count, hi)
                if a < b { out.append(contentsOf: words[(a - g.p)..<(b - g.p)]) }
                i += 1
                continue
            }
            // Visible runs of this segment: between its hidden offsets.
            var k = g.p                                   // presented index of the run's first unit
            var prev = 0
            for stop in g.hidden + [g.sLen] {
                let runLen = stop - prev
                let a = max(k, lo), b = min(k + runLen, hi)
                if a < b {
                    let src = NSRange(location: g.s + prev + (a - k), length: b - a)
                    if buf.count < src.length { buf = [UInt16](repeating: 0, count: src.length) }
                    buf.withUnsafeMutableBufferPointer { storage.getCharacters($0.baseAddress!, range: src) }
                    out.append(contentsOf: buf[0..<src.length])
                }
                k += runLen
                prev = stop + 1
                if k >= hi { break }
            }
            i += 1
        }
        return out
    }

    /// The storage offsets inside `sr` that are NOT presented, ascending.
    func hiddenStorageOffsets(in sr: NSRange) -> [Int] {
        guard !segments.isEmpty, sr.length > 0 else { return [] }
        var out: [Int] = []
        var i = segment(atStorage: sr.location)
        while i < segments.count, segments[i].s < NSMaxRange(sr) {
            for h in segments[i].hidden where NSLocationInRange(segments[i].s + h, sr) { out.append(segments[i].s + h) }
            i += 1
        }
        return out
    }

    /// Presented ranges that end at a SEARCH BOUNDARY: one per run of scene text between excluded segments.
    func chunks() -> [NSRange] {
        var out: [NSRange] = []
        var start = 0, end = 0
        for g in segments {
            if g.excluded {
                if end > start { out.append(NSRange(location: start, length: end - start)) }
                start = g.p + g.pLen; end = start
            } else {
                end = g.p + g.pLen
            }
        }
        if end > start { out.append(NSRange(location: start, length: end - start)) }
        return out
    }

    /// How many of the sorted `hidden` are below `x`.
    static func countBelow(_ hidden: [Int], _ x: Int) -> Int {
        var lo = 0, hi = hidden.count
        while lo < hi {
            let mid = (lo + hi) / 2
            if hidden[mid] < x { lo = mid + 1 } else { hi = mid }
        }
        return lo
    }

    // MARK: — Building

    /// The segments for storage `range`, numbered from presented index `presentedStart`. ✅ Correct for ANY sub-range: a
    /// segment that starts mid-block reads its block's whole analysis.
    @MainActor
    static func makeSegments(_ ts: NSAttributedString, presenter: ManuscriptPresenter, range: NSRange,
                         presentedStart: Int) -> [PresentedSegment] {
        let ns = ts.string as NSString
        let end = NSMaxRange(range)
        var excluded: [NSRange] = []
        var dividers: [Int: DividerRenderState] = [:]
        ts.enumerateAttribute(.scriviHeading, in: range, options: []) { v, r, _ in if v != nil { excluded.append(r) } }
        ts.enumerateAttribute(.scriviDivider, in: range, options: []) { v, r, _ in
            guard let state = v as? DividerRenderState else { return }
            excluded.append(r)
            for k in r.location..<NSMaxRange(r) { dividers[k] = state }
        }
        excluded.sort { $0.location < $1.location }
        let pending = presenter.pending?.range
        var segs: [PresentedSegment] = []
        var pp = presentedStart
        // ⚠️ Only the characters BETWEEN blocks (blank lines) merge into one segment. A block never merges with anything, so a
        // patch never has to re-derive more than the blocks an edit touches (a merged plain-prose run could be the whole book).
        var lastMergeable = false
        func append(_ s: Int, _ len: Int, _ hidden: [Int], _ isExcluded: Bool, mergeable: Bool = false) {
            guard len > 0 else { return }
            if mergeable, hidden.isEmpty, lastMergeable, let last = segs.last, last.hidden.isEmpty, last.s + last.sLen == s {
                segs[segs.count - 1].sLen += len
            } else {
                segs.append(PresentedSegment(s: s, sLen: len, p: pp, hidden: hidden, excluded: isExcluded))
            }
            lastMergeable = mergeable && hidden.isEmpty
            pp += len - hidden.count
        }
        var x = 0
        var p = range.location
        while p < end {
            while x < excluded.count, NSMaxRange(excluded[x]) <= p { x += 1 }
            if x < excluded.count, NSLocationInRange(p, excluded[x]) {
                if let state = dividers[p] {
                    segs.append(PresentedSegment(s: p, sLen: 1, p: pp, hidden: [], excluded: true,
                                                 replacement: Array(Self.words(for: state).utf16)))
                    lastMergeable = false
                    pp += segs[segs.count - 1].pLen
                    p += 1
                    continue
                }
                let stop = min(end, NSMaxRange(excluded[x]))
                append(p, stop - p, [], true)
                p = stop
                continue
            }
            let limit = x < excluded.count ? min(end, excluded[x].location) : end
            if let b = presenter.block(in: ts, at: p), NSLocationInRange(p, b) {
                let info = presenter.info(ns.substring(with: b))
                let stop = min(limit, NSMaxRange(b))
                let lo = p - b.location, hi = stop - b.location
                var h = Set<Int>()
                for e in info.escapes where e >= lo && e < hi { h.insert(e - lo) }
                func hide(_ r: NSRange) {
                    let a = max(lo, r.location), z = min(hi, NSMaxRange(r))
                    if a < z { for k in a..<z { h.insert(k - lo) } }
                }
                for m in info.analysis.markers { hide(m.range) }
                for hd in info.analysis.headings { hide(hd.prefix) }
                if let pending { hide(NSRange(location: pending.location - b.location, length: pending.length)) }
                append(p, stop - p, h.sorted(), false)
                p = stop
            } else {
                append(p, 1, pending.map { NSLocationInRange(p, $0) } == true ? [0] : [], false, mergeable: true)
                p += 1
            }
        }
        return segs
    }

    /// ✅ Q2: what a divider says.
    static func words(for state: DividerRenderState) -> String { state == .chapterEnd ? "End of chapter" : "Scene break" }

    @MainActor
    static func build(_ ts: NSAttributedString, presenter: ManuscriptPresenter, range: NSRange? = nil) -> PresentedLayout {
        let whole = range ?? NSRange(location: 0, length: ts.length)
        let segs = makeSegments(ts, presenter: presenter, range: whole, presentedStart: 0)
        return PresentedLayout(segments: segs, storageStart: whole.location, storageEnd: NSMaxRange(whole),
                               length: segs.reduce(0) { $0 + $1.pLen })
    }
}

/// The manuscript (or part of it) as the writer sees it, with the map back to storage — an IMMUTABLE snapshot.
/// ✅ It owns a copy of the storage text, so a background reader (AppKit's incremental search) may hold it.
final class PresentedText: @unchecked Sendable {
    let layout: PresentedLayout
    private let storage: NSString
    private let lock = NSLock()
    private var cachedString: NSString?
    private var cachedChunks: [NSRange]?

    init(layout: PresentedLayout, storage: NSString) {
        self.layout = layout
        self.storage = storage
    }

    static let empty = PresentedText(layout: PresentedLayout(segments: [], storageStart: 0, storageEnd: 0, length: 0), storage: "")

    var length: Int { layout.length }

    /// What the writer sees — assembled on first use.
    var string: NSString {
        lock.lock(); defer { lock.unlock() }
        if let cachedString { return cachedString }
        let u = layout.units(in: NSRange(location: 0, length: length), from: storage)
        let s = String(utf16CodeUnits: u, count: u.count) as NSString
        cachedString = s
        return s
    }

    /// Presented ranges that end at a SEARCH BOUNDARY — one per run of scene text (a match never crosses a scene break or a
    /// chapter title).
    var chunks: [NSRange] {
        lock.lock(); defer { lock.unlock() }
        if let cachedChunks { return cachedChunks }
        let c = layout.chunks()
        cachedChunks = c
        return c
    }

    func substring(_ r: NSRange) -> String {
        let u = layout.units(in: r, from: storage)
        return String(utf16CodeUnits: u, count: u.count)
    }

    func storageRange(_ r: NSRange) -> NSRange { layout.storageRange(r) }
    func presentedIndex(_ s: Int) -> Int { layout.presentedIndex(s) }
    func presentedRange(_ r: NSRange) -> NSRange { layout.presentedRange(r) }

    /// The first PRESENTED match of `query` (case- and diacritic-insensitive) inside the searchable text, as a storage range —
    /// the Navigator's search jump ([SP-164] Q5). ⛔ Never inside a chapter title or divider.
    func firstMatch(of query: String) -> NSRange? {
        for c in chunks {
            let m = (substring(c) as NSString).range(of: query, options: [.caseInsensitive, .diacriticInsensitive])
            if m.location != NSNotFound { return storageRange(NSRange(location: c.location + m.location, length: m.length)) }
        }
        return nil
    }

    /// Build it for `range` of storage (the whole manuscript by default) — a one-off, not the live map.
    @MainActor
    static func build(_ ts: NSAttributedString, presenter: ManuscriptPresenter, range: NSRange? = nil) -> PresentedText {
        PresentedText(layout: PresentedLayout.build(ts, presenter: presenter, range: range),
                      storage: (ts.string as NSString).copy() as! NSString)
    }
}

/// The LIVE map of a manuscript text view: built at the first request, then PATCHED on every character edit.
@MainActor
final class PresentedMap {
    private weak var textView: NSTextView?
    private(set) var layout: PresentedLayout?
    nonisolated(unsafe) private var observer: NSObjectProtocol?
    /// Whole builds since creation — a test proves an edit PATCHES (this does not grow).
    private(set) var builds = 0

    init(textView: NSTextView) { self.textView = textView }

    deinit { if let observer { NotificationCenter.default.removeObserver(observer) } }

    private var presenter: ManuscriptPresenter? { textView?.textContentStorage?.delegate as? ManuscriptPresenter }

    /// The current map, building it if needed.
    func current() -> PresentedLayout? {
        if let layout { return layout }
        guard let ts = textView?.textStorage, let presenter else { return nil }
        if observer == nil {
            observer = NotificationCenter.default.addObserver(
                forName: NSTextStorage.didProcessEditingNotification, object: ts, queue: nil) { [weak self] note in
                guard let ts = note.object as? NSTextStorage, ts.editedMask.contains(.editedCharacters) else { return }
                let edited = ts.editedRange, delta = ts.changeInLength
                MainActor.assumeIsolated { self?.patch(edited: edited, delta: delta) }
            }
        }
        let t0 = Date()
        let built = PresentedLayout.build(ts, presenter: presenter)
        builds += 1
        let ms = Date().timeIntervalSince(t0) * 1000
        if ms > 5 { NSLog(String(format: "[SCRIVI-AX] presented map built in %.1f ms (%d segments, %d units)", ms, built.segments.count, built.length)) }
        layout = built
        return built
    }

    /// An immutable snapshot for a background reader.
    func snapshot() -> PresentedText {
        guard let lay = current(), let ts = textView?.textStorage else { return .empty }
        return PresentedText(layout: lay, storage: ts.mutableString.copy() as! NSString)
    }

    /// Something other than a character edit changed what `range` presents (the pending pair appearing).
    func noteChange(in range: NSRange) { patch(edited: range, delta: 0) }

    /// Re-derive the blocks an edit touched; shift the segments after them. `edited` is in the NEW storage.
    private func patch(edited: NSRange, delta: Int) {
        guard layout != nil, let ts = textView?.textStorage, let presenter else { return }
        let newLen = ts.length
        // A wholesale replacement (rebuild, typography) is cheaper to forget: the next request builds afresh.
        guard layout!.storageEnd + delta == newLen, !layout!.segments.isEmpty, edited.length <= max(4096, newLen / 2) else {
            layout = nil
            return
        }
        var a = edited.location, b = NSMaxRange(edited)
        for probe in [a - 1, a, b - 1, b] where probe >= 0 && probe < newLen {
            if let blk = presenter.block(in: ts, at: probe) { a = min(a, blk.location); b = max(b, NSMaxRange(blk)) }
        }
        // Old coordinates: before the edit unchanged; after it, shifted back by `delta`.
        let aOld = a, bOld = max(b - delta, a)
        let i = layout!.segment(atStorage: aOld)
        let j = bOld > aOld ? layout!.segment(atStorage: bOld - 1) : i
        let start = layout!.segments[i].s
        let endNew = layout!.segments[j].s + layout!.segments[j].sLen + delta
        guard endNew >= start else { layout = nil; return }
        let pStart = layout!.segments[i].p
        let oldP = layout!.segments[i...j].reduce(0) { $0 + $1.pLen }
        let fresh = PresentedLayout.makeSegments(ts, presenter: presenter, range: NSRange(location: start, length: endNew - start),
                                             presentedStart: pStart)
        let dp = fresh.reduce(0) { $0 + $1.pLen } - oldP
        layout!.segments.replaceSubrange(i...j, with: fresh)
        if delta != 0 || dp != 0 {
            for k in (i + fresh.count)..<layout!.segments.count {
                layout!.segments[k].s += delta
                layout!.segments[k].p += dp
            }
        }
        layout!.storageEnd = newLen
        layout!.length += dp
    }
}
#endif
