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
                // ✅ EP-047 S2 (P2, P5a, P8): the manuscript's type — a BUNDLED face and a size, per project (it travels).
                Section("Typography") {
                    Picker("Typeface", selection: Bindable(prefs).typeface) {
                        ForEach(BundledFonts.faces) { face in
                            Text(face.name).font(.custom(face.name, size: 14)).tag(face.name)
                        }
                        // A face from a later Scrivi stays selected (and stored) — drawn in the default meanwhile.
                        if !BundledFonts.faces.contains(where: { $0.name == prefs.typeface }) {
                            Text("\(prefs.typeface) (not in this version)").tag(prefs.typeface)
                        }
                    }
                    Stepper(value: Bindable(prefs).textSize,
                            in: Double(ManuscriptTypography.sizeRange.lowerBound)...Double(ManuscriptTypography.sizeRange.upperBound),
                            step: 1) {
                        LabeledContent("Text size", value: "\(Int(prefs.textSize)) pt")
                    }
                    // ✅ EP-047 S3 (P3, P10): drawn, never typed — zero characters in the manuscript.
                    Picker("Paragraph indent", selection: Bindable(prefs).paragraphIndent) {
                        Text("Book convention").tag(ManuscriptTypography.ParagraphIndent.book.rawValue)
                        Text("Every paragraph").tag(ManuscriptTypography.ParagraphIndent.every.rawValue)
                        Text("None").tag(ManuscriptTypography.ParagraphIndent.none.rawValue)
                    }
                    Stepper(value: Bindable(prefs).indentEm,
                            in: Double(ManuscriptTypography.indentEmRange.lowerBound)...Double(ManuscriptTypography.indentEmRange.upperBound),
                            step: 0.5) {
                        LabeledContent("Indent", value: String(format: "%.1f em", prefs.indentEm))
                    }
                    .disabled(prefs.paragraphIndent == ManuscriptTypography.ParagraphIndent.none.rawValue)
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
