import SwiftUI

// EP-047 (SP-167 Q1, ruled 2026-10-07): Project ▸ Purge Stale History Branches… — the stale-branch purge
// (T-0212) as an ACTION with its own sheet. It used to sit in Project Settings, a sheet of SETTINGS (user,
// [I-0278] review: *"Find Stale Branches (we should find another home for this)"*). The "Stale after (days)"
// threshold it detects against stays in Project Settings.
struct StaleBranchesSheet: View {

    @Environment(\.dismiss) private var dismiss
    @Environment(ProjectSession.self) private var session

    @State private var staleBranches: [HistoryStaleBranch] = []
    @State private var didScan = false
    @State private var pendingPurge: HistoryStaleBranch?

    var body: some View {
        NavigationStack {
            Form {
                Section {
                    if !didScan {
                        ProgressView()
                    } else if staleBranches.isEmpty {
                        Text("No stale branches.")
                            .foregroundStyle(.secondary)
                    } else {
                        ForEach(staleBranches, id: \.branchRootEventID) { branch in
                            HStack {
                                VStack(alignment: .leading, spacing: 2) {
                                    Text(branch.preview.isEmpty ? "(no preview)" : branch.preview)
                                        .lineLimit(1)
                                    Text(staleSubtitle(branch))
                                        .font(.caption)
                                        .foregroundStyle(.secondary)
                                }
                                Spacer()
                                Button("Purge", role: .destructive) { pendingPurge = branch }
                            }
                        }
                    }
                } footer: {
                    Text("Abandoned undo branches older than the \"Stale after\" threshold in Project Settings.")
                }
            }
            .formStyle(.grouped)
            .onAppear(perform: scan)
            .confirmationDialog(
                "Purge this branch?",
                isPresented: Binding(get: { pendingPurge != nil },
                                     set: { if !$0 { pendingPurge = nil } }),
                presenting: pendingPurge
            ) { branch in
                Button("Purge \(branch.nodeCount) step\(branch.nodeCount == 1 ? "" : "s")",
                       role: .destructive) { purge(branch) }
                Button("Cancel", role: .cancel) { pendingPurge = nil }
            } message: { _ in
                Text("This permanently discards the abandoned branch and its edits. It cannot be undone.")
            }
            .navigationTitle("Stale History Branches")
            #if os(macOS)
            .padding()
            #endif
            .toolbar {
                ToolbarItem(placement: .cancellationAction) {
                    Button("Rescan", action: scan)
                }
                ToolbarItem(placement: .confirmationAction) {
                    Button("Done") { dismiss() }
                }
            }
        }
        #if os(macOS)
        .frame(minWidth: 460, idealWidth: 460, minHeight: 260)
        #endif
    }

    private func scan() {
        staleBranches = session.historyCapture?.listStaleBranches() ?? []
        didScan = true
    }

    private func purge(_ branch: HistoryStaleBranch) {
        pendingPurge = nil
        guard session.historyCapture?.purgeStaleBranch(branchRootEventID: branch.branchRootEventID) == true
        else { return }
        staleBranches.removeAll { $0.branchRootEventID == branch.branchRootEventID }
    }

    // "3 steps · last edited on Jul 2 at 4:10 PM"
    private func staleSubtitle(_ branch: HistoryStaleBranch) -> String {
        let steps = "\(branch.nodeCount) step\(branch.nodeCount == 1 ? "" : "s")"
        if let when = HistoryTimestamp.friendly(branch.tipTimestamp) {
            return "\(steps) · last edited \(when)"
        }
        return steps
    }
}
