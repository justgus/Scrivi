import SwiftUI
#if os(macOS)
import AppKit

// ForkPopover — the inline transient popover that appears at the caret when an
// undo/redo step lands on a fork (a history node with >= 2 children). It lists
// the fork's child branches (preview + timestamp) and lets the writer pick which
// branch to walk forward into. Trade T2 core interaction (EP-019 / SP-055 /
// design §10 T2, T-0211).
//
// Behavior (design §10 T2):
//  - Appears when an undo *lands on* a fork, or when a redo *reaches* a fork.
//  - Keyboard-first (Trade T2 option A): ↑/↓ move the highlight, ↩ selects the
//    highlighted branch, Esc dismisses without choosing.
//  - Transient: dismisses on its own when the writer clicks away or keeps
//    editing; the caller also dismisses it explicitly when the writer undoes
//    past the fork so it never obstructs.
//  - Selecting a branch re-primaries the fork (selectBranch) and redoes onto it;
//    dismissing without a choice leaves the existing primary child in place, so a
//    later redo follows the primary as before.
//
// This is a thin owner around an NSPopover whose content is a SwiftUI list. It
// holds no history state — the fork data comes from a HistoryForkAhead envelope
// and selection is reported back through an onSelect closure.
@MainActor
final class ForkPopoverController {

    private var popover: NSPopover?

    // The fork currently being offered, if the popover is showing. Used so the
    // caller can tell whether an in-flight step is still "on" this fork.
    private(set) var currentForkNodeID: String?

    var isShowing: Bool { popover?.isShown ?? false }

    // Shows the popover for `fork`, anchored at the caret in `textView`. If a
    // popover is already showing (e.g. for a different fork), it is replaced.
    //
    // onSelect(childEventID) is invoked when the writer chooses a branch; the
    // popover closes first, then the closure runs so the caller can selectBranch
    // + redo. onCancel() is invoked when the writer dismisses without choosing.
    func show(fork: HistoryForkAhead,
              in textView: NSTextView,
              onSelect: @escaping (_ childEventID: String) -> Void,
              onCancel: @escaping () -> Void) {
        close()   // replace any prior popover

        guard !fork.children.isEmpty else { onCancel(); return }
        currentForkNodeID = fork.nodeID

        let pop = NSPopover()
        pop.behavior = .transient       // dismisses when the writer clicks/edits away
        pop.animates = true

        // Preselect the primary child so a bare ↩ follows the primary branch,
        // matching "redoing without selecting follows the primary child" (§10 T2).
        let primaryIndex = fork.children.firstIndex(where: { $0.isPrimary }) ?? 0

        let content = ForkPopoverView(
            children: fork.children,
            initialSelection: primaryIndex,
            onSelect: { [weak self] childEventID in
                self?.close()
                onSelect(childEventID)
            },
            onCancel: { [weak self] in
                self?.close()
                onCancel()
            })

        let host = NSHostingController(rootView: content)
        // Size the popover to the content's fitting size (bounded so long scenes
        // don't produce an oversized popover — the row previews are trimmed).
        host.view.layoutSubtreeIfNeeded()
        let fitting = host.view.fittingSize
        pop.contentSize = NSSize(width: min(max(fitting.width, 260), 420),
                                 height: min(fitting.height, 320))
        pop.contentViewController = host

        // Anchor at the caret. firstRect(forCharacterRange:) returns a screen
        // rect for the insertion point; convert it into the text view's bounds so
        // the popover points at the caret rather than a corner.
        let caretRect = caretBoundsRect(in: textView)
        pop.show(relativeTo: caretRect, of: textView, preferredEdge: .maxY)
        popover = pop
    }

    // Explicitly closes the popover (e.g. the writer undid past the fork). Safe
    // to call when nothing is showing.
    func close() {
        popover?.performClose(nil)
        popover = nil
        currentForkNodeID = nil
    }

    // The caret's rect in the text view's own coordinate space, for anchoring.
    // Falls back to a zero-width rect at the selection's glyph bounds.
    private func caretBoundsRect(in textView: NSTextView) -> NSRect {
        let sel = textView.selectedRange()
        let caretRange = NSRange(location: sel.location, length: 0)
        // Screen rect for the caret, then map back into the text view.
        let screenRect = textView.firstRect(forCharacterRange: caretRange, actualRange: nil)
        if screenRect == .zero, let lm = textView.layoutManager, let tc = textView.textContainer {
            let glyphRange = lm.glyphRange(forCharacterRange: caretRange, actualCharacterRange: nil)
            let bounds = lm.boundingRect(forGlyphRange: glyphRange, in: tc)
            return bounds.offsetBy(dx: textView.textContainerOrigin.x,
                                   dy: textView.textContainerOrigin.y)
        }
        // screen → window → view.
        if let window = textView.window {
            let windowRect = window.convertFromScreen(screenRect)
            return textView.convert(windowRect, from: nil)
        }
        return screenRect
    }
}

// SwiftUI content of the fork popover: a keyboard-navigable list of branches.
private struct ForkPopoverView: View {
    let children: [HistoryForkChild]
    let onSelect: (String) -> Void
    let onCancel: () -> Void

    @State private var selection: Int
    @FocusState private var focused: Bool

    init(children: [HistoryForkChild],
         initialSelection: Int,
         onSelect: @escaping (String) -> Void,
         onCancel: @escaping () -> Void) {
        self.children = children
        self.onSelect = onSelect
        self.onCancel = onCancel
        _selection = State(initialValue: initialSelection)
    }

    var body: some View {
        VStack(alignment: .leading, spacing: 4) {
            Text("Redo into which branch?")
                .font(.caption)
                .foregroundStyle(.secondary)
                .padding(.horizontal, 10)
                .padding(.top, 8)
                .padding(.bottom, 2)

            ForEach(Array(children.enumerated()), id: \.element.eventID) { index, child in
                ForkRow(child: child, isSelected: index == selection)
                    .contentShape(Rectangle())
                    .onTapGesture { onSelect(child.eventID) }
                    .onHover { if $0 { selection = index } }
            }
            .padding(.horizontal, 6)

            Text("↑↓ to choose · ↩ to redo · esc to dismiss")
                .font(.caption2)
                .foregroundStyle(.tertiary)
                .padding(.horizontal, 10)
                .padding(.top, 2)
                .padding(.bottom, 8)
        }
        .frame(minWidth: 240)
        .focusable()
        .focused($focused)
        .onKeyPress(.upArrow)   { selection = max(0, selection - 1); return .handled }
        .onKeyPress(.downArrow) { selection = min(children.count - 1, selection + 1); return .handled }
        .onKeyPress(.return)    { onSelect(children[selection].eventID); return .handled }
        .onKeyPress(.escape)    { onCancel(); return .handled }
        .onAppear { focused = true }
    }
}

private struct ForkRow: View {
    let child: HistoryForkChild
    let isSelected: Bool

    var body: some View {
        HStack(spacing: 6) {
            VStack(alignment: .leading, spacing: 1) {
                HStack(spacing: 4) {
                    Text(child.preview.isEmpty ? "(no preview)" : child.preview)
                        .font(.body)
                        .lineLimit(1)
                        .truncationMode(.tail)
                    if child.isPrimary {
                        Text("primary")
                            .font(.caption2)
                            .foregroundStyle(.secondary)
                            .padding(.horizontal, 4)
                            .padding(.vertical, 1)
                            .background(.quaternary, in: Capsule())
                    }
                }
                if let when = HistoryTimestamp.friendly(child.timestamp) {
                    Text(when)
                        .font(.caption2)
                        .foregroundStyle(.secondary)
                }
            }
            Spacer(minLength: 0)
        }
        .padding(.horizontal, 8)
        .padding(.vertical, 5)
        .background(
            RoundedRectangle(cornerRadius: 6)
                .fill(isSelected ? AnyShapeStyle(.selection) : AnyShapeStyle(.clear))
        )
        .foregroundStyle(isSelected ? AnyShapeStyle(.white) : AnyShapeStyle(.primary))
    }
}
#endif
