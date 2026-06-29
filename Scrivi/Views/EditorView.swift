import SwiftUI
#if os(macOS)
import AppKit
#endif

struct EditorView: View {

    @Environment(ProjectSession.self) private var session

    var body: some View {
        if let loader = session.viewportLoader, let prefs = session.projectPreferences {
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
    @Environment(ProjectSession.self) private var session
    var loader: ViewportSceneLoader
    var prefs: ProjectPreferences

    @State private var navigateToSceneID: String? = nil

    #if os(iOS)
    // The scene selected in the Master (navigator). Drives both the Detail push on compact width
    // (iPhone portrait) and the scroll-to-scene in the continuous manuscript. nil until a default
    // is chosen on appear (restored viewport scene, else the first scene). Bound into the
    // navigator's List selection so master and detail stay in sync, and so NavigationSplitView's
    // two-column form triggers the detail column when an item is selected.
    @State private var selectedSceneID: String? = nil
    // Detail-column presence for the two-column NavigationSplitView selection contract. On compact
    // width a non-nil value pushes the detail; on regular width both columns show side by side.
    @State private var columnVisibility: NavigationSplitViewVisibility = .automatic
    #endif

    var body: some View {
        container
        #if os(iOS)
        .sheet(isPresented: Bindable(session).showProjectSettings) {
            ProjectSettingsSheet(prefs: prefs)
        }
        #endif
        // Forward a deep link's requested scene into local navigation, then clear
        // it. .onAppear covers a cold-start link set before this view existed;
        // .onChange covers a link that arrives while the editor is already shown.
        .onAppear { consumePendingNavigation() }
        .onChange(of: session.pendingNavigationSceneID) { _, _ in consumePendingNavigation() }
    }

    // MARK: — Platform container

    @ViewBuilder
    private var container: some View {
        #if os(iOS)
        // Single Master/Detail layout for both iPhone and iPad. NavigationSplitView adapts to
        // idiom and orientation on its own: side-by-side on iPad and iPhone landscape (regular
        // width), and a navigator→manuscript push with a back chevron on iPhone portrait (compact
        // width). The selection binding is what makes the detail track the master and drives the
        // compact-width push.
        NavigationSplitView(columnVisibility: $columnVisibility) {
            SceneNavigatorView(
                loader: loader,
                env: env,
                session: session,
                prefs: prefs,
                onTakeFocus: { loader.takeFocus() },
                selection: $selectedSceneID
            )
            .navigationTitle(projectTitle)
            .navigationBarTitleDisplayMode(.inline)
        } detail: {
            manuscriptDetail
                .navigationTitle(projectTitle)
                .navigationSubtitle(prefs.projectSubtitle)
                .navigationBarTitleDisplayMode(.inline)
                .toolbar { editorMenuToolbar }
        }
        .navigationSplitViewStyle(.balanced)
        // Choose the initial detail scene: the restored viewport scene if any, else the first scene.
        .onAppear { selectDefaultSceneIfNeeded() }
        // Selecting a scene in the master scrolls the continuous manuscript to it.
        .onChange(of: selectedSceneID) { _, newValue in
            if let id = newValue { navigateToSceneID = id }
        }
        #else
        NavigationSplitView {
            SceneNavigatorView(loader: loader, env: env, session: session, prefs: prefs) { sceneID in
                navigateToSceneID = sceneID
            } onTakeFocus: {
                loader.takeFocus()
            }
        } detail: {
#if os(visionOS)
#else
            manuscriptDetail
                .navigationTitle(projectTitle)
                .navigationSubtitle(prefs.projectSubtitle)
#endif
        }
        .frame(minWidth: 700, minHeight: 400)
        .navigationSplitViewStyle(.balanced)
        #endif
    }

    #if os(iOS)
    // On open, seed the Master/Detail selection so the detail has content: prefer the restored
    // viewport scene (we already persist/restore it), otherwise the first scene (chapter one).
    private func selectDefaultSceneIfNeeded() {
        guard selectedSceneID == nil else { return }
        selectedSceneID = loader.viewportSceneID ?? loader.allScenes.first?.sceneID
    }
    #endif

    // MARK: — Detail (manuscript + optional timeline/inspector)

    @ViewBuilder
    private var manuscriptDetail: some View {
        HStack(spacing: 0) {
            VStack(spacing: 0) {
                ManuscriptTextView(
                    loader: loader,
                    env: env,
                    session: session,
                    navigateToSceneID: $navigateToSceneID,
                    showChapterTitles: prefs.showChapterTitles
                )
                .frame(maxWidth: .infinity, maxHeight: .infinity)
                #if os(iOS)
                if UIDevice.current.userInterfaceIdiom != .phone,
                   session.timelineVisible,
                   let tlModel = session.timelineModel,
                   let prp = session.projectRootPath {
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
                if session.timelineVisible,
                   let tlModel = session.timelineModel,
                   let prp = session.projectRootPath {
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
            if UIDevice.current.userInterfaceIdiom != .phone && session.inspectorVisible {
                SceneInspectorView()
            }
            #else
            if session.inspectorVisible {
                SceneInspectorView()
            }
            #endif
        }
    }

    private var projectTitle: String {
        prefs.projectTitle.trimmingCharacters(in: .whitespaces).isEmpty
            ? "Untitled" : prefs.projectTitle
    }

    #if os(iOS)
    // iOS has no menu bar on iPhone, so the project actions live on the editor's navigation bar.
    @ToolbarContentBuilder
    private var editorMenuToolbar: some ToolbarContent {
        ToolbarItem(placement: .primaryAction) {
            Menu {
                Button("Project Settings…") { session.showProjectSettings = true }
                Toggle("Show Scene Inspector", isOn: Bindable(session).inspectorVisible)
                Toggle("Show Timeline", isOn: Bindable(session).timelineVisible)
                Divider()
                Button("Close Project", role: .destructive) {
                    if let pid = session.openProjectResult?.projectID {
                        env.closeProject(projectID: pid)
                    }
                }
            } label: {
                Label("Actions", systemImage: "ellipsis.circle")
            }
        }
    }
    #endif

    private func consumePendingNavigation() {
        guard let sceneID = session.pendingNavigationSceneID else { return }
        navigateToSceneID = sceneID
        session.pendingNavigationSceneID = nil
    }

}
