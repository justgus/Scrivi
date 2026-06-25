import SwiftUI
#if !os(macOS)
import UniformTypeIdentifiers
#endif

struct NewProjectSheet: View {

    @Environment(AppEnvironment.self) private var env
    @Environment(\.dismiss) private var dismiss

    @State private var title: String = ""
    @State private var slug: String = ""
    // macOS: full <name>.scrivi path from NSSavePanel. iOS: nil here; the picked parent
    // directory is held in `chosenDirectory` and the project folder is composed at create.
    @State private var chosenPath: String? = nil
    @State private var slugError: String? = nil
    #if !os(macOS)
    @State private var chosenDirectory: URL? = nil
    @State private var showDirImporter = false
    #endif

    // A location is chosen when macOS has a save path, or iOS has a parent directory.
    private var hasLocation: Bool {
        #if os(macOS)
        chosenPath != nil
        #else
        chosenDirectory != nil
        #endif
    }

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
                    Text(locationLabel)
                        .foregroundStyle(hasLocation ? .primary : .secondary)
                        .lineLimit(1)
                        .truncationMode(.middle)
                    Spacer()
                    Button("Choose…") {
                        #if os(macOS)
                        choosePath()
                        #else
                        showDirImporter = true
                        #endif
                    }
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
                    .disabled(title.isEmpty || !hasLocation)
            }
        }
        .padding(24)
        .frame(minWidth: 420)
        #if !os(macOS)
        // iOS/iPadOS/visionOS: pick the parent folder; the project is created as
        // <folder>/<slug>.scrivi at Create time.
        .fileImporter(
            isPresented: $showDirImporter,
            allowedContentTypes: [.folder],
            allowsMultipleSelection: false
        ) { result in
            if case .success(let urls) = result, let dir = urls.first {
                chosenDirectory = dir
            } else if case .failure(let error) = result {
                env.projectError = ScriviError(code: -1, message: error.localizedDescription)
            }
        }
        #endif
    }

    private var locationLabel: String {
        #if os(macOS)
        chosenPath.map { URL(fileURLWithPath: $0).lastPathComponent } ?? "Not chosen"
        #else
        chosenDirectory.map { "\($0.lastPathComponent)/\(slug.isEmpty ? "my-project" : slug).scrivi" } ?? "Not chosen"
        #endif
    }

    #if os(macOS)
    private func choosePath() {
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
    }
    #endif

    private func createProject() {
        slugError = nil
        let trimmedSlug = slug.trimmingCharacters(in: .whitespaces)
        guard !trimmedSlug.isEmpty else {
            slugError = "Slug cannot be empty."
            return
        }

        #if os(macOS)
        // macOS: NSSavePanel already produced the full <name>.scrivi path.
        guard let path = chosenPath else { return }
        Task {
            await env.createProject(at: path, title: title, slug: trimmedSlug)
            if env.projectError == nil { dismiss() }
        }
        #else
        // iOS: compose <picked directory>/<slug>.scrivi (handles security scope).
        guard let dir = chosenDirectory else { return }
        Task {
            await env.createProjectInPickedDirectory(dir, title: title, slug: trimmedSlug)
            if env.projectError == nil { dismiss() }
        }
        #endif
    }
}
