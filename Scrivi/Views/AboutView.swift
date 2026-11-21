import SwiftUI
#if os(macOS)
import AppKit
#else
import UIKit
#endif

/// [SP-229] Q5: the Wynn stanza of the Old English Rune Poem — ᚹ, "joy", the *wyn* in Inkwyn — with our own translation.
/// ⛔ The rune itself is for the logo only (user, 2026-11-20), so the stanza starts at its first word.
enum WynnStanza {
    static let oldEnglish = "Wenne bruceþ, ðe can weana lyt\nsares and sorge and him sylfa hæfþ\nblæd and blysse and eac byrga geniht."
    static let english = "Joy is his who knows few woes,\nlittle pain or sorrow, and has for himself\nplenty and bliss, and shelter enough."
    static let source = "The Old English Rune Poem, wynn"
}

struct AboutView: View {

    @Environment(\.dismiss) private var dismiss

    private var appName: String {
        Bundle.main.object(forInfoDictionaryKey: "CFBundleName") as? String ?? "Inkwyn"
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

            VStack(spacing: 8) {
                Text(WynnStanza.oldEnglish).italic()
                Text(WynnStanza.english)
                Text("— \(WynnStanza.source)").foregroundStyle(.secondary)
            }
            .font(.footnote)
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
            .credits: credits
        ])
    }

    private static var credits: NSAttributedString {
        let centered = NSMutableParagraphStyle()
        centered.alignment = .center
        let regular = NSFont.systemFont(ofSize: NSFont.smallSystemFontSize)
        let italic = NSFontManager.shared.convert(regular, toHaveTrait: .italicFontMask)
        let text = NSMutableAttributedString()
        func add(_ s: String, _ font: NSFont, _ color: NSColor = .labelColor) {
            text.append(NSAttributedString(string: s, attributes: [.font: font, .foregroundColor: color, .paragraphStyle: centered]))
        }
        add("A multi-platform authoring and worldbuilding app.\n\n", regular)
        add(WynnStanza.oldEnglish + "\n\n", italic)
        add(WynnStanza.english + "\n\n", regular)
        add("— " + WynnStanza.source, regular, .secondaryLabelColor)
        return text
    }
}
#endif

