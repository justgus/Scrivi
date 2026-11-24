#!/usr/bin/env swift
// make-glyph-symbols.swift — EP-060 / SP-241 (T-0704).
//
// Turns the Inkwyn glyph art in Resources/Icons into custom
// SF Symbols: Scrivi/Assets.xcassets/Glyphs/inkwyn.<name>.symbolset, with Small/Medium/Large as the symbol's three
// scales. A glyph comes either as three files, <Name>-Small/-Medium/-Large.svg, or as ONE <Name>.svg holding three groups
// <g id="large|medium|small"> each in its own tile named by <view id="large-view|…" viewBox> (the Runic Whimsy set). Strokes are outlined and white shapes knocked out (a symbol is filled outlines only), and every drawing is
// fitted into one box so the glyphs read as the same size (EP-060 Q5). The output is committed; re-run after the art
// changes. Running it twice writes identical files.
//
//   swift scripts/make-glyph-symbols.swift                     # all glyphs
//   swift scripts/make-glyph-symbols.swift --only Characters   # one glyph
//   swift scripts/make-glyph-symbols.swift --sheet out.png     # also draw a contact sheet of every glyph at S, M, L
//
// Supported SVG: <svg viewBox> (nested too; the last one crops), <g>, <path d> (M L H V Q C Z, absolute or relative), <circle>, <rect rx ry>;
// fill / stroke / stroke-width / stroke-linecap / stroke-linejoin / fill-rule, inherited through <g>;
// transform translate / rotate / scale / matrix. Anything else stops the script with the file named.

import CoreGraphics
import CoreText
import Foundation
import ImageIO
import UniformTypeIdentifiers

struct Failure: Error, CustomStringConvertible { let description: String }

// MARK: - SVG → one filled CGPath

struct Paint {
    var fill: Bool? = true        // SVG default: filled black. nil = white (knock out), false = none.
    var stroke: Bool? = false
    var width: CGFloat = 1
    var cap: CGLineCap = .butt
    var join: CGLineJoin = .miter
    var miter: CGFloat = 4
    var evenOdd = false
    var transform = CGAffineTransform.identity
}

func ink(_ v: String) -> Bool? {
    switch v.lowercased() {
    case "none", "transparent": return false
    case "white", "#fff", "#ffffff": return nil
    default: return true
    }
}

func numbers(_ s: String) -> [CGFloat] {
    let re = try! NSRegularExpression(pattern: #"[-+]?(?:\d+\.?\d*|\.\d+)(?:[eE][-+]?\d+)?"#)
    return re.matches(in: s, range: NSRange(s.startIndex..., in: s)).map { CGFloat(Double(s[Range($0.range, in: s)!])!) }
}

func parseTransform(_ s: String, file: String) throws -> CGAffineTransform {
    var t = CGAffineTransform.identity
    let re = try! NSRegularExpression(pattern: #"(\w+)\s*\(([^)]*)\)"#)
    for m in re.matches(in: s, range: NSRange(s.startIndex..., in: s)) {
        let name = String(s[Range(m.range(at: 1), in: s)!]), a = numbers(String(s[Range(m.range(at: 2), in: s)!]))
        let step: CGAffineTransform
        switch (name, a.count) {
        case ("translate", 1): step = .init(translationX: a[0], y: 0)
        case ("translate", 2): step = .init(translationX: a[0], y: a[1])
        case ("scale", 1): step = .init(scaleX: a[0], y: a[0])
        case ("scale", 2): step = .init(scaleX: a[0], y: a[1])
        case ("rotate", 1): step = .init(rotationAngle: a[0] * .pi / 180)
        case ("rotate", 3):
            step = CGAffineTransform(translationX: -a[1], y: -a[2])
                .concatenating(.init(rotationAngle: a[0] * .pi / 180))
                .concatenating(.init(translationX: a[1], y: a[2]))
        case ("matrix", 6): step = .init(a: a[0], b: a[1], c: a[2], d: a[3], tx: a[4], ty: a[5])
        default: throw Failure(description: "\(file): unsupported transform \(name)(\(a.count) args)")
        }
        t = step.concatenating(t)   // SVG lists outer-first; each later step applies to the shape first
    }
    return t
}

func parsePathData(_ d: String, file: String) throws -> CGMutablePath {
    let p = CGMutablePath()
    let re = try! NSRegularExpression(pattern: #"[A-Za-z]|[-+]?(?:\d+\.?\d*|\.\d+)(?:[eE][-+]?\d+)?"#)
    let tokens = re.matches(in: d, range: NSRange(d.startIndex..., in: d)).map { String(d[Range($0.range, in: d)!]) }
    var i = 0, cmd: Character = " ", cur = CGPoint.zero, start = CGPoint.zero
    func num() throws -> CGFloat {
        guard i < tokens.count, let v = Double(tokens[i]) else { throw Failure(description: "\(file): bad path data near token \(i)") }
        i += 1; return CGFloat(v)
    }
    func pt(_ rel: Bool) throws -> CGPoint {
        let x = try num(), y = try num()
        return rel ? CGPoint(x: cur.x + x, y: cur.y + y) : CGPoint(x: x, y: y)
    }
    while i < tokens.count {
        if let c = tokens[i].first, c.isLetter { cmd = c; i += 1 }
        let rel = cmd.isLowercase
        switch cmd.uppercased() {
        case "M":
            cur = try pt(rel); start = cur; p.move(to: cur)
            cmd = rel ? "l" : "L"   // further pairs are line-tos
        case "L": cur = try pt(rel); p.addLine(to: cur)
        case "H": let x = try num(); cur.x = rel ? cur.x + x : x; p.addLine(to: cur)
        case "V": let y = try num(); cur.y = rel ? cur.y + y : y; p.addLine(to: cur)
        case "Q": let c = try pt(rel); cur = try pt(rel); p.addQuadCurve(to: cur, control: c)
        case "C": let c1 = try pt(rel), c2 = try pt(rel); cur = try pt(rel); p.addCurve(to: cur, control1: c1, control2: c2)
        case "Z": p.closeSubpath(); cur = start
            if i < tokens.count, Double(tokens[i]) != nil { throw Failure(description: "\(file): number after Z") }
        default: throw Failure(description: "\(file): unsupported path command '\(cmd)'")
        }
    }
    return p
}

final class SVGReader: NSObject, XMLParserDelegate {
    let file: String
    let group: String?          // sprite file: paint only inside <g id=group>, crop to <view id="group-view">
    let dropPips: Bool
    var viewBox = CGRect.zero
    var depthInGroup = 0
    var stack = [Paint()]
    var result: CGPath = CGMutablePath()
    var error: Error?

    init(file: String, group: String? = nil, dropPips: Bool = false) { self.file = file; self.group = group; self.dropPips = dropPips }

    func parser(_ parser: XMLParser, didStartElement name: String, namespaceURI: String?, qualifiedName: String?,
                attributes a: [String: String]) {
        do {
            var paint = stack.last!
            if let v = a["fill"] { paint.fill = ink(v) }
            if let v = a["stroke"] { paint.stroke = ink(v) }
            if let v = a["stroke-width"], let w = Double(v) { paint.width = CGFloat(w) }
            if let v = a["stroke-linecap"] { paint.cap = v == "round" ? .round : v == "square" ? .square : .butt }
            if let v = a["stroke-linejoin"] { paint.join = v == "round" ? .round : v == "bevel" ? .bevel : .miter }
            if let v = a["stroke-miterlimit"], let m = Double(v) { paint.miter = CGFloat(m) }
            if let v = a["fill-rule"] { paint.evenOdd = v == "evenodd" }
            if let v = a["transform"] { paint.transform = try parseTransform(v, file: file).concatenating(paint.transform) }
            if a["style"] != nil || a["opacity"] != nil || a["clip-path"] != nil || a["mask"] != nil {
                throw Failure(description: "\(file): <\(name)> uses style/opacity/clip/mask, which this script does not read")
            }
            stack.append(paint)
            if depthInGroup > 0 || (name == "g" && group != nil && a["id"] == group) { depthInGroup += 1 }

            let f = { (k: String) in CGFloat(Double(a[k] ?? "0") ?? 0) }
            var shape: CGPath?
            switch name {
            case "svg":
                let v = numbers(a["viewBox"] ?? "")
                guard v.count == 4 else { throw Failure(description: "\(file): no viewBox") }
                if group == nil { viewBox = CGRect(x: v[0], y: v[1], width: v[2], height: v[3]) }
            case "g", "title", "desc": break
            case "view":
                let v = numbers(a["viewBox"] ?? "")
                if let group, a["id"] == "\(group)-view", v.count == 4 { viewBox = CGRect(x: v[0], y: v[1], width: v[2], height: v[3]) }
            case "path": shape = try parsePathData(a["d"] ?? "", file: file)
            case "circle": let r = f("r"); shape = CGPath(ellipseIn: CGRect(x: f("cx") - r, y: f("cy") - r, width: 2 * r, height: 2 * r), transform: nil)
            case "rect":
                let r = CGRect(x: f("x"), y: f("y"), width: f("width"), height: f("height"))
                var rx = a["rx"].map { _ in f("rx") } ?? a["ry"].map { _ in f("ry") } ?? 0
                var ry = a["ry"].map { _ in f("ry") } ?? rx
                rx = min(rx, r.width / 2); ry = min(ry, r.height / 2)
                shape = rx > 0 ? CGPath(roundedRect: r, cornerWidth: rx, cornerHeight: ry, transform: nil) : CGPath(rect: r, transform: nil)
            default: throw Failure(description: "\(file): unsupported element <\(name)>")
            }
            // A pip is a small diamond at the tile's right edge (x ≥ 105 of 128), drawn in the group's own coordinates — its own
            // <path> (Worlds) or a subpath inside the glyph's main path (Close).
            if var shape, group == nil || depthInGroup > 0 {
                if dropPips { shape = withoutPipSubpaths(shape) }
                if !shape.isEmpty { paintShape(shape, paint) }
            }
        } catch {
            self.error = error; parser.abortParsing()
        }
    }

    func parser(_ parser: XMLParser, didEndElement name: String, namespaceURI: String?, qualifiedName: String?) {
        stack.removeLast()
        if depthInGroup > 0 { depthInGroup -= 1 }
    }

    // Painter's order: ink is added, white is cut out of what is already there.
    func paintShape(_ shape: CGPath, _ paint: Paint) {
        var t = paint.transform
        let placed = shape.copy(using: &t)!
        func apply(_ piece: CGPath, _ on: Bool?) {
            guard on != false else { return }
            result = on == true ? result.union(piece, using: .winding) : result.subtracting(piece, using: .winding)
        }
        apply(placed.normalized(using: paint.evenOdd ? .evenOdd : .winding), paint.fill)
        if paint.stroke != false, paint.width > 0 {
            // Stroke in the shape's own space, then place it, so a scale in the transform scales the line too.
            let outline = shape.copy(strokingWithWidth: paint.width, lineCap: paint.cap, lineJoin: paint.join, miterLimit: paint.miter)
            apply(outline.copy(using: &t)!.normalized(using: .winding), paint.stroke)
        }
    }
}

func isPip(_ box: CGRect) -> Bool { box.minX >= 105 && box.width < 12 && box.height < 12 }

func subpaths(of path: CGPath) -> [CGMutablePath] {
    var subpaths = [CGMutablePath]()
    path.applyWithBlock { e in
        let p = e.pointee.points
        switch e.pointee.type {
        case .moveToPoint: let m = CGMutablePath(); m.move(to: p[0]); subpaths.append(m)
        case .addLineToPoint: subpaths.last?.addLine(to: p[0])
        case .addQuadCurveToPoint: subpaths.last?.addQuadCurve(to: p[1], control: p[0])
        case .addCurveToPoint: subpaths.last?.addCurve(to: p[2], control1: p[0], control2: p[1])
        case .closeSubpath: subpaths.last?.closeSubpath()
        @unknown default: break
        }
    }
    return subpaths
}

/// The path without the subpaths that are pips.
func withoutPipSubpaths(_ path: CGPath) -> CGPath {
    let kept = CGMutablePath()
    for sp in subpaths(of: path) where !isPip(sp.boundingBoxOfPath) { kept.addPath(sp) }
    return kept
}

/// The drawing without stray specks: a tiny piece — under 0.15% of the canvas — that is ISOLATED, i.e. near no other piece (within
/// 2% of the canvas). Merge-Large's and Import-Export-Large's slivers sit alone in empty space; a pip is ~0.5%; Item's handle diamond
/// and Vehicles' edge knots are as small but touch neighbouring ink, so they stay. A speck widens the drawing, so aspect fit shrank
/// the whole glyph (user, 2026-11-24: "Have the script ignore the stray specks").
func withoutSpecks(_ path: CGPath, canvas: CGRect) -> CGPath {
    let pieces = subpaths(of: path), boxes = pieces.map(\.boundingBoxOfPath)
    let near = 0.02 * max(canvas.width, canvas.height)
    let kept = CGMutablePath()
    for (i, sp) in pieces.enumerated() {
        let b = boxes[i]
        let tiny = b.width * b.height < 0.0015 * canvas.width * canvas.height
        let alone = !boxes.indices.contains { $0 != i && boxes[$0].intersects(b.insetBy(dx: -near, dy: -near)) }
        if !(tiny && alone) { kept.addPath(sp) }
    }
    return kept
}

func readGlyph(_ url: URL, group: String? = nil, dropPips: Bool = false) throws -> CGPath {
    let reader = SVGReader(file: url.lastPathComponent + (group.map { " <g id=\($0)>" } ?? ""), group: group, dropPips: dropPips)
    let parser = XMLParser(data: try Data(contentsOf: url))
    parser.delegate = reader
    if !parser.parse() { throw reader.error ?? Failure(description: "\(url.lastPathComponent): \(parser.parserError?.localizedDescription ?? "unreadable")") }
    // An <svg> shows only what lies inside its viewBox (Item-Small nests one whose viewBox hides a leftover shape).
    let visible = withoutSpecks(reader.result.intersection(CGPath(rect: reader.viewBox, transform: nil), using: .winding),
                                canvas: reader.viewBox)
    if reader.viewBox.isEmpty { throw Failure(description: "\(reader.file): no viewBox\(group.map { " (no <view id=\"\($0)-view\">)" } ?? "")") }
    if visible.boundingBoxOfPath.isEmpty { throw Failure(description: "\(reader.file): draws nothing") }
    return visible
}

// MARK: - Fitting into the symbol template

// Template geometry, taken from the template SF Symbols.app ships (point size 100): baseline → cap height is 70.459.
let capHeight: CGFloat = 70.459
// Medium: the ink fits a box this tall, or this wide for wide glyphs, centred on the cap height. 1.5× Apple's kind symbols at
// 100 pt (building.2 96×113, mappin.and.ellipse 92×106 ink): 1× read too small (SP-241), 2× a bit oversized (user, SP-242).
let boxHeight: CGFloat = 159, boxWidth: CGFloat = 210
// Small and Large relative to Medium, measured from system symbols at 100 pt (circle.fill 93/117/151, person.fill 85/108/138).
let scales: [(id: String, factor: CGFloat, baseline: CGFloat)] = [("S", 0.79, 696), ("M", 1.0, 1126), ("L", 1.29, 1556)]
let regularX: CGFloat = 1394.4

func fitted(_ path: CGPath, factor: CGFloat) -> CGPath {
    let b = path.boundingBoxOfPath
    let s = min(boxHeight / b.height, boxWidth / b.width) * factor
    var t = CGAffineTransform(translationX: -b.minX, y: -b.midY)
        .concatenating(.init(scaleX: s, y: s))
        .concatenating(.init(translationX: 0, y: -capHeight / 2))
    return path.copy(using: &t)!
}

func svgPathData(_ path: CGPath) -> String {
    func n(_ v: CGFloat) -> String {
        var s = String(format: "%.3f", Double(v))
        while s.hasSuffix("0") { s.removeLast() }
        if s.hasSuffix(".") { s.removeLast() }
        return s == "-0" ? "0" : s
    }
    func p(_ q: CGPoint) -> String { "\(n(q.x)) \(n(q.y))" }
    var out = [String]()
    path.applyWithBlock { e in
        let pts = e.pointee.points
        switch e.pointee.type {
        case .moveToPoint: out.append("M\(p(pts[0]))")
        case .addLineToPoint: out.append("L\(p(pts[0]))")
        case .addQuadCurveToPoint: out.append("Q\(p(pts[0])) \(p(pts[1]))")
        case .addCurveToPoint: out.append("C\(p(pts[0])) \(p(pts[1])) \(p(pts[2]))")
        case .closeSubpath: out.append("Z")
        @unknown default: break
        }
    }
    return out.joined()
}

func symbolTemplate(_ drawings: [String: CGPath]) -> String {
    var guides = "", symbols = ""
    for sc in scales {
        let path = drawings[sc.id]!, w = path.boundingBoxOfPath.maxX
        let top = sc.baseline - capHeight
        guides += """
          <line id="Baseline-\(sc.id)" style="fill:none;stroke:#27AAE1;opacity:1;stroke-width:0.5;" x1="263" x2="3036" y1="\(svgNumber(sc.baseline))" y2="\(svgNumber(sc.baseline))"/>
          <line id="Capline-\(sc.id)" style="fill:none;stroke:#27AAE1;opacity:1;stroke-width:0.5;" x1="263" x2="3036" y1="\(svgNumber(top))" y2="\(svgNumber(top))"/>
          <line id="left-margin-Regular-\(sc.id)" style="fill:none;stroke:#00AEEF;stroke-width:0.5;opacity:1.0;" x1="\(svgNumber(regularX))" x2="\(svgNumber(regularX))" y1="\(svgNumber(top - 25))" y2="\(svgNumber(sc.baseline + 25))"/>
          <line id="right-margin-Regular-\(sc.id)" style="fill:none;stroke:#00AEEF;stroke-width:0.5;opacity:1.0;" x1="\(svgNumber(regularX + w))" x2="\(svgNumber(regularX + w))" y1="\(svgNumber(top - 25))" y2="\(svgNumber(sc.baseline + 25))"/>

        """
        symbols += """
          <g id="Regular-\(sc.id)" transform="matrix(1 0 0 1 \(svgNumber(regularX)) \(svgNumber(sc.baseline)))">
           <path class="monochrome-0 multicolor-0:tintColor hierarchical-0:primary" d="\(svgPathData(path))"/>
          </g>

        """
    }
    return """
    <?xml version="1.0" encoding="UTF-8"?>
    <!--Generated by scripts/make-glyph-symbols.swift from Resources/Icons. Do not edit; re-run the script.-->
    <!DOCTYPE svg
    PUBLIC "-//W3C//DTD SVG 1.1//EN"
           "http://www.w3.org/Graphics/SVG/1.1/DTD/svg11.dtd">
    <svg version="1.1" xmlns="http://www.w3.org/2000/svg" xmlns:xlink="http://www.w3.org/1999/xlink" width="3300" height="2200">
     <!--glyph: "", point size: 100.0, template writer version: "101"-->
     <style>.monochrome-0 {fill:#000000}

    .multicolor-0:tintColor {fill:#007AFF}

    .hierarchical-0:primary {fill:#212121}
    </style>
     <g id="Notes">
      <rect height="2200" id="artboard" style="fill:white;opacity:1" width="3300" x="0" y="0"/>
     </g>
     <g id="Guides">
    \(guides) </g>
     <g id="Symbols">
    \(symbols) </g>
    </svg>

    """
}

func svgNumber(_ v: CGFloat) -> String {
    var s = String(format: "%.3f", Double(v))
    while s.hasSuffix("0") { s.removeLast() }
    if s.hasSuffix(".") { s.removeLast() }
    return s
}

/// A drawing as a plain SVG: the path moved to the origin, the viewBox its own box.
func plainSVG(_ path: CGPath) -> String {
    let b = path.boundingBoxOfPath
    var t = CGAffineTransform(translationX: -b.minX, y: -b.minY)
    let moved = path.copy(using: &t)!
    return """
    <svg xmlns="http://www.w3.org/2000/svg" viewBox="0 0 \(svgNumber(b.width)) \(svgNumber(b.height))"><path fill="#000000" d="\(svgPathData(moved))"/></svg>

    """
}

// MARK: - Contact sheet

func writeSheet(_ glyphs: [(name: String, drawings: [String: CGPath])], to url: URL) throws {
    // A grid of glyphs, each block = the glyph's name over its S, M and L drawings on baseline + cap-height guides.
    let cell: CGFloat = 60, label: CGFloat = 14, scale: CGFloat = 34 / boxHeight   // Medium ink ≈ 34 px, Large ≈ 44 px
    let blockW = cell * 3 + 20, blockH = cell + label, perColumn = 12
    let columns = (glyphs.count + perColumn - 1) / perColumn
    let w = Int(blockW * CGFloat(columns)), h = Int(blockH * CGFloat(min(glyphs.count, perColumn)))
    let ctx = CGContext(data: nil, width: w * 2, height: h * 2, bitsPerComponent: 8, bytesPerRow: 0,
                        space: CGColorSpaceCreateDeviceRGB(), bitmapInfo: CGImageAlphaInfo.premultipliedLast.rawValue)!
    ctx.scaleBy(x: 2, y: 2)
    ctx.setFillColor(CGColor(gray: 1, alpha: 1)); ctx.fill(CGRect(x: 0, y: 0, width: w, height: h))
    for (i, g) in glyphs.enumerated() {
        let left = CGFloat(i / perColumn) * blockW, top = CGFloat(h) - CGFloat(i % perColumn) * blockH   // CG: y up
        let text = CTLineCreateWithAttributedString(NSAttributedString(string: g.name, attributes: [
            kCTFontAttributeName as NSAttributedString.Key: CTFontCreateWithName("Helvetica" as CFString, 10, nil)]))
        ctx.textPosition = CGPoint(x: left + 6, y: top - 11); CTLineDraw(text, ctx)
        let baseline = top - blockH + 12
        for (col, sc) in scales.enumerated() {
            let ox = left + 6 + CGFloat(col) * cell
            ctx.setStrokeColor(CGColor(red: 0.15, green: 0.67, blue: 0.88, alpha: 1)); ctx.setLineWidth(0.5)
            for y in [baseline, baseline + capHeight * scale] {
                ctx.move(to: CGPoint(x: ox, y: y)); ctx.addLine(to: CGPoint(x: ox + cell - 8, y: y))
            }
            ctx.strokePath()
            var t = CGAffineTransform(scaleX: scale, y: -scale).concatenating(.init(translationX: ox, y: baseline))
            ctx.addPath(g.drawings[sc.id]!.copy(using: &t)!)
            ctx.setFillColor(CGColor(gray: 0, alpha: 1)); ctx.fillPath()
        }
    }
    let dest = CGImageDestinationCreateWithURL(url as CFURL, UTType.png.identifier as CFString, 1, nil)!
    CGImageDestinationAddImage(dest, ctx.makeImage()!, nil)
    guard CGImageDestinationFinalize(dest) else { throw Failure(description: "could not write \(url.path)") }
    print("contact sheet: \(url.path)")
}

// MARK: - Self-check

func selfCheck() throws {
    // A 40×20 filled rect, a 10-wide stroked line below it, a white square knocked out of the rect, and a lone 1×1 speck at
    // (90, 2) that must be dropped — the box would reach x = 91 if it were kept.
    let svg = """
    <svg xmlns="http://www.w3.org/2000/svg" viewBox="0 0 100 100">
      <rect x="10" y="10" width="40" height="20"/>
      <rect x="20" y="15" width="10" height="10" fill="white"/>
      <g transform="translate(0 50)"><path d="M10 0 H50" fill="none" stroke="black" stroke-width="10"/></g>
      <rect x="90" y="2" width="1" height="1"/>
    </svg>
    """
    let url = FileManager.default.temporaryDirectory.appendingPathComponent("glyph-selfcheck.svg")
    try svg.write(to: url, atomically: true, encoding: .utf8)
    let p = try readGlyph(url)
    let b = p.boundingBoxOfPath
    func near(_ a: CGFloat, _ e: CGFloat) -> Bool { abs(a - e) < 0.01 }
    guard near(b.minX, 10), near(b.minY, 10), near(b.maxX, 50), near(b.maxY, 55) else { throw Failure(description: "self-check: bbox \(b)") }
    guard !p.contains(CGPoint(x: 25, y: 20)), p.contains(CGPoint(x: 15, y: 20)), p.contains(CGPoint(x: 30, y: 52)) else {
        throw Failure(description: "self-check: knock-out or stroke outline wrong")
    }
    let f = fitted(p, factor: 1).boundingBoxOfPath
    guard near(f.minX, 0), near(f.midY, -capHeight / 2), near(f.height, boxHeight) || near(f.width, boxWidth) else {
        throw Failure(description: "self-check: fit \(f)")
    }
}

// MARK: - Main

do {
    try selfCheck()
    let repo = URL(fileURLWithPath: #filePath).deletingLastPathComponent().deletingLastPathComponent()
    let source = repo.appendingPathComponent("Resources/Icons")
    let catalog = repo.appendingPathComponent("Scrivi/Assets.xcassets/Glyphs")
    let args = CommandLine.arguments
    let only = args.firstIndex(of: "--only").map { args[$0 + 1] }
    let sheet = args.firstIndex(of: "--sheet").map { URL(fileURLWithPath: args[$0 + 1]) }

    // A set's glyphs get this prefix after "inkwyn." — the Runic Whimsy set is the toolbar's menu buttons, and its Close
    // is different art from the Utility set's Close.
    let setPrefix = ["Inkwyn_Runic_Whimsy_Glyphs": "Menu-"]
    // The diamond pips mean "this button opens a menu". Worlds opens a sheet (user, 2026-11-24: "drop the pips for worlds") and
    // Close Project closes the window, so both are drawn without them.
    let withoutPips: Set<String> = ["Menu-Worlds", "Menu-Close"]

    // Anywhere under Resources/Icons → [Name: [Size: [(file, sprite group)]]]
    var found = [String: [String: [(url: URL, group: String?)]]]()
    let walker = FileManager.default.enumerator(at: source, includingPropertiesForKeys: nil)!
    for case let url as URL in walker where url.pathExtension == "svg" {
        let base = url.deletingPathExtension().lastPathComponent
        let set = url.pathComponents.first { setPrefix[$0] != nil }.flatMap { setPrefix[$0] } ?? ""
        if let dash = base.lastIndex(of: "-"), let size = ["Small": "S", "Medium": "M", "Large": "L"][String(base[base.index(after: dash)...])] {
            found[set + String(base[..<dash]), default: [:]][size, default: []].append((url, nil))
        } else if try String(contentsOf: url, encoding: .utf8).contains(#"<g id="medium""#) {
            for (size, group) in [("S", "small"), ("M", "medium"), ("L", "large")] { found[set + base, default: [:]][size, default: []].append((url, group)) }
        } else {
            throw Failure(description: "\(url.path): neither <Name>-Small|Medium|Large.svg nor a file with <g id=\"small|medium|large\">")
        }
    }

    var glyphs = [(name: String, drawings: [String: CGPath])]()
    for name in found.keys.sorted() where only == nil || only == name {
        var drawings = [String: CGPath]()
        for sc in scales {
            guard let files = found[name]?[sc.id], let first = files.first else { throw Failure(description: "\(name): no \(sc.id) drawing") }
            // The same glyph may sit in two sets (Timeline is in View and in Import/Export); it must be the same art.
            for other in files.dropFirst() where try Data(contentsOf: other.url) != Data(contentsOf: first.url) || other.group != first.group {
                throw Failure(description: "\(name): two different drawings named the same — \(first.url.path) and \(other.url.path)")
            }
            drawings[sc.id] = fitted(try readGlyph(first.url, group: first.group, dropPips: withoutPips.contains(name)), factor: sc.factor)
        }
        glyphs.append((name, drawings))
    }
    if glyphs.isEmpty { throw Failure(description: "no glyph matched") }

    try FileManager.default.createDirectory(at: catalog, withIntermediateDirectories: true)
    try #"{"info":{"author":"xcode","version":1}}"#.appending("\n").write(to: catalog.appendingPathComponent("Contents.json"), atomically: true, encoding: .utf8)
    for g in glyphs {
        let symbol = "inkwyn." + g.name.lowercased().replacingOccurrences(of: "-", with: ".")
        let dir = catalog.appendingPathComponent("\(symbol).symbolset")
        try FileManager.default.createDirectory(at: dir, withIntermediateDirectories: true)
        try symbolTemplate(g.drawings).write(to: dir.appendingPathComponent("\(symbol).svg"), atomically: true, encoding: .utf8)
        let contents = #"{"info":{"author":"xcode","version":1},"symbols":[{"filename":"\#(symbol).svg","idiom":"universal"}]}"#
        try (contents + "\n").write(to: dir.appendingPathComponent("Contents.json"), atomically: true, encoding: .utf8)
        print(symbol)
    }
    // Linux (EP-060 SP-243 Q1): the same cleaned drawings as plain SVGs — one filled black path per size, its viewBox the
    // drawing's own box — for platforms/linux's GlyphIcon, which aspect-fits and tints them. One pipeline for both platforms.
    let linux = repo.appendingPathComponent("platforms/linux/resources/glyphs")
    if only == nil, FileManager.default.fileExists(atPath: linux.path) { try FileManager.default.removeItem(at: linux) }
    try FileManager.default.createDirectory(at: linux, withIntermediateDirectories: true)
    for g in glyphs {
        let symbol = "inkwyn." + g.name.lowercased().replacingOccurrences(of: "-", with: ".")
        for (id, size) in [("S", "small"), ("M", "medium"), ("L", "large")] {
            try plainSVG(g.drawings[id]!).write(to: linux.appendingPathComponent("\(symbol)-\(size).svg"), atomically: true, encoding: .utf8)
        }
    }
    if let sheet { try writeSheet(glyphs, to: sheet) }
    print("\(glyphs.count) symbol(s) written to \(catalog.path)")
} catch {
    FileHandle.standardError.write("make-glyph-symbols: \(error)\n".data(using: .utf8)!)
    exit(1)
}
