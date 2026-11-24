import CoreText
import Foundation
import WebKit
#if os(macOS)
import AppKit
import PDFKit
#else
import UIKit
#endif

// EP-054 S5b (SP-214, T-0675) — the core's print page (`document: "print"`) paginated by WebKit into a PDF.
//
// Rulings: Q1 — page numbers are ours, never WebKit's header and footer (it adds a date and title): the Mac stamps them onto
// the finished pages with PDFKit, the iPad draws them in `UIPrintPageRenderer`'s footer. Q2 — WebKit lays out in its own
// process, which cannot see the faces the app registered, so a bundled face is handed to it as `@font-face` rules on the
// bundle's font files. Q3 — the system's paper (the Mac's print settings; on the iPad, Letter where the region measures in
// US units, A4 elsewhere), 1-inch margins. Q4 — the first chapter page is page 1; the title page shows no number.
@MainActor
enum ManuscriptPDF {

    nonisolated static let margin: CGFloat = 72

    struct Failed: LocalizedError { let errorDescription: String? }

    /// Writes the PDF of `html` to `url`. `titlePage`: the first page is the title page (unnumbered, uncounted).
    static func write(html: String, typeface: String, titlePage: Bool, to url: URL) async throws {
        let view = WKWebView(frame: CGRect(origin: .zero, size: paper))
        let loaded = Loaded()
        view.navigationDelegate = loaded
        view.loadHTMLString(withFaces(html, typeface), baseURL: BundledFonts.directory)
        try await loaded.wait()
        _ = try await view.callAsyncJavaScript("await document.fonts.ready; return 1", contentWorld: .page)
        let numberFont = CTFontCreateWithName((BundledFonts.face(named: typeface)?.name ?? typeface) as CFString, 10, nil)
        #if os(macOS)
        try await printMac(view, titlePage: titlePage, font: numberFont, to: url)
        #else
        try printPhone(view, titlePage: titlePage, font: numberFont, to: url)
        #endif
    }

    /// Q3: the system's default paper.
    static var paper: CGSize {
        #if os(macOS)
        NSPrintInfo.shared.paperSize
        #else
        Locale.current.measurementSystem == .us ? CGSize(width: 612, height: 792) : CGSize(width: 595.28, height: 841.89)
        #endif
    }

    /// Q4: the number shown on page `index` (0-based), or nil on the title page.
    nonisolated static func number(_ index: Int, titlePage: Bool) -> String? {
        titlePage ? (index == 0 ? nil : String(index)) : String(index + 1)
    }

    /// Q2: `@font-face` rules for a bundled face, its files named relative to the bundle's `Fonts` folder (the base URL).
    /// A system face needs none: WebKit's process sees the system's fonts.
    static func withFaces(_ html: String, _ typeface: String) -> String {
        guard let face = BundledFonts.faces.first(where: { $0.name == typeface }) else { return html }
        let rules = face.files.map { file in
            let path = "\(face.folder)/\(file.file)".addingPercentEncoding(withAllowedCharacters: .urlPathAllowed
                .subtracting(CharacterSet(charactersIn: "[]"))) ?? ""
            return "@font-face { font-family: \"\(face.name)\"; src: url(\"\(path)\"); font-style: "
                + (file.italic ? "italic" : "normal") + "; font-weight: " + (face.variable ? "100 900" : "\(file.weight)") + "; }\n"
        }.joined()
        return html.replacingOccurrences(of: "<style>\n", with: "<style>\n" + rules)
    }

    /// The page's load, awaited.
    private final class Loaded: NSObject, WKNavigationDelegate {
        private var done: CheckedContinuation<Void, Error>?
        private var result: Result<Void, Error>?

        func wait() async throws {
            if let result { return try result.get() }
            try await withCheckedThrowingContinuation { done = $0 }
        }
        private func finish(_ r: Result<Void, Error>) {
            if let done { done.resume(with: r); self.done = nil } else { result = r }
        }
        func webView(_ webView: WKWebView, didFinish navigation: WKNavigation!) { finish(.success(())) }
        func webView(_ webView: WKWebView, didFail navigation: WKNavigation!, withError error: Error) { finish(.failure(error)) }
        func webView(_ webView: WKWebView, didFailProvisionalNavigation navigation: WKNavigation!, withError error: Error) {
            finish(.failure(error))
        }
    }

    #if os(macOS)
    /// WebKit's printing, saved to a file beside `url` with no header and footer; then each number stamped on (Q1).
    private static func printMac(_ view: WKWebView, titlePage: Bool, font: CTFont, to url: URL) async throws {
        let pages = url.deletingLastPathComponent().appendingPathComponent(".\(UUID().uuidString).pdf")
        defer { try? FileManager.default.removeItem(at: pages) }
        let info = NSPrintInfo(dictionary: [.jobSavingURL: pages])
        info.paperSize = paper
        info.jobDisposition = .save
        info.topMargin = margin; info.bottomMargin = margin; info.leftMargin = margin; info.rightMargin = margin
        info.horizontalPagination = .fit
        info.verticalPagination = .automatic
        info.dictionary()[NSPrintInfo.AttributeKey.headerAndFooter] = false
        let op = view.printOperation(with: info)
        op.showsPrintPanel = false
        op.showsProgressPanel = false
        // WebKit prints only from a window; an off-screen one, never shown.
        let window = NSWindow(contentRect: view.frame, styleMask: .borderless, backing: .buffered, defer: true)
        window.contentView = view
        let printed: Bool = await withCheckedContinuation { done in
            op.runModal(for: window, delegate: PrintDone.shared, didRun: #selector(PrintDone.printOperationDidRun(_:success:contextInfo:)),
                        contextInfo: Unmanaged.passRetained(PrintDone.Box(done)).toOpaque())
        }
        guard printed, let document = PDFDocument(url: pages), document.pageCount > 0 else {
            throw Failed(errorDescription: "WebKit could not lay out the PDF.")
        }
        try stamp(document, titlePage: titlePage, font: font, to: url)
    }

    /// Redraws each page with its number centred in the bottom margin, the pages' text and fonts kept (Q1).
    private static func stamp(_ document: PDFDocument, titlePage: Bool, font: CTFont, to url: URL) throws {
        var box = document.page(at: 0)?.bounds(for: .mediaBox) ?? CGRect(origin: .zero, size: paper)
        guard let context = CGContext(url as CFURL, mediaBox: &box, [kCGPDFContextTitle: document.documentAttributes?[PDFDocumentAttribute.titleAttribute] ?? ""] as CFDictionary) else {
            throw Failed(errorDescription: "\(url.lastPathComponent) could not be written.")
        }
        for i in 0..<document.pageCount {
            guard let page = document.page(at: i) else { continue }
            var media = page.bounds(for: .mediaBox)
            context.beginPage(mediaBox: &media)
            page.draw(with: .mediaBox, to: context)
            if let n = number(i, titlePage: titlePage) { drawNumber(n, font: font, in: media, context: context) }
            context.endPage()
        }
        context.closePDF()
    }

    /// `runModal`'s callback, as a continuation. ⚠️ AppKit calls it on the print operation's own thread, not the main one.
    private final class PrintDone: NSObject, Sendable {
        static let shared = PrintDone()
        final class Box: Sendable { let done: CheckedContinuation<Bool, Never>; init(_ d: CheckedContinuation<Bool, Never>) { done = d } }
        @objc nonisolated func printOperationDidRun(_ op: NSPrintOperation, success: Bool, contextInfo: UnsafeMutableRawPointer?) {
            guard let contextInfo else { return }
            Unmanaged<Box>.fromOpaque(contextInfo).takeRetainedValue().done.resume(returning: success)
        }
    }
    #else
    /// The print formatter laid out by a renderer whose footer is ours (Q1), drawn into a PDF.
    private final class Renderer: UIPrintPageRenderer {
        let size: CGSize, titlePage: Bool, font: CTFont
        init(size: CGSize, titlePage: Bool, font: CTFont) { self.size = size; self.titlePage = titlePage; self.font = font }
        override var paperRect: CGRect { CGRect(origin: .zero, size: size) }
        // The footer sits in the bottom margin's lower half; the text keeps a whole inch.
        override var printableRect: CGRect {
            CGRect(x: margin, y: margin, width: size.width - 2 * margin, height: size.height - 1.5 * margin)
        }
        override func drawFooterForPage(at pageIndex: Int, in footerRect: CGRect) {
            guard let n = number(pageIndex, titlePage: titlePage), let context = UIGraphicsGetCurrentContext() else { return }
            context.saveGState()
            // UIKit's PDF context is flipped; CoreText draws upright in an unflipped one.
            context.translateBy(x: 0, y: size.height)
            context.scaleBy(x: 1, y: -1)
            drawNumber(n, font: font, in: paperRect, context: context)
            context.restoreGState()
        }
    }

    private static func printPhone(_ view: WKWebView, titlePage: Bool, font: CTFont, to url: URL) throws {
        let renderer = Renderer(size: paper, titlePage: titlePage, font: font)
        renderer.footerHeight = margin / 2
        renderer.addPrintFormatter(view.viewPrintFormatter(), startingAtPageAt: 0)
        let pdf = UIGraphicsPDFRenderer(bounds: renderer.paperRect)
        let data = pdf.pdfData { context in
            renderer.prepare(forDrawingPages: NSRange(location: 0, length: renderer.numberOfPages))
            for i in 0..<renderer.numberOfPages {
                context.beginPage()
                renderer.drawPage(at: i, in: renderer.paperRect)
            }
        }
        guard renderer.numberOfPages > 0 else { throw Failed(errorDescription: "WebKit could not lay out the PDF.") }
        try data.write(to: url, options: .atomic)  // boundary-ok: Export Manuscript…, the PDF the writer asked for, outside any package
    }
    #endif

    /// The number centred, its baseline half an inch from the page's foot.
    nonisolated private static func drawNumber(_ n: String, font: CTFont, in page: CGRect, context: CGContext) {
        let line = CTLineCreateWithAttributedString(NSAttributedString(string: n, attributes: [
            NSAttributedString.Key(kCTFontAttributeName as String): font]))
        let width = CTLineGetTypographicBounds(line, nil, nil, nil)
        context.textPosition = CGPoint(x: page.midX - width / 2, y: page.minY + margin / 2)
        CTLineDraw(line, context)
    }
}
