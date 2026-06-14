import SwiftUI

struct NewProjectSheet: View {

    @Environment(AppEnvironment.self) private var env
    @Environment(\.dismiss) private var dismiss

    @State private var title: String = ""
    @State private var slug: String = ""
    @State private var chosenPath: String? = nil
    @State private var slugError: String? = nil

    var body: some View {
        VStack(alignment: .leading, spacing: 20) {
            Text("New Project")
                .font(.title2)
                .fontWeight(.semibold)

            LabeledContent("Title") {
                TextField("My Novel", text: $title)
                    .onChange(of: title) { _, newValue in
                        slug = newValue
                            .lowercased()
                            .replacingOccurrences(of: " ", with: "-")
                            .filter { $0.isLetter || $0.isNumber || $0 == "-" }
                    }
            }

            LabeledContent("Slug") {
                TextField("my-novel", text: $slug)
                if let err = slugError {
                    Text(err).foregroundStyle(.red).font(.caption)
                }
            }

            LabeledContent("Location") {
                HStack {
                    Text(chosenPath.map { URL(fileURLWithPath: $0).lastPathComponent } ?? "Not chosen")
                        .foregroundStyle(chosenPath == nil ? .secondary : .primary)
                        .lineLimit(1)
                        .truncationMode(.middle)
                    Spacer()
                    Button("Choose…") { choosePath() }
                }
            }

            if let err = env.projectError {
                Text(err.message)
                    .foregroundStyle(.red)
                    .font(.callout)
            }

            HStack {
                Spacer()
                Button("Cancel", role: .cancel) { dismiss() }
                Button("Create") { createProject() }
                    .keyboardShortcut(.defaultAction)
                    .disabled(title.isEmpty || chosenPath == nil)
            }
        }
        .padding(24)
        .frame(minWidth: 420)
    }

    private func choosePath() {
        #if os(macOS)
        let panel = NSSavePanel()
        panel.title = "Choose Project Location"
        panel.nameFieldLabel = "Project folder:"
        panel.nameFieldStringValue = slug.isEmpty ? "my-project" : slug
        panel.canCreateDirectories = true
        panel.prompt = "Choose"

        guard panel.runModal() == .OK, let url = panel.url else { return }
        chosenPath = url.path(percentEncoded: false)
        if slug.isEmpty {
            slug = url.deletingPathExtension().lastPathComponent
        }
        #endif
    }

    private func createProject() {
        slugError = nil
        let trimmedSlug = slug.trimmingCharacters(in: .whitespaces)
        guard !trimmedSlug.isEmpty else {
            slugError = "Slug cannot be empty."
            return
        }
        guard let path = chosenPath else { return }

        Task {
            await env.createProject(at: path, title: title, slug: trimmedSlug)
            if env.projectError == nil {
                dismiss()
            }
        }
    }
}
