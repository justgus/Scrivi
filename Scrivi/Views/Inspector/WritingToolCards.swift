import SwiftUI

// T-0363 / T-0364 — the writing-tool cards: tags, todo, outline.
// Design: Scrivi_Scene_Inspector_Card_Framework_v0_1.md §3.1.
//
// All three read and write the scene sidecar through ScriviEngine (SP-091 T-0392/T-0393).
// Swift is UI only (Architecture v0.3) — no logic beyond presentation and edit buffering.
//
// §2 manual-surfacing: none of these auto-populate. A card is empty until the writer
// types in it.

// MARK: — Shared per-scene notes store

/// Loads a scene's tags/outline/todo once and writes each field back independently.
/// One instance per (scene, card-stack) render, shared by the three cards so a stack
/// costs a single `getSceneNotes` crossing rather than three.
@Observable @MainActor final class SceneNotesModel {

    private(set) var tags: [String] = []
    private(set) var outline: String = ""
    private(set) var todo: [SceneTodoItem] = []
    private(set) var loadError: String?

    private let engine: ScriviEngine
    private let projectRootPath: String
    private let sceneID: String

    init(engine: ScriviEngine, projectRootPath: String, sceneID: String) {
        self.engine = engine
        self.projectRootPath = projectRootPath
        self.sceneID = sceneID
        load()
    }

    func load() {
        do {
            let notes = try engine.getSceneNotes(projectRootPath: projectRootPath, sceneID: sceneID)
            tags = notes.tags
            outline = notes.outline
            todo = notes.todo
            loadError = nil
        } catch {
            // A card that cannot load reports it in place (Doc 2 §7.1) rather than
            // rendering as empty — an empty card would look like lost work.
            loadError = error.localizedDescription
        }
    }

    // MARK: Tags

    func addTag(_ raw: String) {
        let tag = raw.trimmingCharacters(in: .whitespacesAndNewlines)
        guard !tag.isEmpty, !tags.contains(tag) else { return }
        tags.append(tag)
        saveTags()
    }

    func removeTag(_ tag: String) {
        guard tags.contains(tag) else { return }
        tags.removeAll { $0 == tag }
        saveTags()
    }

    private func saveTags() {
        do { try engine.setSceneTags(projectRootPath: projectRootPath, sceneID: sceneID, tags: tags) }
        catch { loadError = error.localizedDescription }
    }

    // MARK: Outline

    /// Called on commit (focus loss / explicit save), not per keystroke.
    func saveOutline(_ text: String) {
        guard text != outline else { return }
        outline = text
        do { try engine.setSceneOutline(projectRootPath: projectRootPath, sceneID: sceneID, outline: text) }
        catch { loadError = error.localizedDescription }
    }

    // MARK: Todo

    func addTodo(_ raw: String) {
        let text = raw.trimmingCharacters(in: .whitespacesAndNewlines)
        guard !text.isEmpty else { return }
        todo.append(SceneTodoItem(text: text, done: false))
        saveTodo()
    }

    func toggleTodo(at index: Int) {
        guard todo.indices.contains(index) else { return }
        todo[index].done.toggle()
        saveTodo()
    }

    func removeTodo(at index: Int) {
        guard todo.indices.contains(index) else { return }
        todo.remove(at: index)
        saveTodo()
    }

    private func saveTodo() {
        do { try engine.setSceneTodo(projectRootPath: projectRootPath, sceneID: sceneID, todo: todo) }
        catch { loadError = error.localizedDescription }
    }
}

// MARK: — Tags card

struct TagsCard: InspectorCard {
    static let typeID = "tags"
    static let title = "Tags"
    static let systemImage = "tag"
    static let stack: InspectorStack = .writing

    init() {}

    func body(context: CardContext) -> AnyView {
        AnyView(TagsCardBody(context: context))
    }
}

private struct TagsCardBody: View {
    let context: CardContext
    @State private var model: SceneNotesModel?
    @State private var newTag: String = ""

    var body: some View {
        VStack(alignment: .leading, spacing: 8) {
            if let model {
                if let error = model.loadError {
                    CardErrorView(message: error)
                }

                if model.tags.isEmpty {
                    Text("No tags yet.")
                        .font(.caption)
                        .foregroundStyle(.secondary)
                } else {
                    FlowLayout(spacing: 6) {
                        ForEach(model.tags, id: \.self) { tag in
                            TagChip(tag: tag) { model.removeTag(tag) }
                        }
                    }
                }

                TextField("Add a tag…", text: $newTag)
                    .textFieldStyle(.roundedBorder)
                    .font(.callout)
                    .onSubmit {
                        model.addTag(newTag)
                        newTag = ""
                    }
            } else {
                ProgressView().controlSize(.small)
            }
        }
        .task(id: context.sceneID) { model = makeModel(context) }
    }
}

private struct TagChip: View {
    let tag: String
    let onRemove: () -> Void

    var body: some View {
        HStack(spacing: 3) {
            Text(tag).font(.caption)
            Button(action: onRemove) {
                Image(systemName: "xmark.circle.fill")
                    .font(.caption2)
                    .foregroundStyle(.secondary)
            }
            .buttonStyle(.plain)
            .help("Remove tag")
        }
        .padding(.horizontal, 7)
        .padding(.vertical, 3)
        .background(.tint.opacity(0.15), in: Capsule())
    }
}

// MARK: — Outline card

struct OutlineCard: InspectorCard {
    static let typeID = "outline"
    static let title = "Outline"
    static let systemImage = "list.bullet.rectangle"
    static let stack: InspectorStack = .writing

    init() {}

    func body(context: CardContext) -> AnyView {
        AnyView(OutlineCardBody(context: context))
    }
}

private struct OutlineCardBody: View {
    let context: CardContext
    @State private var model: SceneNotesModel?
    @State private var draft: String = ""
    @FocusState private var focused: Bool

    var body: some View {
        VStack(alignment: .leading, spacing: 6) {
            if let model {
                if let error = model.loadError {
                    CardErrorView(message: error)
                }

                TextEditor(text: $draft)
                    .font(.callout)
                    .frame(minHeight: 72)
                    .scrollContentBackground(.hidden)
                    .padding(4)
                    .background(.background.opacity(0.5), in: RoundedRectangle(cornerRadius: 5))
                    .overlay(RoundedRectangle(cornerRadius: 5).strokeBorder(.quaternary))
                    .focused($focused)
                    // Commit on focus loss rather than per keystroke — one sidecar write
                    // per editing session, not one per character.
                    .onChange(of: focused) { _, isFocused in
                        if !isFocused { model.saveOutline(draft) }
                    }

                if draft.isEmpty {
                    Text("Summarize what happens in this scene.")
                        .font(.caption)
                        .foregroundStyle(.secondary)
                }
            } else {
                ProgressView().controlSize(.small)
            }
        }
        .task(id: context.sceneID) {
            let m = makeModel(context)
            model = m
            draft = m.outline
        }
        // A scene switch while the editor holds focus would otherwise lose the edit.
        .onDisappear { model?.saveOutline(draft) }
    }
}

// MARK: — Todo card

struct TodoCard: InspectorCard {
    static let typeID = "todo"
    static let title = "To-do"
    static let systemImage = "checklist"
    static let stack: InspectorStack = .writing

    init() {}

    func body(context: CardContext) -> AnyView {
        AnyView(TodoCardBody(context: context))
    }
}

private struct TodoCardBody: View {
    let context: CardContext
    @State private var model: SceneNotesModel?
    @State private var newItem: String = ""

    var body: some View {
        VStack(alignment: .leading, spacing: 6) {
            if let model {
                if let error = model.loadError {
                    CardErrorView(message: error)
                }

                if model.todo.isEmpty {
                    Text("Nothing to do yet.")
                        .font(.caption)
                        .foregroundStyle(.secondary)
                } else {
                    ForEach(Array(model.todo.enumerated()), id: \.offset) { index, item in
                        HStack(spacing: 6) {
                            Button {
                                model.toggleTodo(at: index)
                            } label: {
                                Image(systemName: item.done ? "checkmark.square.fill" : "square")
                                    .foregroundStyle(item.done ? Color.accentColor : .secondary)
                            }
                            .buttonStyle(.plain)

                            Text(item.text)
                                .font(.callout)
                                .strikethrough(item.done, color: .secondary)
                                .foregroundStyle(item.done ? .secondary : .primary)

                            Spacer(minLength: 0)

                            Button {
                                model.removeTodo(at: index)
                            } label: {
                                Image(systemName: "xmark")
                                    .font(.caption2)
                                    .foregroundStyle(.secondary)
                            }
                            .buttonStyle(.plain)
                            .help("Delete this item")
                        }
                    }
                }

                TextField("Add an item…", text: $newItem)
                    .textFieldStyle(.roundedBorder)
                    .font(.callout)
                    .onSubmit {
                        model.addTodo(newItem)
                        newItem = ""
                    }
            } else {
                ProgressView().controlSize(.small)
            }
        }
        .task(id: context.sceneID) { model = makeModel(context) }
    }
}

// MARK: — Shared helpers

@MainActor
private func makeModel(_ context: CardContext) -> SceneNotesModel {
    SceneNotesModel(engine: context.engine,
                    projectRootPath: context.projectRootPath,
                    sceneID: context.sceneID)
}

/// In-card failure presentation (Doc 2 §7.1) — the card reports its own problem and the
/// rest of the stack keeps rendering.
private struct CardErrorView: View {
    let message: String

    var body: some View {
        HStack(alignment: .top, spacing: 5) {
            Image(systemName: "exclamationmark.triangle")
                .font(.caption)
                .foregroundStyle(.orange)
            Text(message)
                .font(.caption)
                .foregroundStyle(.secondary)
        }
        .frame(maxWidth: .infinity, alignment: .leading)
    }
}

/// Minimal wrapping layout for tag chips — SwiftUI has no built-in flow container.
private struct FlowLayout: Layout {
    var spacing: CGFloat = 6

    func sizeThatFits(proposal: ProposedViewSize, subviews: Subviews, cache: inout ()) -> CGSize {
        let maxWidth = proposal.width ?? .infinity
        var x: CGFloat = 0, y: CGFloat = 0, rowHeight: CGFloat = 0
        for view in subviews {
            let size = view.sizeThatFits(.unspecified)
            if x + size.width > maxWidth, x > 0 {
                x = 0
                y += rowHeight + spacing
                rowHeight = 0
            }
            x += size.width + spacing
            rowHeight = max(rowHeight, size.height)
        }
        return CGSize(width: maxWidth == .infinity ? x : maxWidth, height: y + rowHeight)
    }

    func placeSubviews(in bounds: CGRect, proposal: ProposedViewSize,
                       subviews: Subviews, cache: inout ()) {
        var x = bounds.minX, y = bounds.minY, rowHeight: CGFloat = 0
        for view in subviews {
            let size = view.sizeThatFits(.unspecified)
            if x + size.width > bounds.maxX, x > bounds.minX {
                x = bounds.minX
                y += rowHeight + spacing
                rowHeight = 0
            }
            view.place(at: CGPoint(x: x, y: y), proposal: ProposedViewSize(size))
            x += size.width + spacing
            rowHeight = max(rowHeight, size.height)
        }
    }
}
