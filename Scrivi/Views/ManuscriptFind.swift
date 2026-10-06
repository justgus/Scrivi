import Foundation

/// The Edit ▸ Find commands ([SP-164]) — cross-platform so the menu and the session can carry them. Raw values are AppKit's
/// `NSTextFinder.Action` values.
enum ManuscriptFindCommand: Int, Sendable {
    case showFind = 1, nextMatch = 2, previousMatch = 3, useSelectionForFind = 7, showReplace = 12
}

#if os(macOS)
import AppKit

// EP-046 E2-S4 ([SP-164], T-0593 + T-0585) — FIND AND REPLACE OVER WHAT THE WRITER SEES.
//
// ⛔ There was NO in-manuscript Find before this Sprint. ✅ Q1 (ruled 2026-10-06): AppKit's own find bar — an `NSTextFinder`
// whose client hands it the manuscript AS PRESENTED: escape backslashes, emphasis markers and heading prefixes are not there
// (Q-E2-5), and neither are chapter titles (Q4: chapter metadata, not manuscript text). Every match maps back to STORAGE.

/// The manuscript (or part of it) as the writer sees it, with the map back to storage.
/// ✅ Immutable once built (its `NSString` is never mutated), so a background reader may hold it.
struct PresentedText: @unchecked Sendable {
    /// What the writer sees.
    let string: NSString
    /// `source[i]` = the storage offset of presented unit `i`; one extra entry at the end (the storage end of the text).
    let source: [Int]
    /// Presented ranges that end at a SEARCH BOUNDARY — one per run of scene text (a match never crosses a scene break or a
    /// chapter title).
    let chunks: [NSRange]

    var length: Int { string.length }

    /// Build it for `range` of storage. ✅ Per block, from the presenter's cached analysis — hidden: escape backslashes,
    /// emphasis markers, heading prefixes. ⚠️ A list prefix is VISIBLE (dimmed, [SP-163] Q7), so it stays.
    @MainActor
    static func build(_ ts: NSAttributedString, presenter: ManuscriptPresenter, range: NSRange? = nil) -> PresentedText {
        let ns = ts.string as NSString
        let whole = range ?? NSRange(location: 0, length: ns.length)
        var units: [UInt16] = []
        var source: [Int] = []
        units.reserveCapacity(whole.length)
        source.reserveCapacity(whole.length + 1)
        // Divider and chapter-title characters are not manuscript text: each run of them ends a chunk.
        var excluded: [NSRange] = []
        for key in [NSAttributedString.Key.scriviDivider, .scriviHeading] {
            ts.enumerateAttribute(key, in: whole, options: []) { v, r, _ in if v != nil { excluded.append(r) } }
        }
        excluded.sort { $0.location < $1.location }
        var chunks: [NSRange] = []
        var chunkStart = 0
        func closeChunk() {
            if units.count > chunkStart { chunks.append(NSRange(location: chunkStart, length: units.count - chunkStart)) }
            chunkStart = units.count
        }
        var x = 0                                   // index into `excluded`
        var p = whole.location
        let end = NSMaxRange(whole)
        while p < end {
            while x < excluded.count, NSMaxRange(excluded[x]) <= p { x += 1 }
            if x < excluded.count, NSLocationInRange(p, excluded[x]) {
                closeChunk()
                p = NSMaxRange(excluded[x])
                continue
            }
            let limit = x < excluded.count ? min(end, excluded[x].location) : end
            if let b = presenter.block(in: ts, at: p), NSLocationInRange(p, b) {
                let info = presenter.info(ns.substring(with: b))
                var hidden = info.escapes
                for m in info.analysis.markers { for k in m.range.location..<NSMaxRange(m.range) { hidden.insert(k) } }
                for h in info.analysis.headings { for k in h.prefix.location..<NSMaxRange(h.prefix) { hidden.insert(k) } }
                let stop = min(limit, NSMaxRange(b))
                for q in p..<stop where !hidden.contains(q - b.location) {
                    units.append(ns.character(at: q))
                    source.append(q)
                }
                p = stop
            } else {
                units.append(ns.character(at: p))
                source.append(p)
                p += 1
            }
        }
        closeChunk()
        source.append(end)
        return PresentedText(string: String(utf16CodeUnits: units, count: units.count) as NSString,
                             source: source, chunks: chunks)
    }

    /// Storage range of a presented range. Its end is just after the last presented character, so hidden characters that
    /// FOLLOW the match (a closing marker) are not part of it.
    func storageRange(_ r: NSRange) -> NSRange {
        let lo = min(max(r.location, 0), length)
        let start = source[lo]
        let stop = r.length == 0 ? start : source[min(NSMaxRange(r), length) - 1] + 1
        return NSRange(location: start, length: max(0, stop - start))
    }

    /// Presented index of a storage offset: the first presented unit at or after it.
    func presentedIndex(_ s: Int) -> Int {
        var lo = 0, hi = length
        while lo < hi {
            let mid = (lo + hi) / 2
            if source[mid] < s { lo = mid + 1 } else { hi = mid }
        }
        return lo
    }

    func presentedRange(_ r: NSRange) -> NSRange {
        let a = presentedIndex(r.location), b = presentedIndex(NSMaxRange(r))
        return NSRange(location: a, length: max(0, b - a))
    }

    /// The first PRESENTED match of `query` (case- and diacritic-insensitive), as a storage range — the Navigator's search
    /// jump (Q5).
    func firstMatch(of query: String) -> NSRange? {
        let m = string.range(of: query, options: [.caseInsensitive, .diacriticInsensitive])
        return m.location == NSNotFound ? nil : storageRange(m)
    }
}

/// The `NSTextFinderClient` over the presented manuscript. The view owns one, with its own `NSTextFinder`.
/// ⚠️ AppKit runs INCREMENTAL search on a background queue (`NSTextFinder.h`), so the text it reads — `string(at:…)`,
/// `stringLength()` — comes from an immutable SNAPSHOT behind a lock, built on the main thread. Everything that touches the
/// view runs on the main thread (AppKit calls it there).
final class ManuscriptFinderClient: NSObject, NSTextFinderClient, @unchecked Sendable {
    nonisolated(unsafe) weak var textView: ManuscriptNSTextView?
    /// Replace All, done as ONE undoable step (Q2) — the coordinator records it as a grouped edit across scenes. Each
    /// element is a storage range and its replacement (UNescaped; typed text). nil (a view without a coordinator): the
    /// replacements are applied one by one.
    nonisolated(unsafe) var replaceAllHandler: (([(NSRange, String)]) -> Void)?

    private let lock = NSLock()
    nonisolated(unsafe) private var snapshot: PresentedText?
    nonisolated(unsafe) private var snapshotVersion = -1
    /// Bumped by the view on every character edit.
    nonisolated(unsafe) var storageVersion = 0

    /// The presented text — rebuilt on the main thread when the storage changed; a background reader gets the last snapshot.
    var presented: PresentedText {
        lock.lock()
        let current = snapshot, fresh = snapshotVersion == storageVersion
        lock.unlock()
        if let current, fresh || !Thread.isMainThread { return current }
        guard Thread.isMainThread else { return PresentedText(string: "", source: [0], chunks: []) }
        let built: PresentedText = MainActor.assumeIsolated {
            guard let tv = textView, let ts = tv.textStorage, let presenter = tv.presenter else {
                return PresentedText(string: "", source: [0], chunks: [])
            }
            let t0 = Date()
            let p = PresentedText.build(ts, presenter: presenter)
            let ms = Date().timeIntervalSince(t0) * 1000
            if ms > 5 { NSLog(String(format: "[SCRIVI-FIND] presented text built in %.1f ms (%d units)", ms, p.length)) }
            return p
        }
        lock.lock()
        snapshot = built
        snapshotVersion = storageVersion
        lock.unlock()
        return built
    }

    // MARK: — Content, chunk by chunk (each scene ends at a search boundary)

    var isSelectable: Bool { true }
    var isEditable: Bool { true }
    var allowsMultipleSelection: Bool { true }

    func string(at characterIndex: Int, effectiveRange outRange: NSRangePointer,
                endsWithSearchBoundary outFlag: UnsafeMutablePointer<ObjCBool>) -> String {
        let p = presented
        // Chunks are in order: binary search for the one holding the index.
        var lo = 0, hi = p.chunks.count - 1
        while lo <= hi {
            let mid = (lo + hi) / 2
            let c = p.chunks[mid]
            if characterIndex < c.location { hi = mid - 1 }
            else if characterIndex >= NSMaxRange(c) { lo = mid + 1 }
            else {
                outRange.pointee = c
                outFlag.pointee = true
                return p.string.substring(with: c)
            }
        }
        outRange.pointee = NSRange(location: characterIndex, length: 0)
        outFlag.pointee = true
        return ""
    }

    func stringLength() -> Int { presented.length }

    // MARK: — Selection, scrolling, feedback

    var firstSelectedRange: NSRange {
        let p = presented
        return MainActor.assumeIsolated {
            guard let tv = textView else { return NSRange(location: 0, length: 0) }
            return p.presentedRange(tv.selectedRange())
        }
    }

    var selectedRanges: [NSValue] {
        get {
            let p = presented
            let ranges: [NSRange] = MainActor.assumeIsolated { textView?.selectedRanges.map(\.rangeValue) ?? [] }
            return ranges.map { NSValue(range: p.presentedRange($0)) }
        }
        set {
            let p = presented
            let storage = newValue.map { p.storageRange($0.rangeValue) }
            MainActor.assumeIsolated { textView?.selectedRanges = storage.map { NSValue(range: $0) } }
        }
    }

    func scrollRangeToVisible(_ range: NSRange) {
        let r = presented.storageRange(range)
        MainActor.assumeIsolated { textView?.scrollRangeToVisible(r) }
    }

    func contentView(at index: Int, effectiveCharacterRange outRange: NSRangePointer) -> NSView {
        outRange.pointee = NSRange(location: 0, length: presented.length)
        return MainActor.assumeIsolated { textView ?? NSView() }
    }

    func rects(forCharacterRange range: NSRange) -> [NSValue]? {
        let r = presented.storageRange(range)
        let rects: [NSRect]? = MainActor.assumeIsolated { rectsInView(storage: r) }
        return rects?.map { NSValue(rect: $0) }
    }

    @MainActor private func rectsInView(storage r: NSRange) -> [NSRect]? {
        guard let tv = textView, let tlm = tv.textLayoutManager, let cs = tv.textContentStorage else { return nil }
        guard let start = cs.location(cs.documentRange.location, offsetBy: r.location),
              let end = cs.location(start, offsetBy: r.length),
              let textRange = NSTextRange(location: start, end: end) else { return nil }
        var rects: [NSRect] = []
        let origin = tv.textContainerOrigin
        tlm.enumerateTextSegments(in: textRange, type: .standard, options: []) { _, frame, _, _ in
            rects.append(frame.offsetBy(dx: origin.x, dy: origin.y))
            return true
        }
        return rects
    }

    var visibleCharacterRanges: [NSValue] {
        let p = presented
        let visible: NSRange? = MainActor.assumeIsolated {
            guard let tv = textView, let tlm = tv.textLayoutManager, let cs = tv.textContentStorage,
                  let viewport = tlm.textViewportLayoutController.viewportRange else { return nil }
            let a = cs.offset(from: cs.documentRange.location, to: viewport.location)
            let b = cs.offset(from: cs.documentRange.location, to: viewport.endLocation)
            return NSRange(location: a, length: b - a)
        }
        return visible.map { [NSValue(range: p.presentedRange($0))] } ?? []
    }

    // MARK: — Replace (Q3: the replacement takes the style of the match's first character — the type-over rule)

    /// Replace All's edits, applied ONCE (Q2) on the first `replaceCharacters` call of the batch; the rest of the batch's calls
    /// are no-ops. ⚠️ [SP-164] live pass: returning false here (doing it all inside this call) made the find bar show NO
    /// count — so it returns true and AppKit carries on, while the work is done in one pass from the ranges it was given here.
    nonisolated(unsafe) private var batch: (edits: [(NSRange, String)], applied: Bool, remaining: Int)?

    func shouldReplaceCharacters(inRanges ranges: [NSValue], with strings: [String]) -> Bool {
        guard ranges.count > 1 else { batch = nil; return true }
        let p = presented
        let edits = zip(ranges, strings).map { (p.storageRange($0.rangeValue), $1) }
            .sorted { $0.0.location > $1.0.location }
        batch = (edits, false, ranges.count)
        return true
    }

    func replaceCharacters(in range: NSRange, with string: String) {
        if var b = batch {
            if !b.applied {
                let edits = b.edits
                MainActor.assumeIsolated {
                    if let replaceAllHandler { replaceAllHandler(edits) } else { textView?.applyReplacements(edits) }
                }
                b.applied = true
            }
            b.remaining -= 1
            batch = b.remaining > 0 ? b : nil
            return
        }
        let r = presented.storageRange(range)
        MainActor.assumeIsolated { textView?.replaceFound(r, with: string) }
    }

    func didReplaceCharacters() { batch = nil }
}
#endif
