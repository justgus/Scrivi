import SwiftUI

// SceneNavigatorView — sidebar list of all scenes in manuscript order.
//
// Chapters are rendered as non-selectable header rows in a flat ForEach,
// which lets SwiftUI's built-in List reorder handle both scene and chapter
// drag-reorder in a single onMove callback.
//
// Right-click (macOS) / long-press (iOS) on a row opens a context menu
// with Rename and Delete actions.
struct SceneNavigatorView: View {

    var loader: ViewportSceneLoader
    var env: AppEnvironment
    var onNavigate: (String) -> Void        // sceneID — tap-to-navigate
    var onDeleteNavigate: (String) -> Void  // sceneID — navigate after delete (triggers focus transfer)

    // Rename sheet state
    @State private var renameTarget: RenameTarget? = nil
    // Delete confirmation state
    @State private var deleteSceneTarget: SceneEntry? = nil
    @State private var deleteChapterTarget: ChapterGroup? = nil
    // Error alert state
    @State private var alertError: String? = nil
    // List selection — drives navigation without competing with drag gestures
    @State private var selectedRowID: String? = nil

    var body: some View {
        List(selection: $selectedRowID) {
            ForEach(flatRows, id: \.rowID) { row in
                switch row {
                case .chapterHeader(let group):
                    chapterHeaderRow(for: group)
                        .listRowSeparator(.hidden)
                case .scene(let entry, let group):
                    sceneRow(for: entry, in: group)
                }
            }
            .onMove { source, destination in
                performMove(from: source, to: destination)
            }
        }
        .listStyle(.inset)
        .frame(minWidth: 180, idealWidth: 220, maxWidth: 280)
        .onChange(of: selectedRowID) { _, newValue in
            guard let rowID = newValue else { return }
            if rowID.hasPrefix("scene-") {
                let sceneID = String(rowID.dropFirst("scene-".count))
                navigate(to: sceneID)
            } else {
                // Chapter header tapped — clear selection, no navigation
                selectedRowID = nil
            }
        }
        .onChange(of: loader.currentSegment?.sceneID) { _, sceneID in
            if let sceneID {
                selectedRowID = "scene-\(sceneID)"
            }
        }
        // MARK: Rename sheet
        .sheet(item: $renameTarget) { target in
            RenameSheet(
                target: target,
                onSave: { newTitle in
                    renameTarget = nil
                    performRename(target: target, newTitle: newTitle)
                },
                onCancel: {
                    renameTarget = nil
                }
            )
        }
        // MARK: Delete scene confirmation
        .confirmationDialog(
            "Delete Scene?",
            isPresented: Binding(
                get: { deleteSceneTarget != nil },
                set: { if !$0 { deleteSceneTarget = nil } }
            ),
            titleVisibility: .visible
        ) {
            if let entry = deleteSceneTarget {
                Button("Delete \u{201C}\(truncated(entry.title))\u{201D}", role: .destructive) {
                    performDeleteScene(entry: entry)
                    deleteSceneTarget = nil
                }
                Button("Cancel", role: .cancel) {
                    deleteSceneTarget = nil
                }
            }
        } message: {
            Text("This scene will be permanently deleted and cannot be recovered.")
        }
        // MARK: Delete chapter confirmation
        .confirmationDialog(
            "Delete Chapter?",
            isPresented: Binding(
                get: { deleteChapterTarget != nil },
                set: { if !$0 { deleteChapterTarget = nil } }
            ),
            titleVisibility: .visible
        ) {
            if let group = deleteChapterTarget {
                Button("Delete \u{201C}\(truncated(group.chapterTitle))\u{201D} and all its scenes", role: .destructive) {
                    performDeleteChapter(group: group)
                    deleteChapterTarget = nil
                }
                Button("Cancel", role: .cancel) {
                    deleteChapterTarget = nil
                }
            }
        } message: {
            if let group = deleteChapterTarget {
                let count = group.scenes.count
                let noun = count == 1 ? "scene" : "scenes"
                Text("\u{201C}\(truncated(group.chapterTitle))\u{201D} contains \(count) \(noun). All scenes will be permanently deleted and cannot be recovered.")
            }
        }
        // MARK: Error alert
        .alert("Error", isPresented: Binding(
            get: { alertError != nil },
            set: { if !$0 { alertError = nil } }
        )) {
            Button("OK") { alertError = nil }
        } message: {
            Text(alertError ?? "")
        }
    }

    // MARK: — Scene row

    @ViewBuilder
    private func sceneRow(for entry: SceneEntry, in group: ChapterGroup) -> some View {
        let isActive = entry.sceneID == loader.currentSegment?.sceneID
        NavigatorSceneRow(title: entry.title, isActive: isActive)
            .tag("scene-\(entry.sceneID)")
            .listRowBackground(isActive ? Color.accentColor.opacity(0.12) : Color.clear)
            .contextMenu {
                Button("Rename") { renameTarget = .scene(entry) }
                Divider()
                Button("Delete", role: .destructive) { deleteSceneTarget = entry }
            }
            .swipeActions(edge: .trailing, allowsFullSwipe: false) {
                Button("Delete", role: .destructive) { deleteSceneTarget = entry }
            }
    }

    // MARK: — Chapter header row

    @ViewBuilder
    private func chapterHeaderRow(for group: ChapterGroup) -> some View {
        Text(group.chapterTitle)
            .font(.caption)
            .foregroundStyle(.secondary)
            .tag("header-\(group.chapterID)")
            .listRowBackground(Color.clear)
            .contextMenu {
                Button("Rename") {
                    renameTarget = .chapter(group)
                }
                Divider()
                Button("Delete", role: .destructive) {
                    deleteChapterTarget = group
                }
            }
    }

    // MARK: — Navigation

    private func navigate(to sceneID: String) {
        Task { @MainActor in
            if let ref = env.authorshipRef {
                await loader.navigateTo(sceneID: sceneID, engine: env.engine, ref: ref)
            }
            onNavigate(sceneID)
        }
    }

    // MARK: — Rename

    private func performRename(target: RenameTarget, newTitle: String) {
        guard let projectRootPath = env.projectRootPath else { return }
        Task { @MainActor in
            do {
                switch target {
                case .scene(let entry):
                    _ = try env.engine.renameScene(
                        projectRootPath: projectRootPath,
                        metadataPath: entry.metadataPath,
                        newTitle: newTitle
                    )
                    loader.updateSceneTitle(newTitle, forMetadataPath: entry.metadataPath)
                case .chapter(let group):
                    _ = try env.engine.renameChapter(
                        projectRootPath: projectRootPath,
                        metadataPath: group.chapterMetadataPath,
                        newTitle: newTitle
                    )
                    loader.updateChapterTitle(newTitle, forChapterMetadataPath: group.chapterMetadataPath)
                }
            } catch let e as ScriviError {
                alertError = e.message
            } catch {
                alertError = error.localizedDescription
            }
        }
    }

    // MARK: — Delete

    private func performDeleteScene(entry: SceneEntry) {
        guard let projectRootPath = env.projectRootPath else { return }
        let wasCurrentScene = entry.sceneID == loader.currentSegment?.sceneID
        Task { @MainActor in
            do {
                _ = try env.engine.deleteScene(
                    projectRootPath: projectRootPath,
                    sceneID: entry.sceneID
                )
                if let nextSceneID = loader.removeScene(sceneID: entry.sceneID) {
                    if wasCurrentScene, let ref = env.authorshipRef {
                        // Navigate and transfer keyboard focus to the text view.
                        await loader.navigateTo(sceneID: nextSceneID, engine: env.engine, ref: ref)
                        onDeleteNavigate(nextSceneID)
                    } else {
                        navigate(to: nextSceneID)
                    }
                }
            } catch let e as ScriviError {
                alertError = e.message
            } catch {
                alertError = error.localizedDescription
            }
        }
    }

    private func performDeleteChapter(group: ChapterGroup) {
        guard let projectRootPath = env.projectRootPath else { return }
        let wasCurrentChapter = loader.currentSegment.map { seg in group.scenes.contains { $0.sceneID == seg.sceneID } } ?? false
        Task { @MainActor in
            do {
                _ = try env.engine.deleteChapter(
                    projectRootPath: projectRootPath,
                    chapterID: group.chapterID
                )
                if let nextSceneID = loader.removeChapter(chapterID: group.chapterID) {
                    if wasCurrentChapter, let ref = env.authorshipRef {
                        await loader.navigateTo(sceneID: nextSceneID, engine: env.engine, ref: ref)
                        onDeleteNavigate(nextSceneID)
                    } else {
                        navigate(to: nextSceneID)
                    }
                }
            } catch let e as ScriviError {
                alertError = e.message
            } catch {
                alertError = error.localizedDescription
            }
        }
    }

    // MARK: — onMove handler

    private func performMove(from source: IndexSet, to destination: Int) {
        guard let sourceIdx = source.first else { return }
        let rows = flatRows

        guard sourceIdx < rows.count else { return }
        let movedRow = rows[sourceIdx]

        // SwiftUI destination is the insertion point in the original array (source still present).
        // The row immediately before the landing spot is rows[destination - 1].
        let predecessorIdx = destination - 1

        switch movedRow {
        case .scene(let entry, let sourceGroup):
            var afterSceneID = ""
            var targetGroup: ChapterGroup? = nil

            if predecessorIdx < 0 {
                // Dropped to very top of the list — first scene of the first chapter.
                targetGroup = rows.compactMap {
                    if case .chapterHeader(let g) = $0 { return g } else { return nil }
                }.first
                afterSceneID = ""
            } else {
                let prevRow = rows[min(predecessorIdx, rows.count - 1)]
                switch prevRow {
                case .chapterHeader(let g):
                    // Landing right after a chapter header = first scene of that chapter.
                    targetGroup = g
                    afterSceneID = ""
                case .scene(let prevEntry, let prevGroup):
                    targetGroup = prevGroup
                    afterSceneID = prevEntry.sceneID
                }
            }

            guard let tg = targetGroup else { return }

            performReorderScene(
                sceneID: entry.sceneID,
                sourceChapterID: sourceGroup.chapterID,
                targetGroup: tg,
                afterSceneID: afterSceneID
            )

        case .chapterHeader(let group):
            var afterChapterID = ""

            if predecessorIdx < 0 {
                afterChapterID = ""
            } else {
                let prevRow = rows[min(predecessorIdx, rows.count - 1)]
                switch prevRow {
                case .chapterHeader(let g):
                    afterChapterID = g.chapterID
                case .scene(_, let prevGroup):
                    afterChapterID = prevGroup.chapterID
                }
            }

            performReorderChapter(chapterID: group.chapterID, afterChapterID: afterChapterID)
        }
    }

    // MARK: — Reorder scene

    private func performReorderScene(
        sceneID: String,
        sourceChapterID: String,
        targetGroup: ChapterGroup,
        afterSceneID: String
    ) {
        guard let projectRootPath = env.projectRootPath else { return }
        loader.reorderScene(
            sceneID: sceneID,
            targetChapterID: targetGroup.chapterID,
            afterSceneID: afterSceneID,
            targetChapterMetadataPath: targetGroup.chapterMetadataPath,
            targetChapterTitle: targetGroup.chapterTitle
        )
        Task { @MainActor in
            do {
                _ = try env.engine.reorderScene(
                    projectRootPath: projectRootPath,
                    sceneID: sceneID,
                    sourceChapterID: sourceChapterID,
                    targetChapterID: targetGroup.chapterID,
                    afterSceneID: afterSceneID
                )
            } catch let e as ScriviError {
                alertError = e.message
            } catch {
                alertError = error.localizedDescription
            }
        }
    }

    // MARK: — Reorder chapter

    private func performReorderChapter(chapterID: String, afterChapterID: String) {
        guard let projectRootPath = env.projectRootPath else { return }
        loader.reorderChapter(chapterID: chapterID, afterChapterID: afterChapterID)
        Task { @MainActor in
            do {
                _ = try env.engine.reorderChapter(
                    projectRootPath: projectRootPath,
                    chapterID: chapterID,
                    afterChapterID: afterChapterID
                )
            } catch let e as ScriviError {
                alertError = e.message
            } catch {
                alertError = error.localizedDescription
            }
        }
    }

    // MARK: — Helpers

    private func truncated(_ title: String, limit: Int = 30) -> String {
        guard title.count > limit else { return title }
        return title.prefix(limit) + "\u{2026}"
    }

    // MARK: — Data model

    struct SceneEntry: Identifiable {
        var id: String { sceneID }
        let sceneID: String
        let title: String
        let metadataPath: String
    }

    struct ChapterGroup: Identifiable {
        var id: String { chapterID }
        let chapterID: String
        let chapterTitle: String
        let chapterMetadataPath: String
        var scenes: [SceneEntry]
    }

    // Flat row type for the single ForEach that enables onMove across chapter boundaries.
    enum FlatRow {
        case chapterHeader(ChapterGroup)
        case scene(SceneEntry, ChapterGroup)

        var rowID: String {
            switch self {
            case .chapterHeader(let g): return "header-\(g.chapterID)"
            case .scene(let e, _):      return "scene-\(e.sceneID)"
            }
        }
    }

    enum RenameTarget: Identifiable {
        case scene(SceneEntry)
        case chapter(ChapterGroup)

        var id: String {
            switch self {
            case .scene(let e):   return "scene-\(e.sceneID)"
            case .chapter(let g): return "chapter-\(g.chapterID)"
            }
        }

        var currentTitle: String {
            switch self {
            case .scene(let e):   return e.title
            case .chapter(let g): return g.chapterTitle
            }
        }

        var promptLabel: String {
            switch self {
            case .scene:   return "Scene Name"
            case .chapter: return "Chapter Name"
            }
        }
    }

    // Flat array of rows used by the List. Chapter header rows interleaved with scene rows.
    private var flatRows: [FlatRow] {
        var rows: [FlatRow] = []
        for group in chapterGroups {
            rows.append(.chapterHeader(group))
            for entry in group.scenes {
                rows.append(.scene(entry, group))
            }
        }
        return rows
    }

    private var chapterGroups: [ChapterGroup] {
        var groups: [ChapterGroup] = []
        var chapterOrdinal = 0
        var sceneOrdinal = 0

        for info in loader.allScenes {
            sceneOrdinal += 1

            let title: String
            if !info.title.trimmingCharacters(in: .whitespaces).isEmpty {
                title = info.title
            } else {
                let liveText = loader.liveTitles[info.sceneID] ?? ""
                title = liveText.trimmingCharacters(in: .whitespaces).isEmpty
                    ? "Scene \(sceneOrdinal)"
                    : liveText
            }

            let entry = SceneEntry(
                sceneID: info.sceneID,
                title: title,
                metadataPath: info.metadataPath
            )

            if let lastIdx = groups.indices.last,
               groups[lastIdx].chapterID == info.chapterID {
                groups[lastIdx].scenes.append(entry)
            } else {
                chapterOrdinal += 1
                let chapterTitle = info.chapterTitle.trimmingCharacters(in: .whitespaces).isEmpty
                    ? "Chapter \(chapterOrdinal)"
                    : info.chapterTitle
                groups.append(ChapterGroup(
                    chapterID: info.chapterID,
                    chapterTitle: chapterTitle,
                    chapterMetadataPath: info.chapterMetadataPath,
                    scenes: [entry]
                ))
            }
        }
        return groups
    }
}

// MARK: — Scene row view

private struct NavigatorSceneRow: View {
    let title: String
    let isActive: Bool

    var body: some View {
        Text(title)
            .font(.callout)
            .lineLimit(1)
            .truncationMode(.tail)
            .foregroundStyle(isActive ? .primary : .secondary)
            .padding(.vertical, 2)
    }
}

// MARK: — Rename sheet

private struct RenameSheet: View {
    let target: SceneNavigatorView.RenameTarget
    let onSave: (String) -> Void
    let onCancel: () -> Void

    @State private var text: String = ""
    @FocusState private var focused: Bool

    var body: some View {
        NavigationStack {
            Form {
                TextField(target.promptLabel, text: $text)
                    .focused($focused)
                    .onSubmit { onSave(text) }
            }
            .navigationTitle("Rename")
            #if os(macOS)
            .padding()
            #endif
            .toolbar {
                ToolbarItem(placement: .cancellationAction) {
                    Button("Cancel") { onCancel() }
                }
                ToolbarItem(placement: .confirmationAction) {
                    Button("Save") { onSave(text) }
                }
            }
        }
        .onAppear {
            text = target.currentTitle
            focused = true
        }
        #if os(macOS)
        .frame(minWidth: 320, minHeight: 120)
        #endif
    }
}
