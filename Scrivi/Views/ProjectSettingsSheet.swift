import SwiftUI

struct ProjectSettingsSheet: View {

    var prefs: ProjectPreferences
    @Environment(\.dismiss) private var dismiss

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
            }
            .navigationTitle("Project Settings")
            #if os(macOS)
            .padding()
            #endif
            .toolbar {
                ToolbarItem(placement: .confirmationAction) {
                    Button("Done") { dismiss() }
                }
            }
        }
        #if os(macOS)
        .frame(minWidth: 380, minHeight: 220)
        #endif
    }
}
