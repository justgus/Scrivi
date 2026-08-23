import SwiftUI

/// The Detail Sheet's related-objects section — what this object is connected to,
/// and the place a writer connects it to something else (EP-034 SP-118; **R3**,
/// **R4**, **R5**, **R7**, trade **D4-A**). Closes **AC5** and **AC6**.
///
/// ## ⚠️ The label is READ, never recomputed
///
/// `EdgeView.label` already reads correctly **from the queried endpoint** — one
/// stored edge renders as `forwardLabel` from one end and `inverseLabel` from the
/// other (Doc 1 §5.2), and ScriviCore resolves which applies. Recomputing that in
/// Swift would need a local copy of the direction rule, which is the
/// restated-rule defect this Epic has already paid for nine times.
///
/// ⚠️ **`isForward` is used only to GROUP**, never to pick a label.
///
/// ## ⚠️ A pending far endpoint is listed, named and explained
///
/// Doc 3: *absence is never deletion*. When the far object's world is away the row
/// stays, keeps its cached name, says why, and refuses to navigate or be removed —
/// it is never hidden, which would read as the writer's work having vanished
/// (Doc 2 §7.2).
///
/// ⚠️ **I-0124's lesson is load-bearing here:** `otherKind` travels **on the
/// edge** precisely because a pending object is absent from the object index by
/// definition. A pending row must never be attributed by index lookup.
///
/// ## Host-independence (S8)
///
/// Like `ObjectDetailSheet`, this reads nothing from its host. Everything —
/// engine, project root, worlds, the navigation callback — arrives as a parameter.
struct ObjectRelationsSection: View {

    let engine: ScriviEngine
    let projectRootPath: String
    /// The object whose relationships these are.
    let objectID: String
    /// Bound worlds, for naming an absent world in a pending row.
    let worlds: [WorldEntry]
    /// Writer-facing scene names, keyed by sceneID (I-0151). Supplied by the host.
    var sceneNames: [String: String] = [:]
    /// ⚠️ True when this object itself is read-only (its own world is away). The
    /// graph is frozen toward an unavailable world in **both** directions, so a
    /// pending object may not gain relationships either (Doc 3 §4.6 rule 1).
    let isReadOnly: Bool
    /// I-0151 — jump the manuscript to a related SCENE. A scene is not an object
    /// and has no Detail Sheet, so "Show" on a scene row must take the writer to
    /// the scene itself rather than doing nothing.
    var onSelectScene: (String) -> Void = { _ in }
    /// R5/R7 — push-navigate to a related object. The sheet owns the history.
    let onNavigate: (ObjectDetailHistory.Entry) -> Void

    @State private var edges: [EdgeView] = []
    @State private var loadError: String?
    @State private var actionError: String?
    @State private var showPicker = false
    /// The selected row. ⚠️ Bound to `List`'s own `selection`, which handles the
    /// highlight and selects a right-clicked row before showing its menu (I-0158).
    @State private var selectedEdgeID: String?
    /// The row under the pointer — what a right-click will target (I-0158).
    @State private var hoveredEdgeID: String?

    var body: some View {
        VStack(alignment: .leading, spacing: 6) {
            header

            if let loadError {
                Label(loadError, systemImage: "exclamationmark.triangle")
                    .font(.caption)
                    .foregroundStyle(.orange)
            }

            if edges.isEmpty && loadError == nil {
                Text("Not related to anything yet.")
                    .font(.callout)
                    .foregroundStyle(.secondary)
            } else {
                // ⚠️ I-0158: a real `List` with a `selection` binding, NOT a
                // hand-rolled VStack/ForEach.
                //
                // The first version built the rows by hand and therefore had to
                // reimplement selection, the highlight, and right-click behaviour —
                // and got the last one wrong: right-clicking an UNSELECTED row
                // opened its menu while a DIFFERENT row stayed highlighted, so the
                // sheet showed one row selected and acted on another. With "Tall
                // Ash" and "Short Ash" adjacent that is a genuine mis-click waiting
                // to happen.
                //
                // ⚠️ `List` does all of this for free and to the platform's own
                // rules — including "right-click selects the row it targets first",
                // which is exactly the behaviour I was reconstructing badly.
                //
                // Grouped by the label as it reads FROM THIS OBJECT, so "appears in"
                // and "features" never collapse into one undifferentiated list. The
                // label is the group key precisely because it is the core's answer,
                // not ours.
                // ⚠️ I-0159: a nested `List` here HID ROWS BEHIND AN INVISIBLE
                // SCROLL. This section lives inside the sheet's own `ScrollView`,
                // so the inner `List` got a capped 260pt viewport of its own.
                // ⚠️ **Myton appeared to show 3 of his 8 relationships.** The rows
                // were all present and the inner list *could* be scrolled — but
                // nothing on screen said so, so the writer read it as work missing.
                //
                // ⚠️ That is the failure regardless of the mechanism: whether rows
                // are absent or merely unreachable-looking, the writer concludes her
                // work is gone (Doc 3: absence is never deletion). ⚠️ **A scroll
                // region with no indication that it scrolls is a data-loss report
                // waiting to happen** — the same lesson as I-0127, where a collapsed
                // picker let the writer see a fraction of a list she could not
                // survey.
                //
                // ⚠️ `List` was the right answer for selection semantics (I-0158)
                // and the wrong one for layout. A `LazyVStack` in the sheet's
                // EXISTING ScrollView sizes to its content and takes the height the
                // window actually has, so the list is as long as it needs to be and
                // scrolls with the sheet — one scroll region, not two nested ones.
                // Selection and right-click targeting are kept explicitly below.
                LazyVStack(alignment: .leading, spacing: 2) {
                    ForEach(groups, id: \.label) { group in
                        Section {
                            ForEach(group.edges) { edge in
                                row(edge)
                            }
                        } header: {
                            Text(group.label)
                                .font(.caption.weight(.semibold))
                                .foregroundStyle(.secondary)
                                .frame(maxWidth: .infinity, alignment: .leading)
                                .padding(.top, 4)
                        }
                    }
                }
            }

            if let actionError {
                Label(actionError, systemImage: "exclamationmark.triangle")
                    .font(.caption)
                    .foregroundStyle(.orange)
            }
        }
        .onChange(of: objectID) { _, _ in load() }
        .onAppear { load() }
    }

    // MARK: — Chrome

    private var header: some View {
        HStack {
            Text("Related")
                .font(.caption.weight(.semibold))
                .foregroundStyle(.secondary)

            Spacer()

            // D4-A: creation is inline, from here. ⚠️ D4-C (drag-and-drop) was
            // RULED OUT as a sole path — VNC carries no drag gestures and AC11
            // requires Linux parity, so a gesture-only affordance would be
            // unreachable on a supported platform.
            Button {
                showPicker = true
            } label: {
                Label("Relate…", systemImage: "plus.circle")
                    .font(.caption)
            }
            .buttonStyle(.borderless)
            // ⚠️ Disabled AND explained (§7.2) — never merely greyed out.
            .disabled(isReadOnly)
            .help(isReadOnly
                  ? "This object's world is unavailable, so its relationships are frozen."
                  : "Relate this object to another object")
            .popover(isPresented: $showPicker, arrowEdge: .bottom) {
                ObjectRelationPicker(
                    engine: engine,
                    projectRootPath: projectRootPath,
                    sourceObjectID: objectID,
                    existingObjectIDs: Set(edges.map(\.otherID)),
                    onCreate: { targetID, code in
                        showPicker = false
                        relate(toID: targetID, relationTypeCode: code)
                    }
                )
            }
        }
    }

    @ViewBuilder
    private func row(_ edge: EdgeView) -> some View {
        HStack(spacing: 6) {
            Image(systemName: edge.otherPending
                  ? "exclamationmark.triangle.fill"
                  : symbol(for: edge.otherKind))
                .font(.caption)
                .foregroundStyle(edge.otherPending ? .orange : .secondary)
                // The sentence carries the meaning; the icon must not be
                // announced twice.
                .accessibilityHidden(true)

            VStack(alignment: .leading, spacing: 1) {
                // ⚠️ Named even while pending. `otherDisplayName` is cached on the
                // edge for exactly this reason — a writer is never asked to
                // recognise a bare ID (AC-A7).
                Text(displayName(for: edge))
                    .font(.callout)
                    .foregroundStyle(edge.otherPending ? .secondary : .primary)

                if edge.otherPending {
                    // ⚠️ T-0445: listed, named, AND EXPLAINED. Hiding it would
                    // read as deletion; showing it unexplained is the I-0138
                    // defect — a greyed row that says nothing.
                    Text(pendingSentence(for: edge))
                        .font(.caption2)
                        .foregroundStyle(.secondary)
                } else if let kind = edge.otherKind, !kind.isEmpty {
                    Text(kind.capitalized)
                        .font(.caption2)
                        .foregroundStyle(.tertiary)
                }
            }

            Spacer(minLength: 4)
        }
        .padding(.vertical, 3)
        .padding(.horizontal, 4)
        .frame(maxWidth: .infinity, alignment: .leading)
        .background(
            selectedEdgeID == edge.edgeID
                ? Color.accentColor.opacity(0.18)
                : Color.clear,
            in: RoundedRectangle(cornerRadius: 4)
        )
        .contentShape(Rectangle())
        // R5: double-click pushes. ⚠️ A pending object cannot be opened — its
        // record is in a world this project cannot read right now.
        .onTapGesture(count: 2) { navigate(edge) }
        .onTapGesture { selectedEdgeID = edge.edgeID }
        // ⚠️ I-0158's actual requirement: a right-click must SELECT THE ROW IT
        // TARGETS before its menu opens. Without this the sheet highlighted one
        // row and acted on another — worse than no highlight at all.
        //
        // ⚠️ Two earlier attempts failed: `TapGesture().modifiers(.control)` does
        // not observe a real right-click, and a press hook does not fire for one
        // either. **Hover is what actually tracks it** — the row under the pointer
        // IS the row a right-click will hit — and it is the pattern
        // `TimelineStripView` (I-0036) already uses for the same problem.
        .onContinuousHover { phase in
            if case .active = phase { hoveredEdgeID = edge.edgeID }
        }
        .contextMenu {
            // The menu is per-row, so it always ACTS on its own row regardless of
            // the highlight; syncing selection just makes that visible.
            rowMenu(edge)
        }
        .help(edge.otherPending
              ? pendingSentence(for: edge)
              : "Double-click to show \(displayName(for: edge))")
        .accessibilityElement(children: .combine)
        .accessibilityLabel(accessibilityLabel(for: edge))
    }

    /// The per-row context menu (R7).
    @ViewBuilder
    private func rowMenu(_ edge: EdgeView) -> some View {
        // ⚠️ R7 names this entry "Show", not "View Detail" — the sheet is already
        // open here, so the two entry points are worded differently on purpose.
        Button("Show") { navigate(edge) }
            .disabled(edge.otherPending)

        Button("Remove Relationship", role: .destructive) { remove(edge) }
            // Removal is exactly as frozen as addition (Doc 3 §4.6): a writer who
            // could delete a pending edge would be deciding irreversibly on
            // evidence she cannot see.
            .disabled(edge.otherPending || isReadOnly)
    }

    // MARK: — Presentation

    /// Grouped by the endpoint-correct label, alphabetically within each group.
    /// ⚠️ Grouping is a display choice; it never removes a row from the list.
    private var groups: [(label: String, edges: [EdgeView])] {
        Dictionary(grouping: edges, by: \.label)
            .map { (label: $0.key, edges: $0.value.sorted {
                // Sort by what the writer actually SEES, not by a raw field that
                // may be empty — otherwise untitled scenes all sort together at
                // the top under a name nobody is shown.
                displayName(for: $0).localizedCaseInsensitiveCompare(displayName(for: $1))
                    == .orderedAscending
            }) }
            .sorted { $0.label.localizedCaseInsensitiveCompare($1.label) == .orderedAscending }
    }

    /// What to call the far endpoint, in writer's terms.
    ///
    /// ⚠️ **I-0151: NEVER a raw ID.** A scene endpoint arrives with whatever
    /// `scene.meta.json` holds in `title` — and an untitled scene sends `""`,
    /// because `EndpointResolver` copies the title verbatim. This previously fell
    /// through to `otherID`, so the writer was shown
    /// `scene_019faeb3-3326-7730-b307-3845dc2f55b3` and had **no way to tell which
    /// scene it was** — found in the SP-118 live pass on `the-stairs-of-tintagael`,
    /// which has exactly one untitled scene (Ch1 Sc2).
    ///
    /// ⚠️ **The Navigator's rule, reused rather than reinvented**: title → live
    /// first words → "Scene N". The Navigator resolves it for the same scene at the
    /// same moment; showing the writer two different names for one scene is its own
    /// defect. The host computes it (S8) and this reads the answer.
    private func displayName(for edge: EdgeView) -> String {
        let name = edge.otherDisplayName.trimmingCharacters(in: .whitespaces)
        if !name.isEmpty { return name }

        // A scene endpoint carries an EMPTY `otherKind` (scenes are not objects).
        if edge.otherKind?.isEmpty ?? true, let resolved = sceneNames[edge.otherID] {
            return resolved
        }

        // ⚠️ Last resort, and still not an ID. An object whose name we cannot
        // resolve is named by what it IS; a bare identifier is never an answer to
        // "which one is this?".
        if let kind = edge.otherKind, !kind.isEmpty { return "Untitled \(kind)" }
        return "Untitled scene"
    }

    private func pendingSentence(for edge: EdgeView) -> String {
        let status = edge.pendingStatus ?? .unavailable
        let name = edge.otherWorldID.flatMap { id in
            worlds.first { $0.worldID == id }?.displayName
        }
        // ⚠️ Falling back to "its world" rather than an ID keeps the sentence
        // readable when the binding names a world this project cannot resolve —
        // an unnamed world is still better than an unattributed warning.
        let subject = name.map { "“\($0)”" } ?? "its world"
        return "Held pending — \(subject) is \(status.writerDescription)."
    }

    private func accessibilityLabel(for edge: EdgeView) -> String {
        let name = displayName(for: edge)
        if edge.otherPending {
            return "\(edge.label) \(name) — unavailable. \(pendingSentence(for: edge))"
        }
        return "\(edge.label) \(name)"
    }

    /// The icon for a kind. ⚠️ Resolved through `ObjectCardKind.all` — the app's
    /// single kind table — rather than a second switch over kind names, which
    /// would be the restated-partition defect (CLAUDE.md standing rule).
    private func symbol(for kind: String?) -> String {
        guard let kind, !kind.isEmpty else { return "doc.text" }   // "" = a scene
        return ObjectCardKind.all.first { $0.kind == kind }?.systemImage ?? "circle"
    }

    // MARK: — Actions

    private func navigate(_ edge: EdgeView) {
        // ⚠️ A pending object has no readable record — `openObject` would fail and
        // the sheet would show a load error instead of the object. Refuse here,
        // where the reason is already on screen.
        guard !edge.otherPending else { return }

        // ⚠️ I-0151: a scene endpoint carries an EMPTY `otherKind` — scenes are not
        // objects and have no Detail Sheet. This previously just `return`ed, so
        // double-clicking a scene row did NOTHING AT ALL: no navigation, no
        // message, no cursor change. A dead affordance that looks identical to a
        // live one is worse than a disabled one, because the writer cannot tell
        // whether she missed or the app did.
        //
        // A scene HAS a destination — the manuscript — so take her there.
        guard let kind = edge.otherKind, !kind.isEmpty else {
            onSelectScene(edge.otherID)
            return
        }

        onNavigate(ObjectDetailHistory.Entry(
            objectID: edge.otherID,
            kind: kind,
            // ⚠️ I-0151: this passed `""` on the assumption that `openObject`
            // resolves a world-scoped object's world itself. **It does not.**
            // `ObjectStore::kindDirFor` FAILS OUTRIGHT for a world-scoped kind with
            // an empty worldID ("a worldID is required"), and `findByID`'s index
            // lookup cannot cover for it: world objects live in the WORLD's index,
            // and the project index is empty in exactly the projects that have a
            // world. Every navigation to a world-scoped object therefore died with
            // "The operation couldn't be completed. (ScriviError error -1)".
            //
            // ⚠️ The comment that stood here asserted the opposite and was never
            // checked against `ObjectStore.cpp`.
            //
            // ✅ The core already supplies this: I-0142 moved `otherWorldID` OUT of
            // the pending branch precisely so a healthy object reports its home
            // world too (`RelationshipStore.cpp:495`), and the C ABI emits it
            // unconditionally. Empty for project-scoped kinds, which is correct.
            worldID: edge.otherWorldID ?? "",
            displayName: displayName(for: edge)
        ))
    }

    private func relate(toID: String, relationTypeCode: String) {
        do {
            _ = try engine.createEdge(
                projectRootPath: projectRootPath,
                fromID: objectID,
                toID: toID,
                relationTypeCode: relationTypeCode
            )
            actionError = nil
            load()
        } catch {
            // ⚠️ ScriviCore normalizes direction and rejects duplicates (AC21),
            // so "already related" arrives here as an error rather than a second
            // row. Reported in place; the section stays usable.
            actionError = (error as? ScriviError)?.message ?? "\(error)"
        }
    }

    private func remove(_ edge: EdgeView) {
        do {
            _ = try engine.deleteEdge(projectRootPath: projectRootPath, edgeID: edge.edgeID)
            actionError = nil
            load()
        } catch {
            actionError = (error as? ScriviError)?.message ?? "\(error)"
        }
    }

    private func load() {
        guard !objectID.isEmpty, !projectRootPath.isEmpty else {
            edges = []
            return
        }
        do {
            // ⚠️ Every edge touching this object, in EITHER direction, each
            // already carrying the label that reads correctly from this end.
            edges = try engine.listEdgesFor(
                projectRootPath: projectRootPath, endpointID: objectID).edges
            loadError = nil
        } catch {
            edges = []
            loadError = (error as? ScriviError)?.message ?? "\(error)"
        }
    }
}
