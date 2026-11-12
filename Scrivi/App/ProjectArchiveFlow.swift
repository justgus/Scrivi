import SwiftUI
import UniformTypeIdentifiers
#if os(macOS)
import AppKit
#endif

// File ▸ Export Archive… / Import Archive… (EP-054 S2, SP-206), and Export Manuscript… / Copy as HTML (S3, SP-207). ⚠️ No archive logic here: the flow asks the writer and
// calls `scrivi_export_archive` / `scrivi_import_archive`; a clash comes back as a conflict and the call is made again.
//
// Rulings: SP-206 Q1 — an import whose project is already open (same ID) is unpacked but NOT opened, because the open
// path would raise the original's window (R3); Q2 — Export exports the focused window's project, flushed first; Q3 — a
// double-clicked `.scrivarchive` starts Import. EP-054 Q4 — a project name clash is Cancel / Rename / Replace; SP-205 Q3
// — a world name clash is Cancel / Rename.

/// A question the archive flow puts to the writer. The Mac answers it with an `NSAlert`; iOS with an `.alert`.
struct ArchivePrompt: Identifiable {
    let id = UUID()
    let title: String
    let message: String
    let buttons: [String]          // the last one cancels
    let destructive: Int?          // the index of a destructive button (Replace)
    var name: String?              // a name field, prefilled, when non-nil
    let reply: (Int, String) -> Void
}

/// iOS: which file the archive picker is choosing.
enum ArchivePicker: Equatable {
    case archive
    case folder(archive: URL, name: String)   // name: the project folder the writer chose
}

@MainActor
extension AppEnvironment {

    static let archiveType = UTType("com.caposoft.scrivi.archive") ?? .zip

    // MARK: — Export (Q2: the focused window's project)

    func exportArchive(_ session: ProjectSession) async {
        guard let root = session.projectRootPath else { return }
        let engine = session.engine
        let name = URL(fileURLWithPath: root).deletingPathExtension().lastPathComponent
        await session.saveAllDirty()   // the editor's unsaved text; the core then drains the write queue

        var includeWorlds = false
        if let worlds = try? engine.listWorlds(projectRootPath: root).worlds, !worlds.isEmpty {
            let names = worlds.map { "“\($0.displayName)”" }.joined(separator: ", ")
            let r = await ask("Include the world?",
                              "“\(name)” uses \(names). Include \(worlds.count == 1 ? "it" : "them") in the archive, "
                              + "so \(worlds.count == 1 ? "it travels" : "they travel") with the project?",
                              ["Include World", "Project Only", "Cancel"])
            if r.choice == 2 { return }
            includeWorlds = r.choice == 0
        }

        // ⚠️ The core writes the archive through a temporary file beside it, and the sandbox grants only the file the
        // writer named — so the core writes into a folder the app may use, and the result is moved into place.
        let destination: URL
        #if os(macOS)
        let panel = NSSavePanel()
        panel.allowedContentTypes = [Self.archiveType]
        panel.nameFieldStringValue = "\(name).scrivarchive"
        panel.title = "Export Archive"
        panel.prompt = "Export"
        guard panel.runModal() == .OK, let url = panel.url else { return }
        destination = url
        let staging = stagingFolder(for: url)
        #else
        let staging = FileManager.default.temporaryDirectory.appendingPathComponent(UUID().uuidString)
        destination = staging.appendingPathComponent("\(name).scrivarchive")
        #endif
        try? FileManager.default.createDirectory(at: staging, withIntermediateDirectories: true)
        let written = staging.appendingPathComponent("\(name).scrivarchive").path(percentEncoded: false)

        let withWorlds = includeWorlds
        do {
            let result = try await offMain("Exporting archive…", show: { session.archiveProgress = $0 }) {
                try engine.exportArchive(projectRootPath: root, archivePath: written, includeWorlds: withWorlds)
            }
            #if os(macOS)
            try moveIntoPlace(URL(fileURLWithPath: written), destination)
            try? FileManager.default.removeItem(at: staging)
            #endif
            if !result.skippedWorlds.isEmpty {
                let ids = Set(result.skippedWorlds.map(\.worldID))
                let names = ((try? engine.listWorlds(projectRootPath: root).worlds) ?? [])
                    .filter { ids.contains($0.worldID) }.map { "“\($0.displayName)”" }.joined(separator: ", ")
                await tell("The archive has no world",
                           "\(names.isEmpty ? "A world" : names) could not be reached, so it was left out. "
                           + "The project is in the archive.")
            }
            #if !os(macOS)
            share(destination)
            #endif
        } catch {
            try? FileManager.default.removeItem(at: staging)
            await tell("The archive could not be exported", message(for: error))
        }
    }

    // MARK: — Manuscript (EP-054 S3, SP-207)

    /// File ▸ Export Manuscript…: Markdown, HTML (a whole page), a Word document (S4, SP-208) or an ePub (S5, SP-213), the
    /// whole manuscript or the caret's chapter (Q2, Q3); an ePub is always the whole manuscript (SP-213 Q7).
    func exportManuscript(_ session: ProjectSession) async {
        guard let root = session.projectRootPath else { return }
        let engine = session.engine
        await session.saveAllDirty()   // the editor's unsaved text; the core then drains the write queue
        let f = await ask("Export Manuscript",
                          "Markdown keeps the text as Scrivi stores it. HTML is a web page with every paragraph its own <p>. "
                          + "A Word document keeps your typeface and indent in named styles. "
                          + "An ePub is the whole manuscript as a book for e-readers.",
                          ["Markdown", "HTML", "Word Document (.docx)", "ePub", "Cancel"])
        guard f.choice < 4 else { return }
        let format = ["markdown", "html", "docx", "epub"][f.choice]
        let ext = ["md", "html", "docx", "epub"][f.choice]
        let scope = format == "epub" ? (cancelled: false, chapterID: nil, chapterName: nil)
                                     : await askScope(session, "Export Manuscript")
        guard !scope.cancelled else { return }

        let projectName = URL(fileURLWithPath: root).deletingPathExtension().lastPathComponent
        let name = (scope.chapterName ?? projectName).replacingOccurrences(of: "/", with: "-")
            .replacingOccurrences(of: ":", with: "-")
        let fileName = "\(name).\(ext)"
        #if os(macOS)
        let panel = NSSavePanel()
        panel.allowedContentTypes = [UTType(filenameExtension: ext) ?? .plainText]
        panel.nameFieldStringValue = fileName
        panel.title = "Export Manuscript"
        panel.prompt = "Export"
        guard panel.runModal() == .OK, let destination = panel.url else { return }
        #else
        let folder = FileManager.default.temporaryDirectory.appendingPathComponent(UUID().uuidString)
        try? FileManager.default.createDirectory(at: folder, withIntermediateDirectories: true)
        let destination = folder.appendingPathComponent(fileName)
        #endif
        let chapterID = scope.chapterID
        do {
            if format == "docx" || format == "epub" {
                // As the archive: the core writes through a file beside the .docx or .epub, which the sandbox does not grant beside
                // the writer's file — so it writes into a folder the app may use, and the result is moved into place.
                #if os(macOS)
                let staging = stagingFolder(for: destination)
                try? FileManager.default.createDirectory(at: staging, withIntermediateDirectories: true)
                defer { try? FileManager.default.removeItem(at: staging) }
                let written = staging.appendingPathComponent(fileName)
                #else
                let written = destination
                #endif
                let path = written.path(percentEncoded: false)
                let face = BundledFonts.manifest.default
                let language = Locale.preferredLanguages.first   // SP-213 Q6: there is no project language
                _ = try await offMain("Exporting manuscript…", show: { session.archiveProgress = $0 }) {
                    try engine.exportManuscript(projectRootPath: root, format: format, chapterID: chapterID,
                                                destPath: path, defaultTypeface: face, language: language)
                }
                #if os(macOS)
                try moveIntoPlace(written, destination)
                #endif
            } else {
                let result = try await offMain("Exporting manuscript…", show: { session.archiveProgress = $0 }) {
                    try engine.exportManuscript(projectRootPath: root, format: format, chapterID: chapterID)
                }
                // Not atomic: an atomic write goes through a file beside the destination, which the sandbox does not grant.
                try (result.text ?? "").write(to: destination, atomically: false, encoding: .utf8)  // boundary-ok: Export Manuscript…, the file the writer chose in the save panel, outside any package
            }
            #if !os(macOS)
            share(destination)
            #endif
        } catch {
            await tell("The manuscript could not be exported", message(for: error))
        }
    }

    /// File ▸ Copy as HTML: the HTML SOURCE as plain text, the body alone, for pasting into AO3's box (Q1, Q3).
    func copyManuscriptAsHTML(_ session: ProjectSession) async {
        guard let root = session.projectRootPath else { return }
        let engine = session.engine
        await session.saveAllDirty()
        let scope = await askScope(session, "Copy as HTML")
        guard !scope.cancelled else { return }
        let chapterID = scope.chapterID
        do {
            let result = try await offMain("Copying as HTML…", show: { session.archiveProgress = $0 }) {
                try engine.exportManuscript(projectRootPath: root, format: "html", chapterID: chapterID, wholePage: false)
            }
            #if os(macOS)
            NSPasteboard.general.clearContents()
            NSPasteboard.general.setString(result.text ?? "", forType: .string)
            #else
            UIPasteboard.general.string = result.text ?? ""
            #endif
            await tell("Copied as HTML", "\(scope.chapterName.map { "“\($0)”" } ?? "The manuscript") is on the clipboard "
                       + "as HTML. Paste it into the site's HTML box.")
        } catch {
            await tell("The manuscript could not be copied", message(for: error))
        }
    }

    /// Q2: the whole manuscript, or the chapter the caret is in (asked only when there is one).
    private func askScope(_ session: ProjectSession, _ title: String)
        async -> (cancelled: Bool, chapterID: String?, chapterName: String?) {
        guard let chapter = caretChapter(session) else { return (false, nil, nil) }
        let r = await ask(title, "The whole manuscript, or only “\(chapter.name)”, the chapter the caret is in?",
                          ["Whole Manuscript", "“\(chapter.name)”", "Cancel"])
        switch r.choice {
        case 0: return (false, nil, nil)
        case 1: return (false, chapter.id, chapter.name)
        default: return (true, nil, nil)
        }
    }

    /// The chapter the caret is in, named as the editor names it ("Chapter N" when untitled).
    private func caretChapter(_ session: ProjectSession) -> (id: String, name: String)? {
        guard let loader = session.viewportLoader,
              let sceneID = loader.cursorSceneID ?? loader.viewportSceneID,
              let info = loader.allScenes.first(where: { $0.sceneID == sceneID }) else { return nil }
        let title = info.chapterTitle.trimmingCharacters(in: .whitespaces)
        if !title.isEmpty { return (info.chapterID, title) }
        var chapters: [String] = []
        for s in loader.allScenes where chapters.last != s.chapterID { chapters.append(s.chapterID) }
        return (info.chapterID, "Chapter \((chapters.firstIndex(of: info.chapterID) ?? 0) + 1)")
    }

    // MARK: — Import

    #if os(macOS)
    /// File ▸ Import Archive…: the archive, then the folder to unpack into.
    func presentImportArchivePanel() {
        let panel = NSOpenPanel()
        panel.allowedContentTypes = [Self.archiveType]
        panel.allowsMultipleSelection = false
        panel.title = "Import Archive"
        panel.prompt = "Choose"
        guard panel.runModal() == .OK, let archive = panel.url else { return }
        chooseFolderAndImport(archive)
    }

    /// Q3: a double-clicked archive arrives here too. The name first (prefilled with the archive's), then the folder.
    func chooseFolderAndImport(_ archive: URL) {
        Task {
            let stem = archive.deletingPathExtension().lastPathComponent
            let r = await ask("Import “\(stem)”", "Name the project, then choose the folder to unpack it into.",
                              ["Choose Folder…", "Cancel"], name: stem)
            guard r.choice == 0 else { return }
            chooseFolder(archive, name: r.name)
        }
    }

    private func chooseFolder(_ archive: URL, name: String) {
        let panel = NSOpenPanel()
        panel.canChooseFiles = false
        panel.canChooseDirectories = true
        panel.canCreateDirectories = true
        panel.allowsMultipleSelection = false
        panel.directoryURL = archive.deletingLastPathComponent()
        panel.title = "Import “\(archive.deletingPathExtension().lastPathComponent)”"
        panel.message = "Choose the folder to unpack the project into."
        panel.prompt = "Import Here"
        guard panel.runModal() == .OK, let folder = panel.url else { return }
        Task { await importArchive(archive, into: folder, name: name) }
    }
    #endif

    func importArchive(_ archive: URL, into folder: URL, name: String) async {
        #if os(macOS)
        // ⚠️ The sandbox: a world already on this Mac is found only if its folder is readable — otherwise the core sees
        // no world there and unpacks a second copy (EP-054 Q8).
        WorldBookmarkStore.activateEverything()
        #endif
        let archiveScoped = archive.startAccessingSecurityScopedResource()
        defer { if archiveScoped { archive.stopAccessingSecurityScopedResource() } }
        // ⚠️ iOS: the folder's grant is HELD — the imported project is read through it while it is open.
        _ = folder.startAccessingSecurityScopedResource()

        let archivePath = archive.path(percentEncoded: false), folderPath = folder.path(percentEncoded: false)
        let host = frontmostSession ?? activeSession
        let show: @MainActor (String?) -> Void = { [weak self] text in
            if let host { host.archiveProgress = text } else { self?.archiveProgress = text }
        }
        var projectFolderName = name, replaceProject = false, worldFolderNames: [String: String] = [:]

        while true {
            let result: ImportArchiveResult
            do {
                let engine = engine, p = projectFolderName, r = replaceProject, w = worldFolderNames
                result = try await offMain("Importing archive…", show: show) {
                    try engine.importArchive(archivePath: archivePath, destFolderPath: folderPath, projectFolderName: p,
                                             replaceProject: r, worldFolderNames: w)
                }
            } catch let e as ScriviError where e.detail == "projectOpen" {
                await tell("Close the project first",
                           "The project in “\(folder.lastPathComponent)” that Replace would delete is open. "
                           + "Close its window, then import again.")
                return
            } catch {
                await tell("The archive could not be imported", message(for: error))
                return
            }

            if let conflict = result.conflict {
                let stem = (conflict.folderName as NSString).deletingPathExtension
                if conflict.kind == "project" {
                    let r = await ask("“\(conflict.folderName)” already exists",
                                      "“\(folder.lastPathComponent)” already holds a project with this name. Rename the "
                                      + "imported project, or replace the one that is there?",
                                      ["Rename", "Replace", "Cancel"], destructive: 1, name: "\(stem) 2")
                    switch r.choice {
                    case 0: projectFolderName = r.name
                    case 1: replaceProject = true
                    default: return
                    }
                } else {
                    let r = await ask("“\(conflict.folderName)” already exists",
                                      "“\(folder.lastPathComponent)” already holds a different world with this name. "
                                      + "Rename the imported world?",
                                      ["Rename", "Cancel"], name: "\(stem) 2")
                    guard r.choice == 0, let id = conflict.worldID else { return }
                    worldFolderNames[id] = r.name
                }
                continue
            }

            for world in result.worlds where world.unpacked {
                WorldBookmarkStore.record(worldID: world.worldID, url: URL(fileURLWithPath: world.packagePath))
            }
            // Q1: the open path would raise the window of the copy that is already open (R3) — say so instead.
            if let open = openProjects.session(for: result.projectID) {
                await tell("A copy of this project is already open",
                           "“\(open.projectRootPath ?? "")” is open. Close it to open the one you imported, "
                           + "at “\(result.projectRoot)”.")
                return
            }
            await openProjectFile(at: URL(fileURLWithPath: result.projectRoot, isDirectory: true))
            return
        }
    }

    #if os(macOS)
    /// A folder the app may write in, on the destination's volume (the sandbox grants only the file the writer named).
    private func stagingFolder(for url: URL) -> URL {
        (try? FileManager.default.url(for: .itemReplacementDirectory, in: .userDomainMask, appropriateFor: url, create: true))
            ?? FileManager.default.temporaryDirectory.appendingPathComponent(UUID().uuidString)
    }

    private func moveIntoPlace(_ made: URL, _ destination: URL) throws {
        if FileManager.default.fileExists(atPath: destination.path(percentEncoded: false)) {
            _ = try FileManager.default.replaceItemAt(destination, withItemAt: made)   // the panel asked about overwriting
        } else {
            try FileManager.default.moveItem(at: made, to: destination)
        }
    }
    #endif

    // MARK: — Asking the writer

    func ask(_ title: String, _ message: String, _ buttons: [String], destructive: Int? = nil,
             name: String? = nil) async -> (choice: Int, name: String) {
        #if os(macOS)
        let alert = NSAlert()
        alert.messageText = title
        alert.informativeText = message
        for (i, b) in buttons.enumerated() {
            let button = alert.addButton(withTitle: b)
            button.hasDestructiveAction = i == destructive
        }
        var field: NSTextField?
        if let name {
            let f = NSTextField(string: name)
            f.frame = NSRect(x: 0, y: 0, width: 260, height: 24)
            alert.accessoryView = f
            alert.window.initialFirstResponder = f
            field = f
        }
        let choice = alert.runModal().rawValue - NSApplication.ModalResponse.alertFirstButtonReturn.rawValue
        return (choice, field?.stringValue ?? "")
        #else
        return await withCheckedContinuation { done in
            archivePrompt = ArchivePrompt(title: title, message: message, buttons: buttons, destructive: destructive,
                                          name: name) { [weak self] choice, text in
                self?.archivePrompt = nil
                done.resume(returning: (choice, text))
            }
        }
        #endif
    }

    func tell(_ title: String, _ message: String) async {
        _ = await ask(title, message, ["OK"])
    }

    private func message(for error: Error) -> String {
        (error as? ScriviError)?.message ?? error.localizedDescription
    }

    /// Runs `work` off the main actor; `show` gets `label` once it has run 400 ms, and nil when it ends.
    private func offMain<T: Sendable>(_ label: String, show: @escaping @MainActor (String?) -> Void,
                                      _ work: @escaping @Sendable () throws -> T) async throws -> T {
        let reveal = Task { @MainActor in
            try? await Task.sleep(for: .milliseconds(400))
            if !Task.isCancelled { show(label) }
        }
        defer { reveal.cancel(); show(nil) }
        return try await Task.detached(priority: .userInitiated, operation: work).value
    }

    #if !os(macOS)
    /// The share sheet, for the archive just written (AC3).
    private func share(_ url: URL) {
        guard let scene = UIApplication.shared.connectedScenes.first(where: { $0.activationState == .foregroundActive })
                as? UIWindowScene,
              var top = scene.keyWindow?.rootViewController else { return }
        while let presented = top.presentedViewController { top = presented }
        let sheet = UIActivityViewController(activityItems: [url], applicationActivities: nil)
        sheet.popoverPresentationController?.sourceView = top.view
        sheet.popoverPresentationController?.sourceRect = CGRect(x: top.view.bounds.midX, y: top.view.bounds.minY, width: 0, height: 0)
        top.present(sheet, animated: true)
    }
    #endif
}

#if !os(macOS)
/// iOS: the archive picker (File ▸ Import Archive…, Landing, a double-clicked archive) and the archive flow's questions.
struct ArchiveFlowHost: ViewModifier {
    @Bindable var env: AppEnvironment
    @State private var name = ""
    // The step on screen. ⚠️ Not read back from `env.archivePicker`: the binding's setter clears that when the picker
    // closes, which can be BEFORE the completion runs — the pick was then dropped as `nil` (SP-206 AC7, iPad).
    @State private var step: ArchivePicker?

    func body(content: Content) -> some View {
        content
            .onChange(of: env.archivePicker) { _, now in if let now { step = now } }
            .fileImporter(isPresented: Binding(get: { env.archivePicker != nil },
                                               set: { if !$0 { env.archivePicker = nil } }),
                          allowedContentTypes: env.archivePicker == .archive ? [AppEnvironment.archiveType] : [.folder],
                          allowsMultipleSelection: false) { result in
                env.archivePicker = nil
                guard case .success(let urls) = result, let url = urls.first else { return }
                switch step {
                case .archive:
                    // Say why a second picker is coming (the Mac's panel has a title and message; this one has neither).
                    // ⚠️ "The same folder" cannot be a button: the picker grants this file, not the folder it is in — the
                    // folder picker opens there instead. Each presentation waits for the last: UIKit drops one made while
                    // another is dismissing.
                    Task { @MainActor in
                        await Self.untilNothingPresented()
                        let stem = url.deletingPathExtension().lastPathComponent
                        let r = await env.ask("Import “\(stem)”",
                                              "Name the project, then choose the folder to unpack it into. The picker "
                                              + "opens in the archive's own folder — tap Open to use it.",
                                              ["Choose Folder…", "Cancel"], name: stem)
                        guard r.choice == 0 else { return }
                        await Self.untilNothingPresented()
                        env.archivePicker = .folder(archive: url, name: r.name)
                    }
                case .folder(let archive, let name):
                    Task { await env.importArchive(archive, into: url, name: name) }
                case nil:
                    break
                }
            }
            .fileDialogDefaultDirectory({ if case .folder(let a, _) = step { a.deletingLastPathComponent() } else { nil } }())
            .alert(env.archivePrompt?.title ?? "", isPresented: Binding(get: { env.archivePrompt != nil }, set: { _ in }),
                   presenting: env.archivePrompt) { prompt in
                if prompt.name != nil { TextField("Name", text: $name) }
                ForEach(Array(prompt.buttons.enumerated()), id: \.offset) { i, title in
                    Button(title, role: i == prompt.destructive ? .destructive
                                      : (i == prompt.buttons.count - 1 && prompt.buttons.count > 1 ? .cancel : nil)) {
                        prompt.reply(i, name)
                    }
                }
            } message: { prompt in
                Text(prompt.message)
            }
            .onChange(of: env.archivePrompt?.id) { _, _ in name = env.archivePrompt?.name ?? "" }
    }

    /// Waits (≤ 2 s) until nothing is presented over the key window — a closing picker or alert included.
    @MainActor private static func untilNothingPresented() async {
        for _ in 0..<40 where UIApplication.shared.connectedScenes.contains(where: {
            ($0 as? UIWindowScene)?.keyWindow?.rootViewController?.presentedViewController != nil
        }) {
            try? await Task.sleep(for: .milliseconds(50))
        }
    }
}
#endif
