import SwiftUI

struct ProjectSettingsSheet: View {

    var prefs: ProjectPreferences
    @Environment(\.dismiss) private var dismiss
    @Environment(AppEnvironment.self) private var env
    @Environment(ProjectSession.self) private var session

    // Undo-history capacity (Trade T1), loaded from the engine on appear.
    @State private var historyCapacity: Int = 20000
    @State private var historyLoaded = false

    var body: some View {
        NavigationStack {
            Form {
                Section("Project") {
                    LabeledContent("Title") {
                        TextField("Untitled", text: Bindable(prefs).projectTitle)
                            .multilineTextAlignment(.trailing)
                    }
                    LabeledContent("Subtitle") {
                        TextField("Optional", text: Bindable(prefs).projectSubtitle)
                            .multilineTextAlignment(.trailing)
                    }
                }
                Section("Writing Surface") {
                    Toggle("Show chapter titles in manuscript", isOn: Bindable(prefs).showChapterTitles)
                }
                Section("Undo History") {
                    LabeledContent("Maximum undo events") {
                        TextField("20000", value: $historyCapacity, format: .number)
                            .multilineTextAlignment(.trailing)
                            #if os(macOS)
                            .frame(width: 100)
                            #endif
                    }
                    Text("Oldest edits fall off once this limit is reached. The current text is always kept.")
                        .font(.caption)
                        .foregroundStyle(.secondary)
                }
            }
            .onAppear(perform: loadHistorySettings)
            .navigationTitle("Project Settings")
            #if os(macOS)
            .padding()
            #endif
            .toolbar {
                ToolbarItem(placement: .confirmationAction) {
                    Button("Done") { saveHistorySettings(); dismiss() }
                }
            }
        }
        #if os(macOS)
        .frame(minWidth: 380, minHeight: 300)
        #endif
    }

    private func loadHistorySettings() {
        guard let root = session.projectRootPath else { return }
        if let s = try? env.engine.historyGetSettings(projectRootPath: root) {
            historyCapacity = s.capacityEvents
            historyLoaded = true
        }
    }

    private func saveHistorySettings() {
        guard historyLoaded, let root = session.projectRootPath else { return }
        let capacity = max(0, historyCapacity)
        // Preserve the other settings; only capacity is user-editable here.
        let current = try? env.engine.historyGetSettings(projectRootPath: root)
        try? env.engine.historySetSettings(
            projectRootPath: root,
            capacityEvents: capacity,
            staleBranchDays: current?.staleBranchDays ?? 7,
            idleRolloverHours: current?.idleRolloverHours ?? 8)
    }
}
