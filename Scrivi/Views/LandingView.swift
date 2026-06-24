import SwiftUI

struct LandingView: View {

    @Environment(AppEnvironment.self) private var env
    @State private var showNewProjectSheet = false
    @State private var showErrorAlert = false

    var body: some View {
        VStack(spacing: 24) {
            Text("Scrivi")
                .font(.largeTitle)
                .fontWeight(.bold)

            HStack(spacing: 16) {
                Button("Open Project…") {
                    env.presentOpenProjectPanel()
                }
                .keyboardShortcut("o", modifiers: .command)

                Button("New Project…") {
                    showNewProjectSheet = true
                }
                .keyboardShortcut("n", modifiers: .command)
            }
            .buttonStyle(.borderedProminent)

            if let err = env.projectError {
                Text(err.message)
                    .foregroundStyle(.red)
                    .font(.callout)
                    .multilineTextAlignment(.center)
                    .padding(.horizontal)
            }
        }
        .frame(minWidth: 400, minHeight: 300)
        .padding(40)
        .sheet(isPresented: $showNewProjectSheet) {
            NewProjectSheet()
                .environment(env)
        }
        // File ▸ New Project… (menu) sets this; present the sheet and clear the request.
        .onChange(of: env.requestNewProject) { _, requested in
            if requested {
                showNewProjectSheet = true
                env.requestNewProject = false
            }
        }
        .onAppear {
            if env.requestNewProject {
                showNewProjectSheet = true
                env.requestNewProject = false
            }
        }
    }

}
