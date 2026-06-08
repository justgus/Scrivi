import SwiftUI

@main
struct ScriviApp: App {

    @State private var env = AppEnvironment()

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
        }
        .defaultSize(width: 1100, height: 700)
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
