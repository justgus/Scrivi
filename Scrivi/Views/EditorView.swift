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

    @State private var navigateToSceneID: String? = nil

    var body: some View {
        NavigationSplitView {
            SceneNavigatorView(loader: loader, env: env, prefs: prefs) { sceneID in
                navigateToSceneID = sceneID
            } onTakeFocus: {
                loader.takeFocus()
            }
        } detail: {
            VStack(spacing: 0) {
                #if os(iOS)
                if UIDevice.current.userInterfaceIdiom == .phone {
                    phoneToolbar
                    Divider()
                }
                #endif
                ManuscriptTextView(
                    loader: loader,
                    env: env,
                    navigateToSceneID: $navigateToSceneID,
                    showChapterTitles: prefs.showChapterTitles
                )
                .frame(maxWidth: .infinity, maxHeight: .infinity)
            }
            .navigationTitle(prefs.projectTitle.trimmingCharacters(in: .whitespaces).isEmpty
                             ? "Untitled" : prefs.projectTitle)
            .navigationSubtitle(prefs.projectSubtitle)
        }
        .frame(minWidth: 700, minHeight: 400)
        .navigationSplitViewStyle(.balanced)
    }

    #if os(iOS)
    private var phoneToolbar: some View {
        HStack {
            Spacer()
            Button("Project Settings") { env.showProjectSettings = true }
                .buttonStyle(.borderless)
                .font(.callout)
            Button("Close Project") { env.closeProject() }
                .buttonStyle(.borderless)
                .font(.callout)
        }
        .padding(.horizontal, 12)
        .padding(.vertical, 8)
    }
    #endif
}
