import SwiftUI

// SceneNavigatorView — sidebar list of all scenes in manuscript order.
//
// Chapters are non-selectable section headers derived from the ordered
// chapterID sequence in ViewportSceneLoader.allScenes.
// Scene titles: first non-empty line from liveTitles[sceneID], else "Scene X"
// (X = 1-based position in the full manuscript scene list).
// Clicking a row navigates the editor to that scene.
struct SceneNavigatorView: View {

    var loader: ViewportSceneLoader
    var env: AppEnvironment
    // Called by the parent to scroll the NSTextView to the target segment.
    var onNavigate: (String) -> Void  // sceneID

    var body: some View {
        List(selection: .constant(loader.currentSegment?.sceneID)) {
            ForEach(chapterGroups, id: \.chapterID) { group in
                Section(header: Text(group.chapterTitle).font(.caption).foregroundStyle(.secondary)) {
                    ForEach(Array(group.scenes.enumerated()), id: \.element.sceneID) { _, entry in
                        NavigatorRow(
                            title: entry.title,
                            isActive: entry.sceneID == loader.currentSegment?.sceneID
                        )
                        .contentShape(Rectangle())
                        .onTapGesture {
                            navigate(to: entry.sceneID)
                        }
                        .listRowBackground(
                            entry.sceneID == loader.currentSegment?.sceneID
                                ? Color.accentColor.opacity(0.12)
                                : Color.clear
                        )
                    }
                }
            }
        }
        .listStyle(.sidebar)
        .frame(minWidth: 180, idealWidth: 220, maxWidth: 280)
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

    // MARK: — Data preparation

    private struct SceneEntry {
        let sceneID: String
        let title: String
    }

    private struct ChapterGroup {
        let chapterID: String
        let chapterTitle: String
        var scenes: [SceneEntry]
    }

    // Build chapter groups in manuscript order, with titles derived from
    // liveTitles or "Scene X" (1-based across full manuscript).
    private var chapterGroups: [ChapterGroup] {
        var groups: [ChapterGroup] = []
        var chapterOrdinal = 0
        var sceneOrdinal = 0  // 1-based across full manuscript

        for info in loader.allScenes {
            sceneOrdinal += 1

            // Resolve title: live first-line text → "Scene X"
            let liveText = loader.liveTitles[info.sceneID] ?? ""
            let title = liveText.trimmingCharacters(in: .whitespaces).isEmpty
                ? "Scene \(sceneOrdinal)"
                : liveText

            let entry = SceneEntry(sceneID: info.sceneID, title: title)

            if let lastIdx = groups.indices.last,
               groups[lastIdx].chapterID == info.chapterID {
                groups[lastIdx].scenes.append(entry)
            } else {
                chapterOrdinal += 1
                groups.append(ChapterGroup(
                    chapterID: info.chapterID,
                    chapterTitle: "Chapter \(chapterOrdinal)",
                    scenes: [entry]
                ))
            }
        }
        return groups
    }
}

// MARK: — Row

private struct NavigatorRow: View {
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
