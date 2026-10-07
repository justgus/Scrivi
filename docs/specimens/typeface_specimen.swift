import AppKit
import CoreText

// Scrivi typeface specimen (EP-047) — rendered with Core Text, the engine the Mac app draws with, so what it shows is
// what Scrivi will show.
//
// Usage (from the repo root):
//   swiftc -O docs/specimens/typeface_specimen.swift -o /tmp/specimen
//   /tmp/specimen Resources/Fonts Resources/Fonts/fonts.json docs/specimens/Scrivi-Typeface-Specimen.pdf
// Font files are found by name anywhere under the fonts directory (one folder per family). The manifest lists the
// families, their category and note, and each file's style and weight (from Google Fonts' METADATA.pb).

let args = CommandLine.arguments
let dir = URL(fileURLWithPath: args[1])
let out = URL(fileURLWithPath: args[3])

struct FontFile: Decodable { let file: String; let italic: Bool; let weight: Int }
struct Face: Decodable {
    let name, category, folder, note: String
    let variable: Bool
    let files: [FontFile]
}
struct Manifest: Decodable { let faces: [Face] }
let faces = try! JSONDecoder().decode(Manifest.self, from: Data(contentsOf: URL(fileURLWithPath: args[2]))).faces

var fontFiles: [String: URL] = [:]
for case let u as URL in FileManager.default.enumerator(at: dir, includingPropertiesForKeys: nil)! where u.pathExtension == "ttf" {
    fontFiles[u.lastPathComponent] = u
}
func descriptor(_ file: String) -> CTFontDescriptor {
    guard let url = fontFiles[file] else { fatalError("font file not found under \(dir.path): \(file)") }
    return (CTFontManagerCreateFontDescriptorsFromURL(url as CFURL) as! [CTFontDescriptor])[0]
}
let wghtTag = 0x77676874   // 'wght'

// The weight range a face really has: a variable font's own `wght` axis, or its static files.
func weightRange(_ f: Face) -> ClosedRange<CGFloat> {
    if f.variable {
        let font = CTFontCreateWithFontDescriptor(descriptor(f.files[0].file), 12, nil)
        for axis in (CTFontCopyVariationAxes(font) as? [[CFString: Any]]) ?? [] {
            if (axis[kCTFontVariationAxisIdentifierKey] as? Int) == wghtTag {
                return CGFloat(axis[kCTFontVariationAxisMinimumValueKey] as! Double)...CGFloat(axis[kCTFontVariationAxisMaximumValueKey] as! Double)
            }
        }
        return 400...400
    }
    let ws = f.files.map { CGFloat($0.weight) }
    return ws.min()!...ws.max()!
}

func font(_ f: Face, size: CGFloat, weight: CGFloat, italic: Bool) -> NSFont {
    let candidates = f.files.filter { $0.italic == italic }.isEmpty ? f.files : f.files.filter { $0.italic == italic }
    if f.variable {
        let r = weightRange(f)
        let d = CTFontDescriptorCreateCopyWithAttributes(descriptor(candidates[0].file),
            [kCTFontVariationAttribute: [wghtTag: min(max(weight, r.lowerBound), r.upperBound)]] as CFDictionary)
        return CTFontCreateWithFontDescriptor(d, size, nil) as NSFont
    }
    let best = candidates.min { abs(CGFloat($0.weight) - weight) < abs(CGFloat($1.weight) - weight) }!
    return CTFontCreateWithFontDescriptor(descriptor(best.file), size, nil) as NSFont
}

// ⚠️ A FIXED line height (size × spacing) for every face: a line-height MULTIPLE inherits each font's own built-in
// leading, which made some faces look double-spaced and others tight — not a fair comparison.
func para(_ size: CGFloat, indent: CGFloat = 0, spacing: CGFloat = 1.45) -> NSParagraphStyle {
    let p = NSMutableParagraphStyle()
    p.firstLineHeadIndent = indent
    p.minimumLineHeight = size * spacing
    p.maximumLineHeight = size * spacing
    return p
}

// Runs: text with *italic* and **bold** markers, as Scrivi's presenter renders them (markers hidden). Bold is 700; bold
// inside a heading (base 700) goes to the face's heaviest weight — Scrivi's rule.
func runs(_ s: String, _ f: Face, size: CGFloat, base: CGFloat, indent: CGFloat = 0, spacing: CGFloat = 1.45) -> NSAttributedString {
    let p = para(size, indent: indent, spacing: spacing)
    let out = NSMutableAttributedString()
    var bold = false, italic = false
    var i = s.startIndex
    var buf = ""
    func flush() {
        guard !buf.isEmpty else { return }
        let w = bold ? (base >= 600 ? weightRange(f).upperBound : 700) : base
        out.append(NSAttributedString(string: buf, attributes: [.font: font(f, size: size, weight: w, italic: italic),
                                                                 .foregroundColor: ink, .paragraphStyle: p]))
        buf = ""
    }
    while i < s.endIndex {
        if s[i...].hasPrefix("**") { flush(); bold.toggle(); i = s.index(i, offsetBy: 2); continue }
        if s[i] == "*" { flush(); italic.toggle(); i = s.index(after: i); continue }
        buf.append(s[i]); i = s.index(after: i)
    }
    flush()
    return out
}

func folderSize(_ f: Face) -> String {
    let bytes = f.files.compactMap { fontFiles[$0.file] }
        .compactMap { try? $0.resourceValues(forKeys: [.fileSizeKey]).fileSize }.reduce(0, +)
    return String(format: "%.1f MB", Double(bytes) / 1_048_576)
}
func weightNote(_ f: Face) -> String {
    let top = Int(weightRange(f).upperBound)
    return top < 800 ? "heaviest weight \(top): bold inside a heading cannot be heavier than the heading" : "weights to \(top)"
}
func hasItalic(_ f: Face) -> Bool { f.files.contains { $0.italic } }

let ink = NSColor(calibratedWhite: 0.10, alpha: 1)
let gray = NSColor(calibratedWhite: 0.45, alpha: 1)
let paragraphs = [
    "The ship came in on the evening tide, her sails the colour of old parchment against a sky already turning to brass. On the quay a boy of perhaps twelve watched her the way other boys watch fireworks — with his whole body, and without blinking.",
    "“She is late,” said the harbour-master, though no one had asked him. “Three days late. *Three.*” He said it as if the number itself were an insult, and spat neatly into the water.",
    "Edmond had heard the rumours: a storm off Elba, a captain buried at sea, a letter that must **never** reach Paris. He believed none of them, and *all* of them — and he ran.",
]
let sample = "The ship came in on the evening tide, her sails the colour of old parchment… “*Three.*” **Never.**"
let glyphs = "ABCDEFGHIJKLMNOPQRSTUVWXYZ  abcdefghijklmnopqrstuvwxyz  0123456789  “quotes” ‘single’ — – … & ? ! ; :"

let page = CGRect(x: 0, y: 0, width: 612, height: 792)   // US Letter
var box = page
let ctx = CGContext(out as CFURL, mediaBox: &box, [kCGPDFContextTitle: "Scrivi — Manuscript Typeface Specimen"] as CFDictionary)!
NSGraphicsContext.current = NSGraphicsContext(cgContext: ctx, flipped: false)
let margin: CGFloat = 64
let width = page.width - 2 * margin
let bottom = page.height - 48

func height(_ s: NSAttributedString) -> CGFloat {
    ceil(s.boundingRect(with: NSSize(width: width, height: 10_000), options: [.usesLineFragmentOrigin, .usesFontLeading]).height)
}
// Draws top-down from `y` (distance from the page top); returns the new y.
@discardableResult
func draw(_ s: NSAttributedString, at y: CGFloat) -> CGFloat {
    let h = height(s)
    s.draw(with: NSRect(x: margin, y: page.height - y - h, width: width, height: h), options: [.usesLineFragmentOrigin, .usesFontLeading])
    return y + h
}
func label(_ t: String, size: CGFloat = 9, color: NSColor = gray, weight: NSFont.Weight = .regular) -> NSAttributedString {
    NSAttributedString(string: t, attributes: [.font: NSFont.systemFont(ofSize: size, weight: weight), .foregroundColor: color])
}

var pageNumber = 0
func beginPage() {
    ctx.beginPDFPage(nil)
    pageNumber += 1
    let n = label("\(pageNumber)", size: 8)
    n.draw(at: NSPoint(x: page.width - margin - 10, y: 28))
}

// ── Comparison pages, grouped by category ──
let categories = ["Serif", "Sans", "Monospaced", "Manuscript submission"]
beginPage()
var y: CGFloat = 60
y = draw(label("Scrivi — Manuscript Typeface Specimen", size: 20, color: ink, weight: .semibold), at: y) + 6
y = draw(label("EP-047 · the bundled set (\(faces.count) faces) · every one SIL Open Font License 1.1 (read from each font file) · rendered with Core Text, as Scrivi draws", size: 9.5), at: y) + 8
y = draw(label("Contents: comparison by category, then one manuscript page per face. Page numbers are at the bottom right.", size: 9.5), at: y) + 22
var facePage: [String: Int] = [:]
let firstFacePage: Int = {
    // Lay out the comparison once, invisibly, to know where the per-face pages begin.
    var p = 1, yy: CGFloat = y
    for c in categories {
        yy += 30
        for f in faces where f.category == c {
            let need = 14 + height(runs(sample, f, size: 14, base: 400)) + 16
            if yy + need > bottom { p += 1; yy = 60 }
            yy += need
        }
    }
    return p + 1
}()
for (k, f) in faces.enumerated() { facePage[f.name] = firstFacePage + k }
for c in categories {
    if y + 30 + 60 > bottom { ctx.endPDFPage(); beginPage(); y = 60 }
    y = draw(label(c.uppercased(), size: 11, color: ink, weight: .bold), at: y + 8) + 10
    for f in faces where f.category == c {
        let line = runs(sample, f, size: 14, base: 400)
        if y + 14 + height(line) + 16 > bottom { ctx.endPDFPage(); beginPage(); y = 60 }
        y = draw(label("\(f.name.uppercased())   ·   \(folderSize(f))   ·   page \(facePage[f.name]!)", size: 7.5, weight: .medium), at: y) + 3
        y = draw(line, at: y) + 16
    }
}
ctx.endPDFPage()

// ── One page per face ──
for f in faces {
    beginPage()
    var y: CGFloat = 52
    y = draw(label("\(f.category.uppercased())  ·  \(f.name)  —  \(f.note)", size: 9), at: y) + 2
    y = draw(label("\(folderSize(f))  ·  \(weightNote(f))\(hasItalic(f) ? "" : "  ·  no italic")", size: 8), at: y) + 24
    let body: CGFloat = 15
    y = draw(runs("Chapter One", f, size: body * 22 / 13, base: 700, spacing: 1.25), at: y) + 14
    y = draw(runs("The Harbour at **Marseille**", f, size: body * 18 / 13, base: 700, spacing: 1.25), at: y) + 12
    for (k, p) in paragraphs.enumerated() {
        y = draw(runs(p, f, size: body, base: 400, indent: k == 0 ? 0 : body * 1.6), at: y) + 2
    }
    y += 16
    y = draw(runs("A Smaller Heading, With *Italic* and **Bold**", f, size: body * 16 / 13, base: 700, spacing: 1.25), at: y) + 10
    y = draw(runs(paragraphs[0], f, size: 12, base: 400), at: y) + 4
    y = draw(label("the same paragraph at 12 pt", size: 8), at: y) + 14
    y = draw(runs(glyphs, f, size: 11.5, base: 400, spacing: 1.4), at: y) + 4
    y = draw(runs("*" + glyphs + "*", f, size: 11.5, base: 400, spacing: 1.4), at: y) + 4
    y = draw(runs("**" + glyphs + "**", f, size: 11.5, base: 400, spacing: 1.4), at: y)
    if y > bottom { print("⚠️ \(f.name): overflows (ends at \(Int(y)) of \(Int(bottom)))") }
    ctx.endPDFPage()
}
ctx.closePDF()
print("wrote \(out.path): \(pageNumber) pages, \(faces.count) faces")
