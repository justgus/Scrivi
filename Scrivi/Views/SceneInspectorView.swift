import SwiftUI

// T-0361 — the Scene Inspector shell: bottom tabs + resizable pane.
// Design: Scrivi_Scene_Inspector_Card_Framework_v0_1.md §4.7, §C3.
//
// Replaces the SP-090 placeholder (a single segmented tab over a stub "Scene Entities"
// body at a fixed 280pt). Tabs are Writing | Worldbuilding | Properties, defaulting to
// Writing; the selection persists at PROJECT level and does NOT follow the scene —
// switching scenes reloads the current tab's cards for the new scene.

struct SceneInspectorView: View {

    let sceneID: String?
    let projectRootPath: String?
    let engine: ScriviEngine
    let allSceneIDs: [String]
    let history: HistoryCapture?
    @Bindable var layout: InspectorLayoutStore

    // Pane width is a per-device view preference (unlike the layout itself), so it
    // stays in UserDefaults rather than inspector-layout.json.
    @AppStorage("inspectorPaneWidth") private var paneWidth: Double = 300

    private static let minWidth: Double = 220
    private static let maxWidth: Double = 560

    var body: some View {
        HStack(spacing: 0) {
            resizeHandle

            VStack(spacing: 0) {
                Divider()

                selectedTabContent
                    .frame(maxWidth: .infinity, maxHeight: .infinity)

                Divider()
                tabBar
            }
        }
        .frame(width: paneWidth)
    }

    // MARK: — Tab content

    @ViewBuilder
    private var selectedTabContent: some View {
        switch layout.document.selectedTab {
        case .writing:
            InspectorCardStackView(stack: .writing,
                                   sceneID: sceneID,
                                   projectRootPath: projectRootPath,
                                   engine: engine,
                                   allSceneIDs: allSceneIDs,
                                   history: history,
                                   layout: layout)
        case .worldbuilding:
            InspectorCardStackView(stack: .worldbuilding,
                                   sceneID: sceneID,
                                   projectRootPath: projectRootPath,
                                   engine: engine,
                                   allSceneIDs: allSceneIDs,
                                   history: history,
                                   layout: layout)
        case .properties:
            ScenePropertiesView(sceneID: sceneID,
                                projectRootPath: projectRootPath,
                                engine: engine)
        }
    }

    // MARK: — Bottom tab bar

    private var tabBar: some View {
        HStack(spacing: 0) {
            ForEach(InspectorTab.allCases) { tab in
                Button {
                    layout.setSelectedTab(tab)
                } label: {
                    VStack(spacing: 2) {
                        Image(systemName: tab.systemImage)
                            .font(.system(size: 13, weight: .medium))
                        Text(tab.title)
                            .font(.system(size: 10))
                            .lineLimit(1)
                            .minimumScaleFactor(0.8)
                    }
                    .frame(maxWidth: .infinity)
                    .padding(.vertical, 6)
                    .contentShape(Rectangle())
                    .foregroundStyle(layout.document.selectedTab == tab ? Color.accentColor : .secondary)
                }
                .buttonStyle(.plain)
                .help(tab.title)
            }
        }
        .padding(.horizontal, 4)
        .background(.bar)
    }

    // MARK: — Resize handle

    private var resizeHandle: some View {
        Rectangle()
            .fill(Color.clear)
            .frame(width: 5)
            .contentShape(Rectangle())
            .overlay(Divider(), alignment: .leading)
            #if os(macOS)
            .onHover { inside in
                if inside { NSCursor.resizeLeftRight.push() } else { NSCursor.pop() }
            }
            #endif
            .gesture(
                DragGesture(minimumDistance: 1)
                    .onChanged { value in
                        // Dragging the LEFT edge: moving left widens the pane.
                        let proposed = paneWidth - value.translation.width
                        paneWidth = min(max(proposed, Self.minWidth), Self.maxWidth)
                    }
            )
    }
}
