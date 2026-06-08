import SwiftUI
import AppKit

struct EditorView: View {

    @Environment(AppEnvironment.self) private var env

    var body: some View {
        if let loader = env.viewportLoader, let prefs = env.projectPreferences {
            ManuscriptEditorView(loader: loader, prefs: prefs)
        } else {
            ProgressView("Loading…")
                .frame(maxWidth: .infinity, maxHeight: .infinity)
        }
    }
}

// MARK: — Manuscript editor shell

private struct ManuscriptEditorView: View {

    @Environment(AppEnvironment.self) private var env
    var loader: ViewportSceneLoader
    var prefs: ProjectPreferences

    // Scene ID to scroll to — set by Navigator on tap and after delete.
    @State private var navigateToSceneID: String? = nil
    @State private var showProjectSettings: Bool = false

    var body: some View {
        NavigationSplitView {
            SceneNavigatorView(loader: loader, env: env, prefs: prefs) { sceneID in
                navigateToSceneID = sceneID
            } onTakeFocus: {
                loader.takeFocus()
            }
        } detail: {
            VStack(spacing: 0) {
                toolbar
                Divider()
                ManuscriptTextView(
                    loader: loader,
                    env: env,
                    navigateToSceneID: $navigateToSceneID,
                    showChapterTitles: prefs.showChapterTitles
                )
                .frame(maxWidth: .infinity, maxHeight: .infinity)
            }
        }
        .frame(minWidth: 700, minHeight: 400)
        .navigationSplitViewStyle(.balanced)
        .sheet(isPresented: $showProjectSettings) {
            ProjectSettingsSheet(prefs: prefs)
        }
    }

    private var toolbar: some View {
        HStack {
            Spacer()
            Button("Project Settings") { showProjectSettings = true }
                .buttonStyle(.borderless)
                .font(.callout)
            Button("Close Project") {
                env.openProjectResult = nil
                env.projectRootPath = nil
                env.projectError = nil
                env.viewportLoader = nil
                env.projectPreferences = nil
            }
        }
        .padding(.horizontal, 12)
        .padding(.vertical, 8)
    }
}
