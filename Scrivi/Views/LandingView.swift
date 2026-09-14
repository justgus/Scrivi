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

            // T-0523 / AC6 — the load's progress bar.
            //
            // ⚠️ **[I-0199] IS WHY IT IS HERE.** Linux put the equivalent bar inside
            // the editor shell, which was NOT the visible page until the load
            // finished — so it rendered where nobody could see it, at any project
            // size. On Apple the editor WINDOW does not exist during a load (every
            // caller awaits `loadProject` and only then calls `requestOpenWindow`),
            // so this landing window is the surface actually on screen.
            if let progress = env.loadProgress, progress.isVisible {
                VStack(spacing: 8) {
                    Text("Opening \(progress.projectName)…")
                        .font(.callout)
                    // Determinate whenever the scene count is known (AC6). Before the
                    // count arrives it is indeterminate rather than a false 0%.
                    if progress.sceneCount > 0 {
                        ProgressView(value: progress.fraction)
                            .progressViewStyle(.linear)
                        Text("\(progress.scenesLoaded) of \(progress.sceneCount) scenes")
                            .font(.caption)
                            .foregroundStyle(.secondary)
                    } else {
                        ProgressView().progressViewStyle(.linear)
                    }
                }
                .frame(maxWidth: 320)
                .transition(.opacity)
            }

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
        #if !os(macOS)
        // File ▸ Open Project… (iPad menu) sets this; raise the importer and clear the request.
        .onChange(of: env.requestOpenImporter) { _, requested in
            if requested {
                showOpenImporter = true
                env.requestOpenImporter = false
            }
        }
        #endif
        .onAppear {
            if env.requestNewProject {
                showNewProjectSheet = true
                env.requestNewProject = false
            }
            #if !os(macOS)
            if env.requestOpenImporter {
                showOpenImporter = true
                env.requestOpenImporter = false
            }
            #endif
        }
    }

}
