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


/// EP-055 S3a (SP-222, T-0685) — undo/redo, structural edits and the cross-scene clipboard on iOS: the SAME `ManuscriptStructure`
/// the Mac's interop suite pins ("Undo after split / merge"), driven here through the iOS host — the system's undo manager, the
/// session's actions (the menu bar and the keys call these), cut and paste — on a real project opened by `loadAsync`.
@Suite("iOS manuscript: undo, structural edits, cross-scene clipboard (EP-055 S3a)")
@MainActor
struct IOSManuscriptStructureTests {

    @MainActor final class Harness {
        let root: URL
        let env = AppEnvironment()
        let session: ProjectSession
        let loader: ViewportSceneLoader
        let c: ManuscriptTextView.Coordinator
        let tv = ManuscriptUITextView()
        let window: UIWindow

        /// One chapter holding `bodies`, opened through `loadAsync` (history opened, every scene validated).
        init(_ bodies: [String]) async throws {
            root = FileManager.default.temporaryDirectory.appendingPathComponent("scrivi-ios-s3a-\(UUID().uuidString)")
            try FileManager.default.createDirectory(at: root, withIntermediateDirectories: true)
            let engine = env.engine, support = env.appSupportRoot, path = root.path(percentEncoded: false)
            let identity = try engine.ensureLocalIdentity(displayName: "iOS Test", appSupportRoot: support)
            let ref = AuthorshipRef(identityID: identity.identityID, personaID: identity.defaultPersonaID,
                                    displayName: identity.displayName)
            env.authorshipRef = ref
            let created = try engine.createProject(projectRootPath: path, appSupportRoot: support,
                                                   title: "iOS S3a", slug: "ios-s3a", authorshipRef: ref)
            _ = try engine.saveScene(projectID: created.projectID, projectRootPath: path, appSupportRoot: support,
                                     sceneID: created.firstScene.sceneID, sceneMetadataPath: created.firstScene.metadataPath,
                                     sceneContentPath: created.firstScene.contentPath, markdown: bodies[0], authorshipRef: ref)
            let chapterID = try #require(try engine.openProject(projectRootPath: path, appSupportRoot: support,
                                                                identityID: identity.identityID).scenes.first?.chapterID)
            var after = created.firstScene.sceneID
            for body in bodies.dropFirst() {
                let s = try engine.createScene(projectRootPath: path, appSupportRoot: support, projectID: created.projectID,
                                               chapterID: chapterID, afterSceneID: after, authorshipRef: ref)
                _ = try engine.saveScene(projectID: created.projectID, projectRootPath: path, appSupportRoot: support,
                                         sceneID: s.sceneID, sceneMetadataPath: s.metadataPath, sceneContentPath: s.contentPath,
                                         markdown: body, authorshipRef: ref)
                after = s.sceneID
            }
            session = ProjectSession(engine: engine, authorshipRef: ref, appSupportRoot: support, identityID: identity.identityID)
            _ = try await session.loadAsync(at: path) { _, _ in }
            loader = try #require(session.viewportLoader)
            let view = ManuscriptTextView(loader: loader, env: env, session: session, navigateToSceneID: .constant(nil),
                                          showChapterTitles: false, typography: .default)
            c = view.makeCoordinator()
            (tv.textLayoutManager?.textContentManager as? NSTextContentStorage)?.delegate = c.sync.presenter
            tv.textStorage.delegate = c.sync.presenter
            c.sync.boundaryTable.observe(tv.textStorage)
            tv.delegate = c
            tv.pasteboard = UIPasteboard.withUniqueName()   // ⚠️ never the writer's clipboard
            // The test host's own scene (⛔ `UIWindow(frame:)` is deprecated since iOS 26).
            let scene = try #require(UIApplication.shared.connectedScenes.compactMap { $0 as? UIWindowScene }.first)
            window = UIWindow(windowScene: scene)
            window.frame = CGRect(x: 0, y: 0, width: 600, height: 800)
            tv.textContainerInset = UIEdgeInsets(top: 24, left: 20, bottom: 24, right: 20)   // as `makeUIView`
            tv.frame = window.bounds
            window.addSubview(tv)
            c.textView = tv
            tv.structure = c.structure
            // The session's actions, as `makeUIView` installs them (the menu bar and the keys call these).
            session.createSceneAction   = { [c] in c.structure.handleCreateScene() }
            session.createChapterAction = { [c] in c.structure.handleCreateChapter() }
            session.mergeSceneAction    = { [c] in c.structure.handleMergeScene() }
            session.mergeChapterAction  = { [c] in c.structure.handleMergeChapter() }
            tv.searcher.replaceAllHandler = { [c] edits in c.structure.replaceAll(edits) }   // as `makeUIView` (SP-223)
            // An alert would wait for a tap that never comes: an unexpected one fails the test instead.
            c.onBarrierNotice = { Issue.record("unexpected barrier notice: \($0)") }
            c.onRedoFork = { fork in Issue.record("unexpected branch chooser at \(fork.nodeID)"); return nil }
            c.rebuildStorage(tv)
            #expect(loader.segments.map(\.text) == bodies)
        }
        deinit { try? FileManager.default.removeItem(at: root) }

        var texts: [String] { loader.segments.map(\.text) }
        func caret(scene i: Int, at offset: Int) {
            tv.selectedRange = NSRange(location: c.sync.sceneBoundaries(in: tv.textStorage)[i].location + offset, length: 0)
        }
        /// [SP-191] Waits until no structural edit is in flight (they run off the main actor).
        func landed() async throws {
            for _ in 0..<500 where loader.structuralInFlight { try await Task.sleep(for: .milliseconds(10)) }
            #expect(!loader.structuralInFlight, "the structural edit landed")
        }
        /// The storage shows the scene list: each scene's text in its range.
        func storageMatches(_ what: String) {
            let ns = tv.textStorage.string as NSString
            let shown = c.sync.sceneBoundaries(in: tv.textStorage).map { ns.substring(with: $0) }
            #expect(shown == texts, "\(what): the view shows the scene list")
        }
        /// The scene list is the disk (identity, chapter, text), against a fresh open.
        func listMatchesDisk(_ what: String) throws {
            let opened = try env.engine.openProject(projectRootPath: root.path(percentEncoded: false),
                                                    appSupportRoot: env.appSupportRoot, identityID: session.identityID)
            let disk = try opened.scenes.map { s in
                [s.sceneID, s.chapterID, try String(contentsOf: root.appendingPathComponent(s.contentPath), encoding: .utf8)]
            }
            #expect(loader.segments.map { [$0.sceneID, $0.chapterID, $0.text] } == disk, "\(what): the scene list is the disk")
        }
    }

    @Test("typing, then the SYSTEM's undo and redo (⌘Z, the keyboard's key, the swipe): Scrivi's history runs")
    func systemUndoRunsHistory() async throws {
        let h = try await Harness(["Alpha."])
        #expect(h.tv.undoManager?.canUndo == false, "nothing to undo yet")
        h.caret(scene: 0, at: 6)
        h.tv.insertText(" Beta")
        #expect(h.texts == ["Alpha. Beta"])
        #expect(h.tv.undoManager?.canUndo == true, "the system's Undo is enabled by the history")
        h.tv.undoManager?.undo()
        #expect(h.texts == ["Alpha."], "undone by the history")
        #expect(h.tv.textStorage.string == "Alpha.")
        #expect(h.tv.undoManager?.canRedo == true)
        h.tv.undoManager?.redo()
        #expect(h.texts == ["Alpha. Beta"], "redone")
        h.storageMatches("after redo")
    }

    @Test("split, merge, chapter split and chapter merge each undo and redo in ONE step; the view and the disk follow")
    func structuralEditsUndoAndRedo() async throws {
        let h = try await Harness(["Alpha one. Alpha two.", "Beta.", "Gamma."])
        let start = h.texts
        let ids = h.loader.segments.map(\.sceneID)
        func state() -> [[String]] { h.loader.segments.map { [$0.sceneID, $0.chapterID, $0.text] } }
        // Each op after the first starts a new branch from the same point: redo there asks (Q4) — the newest, here.
        var asked = 0
        h.c.onRedoFork = { fork in asked += 1; return fork.children.first(where: \.isPrimary)?.eventID }
        func roundTrip(_ what: String, _ op: () -> Void) async throws {
            let before = state()
            op()
            try await h.landed()
            let after = state()
            #expect(after != before, "\(what) changed the manuscript")
            h.storageMatches(what)
            try h.listMatchesDisk(what)
            h.tv.undoManager?.undo()
            try await h.landed()
            #expect(state() == before, "\(what): undone exactly, in ONE step")
            h.storageMatches("\(what), undone")
            h.tv.undoManager?.redo()
            try await h.landed()
            #expect(state() == after, "\(what): redone exactly")
            h.tv.undoManager?.undo()
            try await h.landed()
            #expect(state() == before)
        }
        try await roundTrip("split") { h.caret(scene: 0, at: 11); h.session.createSceneAction?() }
        try await roundTrip("merge") { h.caret(scene: 1, at: 0); h.session.mergeSceneAction?() }
        try await roundTrip("chapter split") { h.caret(scene: 0, at: 6); h.session.createChapterAction?() }
        h.caret(scene: 1, at: 0)
        h.session.createChapterAction?()
        try await h.landed()
        try await roundTrip("chapter merge") { h.caret(scene: 1, at: 0); h.session.mergeChapterAction?() }
        h.tv.undoManager?.undo()
        try await h.landed()
        #expect(h.texts == start)
        #expect(h.loader.segments.map(\.sceneID) == ids, "every scene kept its identity through all of it")
        #expect(asked > 0, "redo at a branch asked which (Q4)")
        try h.listMatchesDisk("the end")
    }

    @Test("a cut across scenes removes the break; Scrivi's paste puts the scenes back; each undoes in one step")
    func cutAndPasteAcrossScenes() async throws {
        let h = try await Harness(["Aaa.", "Bbb.", "Ccc."])
        let b = h.c.sync.sceneBoundaries(in: h.tv.textStorage)
        h.tv.selectedRange = NSRange(location: b[0].location + 2, length: b[1].location + 2 - (b[0].location + 2))
        h.tv.cut(nil)
        try await h.landed()
        #expect(h.texts == ["Aab.", "Ccc."], "the head of the first joins the tail of the second: \(h.texts)")
        #expect(h.tv.pasteboard.string?.isEmpty == false, "other apps get the text")
        #expect(h.c.structure.hasInternalStructuredRange, "Scrivi's clipboard holds the scenes")
        h.storageMatches("after the cut")
        h.caret(scene: 1, at: 4)
        h.tv.paste(nil)
        try await h.landed()
        #expect(h.texts.count == 3, "the cut's break comes back with the paste: \(h.texts)")
        #expect(h.texts.joined() == "Aab.Ccc.a.Bb", "\(h.texts)")
        h.storageMatches("after the paste")
        try h.listMatchesDisk("after the paste")
        h.tv.undoManager?.undo()
        try await h.landed()
        #expect(h.texts == ["Aab.", "Ccc."], "the paste undone in one step")
        h.tv.undoManager?.undo()
        try await h.landed()
        #expect(h.texts == ["Aaa.", "Bbb.", "Ccc."], "the cut undone in one step")
        h.storageMatches("the end")
    }

    @Test("⌫ over a selection across scenes is the structural cut, and leaves the clipboard alone")
    func deleteAcrossScenes() async throws {
        let h = try await Harness(["One two.", "Three four."])
        h.tv.pasteboard.string = "kept"
        let b = h.c.sync.sceneBoundaries(in: h.tv.textStorage)
        h.tv.selectedRange = NSRange(location: b[0].location + 4, length: b[1].location + 6 - (b[0].location + 4))
        h.tv.deleteBackward()
        try await h.landed()
        #expect(h.texts == ["One four."])
        #expect(h.tv.pasteboard.string == "kept")
        #expect(!h.c.structure.hasInternalStructuredRange)
    }

    @Test("while a structural edit is in flight the view takes no typing (the cue), and takes it again after")
    func inputHeldInFlight() async throws {
        let h = try await Harness(["Alpha one. Alpha two."])
        h.caret(scene: 0, at: 11)
        h.session.createSceneAction?()
        #expect(h.tv.inputHeld)
        h.tv.insertText("X")
        try await h.landed()
        #expect(!h.tv.inputHeld)
        #expect(h.texts == ["Alpha one. ", "Alpha two."], "nothing typed went in mid-flight: \(h.texts)")
    }

    // MARK: — EP-055 S3b (SP-223, T-0686)

    func range(_ h: Harness, scene i: Int, of needle: String) -> NSRange {
        let b = h.c.sync.sceneBoundaries(in: h.tv.textStorage)[i]
        let r = (h.tv.textStorage.string as NSString).range(of: needle, options: [], range: b)
        #expect(r.location != NSNotFound, "\(needle) in scene \(i)")
        return r
    }

    @Test("Find searches what the writer SEES: markers are not found, a match maps to its storage, never across a break")
    func findOverPresentedText() async throws {
        let h = try await Harness(["Some **bold** text.", "More bold here."])
        let text = h.tv.presentedMap.snapshot()
        let found = ManuscriptTextSearcher.matches(of: "bold", in: text, compare: [.caseInsensitive], method: .contains)
        let ns = h.tv.textStorage.string as NSString
        #expect(found.map { ns.substring(with: $0) } == ["bold", "bold"])
        #expect(found.first == range(h, scene: 0, of: "bold"), "the match is the storage under the bold text, no markers")
        #expect(ManuscriptTextSearcher.matches(of: "**", in: text, compare: [], method: .contains).isEmpty, "markers are hidden")
        #expect(ManuscriptTextSearcher.matches(of: "text.More", in: text, compare: [], method: .contains).isEmpty,
                "never across a scene break")
        #expect(ManuscriptTextSearcher.matches(of: "ol", in: text, compare: [], method: .fullWord).isEmpty, "whole words only")

        // ⌘E (SP-223 live pass: it did nothing): the selection, as presented, becomes the search text.
        h.tv.selectedRange = NSRange(location: range(h, scene: 0, of: "bold").location - 2, length: 8)   // `**bold**`
        let keys = h.tv.keyCommands ?? []
        #expect(keys.contains { $0.input == "e" && $0.modifierFlags == .command && $0.action == #selector(UIResponder.useSelectionForFind(_:)) },
                "⌘E is the view's own key")
        h.tv.useSelectionForFind(nil)
        #expect(h.tv.findInteraction?.searchText == "bold")
    }

    @Test("Replace replaces one match as typing; Replace All is ONE undo step across scenes, kept balanced")
    func replaceAndReplaceAll() async throws {
        let h = try await Harness(["Some **bold** text.", "More bold here."])
        let start = h.texts
        let some = try #require(h.tv.textRange(storage: range(h, scene: 0, of: "Some")))
        h.tv.searcher.replace(foundTextRange: some, document: nil, withText: "Any")
        #expect(h.texts == ["Any **bold** text.", "More bold here."])
        h.tv.undoManager?.undo()
        #expect(h.texts == start, "one replacement, one undo")

        h.tv.searcher.replaceAll(queryString: "bold", options: UITextSearchOptions(), withText: "bright*")
        try await h.landed()
        #expect(h.texts == ["Some **bright\\*** text.", "More bright\\* here."], "escaped, inside the span: \(h.texts)")
        h.storageMatches("after Replace All")
        try h.listMatchesDisk("after Replace All")
        h.tv.undoManager?.undo()
        try await h.landed()
        #expect(h.texts == start, "Replace All undone in ONE step")
        h.storageMatches("Replace All undone")
    }

    @Test("Format: Bold on a selection, ⌘I in a word — each one history event; between words a pending pair, never stored empty")
    func formatCommands() async throws {
        let h = try await Harness(["Some text here."])
        let start = h.texts
        h.tv.selectedRange = range(h, scene: 0, of: "text")
        h.session.formatAction = { [tv = h.tv] f in tv.applyFormat(f) }   // as `makeUIView`
        h.session.formatAction?(.bold)
        #expect(h.texts == ["Some **text** here."])
        h.tv.undoManager?.undo()
        #expect(h.texts == start, "one undo step")

        h.tv.selectedRange = NSRange(location: range(h, scene: 0, of: "here").location + 2, length: 0)
        h.tv.toggleItalics(nil)   // ⌘I, the keyboard's I
        #expect(h.texts == ["Some text *here*."])
        h.tv.undoManager?.undo()
        #expect(h.texts == start)

        // After the full stop: no word — a pending pair, in the view only.
        h.caret(scene: 0, at: 15)
        h.tv.applyFormat(.bold)
        #expect(h.tv.textStorage.string == "Some text here.****")
        #expect(h.texts == start, "an empty pair is never stored")
        h.tv.insertText("X")
        #expect(h.texts == ["Some text here.**X**"], "typing into it makes it a real span")
        h.tv.undoManager?.undo()
        #expect(h.texts == start)
        h.caret(scene: 0, at: 15)
        h.tv.applyFormat(.bold)
        h.tv.applyFormat(.bold)
        #expect(h.tv.textStorage.string == "Some text here.", "the same command again takes the pair away")
        h.tv.applyFormat(.italic)
        h.tv.deleteBackward()
        #expect(h.tv.textStorage.string == "Some text here.", "⌫ takes an empty pair away")
        h.tv.applyFormat(.italic)
        h.caret(scene: 0, at: 0)
        #expect(h.tv.textStorage.string == "Some text here.", "leaving it takes it away")
        #expect(h.texts == start)
    }

    @Test("SP-223 live pass: one arrow step passes one visible letter (never into a hidden marker); the pending pair is drawn")
    func caretSnapAndPendingDrawn() async throws {
        let h = try await Harness(["A **bold** word."])
        let b = range(h, scene: 0, of: "bold").location        // just after the opener
        h.tv.selectedRange = NSRange(location: b - 3, length: 0)  // after "A"
        h.tv.selectedRange = NSRange(location: b - 2, length: 0)  // → proposes the spot before the hidden `**`
        #expect(h.tv.selectedRange.location == b, "→ lands in the bold word (⛔ was three presses)")
        #expect(try drawnColor(h, at: b - 2) == UIColor.manuscriptHint, "the caret at the span's start shows its markers")
        h.tv.selectedRange = NSRange(location: b - 1, length: 0)  // ← proposes the middle of the `**`
        #expect(h.tv.selectedRange.location == b - 3, "← passes the space before it")
        #expect(try drawnColor(h, at: b - 2) == UIColor.clear, "and away from it they hide again")

        // ⌘B after the full stop: the pair is drawn as hints, not hidden.
        let end = (h.tv.textStorage.string as NSString).length
        h.tv.selectedRange = NSRange(location: end, length: 0)
        _ = try drawnColor(h, at: end - 1)   // the line is laid out already, as on screen
        h.tv.applyFormat(.bold)
        #expect(try drawnColor(h, at: end) == UIColor.manuscriptHint, "the pending pair shows, dimmed")
    }

    /// The colour the text view DRAWS storage unit `i` in — the presented paragraph, not the storage.
    func drawnColor(_ h: Harness, at i: Int) throws -> UIColor? {
        let lm = try #require(h.tv.textLayoutManager), cm = try #require(lm.textContentManager)
        lm.ensureLayout(for: cm.documentRange)
        let at = try #require(cm.location(cm.documentRange.location, offsetBy: i))
        let paragraph = try #require(lm.textLayoutFragment(for: at)?.textElement as? NSTextParagraph)
        let local = cm.offset(from: try #require(paragraph.elementRange?.location), to: at)
        return paragraph.attributedString.attribute(.foregroundColor, at: local, effectiveRange: nil) as? UIColor
    }

    @Test("SP-223 live pass: the edit menu's Format is Scrivi's (headings and lists too), in place of the system's Bold / Italic")
    func editMenuFormatIsScrivis() async throws {
        let h = try await Harness(["Some text."])
        let system = UIMenu(title: "Format", identifier: .format, children: [UIAction(title: "Bold") { _ in }])
        let menu = try #require(h.c.textView(h.tv, editMenuForTextIn: NSRange(location: 0, length: 4),
                                             suggestedActions: [UIAction(title: "Copy") { _ in }, system]))
        let formats = menu.children.compactMap { $0 as? UIMenu }.filter { $0.title == "Format" }
        #expect(formats.count == 1)
        let titles = formats.first?.children.flatMap { ($0 as? UIMenu)?.children ?? [$0] }.map(\.title) ?? []
        #expect(titles.contains("Heading 1") && titles.contains("Numbered List"))
    }

    @Test("the copy buffers: copy then paste, cut — the shared code, each change one history event")
    func copyBuffers() async throws {
        let h = try await Harness(["Some **text** here.", "Other."])
        let start = h.texts
        let service = try #require(h.session.bufferService)
        h.tv.selectedRange = NSRange(location: range(h, scene: 0, of: "text").location - 2, length: 6)
        h.c.structure.copyIntoBuffer("1")
        #expect(service.text(inSlot: "1") == "**text**", "the balanced source")
        #expect(h.texts == start, "a copy changes nothing")
        h.caret(scene: 1, at: 6)
        h.c.structure.pasteFromBuffer("1")
        #expect(h.texts == ["Some **text** here.", "Other.**text**"])
        h.tv.undoManager?.undo()
        #expect(h.texts == start, "the paste undone in one step")

        h.tv.selectedRange = range(h, scene: 0, of: "here")
        h.c.structure.cutIntoBuffer("2")
        #expect(service.text(inSlot: "2") == "here")
        #expect(h.texts == ["Some **text** .", "Other."])
        h.tv.undoManager?.undo()
        #expect(h.texts == start, "the cut undone in one step")
        #expect(service.text(inSlot: "2") == "here", "the slot keeps it")
    }

    @Test("a branch point shows a mark in the margin on its line; a tap asks which branch, and redoes it")
    func branchMarks() async throws {
        let h = try await Harness(["Alpha."])
        h.caret(scene: 0, at: 6)
        h.tv.insertText(" Beta.")
        h.tv.undoManager?.undo()
        h.caret(scene: 0, at: 6)
        h.tv.insertText(" Gamma.")
        h.tv.undoManager?.undo()
        #expect(h.session.historyCapture?.redoForkNow != nil, "two branches part here")
        h.tv.layoutIfNeeded()
        h.tv.textLayoutManager?.textViewportLayoutController.layoutViewport()
        h.c.refreshBranchMarks()
        let mark = try #require(h.c.branchMarks.first)
        #expect(h.c.branchMarks.count == 1, "one line, one mark")
        #expect(mark.frame.maxX <= h.tv.textContainerInset.left, "in the margin, not over the text")
        var asked = 0
        h.c.onRedoFork = { fork in asked += 1; return fork.children.first(where: { !$0.isPrimary })?.eventID }
        mark.sendActions(for: .touchUpInside)
        #expect(asked == 1)
        #expect(h.texts == ["Alpha. Beta\\."], "the other branch was redone (a typed full stop is stored escaped): \(h.texts)")
    }
}

@Suite("iOS panel swipes (SP-199, design §8)")
struct PanelSwipeTests {
    let w: CGFloat = 1000

    @Test("width regular: each edge rule, and a full-width swipe does both (G1)")
    func regular() {
        #expect(PanelSwipe.actions(startX: 5, endX: 400, width: w, compact: false) == [.showNavigator])
        #expect(PanelSwipe.actions(startX: 500, endX: 950, width: w, compact: false) == [.hideInspector])
        #expect(PanelSwipe.actions(startX: 5, endX: 990, width: w, compact: false) == [.showNavigator, .hideInspector])
        #expect(PanelSwipe.actions(startX: 995, endX: 600, width: w, compact: false) == [.showInspector])
        #expect(PanelSwipe.actions(startX: 500, endX: 50, width: w, compact: false) == [.hideNavigator])
        #expect(PanelSwipe.actions(startX: 995, endX: 10, width: w, compact: false) == [.showInspector, .hideNavigator])
        #expect(PanelSwipe.actions(startX: 300, endX: 700, width: w, compact: false).isEmpty, "mid-screen does nothing")
    }

    @Test("width compact: a leftward swipe from the right edge is the next page; rightward is the system's back")
    func compact() {
        #expect(PanelSwipe.actions(startX: 995, endX: 600, width: w, compact: true) == [.nextPage])
        #expect(PanelSwipe.actions(startX: 995, endX: 10, width: w, compact: true) == [.nextPage])
        #expect(PanelSwipe.actions(startX: 500, endX: 10, width: w, compact: true).isEmpty)
        #expect(PanelSwipe.actions(startX: 5, endX: 990, width: w, compact: true).isEmpty)
    }

    @Test("from an owned edge any inward swipe counts, even curving up; mid-screen it must be sideways")
    func edgeConsumes() {
        #expect(PanelSwipe.isSwipe(startX: 10, tx: 80, ty: -300, vx: 50, width: w, compact: false))
        #expect(PanelSwipe.isSwipe(startX: 990, tx: -80, ty: 250, vx: -50, width: w, compact: false))
        #expect(!PanelSwipe.isSwipe(startX: 10, tx: -5, ty: 300, vx: 0, width: w, compact: false), "outward is not a swipe")
        #expect(!PanelSwipe.isSwipe(startX: 500, tx: 80, ty: -300, vx: 50, width: w, compact: false), "mid-screen scroll")
        #expect(!PanelSwipe.ownsEdge(startX: 10, width: w, compact: true), "the left edge is the system's back swipe")
        #expect(PanelSwipe.ownsEdge(startX: 990, width: w, compact: true))
    }
}

@Suite("Timeline time scale (T-0641)")
struct TimelineScaleTests {
    private let hour: Int64 = 3_600_000, day: Int64 = 86_400_000

    @Test func labelsCountFromOneAtTheEpoch() {
        #expect(TimelineScale.label(ms: 0, step: day) == "Day 1")
        #expect(TimelineScale.label(ms: 2 * day, step: day) == "Day 3")
        #expect(TimelineScale.label(ms: -day, step: day) == "Day -1")
        #expect(TimelineScale.label(ms: 2 * day + 14 * hour, step: hour) == "Day 3 14:00")
        #expect(TimelineScale.label(ms: 365 * day, step: 365 * day) == "Year 2")
    }

    @Test func ticksAreSpacedAndInsideTheWindow() {
        // One day across 900 pt: 90 pt apart needs a 2.4 h step, so 3 h ticks.
        let ticks = TimelineScale.ticks(visibleMinMs: hour / 2, visibleSpanMs: day, usable: 900)
        #expect(ticks.first?.ms == 3 * hour)
        #expect(zip(ticks, ticks.dropFirst()).allSatisfy { $1.ms - $0.ms == 3 * hour })
        #expect(ticks.allSatisfy { $0.ms >= hour / 2 && $0.ms <= hour / 2 + day })
    }

    @Test func aHugeSpanStillGetsLabels() {
        // ~240 years (the Dumas fixture's framed span) across 1,000 pt.
        let ticks = TimelineScale.ticks(visibleMinMs: 0, visibleSpanMs: 240 * 365 * day, usable: 1000)
        #expect(!ticks.isEmpty && ticks.count < 20)
        #expect(ticks.first?.label == "Year 1")
    }
}

@Suite("Toolbar Navigate targets (SP-199, shared with the Mac)")
@MainActor
struct NavigateTargetTests {
    // Scenes at 10..<20, 21..<30 (chapter A), 31..<40 (chapter B); a chapter heading before 10; length 40.
    let b = [NSRange(location: 10, length: 10), NSRange(location: 21, length: 9), NSRange(location: 31, length: 9)]
    let ch = ["A", "A", "B"]
    func t(_ s: ManuscriptSceneSync.NavigateScope, _ e: ManuscriptEdge, _ caret: Int) -> Int? {
        ManuscriptSceneSync.navigateTarget(s, e, caret: caret, boundaries: b, chapterIDs: ch, length: 40)
    }

    @Test("scene, chapter and manuscript edges from the caret")
    func edges() {
        #expect(t(.scene, .start, 25) == 21)
        #expect(t(.scene, .end, 25) == 30)
        #expect(t(.chapter, .start, 25) == 10)
        #expect(t(.chapter, .end, 12) == 30)
        #expect(t(.chapter, .start, 35) == 31)
        #expect(t(.manuscript, .start, 35) == 10, "[I-0266] the first scene, not offset 0")
        #expect(t(.manuscript, .end, 12) == 40)
    }
}
