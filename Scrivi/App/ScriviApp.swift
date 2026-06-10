import SwiftUI

@main
struct ScriviApp: App {

    @State private var env = AppEnvironment()
    @State private var showAbout = false

    var body: some Scene {
        WindowGroup {
            ContentRootView()
                .environment(env)
                .task {
                    await env.bootstrap()
                }
                .onReceive(
                    NotificationCenter.default.publisher(
                        for: NSApplication.willResignActiveNotification
                    )
                ) { _ in
                    Task { await env.onAppResign() }
                }
                .background(WindowFrameAutosave())
                .sheet(isPresented: $showAbout) {
                    AboutView()
                }
                .sheet(isPresented: Bindable(env).showProjectSettings) {
                    if let prefs = env.projectPreferences {
                        ProjectSettingsSheet(prefs: prefs)
                    }
                }
        }
        .defaultSize(width: 1100, height: 700)
        .commands {
            // Replace the system File menu with a Project menu.
            CommandGroup(replacing: .newItem) { }
            CommandMenu("Project") {
                Button("Open Project…") {
                    env.presentOpenProjectPanel()
                }
                .keyboardShortcut("o", modifiers: .command)

                Divider()

                Button("Project Settings…") {
                    env.showProjectSettings = true
                }
                .keyboardShortcut(",", modifiers: .command)
                .disabled(env.openProjectResult == nil)

                Divider()

                Button("Close Project") {
                    env.closeProject()
                }
                .keyboardShortcut("w", modifiers: .command)
                .disabled(env.openProjectResult == nil)
            }

            // View menu.
            CommandMenu("View") {
                Toggle("Show Scene Inspector", isOn: Bindable(env).inspectorVisible)
                    .keyboardShortcut("i", modifiers: [.command, .option])
                    .disabled(env.openProjectResult == nil)
                Toggle("Show Timeline", isOn: Bindable(env).timelineVisible)
                    .keyboardShortcut("t", modifiers: [.command, .option])
                    .disabled(env.openProjectResult == nil)
            }

            // About menu.
            CommandGroup(replacing: .appInfo) {
                Button("About Scrivi") {
                    showAbout = true
                }
            }
            CommandGroup(after: .appInfo) {
                Button("User Manual") { }
                    .disabled(true)
            }
        }
    }
}

private struct ContentRootView: View {
    @Environment(AppEnvironment.self) private var env

    var body: some View {
        if env.openProjectResult != nil {
            EditorView()
        } else {
            LandingView()
        }
    }
}
