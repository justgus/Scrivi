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
    private var build: String {
        Bundle.main.object(forInfoDictionaryKey: "CFBundleVersion") as? String ?? "—"
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

            Text("Version \(version) (\(build))")
                .font(.callout)
                .foregroundStyle(.secondary)

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
