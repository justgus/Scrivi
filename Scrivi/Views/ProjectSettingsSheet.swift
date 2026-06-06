import SwiftUI

struct ProjectSettingsSheet: View {

    @Binding var showChapterTitles: Bool
    @Environment(\.dismiss) private var dismiss

    var body: some View {
        NavigationStack {
            Form {
                Section("Writing Surface") {
                    Toggle("Show chapter titles in manuscript", isOn: $showChapterTitles)
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
        .frame(minWidth: 360, minHeight: 160)
        #endif
    }
}
