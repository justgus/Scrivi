import SwiftUI

// EP-058 S2 (SP-210) — authorship on the Mac and iPad. Design: docs/Scrivi_Authorship_Design_v0_2.md §4.2, §4.4.
// ⛔ An `unnamed` author's displayName is a host name or "Scrivi": it is never shown as a name, only as "Unnamed author".

private let unnamedLabel = "Unnamed author"

// MARK: — App Settings (Q1)

/// The app's own settings — the writer's, not a project's. Mac: the `Settings` scene (⌘,), one tab per pane.
/// iPad: a sheet from the Launch toolbar and the project's ⋯ menu, one row per pane.
struct AppSettingsView: View {
    #if !os(macOS)
    @Environment(\.dismiss) private var dismiss
    #endif

    var body: some View {
        #if os(macOS)
        TabView {
            PenNamesView()
                .tabItem { Label("Pen Names", systemImage: "person.text.rectangle") }
        }
        .frame(width: 460, height: 380)
        #else
        NavigationStack {
            Form {
                NavigationLink { PenNamesView() } label: {
                    Label("Pen Names", systemImage: "person.text.rectangle")
                }
            }
            .navigationTitle("Settings")
            .toolbar {
                ToolbarItem(placement: .confirmationAction) { Button("Done") { dismiss() } }
            }
        }
        #endif
    }
}

/// The pen-name menu: the names this writer writes under, and the default New Project and the prompts offer.
struct PenNamesView: View {
    @Environment(AppEnvironment.self) private var env
    @State private var newName = ""
    @State private var error: String?

    var body: some View {
        Form {
            Section {
                if env.penNames.penNames.isEmpty {
                    Text("No pen names yet.").foregroundStyle(.secondary)
                }
                ForEach(Array(env.penNames.penNames.enumerated()), id: \.element) { index, name in
                    PenNameRow(name: name, isDefault: name == env.penNames.default,
                               canMoveUp: index > 0, canMoveDown: index < env.penNames.penNames.count - 1,
                               rename: { rename(index, to: $0) }, makeDefault: { update { $0.default = name } },
                               move: { move(index, by: $0) }, remove: { remove(index) })
                }
            } footer: {
                Text("The default is offered for a new project, and when Scrivi asks who wrote a project.")
            }
            Section {
                HStack {
                    TextField("New pen name", text: $newName)
                        .onSubmit(add)
                    Button("Add", action: add)
                        .disabled(newName.trimmingCharacters(in: .whitespacesAndNewlines).isEmpty)
                }
                if let error { Text(error).foregroundStyle(.red).font(.callout) }
            }
        }
        .formStyle(.grouped)
        .navigationTitle("Pen Names")
    }

    private func update(_ change: (inout PenNames) -> Void) {
        var next = env.penNames
        change(&next)
        do { try env.savePenNames(next); error = nil } catch let e as ScriviError { error = e.message } catch {
            self.error = error.localizedDescription
        }
    }

    private func add() {
        let name = newName.trimmingCharacters(in: .whitespacesAndNewlines)
        guard !name.isEmpty else { return }
        update { names in
            names.penNames.append(name)
            if names.default.isEmpty { names.default = name }
        }
        if error == nil { newName = "" }
    }

    private func rename(_ index: Int, to name: String) {
        let name = name.trimmingCharacters(in: .whitespacesAndNewlines)
        update { names in
            if names.default == names.penNames[index] { names.default = name }
            names.penNames[index] = name
        }
    }

    private func move(_ index: Int, by offset: Int) {
        update { $0.penNames.swapAt(index, index + offset) }
    }

    private func remove(_ index: Int) {
        update { names in
            let removed = names.penNames.remove(at: index)
            if names.default == removed { names.default = names.penNames.first ?? "" }
        }
    }
}

private struct PenNameRow: View {
    let name: String
    let isDefault: Bool
    let canMoveUp: Bool
    let canMoveDown: Bool
    let rename: (String) -> Void
    let makeDefault: () -> Void
    let move: (Int) -> Void
    let remove: () -> Void
    @State private var text = ""

    var body: some View {
        HStack {
            TextField("Pen name", text: $text)
                .labelsHidden()
                .onSubmit { if text != name { rename(text) } }
            if isDefault {
                Text("Default").font(.caption).foregroundStyle(.secondary)
            }
            Menu {
                Button("Make Default", action: makeDefault).disabled(isDefault)
                Button("Move Up") { move(-1) }.disabled(!canMoveUp)
                Button("Move Down") { move(1) }.disabled(!canMoveDown)
                Divider()
                Button("Remove", role: .destructive, action: remove)
            } label: { Image(systemName: "ellipsis.circle") }
            .menuStyle(.button)
            .buttonStyle(.borderless)
            .fixedSize()
        }
        .onAppear { text = name }
        .onChange(of: name) { _, new in text = new }
    }
}

// MARK: — Authors of a project or a world

/// The ordered authors of a project or a world, and this machine's I write as. Each change is written at once.
struct AuthorsSection: View {
    /// The project's root, or a world's package path.
    let rootPath: String
    /// For a world: the open project's root (a world with no authors yet is given the project's author, SP-209 Q2).
    var projectRootPath: String = ""
    /// Called after I write as changes (the project's window adopts it).
    var onWritingAs: (Author) -> Void = { _ in }

    @Environment(AppEnvironment.self) private var env
    @Environment(ProjectSession.self) private var session
    @State private var authors: [Author] = []
    @State private var writingAs: Author?
    @State private var names: [String: String] = [:]   // personaID → the name in its field
    @State private var newName = ""
    @State private var error: String?

    var body: some View {
        Section {
            ForEach(Array(authors.enumerated()), id: \.element.id) { index, author in
                HStack {
                    TextField(author.unnamed ? unnamedLabel : "Name", text: binding(author))
                        .labelsHidden()
                        .onSubmit { save() }
                    Menu {
                        Button("Move Up") { move(index, by: -1) }.disabled(index == 0)
                        Button("Move Down") { move(index, by: 1) }.disabled(index == authors.count - 1)
                        Divider()
                        // A work keeps at least one author (the core refuses none).
                        Button("Remove", role: .destructive) { remove(index) }.disabled(authors.count == 1)
                    } label: { Image(systemName: "ellipsis.circle") }
                    .menuStyle(.button)
                    .buttonStyle(.borderless)
                    .fixedSize()
                }
            }
            HStack {
                TextField("Add an author", text: $newName)
                    .onSubmit(add)
                let offered = env.penNames.penNames.filter { name in !authors.contains { $0.displayName == name } }
                if !offered.isEmpty {
                    Menu {
                        ForEach(offered, id: \.self) { name in Button(name) { newName = name; add() } }
                    } label: { Image(systemName: "person.crop.circle.badge.plus") }
                    .menuStyle(.button)
                    .buttonStyle(.borderless)
                    .fixedSize()
                    .help("Add one of your pen names")
                }
                Button("Add", action: add)
                    .disabled(newName.trimmingCharacters(in: .whitespacesAndNewlines).isEmpty)
            }
            Picker("I write as", selection: Binding(get: { writingAs?.personaID ?? "" }, set: setWritingAs)) {
                if writingAs == nil { Text("—").tag("") }
                ForEach(authors) { Text($0.unnamed ? unnamedLabel : $0.displayName).tag($0.personaID) }
            }
            if let error { Text(error).foregroundStyle(.red).font(.callout) }
        } header: {
            Text("Authors")
        } footer: {
            Text("In byline order. Your edits here are recorded as the author you write as.")
        }
        .task(id: rootPath) { load() }
        .onDisappear { save() }
    }

    private func binding(_ author: Author) -> Binding<String> {
        Binding(get: { names[author.personaID] ?? "" }, set: { names[author.personaID] = $0 })
    }

    private func load() {
        do {
            let r = try env.engine.getAuthors(rootPath: rootPath, identityID: session.identityID,
                                              projectRootPath: projectRootPath)
            authors = r.authors
            writingAs = r.writingAs
            names = Dictionary(uniqueKeysWithValues: r.authors.map { ($0.personaID, $0.unnamed ? "" : $0.displayName) })
            error = nil
        } catch {
            self.error = (error as? ScriviError)?.message ?? error.localizedDescription
        }
    }

    /// The list as the fields show it; an unnamed author with an empty field keeps its old name (it stays unnamed).
    private func edits(_ list: [Author]) -> [AuthorEdit] {
        list.map { a in
            let typed = (names[a.personaID] ?? "").trimmingCharacters(in: .whitespacesAndNewlines)
            return AuthorEdit(personaID: a.personaID, displayName: typed.isEmpty ? a.displayName : typed)
        }
    }

    private func write(_ list: [AuthorEdit]) {
        do {
            try env.engine.setAuthors(rootPath: rootPath, authors: list)
            for edit in list where edit.personaID == nil { env.rememberPenName(edit.displayName) }
            load()
            if let writingAs { onWritingAs(writingAs) }   // a rename reaches the window's author too
        } catch {
            self.error = (error as? ScriviError)?.message ?? error.localizedDescription
        }
    }

    private func save() {
        let list = edits(authors)
        guard list.map(\.displayName) != authors.map(\.displayName) else { return }
        write(list)
    }

    private func add() {
        let name = newName.trimmingCharacters(in: .whitespacesAndNewlines)
        guard !name.isEmpty else { return }
        newName = ""
        write(edits(authors) + [AuthorEdit(personaID: nil, displayName: name)])
    }

    private func move(_ index: Int, by offset: Int) {
        var list = authors
        list.swapAt(index, index + offset)
        write(edits(list))
    }

    private func remove(_ index: Int) {
        var list = authors
        list.remove(at: index)
        write(edits(list))
    }

    private func setWritingAs(_ personaID: String) {
        guard !personaID.isEmpty else { return }
        do {
            try env.engine.setWritingAs(rootPath: rootPath, identityID: session.identityID, personaID: personaID)
            load()
            if let writingAs { onWritingAs(writingAs) }
        } catch {
            self.error = (error as? ScriviError)?.message ?? error.localizedDescription
        }
    }
}

/// Worlds ▸ Authors… — a world's own byline (Q5: edits to its objects are recorded as the project's author).
struct WorldAuthorsSheet: View {
    let worldName: String
    let worldPath: String
    let projectRootPath: String
    @Environment(\.dismiss) private var dismiss

    var body: some View {
        NavigationStack {
            Form {
                AuthorsSection(rootPath: worldPath, projectRootPath: projectRootPath)
            }
            .formStyle(.grouped)
            .navigationTitle("Authors of “\(worldName)”")
            .toolbar {
                ToolbarItem(placement: .confirmationAction) { Button("Done") { dismiss() } }
            }
        }
        #if os(macOS)
        .frame(minWidth: 420, minHeight: 320)
        #endif
    }
}

// MARK: — The prompts on open (Q2, Q4, Q6)

/// *Who wrote “Title”?* (every author unnamed) and *Which of these authors are you?* (this identity writes as no one).
/// ⚠️ It cannot be dismissed without an answer (Q4): nothing in the window is recorded as no one.
struct AuthorPromptSheet: View {
    @Environment(AppEnvironment.self) private var env
    @Environment(ProjectSession.self) private var session

    @State private var authors: [Author] = []
    @State private var names: [String: String] = [:]   // unnamed personaID → typed name
    @State private var newName = ""                    // a project with no authors, or Someone else…
    @State private var someoneElse = false
    @State private var error: String?

    private var root: String { session.projectRootPath ?? "" }
    private var title: String { session.openProjectResult?.projectTitle ?? "this project" }
    private var unnamed: [Author] { authors.filter(\.unnamed) }

    var body: some View {
        VStack(alignment: .leading, spacing: 16) {
            switch session.authorPrompt {
            case .nameAuthor: nameAuthor
            case .chooseAuthor: chooseAuthor
            case .none: EmptyView()
            }
            if let error { Text(error).foregroundStyle(.red).font(.callout) }
        }
        .padding(24)
        .frame(minWidth: 380)
        .interactiveDismissDisabled()
        .task { load() }
    }

    // MARK: Who wrote…?

    @ViewBuilder private var nameAuthor: some View {
        Text("Who wrote “\(title)”?").font(.title2.weight(.semibold))
        Text("Scrivi needs the author’s name — it goes in the byline when you export.")
            .foregroundStyle(.secondary)
        if unnamed.isEmpty {
            nameField("Author", text: $newName)
        } else {
            ForEach(unnamed) { author in
                nameField(unnamed.count > 1 ? "Author \((unnamed.firstIndex(of: author) ?? 0) + 1)" : "Author",
                          text: Binding(get: { names[author.personaID] ?? "" }, set: { names[author.personaID] = $0 }))
            }
        }
        HStack {
            Spacer()
            Button("Continue", action: answerNames)
                .keyboardShortcut(.defaultAction)
                .disabled(unnamed.isEmpty ? trimmed(newName).isEmpty
                                          : unnamed.contains { trimmed(names[$0.personaID] ?? "").isEmpty })
        }
    }

    private func answerNames() {
        do {
            var list = authors.map { a in
                AuthorEdit(personaID: a.personaID, displayName: a.unnamed ? trimmed(names[a.personaID] ?? "") : a.displayName)
            }
            if authors.isEmpty { list = [AuthorEdit(personaID: nil, displayName: trimmed(newName))] }
            try env.engine.setAuthors(rootPath: root, authors: list)
            for edit in list { env.rememberPenName(edit.displayName) }
            var r = try env.engine.getAuthors(rootPath: root, identityID: session.identityID)
            if r.writingAs == nil, r.authors.count == 1, let only = r.authors.first {
                // A project with no authors at all: the name just given is this writer's.
                try env.engine.setWritingAs(rootPath: root, identityID: session.identityID, personaID: only.personaID)
                r = try env.engine.getAuthors(rootPath: root, identityID: session.identityID)
            }
            finish(r)
        } catch {
            self.error = (error as? ScriviError)?.message ?? error.localizedDescription
        }
    }

    // MARK: Which of these authors are you?

    @ViewBuilder private var chooseAuthor: some View {
        Text("Which of these authors are you?").font(.title2.weight(.semibold))
        Text("Your edits to “\(title)” on this device are recorded as the author you choose.")
            .foregroundStyle(.secondary)
        ForEach(authors) { author in
            Button { choose(author.personaID) } label: {
                Text(author.unnamed ? unnamedLabel : author.displayName).frame(maxWidth: .infinity, alignment: .leading)
            }
        }
        if someoneElse {
            nameField("Your name", text: $newName)
            HStack {
                Spacer()
                Button("Add Me as an Author", action: addSelf)
                    .keyboardShortcut(.defaultAction)
                    .disabled(trimmed(newName).isEmpty)
            }
        } else {
            Button("Someone else…") { someoneElse = true; newName = env.penNames.default }
        }
    }

    private func choose(_ personaID: String) {
        do {
            try env.engine.setWritingAs(rootPath: root, identityID: session.identityID, personaID: personaID)
            finish(try env.engine.getAuthors(rootPath: root, identityID: session.identityID))
        } catch {
            self.error = (error as? ScriviError)?.message ?? error.localizedDescription
        }
    }

    private func addSelf() {
        let name = trimmed(newName)
        do {
            let before = Set(authors.map(\.personaID))
            let list = authors.map { AuthorEdit(personaID: $0.personaID, displayName: $0.displayName) }
                + [AuthorEdit(personaID: nil, displayName: name)]
            try env.engine.setAuthors(rootPath: root, authors: list)
            env.rememberPenName(name)
            let added = try env.engine.getAuthors(rootPath: root, identityID: session.identityID).authors
                .first { !before.contains($0.personaID) }
            guard let added else { return }
            choose(added.personaID)
        } catch {
            self.error = (error as? ScriviError)?.message ?? error.localizedDescription
        }
    }

    // MARK: Shared

    private func nameField(_ label: String, text: Binding<String>) -> some View {
        HStack {
            TextField(label, text: text)
                .textFieldStyle(.roundedBorder)
            if !env.penNames.penNames.isEmpty {
                Menu {
                    ForEach(env.penNames.penNames, id: \.self) { name in Button(name) { text.wrappedValue = name } }
                } label: { Image(systemName: "person.crop.circle") }
                .menuStyle(.button)
                .buttonStyle(.borderless)
                .fixedSize()
                .help("Choose a pen name")
            }
        }
    }

    private func load() {
        do {
            authors = try env.engine.getAuthors(rootPath: root, identityID: session.identityID).authors
            // Q6: one field per unnamed author, the first offered the default pen name.
            if let first = authors.first(where: \.unnamed) { names[first.personaID] = env.penNames.default }
            if authors.isEmpty { newName = env.penNames.default }
        } catch {
            self.error = (error as? ScriviError)?.message ?? error.localizedDescription
        }
    }

    /// The answer leads to the next question, if any (named, but this identity is not yet one of them); else the window
    /// writes as the chosen author.
    private func finish(_ r: AuthorsResult) {
        if let w = r.writingAs { session.adoptWritingAs(w) }
        authors = r.authors
        someoneElse = false
        error = nil
        session.authorPrompt = r.authorPrompt
    }

    private func trimmed(_ s: String) -> String { s.trimmingCharacters(in: .whitespacesAndNewlines) }
}
