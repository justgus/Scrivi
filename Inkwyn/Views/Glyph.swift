import SwiftUI

// EP-060: the Inkwyn glyphs are custom symbols in Assets.xcassets/Glyphs (made by scripts/make-glyph-symbols.swift),
// named `inkwyn.*`. A call site that holds a symbol name as a String draws it through these, so one name can be either
// an Inkwyn glyph or an SF Symbol; both size with the font and tint with foregroundStyle.

extension Image {
    init(glyph name: String) {
        if name.hasPrefix("inkwyn.") { self.init(name) } else { self.init(systemName: name) }
    }
}

extension Label where Title == Text, Icon == Image {
    // The (title, image:) / (title, systemImage:) forms, not a custom icon closure: a macOS menu item shows only those.
    init(_ title: some StringProtocol, glyph name: String) {
        if name.hasPrefix("inkwyn.") { self.init(title, image: name) } else { self.init(title, systemImage: name) }
    }
}

/// EP-060 SP-242: a glyph drawn ASPECT-FIT in a box tied to the text size, using the chosen drawing (Large by default). The box is
/// wider than tall so a glyph's pips sit beside the art instead of shrinking it (user, 2026-11-24: "the adaptation is too small").
/// The 2× glyphs differ widely in width, so a fixed box is the only way they all fit a toolbar button (user, 2026-11-24:
/// "Large in the toolbar"). ⚠️ Only where SwiftUI lays the image out: a menu item takes the image alone and ignores this.
struct FittedGlyph: View {
    let name: String
    let scale: Image.Scale
    let aspect: CGFloat   // width ÷ height
    @ScaledMetric private var height: CGFloat

    /// `height` is at body text size and grows with it.
    init(name: String, scale: Image.Scale = .large, height: CGFloat = 19.5, aspect: CGFloat = 1.4) {   // 26 at 2×; 1.5× (SP-242)
        self.name = name; self.scale = scale; self.aspect = aspect
        _height = ScaledMetric(wrappedValue: height, relativeTo: .body)
    }

    var body: some View {
        Image(name).resizable().scaledToFit().imageScale(scale).frame(width: height * aspect, height: height)
    }
}
