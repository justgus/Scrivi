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

// The presented text and its map back to storage live in `ManuscriptPresentedMap.swift` (EP-050 Q3: ONE map for Find and
// accessibility). ✅ Chapter titles and dividers are now IN it; Find skips them through its chunks.

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
        guard Thread.isMainThread else { return .empty }
        // ✅ EP-050 S1: a snapshot of the view's LIVE map (patched per edit), not a whole rebuild per edit.
        let built: PresentedText = MainActor.assumeIsolated { textView?.presentedMap.snapshot() ?? .empty }
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
                return p.substring(c)
            }
        }
        // Between chunks: a chapter title or divider (Q4: never searched) — handed over MASKED, the same length, so the
        // finder's walk through the text stays in step and nothing can match inside it.
        let gapStart = lo > 0 ? NSMaxRange(p.chunks[lo - 1]) : 0
        let gapEnd = lo < p.chunks.count ? p.chunks[lo].location : p.length
        guard gapStart <= characterIndex, characterIndex < gapEnd else {
            outRange.pointee = NSRange(location: characterIndex, length: 0)
            outFlag.pointee = true
            return ""
        }
        outRange.pointee = NSRange(location: gapStart, length: gapEnd - gapStart)
        outFlag.pointee = true
        return String(repeating: "\u{FFFC}", count: gapEnd - gapStart)
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
#else
import UIKit

// EP-055 S3b ([SP-223], T-0686) — FIND AND REPLACE ON iOS, OVER WHAT THE WRITER SEES.
//
// ✅ Q1 (ruled 2026-11-18): the system's find bar (`UIFindInteraction`) over the PRESENTED text, as the Mac's [SP-164] Q1 —
// escapes, emphasis markers and heading prefixes are not there to match, chapter titles and dividers are skipped (the chunks
// of the shared `PresentedText`). A match is reported as the text view's own range of STORAGE, so the view's own highlighting,
// scrolling and ordering do the rest. ⛔ Not UITextView's built-in find: it searches storage, markers and all.

/// The searchable object behind the manuscript's find bar.
@MainActor
final class ManuscriptTextSearcher: NSObject, @preconcurrency UITextSearching {
    typealias DocumentIdentifier = AnyHashable?
    private weak var textView: ManuscriptUITextView?
    /// Replace All, as ONE undoable step — the shared `ManuscriptStructure.replaceAll` ([SP-164] Q2, [I-0333]). Storage
    /// ranges and UNescaped text. nil (a view with no project): applied one by one through the typing path.
    var replaceAllHandler: (([(NSRange, String)]) -> Bool)?

    init(textView: ManuscriptUITextView) { self.textView = textView }

    var selectedTextRange: UITextRange? { textView?.selectedTextRange }

    func compare(_ foundRange: UITextRange, toRange: UITextRange, document: AnyHashable??) -> ComparisonResult {
        textView?.compare(foundRange.start, to: toRange.start) ?? .orderedSame
    }

    func performTextSearch(queryString: String, options: UITextSearchOptions, resultAggregator: UITextSearchAggregator<AnyHashable?>) {
        guard let tv = textView else { return resultAggregator.finishedSearching() }
        let text = tv.presentedMap.snapshot()
        let compare = options.stringCompareOptions, method = options.wordMatchMethod
        // The search runs off the main actor (main is for UI only); the results are reported back on it.
        Task { @MainActor [weak tv] in
            let found = await Task.detached(priority: .userInitiated) {
                Self.matches(of: queryString, in: text, compare: compare, method: method)
            }.value
            if let tv {
                for r in found {
                    if let range = tv.textRange(storage: r) { resultAggregator.foundRange(range, searchString: queryString, document: nil) }
                }
            }
            resultAggregator.finishedSearching()
        }
    }

    /// Every PRESENTED match of `query`, as storage ranges — never across a scene break or inside a chapter title.
    nonisolated static func matches(of query: String, in text: PresentedText, compare: String.CompareOptions,
                                    method: UITextSearchOptions.WordMatchMethod) -> [NSRange] {
        guard !query.isEmpty else { return [] }
        var out: [NSRange] = []
        for c in text.chunks {
            let chunk = text.substring(c) as NSString
            var from = 0
            while from < chunk.length {
                let m = chunk.range(of: query, options: compare, range: NSRange(location: from, length: chunk.length - from))
                guard m.location != NSNotFound, m.length > 0 else { break }
                from = m.location + m.length
                if method != .contains {
                    func isWord(_ i: Int) -> Bool {
                        guard i >= 0, i < chunk.length else { return false }
                        return CharacterSet.alphanumerics.contains(UnicodeScalar(chunk.character(at: i)) ?? " ")
                    }
                    if isWord(m.location - 1) { continue }
                    if method == .fullWord, isWord(NSMaxRange(m)) { continue }
                }
                out.append(text.storageRange(NSRange(location: c.location + m.location, length: m.length)))
            }
        }
        return out
    }

    func decorate(foundTextRange: UITextRange, document: AnyHashable??, usingStyle style: UITextSearchFoundTextStyle) {
        textView?.decorate(foundTextRange: foundTextRange, document: document, usingStyle: style)
    }

    func clearAllDecoratedFoundText() { textView?.clearAllDecoratedFoundText() }

    func scrollRangeToVisible(_ range: UITextRange, inDocument document: AnyHashable??) {
        textView?.scrollRangeToVisible(range, inDocument: document)
    }

    // MARK: Replace

    var supportsTextReplacement: Bool { !(textView?.inputHeld ?? true) }

    func shouldReplace(foundTextRange: UITextRange, document: AnyHashable??, withText: String) -> Bool {
        supportsTextReplacement
    }

    /// One replacement: the typing path, as the Mac's `replaceFound` — escaped, through the rules, one history event.
    func replace(foundTextRange: UITextRange, document: AnyHashable??, withText replacementText: String) {
        guard let tv = textView, let r = tv.storageRange(foundTextRange) else { return }
        tv.selectedRange = r
        tv.insertText(replacementText)
    }

    func replaceAll(queryString: String, options: UITextSearchOptions, withText replacementText: String) {
        guard let tv = textView else { return }
        let edits = Self.matches(of: queryString, in: tv.presentedMap.snapshot(), compare: options.stringCompareOptions,
                                 method: options.wordMatchMethod)
            .sorted { $0.location > $1.location }
            .map { ($0, replacementText) }
        guard !edits.isEmpty else { return }
        if replaceAllHandler?(edits) == true { return }
        for (r, t) in edits {
            tv.selectedRange = r
            tv.insertText(t)
        }
    }
}

extension ManuscriptUITextView {
    /// Storage offsets ⇄ the view's own text ranges.
    func textRange(storage r: NSRange) -> UITextRange? {
        guard let a = position(from: beginningOfDocument, offset: r.location),
              let b = position(from: a, offset: r.length) else { return nil }
        return textRange(from: a, to: b)
    }
    func storageRange(_ range: UITextRange) -> NSRange? {
        let a = offset(from: beginningOfDocument, to: range.start), b = offset(from: beginningOfDocument, to: range.end)
        return a >= 0 && b >= a ? NSRange(location: a, length: b - a) : nil
    }
}
#endif
