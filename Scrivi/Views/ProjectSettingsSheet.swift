import SwiftUI

struct ProjectSettingsSheet: View {

    var prefs: ProjectPreferences
    @Environment(\.dismiss) private var dismiss
    @Environment(AppEnvironment.self) private var env
    @Environment(ProjectSession.self) private var session

    // Undo-history capacity (Trade T1), loaded from the engine on appear.
    @State private var historyCapacity: Int = 20000
    // Stale-branch age threshold (§5, T-0212); a branch untouched this many days
    // is offered for purge. Loaded from the engine on appear.
    @State private var staleBranchDays: Int = 7
    @State private var historyLoaded = false

    var body: some View {
        NavigationStack {
            Form {
                Section("Project") {
                    LabeledContent("Title") {
                        TextField("Title", text: Bindable(prefs).projectTitle, prompt: Text("Untitled"))
                            .multilineTextAlignment(.trailing)
                            .labelsHidden()
                    }
                    LabeledContent("Subtitle") {
                        TextField("Subtitle", text: Bindable(prefs).projectSubtitle, prompt: Text("Optional"))
                            .multilineTextAlignment(.trailing)
                            .labelsHidden()
                    }
                }
                Section("Writing Surface") {
                    Toggle("Show chapter titles in manuscript", isOn: Bindable(prefs).showChapterTitles)
                }
                Section("Undo History") {
                    LabeledContent("Maximum undo events") {
                        TextField("Maximum undo events", value: $historyCapacity, format: .number)
                            .multilineTextAlignment(.trailing)
                            .labelsHidden()
                            #if os(macOS)
                            .frame(width: 100)
                            #endif
                    }
                    Text("Oldest edits fall off once this limit is reached. The current text is always kept.")
                        .font(.caption)
                        .foregroundStyle(.secondary)
                    LabeledContent("Stale after (days)") {
                        TextField("Stale after (days)", value: $staleBranchDays, format: .number)
                            .multilineTextAlignment(.trailing)
                            .labelsHidden()
                            #if os(macOS)
                            .frame(width: 100)
                            #endif
                    }
                    Text("Abandoned branches untouched for this many days can be purged from Project ▸ Purge Stale History Branches….")
                        .font(.caption)
                        .foregroundStyle(.secondary)
                }

            }
            // I-0254: the default macOS form style (`.columns`) needed ~574 pt for these
            // labels but the sheet opened at its 380 pt minimum, clipping them. Grouped
            // rows put label and value on one line at a width the sheet can hold.
            // ⚠️ `.labelsHidden()` on each field: a TextField inside LabeledContent
            // otherwise draws its title as a SECOND label beside the row's own.
            .formStyle(.grouped)
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
        .frame(minWidth: 460, idealWidth: 460, minHeight: 300)
        #endif
    }

    private func loadHistorySettings() {
        guard let root = session.projectRootPath else { return }
        if let s = try? env.engine.historyGetSettings(projectRootPath: root) {
            historyCapacity = s.capacityEvents
            staleBranchDays = s.staleBranchDays
            historyLoaded = true
        }
    }

    private func saveHistorySettings() {
        guard historyLoaded, let root = session.projectRootPath else { return }
        let capacity = max(0, historyCapacity)
        // Preserve idleRolloverHours; capacity and stale threshold are user-editable here.
        let current = try? env.engine.historyGetSettings(projectRootPath: root)
        _ = try? env.engine.historySetSettings(
            projectRootPath: root,
            capacityEvents: capacity,
            staleBranchDays: max(0, staleBranchDays),
            idleRolloverHours: current?.idleRolloverHours ?? 8)
    }
}
