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
    @State private var loadFailed = false

    /// Show only events belonging to the scene in view. Default ON: a project-wide
    /// history is hundreds of rows and mostly about other scenes, which is what made
    /// the unfiltered card unreadable. Per-device, not per-project — it is a viewing
    /// preference, not a creative decision.
    @AppStorage("historyCardSceneOnly") private var sceneOnly: Bool = true

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
        // I-0105: reload on scene identity AND on every history mutation. Keyed on
        // sceneID alone the card went stale the moment an event was committed —
        // the tree lives in HistoryCapture/ScriviCore, neither of which the view
        // can observe, so nothing invalidated it until the project was reopened.
        .task(id: reloadKey) { reload() }
    }

    // I-0108: purge and its confirmation moved OUT of this card to Project Settings,
    // which has owned the same flow since T-0212/SP-055 (scan, list, purge, plus the
    // staleness threshold). Two destructive surfaces for one irreversible operation was
    // the duplication that made the project-wide list look like a filter bug here.

    /// Identity for the reload task (I-0105): the scene AND the history revision, so
    /// the tree is re-fetched both when the writer moves to another scene and when an
    /// edit is committed in the current one. The caret is deliberately NOT part of this
    /// — caret movement only changes which row is bolded, which is pure render, and
    /// re-fetching on every arrow key would be gratuitous engine traffic.
    private var reloadKey: String {
        "\(context.sceneID)#\(context.historyRevision)"
    }

    // MARK: — Tree

    @ViewBuilder
    private func treeView(_ tree: HistoryTreeResult) -> some View {
        let shown = orderedNodes(tree)

        if tree.nodes.isEmpty {
            notice("No history yet.")
        } else {
            // Resolved once for the whole list, not per row: exactly one entry can be
            // "where the caret is" (I-0106).
            let caretNode = caretNodeID(in: shown)

            VStack(alignment: .leading, spacing: 3) {
                scopeToggle(shown: shown.count, total: tree.nodes.count)

                if shown.isEmpty {
                    notice("No history for this scene yet.")
                }

                // Newest first — the writer's attention is at the tip, not the root.
                ForEach(shown) { node in
                    HistoryNodeRow(
                        node: node,
                        // Bold the entry whose change the caret is sitting inside, so
                        // a long list of similar rows still says "you are here"
                        // (user request, 2026-08-06).
                        atCaret: node.eventID == caretNode,
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
        // Scene scope first. The root is always dropped when scoped — it is a
        // non-textual anchor belonging to no scene.
        let scoped = sceneOnly
            ? tree.nodes.filter { $0.sceneID == context.sceneID }
            : tree.nodes
        let spine = scoped.filter(\.onPrimarySpine).reversed()
        let branches = scoped.filter { !$0.onPrimarySpine }
        return Array(spine) + branches
    }

    /// Scope toggle + the honest count, so a writer can see that filtering is why the
    /// list is short rather than assuming history was lost.
    private func scopeToggle(shown: Int, total: Int) -> some View {
        HStack(spacing: 6) {
            Toggle("This scene only", isOn: $sceneOnly)
                .toggleStyle(.checkbox)
                .font(.caption)
            Spacer(minLength: 0)
            Text(sceneOnly ? "\(shown) of \(total)" : "\(total)")
                .font(.caption2)
                .foregroundStyle(.secondary)
                .monospacedDigit()
        }
        .padding(.bottom, 2)
    }

    /// The single event whose change the caret is sitting inside, or nil.
    ///
    /// I-0106 (b): this is resolved ACROSS the shown set rather than per row, because
    /// ranges legitimately touch — a caret at the boundary between two entries is
    /// inside both under a half-open range, and before this the card simply bolded
    /// every match. **The most recent node wins a shared boundary**, matching how the
    /// writer perceives it: the caret sitting where they just typed belongs to the
    /// text they just typed, not to whatever preceded it.
    ///
    /// `shown` is spine-newest-first followed by branch nodes (see `orderedNodes`), so
    /// the first match in that order is the most recent one.
    private func caretNodeID(in shown: [HistoryTreeNode]) -> String? {
        guard let caret = context.caretByteOffset, caret >= 0 else { return nil }
        return shown.first { node in
            node.sceneID == context.sceneID && node.contains(caret: caret)
        }?.eventID
    }

    /// Clicking a node off the current line re-primaries its fork so redo walks it.
    /// A node already on the spine is a no-op — selecting it would not move anything.
    private func select(_ node: HistoryTreeNode, in tree: HistoryTreeResult) {
        guard !node.onPrimarySpine, !node.parentID.isEmpty else { return }
        context.history?.selectBranch(forkNodeID: node.parentID, childEventID: node.eventID)
        reload()
    }

    // MARK: — Stale branches

    // I-0108 — a BADGE, not a list.
    //
    // The card used to render the full stale-branch list with per-branch purge. That was
    // confusing for a reason the writer named exactly (2026-08-11): stale branches are
    // **project-wide**, so they kept appearing even with "This scene only" checked, which
    // reads as a bug in the filter. It was also a duplicate — Project Settings has shipped
    // a complete scan/list/purge surface since T-0212/SP-055, alongside the staleness
    // threshold that governs it.
    //
    // So the card keeps the passive signal (you still learn abandoned branches exist,
    // where you are actually working) and hands off the management, which is project-level
    // housekeeping. `StaleBranch` carries no sceneID, so per-scene filtering would have
    // needed a C ABI change to show something Project Settings already shows better.
    //
    // Scope note: this amends T-0366, which specified "stale badges + user-confirmed
    // purge" in the card. Purge now lives in one place instead of two.
    private var staleSection: some View {
        VStack(alignment: .leading, spacing: 2) {
            Label("\(stale.count) abandoned \(stale.count == 1 ? "branch" : "branches")",
                  systemImage: "exclamationmark.triangle")
                .font(.caption.weight(.medium))
                .foregroundStyle(.secondary)

            // Stated plainly so the count is never read as scoped to this scene.
            Text("Across the whole project — manage in Project Settings…")
                .font(.caption2)
                .foregroundStyle(.tertiary)
                .fixedSize(horizontal: false, vertical: true)
        }
        .frame(maxWidth: .infinity, alignment: .leading)
        .accessibilityElement(children: .combine)
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
    let atCaret: Bool
    let onSelect: () -> Void

    var body: some View {
        Button(action: onSelect) {
            HStack(spacing: 6) {
                // Spine vs. branch is the load-bearing distinction: the writer needs to
                // see at a glance which line she is on.
                Image(systemName: glyph)
                    .font(.caption2)
                    .foregroundStyle(glyphColor)
                    .frame(width: 12)
                    .help(isDeletion ? "Text removed" : "Text added")

                Text(label)
                    .font(atCaret ? .caption.weight(.bold) : .caption)
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
        // T-0398: a deletion must not look like an insertion. Before this every row
        // drew `circle.fill`, so the writer could not tell that an entry was text they
        // had REMOVED — the reported case was a deletion of "is the" reading exactly
        // like typed text.
        if isDeletion { return node.onPrimarySpine ? "minus.circle.fill" : "minus.circle" }
        return node.onPrimarySpine ? "circle.fill" : "circle"
    }

    /// Events that removed text. A pure deletion has no insertion at all; `cut` is
    /// included because it removes from the scene too (the text lives on in a buffer).
    private var isDeletion: Bool {
        node.isPureDeletion || node.kind == "delete" || node.kind == "cut"
    }

    /// Deletions read in a distinct hue so insert-vs-delete is legible at a glance,
    /// not only on close reading of the label. Non-deletions keep the original
    /// `.secondary` treatment — this task adds a distinction, it does not restyle
    /// every existing row.
    private var glyphColor: Color {
        if node.isCurrent { return .accentColor }
        return isDeletion ? .orange : .secondary
    }

    private var label: String {
        if !node.barrierKind.isEmpty {
            return node.barrierNote.isEmpty ? node.barrierKind : node.barrierNote
        }
        // T-0397: an all-whitespace change is NAMED before anything else. It must be
        // checked ahead of `preview`, because preview rewrites \n/\r/\t to spaces and
        // trimming then leaves an empty string — the exact path that produced the
        // "(no text)" rows the writer could not identify.
        if let whitespace = node.whitespaceLabel {
            return isDeletion ? "Deleted \(whitespace)" : whitespace
        }
        let preview = node.preview.trimmingCharacters(in: .whitespacesAndNewlines)
        if !preview.isEmpty {
            // T-0398: prefix so the row states what happened rather than leaving the
            // glyph to carry it alone.
            return isDeletion ? "Deleted \(preview)" : preview
        }
        if node.kind == "barrier" { return "(structural change)" }
        // A deletion whose preview is empty still has a story to tell.
        if isDeletion, node.removedLength > 0 {
            return "Deleted \(node.removedLength) \(node.removedLength == 1 ? "byte" : "bytes")"
        }
        return "(no text)"
    }
}
