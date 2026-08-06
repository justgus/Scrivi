import SwiftUI

// T-0366 — the history card (supersedes EP-019's T-0215 "history panel").
// Design: Scrivi_UndoRedo_History_and_Copy_Buffers_Design_v0_1.md §10 Trade T2 option B;
// Scrivi_Scene_Inspector_Card_Framework_v0_1.md §3.1.
//
// The T2 management surface, delivered as a dismissible card in the Writing stack rather
// than a fixed panel: the windowed tree (primary spine + fork glyphs), click-to-select a
// branch, stale-branch badges, and user-confirmed purge.

struct HistoryCard: InspectorCard {
    static let typeID = "history"
    static let title = "History"
    static let systemImage = "clock.arrow.circlepath"
    static let stack: InspectorStack = .writing

    init() {}

    func body(context: CardContext) -> AnyView {
        AnyView(HistoryCardBody(context: context))
    }
}

private struct HistoryCardBody: View {
    let context: CardContext

    @State private var tree: HistoryTreeResult?
    @State private var stale: [HistoryStaleBranch] = []
    @State private var pendingPurge: HistoryStaleBranch?
    @State private var loadFailed = false

    var body: some View {
        VStack(alignment: .leading, spacing: 8) {
            if context.history == nil {
                notice("Undo history isn't open for this project.")
            } else if loadFailed {
                notice("Couldn't read the undo history.")
            } else if let tree {
                treeView(tree)
                if !stale.isEmpty {
                    Divider()
                    staleSection
                }
            } else {
                ProgressView().controlSize(.small)
            }
        }
        .task(id: context.sceneID) { reload() }
        .confirmationDialog(
            purgeTitle,
            isPresented: Binding(get: { pendingPurge != nil },
                                 set: { if !$0 { pendingPurge = nil } }),
            titleVisibility: .visible
        ) {
            // Purge is irreversible — it writes a ctl:purge record so the branch cannot
            // resurrect on reload. The button names what is discarded rather than
            // saying a bare "OK".
            if let branch = pendingPurge {
                Button("Discard \(branch.nodeCount) \(branch.nodeCount == 1 ? "Edit" : "Edits")",
                       role: .destructive) {
                    context.history?.purgeStaleBranch(branchRootEventID: branch.branchRootEventID)
                    pendingPurge = nil
                    reload()
                }
            }
            Button("Keep", role: .cancel) { pendingPurge = nil }
        } message: {
            if let branch = pendingPurge {
                Text("This permanently discards this branch and everything on it. "
                     + "It can't be undone.\n\n\(branch.preview)")
            }
        }
    }

    private var purgeTitle: String {
        pendingPurge.map { _ in "Discard this abandoned branch?" } ?? ""
    }

    // MARK: — Tree

    @ViewBuilder
    private func treeView(_ tree: HistoryTreeResult) -> some View {
        if tree.nodes.isEmpty {
            notice("No history yet.")
        } else {
            VStack(alignment: .leading, spacing: 3) {
                // Newest first — the writer's attention is at the tip, not the root.
                ForEach(orderedNodes(tree)) { node in
                    HistoryNodeRow(
                        node: node,
                        onSelect: { select(node, in: tree) }
                    )
                }

                if tree.truncated {
                    Text("Showing \(tree.nodes.count) of \(tree.totalNodeCount) events.")
                        .font(.caption2)
                        .foregroundStyle(.secondary)
                        .padding(.top, 2)
                }
            }
        }
    }

    /// Spine nodes newest-first, then any off-spine (branch) nodes — so the line the
    /// writer is on reads top-down and abandoned branches sit below it.
    private func orderedNodes(_ tree: HistoryTreeResult) -> [HistoryTreeNode] {
        let spine = tree.nodes.filter(\.onPrimarySpine).reversed()
        let branches = tree.nodes.filter { !$0.onPrimarySpine }
        return Array(spine) + branches
    }

    /// Clicking a node off the current line re-primaries its fork so redo walks it.
    /// A node already on the spine is a no-op — selecting it would not move anything.
    private func select(_ node: HistoryTreeNode, in tree: HistoryTreeResult) {
        guard !node.onPrimarySpine, !node.parentID.isEmpty else { return }
        context.history?.selectBranch(forkNodeID: node.parentID, childEventID: node.eventID)
        reload()
    }

    // MARK: — Stale branches

    private var staleSection: some View {
        VStack(alignment: .leading, spacing: 4) {
            Label("\(stale.count) abandoned \(stale.count == 1 ? "branch" : "branches")",
                  systemImage: "exclamationmark.triangle")
                .font(.caption.weight(.medium))
                .foregroundStyle(.secondary)

            ForEach(stale, id: \.branchRootEventID) { branch in
                HStack(spacing: 6) {
                    VStack(alignment: .leading, spacing: 1) {
                        Text(branch.preview.isEmpty ? "(no preview)" : branch.preview)
                            .font(.caption)
                            .lineLimit(1)
                        Text("\(branch.nodeCount) \(branch.nodeCount == 1 ? "edit" : "edits")")
                            .font(.caption2)
                            .foregroundStyle(.secondary)
                    }
                    Spacer(minLength: 0)
                    Button("Discard…") { pendingPurge = branch }
                        .buttonStyle(.borderless)
                        .font(.caption)
                }
            }
        }
    }

    // MARK: — Helpers

    private func reload() {
        guard let history = context.history else { return }
        guard let t = history.tree() else {
            loadFailed = true
            return
        }
        loadFailed = false
        tree = t
        stale = history.listStaleBranches()
    }

    private func notice(_ message: String) -> some View {
        Text(message)
            .font(.caption)
            .foregroundStyle(.secondary)
            .frame(maxWidth: .infinity, alignment: .leading)
    }
}

// MARK: — One node row

private struct HistoryNodeRow: View {
    let node: HistoryTreeNode
    let onSelect: () -> Void

    var body: some View {
        Button(action: onSelect) {
            HStack(spacing: 6) {
                // Spine vs. branch is the load-bearing distinction: the writer needs to
                // see at a glance which line she is on.
                Image(systemName: glyph)
                    .font(.caption2)
                    .foregroundStyle(node.isCurrent ? Color.accentColor : .secondary)
                    .frame(width: 12)

                Text(label)
                    .font(.caption)
                    .lineLimit(1)
                    .foregroundStyle(node.onPrimarySpine ? .primary : .secondary)

                if node.isFork {
                    Image(systemName: "arrow.triangle.branch")
                        .font(.caption2)
                        .foregroundStyle(.secondary)
                        .help("This point has more than one continuation")
                }

                Spacer(minLength: 0)
            }
            .padding(.vertical, 1)
            .contentShape(Rectangle())
        }
        .buttonStyle(.plain)
        .background(node.isCurrent ? Color.accentColor.opacity(0.12) : .clear,
                    in: RoundedRectangle(cornerRadius: 4))
        .help(node.onPrimarySpine ? "" : "Switch to this branch")
    }

    private var glyph: String {
        if node.isCurrent { return "largecircle.fill.circle" }
        if node.kind == "barrier" || node.kind == "structural" { return "minus.circle" }
        return node.onPrimarySpine ? "circle.fill" : "circle"
    }

    private var label: String {
        if !node.barrierKind.isEmpty {
            return node.barrierNote.isEmpty ? node.barrierKind : node.barrierNote
        }
        let preview = node.preview.trimmingCharacters(in: .whitespacesAndNewlines)
        if !preview.isEmpty { return preview }
        return node.kind == "barrier" ? "(structural change)" : "(no text)"
    }
}
