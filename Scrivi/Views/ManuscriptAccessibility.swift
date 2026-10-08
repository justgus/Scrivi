#if os(macOS)
import AppKit

// EP-050 S1 ([SP-171], T-0599, [I-0277]) — THE MANUSCRIPT'S ACCESSIBILITY: VoiceOver reads the page AT REST (A1).
//
// ✅ Every position member answers in PRESENTED positions through the ONE map (`presentedMap`, Q3) — AC1.
// ✅ MEASURED ([SP-171] Plan 1(b), the user's VoiceOver run): VoiceOver enters at the NEW-STYLE members, and `NSTextView`'s own
// implementations forward to the legacy `accessibilityAttributeValue:`. So the members are overridden at the new-style level.
// ⚠️ Exception: `accessibilityValue()` cannot be overridden from Swift (`NSView` does not declare it — measured), and `AXValue`
// arrives at the legacy entry (once per focus), so it is served there.
// ✅ LINES and FRAMES are AppKit's: lines are VISUAL, and a hidden character is zero-width, so the presented and stored text
// have the same lines — positions are mapped in and out, and the line / frame work is `super`'s on the mapped range.
// ✅ AC2: a selection VoiceOver SETS goes through Scrivi's caret rules (`axStorageSelection`).
// DEBUG: each member is timed into `[SCRIVI-AX]` bursts (Plan 1(b)'s probe, kept for AC3 and the live pass).

#if DEBUG
/// ⚠️ Not `@MainActor`: the legacy entry points are nonisolated. AppKit serves AX requests on the main thread; the tallies
/// assume it (diagnostic only).
enum AXProbe {
    private struct Tally { var calls = 0; var ms = 0.0 }
    nonisolated(unsafe) private static var tallies: [String: Tally] = [:]
    nonisolated(unsafe) private static var order: [String] = []
    nonisolated(unsafe) private static var burstStart: Date?
    nonisolated(unsafe) private static var flush: DispatchWorkItem?

    static func time<T>(_ key: String, _ body: () -> T) -> T {
        let t0 = Date()
        let result = body()
        let ms = Date().timeIntervalSince(t0) * 1000
        if tallies[key] == nil { order.append(key) }
        tallies[key, default: Tally()].calls += 1
        tallies[key, default: Tally()].ms += ms
        if burstStart == nil { burstStart = t0 }
        if ms > 5 { NSLog(String(format: "[SCRIVI-AX] slow %@ %.1f ms", key, ms)) }
        flush?.cancel()
        let item = DispatchWorkItem { report() }
        flush = item
        DispatchQueue.main.asyncAfter(deadline: .now() + 0.5, execute: item)
        return result
    }

    private static func report() {
        let span = (burstStart.map { Date().timeIntervalSince($0) - 0.5 } ?? 0) * 1000
        let parts = order.map { k in String(format: "%@×%d %.1fms", k, tallies[k]!.calls, tallies[k]!.ms) }
        NSLog(String(format: "[SCRIVI-AX] burst %.0f ms: ", span) + parts.joined(separator: " · "))
        tallies = [:]; order = []; burstStart = nil
    }
}
#endif

extension ManuscriptNSTextView {

    @inline(__always)
    private func ax<T>(_ name: @autoclosure () -> String, _ body: () -> T) -> T {
        #if DEBUG
        return AXProbe.time(name(), body)
        #else
        return body()
        #endif
    }

    /// The map, when this view presents a manuscript (nil → plain `NSTextView` behaviour).
    private var axMap: PresentedLayout? { presenter == nil ? nil : presentedMap.current() }

    private func axString(_ m: PresentedLayout, _ r: NSRange) -> String {
        guard let storage = textStorage else { return "" }
        let u = m.units(in: r, from: storage.mutableString)
        return String(utf16CodeUnits: u, count: u.count)
    }

    private func clamped(_ r: NSRange, _ m: PresentedLayout) -> NSRange {
        let lo = min(max(r.location, 0), m.length)
        return NSRange(location: lo, length: min(max(r.length, 0), m.length - lo))
    }

    // MARK: — The whole text (legacy entry, once per focus)

    /// ⚠️ The legacy entry is nonisolated. AppKit serves accessibility on the main thread; off it (never observed) this falls
    /// back to AppKit's answer rather than ASSERTING isolation (the [I-0202] lesson).
    override func accessibilityAttributeValue(_ attribute: NSAccessibility.Attribute) -> Any? {
        if attribute == .value, Thread.isMainThread {
            let presented: String? = MainActor.assumeIsolated {
                ax("L:AXValue") { axMap.map { axString($0, NSRange(location: 0, length: $0.length)) } }
            }
            if let presented { return presented }
        }
        return super.accessibilityAttributeValue(attribute)
    }

    // MARK: — Text and counts

    override func accessibilityNumberOfCharacters() -> Int {
        ax("numberOfCharacters") { axMap?.length ?? super.accessibilityNumberOfCharacters() }
    }

    override func accessibilityString(for range: NSRange) -> String? {
        ax("stringForRange") {
            guard let m = axMap else { return super.accessibilityString(for: range) }
            return axString(m, clamped(range, m))
        }
    }

    /// AppKit's attributed text for the covering storage range, with the hidden characters taken out — fonts and attachments
    /// stay where they were.
    override func accessibilityAttributedString(for range: NSRange) -> NSAttributedString? {
        ax("attributedStringForRange") {
            guard let m = axMap else { return super.accessibilityAttributedString(for: range) }
            let r = clamped(range, m)
            let sr = m.storageRange(r)
            guard let base = super.accessibilityAttributedString(for: sr), base.length == sr.length else {
                return NSAttributedString(string: axString(m, r))
            }
            // AppKit's text for the covering storage range; then, from the END back, hidden characters out and each divider
            // character replaced by its WORDS (Q2) — fonts kept; then trimmed to the presented range asked for.
            let a = NSMutableAttributedString(attributedString: base)
            var edits: [(Int, String?)] = m.hiddenStorageOffsets(in: sr).map { ($0, nil) }
            textStorage?.enumerateAttribute(.scriviDivider, in: sr, options: []) { v, rr, _ in
                guard let state = v as? DividerRenderState else { return }
                for k in rr.location..<NSMaxRange(rr) { edits.append((k, PresentedLayout.words(for: state))) }
            }
            for (k, words) in edits.sorted(by: { $0.0 > $1.0 }) {
                a.replaceCharacters(in: NSRange(location: k - sr.location, length: 1), with: words ?? "")
            }
            let p0 = m.presentedIndex(sr.location)
            let want = NSRange(location: r.location - p0, length: r.length)
            guard want.location >= 0, NSMaxRange(want) <= a.length else { return NSAttributedString(string: axString(m, r)) }
            return a.attributedSubstring(from: want)
        }
    }

    override func accessibilityRTF(for range: NSRange) -> Data? {
        ax("rtfForRange") {
            guard axMap != nil, let a = accessibilityAttributedString(for: range) else { return super.accessibilityRTF(for: range) }
            return try? a.data(from: NSRange(location: 0, length: a.length), documentAttributes: [.documentType: NSAttributedString.DocumentType.rtf])
        }
    }

    // MARK: — Selection (AC2)

    override func accessibilitySelectedText() -> String? {
        ax("selectedText") {
            guard let m = axMap else { return super.accessibilitySelectedText() }
            return axString(m, m.presentedRange(selectedRange()))
        }
    }

    override func accessibilitySelectedTextRange() -> NSRange {
        ax("selectedTextRange") { axMap?.presentedRange(selectedRange()) ?? super.accessibilitySelectedTextRange() }
    }

    override func accessibilitySelectedTextRanges() -> [NSValue]? {
        ax("selectedTextRanges") {
            guard let m = axMap else { return super.accessibilitySelectedTextRanges() }
            return selectedRanges.map { NSValue(range: m.presentedRange($0.rangeValue)) }
        }
    }

    /// ✅ AC2 (Plan 5): a selection VoiceOver SETS lands where Scrivi's caret rules put it. Mapped to storage, then a caret is
    /// sent HOME as a PLACEMENT (`from:` itself), not as an arrow step: `setSelectedRanges`' own snap reads the previous caret to
    /// recognise → / ← steps, and VoiceOver re-setting the position it is already at (after a one-character closer) would read as
    /// a → step and move one character on — the drift AC2 forbids. A selection with length gets `setSelectedRanges`' own rule
    /// (its ends never split a marker or an escape).
    private func axStorageSelection(_ range: NSRange, _ m: PresentedLayout) -> NSRange {
        var r = m.storageRange(clamped(range, m))
        if r.length == 0, let presenter, let ts = textStorage,
           let home = MarkdownEscapes.snapCaret(r.location, from: r.location, length: ts.length,
                                               runAt: Self.runLookup(presenter, ts)) {
            r.location = home
        }
        return r
    }

    override func setAccessibilitySelectedTextRange(_ range: NSRange) {
        ax("set selectedTextRange") {
            guard let m = axMap else { return super.setAccessibilitySelectedTextRange(range) }
            setSelectedRange(axStorageSelection(range, m))
        }
    }

    override func setAccessibilitySelectedTextRanges(_ ranges: [NSValue]?) {
        ax("set selectedTextRanges") {
            guard let m = axMap, let ranges, !ranges.isEmpty else { return super.setAccessibilitySelectedTextRanges(ranges) }
            selectedRanges = ranges.map { NSValue(range: axStorageSelection($0.rangeValue, m)) }
        }
    }

    override func accessibilityVisibleCharacterRange() -> NSRange {
        ax("visibleCharacterRange") {
            let r = super.accessibilityVisibleCharacterRange()
            return axMap?.presentedRange(r) ?? r
        }
    }

    // MARK: — Positions: mapped in, AppKit's answer mapped out

    override func accessibilityRange(for index: Int) -> NSRange {
        ax("rangeForIndex") {
            guard let m = axMap else { return super.accessibilityRange(for: index) }
            return m.presentedRange(super.accessibilityRange(for: m.storageIndex(index)))
        }
    }

    override func accessibilityStyleRange(for index: Int) -> NSRange {
        ax("styleRangeForIndex") {
            guard let m = axMap else { return super.accessibilityStyleRange(for: index) }
            return m.presentedRange(super.accessibilityStyleRange(for: m.storageIndex(index)))
        }
    }

    override func accessibilityRange(for point: NSPoint) -> NSRange {
        ax("rangeForPosition") {
            let r = super.accessibilityRange(for: point)
            return axMap?.presentedRange(r) ?? r
        }
    }

    override func accessibilityFrame(for range: NSRange) -> NSRect {
        ax("frameForRange") {
            guard let m = axMap else { return super.accessibilityFrame(for: range) }
            return super.accessibilityFrame(for: m.storageRange(clamped(range, m)))
        }
    }

    // MARK: — Lines: VISUAL, the same in both spaces

    override func accessibilityInsertionPointLineNumber() -> Int {
        ax("insertionPointLineNumber") { super.accessibilityInsertionPointLineNumber() }
    }

    override func accessibilityLine(for index: Int) -> Int {
        ax("lineForIndex") {
            guard let m = axMap else { return super.accessibilityLine(for: index) }
            return super.accessibilityLine(for: m.storageIndex(index))
        }
    }

    override func accessibilityRange(forLine line: Int) -> NSRange {
        ax("rangeForLine") {
            let r = super.accessibilityRange(forLine: line)
            return axMap?.presentedRange(r) ?? r
        }
    }
}
#endif
