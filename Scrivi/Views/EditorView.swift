import SwiftUI
#if os(macOS)
import AppKit
#endif

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
            HStack(spacing: 0) {
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
                    #if os(iOS)
                    if UIDevice.current.userInterfaceIdiom != .phone,
                       env.timelineVisible,
                       let tlModel = env.timelineModel,
                       let prp = env.projectRootPath {
                        TimelineStripView(
                            model: tlModel,
                            engine: env.engine,
                            projectRootPath: prp,
                            authorshipRef: env.authorshipRef,
                            loader: loader,
                            onSelectScene: { sceneID in navigateToSceneID = sceneID }
                        )
                    }
                    #else
                    if env.timelineVisible,
                       let tlModel = env.timelineModel,
                       let prp = env.projectRootPath {
                        TimelineStripView(
                            model: tlModel,
                            engine: env.engine,
                            projectRootPath: prp,
                            authorshipRef: env.authorshipRef,
                            loader: loader,
                            onSelectScene: { sceneID in navigateToSceneID = sceneID }
                        )
                    }
                    #endif
                }
                #if os(iOS)
                if UIDevice.current.userInterfaceIdiom != .phone && env.inspectorVisible {
                    SceneInspectorView()
                }
                #else
                if env.inspectorVisible {
                    SceneInspectorView()
                }
                #endif
            }
            .navigationTitle(prefs.projectTitle.trimmingCharacters(in: .whitespaces).isEmpty
                             ? "Untitled" : prefs.projectTitle)
            #if os(macOS) || os(iOS)
            .navigationSubtitle(prefs.projectSubtitle)
            #endif
        }
        .frame(minWidth: 700, minHeight: 400)
        .navigationSplitViewStyle(.balanced)
        // Forward a deep link's requested scene into local navigation, then clear
        // it. .onAppear covers a cold-start link set before this view existed;
        // .onChange covers a link that arrives while the editor is already shown.
        .onAppear { consumePendingNavigation() }
        .onChange(of: env.pendingNavigationSceneID) { _, _ in consumePendingNavigation() }
    }

    private func consumePendingNavigation() {
        guard let sceneID = env.pendingNavigationSceneID else { return }
        navigateToSceneID = sceneID
        env.pendingNavigationSceneID = nil
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
