#if os(macOS)
@preconcurrency import AppKit
import SwiftUI
import os

// ProjectWindowManager — owns the AppKit NSWindow for each open project (EP-018 / T-0194).
//
// Why AppKit instead of SwiftUI WindowGroup(for:): WindowGroup(for:) caches dismissed
// windows and reopening a closed project rebinds to a dead view (no .task/.onAppear), so
// reopen hung on "Loading…". Managing NSWindows directly gives deterministic lifecycle:
// create on open, focus on R3, fully close+release on close. One window per ProjectSession.
//
// Keyed by projectID. The Welcome/Landing surface stays a SwiftUI Window; only project
// editor windows are AppKit-managed.
@MainActor
final class ProjectWindowManager {

    private let log = Logger(subsystem: "com.caposoft.scrivi", category: "Windows")

    // projectID → its window controller.
    private var controllers: [String: ProjectWindowController] = [:]

    var openProjectIDs: [String] { Array(controllers.keys) }
    var isEmpty: Bool { controllers.isEmpty }

    // Opens a new window for `session`, or focuses the existing one for its project (R3).
    func openOrFocus(session: ProjectSession, env: AppEnvironment) {
        guard let projectID = session.openProjectResult?.projectID else {
            log.error("openOrFocus: session has no projectID")
            return
        }
        if let existing = controllers[projectID] {
            log.notice("focus existing window for \(projectID, privacy: .public)")
            existing.showAndFocus()
            return
        }
        log.notice("create window for \(projectID, privacy: .public)")
        let controller = ProjectWindowController(session: session, env: env) { [weak self] in
            // onClose: window closed by the user (red button or ⌘W) — tear down.
            self?.handleWindowClosed(projectID: projectID, env: env)
        }
        controllers[projectID] = controller
        controller.showAndFocus()
    }

    // Closes the window for a project (called by the Close Project menu command). The
    // window's close handler does the session teardown.
    func closeWindow(projectID: String) {
        controllers[projectID]?.close()
    }

    private func handleWindowClosed(projectID: String, env: AppEnvironment) {
        log.notice("window closed for \(projectID, privacy: .public)")
        controllers.removeValue(forKey: projectID)
        env.didCloseProjectWindow(projectID: projectID)
    }
}

// Holds a NotificationCenter observer token so a one-shot @Sendable observer closure can
// remove itself. @unchecked Sendable is sound: the token is only written after addObserver
// returns and read back on the main queue where the notification is delivered.
private final class ObserverTokenBox: @unchecked Sendable {
    var token: NSObjectProtocol?
}

// One NSWindow hosting a project's SwiftUI EditorView.
@MainActor
final class ProjectWindowController: NSObject, NSWindowDelegate {

    private let window: NSWindow
    private let projectID: String
    private let onClose: () -> Void

    private let session: ProjectSession
    private weak var env: AppEnvironment?

    init(session: ProjectSession, env: AppEnvironment, onClose: @escaping () -> Void) {
        self.session = session
        self.env = env
        self.projectID = session.openProjectResult?.projectID ?? ""
        self.onClose = onClose

        let root = ProjectWindowContent(session: session)
            .environment(env)
            .environment(session)

        window = NSWindow(
            contentRect: NSRect(x: 0, y: 0, width: 1100, height: 700),
            styleMask: [.titled, .closable, .miniaturizable, .resizable],
            backing: .buffered,
            defer: false
        )
        window.contentView = NSHostingView(rootView: root)
        window.isReleasedWhenClosed = false

        // ⚠️ T-0544 / [I-0243] — WINDOWS ONLY, NEVER TABS (user ruling 2026-09-22).
        //
        // ⛔ `tabbingMode` was NEVER SET, so macOS's default `.automatic` applied — and that
        // default tabs windows IN FULL SCREEN but not outside it. The observable result was one
        // command doing two different things: "Open Project" made a NEW WINDOW when windowed and
        // a NEW TAB when full-screen. ⚠️ Nobody chose that; it was the platform showing through.
        //
        // ✅ WHY WINDOWS: a tab bar teaches "these are peers within one workspace", which is what
        // a MULTI-document app looks like. The app-shape study ruled Scrivi a SINGLE-DOCUMENT
        // editor — one `NSTextView` holding the whole manuscript; a scene is a structural unit of
        // the document, not a document. One manuscript per window says that honestly.
        //
        // ⚠️ THIS IS NOT A REVERSAL OF [EP-018]. "One manuscript" and "one window" are different
        // claims: EP-018 deliberately delivered multiple PROJECTS, each in its own window. Tabs
        // were a THIRD way of showing multiple projects, arriving by accident beside the two that
        // were designed.
        //
        // ✅ It also removes the tab header — which was rendering the project title a THIRD time.
        window.tabbingMode = .disallowed

        // Restore this project's last size/position instead of defaulting (I-0051). A saved
        // frame is clamped back on-screen in case its display is gone. With no saved frame
        // (first-ever open), center then cascade so concurrently-opened windows don't stack
        // exactly on top of one another.
        if let saved = ProjectWindowFrameStore.savedFrame(projectID: projectID) {
            window.setFrame(ProjectWindowFrameStore.clampedOnscreen(saved), display: false)
        } else {
            window.center()
            // cascadeTopLeft(from:) places the window and returns the next anchor point.
            ProjectWindowController.lastCascadePoint =
                window.cascadeTopLeft(from: ProjectWindowController.lastCascadePoint)
        }

        let title = session.projectPreferences?.projectTitle ?? "Scrivi"
        window.title = title.trimmingCharacters(in: .whitespaces).isEmpty ? "Untitled" : title

        super.init()
        window.delegate = self

        // The saved windowed frame is applied above (setFrame) and is the size the window returns to
        // when it leaves full screen. If the project was last quit in macOS Full Screen, restore
        // that state after the window is shown (showAndFocus) via the proper toggleFullScreen API —
        // NOT a geometry hack (a manually screen-sized window is NOT full screen) and NOT
        // window.zoom(nil) (unreliable). I-0055.
        restoreFullScreenOnShow = ProjectWindowFrameStore.savedFullScreen(projectID: projectID)
    }

    // Set in init from the persisted per-project full-screen flag; consumed once in showAndFocus().
    private var restoreFullScreenOnShow = false

    // Shared cascade anchor so successive first-open windows step down/right instead of
    // landing on the same centered point. AppKit advances and returns the next point.
    private static var lastCascadePoint = NSPoint.zero

    func showAndFocus() {
        window.makeKeyAndOrderFront(nil)
        NSApp.activate(ignoringOtherApps: true)

        // Restore macOS Full Screen. toggleFullScreen drives the real full-screen transition
        // (menu-bar hide, own Space); the windowDidEnterFullScreen delegate persists the flag and
        // the windowed frame (applied in init) is never overwritten while full screen (I-0055).
        if restoreFullScreenOnShow {
            restoreFullScreenOnShow = false
            // A programmatic toggleFullScreen fired DURING app launch stalls mid-flight —
            // windowWillEnterFullScreen arrives but windowDidEnterFullScreen never does, so the
            // window is left windowed (I-0097). Manual (green-button) full screen works because
            // the app is already fully active. So defer the restore until the app is active AND
            // the launch storm has settled: wait for NSApplication.didBecomeActive (or run now if
            // already active), then a short delay before toggling.
            if NSApp.isActive {
                scheduleFullScreenAfterSettle()
            } else {
                // The block-observer closure is @Sendable, so it can capture only Sendable state:
                // sf (weak, hopped back onto the main actor) and a boxed token so the one-shot
                // observer can remove itself. Delivery is already on .main, so assumeIsolated is
                // sound.
                let box = ObserverTokenBox()
                box.token = NotificationCenter.default.addObserver(
                    forName: NSApplication.didBecomeActiveNotification,
                    object: nil, queue: .main
                ) { [weak self] _ in
                    MainActor.assumeIsolated {
                        if let token = box.token { NotificationCenter.default.removeObserver(token) }
                        self?.scheduleFullScreenAfterSettle()
                    }
                }
            }
        }
    }

    // Wait out the launch storm, then toggle into full screen if not already there (I-0097).
    private func scheduleFullScreenAfterSettle() {
        DispatchQueue.main.asyncAfter(deadline: .now() + 0.35) { [weak self] in
            guard let self, !self.window.styleMask.contains(.fullScreen) else { return }
            self.window.toggleFullScreen(nil)
        }
    }

    func close() {
        window.close()
    }

    // NSWindowDelegate: fires for every close path (red button, ⌘W, window.close()).
    func windowWillClose(_ notification: Notification) {
        // Persist final size/position before teardown so reopening this project restores it
        // (I-0051). Quit-time also routes through here as each window closes.
        ProjectWindowFrameStore.save(window: window, projectID: projectID)
        onClose()
    }

    // Persist frame on user resize/move so a crash or force-quit still restores recent layout.
    func windowDidEndLiveResize(_ notification: Notification) {
        // A full-screen transition emits live-resize end callbacks with the styleMask flipping
        // mid-flight; saving here would persist the transient state (fs=1 then fs=0) and could
        // clobber the real value. Suppress during the transition, exactly like windowDidResize
        // (the ent/ exit did-callbacks persist the settled state). I-0097.
        guard !isTransitioningFullScreen else { return }
        ProjectWindowFrameStore.save(window: window, projectID: projectID)
    }

    func windowDidMove(_ notification: Notification) {
        ProjectWindowFrameStore.save(window: window, projectID: projectID)
    }

    // Live drag-resize and tiling (quadrant/half/third) end here; persist the new windowed frame.
    // Suppressed during a full-screen transition so the transient full-screen resize does not
    // overwrite the saved windowed frame (save() also guards via styleMask, this avoids the churn).
    func windowDidResize(_ notification: Notification) {
        guard !isTransitioningFullScreen else { return }
        ProjectWindowFrameStore.save(window: window, projectID: projectID)
    }

    // MARK: Full-screen transitions (the green-button "maximize" path)

    // True from willEnter/willExit until the matching did-callback, so windowDidResize does not
    // persist the transient full-screen geometry as the windowed frame.
    private var isTransitioningFullScreen = false

    func windowWillEnterFullScreen(_ notification: Notification) {
        isTransitioningFullScreen = true
    }

    func windowDidEnterFullScreen(_ notification: Notification) {
        isTransitioningFullScreen = false
        // Persist the full-screen flag; save() leaves the windowed frame untouched while full screen.
        ProjectWindowFrameStore.save(window: window, projectID: projectID)
    }

    func windowWillExitFullScreen(_ notification: Notification) {
        isTransitioningFullScreen = true
    }

    func windowDidExitFullScreen(_ notification: Notification) {
        isTransitioningFullScreen = false
        // Back to windowed: record fullScreen=false and the restored windowed frame.
        ProjectWindowFrameStore.save(window: window, projectID: projectID)
    }

    // Frontmost tracking — drives the menu bar's enabled state and per-window toggles
    // (AppKit windows don't feed SwiftUI's @FocusedValue, so we track focus explicitly).
    func windowDidBecomeKey(_ notification: Notification) {
        env?.frontmostSession = session
    }
}

// SwiftUI content hosted in the AppKit project window. Mirrors what ContentRoot did:
// the editor plus the Project Settings sheet, with the session as a FocusedValue so the
// menu bar acts on the frontmost project window.
private struct ProjectWindowContent: View {
    let session: ProjectSession

    // ⚠️ SP-134: the toolbar's Buffers toggle binds here, not to the session. The buffers
    // palette is APP-GLOBAL — one floating panel follows the frontmost project
    // (ProjectSession.swift:48-50) — so its visibility lives on AppEnvironment, exactly as the
    // View menu's "Show Buffers" binds it. `env` is already injected at window creation.
    @Environment(AppEnvironment.self) private var env

    var body: some View {
        EditorView()
            .sheet(isPresented: Bindable(session).showProjectSettings) {
                if let prefs = session.projectPreferences {
                    ProjectSettingsSheet(prefs: prefs)
                }
            }
            .frame(minWidth: 700, minHeight: 400)
            .toolbar { projectToolbar }
    }

    // MARK: — The window toolbar (EP-040 SP-134 / T-0543, app-shape study S1+S2)
    //
    // ⚠️ WHY IT IS DECLARED HERE, IN SWIFTUI, AND NOT AS AN AppKit `NSToolbar` (ruled 2026-09-22).
    // The project window IS an AppKit `NSWindow`, so an `NSToolbar` was the obvious-looking
    // choice. ⛔ It is the wrong one: `ScriviApp.swift` records that "AppKit NSWindows don't feed
    // SwiftUI's @FocusedValue", which is why the MENU BAR has to reach the session through
    // `env.frontmostSession`. An `NSToolbar` would need that same workaround for EVERY item — a
    // second hand-maintained path to state this view already holds directly.
    // ✅ Declared here, `session` is simply in scope.
    //
    // ⚠️ EVERY ITEM CALLS AN EXISTING CLOSURE. Nothing here re-implements a verb; the toolbar is a
    // SURFACE for what the menu bar already does (EP-040 AC2), and the two can never disagree
    // because there is one code path.
    @ToolbarContentBuilder
    private var projectToolbar: some ToolbarContent {
        // ⚠️ GROUPED PER MENU, AND THE GROUPING IS FUNCTIONAL — NOT COSMETIC (user ruling,
        // live pass 2026-09-22).
        //
        // ✅ `ControlGroup` is what lets macOS COLLAPSE a group into a single popup control when
        // the window is too narrow to show its members. ⛔ `ToolbarItemGroup` ALONE does not do
        // that — it is layout adjacency only, which is what the first draft used, and at narrow
        // widths its items would simply drop into the generic overflow menu instead of staying
        // together as "Scene" and "Chapter".
        //
        // ⚠️ SO THE NESTING MATTERS: `ToolbarItemGroup { ControlGroup { … } label: { … } }`.
        // The label is what the collapsed popup is titled with, which is why each one names its
        // MENU — a writer who loses the buttons still finds "Scene" and "Chapter".
        //
        // ⚠️ Worlds and Project Settings are DELIBERATELY FREE-STANDING (user ruling): they are
        // project-level surfaces, not members of a verb family, and collapsing them under a
        // shared label would invent a grouping the menu bar does not have.

        // Scene — mirrors the Scene menu, in its order (ScriviApp.swift:202-218).
        ToolbarItemGroup {
            ControlGroup {
                Button { session.createSceneAction?() } label: {
                    Label("New Scene", systemImage: "plus.rectangle")
                }
                .disabled(session.createSceneAction == nil)
                .help("New Scene (⌘↩)")

                Button { session.mergeSceneAction?() } label: {
                    Label("Merge Scene", systemImage: "arrow.triangle.merge")
                }
                .disabled(session.mergeSceneAction == nil)
                .help("Merge Scene with Previous (⌘⌫)")

                Button { session.sceneStartAction?() } label: {
                    Label("Scene Start", systemImage: "arrow.up.to.line.compact")
                }
                .disabled(session.sceneStartAction == nil)
                .help("Go to Scene Start")

                Button { session.sceneEndAction?() } label: {
                    Label("Scene End", systemImage: "arrow.down.to.line.compact")
                }
                .disabled(session.sceneEndAction == nil)
                .help("Go to Scene End")
            } label: {
                Label("Scene", systemImage: "plus.rectangle")
            }
        }

        // Chapter — mirrors the Chapter menu (ScriviApp.swift:220-231).
        ToolbarItemGroup {
            ControlGroup {
                Button { session.createChapterAction?() } label: {
                    Label("New Chapter", systemImage: "plus.rectangle.on.folder")
                }
                .disabled(session.createChapterAction == nil)
                .help("New Chapter (⇧⌘↩)")

                // ⚠️ EVERY VERB HAS ITS OWN GLYPH (user ruling, live pass 2026-09-22): a writer
                // must not have to read a tooltip to tell Merge Scene from Merge Chapter.
                //
                // ✅ `rectangle.compress.vertical` — two bounded regions collapsing into one.
                // ⚠️ Chosen after RENDERING the candidates, not from their names: `arrow.merge`
                // and `arrow.trianglehead.merge` draw the SAME PICTURE as `arrow.triangle.merge`,
                // and no circled merge symbol exists at all. ⛔ `arrow.trianglehead.branch` points
                // OUTWARD — it means branch, the opposite of merge.
                // ✅ It is arguably the truer metaphor here: a chapter IS a container, and merging
                // collapses two containers into one.
                //
                // ⛔ The first draft used `arrow.triangle.merge.circle`, WHICH DOES NOT EXIST, and
                // it rendered as a BLANK BUTTON with only a tooltip. SF Symbol names are not
                // guessable; every symbol here was verified against `NSImage(systemSymbolName:)`.
                Button { session.mergeChapterAction?() } label: {
                    Label("Merge Chapter", systemImage: "rectangle.compress.vertical")
                }
                .disabled(session.mergeChapterAction == nil)
                .help("Merge Chapter with Previous (⇧⌘⌫)")

                // ✅ `.circle` distinguishes Chapter from Scene's `.compact` variant — the
                // user's "enclosed" metaphor. ⚠️ `.alt` was tried first and REJECTED on render:
                // it is NOT a double line, it draws the same as the plain symbol.
                Button { session.chapterStartAction?() } label: {
                    Label("Chapter Start", systemImage: "arrow.up.to.line.circle")
                }
                .disabled(session.chapterStartAction == nil)
                .help("Go to Chapter Start")

                Button { session.chapterEndAction?() } label: {
                    Label("Chapter End", systemImage: "arrow.down.to.line.circle")
                }
                .disabled(session.chapterEndAction == nil)
                .help("Go to Chapter End")
            } label: {
                Label("Chapter", systemImage: "plus.rectangle.on.folder")
            }
        }

        // Visibility toggles.
        //
        // ⚠️ THERE IS NO NAVIGATOR BUTTON HERE. The first draft added one and it DUPLICATED the
        // system sidebar control macOS already puts in the top-left — same icon, same behaviour.
        // ✅ The platform's control is the affordance [I-0203] wanted; what was missing was STATE
        // behind it, and T-0543's `columnVisibility` binding supplies that.
        // ⛔ Do not re-add a second one.
        ToolbarItemGroup {
            ControlGroup {
                Toggle(isOn: Bindable(session).inspectorVisible) {
                    Label("Inspector", systemImage: "sidebar.trailing")
                }
                .help("Show or hide the Scene Inspector (⌥⌘I)")

                Toggle(isOn: Bindable(session).timelineVisible) {
                    Label("Timeline", systemImage: "calendar.day.timeline.left")
                }
                .help("Show or hide the Timeline (⌥⌘T)")

                // ⚠️ Buffers visibility is APP-GLOBAL, not per-session: one floating palette
                // follows the frontmost project (ProjectSession.swift:48-50). It binds to
                // AppEnvironment, exactly as the View menu's "Show Buffers" does.
                Toggle(isOn: Bindable(env).buffersPaletteVisible) {
                    Label("Buffers", systemImage: "doc.on.clipboard")
                }
                .help("Show or hide the copy buffers (⌥⌘B)")
            } label: {
                Label("Show", systemImage: "sidebar.trailing")
            }
        }

        // Project surfaces — FREE-STANDING, one item each (user ruling).
        ToolbarItem {
            Button { session.showWorlds = true } label: {
                Label("Worlds", systemImage: "globe")
            }
            .help("Manage Worlds…")
        }

        ToolbarItem {
            Button { session.showProjectSettings = true } label: {
                Label("Project Settings", systemImage: "gearshape")
            }
            .help("Project Settings… (⌘,)")
        }

        // ⚠️ DECLARED SLOTS — EP-040 AC7 / AC5. Export and the text-size lens are what this
        // toolbar makes ROOM for; ⛔ BUILDING THEM IS OUT OF SCOPE for this Epic (app-shape §4.2:
        // "they are NEW FEATURES and should not be bundled into a structural restructure").
        //
        //   Document ▸ Export…              — no implementation exists yet
        //   View     ▸ text-size lens       — no implementation exists yet (app-shape §2.3)
    }
}

#endif
