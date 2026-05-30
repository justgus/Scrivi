import SwiftUI
import AppKit

struct EditorView: View {

    @Environment(AppEnvironment.self) private var env
    @State private var markdown: String = ""
    @State private var cursorOffset: Double = 0.0
    @State private var saveStatus: SaveStatus = .idle

    var body: some View {
        VStack(spacing: 0) {
            toolbar
            Divider()
            TrackingTextEditor(text: $markdown, cursorOffset: $cursorOffset)
                .frame(maxWidth: .infinity, maxHeight: .infinity)
        }
        .frame(minWidth: 600, minHeight: 400)
        .onAppear {
            markdown = env.openProjectResult?.activeScene?.markdown ?? ""
        }
        .onKeyPress("s", phases: .down) { press in
            guard press.modifiers.contains(.command) else { return .ignored }
            performSave()
            return .handled
        }
    }

    private var toolbar: some View {
        HStack {
            Text(env.openProjectResult?.projectID ?? "")
                .font(.caption)
                .foregroundStyle(.secondary)
                .lineLimit(1)
                .truncationMode(.middle)

            Spacer()

            saveStatusView

            Button("Close Project") {
                env.openProjectResult = nil
                env.projectRootPath = nil
                env.projectError = nil
            }
        }
        .padding(.horizontal, 12)
        .padding(.vertical, 8)
    }

    @ViewBuilder
    private var saveStatusView: some View {
        switch saveStatus {
        case .idle:
            EmptyView()
        case .saving:
            ProgressView()
                .controlSize(.small)
                .padding(.trailing, 8)
        case .saved:
            Text("Saved")
                .font(.caption)
                .foregroundStyle(.secondary)
                .padding(.trailing, 8)
        case .failed(let msg):
            Text(msg)
                .font(.caption)
                .foregroundStyle(.red)
                .padding(.trailing, 8)
        }
    }

    private func performSave() {
        guard saveStatus != .saving else { return }
        saveStatus = .saving
        let currentMarkdown = markdown
        let offset = cursorOffset
        Task {
            await env.saveScene(markdown: currentMarkdown, cursorOffset: offset, scrollPosition: 0.0)
            if let err = env.projectError {
                saveStatus = .failed(err.message)
            } else {
                saveStatus = .saved
                try? await Task.sleep(nanoseconds: 2_000_000_000)
                saveStatus = .idle
            }
        }
    }
}

// MARK: — Save status

private enum SaveStatus: Equatable {
    case idle
    case saving
    case saved
    case failed(String)
}

// MARK: — Cursor-tracking NSTextView wrapper

private struct TrackingTextEditor: NSViewRepresentable {

    @Binding var text: String
    @Binding var cursorOffset: Double

    func makeCoordinator() -> Coordinator { Coordinator(self) }

    func makeNSView(context: Context) -> NSScrollView {
        let textView = NSTextView()
        textView.isEditable = true
        textView.isRichText = false
        textView.font = NSFont.monospacedSystemFont(ofSize: NSFont.systemFontSize, weight: .regular)
        textView.autoresizingMask = [.width]
        textView.isVerticallyResizable = true
        textView.textContainer?.widthTracksTextView = true
        textView.delegate = context.coordinator
        textView.string = text

        let scroll = NSScrollView()
        scroll.documentView = textView
        scroll.hasVerticalScroller = true
        scroll.autohidesScrollers = true
        return scroll
    }

    func updateNSView(_ scrollView: NSScrollView, context: Context) {
        guard let textView = scrollView.documentView as? NSTextView else { return }
        if textView.string != text {
            let sel = textView.selectedRanges
            textView.string = text
            textView.selectedRanges = sel
        }
    }

    @MainActor
    final class Coordinator: NSObject, NSTextViewDelegate {
        var parent: TrackingTextEditor

        init(_ parent: TrackingTextEditor) { self.parent = parent }

        func textDidChange(_ notification: Notification) {
            guard let tv = notification.object as? NSTextView else { return }
            parent.text = tv.string
            updateCursor(tv)
        }

        func textViewDidChangeSelection(_ notification: Notification) {
            guard let tv = notification.object as? NSTextView else { return }
            updateCursor(tv)
        }

        private func updateCursor(_ tv: NSTextView) {
            let loc = tv.selectedRange().location
            parent.cursorOffset = Double(loc == NSNotFound ? 0 : loc)
        }
    }
}
