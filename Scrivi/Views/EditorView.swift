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

    // The scene selected in the Master (navigator) — **the source of truth for which scene
    // is current**, on every platform. Bound into the navigator's List selection, so the
    // table that backs it writes here directly, and the manuscript follows from `onChange`.
    // nil until a default is chosen on appear (restored viewport scene, else the first).
    //
    // ⚠️ **Shared by macOS as of I-0132 (2026-08-18).** macOS previously had no selection
    // binding and pushed navigation through a tap-gesture callback into a one-shot
    // trigger — parallel state that could silently disagree with the navigator, which is
    // what produced the intermittent "click did nothing" and lost-focus reports. On iOS it
    // additionally drives the Detail push on compact width.
    @State private var selectedSceneID: String? = nil

    #if os(iOS)
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
        // Worlds manager (T-0408). Not platform-gated: the Worlds menu is macOS
        // today, but nothing here is macOS-specific except the file panels.
        .sheet(isPresented: Bindable(session).showWorlds) {
            if let root = session.projectRootPath {
                WorldsView(engine: env.engine, projectRootPath: root,
                           onWorldsChanged: { env.reconnectWorlds() })
            }
        }
        // I-0128: every world mutation (add, remove, relink, create) happens inside
        // the Worlds sheet, so its dismissal is the one place that covers them all —
        // the cards re-read rather than showing a world the writer just unbound.
        .onChange(of: session.showWorlds) { _, isShowing in
            if !isShowing { session.bumpWorldRevision() }
        }
        // Forward a deep link's requested scene into local navigation, then clear
        // it. .onAppear covers a cold-start link set before this view existed;
        // .onChange covers a link that arrives while the editor is already shown.
        .onAppear { consumePendingNavigation() }
        .onChange(of: session.pendingNavigationSceneID) { _, _ in consumePendingNavigation() }
        // §7.3: refresh the pending report when the editor appears, and again when
        // the writer returns to this window.
        //
        // ⚠️ A world can become unavailable WHILE the project is open — that is the
        // whole AC23 scenario: the drive is ejected under a running app. Re-checking
        // on foreground is what makes the strip appear then, rather than only at
        // open. T-0415 exercises exactly this path.
        .onAppear { session.worldWarning.reload(engine: env.engine,
                                                projectRootPath: session.projectRootPath ?? "") }
        #if os(macOS)
        .onReceive(NotificationCenter.default.publisher(
            for: NSApplication.didBecomeActiveNotification)) { _ in
            // ⚠️ I-0123: RE-ACQUIRE world access first, then re-report. Reloading the
            // warning alone would only restate "unavailable" for a drive that is
            // physically back — the sandbox grant has to be re-taken before ScriviCore
            // can read the package at all. `reconnectWorlds` does both.
            env.reconnectWorlds()
        }
        // ⚠️ I-0129: focus is NOT the event we actually care about — mounting is.
        //
        // Foregrounding was the only trigger, which worked solely because ejecting a
        // drive normally requires leaving the app. Plug a drive in while Scrivi is
        // already frontmost and nothing fired: the world came back and the warning
        // stayed up until some unrelated focus change happened to refresh it.
        //
        // `NSWorkspace` reports mount/unmount directly, so this is exact and
        // immediate rather than polled — no timer, no idle wakeups, and it fires
        // whether or not Scrivi has focus. The volume-level event is the right
        // granularity: a world package cannot appear or vanish without one.
        .onReceive(NSWorkspace.shared.notificationCenter.publisher(
            for: NSWorkspace.didMountNotification)) { _ in
            env.reconnectWorlds()
        }
        .onReceive(NSWorkspace.shared.notificationCenter.publisher(
            for: NSWorkspace.didUnmountNotification)) { _ in
            env.reconnectWorlds()
        }
        #endif
        #if os(macOS)
        // The buffers palette is app-global (owned by AppEnvironment, follows the
        // frontmost project). Once this editor's BufferService has installed its
        // paste/load handlers (makeNSView), re-sync so a palette opened before this
        // window's coordinator existed re-points at the now-ready service.
        .onAppear { env.syncBuffersPalette() }
        #endif
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
        // ⚠️ **macOS uses the same selection-as-source-of-truth shape as iOS above
        // (I-0132, 2026-08-18).** It previously passed navigation through a tap-gesture
        // callback with no `selection` binding, which made it the only platform where the
        // navigator's state and the manuscript's state were separate things kept in step
        // by hand — and the only platform with the intermittent click failures.
        //
        // The failure was NOT gesture arbitration, which is what three earlier attempts
        // assumed. `navigateToSceneID` is a **one-shot trigger** cleared asynchronously
        // after use, so re-selecting the same scene wrote an unchanged value (SwiftUI
        // coalesces it away, no `updateNSView`, no navigation, no focus transfer), and a
        // fast second click could be clobbered by the previous click's pending `nil`.
        // Selection is durable state, so it survives both cases.
        NavigationSplitView {
            SceneNavigatorView(
                loader: loader,
                env: env,
                session: session,
                prefs: prefs,
                onTakeFocus: { loader.takeFocus() },
                selection: $selectedSceneID
            )
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
        // Same two rules as iOS, now that macOS is selection-driven too.
        .onAppear { selectDefaultSceneIfNeeded() }
        // Selecting a scene in the master scrolls the manuscript to it, places the caret
        // at its first character (§3) and hands focus over. Firing on *change* means a
        // click on the already-selected scene is correctly a no-op rather than a re-scroll.
        .onChange(of: selectedSceneID) { _, newValue in
            guard let id = newValue else { return }
            // ⚠️ **Loop break, two layers (I-0132).** The navigator and the manuscript form
            // a deliberate cycle: scroll → `setViewportScene` → navigator mirrors it into
            // this selection → here → navigate the manuscript → scroll. Unbroken, a scroll
            // re-scrolls itself and steals focus mid-gesture.
            //
            // 1. **Suspended notification (primary).** The loader raises a flag while it is
            //    pushing a viewport change out to the navigator, so this write is *known*
            //    to be an echo — no inference about who wrote first.
            // 2. **Value equality (backstop).** Also skip when the selection merely equals
            //    the viewport. On its own this was fragile: it assumed the scroll handler
            //    had already written `viewportSceneID`, and would loop if that order ever
            //    inverted. Kept as a second line, not the mechanism.
            guard !loader.isMirroringViewportToSelection else { return }
            guard id != loader.viewportSceneID else { return }
            navigateToSceneID = id
            loader.takeFocus()
        }
        #endif
    }

    // On open, seed the selection so the navigator shows the current scene (and, on iOS,
    // so the detail has content): prefer the restored viewport scene, else the first.
    //
    // ⚠️ Seeding it to the ALREADY-restored scene is what keeps this from double-navigating
    // at launch — `onChange` fires, sees `id == loader.viewportSceneID`, and returns.
    private func selectDefaultSceneIfNeeded() {
        guard selectedSceneID == nil else { return }
        selectedSceneID = loader.viewportSceneID ?? loader.allScenes.first?.sceneID
    }

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

                // §7.3: the project-wide pending report. Sits directly under the
                // manuscript and ABOVE the Timeline, so it is visible whether or not
                // the Timeline is shown (SP-102 R1 — it has its own visibility).
                //
                // It renders only when a bound world is actually unavailable, so in
                // normal use this strip does not exist and costs no vertical space.
                if session.worldWarningVisible, session.worldWarning.isVisible {
                    WorldWarningView(model: session.worldWarning) {
                        session.showWorlds = true
                    }
                }
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
                inspector(loader: loader)
            }
            #else
            if session.inspectorVisible {
                inspector(loader: loader)
            }
            #endif
        }
    }

    /// The Scene Inspector (EP-030 SP-090). The card stack is per-scene, so it follows
    /// the viewport scene — the same one the Navigator highlights. The selected TAB does
    /// not follow the scene (Doc 2 §4.7); that lives in the layout store.
    @ViewBuilder
    private func inspector(loader: ViewportSceneLoader) -> some View {
        if let layout = session.inspectorLayout {
            SceneInspectorView(
                sceneID: loader.viewportSceneID ?? loader.segments.first?.sceneID,
                projectRootPath: session.projectRootPath,
                engine: env.engine,
                allSceneIDs: loader.segments.map(\.sceneID),
                history: session.historyCapture,
                caretByteOffset: loader.cursorByteOffset,
                // I-0128: reconnecting a world bumps this, which re-keys each card's
                // `.task(id:)` so pending entries relink without a scene change.
                worldRevision: session.worldRevision,
                authorshipRef: env.authorshipRef,
                layout: layout
            )
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
                Toggle("Show World Warnings", isOn: Bindable(session).worldWarningVisible)
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
