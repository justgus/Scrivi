import SwiftUI
#if os(macOS)
import AppKit
#else
import UIKit
#endif

struct AboutView: View {

    @Environment(\.dismiss) private var dismiss

    private var appName: String {
        Bundle.main.object(forInfoDictionaryKey: "CFBundleName") as? String ?? "Scrivi"
    }
    private var version: String {
        Bundle.main.object(forInfoDictionaryKey: "CFBundleShortVersionString") as? String ?? "—"
    }
    /// [T-0622]: the commit it was built from (`scripts/stamp-version.sh`), `-dirty` with uncommitted changes.
    private var commit: String {
        Bundle.main.object(forInfoDictionaryKey: "ScriviCommit") as? String ?? "—"
    }

    private var appIcon: Image {
        #if os(macOS)
        return Image(nsImage: NSApp.applicationIconImage)
        #else
        return Image(uiImage: UIImage(named: "AppIcon") ?? UIImage(systemName: "pencil.and.outline")!)
        #endif
    }

    var body: some View {
        VStack(spacing: 16) {
            appIcon
                .resizable()
                .frame(width: 80, height: 80)
                .cornerRadius(16)

            Text(appName)
                .font(.title2.bold())

            Text("Version \(version) (\(commit))")
                .font(.callout)
                .foregroundStyle(.secondary)
                .textSelection(.enabled)   // a tester copies it into a report

            Text("A multi-platform authoring and worldbuilding app.")
                .font(.footnote)
                .foregroundStyle(.tertiary)
                .multilineTextAlignment(.center)

            Button("Close") { dismiss() }
                .keyboardShortcut(.defaultAction)
        }
        .padding(32)
        #if os(macOS)
        .frame(width: 300)
        #endif
    }
}

#if os(macOS)
/// [SP-193] The Mac's About: AppKit's standard panel. ⛔ The `AboutView` sheet hung off the Welcome window, which is not
/// on screen while a project is open, so About showed nothing (live pass, 2026-10-24). The panel needs no window.
/// It reads "Version 1.0.296 (a566a30)": `ApplicationVersion` (the marketing version), then `Version` in brackets
/// (NSApplication.h), here the commit ([T-0622]).
enum AboutPanel {
    @MainActor static func show() {
        let info = Bundle.main.infoDictionary ?? [:]
        NSApp.orderFrontStandardAboutPanel(options: [
            .applicationVersion: info["CFBundleShortVersionString"] as? String ?? "—",
            .version: info["ScriviCommit"] as? String ?? "—",
            .credits: NSAttributedString(string: "A multi-platform authoring and worldbuilding app.")
        ])
    }
}
#endif

