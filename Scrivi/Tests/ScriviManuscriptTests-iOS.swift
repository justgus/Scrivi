import Testing
import Foundation
import SwiftUI
import UIKit
@testable import Scrivi

// EP-055 S1 (T-0635) — the iOS manuscript, on the iOS simulator. The SAME storage rules the Mac's interop suite pins
// ("Escape layer", "Return and Backspace", "Typed scene dividers") — now applied by `ManuscriptUITextView`, which must
// store the same bytes. Run: `xcodebuild -workspace Scrivi.xcworkspace -scheme ScriviApp-iOS
// -destination 'platform=iOS Simulator,name=iPhone Duo' test`.

/// The REAL `ManuscriptUITextView` with the REAL `ManuscriptPresenter` installed as the app installs it, in a window.
/// ⚠️ A PRIVATE pasteboard: the simulator can share the Mac's clipboard, and a test run must never touch the writer's.
@MainActor private final class IOSManuscriptFixture {
    let tv = ManuscriptUITextView()
    let presenter = ManuscriptPresenter()
    let window = UIWindow(frame: CGRect(x: 0, y: 0, width: 400, height: 600))

    init(_ text: String = "") {
        (tv.textLayoutManager?.textContentManager as? NSTextContentStorage)?.delegate = presenter
        tv.textStorage.delegate = presenter
        tv.frame = window.bounds
        tv.pasteboard = UIPasteboard.withUniqueName()
        window.addSubview(tv)
        tv.typingAttributes = ManuscriptTypography.default.bodyAttributes
        if !text.isEmpty {
            tv.textStorage.replaceCharacters(in: NSRange(location: 0, length: 0),
                                             with: NSAttributedString(string: text, attributes: ManuscriptTypography.default.bodyAttributes))
        }
    }
    var text: String { tv.textStorage.string }
    func caret(_ i: Int) { tv.selectedRange = NSRange(location: i, length: 0) }
    /// What either keyboard sends.
    func type(_ s: String) { tv.insertText(s) }
}

/// Two scenes as the render builds them: `a`, a divider, `b`.
@MainActor private func twoScenes(_ a: String, _ b: String) -> IOSManuscriptFixture {
    let f = IOSManuscriptFixture(a)
    let divider = SceneDivider.string(DividerTextAttachment.make(.sceneBreak), state: .sceneBreak,
                                      newlineAttributes: ManuscriptTypography.default.bodyAttributes)
    f.tv.textStorage.append(divider)
    f.tv.textStorage.append(NSAttributedString(string: b, attributes: ManuscriptTypography.default.bodyAttributes))
    return f
}

@Suite("iOS manuscript: what typing stores (EP-055 S1)")
@MainActor
struct IOSManuscriptStorageTests {

    @Test("the view is TextKit 2, and its own defaults are set (no nil pasteboard)")
    func textKit2() {
        let tv = ManuscriptUITextView()
        #expect(tv.textLayoutManager != nil, "TextKit 2")
        #expect(tv.pasteboard === UIPasteboard.general)
    }

    @Test("typed Markdown characters are stored ESCAPED, as on the Mac (EP-045 AC4)")
    func typingEscapes() {
        let f = IOSManuscriptFixture()
        f.type("a*b_c#")
        #expect(f.text == "a\\*b\\_c\\#")
    }

    @Test("Return stores a paragraph, trailing spaces reduced to one (EP-045 AC5/AC6)")
    func returnStoresParagraph() {
        let f = IOSManuscriptFixture("x")
        f.caret(1); f.type("\n")
        #expect(f.text == "x\n\n")
        #expect(f.tv.selectedRange == NSRange(location: 3, length: 0))
        let s = IOSManuscriptFixture("x   ")
        s.caret(4); s.type("\n")
        #expect(s.text == "x \n\n")
    }

    @Test("⌫ at a paragraph's start joins it to the one above with ONE space (EP-045 Q3)")
    func backspaceJoins() {
        let f = IOSManuscriptFixture("a.\n\nb.")
        f.caret(4); f.tv.deleteBackward()
        #expect(f.text == "a. b.")
    }

    @Test("⌫ removes a whole escape pair, never half of it (EP-045 AC4)")
    func backspaceEscapePair() {
        let f = IOSManuscriptFixture()
        f.type("a*")
        #expect(f.text == "a\\*")
        f.tv.deleteBackward()
        #expect(f.text == "a")
    }

    @Test("⛔ [I-0270] ⌫ never deletes a scene divider")
    func dividerSurvivesBackspace() {
        let f = twoScenes("one", "two")
        let before = f.text
        let bStart = (f.text as NSString).range(of: "two").location
        f.caret(bStart); f.tv.deleteBackward()
        #expect(f.text == before, "⌫ at a scene's start is refused")
        // A selection holding the divider is refused too (the shared verdict).
        #expect(!f.tv.shouldChange(NSRange(location: 2, length: 3), to: ""))
        #expect(SceneDivider.sceneBoundaries(in: f.tv.textStorage)?.count == 2)
    }

    @Test("a chapter title cannot be edited")
    func headingRefused() {
        var attrs = ManuscriptTypography.default.chapterTitleAttributes
        attrs[.scriviHeading] = true
        let f = IOSManuscriptFixture()
        f.tv.textStorage.append(NSAttributedString(string: "Chapter 1\n", attributes: attrs))
        f.tv.textStorage.append(NSAttributedString(string: "text", attributes: ManuscriptTypography.default.bodyAttributes))
        #expect(!f.tv.shouldChange(NSRange(location: 3, length: 1), to: "X"))
        #expect(!f.tv.shouldChange(NSRange(location: 0, length: 0), to: "X"), "[I-0266] typing at offset 0, before the title")
    }

    @Test("paste from another app is stored escaped; Scrivi's own copy keeps its source")
    func pasteEscapes() {
        let f = IOSManuscriptFixture("ab")
        f.tv.pasteboard.string = "x*y"
        f.caret(2); f.tv.paste(nil)
        #expect(f.text == "abx\\*y")
        let g = IOSManuscriptFixture("p\\*q")
        g.tv.selectedRange = NSRange(location: 0, length: 4)
        g.tv.copy(nil)
        #expect(g.tv.pasteboard.string == "p*q", "other apps get what the writer SEES")
        g.caret(4); g.tv.paste(nil)
        #expect(g.text == "p\\*qp\\*q", "Scrivi's own paste gets the stored source")
    }

    @Test("[I-0325] gaining focus keeps the caret where the navigation put it")
    func focusKeepsCaret() {
        let f = IOSManuscriptFixture(String(repeating: "Some words here. ", count: 2000))
        f.window.makeKeyAndVisible()
        f.caret(500)
        #expect(!f.tv.isFirstResponder)
        let ok = f.tv.becomeFirstResponder()
        #expect(ok)
        #expect(f.tv.selectedRange == NSRange(location: 500, length: 0), "after focus: \(f.tv.selectedRange)")
    }

    @Test("[I-0325] after a long jump the target is centred, and a tap there hits it (⛔ was 35,507 characters away)")
    func jumpCentresAndHitTests() async throws {
        let para = String(repeating: "Some words in a paragraph that wraps across several lines of the view. ", count: 6) + "\n\n"
        let scene = String(repeating: para, count: 3) + "## A heading"
        let f = IOSManuscriptFixture(scene)
        for _ in 0..<1300 {     // ~1.8 MB in 1,300 scenes, a divider between each, like dumas
            f.tv.textStorage.append(SceneDivider.string(DividerTextAttachment.make(.sceneBreak), state: .sceneBreak,
                                                        newlineAttributes: ManuscriptTypography.default.bodyAttributes))
            f.tv.textStorage.append(NSAttributedString(string: scene, attributes: ManuscriptTypography.default.bodyAttributes))
        }
        f.window.makeKeyAndVisible()
        f.tv.layoutIfNeeded()
        let target = f.text.utf16.count * 7 / 8
        f.caret(target)
        f.tv.centre(target)
        let centre = CGPoint(x: f.tv.bounds.midX, y: f.tv.contentOffset.y + f.tv.bounds.height / 2)
        let pos = try #require(f.tv.closestPosition(to: centre))
        let hit = f.tv.offset(from: f.tv.beginningOfDocument, to: pos)
        #expect(abs(hit - target) < 1500, "within the screen: target \(target), hit \(hit), length \(f.text.utf16.count), offsetY \(f.tv.contentOffset.y), contentH \(f.tv.contentSize.height)")
    }

    @Test("[SP-198] undo is Scrivi's: the view registers nothing for the system to undo")
    func noSystemUndo() {
        let f = IOSManuscriptFixture()
        f.type("abc")
        #expect(f.tv.undoManager != nil)
        #expect(f.tv.undoManager?.canUndo == false)
    }
}

/// The shared model sync, as the iOS host drives it — a real project on disk.
@Suite("iOS manuscript: typed text reaches the scene file (EP-055 S1)")
@MainActor
struct IOSManuscriptSaveTests {

    @Test("a scene typed on iOS is autosaved to its .md, escaped")
    func typedTextIsSaved() async throws {
        let root = FileManager.default.temporaryDirectory.appendingPathComponent("scrivi-ios-\(UUID().uuidString)")
        let support = FileManager.default.temporaryDirectory.appendingPathComponent("scrivi-ios-support-\(UUID().uuidString)")
        try FileManager.default.createDirectory(at: root, withIntermediateDirectories: true)
        try FileManager.default.createDirectory(at: support, withIntermediateDirectories: true)
        defer { try? FileManager.default.removeItem(at: root); try? FileManager.default.removeItem(at: support) }

        let env = AppEnvironment()
        let identity = try env.engine.ensureLocalIdentity(displayName: "iOS Test", appSupportRoot: support.path)
        let ref = AuthorshipRef(identityID: identity.identityID, personaID: identity.defaultPersonaID,
                                displayName: identity.displayName)
        env.authorshipRef = ref
        _ = try env.engine.createProject(projectRootPath: root.path, appSupportRoot: support.path,
                                         title: "iOS Save", slug: "ios-save", authorshipRef: ref)
        let session = ProjectSession(engine: env.engine, authorshipRef: ref, appSupportRoot: support.path,
                                     identityID: identity.identityID)
        try await session.loadAsync(at: root.path)
        let loader = try #require(session.viewportLoader)
        let sceneID = try #require(loader.segments.first?.sceneID)

        let view = ManuscriptTextView(loader: loader, env: env, session: session, navigateToSceneID: .constant(nil),
                                      showChapterTitles: false, typography: .default)
        let c = view.makeCoordinator()
        let tv = ManuscriptUITextView()
        (tv.textLayoutManager?.textContentManager as? NSTextContentStorage)?.delegate = c.sync.presenter
        tv.textStorage.delegate = c.sync.presenter
        c.sync.boundaryTable.observe(tv.textStorage)
        tv.delegate = c
        c.textView = tv
        c.rebuildStorage(tv)

        tv.selectedRange = NSRange(location: tv.textStorage.length, length: 0)
        tv.insertText("Hello *world*")
        #expect(loader.segments.first?.text.hasSuffix("Hello \\*world\\*") == true, "the loader has the scene text at once")

        try await Task.sleep(nanoseconds: 1_600_000_000)   // the 1 s autosave
        _ = try env.engine.waitForWrites(projectRootPath: root.path, timeoutMs: 3000)
        let read = try env.engine.readSceneTexts(projectRootPath: root.path, sceneIDs: [sceneID])
        let markdown = try #require(read.scenes.first?.markdown)
        #expect(markdown.hasSuffix("Hello \\*world\\*") || markdown.hasSuffix("Hello \\*world\\*\n"),
                "on disk: \(markdown.debugDescription)")
    }
}
