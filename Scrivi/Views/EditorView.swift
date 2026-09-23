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

    // Detail-column presence for the two-column NavigationSplitView selection contract. On compact
    // width a non-nil value pushes the detail; on regular width both columns show side by side.
    //
    // ⚠️ SP-134 / T-0543 (EP-040 AC4): this was `#if os(iOS)` ONLY, and that is why the Scene
    // Navigator was the one pane a writer could not get back ([I-0203]). The Inspector and the
    // Timeline are plain `Bool`s this app owns, so the View menu could always toggle them; the
    // Navigator's visibility belonged to SwiftUI and the app could neither READ nor SET it.
    // Binding it here is what lets a toolbar control restore the sidebar at all.
    //
    // ⚠️ `.automatic` is deliberate — it is what SwiftUI applied implicitly before this binding
    // existed, so introducing the binding does NOT change first-open behaviour. ⛔ `.all` would
    // FORCE the sidebar open and `.detailOnly` would hide it, either of which would be a
    // behaviour change smuggled in under a structural sprint.
    @State private var columnVisibility: NavigationSplitViewVisibility = .automatic

    // MARK: — Object Detail Sheet (EP-034 SP-117, D1-E)
    //
    // ⚠️ Hosted HERE, at editor level, not in the inspector. D1-C (pushing into
    // the 280pt inspector pane) was rejected precisely because that width is
    // wrong for long-form notes, and D1-B (a window) was deferred because Scrivi
    // has no auxiliary window type and EP-018 documents what adding one costs.
    //
    // ⚠️ The history is owned HERE rather than by the pane, so it survives the
    // pane being rebuilt — and so a window host could own it identically when
    // D1-B's successor arrives.
    //
    // ⚠️ Declared OUTSIDE the iOS-only block above: the first draft put it
    // inside, so it did not exist on macOS at all — the platform the sheet
    // actually ships on this sprint.
    @State private var detailHistory = ObjectDetailHistory()
    /// I-0155 — bumped when the Detail Sheet saves, so the Scene Inspector's cards
    /// re-read and a rename does not leave two names on screen for one object.
    @State private var objectRevision = 0
    /// I-0161 — set when navigation comes from another surface, so the Scene
    /// Navigator scrolls the target into view. ⚠️ Not set for the writer's own
    /// clicks in the navigator, which must not move the list (I-0132).
    @State private var revealRequest: SceneRevealRequest?
    @State private var revealToken = 0
    /// I-0168 — a Scene Inspector navigation awaiting the sheet's unsaved-edits
    /// guard. Nil except between the request and the sheet consuming it.
    @State private var pendingDetailNavigation: ObjectDetailHistory.Entry?
    @State private var showDetailSheet = false

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
        // T-0523 / [I-0207](b): async, same as the activation path — this runs on
        // every editor appearance and must not block the window showing itself.
        .task { await session.worldWarning.reloadAsync(
            engine: env.engine,
            projectRootPath: session.projectRootPath ?? "") }
        #if os(macOS)
        .onReceive(NotificationCenter.default.publisher(
            for: NSApplication.didBecomeActiveNotification)) { _ in
            // ⚠️ I-0123: RE-ACQUIRE world access first, then re-report. Reloading the
            // warning alone would only restate "unavailable" for a drive that is
            // physically back — the sandbox grant has to be re-taken before ScriviCore
            // can read the package at all. `reconnectWorlds` does both.
            //
            // ⚠️ I-0207: this view is PER WINDOW but `reconnectWorlds` loops EVERY
            // session, so N windows ran N sweeps over N sessions on one activation —
            // measured at 81% of the main thread and a 1–3 s beachball per click.
            // `coalescedReconnectWorlds` collapses a burst to one run.
            // ⚠️ Do NOT call `env.reconnectWorlds()` directly from an observer.
            env.coalescedReconnectWorlds()
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
            // I-0207: same per-window duplication as didBecomeActive above.
            env.coalescedReconnectWorlds()
        }
        .onReceive(NSWorkspace.shared.notificationCenter.publisher(
            for: NSWorkspace.didUnmountNotification)) { _ in
            env.coalescedReconnectWorlds()
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
                selection: $selectedSceneID,
                // I-0161: only scenes reached from elsewhere are revealed.
                revealRequest: revealRequest
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
        // ⚠️ SP-134 (EP-040 AC4): `columnVisibility` is BOUND on macOS as of T-0543. Before
        // that this call site had no binding at all, so when the sidebar went away the app
        // had no state to read and nothing to set — which is [I-0203]: "the Scene Navigator
        // hid itself and I couldn't see it." ⛔ Do not drop the binding.
        NavigationSplitView(columnVisibility: $columnVisibility) {
            SceneNavigatorView(
                loader: loader,
                env: env,
                session: session,
                prefs: prefs,
                onTakeFocus: { loader.takeFocus() },
                selection: $selectedSceneID,
                // ⚠️ I-0161: this is the macOS branch — the one actually running on
                // this platform. The first fix passed `revealRequest` to the iOS
                // call site ONLY, so on macOS it stayed `nil`, the reveal code never
                // ran, and two rounds of diagnosis chased a behaviour change that
                // was never in the build.
                revealRequest: revealRequest
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
            // T-0531 DIAGNOSTIC — the navigator click ENTERS here. `[SCRIVI-NAV]` in
            // ManuscriptTextView is the END of the chain; anything between the two is
            // SwiftUI's update pass.
            NSLog("[SCRIVI-CLICK] selection onChange -> navigateToSceneID=%@", id as NSString)
            navigateToSceneID = id
            loader.takeFocus()
            NSLog("[SCRIVI-CLICK] onChange returned (takeFocus queued)")
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
                // ⚠️ T-0548: THE MANUSCRIPT ITSELF — the view that SHOULD absorb
                // whatever width is left. [I-0245] was about a SECOND claimant beside
                // it, never about this one.
                .frame(maxWidth: .infinity, maxHeight: .infinity)  // layout-ok: the manuscript absorbs the remaining width; it IS the axis, not a claimant on it
            }

            // ⚠️ T-0545 / [I-0203] — THE TIMELINE IS A `safeAreaBar` TOO.
            //
            // ⛔ It was a `VStack` sibling of the manuscript, so showing it SHRANK the text by
            // taking stack space. ✅ As a bar it INSETS instead — the manuscript keeps its
            // layout and simply has less visible height.
            //
            // ⚠️ INSIDE the world-warning bar (attached after this one, so nearer the content):
            // the Timeline is FURNITURE a writer leaves on, the warning is TRANSIENT. Q2 ruled
            // the transient bar outermost so its appearance moves the fewest stable surfaces.
            //
            // ⚠️ Its visibility remains its OWN (`session.timelineVisible`) and is deliberately
            // NOT tied to the warning strip (SP-102 R1). Converting both to bars must not couple
            // them, and AC4 exists to check that.
            .safeAreaBar(edge: .bottom) {
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
                    // ⚠️ I-0209 — set SELECTION + REVEAL, not the one-shot trigger.
                    //
                    // Writing `navigateToSceneID` directly scrolls the MANUSCRIPT but
                    // leaves the Navigator's selection and scroll position untouched,
                    // so the writer lands in a scene the list is not showing — the
                    // same defect I-0157 fixed for the Detail Sheet, which is why
                    // that call site (below) already does it this way.
                    //
                    // I-0132 ruled selection the source of truth on both platforms;
                    // `onChange(of: selectedSceneID)` then drives the manuscript.
                    // `revealRequest` is what scrolls the list, and it is deliberately
                    // the OUTSIDE-DRIVEN reveal path (I-0161): the navigator defers
                    // it until the manuscript reports arriving, and uses `anchor: nil`
                    // so an already-visible row is not nudged (I-0132).
                    onSelectScene: { sceneID in
                        selectedSceneID = sceneID
                        revealToken += 1
                        revealRequest = SceneRevealRequest(sceneID: sceneID,
                                                           token: revealToken)
                    }
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
                    // ⚠️ I-0209 — set SELECTION + REVEAL, not the one-shot trigger.
                    //
                    // Writing `navigateToSceneID` directly scrolls the MANUSCRIPT but
                    // leaves the Navigator's selection and scroll position untouched,
                    // so the writer lands in a scene the list is not showing — the
                    // same defect I-0157 fixed for the Detail Sheet, which is why
                    // that call site (below) already does it this way.
                    //
                    // I-0132 ruled selection the source of truth on both platforms;
                    // `onChange(of: selectedSceneID)` then drives the manuscript.
                    // `revealRequest` is what scrolls the list, and it is deliberately
                    // the OUTSIDE-DRIVEN reveal path (I-0161): the navigator defers
                    // it until the manuscript reports arriving, and uses `anchor: nil`
                    // so an already-visible row is not nudged (I-0132).
                    onSelectScene: { sceneID in
                        selectedSceneID = sceneID
                        revealToken += 1
                        revealRequest = SceneRevealRequest(sceneID: sceneID,
                                                           token: revealToken)
                    }
                )
            }
            #endif
            }
            // ⚠️ T-0545 / [I-0203] — THE BANNER IS A `safeAreaBar`, NOT A STACK SIBLING.
            //
            // ⛔ IT USED TO SIT INSIDE THE VSTACK ABOVE, between the manuscript and the Timeline.
            // A sibling TAKES vertical space from the stack it joins, so the banner appearing
            // pushed on everything sharing that stack — which is why [I-0203] read as FOUR bugs
            // (Nav Bar, Inspector tab bar, Timeline and a stray title panel all moving at once)
            // rather than one structural mistake.
            //
            // ✅ A safe-area bar INSETS the content instead. The manuscript gets smaller; nothing
            // else moves.
            //
            // ⚠️ BELOW THE TIMELINE, DELIBERATELY (user ruling Q2, 2026-09-22). This REVERSES the
            // old order — the previous comment read "ABOVE the Timeline" — because a warning is
            // TRANSIENT and the Timeline is FURNITURE: putting the transient thing outermost means
            // its appearance disturbs the fewest stable surfaces, which is exactly AC2's subject.
            // ⚠️ The old note's reasoning (visible whether or not the Timeline shows) is satisfied
            // either way, and still holds: this bar has its OWN visibility (SP-102 R1) and is not
            // tied to the Timeline's.
            //
            // ⚠️ It renders only when a bound world is actually unavailable, so in normal use the
            // bar does not exist and costs no space at all.
            .safeAreaBar(edge: .bottom) {
                if session.worldWarningVisible, session.worldWarning.isVisible {
                    WorldWarningView(model: session.worldWarning) {
                        session.showWorlds = true
                    }
                }
            }
            #if os(iOS)
            if UIDevice.current.userInterfaceIdiom != .phone && session.inspectorVisible {
                inspector(loader: loader)
            }
            #endif
            // ⚠️ macOS: the Inspector is NOT an HStack member — see the `.inspector`
            // modifier applied to this HStack below (T-0546). It is a REAL trailing
            // column, which is what `.inspector(isPresented:)` presents.
            // ⛔ Do not re-add a macOS branch here.

            // ⚠️ T-0546 / [I-0245] — THE DETAIL SHEET IS A REAL SHEET, AND LIVES BELOW.
            //
            // ⛔ D1-E PUT IT HERE, AS AN `HStack` SIBLING of the manuscript — a
            // "non-modal pane beside the text". ⛔ THAT SHAPE CRASHES THE APP:
            // opening it demands more width than the window has, so the manuscript,
            // the pane and the `.inspector` column can NEVER all clear their
            // minimums. AppKit shuffles the overflow between them forever and throws
            // `NSGenericException: … more Update Constraints in Window passes than
            // there are views in the window`.
            //
            // ⛔ A FIXED `.frame(width: 520)` DID NOT FIX IT — the demand is still
            // unsatisfiable; only the jitter pattern changed. ⛔ Zeroing the minimums
            // WAS REJECTED: it resolves the arithmetic by letting the manuscript
            // collapse to nothing, and re-opens the same negotiation for every future
            // pane. ✅ A real sheet has NO width negotiation at all.
            //
            // ⛔ Do not re-add a detail pane to this HStack.
        }
        #if os(macOS)
        // ⚠️ T-0546 / [EP-040] AC6 — THE INSPECTOR IS A REAL TRAILING COLUMN.
        //
        // ⛔ IT USED TO BE AN `HStack` MEMBER with a hand-rolled resize handle: a
        // `Rectangle` with a `DragGesture` and manual `NSCursor.resizeLeftRight`
        // push/pop, plus `.frame(width:)` fed by `@AppStorage`. That is three pieces
        // of platform behaviour re-implemented by hand — the defect class this Epic
        // exists to remove, after the toolbar (SP-134) and the bars (SP-135).
        //
        // ✅ `.inspector(isPresented:)` has existed since macOS 14.0 — SIX major
        // versions below this app's deployment target, so availability was never the
        // obstacle. It presents the trailing column, owns the drag, owns the cursor,
        // and restores its own width.
        //
        // ⚠️ `isPresented` binds to the SAME `session.inspectorVisible` the View menu
        // and the toolbar already share (SP-134 AC3), so all three cannot disagree.
        // ⛔ `InspectorCommands()` was considered and DECLINED (Q3): it would add a
        // SECOND control path over state that already has one.
        // ⚠️ T-0548: the ONE `.inspector` column ([SP-136]). It is a real split-view
        // item, not an `HStack` sibling — AppKit negotiates it against the content as a
        // pair, which is exactly what D1-E's third view broke. ⛔ A SECOND one WOULD
        // contend, and this guard must fail on it.
        .inspector(isPresented: Bindable(session).inspectorVisible) {  // layout-ok: the single split-view Inspector ([SP-136]); a SECOND column would contend and must fail
            inspector(loader: loader)
                // ⚠️ Q2: the PLATFORM owns the width now. `ideal:` is seeded ONCE from
                // the retired `@AppStorage("inspectorPaneWidth")` so a writer who had
                // dragged the pane keeps her setting — ⛔ retiring the key without
                // migrating it would silently reset her, which AC2 forbids.
                // ✅ The 220/560 bounds are the ones the hand-rolled handle enforced.
                .inspectorColumnWidth(min: 220,
                                      ideal: SceneInspectorView.migratedIdealWidth,
                                      max: 560)
        }
        #endif
        // ⚠️ T-0546 / [I-0245] — THE OBJECT DETAIL SHEET, AS A REAL SHEET.
        //
        // ⛔ IT WAS AN `HStack` SIBLING (D1-E). That made three views — manuscript,
        // pane, `.inspector` column — compete for a width that could not satisfy all
        // three minimums, and AppKit crashed trying. See the note at its old site.
        //
        // ✅ A sheet is presented ABOVE the window: it takes part in no width
        // negotiation, so the ring cannot form. ⚠️ The Timeline strip sat at its floor
        // (`usable=126`, `visible=2`) for the entire failing run — proof the window had
        // no space left to give.
        //
        // ⚠️ `interactiveDismissDisabled()` — THE USER'S RULING: the sheet stays up
        // until it is dismissed DELIBERATELY (the X, Cancel, or Save), all of which run
        // `onClose`. ✅ Safe precisely because it cannot appear by accident: reaching it
        // takes a DOUBLE-CLICK on an object.
        .sheet(isPresented: $showDetailSheet) {
            ObjectDetailSheet(
                engine: env.engine,
                projectRootPath: session.projectRootPath ?? "",
                authorshipRef: env.authorshipRef ?? AuthorshipRef(identityID: "",
                                                                  personaID: "",
                                                                  displayName: ""),
                worlds: detailWorlds,
                sceneNames: detailSceneNames(loader: loader),
                // ⚠️ I-0157: set SELECTION, not the one-shot trigger.
                //
                // This wrote `navigateToSceneID` directly, which scrolls the
                // manuscript but leaves the Navigator's selection untouched —
                // so the writer arrived at a scene the Navigator was not
                // highlighting, with no way to see where she now was.
                //
                // ⚠️ **I-0132 already ruled selection the source of truth** on
                // both platforms, precisely so the two cannot drift; the
                // `onChange(of: selectedSceneID)` above then drives
                // `navigateToSceneID`. Writing the trigger directly bypassed
                // that ruling and reintroduced the split it exists to prevent.
                onSelectScene: { sceneID in
                    selectedSceneID = sceneID
                    revealToken += 1
                    revealRequest = SceneRevealRequest(sceneID: sceneID,
                                                       token: revealToken)
                },
                // I-0155: a save here must reach the inspector, which is
                // showing the same object's name a few points to the left.
                onDidSave: { objectRevision += 1 },
                projectID: session.openProjectResult?.projectID ?? "",
                // I-0162: a mount/eject must reach the sheet, whose imagePath
                // and read-only state are load-time snapshots.
                worldRevision: session.worldRevision,
                objectRevision: objectRevision,
                onClose: {
                    showDetailSheet = false
                    detailHistory.reset()
                },
                history: detailHistory,
                externalNavigation: pendingDetailNavigation,
                onExternalNavigationHandled: { pendingDetailNavigation = nil }
            )
            // ⚠️ A macOS sheet hugs its content, so it needs an explicit size or it
            // renders cramped. 520 was the pane's `idealWidth`; the height is the
            // window's usual working height.
            // ⚠️ T-0548: this sizes the SHEET, which is presented ABOVE the window and
            // takes part in NO width negotiation — the [I-0245] fix itself, and the
            // shape [SP-137] Q1 ruled intended.
            .frame(minWidth: 520, idealWidth: 620, minHeight: 520, idealHeight: 680)  // layout-ok: sizes a .sheet, presented above the window; it joins no width negotiation
        }
        // ⚠️ [I-0245] — THIS BLOCKS CLICK-OUTSIDE, AND THE SHEET GIVES ESC BACK ITSELF.
        //
        // ⛔ SwiftUI's built-in dismissal (Esc, click-outside) BYPASSES
        // `ObjectDetailSheet.requestClose()`, which is where the unsaved-changes
        // guard lives — so a stray click would discard the writer's typing with no
        // prompt. Disabling it is what keeps that guard on the only exit.
        //
        // ⚠️ **Esc IS REQUIRED** — the user's ruling: *"Escape to cancel is correct,
        // it is expected behavior and people would notice if it didn't."*
        // ✅ The sheet's own X button carries `.keyboardShortcut(.cancelAction)`, so
        // Esc closes it THROUGH the guard. ⛔ Do not re-enable interactive dismissal
        // to get Esc back — that reintroduces silent data loss on click-outside.
        //
        // ⚠️ `Cancel` and `Save` do NOT close, by design: Cancel REVERTS to the saved
        // version (T-0452) and Save keeps the sheet open to keep working.
        .interactiveDismissDisabled()
    }

    /// Writer-facing scene names for the Detail Sheet's related list (I-0151).
    ///
    /// ⚠️ **The Navigator's rule, applied to the same data** — title, else the
    /// scene's live first words, else "Scene N". ⚠️ **Never an ID**: a bare
    /// `scene_019faeb3-…` tells the writer nothing about which scene she is
    /// looking at, which is exactly what she saw for the one scene in
    /// `the-stairs-of-tintagael` with an empty title.
    ///
    /// ⚠️ Computed HERE, by the host that already owns the loader, and passed down
    /// as plain data — the pane must not reach into `ViewportSceneLoader`, or D1-E's
    /// host-independence condition (S8) is lost and a window host could not serve
    /// the same sheet later.
    private func detailSceneNames(loader: ViewportSceneLoader) -> [String: String] {
        var names: [String: String] = [:]
        for (i, info) in loader.allScenes.enumerated() {
            if !info.title.trimmingCharacters(in: .whitespaces).isEmpty {
                names[info.sceneID] = info.title
            } else {
                let live = (loader.liveTitles[info.sceneID] ?? "")
                    .trimmingCharacters(in: .whitespaces)
                names[info.sceneID] = live.isEmpty ? "Scene \(i + 1)" : live
            }
        }
        return names
    }

    /// Worlds for the Detail Sheet's read-only/pending logic (R9) and T-0440's
    /// explanation. Read once here rather than by the pane, which keeps the pane
    /// host-independent (S8) and testable with fixtures.
    private var detailWorlds: [WorldEntry] {
        guard let root = session.projectRootPath else { return [] }
        do {
            return try env.engine.listWorlds(projectRootPath: root).worlds
        } catch {
            // ⚠️ SP-130: this was `try?`, which swallowed the throw and returned
            // an empty array — and `ObjectDetailSheet.isReadOnly` reads an empty
            // `worlds` as "bound to a world this project cannot see", so EVERY
            // object silently became read-only with NO banner and no way to tell
            // a failed call from an unavailable world. The failure is now named.
            NSLog("[Scrivi] detailWorlds: listWorlds failed — every world-scoped "
                  + "object will read as read-only: \(error)")
            return []
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
                objectRevision: objectRevision,
                // I-0160: a rename in a card must reach a Detail Sheet open on the
                // same object — the mirror of the sheet's own `onDidSave`.
                onObjectChanged: { objectRevision += 1 },
                authorshipRef: env.authorshipRef,
                // T-0438 / R7: the card ASKS; the editor hosts. A card cannot
                // present an editor-level pane from inside the 280pt inspector.
                openObjectDetail: { objectID, kind, worldID, displayName in
                    let entry = ObjectDetailHistory.Entry(objectID: objectID,
                                                          kind: kind,
                                                          worldID: worldID,
                                                          displayName: displayName)
                    // ⚠️ I-0168: do NOT visit() directly when the sheet is already
                    // open — it may be holding unsaved edits, and this path
                    // bypassed T-0452's guard entirely because the guard lives in
                    // the sheet while the navigation was decided out here.
                    //
                    // Hand it to the sheet, which performs it or prompts first.
                    // ⚠️ When the sheet is CLOSED there is nothing to protect, so
                    // the original behaviour is kept.
                    if showDetailSheet {
                        pendingDetailNavigation = entry
                    } else {
                        detailHistory.visit(entry)
                        showDetailSheet = true
                    }
                },
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
