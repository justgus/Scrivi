import SwiftUI
import UniformTypeIdentifiers

struct LandingView: View {

    @Environment(AppEnvironment.self) private var env
    @State private var showNewProjectSheet = false
    @State private var showErrorAlert = false
    #if !os(macOS)
    @State private var showOpenImporter = false
    // The .scrivi package UTI (declared in Info.plist), with a folder/package fallback.
    private var scriviProjectType: UTType {
        UTType("com.caposoft.scrivi.project") ?? .package
    }
    #endif

    var body: some View {
        VStack(spacing: 24) {
            Text("Scrivi")
                .font(.largeTitle)
                .fontWeight(.bold)

            HStack(spacing: 16) {
                Button("Open Project…") {
                    #if os(macOS)
                    env.presentOpenProjectPanel()
                    #else
                    showOpenImporter = true
                    #endif
                }
                .keyboardShortcut("o", modifiers: .command)

                Button("New Project…") {
                    showNewProjectSheet = true
                }
                .keyboardShortcut("n", modifiers: .command)
            }
            .buttonStyle(.borderedProminent)

            if let err = env.projectError {
                Text(err.message)
                    .foregroundStyle(.red)
                    .font(.callout)
                    .multilineTextAlignment(.center)
                    .padding(.horizontal)
            }
        }
        .frame(minWidth: 400, minHeight: 300)
        .padding(40)
        .sheet(isPresented: $showNewProjectSheet) {
            NewProjectSheet()
                .environment(env)
        }
        #if !os(macOS)
        // iOS/iPadOS/visionOS: pick the .scrivi package via the document browser.
        .fileImporter(
            isPresented: $showOpenImporter,
            allowedContentTypes: [scriviProjectType],
            allowsMultipleSelection: false
        ) { result in
            switch result {
            case .success(let urls):
                if let url = urls.first {
                    Task { await env.openProjectFromPickedURL(url) }
                }
            case .failure(let error):
                env.projectError = ScriviError(code: -1, message: error.localizedDescription)
            }
        }
        #endif
        // File ▸ New Project… (menu) sets this; present the sheet and clear the request.
        .onChange(of: env.requestNewProject) { _, requested in
            if requested {
                showNewProjectSheet = true
                env.requestNewProject = false
            }
        }
        .onAppear {
            if env.requestNewProject {
                showNewProjectSheet = true
                env.requestNewProject = false
            }
        }
    }

}
