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
