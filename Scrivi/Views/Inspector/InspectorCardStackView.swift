import SwiftUI

// T-0362 — the card stack: a scrolling column of collapsible cards, with add/remove/
// reorder/collapse, per-STACK sort, and "apply to all scenes".
// Design: Scrivi_Scene_Inspector_Card_Framework_v0_1.md §C1, §C3, §C6, §4.5.

struct InspectorCardStackView: View {

    let stack: InspectorStack
    let sceneID: String?
    let projectRootPath: String?
    let engine: ScriviEngine
    let allSceneIDs: [String]
    let history: HistoryCapture?
    let caretByteOffset: Int?
    /// Bumped when world availability changes (I-0128), so object cards re-read
    /// after a drive is reconnected instead of waiting for a scene change.
    var worldRevision: Int = 0
    /// I-0155 — bumped by a Detail Sheet save so cards re-read (passed through to
    /// `InspectorCardContext`, which is what the cards' `.task(id:)` watches).
    var objectRevision: Int = 0
    /// I-0160 — a card changed an object; the host refreshes the Detail Sheet.
    var onObjectChanged: (() -> Void)? = nil
    let authorshipRef: AuthorshipRef?
    /// Passed through to every card so an object card can ask the HOST to open
    /// the Detail Sheet (SP-117 T-0438). Optional end to end: a host that does
    /// not offer the sheet passes nothing and no affordance appears.
    var openObjectDetail: ((String, String, String, String) -> Void)? = nil
    @Bindable var layout: InspectorLayoutStore

    var body: some View {
        VStack(spacing: 0) {
            header

            Divider()

            if let sceneID, let projectRootPath {
                content(sceneID: sceneID, projectRootPath: projectRootPath)
            } else {
                emptyState("No scene selected.")
            }
        }
    }

    // MARK: — Header (add menu + per-stack sort + apply-to-all)

    private var header: some View {
        HStack(spacing: 8) {
            Menu {
                let available = InspectorCardRegistry.available(in: stack)
                if available.isEmpty {
                    Text("No cards available yet")
                } else {
                    ForEach(available, id: \.typeID) { card in
                        Button {
                            guard let sceneID else { return }
                            layout.addCard(typeID: card.typeID, sceneID: sceneID, stack: stack)
                        } label: {
                            Label(card.title, systemImage: card.systemImage)
                        }
                        .disabled(sceneID == nil || isPresent(card.typeID))
                    }
                }
            } label: {
                Label("Add Card", systemImage: "plus")
                    .labelStyle(.iconOnly)
            }
            .menuStyle(.borderlessButton)
            .fixedSize()
            .help("Add a card to this stack")
            .disabled(sceneID == nil)

            Spacer()

            // Sort is per-STACK, not per-card (C6): one setting for the whole column.
            Menu {
                Picker("Sort", selection: sortBinding) {
                    ForEach(InspectorSort.allCases, id: \.self) { option in
                        Text(option.title).tag(option)
                    }
                }
                .pickerStyle(.inline)

                Divider()

                Button("Apply Layout to All Scenes…") { applyToAllScenes() }
                    .disabled(sceneID == nil || allSceneIDs.isEmpty)
            } label: {
                Label("Stack Options", systemImage: "ellipsis.circle")
                    .labelStyle(.iconOnly)
            }
            .menuStyle(.borderlessButton)
            .fixedSize()
            .help("Sort and layout options for this stack")
        }
        .padding(.horizontal, 10)
        .padding(.vertical, 6)
    }

    private var sortBinding: Binding<InspectorSort> {
        Binding(
            get: { layout.sort(for: stack) },
            set: { layout.setSort($0, for: stack) }
        )
    }

    // MARK: — Content

    @ViewBuilder
    private func content(sceneID: String, projectRootPath: String) -> some View {
        let resolved = layout.resolvedStack(sceneID: sceneID, stack: stack)
        let ordered = applySort(resolved.entries)

        if ordered.isEmpty && resolved.unknownTypeIDs.isEmpty {
            emptyState(stack == .worldbuilding
                       ? "No cards yet. Use + to add one."
                       : "No cards in this stack. Use + to add one.")
        } else {
            ScrollView {
                LazyVStack(alignment: .leading, spacing: 8) {
                    ForEach(ordered, id: \.type) { entry in
                        if let card = InspectorCardRegistry.card(for: entry.type) {
                            InspectorCardView(
                                card: card,
                                entry: entry,
                                context: CardContext(
                                    sceneID: sceneID,
                                    projectRootPath: projectRootPath,
                                    engine: engine,
                                    config: entry.config,
                                    allSceneIDs: allSceneIDs,
                                    history: history,
                                    caretByteOffset: caretByteOffset,
                                    historyRevision: history?.revision ?? 0,
                                    worldRevision: worldRevision,
                                    objectRevision: objectRevision,
                                    onObjectChanged: onObjectChanged,
                                    // C6: one sort for the whole stack, handed to
                                    // every card rather than each owning its own.
                                    openObjectDetail: openObjectDetail,
                                    sort: layout.sort(for: stack),
                                    authorshipRef: authorshipRef
                                ),
                                onToggleCollapsed: {
                                    layout.setCollapsed(!entry.collapsed,
                                                        typeID: entry.type,
                                                        sceneID: sceneID,
                                                        stack: stack)
                                },
                                onRemove: {
                                    layout.removeCard(typeID: entry.type,
                                                      sceneID: sceneID,
                                                      stack: stack)
                                }
                            )
                        }
                    }

                    // AC11 — a layout referencing a card this build doesn't have is
                    // reported, never silently dropped, and is REMOVABLE. A layout
                    // written by a newer Scrivi (or by SP-090, which registered a
                    // `placeholder` card that SP-091 retired) must not strand an entry
                    // the writer has no way to clear.
                    if !resolved.unknownTypeIDs.isEmpty {
                        unknownCardsNotice(resolved.unknownTypeIDs, sceneID: sceneID)
                    }
                }
                .padding(10)
            }
        }
    }

    /// One removable row per unavailable card. Each carries its own ✕ so the writer can
    /// clear a stranded entry — the same affordance a real card has, because from her
    /// point of view it *is* a card in her stack that she wants gone.
    private func unknownCardsNotice(_ typeIDs: [String], sceneID: String) -> some View {
        VStack(alignment: .leading, spacing: 6) {
            ForEach(typeIDs.sorted(), id: \.self) { typeID in
                HStack(spacing: 6) {
                    Image(systemName: "questionmark.square.dashed")
                        .font(.caption)
                        .foregroundStyle(.secondary)

                    VStack(alignment: .leading, spacing: 1) {
                        Text(typeID)
                            .font(.callout.weight(.medium))
                        Text("This card isn't available in this version.")
                            .font(.caption)
                            .foregroundStyle(.secondary)
                    }

                    Spacer(minLength: 0)

                    Button {
                        layout.removeCard(typeID: typeID, sceneID: sceneID, stack: stack)
                    } label: {
                        Image(systemName: "xmark")
                            .font(.caption2.weight(.semibold))
                            .foregroundStyle(.secondary)
                    }
                    .buttonStyle(.plain)
                    .help("Remove this card from the scene")
                }
                .padding(8)
                .frame(maxWidth: .infinity, alignment: .leading)
                .background(.quaternary.opacity(0.4), in: RoundedRectangle(cornerRadius: 6))
                .overlay(
                    RoundedRectangle(cornerRadius: 6)
                        .strokeBorder(.quaternary, style: StrokeStyle(lineWidth: 1, dash: [3, 2]))
                )
            }
        }
    }

    private func emptyState(_ message: String) -> some View {
        VStack {
            Spacer()
            Text(message)
                .font(.callout)
                .foregroundStyle(.secondary)
                .multilineTextAlignment(.center)
                .padding(.horizontal, 16)
            Spacer()
        }
        .frame(maxWidth: .infinity, maxHeight: .infinity)
    }

    // MARK: — Helpers

    private func isPresent(_ typeID: String) -> Bool {
        guard let sceneID else { return false }
        return layout.rawEntries(sceneID: sceneID, stack: stack).contains { $0.type == typeID }
    }

    /// `.manual` preserves the writer's order as stored — it must not re-sort.
    private func applySort(_ entries: [InspectorCardEntry]) -> [InspectorCardEntry] {
        switch layout.sort(for: stack) {
        case .manual:
            return entries
        case .name:
            return entries.sorted {
                let l = InspectorCardRegistry.card(for: $0.type)?.title ?? $0.type
                let r = InspectorCardRegistry.card(for: $1.type)?.title ?? $1.type
                return l.localizedCaseInsensitiveCompare(r) == .orderedAscending
            }
        case .recentlyAdded:
            // Entries are appended on add, so reverse insertion order is "most recent".
            return entries.reversed()
        }
    }

    private func applyToAllScenes() {
        guard let sceneID else { return }
        layout.applyStackToAllScenes(sceneID: sceneID, stack: stack, allSceneIDs: allSceneIDs)
    }
}

// MARK: — One card

private struct InspectorCardView: View {

    let card: AnyInspectorCard
    let entry: InspectorCardEntry
    let context: CardContext
    let onToggleCollapsed: () -> Void
    let onRemove: () -> Void

    var body: some View {
        VStack(alignment: .leading, spacing: 0) {
            header
            if !entry.collapsed {
                Divider()
                // T-0399 — soft-failure isolation as a FRAMEWORK guarantee (AC12 as
                // rescoped 2026-08-11). A card that throws while building its content
                // shows an inline warning in its place; the rest of the stack renders.
                // Cards that report their own load errors still do — this is the
                // backstop for the ones that don't. Trapping cards are out of scope:
                // SwiftUI cannot contain them (Doc 2 §7.1).
                CardBodyBoundary(cardTitle: card.title) {
                    try card.body(context: context)
                }
                .padding(10)
            }
        }
        .background(.quaternary.opacity(0.25), in: RoundedRectangle(cornerRadius: 8))
        .overlay(
            RoundedRectangle(cornerRadius: 8).strokeBorder(.quaternary, lineWidth: 1)
        )
    }

    private var header: some View {
        HStack(spacing: 6) {
            Button(action: onToggleCollapsed) {
                Image(systemName: entry.collapsed ? "chevron.right" : "chevron.down")
                    .font(.caption.weight(.semibold))
                    .foregroundStyle(.secondary)
                    .frame(width: 14)
            }
            .buttonStyle(.plain)
            .help(entry.collapsed ? "Expand" : "Collapse")

            Image(systemName: card.systemImage)
                .font(.caption)
                .foregroundStyle(.secondary)

            Text(card.title)
                .font(.callout.weight(.medium))

            Spacer(minLength: 0)

            Button(action: onRemove) {
                Image(systemName: "xmark")
                    .font(.caption2.weight(.semibold))
                    .foregroundStyle(.secondary)
            }
            .buttonStyle(.plain)
            .help("Remove this card from the scene")
        }
        .padding(.horizontal, 10)
        .padding(.vertical, 7)
        .contentShape(Rectangle())
    }
}

// MARK: — Failure isolation (T-0368)

/// Contains a card body so one card's trouble never takes the stack down (Doc 2 §7.1,
/// AC12).
///
/// **What this can and cannot do, stated plainly.** SwiftUI offers no `catch` around a
/// view body, so a genuine Swift trap inside a card still terminates the process — no
/// wrapper can change that. What isolation *does* mean here, and what AC12 is verified
/// against:
///
/// 1. **Independent data loads.** Every card fetches its own data in its own `.task`,
///    so a failing read (`getSceneNotes`, `historyGetTree`) is confined to that card and
///    surfaces as its inline error. No card's load is a precondition for another's.
/// 2. **No shared mutable render state.** Cards receive a value-type `CardContext` and
///    hold their own `@State`; there is nothing for a failing card to corrupt.
/// 3. **A card that renders nothing is visible as such** — this wrapper reserves the
///    row so an empty body reads as an empty card, not as a card that silently vanished.
///    A vanished card looks like lost configuration to a writer.
/// The framework's soft-failure backstop (EP-030 AC12 as rescoped 2026-08-11, Doc 2 §7.1).
///
/// A card that throws while building its content renders an inline warning **in place of its
/// content**, and the rest of the stack keeps rendering. Cards that report their own load
/// errors (`CardErrorView`) still do — that message is specific and better; this is the
/// backstop for cards that don't, so isolation doesn't depend on every card author.
///
/// ⚠️ **Soft failures only.** SwiftUI cannot catch a trapping view body — a card that traps
/// terminates the process, and no wrapper can contain it. That case is out of scope by
/// design, not an oversight; it is caught by tests and review, never absorbed at runtime.
private struct CardBodyBoundary: View {
    let cardTitle: String
    let build: () throws -> AnyView

    var body: some View {
        Group {
            if let content = try? build() {
                content
            } else {
                CardFailureView(cardTitle: cardTitle)
            }
        }
        .frame(maxWidth: .infinity, minHeight: 18, alignment: .leading)
    }
}

/// Generic in-place failure presentation. Deliberately plain: it names the card so the
/// writer knows *which* one is broken, and says the rest of the inspector still works —
/// a blank card body with no explanation was the failure mode this replaces.
private struct CardFailureView: View {
    let cardTitle: String

    var body: some View {
        HStack(alignment: .top, spacing: 5) {
            Image(systemName: "exclamationmark.triangle")
                .font(.caption)
                .foregroundStyle(.orange)
            Text("“\(cardTitle)” couldn't be displayed. Other cards are unaffected.")
                .font(.caption)
                .foregroundStyle(.secondary)
        }
        .frame(maxWidth: .infinity, alignment: .leading)
        .accessibilityIdentifier("card-failure-\(cardTitle)")
    }
}
