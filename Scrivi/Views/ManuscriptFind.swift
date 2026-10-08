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
#endif
