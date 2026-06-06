import SwiftUI
import AppKit

struct EditorView: View {

    @Environment(AppEnvironment.self) private var env

    var body: some View {
        if let loader = env.viewportLoader {
            ManuscriptEditorView(loader: loader)
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

    // Scene ID to scroll to, set by the Navigator on tap or after delete.
    @State private var navigateToSceneID: String? = nil
    // Set to true by the Navigator after a delete to steal focus back to the text view.
    @State private var focusManuscriptView: Bool = false
    // Controls whether chapter titles render as headings in the writing surface.
    @State private var showChapterTitles: Bool = false

    var body: some View {
        NavigationSplitView {
            SceneNavigatorView(loader: loader, env: env) { sceneID in
                navigateToSceneID = sceneID
            } onDeleteNavigate: { sceneID in
                navigateToSceneID = sceneID
                focusManuscriptView = true
            }
        } detail: {
            VStack(spacing: 0) {
                toolbar
                Divider()
                ManuscriptTextView(
                    loader: loader,
                    env: env,
                    navigateToSceneID: $navigateToSceneID,
                    focusManuscriptView: $focusManuscriptView,
                    showChapterTitles: showChapterTitles
                )
                .frame(maxWidth: .infinity, maxHeight: .infinity)
            }
        }
        .frame(minWidth: 700, minHeight: 400)
        .navigationSplitViewStyle(.balanced)
    }

    @State private var showProjectSettings: Bool = false

    private var toolbar: some View {
        HStack {
            Text(env.openProjectResult?.projectID ?? "")
                .font(.caption)
                .foregroundStyle(.secondary)
                .lineLimit(1)
                .truncationMode(.middle)

            Spacer()

            Button("Project Settings") {
                showProjectSettings = true
            }
            .buttonStyle(.borderless)
            .font(.callout)

            Button("Close Project") {
                env.openProjectResult = nil
                env.projectRootPath = nil
                env.projectError = nil
                env.viewportLoader = nil
            }
        }
        .padding(.horizontal, 12)
        .padding(.vertical, 8)
        .sheet(isPresented: $showProjectSettings) {
            ProjectSettingsSheet(showChapterTitles: $showChapterTitles)
        }
    }
}
