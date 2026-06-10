import SwiftUI

enum InspectorTab: String, CaseIterable, Identifiable {
    case entities = "Scene Entities"
    var id: String { rawValue }
}

struct SceneInspectorView: View {

    @State private var selectedTab: InspectorTab = .entities

    var body: some View {
        Divider()
        VStack(spacing: 0) {
            Picker("", selection: $selectedTab) {
                ForEach(InspectorTab.allCases) { tab in
                    Text(tab.rawValue).tag(tab)
                }
            }
            .pickerStyle(.segmented)
            .padding(.horizontal, 10)
            .padding(.vertical, 8)

            Divider()

            switch selectedTab {
            case .entities:
                SceneEntitiesTabView()
            }
        }
        .frame(minWidth: 240, idealWidth: 280, maxWidth: 280)
    }
}

// MARK: — Scene Entities stub tab

private struct SceneEntitiesTabView: View {
    var body: some View {
        VStack(spacing: 12) {
            Text("Scene Entities")
                .font(.headline)
                .frame(maxWidth: .infinity, alignment: .leading)
                .padding(.horizontal, 12)
                .padding(.top, 12)

            Spacer()

            Text("No entities yet.")
                .foregroundStyle(.secondary)

            Button("Add Entity") { }
                .disabled(true)

            Spacer()
        }
    }
}
