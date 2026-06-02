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
                    openProject()
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
    }

    private func openProject() {
        let panel = NSOpenPanel()
        panel.canChooseFiles = false
        panel.canChooseDirectories = true
        panel.allowsMultipleSelection = false
        panel.title = "Open Scrivi Project"
        panel.prompt = "Open"

        guard panel.runModal() == .OK, let url = panel.url else { return }

        Task {
            await env.openProject(at: url.path(percentEncoded: false))

            if let result = env.openProjectResult, result.mode == "repairRequired",
               let issue = result.repairIssues.first {
                env.openProjectResult = nil
                env.projectError = ScriviError(code: -1, message: "Repair required: \(issue.title)")
            }
        }
    }
}
