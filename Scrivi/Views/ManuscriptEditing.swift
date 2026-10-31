import Foundation
#if os(macOS)
import AppKit
#else
import UIKit
/// UIKit names AppKit's `NSTextStorageEditActions` this way; the alias keeps the presenter's delegate signature shared.
typealias NSTextStorageEditActions = NSTextStorage.EditActions
#endif

// EP-055 S1 (T-0633): the manuscript's shared pieces — every host (the Mac's `ManuscriptNSTextView`, iOS's
// `ManuscriptUITextView`) uses these, so the storage they build and the bytes they save cannot differ.
// Design: `docs/Scrivi_iOS_Manuscript_Design_v0_1.md` §3 (A, B, C1, C2).

// Custom attribute key used to mark chapter title heading ranges as non-editable.
extension NSAttributedString.Key {
    static let scriviHeading = NSAttributedString.Key("scrivi.heading")
    /// ✅ EP-045 AC1 — THE ONE TEST FOR "is this a scene divider?". Its value is a
    /// `DividerRenderState`. ⛔ Never test `.attachment` for that question: any attachment a
    /// future feature inserts (EP-032 references, EP-046 rendering) would then split scenes,
    /// and `sceneBoundaries` is what the save path slices scene bytes with.
    static let scriviDivider = NSAttributedString.Key("scrivi.divider")
}

/// EP-045 AC1 — the rendering state of a `DividerTextAttachment`, carried as the value of
/// `.scriviDivider` on the divider's character (user ruling 2026-10-03: *"We can make the class
/// manage different rendering states without changing the type. The attribute key can be
/// managed via an enum and can represent the rendering state of the class."*).
/// ⚠️ View-only: ScriviCore has no dividers, so this never crosses the ABI or reaches disk.
enum DividerRenderState: Equatable {
    /// Between two scenes of the same chapter.
    case sceneBreak
    /// Closes a chapter — drawn tinted (T-0575).
    case chapterEnd
}


// MARK: — Divider attachment cell

// Renders a 1pt horizontal rule across the full text column.
// No text, no label — purely visual separation.
private let dividerCellHeight: CGFloat = 24

// MARK: — EP-045 AC1: the ONE definition of "scene divider"

/// ✅ Every question "is this a scene divider?" is answered here, from the `.scriviDivider` key.
/// ⛔ Never from `.attachment` — that matches ANY attachment, and `sceneBoundaries` (computed
/// here) is what the save path slices scene bytes with.
enum SceneDivider {

    /// The divider as it goes into storage: the attachment character carrying the
    /// `.scriviDivider` KEY, then a "\n". ✅ The ONLY place a divider is built, so the key
    /// cannot be forgotten. ⚠️ The key is on the attachment character ALONE — the "\n" keeps
    /// the body attributes, so text typed after it never inherits the key.
    static func string(_ attachment: NSTextAttachment, state: DividerRenderState,
                       newlineAttributes: [NSAttributedString.Key: Any]) -> NSAttributedString {
        let s = NSMutableAttributedString(attachment: attachment)
        s.addAttribute(.scriviDivider, value: state, range: NSRange(location: 0, length: s.length))
        s.append(NSAttributedString(string: "\n", attributes: newlineAttributes))
        return s
    }

    /// True when the character at `i` is a scene divider.
    static func isDivider(in storage: NSAttributedString, at i: Int) -> Bool {
        i >= 0 && i < storage.length
            && storage.attribute(.scriviDivider, at: i, effectiveRange: nil) != nil
    }

    /// Scene ranges in storage: the text between dividers, skipping chapter-heading runs.
    /// Pure — reads only `storage` — so the save path's slicing can be tested directly.
    /// `nil` for empty storage (the caller keeps its previous boundaries).
    static func sceneBoundaries(in storage: NSAttributedString) -> [NSRange]? {
        let fullLen = storage.length
        guard fullLen > 0 else { return nil }
        let whole = NSRange(location: 0, length: fullLen)

        // Scene divider positions, found by RUN, not by character. ✅ EP-045 AC1: by the
        // `.scriviDivider` KEY — ⛔ not `.attachment`, which any attachment would match.
        var dividers: [Int] = []
        storage.enumerateAttribute(.scriviDivider, in: whole, options: []) { value, range, _ in
            if value != nil { dividers.append(range.location) }
        }

        // Ranges carrying chapter-heading text, so a segment can start AFTER one.
        // ⚠️ `skipHeading`'s job, expressed once up front instead of per position.
        var headings: [NSRange] = []
        storage.enumerateAttribute(.scriviHeading, in: whole, options: []) { value, range, _ in
            if value != nil { headings.append(range) }
        }

        // First position at or after `pos` that is not inside a heading run.
        func skipHeading(from pos: Int) -> Int {
            var p = pos
            var moved = true
            while moved {
                moved = false
                for h in headings where h.location <= p && p < h.location + h.length {
                    p = h.location + h.length
                    moved = true
                }
            }
            return min(p, fullLen)
        }

        var newBoundaries: [NSRange] = []
        newBoundaries.reserveCapacity(dividers.count + 1)
        var segStart = skipHeading(from: 0)

        for divider in dividers {
            // ⚠️ A divider inside the leading heading run cannot close a segment.
            guard divider >= segStart else { continue }
            newBoundaries.append(NSRange(location: segStart, length: divider - segStart))
            // Skip the attachment + its trailing newline, then any heading after it.
            segStart = skipHeading(from: divider + 2)
        }
        // Last (or only) segment runs to end of storage.
        newBoundaries.append(NSRange(location: segStart,
                                     length: max(0, fullLen - segStart)))

        return newBoundaries
    }
}


// MARK: — [I-0279]: the held cross-scene Range

/// Scrivi's cross-scene copy (a Range ScriviCore can paste back as scenes), valid only while the system pasteboard still
/// holds what that copy put there. ⛔ It was held forever (SP-089), so after one cross-scene copy every ⌘V pasted it —
/// whatever had been copied since ([I-0279], found in [SP-163]'s live pass). ✅ [SP-191] Q4a: it holds a Range, as the
/// buffers and history do.
struct StructuredClipboard {
    private var held: (range: ScriviRange, changeCount: Int)?

    mutating func hold(_ range: ScriviRange?, changeCount: Int) {
        held = range.map { ($0, changeCount) }
    }

    /// The Range, or nil once anything else has been copied (the change count moved on).
    func range(currentChangeCount: Int) -> ScriviRange? {
        guard let held, held.changeCount == currentChangeCount else { return nil }
        return held.range
    }
}

// MARK: — EP-045 AC11: the scene-boundary table, MAINTAINED (T-0583)

/// `sceneBoundaries`, kept current from each edit instead of rescanned over the whole manuscript.
/// ⛔ It used to be REBUILT by `SceneDivider.sceneBoundaries` (two whole-storage attribute scans)
/// on every keystroke: measured 2.3–3.9 ms on 1.85 MB (SP-157 AC10).
/// ✅ The shape Linux already ships (`SceneDocument::applyContentsChange`): an edit inside scene
/// `i` changes that scene's length and shifts every LATER scene by the edit's `changeInLength`.
/// ⚠️ Anything it cannot prove local — an edit spanning scenes, or text that carries a divider or
/// heading — marks the table DIRTY, and the next reader rescans. ⛔ ONE authority ([I-0131] was a
/// second scene-offset table drifting from the real one): readers take `ranges(in:)`, nothing else.
/// ⚠️ It observes a NOTIFICATION, not the storage delegate (the presenter's slot), and it needs
/// `willProcessEditing` anyway — see below.
/// ⛔ MEASURED 2026-10-04: by `didProcessEditing`, `editedRange` has been WIDENED to whole lines
/// (cause not traced), and a scene's LAST line holds its divider character — so every edit in a
/// scene's last line looked cross-scene and forced a rescan. ✅ `willProcessEditing` carries the
/// raw edit (the AC11 tests fail with `did`, pass with `will`).
final class SceneBoundaryTable: NSObject {
    private var table: [NSRange] = []
    private(set) var isDirty = true
    private weak var storage: NSTextStorage?

    func observe(_ storage: NSTextStorage) {
        if let old = self.storage {
            NotificationCenter.default.removeObserver(self, name: NSTextStorage.willProcessEditingNotification, object: old)
        }
        self.storage = storage
        isDirty = true
        NotificationCenter.default.addObserver(self, selector: #selector(storageWillProcessEditing(_:)),
                                               name: NSTextStorage.willProcessEditingNotification, object: storage)
    }

    /// The scene ranges, rescanned only when the table cannot be trusted.
    func ranges(in storage: NSAttributedString) -> [NSRange] {
        if isDirty, let scanned = SceneDivider.sceneBoundaries(in: storage) {
            table = scanned
            isDirty = false
        }
        return table
    }

    /// Install ranges already known to be exact (`rebuildStorage` builds them as it appends).
    func reset(_ ranges: [NSRange]) { table = ranges; isDirty = false }

    func invalidate() { isDirty = true }

    @objc private func storageWillProcessEditing(_ note: Notification) {
        guard let ts = note.object as? NSTextStorage, ts.editedMask.contains(.editedCharacters) else { return }
        apply(editedRange: ts.editedRange, changeInLength: ts.changeInLength, in: ts)
    }

    /// One processed edit. `editedRange` is in POST-edit coordinates.
    func apply(editedRange: NSRange, changeInLength delta: Int, in storage: NSAttributedString) {
        guard !isDirty, !table.isEmpty, editedRange.location != NSNotFound else { return }
        let start = editedRange.location
        let oldEnd = start + editedRange.length - delta
        // The last scene starting at or before the edit (binary search; scenes are ordered).
        var lo = 0, hi = table.count - 1
        while lo < hi {
            let mid = (lo + hi + 1) / 2
            if table[mid].location <= start { lo = mid } else { hi = mid - 1 }
        }
        let scene = table[lo]
        guard scene.location <= start, oldEnd <= scene.location + scene.length, oldEnd >= start else {
            isDirty = true; return
        }
        // Inserted text that brings structure (a divider or heading) changes the scene list.
        if editedRange.length > 0 {
            var structural = false
            for key in [NSAttributedString.Key.scriviDivider, .scriviHeading] where !structural {
                storage.enumerateAttribute(key, in: editedRange, options: []) { value, _, stop in
                    if value != nil { structural = true; stop.pointee = true }
                }
            }
            if structural { isDirty = true; return }
        }
        table[lo].length += delta
        for j in (lo + 1)..<table.count { table[j].location += delta }
    }
}

// Renders a 1pt horizontal rule across the full text column — the scene divider.
// No text, no label; purely visual separation.
//
// ⚠️ EP-039 T-0526 — REPLACES `DividerAttachmentCell: NSTextAttachmentCell`.
// ⚠️ `NSTextAttachmentCell` is TextKit 1 AND APPKIT-ONLY: there is no UIKit counterpart, so
// it blocked the iOS port on its own, before any performance question.
// ✅ `attachmentBounds(for:location:textContainer:proposedLineFragment:position:)` and
// `image(forBounds:attributes:location:textContainer:)` are `macos(12.0), ios(15.0)` — the
// SAME API on both platforms, so this class ports with only its drawing primitives changed.
//
// ✅ EP-045 AC1: dividers are found by the `.scriviDivider` KEY, whose value is this class's
// `renderState` — never by `.attachment`. ONE class; its rendering states live in the enum.
final class DividerTextAttachment: NSTextAttachment {

    // ✅ T-0575 (user request 2026-10-02): with chapter titles OFF nothing showed where a
    // chapter ended — *"slightly dim the current Scene separator and use the full color one
    // for the last scene in the Chapter."* ⚠️ Done by RAISING the chapter-end rule to
    // `labelColor`, NOT by dimming the scene rule: `secondaryLabelColor` (5.89:1) is the
    // measured visibility floor for a 1 px rule ([I-0252]; `tertiaryLabelColor` at 2.26:1
    // was rejected as too faint), so the relative difference is made above it.
    // ✅ Drawn the same with titles ON — what a divider means does not change with the toggle.
    var renderState: DividerRenderState = .sceneBreak

    override func attachmentBounds(
        for attributes: [NSAttributedString.Key: Any],
        location: any NSTextLocation,
        textContainer: NSTextContainer?,
        proposedLineFragment: CGRect,
        position: CGPoint
    ) -> CGRect {
        // ⚠️ Full column width, as the cell did. `proposedLineFragment` is the authority
        // under TextKit 2; the container may not be sized yet on first layout.
        let width = proposedLineFragment.width > 0
            ? proposedLineFragment.width
            : (textContainer?.size.width ?? 0)
        return CGRect(x: 0, y: 0, width: width, height: dividerCellHeight)
    }

    #if os(macOS)
    override func image(
        for bounds: CGRect,
        attributes: [NSAttributedString.Key: Any],
        location: any NSTextLocation,
        textContainer: NSTextContainer?
    ) -> NSImage? {
        // ⚠️ [I-0112]: `NSColor.separatorColor` is semantic and must be resolved against the
        // right appearance. ✅ An `NSImage` drawn with a handler resolves against the CURRENT
        // appearance at draw time, which the text view sets — so Light/Dark tracks correctly
        // without the cell's `controlView` dance.
        //
        // ✅ [I-0252] (T-0554, 2026-09-28): THAT COMMENT IS CORRECT AND WAS VERIFIED BY
        // MEASUREMENT — a harness rasterising this exact handler under each appearance got
        // `rgba(1,1,1,0.047)` under Dark and `rgba(0,0,0,0.047)` under Light. ⛔ The appearance
        // was NEVER the bug.
        //
        // ⛔ THE BUG WAS THE COLOUR. `separatorColor` is `white @ 9.8% alpha` in Dark, which
        // COMPOSITES over the editor background to a contrast ratio of **1.34 : 1** — against
        // `16.67 : 1` for body text. It is a chrome hairline meant to divide CONTROLS, and it
        // is invisible as a content mark inside a writing surface. ⚠️ It measures `1.25 : 1`
        // in Light too, so this was never a Dark Mode defect — Dark is only where it was
        // noticed.
        //
        // ✅ `secondaryLabelColor` measures `5.89 : 1` (Dark) and `3.95 : 1` (Light): visible
        // as a structural mark without competing with prose. ⚠️ `tertiaryLabelColor` was
        // measured too and rejected at `2.26 : 1` — still too faint for a 1 px rule.
        //
        // ⚠️ THIS IS A VISIBILITY FIX, NOT THE FINAL DESIGN. The scene break is to become a
        // typeset `* * *` mark (the writer must be able to SEE that the caret is at a scene
        // boundary, because operations depend on it) — see
        // `docs/Scrivi_Manuscript_Rendering_Trade_Study_v0_1.md` §1.5. ⛔ Do not elaborate
        // this drawing code in the meantime; it is scheduled to be replaced.
        let size = NSSize(width: max(bounds.width, 1), height: max(bounds.height, 1))
        return NSImage(size: size, flipped: false) { rect in
            // ⚠️ [I-0252] SECOND DEFECT, also measured: a 1 pt line centred on an INTEGRAL
            // y straddles the pixel grid, so antialiasing splits it across TWO rows at
            // HALF alpha each — measured `0.275 / 0.275` instead of one row at `0.549`.
            // ⛔ That halved the line's contrast on its own, independently of the colour.
            // ✅ Offsetting by half a point puts the 1 pt stroke inside ONE pixel row:
            // measured `0.549` on a single row.
            let lineY = rect.midY.rounded() + 0.5
            let path = NSBezierPath()
            path.lineWidth = 1.0
            path.move(to: NSPoint(x: rect.minX + 20, y: lineY))
            path.line(to: NSPoint(x: rect.maxX - 20, y: lineY))
            // T-0575 — first pass used `labelColor`; ⛔ user: *"The dividers are too subtle.
            // Perhaps a slight tint to the chapter divider?"* ✅ The ACCENT colour: a different
            // KIND of mark, not merely a darker line, and it follows the writer's own
            // System Settings accent.
            // ⚠️ 3rd pass — user: *"The tinted divider may be too much."* ✅ Softened to 60%.
            (self.renderState == .chapterEnd ? NSColor.controlAccentColor.withAlphaComponent(0.6)
                              : NSColor.secondaryLabelColor).setStroke()
            path.stroke()
            return true
        }
    }
    #else
    // ✅ EP-055 S1 (T-0633): the same rule on UIKit — the same width, half-point offset, colours and 60% accent as above.
    // shortcut: drawn against the trait collection current at layout; a Light/Dark switch shows once the line re-lays out.
    override func image(
        for bounds: CGRect,
        attributes: [NSAttributedString.Key: Any],
        location: any NSTextLocation,
        textContainer: NSTextContainer?
    ) -> UIImage? {
        let size = CGSize(width: max(bounds.width, 1), height: max(bounds.height, 1))
        return UIGraphicsImageRenderer(size: size).image { ctx in
            let lineY = (size.height / 2).rounded() + 0.5
            let path = UIBezierPath()
            path.lineWidth = 1.0
            path.move(to: CGPoint(x: 20, y: lineY))
            path.addLine(to: CGPoint(x: size.width - 20, y: lineY))
            (self.renderState == .chapterEnd ? UIColor.tintColor.withAlphaComponent(0.6) : UIColor.secondaryLabel).setStroke()
            path.stroke()
        }
    }
    #endif

    /// ✅ The ONE way to make a divider attachment, for every host.
    /// ⛔ [I-0252] / T-0554: without a placeholder `image` TextKit 2 never calls `image(for:)` and the divider draws
    /// NOTHING (measured 2026-09-29). 1×1 is deliberate: the override always replaces it; `attachmentBounds` owns the size.
    static func make(_ state: DividerRenderState) -> DividerTextAttachment {
        let attachment = DividerTextAttachment()
        attachment.renderState = state
        #if os(macOS)
        attachment.image = NSImage(size: NSSize(width: 1, height: 1))
        #else
        attachment.image = UIGraphicsImageRenderer(size: CGSize(width: 1, height: 1)).image { _ in }
        #endif
        return attachment
    }
}

// MARK: — C1: the write path — what an edit stores (design §2)
//
// ✅ EP-055 S1 (T-0633, Q1 = (c), ruled 2026-10-30): these rules USED TO live inside `ManuscriptNSTextView`'s AppKit overrides.
// They decide the bytes a keystroke puts in the `.md`, so every host asks them; a host only applies the answer.

@MainActor
enum ManuscriptEditRules {

    /// The presenter's stop runs in the form the snap takes.
    static func runLookup(_ presenter: ManuscriptPresenter, _ ts: NSAttributedString) -> (Int) -> MarkdownEscapes.StopRun? {
        let stops = presenter.stopTest(in: ts)
        return { i in stops(i).map { ($0.range, $0.homeAfter, $0.kind == .prefix || $0.kind == .listPrefix) } }
    }

    /// What a proposed edit may do.
    enum Verdict: Equatable {
        /// Let the text system make it.
        case allow
        case refuse
        /// Make THIS edit instead, then put the caret at `caret`. `balanced`: it was computed, so it must not be balanced again.
        case reissue(range: NSRange, replacement: String, caret: Int, balanced: Bool)
    }

    /// The rules every edit passes before it reaches storage (the Mac's `shouldChangeText`).
    /// `applyingBalancedEdit`: the host is applying an edit the presenter computed. `pastingOwnCopy`: Scrivi's own copy is
    /// being pasted (it keeps its own formatting).
    static func verdict(changing affectedCharRange: NSRange, with replacementString: String?, in storage: NSTextStorage,
                        presenter: ManuscriptPresenter?, applyingBalancedEdit: Bool, pastingOwnCopy: Bool) -> Verdict {
        guard affectedCharRange.length > 0 || (replacementString?.isEmpty == false) else { return .allow }
        // Block any edit that touches a character with the scriviHeading attribute.
        // For pure insertions (length == 0, non-empty replacement) check the character
        // AT the insertion point — do NOT look backward, as that would land inside
        // heading text when the cursor is at the start of a scene.
        // ⚠️ [I-0266] It used to check a ZERO-length range, which matches nothing, so
        // typing at offset 0 (before Chapter 1's heading) was let through. ✅ An insertion
        // whose next character is heading text is BEFORE or INSIDE a heading — the same
        // rule `caretInHeading` applies to structured paste.
        // For deletions (length > 0) or backspace-style (length == 0, empty replacement)
        // check the character being removed, falling back one position for backspace.
        let isInsertion = affectedCharRange.length == 0 && replacementString?.isEmpty == false
        let checkRange: NSRange
        if isInsertion {
            checkRange = NSRange(location: affectedCharRange.location, length: 1)
        } else {
            checkRange = affectedCharRange.length > 0
                ? affectedCharRange
                : NSRange(location: max(0, affectedCharRange.location - 1), length: 1)
        }
        var isHeading = false
        if checkRange.location < storage.length {
            let safeRange = NSRange(
                location: checkRange.location,
                length: min(checkRange.length, storage.length - checkRange.location)
            )
            if safeRange.length > 0 {
                storage.enumerateAttribute(.scriviHeading, in: safeRange, options: []) { value, _, stop in
                    if value != nil { isHeading = true; stop.pointee = true }
                }
            }
        }
        if isHeading {
            #if DEBUG
            NSLog("[SCRIVI-ROTOR] refused edit %@ (chapter title)", NSStringFromRange(affectedCharRange))
            #endif
            return .refuse
        }
        // ⛔ [I-0270] NEVER let an ordinary edit remove a scene DIVIDER. Each divider is one
        // scene boundary; `recomputeBoundaries` maps the Nth text segment to the Nth loaded
        // scene, so deleting one made the merged text save into the first scene (the second
        // scene's tail then existed TWICE on disk) and shifted every later scene's edits into
        // the PREVIOUS scene's file. Joining scenes is a structural op (⌘⌫ merge, cross-scene
        // ⌘X) that goes through ScriviCore and rebuilds — never a text edit.
        if affectedCharRange.length > 0, affectedCharRange.location < storage.length {
            let safe = NSRange(location: affectedCharRange.location,
                               length: min(affectedCharRange.length, storage.length - affectedCharRange.location))
            var hasDivider = false
            storage.enumerateAttribute(.scriviDivider, in: safe, options: []) { value, _, stop in
                if value != nil { hasDivider = true; stop.pointee = true }
            }
            if hasDivider { return .refuse }
        }
        // ✅ EP-045 AC4: an escape pair (`\*`) is ONE unit. ⚠️ Measured 2026-10-03: ⌫ after a `*`
        // removed only the `*` (leaving `a\b` — an orphaned backslash that becomes VISIBLE), and
        // ⌦ before the `\` removed only the backslash (leaving a LIVE `*`). Re-issue the edit
        // over the whole pair instead. (EP-046: the same for any stop run — a marker is never split.)
        // ⚠️ Never on an edit the presenter COMPUTED (balanced edits, Format commands): it is exact, and widening it
        // past a stop run it starts beside (a list's `1. `) replaced the prefix — [SP-163] live pass, step 12.
        if affectedCharRange.length > 0, !applyingBalancedEdit, let presenter,
           let pair = MarkdownEscapes.snapSelection(affectedCharRange, in: storage.string as NSString,
                                                    runAt: runLookup(presenter, storage)) {
            let replacement = replacementString ?? ""
            return .reissue(range: pair, replacement: replacement, caret: pair.location + replacement.utf16.count,
                            balanced: false)
        }
        // ✅ EP-046 AC12 / [SP-162] Q3: an edit that would leave emphasis UNBALANCED — a cut or delete
        // across a span edge, typing or pasting over one, a Return inside bold, Scrivi's own copy pasted
        // into bold — is re-issued as the BALANCED edit of the stretch it touches (`balancedEdit`). ONE
        // replacement, so ONE history event. ✅ Plain typing inside or outside a span returns nil there.
        if !applyingBalancedEdit, let presenter, let replacementString,
           let edit = presenter.balancedEdit(in: storage, replacing: affectedCharRange, with: replacementString,
                                             keepsOwnFormatting: pastingOwnCopy) {
            return .reissue(range: edit.range, replacement: edit.replacement, caret: edit.caret, balanced: true)
        }
        return .allow
    }

    /// What Return stores.
    enum ReturnEdit {
        /// In a list item: the presenter's edit (the next item, or the end of the list).
        case command(ManuscriptPresenter.CommandEdit)
        /// Otherwise a paragraph: `source` (already in stored form) over `range`.
        case paragraph(range: NSRange, source: String)
    }

    /// ✅ EP-045 AC5 + AC6 — Return starts a PARAGRAPH: it stores `\n\n` (study §3A.6), first
    /// reducing the trailing spaces before the caret to AT MOST ONE (§4B.6: two or more are a
    /// CommonMark hard break once a ⌫ merges the paragraphs) and collapsing a trailing typed
    /// backslash (`\\`) to `\`, so that a merge leaves the break the writer typed.
    /// ⛔ ONE replacement, never several: Return is a history commit boundary
    /// (the core's `CaptureSession::isCommitBoundary`, SP-177), so a split edit would commit half the gesture as its own undo step.
    static func returnEdit(in storage: NSTextStorage, selection sel: NSRange, presenter: ManuscriptPresenter?) -> ReturnEdit {
        // ✅ [SP-163] Return in a LIST ITEM starts the next item (an empty item ends the list), renumbering what follows.
        if let presenter, let edit = presenter.listReturnEdit(in: storage, selection: sel) {
            return .command(edit)
        }
        // ⚠️ Read backward from the caret only — ⛔ never copy the document (EP-045 trap #2).
        let ns = storage.string as NSString
        var start = sel.location
        while start > 0, ns.character(at: start - 1) == 0x20 { start -= 1 }
        var tail = start < sel.location ? " " : ""
        if tail.isEmpty {
            // Only an escape PAIR collapses: an even run of backslashes is all `\\` pairs.
            var run = 0
            while run < start, ns.character(at: start - 1 - run) == 0x5C { run += 1 }
            if run >= 2, run % 2 == 0 { start -= 2; tail = "\\" }
        }
        return .paragraph(range: NSRange(location: start, length: NSMaxRange(sel) - start), source: tail + "\n\n")
    }

    /// What ⌫ / ⌦ at a bare caret does.
    enum DeleteEdit: Equatable {
        /// The text system deletes one character.
        case system
        /// Nothing is deleted.
        case nothing
        /// Store `source` over `range` instead.
        case verbatim(range: NSRange, source: String)
    }

    /// ⌫ at a bare caret (no selection, no pending pair).
    static func deleteBackwardEdit(at loc: Int, in storage: NSTextStorage, presenter: ManuscriptPresenter?) -> DeleteEdit {
        guard loc > 0 else { return .nothing }
        // ✅ EP-046: markers are ATOMIC — ⌫ never deletes part of one. Just after an opening marker (the
        // start of a bold word, [SP-162] Q1) ⌫ acts on what the writer SEES before the caret: the
        // character before the marker, or the paragraph join. ⚠️ Just after a heading's `## ` (the
        // visible start of the heading) ⌫ removes the WHOLE prefix — the line becomes body text.
        if let presenter, let stop = presenter.stopTest(in: storage)(loc - 1), NSMaxRange(stop.range) == loc {
            switch stop.kind {
            case .prefix, .listPrefix:
                // ✅ [SP-163]: a list item's `- ` / `1. ` goes the same way — the item becomes body text.
                return .verbatim(range: stop.range, source: "")
            case .opener:
                let s = stop.range.location
                guard s > 0, !isSeparatorPosition(s - 1, in: storage) else { return .nothing }
                if let join = paragraphJoin(at: s, in: storage.string as NSString) {
                    return .verbatim(range: join, source: " ")
                }
                return .verbatim(range: NSRange(location: s - 1, length: 1), source: "")
            case .escape, .closer:
                break
            }
        }
        // The character that would be deleted is at loc-1.
        if isSeparatorPosition(loc - 1, in: storage) { return .nothing }
        if let join = paragraphJoin(at: loc, in: storage.string as NSString) {
            return .verbatim(range: join, source: " ")
        }
        return .system
    }

    /// ⌦ at a bare caret (no selection, no pending pair).
    static func deleteForwardEdit(at loc: Int, in storage: NSTextStorage, presenter: ManuscriptPresenter?) -> DeleteEdit {
        guard loc < storage.length else { return .nothing }
        // ✅ EP-046: just before a CLOSING marker (the end of a bold word) ⌦ deletes what the writer SEES
        // after the caret — the character after the marker — never the marker itself.
        if let presenter, let stop = presenter.stopTest(in: storage)(loc), stop.kind == .closer,
           stop.range.location == loc {
            let after = NSMaxRange(stop.range)
            guard after < storage.length, !isSeparatorPosition(after, in: storage) else { return .nothing }
            return .verbatim(range: NSRange(location: after, length: 1), source: "")
        }
        // The character that would be deleted is at loc.
        if isSeparatorPosition(loc, in: storage) { return .nothing }
        return .system
    }

    /// ✅ EP-045 Q3 (ruled 2026-10-04): ⌫ at the start of a paragraph JOINS it to the one above
    /// with ONE space — `a.⏎⏎b.` → `a. b.`. ⚠️ §3A.6 ruled "delete one `\n`" on the premise that
    /// a soft break RENDERS as a space; ⛔ E1 renders nothing, so it showed as a line break and
    /// the paragraphs never visibly joined (live check 2026-10-04). ✅ A space is what [EP-046]
    /// would render for that soft break anyway, so nothing needs reverting later.
    /// ✅ Q2 is subsumed: the trailing-space run becomes exactly ONE space and no newline is
    /// left, so no hard break can form. Returns the range to replace with `" "`, or nil to let
    /// ⌫ delete one character:
    /// - ⛔ the line above ends in a BARE `\` — the writer's deliberate hard break (§4B.6): one
    ///   `\n` goes, `end\⏎next` remains, and the styler hides the `\`;
    /// - the paragraph above or the one being joined is EMPTY — ⌫ just removes a blank line.
    static func paragraphJoin(at loc: Int, in ns: NSString) -> NSRange? {
        let nl = loc - 2   // the first newline of the `\n\n` before the caret
        guard nl > 0, ns.character(at: loc - 1) == 0x0A, ns.character(at: nl) == 0x0A else { return nil }
        guard loc < ns.length, ns.character(at: loc) != 0x0A else { return nil }
        var start = nl
        while start > 0, ns.character(at: start - 1) == 0x20 { start -= 1 }
        guard start > 0, ns.character(at: start - 1) != 0x0A else { return nil }
        if start == nl {
            var run = 0
            while run < start, ns.character(at: start - 1 - run) == 0x5C { run += 1 }
            if run % 2 == 1 { return nil }
        }
        return NSRange(location: start, length: loc - start)
    }

    /// True if the character at `pos` is part of a separator (the divider or its `\n`).
    static func isSeparatorPosition(_ pos: Int, in storage: NSTextStorage) -> Bool {
        guard pos >= 0, pos < storage.length else { return false }
        // The divider character itself (EP-045 AC1: by `.scriviDivider`, not `.attachment`).
        if SceneDivider.isDivider(in: storage, at: pos) { return true }
        // The \n that immediately follows a divider.
        if (storage.string as NSString).character(at: pos) == 10,
           pos >= 1,
           SceneDivider.isDivider(in: storage, at: pos - 1) { return true }
        return false
    }
}

// MARK: — C2: the model sync — what the storage and an edit do to the scene list (design §3 C2)
//
// ✅ EP-055 S1 (T-0633, Q1 = (c)): moved out of the Mac `Coordinator`. It needs the storage and the caret, never the view,
// so every host owns ONE of these and calls it from its own delegate callbacks.

@MainActor
final class ManuscriptSceneSync {
    var loader: ViewportSceneLoader
    var env: AppEnvironment
    var session: ProjectSession
    var showChapterTitles: Bool

    init(loader: ViewportSceneLoader, env: AppEnvironment, session: ProjectSession, showChapterTitles: Bool) {
        self.loader = loader
        self.env = env
        self.session = session
        self.showChapterTitles = showChapterTitles
    }

    // Character range for each scene segment in the NSTextStorage.
    // Dividers occupy 1 character each between segments.
    // ✅ EP-045 AC11 (T-0583): MAINTAINED by `boundaryTable` from each edit; read through it, it
    // is current or rescanned — never a stale copy (ONE authority, [I-0131]).
    let boundaryTable = SceneBoundaryTable()
    /// EP-046 E2-S1 — presents headings and hides escape backslashes (see `ManuscriptPresenter`).
    let presenter = ManuscriptPresenter()

    // Set true during a rebuild to suppress the text-changed callbacks
    // that fire from NSTextStorage delegate notifications mid-rebuild.
    var isRebuilding: Bool = false

    // Tracks which segment index the cursor was in at last check.
    var lastCursorSegmentIndex: Int = 0

    // Set by `textChanged` so the immediately-following selection-change
    // callback (fired on the same run loop turn) does not treat an edit as a
    // pure cursor move; cleared each turn. History trigger #2 (§4.a).
    private var editInFlight: Bool = false

    private var saveTask: Task<Void, Never>?
    private var titleTask: Task<Void, Never>?
    var highlightTask: Task<Void, Never>?
    var scrollTask: Task<Void, Never>?

    // The loaded segment at `index`, or nil if out of range.
    func segment(at index: Int) -> SceneSegment? {
        loader.segments.indices.contains(index) ? loader.segments[index] : nil
    }

    func sceneBoundaries(in storage: NSTextStorage) -> [NSRange] { boundaryTable.ranges(in: storage) }

    func segmentIndex(for characterLocation: Int, in storage: NSTextStorage) -> Int? {
        let boundaries = sceneBoundaries(in: storage)
        for (i, range) in boundaries.enumerated() {
            let end = range.location + range.length
            if characterLocation >= range.location && characterLocation <= end {
                return i
            }
        }
        // If after all segments, return last.
        return boundaries.isEmpty ? nil : boundaries.count - 1
    }

    // Scene-local UTF-8 byte offset of the insertion point. `sceneText` is
    // the scene's full text, `storageLoc` the storage char offset, `segRange`
    // the scene's storage char range. History offsets are UTF-8 bytes (§4.b).
    static func sceneLocalByteOffset(in sceneText: String, storageLoc: Int, segRange: NSRange) -> Int {
        let charOffset = max(0, min(storageLoc - segRange.location, (sceneText as NSString).length))
        let prefix = (sceneText as NSString).substring(to: charOffset)
        return prefix.utf8.count
    }

    func storageOffsetToManuscriptPosition(_ storageOffset: Int, in storage: NSTextStorage) -> Int {
        var separatorCount = 0
        var pos = 0
        while pos < storageOffset && pos < storage.length {
            if SceneDivider.isDivider(in: storage, at: pos) {
                separatorCount += 1
                pos += 2
            } else {
                pos += 1
            }
        }
        return storageOffset - (separatorCount * 2)
    }

    // MARK: Rendering

    static func chapterCount(_ segments: [SceneSegment]) -> Int {
        var n = 0
        for (i, seg) in segments.enumerated() where i == 0 || segments[i - 1].chapterID != seg.chapterID { n += 1 }
        return n
    }

    /// What goes before scene `i`: the divider (every scene but the first; the one that CLOSES a chapter at full
    /// strength, T-0575), then its chapter title at a chapter boundary when titles show.
    func renderedPrefix(_ i: Int, _ segments: [SceneSegment]) -> NSAttributedString {
        let out = NSMutableAttributedString()
        let seg = segments[i]
        let isChapterBoundary = i == 0 || segments[i - 1].chapterID != seg.chapterID
        if i > 0 {
            let state: DividerRenderState = isChapterBoundary ? .chapterEnd : .sceneBreak
            out.append(SceneDivider.string(DividerTextAttachment.make(state), state: state,
                                           newlineAttributes: presenter.typography.bodyAttributes))
        }
        if showChapterTitles && isChapterBoundary {
            let chapterTitle = loader.allScenes
                .first(where: { $0.sceneID == seg.sceneID })
                .map { info -> String in
                    let t = info.chapterTitle.trimmingCharacters(in: .whitespaces)
                    if !t.isEmpty { return t }
                    let chapterIDs = loader.allScenes.map(\.chapterID)
                    var seen: [String: Int] = [:]
                    var ordinal = 0
                    for cid in chapterIDs {
                        if seen[cid] == nil { ordinal += 1; seen[cid] = ordinal }
                    }
                    return "Chapter \(seen[info.chapterID] ?? ordinal)"
                } ?? ""
            // ✅ EP-047 P7 + P9: chapter titles in the manuscript face, at Heading 1 size, in the text colour.
            var headingAttrs = presenter.typography.chapterTitleAttributes
            headingAttrs[.scriviHeading] = true
            out.append(NSAttributedString(string: i == 0 ? "\(chapterTitle)\n" : "\n\(chapterTitle)\n",
                                          attributes: headingAttrs))
        }
        return out
    }

    /// The whole manuscript as the storage shows it, with each scene body's range. ✅ EP-047 AC4: the ONE source
    /// (font, line height); I-0112: it carries `.foregroundColor` (a run without it is black in Dark Mode).
    func renderedManuscript(_ segments: [SceneSegment]) -> (NSAttributedString, [NSRange]) {
        let out = NSMutableAttributedString()
        var ranges: [NSRange] = []
        ranges.reserveCapacity(segments.count)
        let attrs = presenter.typography.bodyAttributes
        for i in segments.indices {
            ScriviDiag.tick("rebuildStorage", i, of: segments.count)
            out.append(renderedPrefix(i, segments))
            let body = NSAttributedString(string: segments[i].text, attributes: attrs)
            ranges.append(NSRange(location: out.length, length: body.length))
            out.append(body)
        }
        return (out, ranges)
    }

    // MARK: An edit, and a caret move

    /// The writer changed the text with the caret at `loc`: the scene's text goes to the loader, the history hears the edit,
    /// the 1 s autosave and the 300 ms live title are rescheduled.
    func textChanged(in storage: NSTextStorage, caret loc: Int) {
        // While a history undo/redo apply mutates storage, the resulting
        // change is not a user edit — skip capture (still update the
        // loader below so in-memory text stays in sync).
        let isHistoryApply = session.historyCapture?.isApplying ?? false

        // T-0531 DIAGNOSTIC — per-keystroke cost, by step.
        let __k0 = Date()
        // Recompute boundaries from live storage — they shift with every keystroke.
        let boundaries = sceneBoundaries(in: storage)
        let __kBounds = Date()

        guard let segIdx = segmentIndex(for: loc, in: storage) else { return }
        let __kSeg = Date()

        // Extract this segment's text from storage.
        let range = boundaries[segIdx]
        let extracted = (storage.string as NSString).substring(with: range)
        let __kExtract = Date()

        // The scene's text *before* this edit (for first-edit baseline seeding).
        let preEditText = segment(at: segIdx)?.text ?? ""

        // Update loader in-memory; segment stays loaded.
        loader.updateText(extracted, at: segIdx)
        let __kUpdate = Date()
        defer {
            let ms = { (a: Date, b: Date) in b.timeIntervalSince(a) * 1000 }
            let total = Date().timeIntervalSince(__k0) * 1000
            if total > 0.5 {
                NSLog(String(format:
                    "[SCRIVI-KEY] total=%.1f  bounds=%.1f  segIdx=%.1f  extract=%.1f  update=%.1f  rest=%.1f (len=%d)",
                    total, ms(__k0,__kBounds), ms(__kBounds,__kSeg), ms(__kSeg,__kExtract),
                    ms(__kExtract,__kUpdate), Date().timeIntervalSince(__kUpdate) * 1000,
                    storage.length))
            }
        }

        if segIdx != lastCursorSegmentIndex {
            lastCursorSegmentIndex = segIdx
            loader.setCurrentIndex(segIdx)
        }

        // History capture (EP-019 §4.a): report the edit; ✅ SP-177: the CORE latches it and decides whether it ends
        // an undo step (a sentence terminator or Return; not trailing whitespace). `editInFlight` lets the following
        // selection-change callback know this run came from an edit.
        if !isHistoryApply, let capture = session.historyCapture,
           let sceneID = segment(at: segIdx)?.sceneID {
            let cursorCharOffset = loc - range.location
            // The history card's caret highlight reads UTF-8 bytes (EP-030) — not a capture rule.
            let cursorByte = Self.sceneLocalByteOffset(in: extracted, storageLoc: loc, segRange: range)
            loader.setCursorByteOffset(cursorByte, sceneID: sceneID)
            capture.noteEdit(sceneID: sceneID, text: extracted, caretUTF16: cursorCharOffset, textBefore: preEditText)
            editInFlight = true
        }

        // Debounce 1-second auto-save.
        saveTask?.cancel()
        let loader = loader
        let env = env
        let session = session
        saveTask = Task { @MainActor in
            try? await Task.sleep(nanoseconds: 1_000_000_000)
            guard !Task.isCancelled else { return }
            if let ref = env.authorshipRef {
                // T-0396: this no longer seals the typing session. Mid-session
                // it deliberately records nothing (so a continuously-typed
                // sentence stays ONE entry); it commits only if the writer has
                // already gone idle past the threshold. The scene is written to
                // disk either way — see the §4.d note on `flushThenSave`.
                session.historyCapture?.flushThenSave()
                await loader.saveCurrentIfDirty(engine: env.engine, ref: ref)
            }
        }

        // Debounce 300ms live-title update for the Navigator.
        titleTask?.cancel()
        let sceneID = loader.segments.indices.contains(segIdx)
            ? loader.segments[segIdx].sceneID : nil
        titleTask = Task { @MainActor in
            try? await Task.sleep(nanoseconds: 300_000_000)
            guard !Task.isCancelled, let sid = sceneID else { return }
            let firstLine = ManuscriptPresenter.presentedFirstLine(of: extracted)
            loader.updateLiveTitle(firstLine, forSceneID: sid)
            session.timelineModel?.updateDotTitles(liveTitles: loader.liveTitles, allScenes: loader.allScenes)
        }
    }

    /// The caret (or selection start) moved to `loc`.
    func selectionChanged(in storage: NSTextStorage, caret loc: Int) {
        guard let segIdx = segmentIndex(for: loc, in: storage) else { return }
        let boundaries = sceneBoundaries(in: storage)

        // History triggers #2 and #5 (§4.a), ✅ SP-177 decided by the CORE: a scene switch commits (hard); a caret
        // move in the same scene commits unless only whitespace changed (soft); a selection change that came from an
        // edit this turn, or from an undo/redo apply, is not a move.
        if let capture = session.historyCapture, let sceneID = segment(at: segIdx)?.sceneID {
            capture.caretMoved(sceneID: sceneID, editInFlight: editInFlight, applying: capture.isApplying)
        }
        editInFlight = false

        // Publish the caret's scene-local byte offset so the history card can bold
        // the entry whose change the caret sits inside (user request, 2026-08-06).
        // Done on every selection change, not just edits, so plain arrow-key moves
        // update the highlight too.
        if let seg = segment(at: segIdx), boundaries.indices.contains(segIdx) {
            let byte = Self.sceneLocalByteOffset(in: seg.text, storageLoc: loc,
                                                 segRange: boundaries[segIdx])
            loader.setCursorByteOffset(byte, sceneID: seg.sceneID)
        }

        if segIdx != lastCursorSegmentIndex {
            lastCursorSegmentIndex = segIdx
            loader.setCurrentIndex(segIdx)
            // Cursor moved to a new scene — update viewport scene immediately.
            // Cancel any pending scroll-based update; cursor movement takes priority.
            scrollTask?.cancel()
            highlightTask?.cancel()
            let loader = loader
            let sceneID = loader.segments.indices.contains(segIdx)
                ? loader.segments[segIdx].sceneID : nil
            highlightTask = Task { @MainActor in
                try? await Task.sleep(nanoseconds: 80_000_000) // 80ms debounce
                guard !Task.isCancelled else { return }
                loader.setViewportScene(sceneID)
            }
        }

        let manuscriptPos = storageOffsetToManuscriptPosition(loc, in: storage)
        loader.updateCursorPosition(manuscriptPos)

        // Record the scene-local cursor offset so the next save persists it as the
        // restored selection (I-0058).
        if boundaries.indices.contains(segIdx) {
            let sceneLocalOffset = max(0, loc - boundaries[segIdx].location)
            loader.updateSceneCursorOffset(sceneLocalOffset)
        }
    }
}
