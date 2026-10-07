import CoreText
import Foundation
#if canImport(AppKit)
import AppKit
#endif

// EP-047 S2 (SP-168, T-0597) — the manuscript's TYPE: the bundled faces, and the one source every type decision reads.

// MARK: — The bundled faces (all Apple platforms)

/// The typefaces Scrivi ships (EP-047 P5a). ✅ The list is `Fonts/fonts.json` in the app bundle — the SAME file the Linux app
/// (EP-048 L10) and the specimen generator read. ⛔ Never restate the faces in code: derive them from this.
enum BundledFonts {

    struct FontFile: Decodable, Sendable {
        let file: String
        let italic: Bool
        let weight: Int
    }

    struct Face: Decodable, Sendable, Identifiable, Equatable {
        let name: String
        let role: String
        let folder: String
        let variable: Bool
        let files: [FontFile]
        var id: String { name }
        static func == (a: Face, b: Face) -> Bool { a.name == b.name }
    }

    struct Manifest: Decodable, Sendable {
        let `default`: String
        let faces: [Face]
    }

    /// The bundle's `Fonts` folder (`Resources/Fonts` in the repo, copied whole — licenses included).
    static let directory: URL? = Bundle.main.url(forResource: "Fonts", withExtension: nil)

    static let manifest: Manifest = {
        guard let dir = directory,
              let data = try? Data(contentsOf: dir.appendingPathComponent("fonts.json")),
              let m = try? JSONDecoder().decode(Manifest.self, from: data), !m.faces.isEmpty else {
            NSLog("[BundledFonts] ⚠️ Fonts/fonts.json missing or unreadable — the manuscript falls back to the system font")
            return Manifest(default: "", faces: [])
        }
        return m
    }()

    static var faces: [Face] { manifest.faces }

    /// The face called `name`, or the default face (P6) when no bundled face has that name — a project set in a face from a
    /// LATER Scrivi still opens. ⚠️ The caller keeps the stored name; only the drawing falls back.
    static func face(named name: String) -> Face? {
        faces.first { $0.name == name } ?? faces.first { $0.name == manifest.default }
    }

    static func url(of file: FontFile, in face: Face) -> URL? {
        directory?.appendingPathComponent(face.folder).appendingPathComponent(file.file)
    }

    /// Registers every bundled font for THIS process — macOS 10.15+, iOS 13+, visionOS 1+ (EP-047 P5). Call once at launch.
    static func register() {
        let urls = faces.flatMap { face in face.files.compactMap { url(of: $0, in: face) } }
        guard !urls.isEmpty else { return }
        CTFontManagerRegisterFontURLs(urls as CFArray, .process, true) { errors, _ in
            let list = errors as? [CFError] ?? []
            if !list.isEmpty { NSLog("[BundledFonts] \(list.count) font(s) not registered: \(list)") }
            return true
        }
    }
}

// MARK: — The manuscript's typography

/// THE one source of the manuscript's type (EP-047 AC4). ⛔ Every font, size and line height the manuscript draws with comes
/// from here — the storage sites in `ManuscriptTextView`, the presenter's headings and emphasis, the chapter titles. A second
/// source is the trap EP-047 recorded: a face that vanishes on undo, on rebuild or on Replace All.
/// ✅ Rulings: P2/P8 per project, P5a the bundled faces, P6 default Literata, P7 headings and chapter titles in the face, P9 16 pt
/// on a fixed 1.45 line, chapter titles at Heading 1 size in the text colour.
struct ManuscriptTypography: Equatable, Sendable {

    /// The face's NAME as stored in `project-settings.json` — kept even when this build does not bundle it.
    let faceName: String
    let size: CGFloat

    static let defaultSize: CGFloat = 16
    static let sizeRange: ClosedRange<CGFloat> = 10...32
    /// P9: body lines sit on a FIXED 1.45 × size line, the same for every face (a face's own leading varies).
    static let lineSpacing: CGFloat = 1.45
    /// Heading and chapter-title lines: 1.25 × their own size (the specimen the user chose from).
    static let headingLineSpacing: CGFloat = 1.25

    static var `default`: ManuscriptTypography {
        ManuscriptTypography(faceName: BundledFonts.manifest.default, size: defaultSize)
    }

    init(faceName: String, size: CGFloat) {
        self.faceName = faceName
        self.size = min(max(size, Self.sizeRange.lowerBound), Self.sizeRange.upperBound)
    }

    /// The face drawn — the stored one, or the default if this build does not bundle it.
    var face: BundledFonts.Face? { BundledFonts.face(named: faceName) }
}

#if canImport(AppKit)

// The drawing half (macOS — the only manuscript surface today; iOS/visionOS have a placeholder).
extension ManuscriptTypography {

    // MARK: Sizes (Apple's SP-161 Q1 heading sizes, 22 / 18 / 16 over a 13 pt body, kept as RATIOS)

    func headingSize(level: Int) -> CGFloat {
        switch level {
        case 1: size * 22 / 13
        case 2: size * 18 / 13
        case 3: size * 16 / 13
        default: size
        }
    }

    // MARK: Fonts

    var bodyFont: NSFont { font(size: size, weight: 400, italic: false) }

    /// A heading (level 1–6) or body (nil) run with inline `style` bits (`MarkdownBlocks.bold` / `.italic`).
    /// ✅ Headings are bold (700). Bold is 700 in body text; bold INSIDE a heading is the face's HEAVIEST weight, so it still
    /// shows ([SP-162] live pass: "bold is one real weight step above the text").
    func font(level: Int?, style: UInt8) -> NSFont {
        let s = level.map(headingSize(level:)) ?? size
        let bold = style & MarkdownBlocks.bold != 0
        let weight: CGFloat = level == nil ? (bold ? 700 : 400) : (bold ? heaviestWeight : 700)
        return font(size: s, weight: weight, italic: style & MarkdownBlocks.italic != 0)
    }

    /// P9: chapter titles are Heading 1, in the text colour.
    var chapterTitleFont: NSFont { font(level: 1, style: 0) }

    var heaviestWeight: CGFloat {
        guard let face else { return 700 }
        return Self.weightRange(face).upperBound
    }

    // MARK: Paragraphs

    /// THE paragraph-style builder: every manuscript paragraph style is made here (body, heading, list — and S3's first-line
    /// indent), so none can lose the line height. `lineFor` is the line's font size (a heading's own size).
    func paragraphStyle(lineFor fontSize: CGFloat? = nil, heading: Bool = false, headIndent: CGFloat = 0) -> NSParagraphStyle {
        let line = (fontSize ?? size) * (heading ? Self.headingLineSpacing : Self.lineSpacing)
        let p = NSMutableParagraphStyle()
        p.minimumLineHeight = line
        p.maximumLineHeight = line
        p.headIndent = headIndent
        return p
    }

    /// The attributes of plain body text in storage — every site that writes body text uses exactly these.
    var bodyAttributes: [NSAttributedString.Key: Any] {
        [.font: bodyFont, .foregroundColor: NSColor.textColor, .paragraphStyle: paragraphStyle()]
    }

    /// A chapter title as stored (EP-047 P7, P9).
    var chapterTitleAttributes: [NSAttributedString.Key: Any] {
        [.font: chapterTitleFont, .foregroundColor: NSColor.textColor,
         .paragraphStyle: paragraphStyle(lineFor: headingSize(level: 1), heading: true)]
    }

    // MARK: Building a font from the bundled files

    private static let wght = 0x77676874   // the 'wght' variation axis

    /// The weight range a face really has: a variable font's own `wght` axis, or its static files' weights.
    static func weightRange(_ face: BundledFonts.Face) -> ClosedRange<CGFloat> {
        if face.variable, let file = face.files.first, let d = descriptor(file, face) {
            let font = CTFontCreateWithFontDescriptor(d, 12, nil)
            for axis in (CTFontCopyVariationAxes(font) as? [[CFString: Any]]) ?? []
            where (axis[kCTFontVariationAxisIdentifierKey] as? Int) == wght {
                if let lo = axis[kCTFontVariationAxisMinimumValueKey] as? Double,
                   let hi = axis[kCTFontVariationAxisMaximumValueKey] as? Double { return CGFloat(lo)...CGFloat(hi) }
            }
            return 400...400
        }
        let ws = face.files.map { CGFloat($0.weight) }
        return (ws.min() ?? 400)...(ws.max() ?? 400)
    }

    private static func descriptor(_ file: BundledFonts.FontFile, _ face: BundledFonts.Face) -> CTFontDescriptor? {
        guard let url = BundledFonts.url(of: file, in: face) else { return nil }
        return (CTFontManagerCreateFontDescriptorsFromURL(url as CFURL) as? [CTFontDescriptor])?.first
    }

    /// The face at `weight` and `italic`. ✅ The italic is chosen BY FILE (the face's italic font), never by asking the font
    /// manager for an "italic trait": that resolves INSTALLED families, and these are bundled. A variable face takes the weight
    /// on its `wght` axis (clamped to the face's range); a static face takes the nearest-weight file.
    func font(size: CGFloat, weight: CGFloat, italic: Bool) -> NSFont {
        let key = "\(faceName)|\(size)|\(weight)|\(italic)"
        if let hit = FontCache.shared.get(key) { return hit }
        guard let face else { return .systemFont(ofSize: size) }
        let styled = face.files.filter { $0.italic == italic }
        let candidates = styled.isEmpty ? face.files : styled
        var made: NSFont?
        if face.variable, let file = candidates.first, let d = Self.descriptor(file, face) {
            let r = Self.weightRange(face)
            let v = CTFontDescriptorCreateCopyWithAttributes(
                d, [kCTFontVariationAttribute: [Self.wght: min(max(weight, r.lowerBound), r.upperBound)]] as CFDictionary)
            made = CTFontCreateWithFontDescriptor(v, size, nil) as NSFont
        } else if let best = candidates.min(by: { abs(CGFloat($0.weight) - weight) < abs(CGFloat($1.weight) - weight) }),
                  let d = Self.descriptor(best, face) {
            made = CTFontCreateWithFontDescriptor(d, size, nil) as NSFont
        }
        let f = made ?? .systemFont(ofSize: size)
        FontCache.shared.set(key, f)
        return f
    }
}

/// Fonts are built from bundled files and variation axes — not cheap — and the presenter asks for them per paragraph.
private final class FontCache: @unchecked Sendable {
    static let shared = FontCache()
    private let lock = NSLock()
    private var fonts: [String: NSFont] = [:]
    func get(_ key: String) -> NSFont? { lock.withLock { fonts[key] } }
    func set(_ key: String, _ font: NSFont) { lock.withLock { fonts[key] = font } }
}

#endif
