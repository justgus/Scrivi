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
