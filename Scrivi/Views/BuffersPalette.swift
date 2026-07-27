import SwiftUI
#if os(macOS)
import AppKit

// BuffersPalette — the floating, non-modal utility panel that shows the nine
// persistent copy-buffer slots (EP-019 SP-056, T-0214; Trade T4 option B). It is a
// live mirror of the project's slots 1–9: each row shows the slot number, a content
// preview, and when it was last updated, with click-to-paste and load-from-selection.
//
// Buffer 0 (the system pasteboard) is deliberately not shown — it is ordinary ⌘C/⌘V,
// not a Scrivi-managed slot, and lives outside buffers.json.
//
// The panel is owned by BuffersPanelController (an NSPanel wrapper mirroring the
// ForkPopover NSPopover pattern). Toggled from the View menu (⌥⌘B); it tracks the
// frontmost project's BufferService and stays above the editor without stealing key.
//
// The actual paste/load actions are performed by the editor coordinator (so text
// flows through the history + auto-save path); the palette reports intent through
// closures rather than mutating text itself.

// MARK: — Panel controller

@MainActor
final class BuffersPanelController {

    private var panel: NSPanel?
    private var host: NSHostingController<BuffersPaletteView>?

    var isShowing: Bool { panel?.isVisible ?? false }

    // Shows the palette for `service`, wiring the row actions to the front project's
    // paste/load/clear. Called every time the frontmost project or the visible toggle
    // changes: if the panel already exists it is RE-POINTED at `service` (its hosted
    // content is rebuilt) so switching projects reloads the slots to the front project;
    // otherwise a new panel is created. `onClose` fires when the user closes the panel.
    func show(service: BufferService,
              onPaste: @escaping (_ bufferID: String) -> Void,
              onLoadFromSelection: @escaping (_ bufferID: String) -> Void,
              onCut: @escaping (_ bufferID: String) -> Void,
              onClear: @escaping (_ bufferID: String) -> Void,
              onClose: @escaping () -> Void) {
        let content = BuffersPaletteView(
            service: service,
            onPaste: onPaste,
            onLoadFromSelection: onLoadFromSelection,
            onCut: onCut,
            onClear: onClear)

        if let panel, let host {
            host.rootView = content       // re-point at the new front project's buffers
            panel.orderFront(nil)
            return
        }

        let hc = NSHostingController(rootView: content)
        let p = NSPanel(contentViewController: hc)
        p.styleMask = [.titled, .closable, .utilityWindow, .nonactivatingPanel]
        p.title = "Buffers"
        p.isFloatingPanel = true
        p.hidesOnDeactivate = false
        p.becomesKeyOnlyIfNeeded = true          // don't steal key from the editor
        p.setContentSize(NSSize(width: 280, height: 360))
        p.delegate = closeObserver(onClose)
        p.center()
        p.orderFront(nil)
        panel = p
        host = hc
    }

    // Closes and releases the panel. Safe when nothing is showing.
    func close() {
        panel?.orderOut(nil)
        panel = nil
        host = nil
        observer = nil
    }

    // Retained window-delegate that reports a user-initiated close back to the caller.
    private var observer: PanelCloseObserver?
    private func closeObserver(_ onClose: @escaping () -> Void) -> PanelCloseObserver {
        let o = PanelCloseObserver { [weak self] in
            self?.panel = nil
            self?.host = nil
            self?.observer = nil
            onClose()
        }
        observer = o
        return o
    }
}

// Reports the panel's close button back to the controller so the View-menu toggle
// stays in sync when the user dismisses the palette directly.
private final class PanelCloseObserver: NSObject, NSWindowDelegate {
    private let onClose: () -> Void
    init(onClose: @escaping () -> Void) { self.onClose = onClose }
    func windowWillClose(_ notification: Notification) { onClose() }
}

// MARK: — Modifier tracking

// The action a row's single action button performs, chosen by the modifier the writer
// is holding — mirroring the ⌘/⌃/⌥ + digit keyboard chords exactly.
private enum BufferAction {
    case copy    // no modifier  → ⌘N: copy selection into the slot (down arrow)
    case paste   // Control held → ⌃N: paste the slot at the caret (up arrow)
    case cut     // Option held  → ⌥N: cut selection into the slot (scissors)

    // Which modifier drives this action, derived from live NSEvent flags. Command,
    // Shift, or no relevant modifier all fall back to copy (the default, matching ⌘N).
    static func current(_ flags: NSEvent.ModifierFlags) -> BufferAction {
        if flags.contains(.control) { return .paste }
        if flags.contains(.option)  { return .cut }
        return .copy
    }

    // Standard Cut/Copy/Paste iconography so the button's function reads at a glance.
    var symbol: String {
        switch self {
        case .copy:  return "doc.on.doc"        // standard Copy
        case .paste: return "doc.on.clipboard"  // standard Paste
        case .cut:   return "scissors"          // standard Cut
        }
    }
}

// Publishes the currently-held modifier flags so the palette buttons can reflect them
// live (⌃ → paste icon, ⌥ → cut icon), the way vim registers change meaning by prefix.
// SwiftUI has no built-in live modifier state, so we watch NSEvent flagsChanged. The
// monitor is local (this process only) and removed on deinit.
@MainActor
private final class ModifierWatcher: ObservableObject {
    @Published var flags: NSEvent.ModifierFlags = []
    // nonisolated(unsafe): the monitor token is only ever created in init and read in
    // deinit (never concurrently), so touching it from the nonisolated deinit is safe.
    private nonisolated(unsafe) var monitor: Any?

    init() {
        monitor = NSEvent.addLocalMonitorForEvents(matching: .flagsChanged) { [weak self] event in
            self?.flags = event.modifierFlags
            return event
        }
    }
    deinit { if let monitor { NSEvent.removeMonitor(monitor) } }
}

// MARK: — Palette content

struct BuffersPaletteView: View {
    let service: BufferService
    let onPaste: (String) -> Void
    let onLoadFromSelection: (String) -> Void
    let onCut: (String) -> Void
    let onClear: (String) -> Void

    @StateObject private var modifiers = ModifierWatcher()

    var body: some View {
        let action = BufferAction.current(modifiers.flags)
        VStack(alignment: .leading, spacing: 0) {
            Text("Copy Buffers")
                .font(.headline)
                .padding(.horizontal, 12)
                .padding(.top, 10)
                .padding(.bottom, 2)
            // The hint states all three chords and how the button follows the modifier.
            Text("⌘1–9 copy · ⌃1–9 paste · ⌥1–9 cut  ·  hold ⌃/⌥ to change the button")
                .font(.caption2)
                .foregroundStyle(.secondary)
                .fixedSize(horizontal: false, vertical: true)
                .padding(.horizontal, 12)
                .padding(.bottom, 6)

            ScrollView {
                VStack(spacing: 2) {
                    ForEach(BufferService.slotIDs, id: \.self) { slotID in
                        BufferRow(
                            slotID: slotID,
                            preview: service.preview(forSlot: slotID),
                            updatedAt: service.slots.first(where: { $0.bufferID == slotID })?.updatedAt,
                            action: action,
                            onAction: {
                                switch action {
                                case .copy:  onLoadFromSelection(slotID)
                                case .paste: onPaste(slotID)
                                case .cut:   onCut(slotID)   // load + delete + `cut` event (coordinator's ⌥ path)
                                }
                            },
                            onClear: { onClear(slotID) })
                    }
                }
                .padding(.horizontal, 8)
                .padding(.bottom, 8)
            }
        }
        .frame(minWidth: 300, minHeight: 320)
    }
}

private struct BufferRow: View {
    let slotID: String
    let preview: String?
    let updatedAt: String?
    let action: BufferAction
    let onAction: () -> Void
    let onClear: () -> Void

    private var isEmpty: Bool { preview?.isEmpty ?? true }

    // The action button is disabled only when it can't do anything: paste needs a
    // non-empty slot; copy/cut always work (they read the editor selection).
    private var actionDisabled: Bool { action == .paste && isEmpty }

    private var actionHelp: String {
        switch action {
        case .copy:  return "Copy the selected manuscript text into buffer \(slotID) (⌘\(slotID))"
        case .paste: return isEmpty
            ? "Buffer \(slotID) is empty — nothing to paste"
            : "Paste buffer \(slotID) at the cursor, replacing any selection (⌃\(slotID))"
        case .cut:   return "Cut the selected manuscript text into buffer \(slotID) (⌥\(slotID))"
        }
    }

    var body: some View {
        HStack(spacing: 8) {
            Text(slotID)
                .font(.system(.body, design: .monospaced).weight(.semibold))
                .frame(width: 18)
                .foregroundStyle(isEmpty ? AnyShapeStyle(.tertiary) : AnyShapeStyle(.primary))

            VStack(alignment: .leading, spacing: 1) {
                Text(isEmpty ? "empty" : (preview ?? ""))
                    .font(.body)
                    .lineLimit(1)
                    .truncationMode(.tail)
                    .foregroundStyle(isEmpty ? AnyShapeStyle(.secondary) : AnyShapeStyle(.primary))
                if let updatedAt, let when = HistoryTimestamp.friendly(updatedAt) {
                    Text(when)
                        .font(.caption2)
                        .foregroundStyle(.secondary)
                }
            }

            Spacer(minLength: 0)

            // One modifier-sensitive action button. Its icon follows the held modifier
            // (↓ copy / ↑ paste / ✂ cut), mirroring the ⌘/⌃/⌥ + digit chords, so the
            // button IS the chord. Rendered inert (not .disabled) so the tooltip still
            // attaches even when a paste would no-op.
            Button(action: { if !actionDisabled { onAction() } }) {
                Image(systemName: action.symbol)
                    .foregroundStyle(actionDisabled ? AnyShapeStyle(.tertiary) : AnyShapeStyle(.primary))
            }
            .buttonStyle(.borderless)
            .allowsHitTesting(!actionDisabled)
            .help(actionHelp)

            // Clear this slot. Rendered inert (not .disabled) when empty so the tooltip
            // still attaches — a .disabled button drops its .help.
            Button(action: { if !isEmpty { onClear() } }) {
                Image(systemName: "xmark.circle")
                    .foregroundStyle(isEmpty ? AnyShapeStyle(.tertiary) : AnyShapeStyle(.secondary))
            }
            .buttonStyle(.borderless)
            .allowsHitTesting(!isEmpty)
            .help(isEmpty ? "Buffer \(slotID) is empty" : "Clear buffer \(slotID)")
        }
        .padding(.horizontal, 8)
        .padding(.vertical, 5)
        .background(
            RoundedRectangle(cornerRadius: 6)
                .fill(.quaternary.opacity(0.35)))
    }
}
#endif
