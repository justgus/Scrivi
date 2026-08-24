import SwiftUI

/// The Detail Sheet's **sources** section — where a citation is created on the
/// object it documents, and where the object's own citations are read
/// (EP-034 SP-120; **R6**, closing **AC8** and T-0365's write half).
///
/// ## ⚠️ Why creation lives HERE and not on the `sources` card
///
/// The user's founding ruling for this Epic: adding a source from the aggregate
/// scene card *"would muddy the fact that the source must be associated with an
/// object in the world."* A citation documents an **object** — a map's image
/// attribution belongs to the map, a chronicle's provenance to the chronicle.
/// Creating from a scene-scoped aggregate inverts the model the graph is built on
/// (Doc 1 §3.4).
///
/// ## ⚠️ Two things here are NOT in AC8, and both are deliberate
///
/// 1. **Attach an EXISTING source** — the mirror of I-0164, named at SP-120
///    planning rather than after a writer hit it. A writer citing one work across
///    three characters must not be made to create it three times. It is an
///    *operation*, which is exactly the class SP-119's field-only S11 missed.
/// 2. **The second entry point to `CitationPopover`** — §3.1.1 requires *"one
///    popup implementation, two entry points"*, owed since SP-102 and, until
///    SP-120, owned by **no acceptance criterion and no sprint**. `grep` found the
///    popup referenced by exactly one file: the card that declares it.
///
/// ## ⚠️ The popup is REUSED, never reimplemented
///
/// `CitationPopover` is imported from `SourcesCard.swift` as-is. A second
/// implementation would satisfy a screenshot and violate the design — and would
/// drift the moment either copy changed.
///
/// ## Host-independence (S8)
///
/// Like the sheet and the relations section, this reads nothing from its host.
struct ObjectSourcesSection: View {

    let engine: ScriviEngine
    let projectRootPath: String
    /// The object being documented.
    let objectID: String
    let objectDisplayName: String
    let authorshipRef: AuthorshipRef
    /// ⚠️ True when this object's own world is away. The graph is frozen toward an
    /// unavailable world in **both** directions (Doc 3 §4.6 rule 1), so a pending
    /// object may not gain citations either — the same gate the relations section
    /// applies, honoured here rather than re-decided (S11 §5.1).
    let isReadOnly: Bool
    /// R5/R7 — push-navigate to the source's own sheet, so it can be edited after
    /// creation. The sheet owns the history.
    let onNavigate: (ObjectDetailHistory.Entry) -> Void
    /// Announces a graph change so the host can refresh other surfaces.
    var onChanged: () -> Void = {}

    /// The relation type seeded by `RelationTypeStore` (SP-096/T-0373), and the
    /// one project-scoped kind (SP-098/T-0406).
    ///
    /// ⚠️ Referenced from `SourcesCardModel` rather than restated, so the two
    /// surfaces cannot disagree about what a citation *is*.
    private var citesType: String { SourcesCardModel.citesType }
    private var sourceKind: String { SourcesCardModel.sourceKind }

    @State private var citations: [EdgeView] = []
    @State private var loadError: String?
    @State private var actionError: String?
    /// ⚠️ Set when the source was created but the `cites` edge was not attached
    /// (S11 §5.3). Distinct from `actionError` because it reports a PARTIAL
    /// success — the writer's typing is on disk and must not be described as lost.
    @State private var partialWarning: String?
    @State private var showCreate = false
    @State private var showAttach = false
    @State private var draftTitle = ""
    @State private var detailEntry: SourceEntry?
    @State private var isWorking = false

    var body: some View {
        VStack(alignment: .leading, spacing: 6) {
            header

            if let loadError {
                Label(loadError, systemImage: "exclamationmark.triangle")
                    .font(.caption)
                    .foregroundStyle(.orange)
            }

            if let partialWarning {
                // ⚠️ S11 §5.3 — the source EXISTS. Never worded as a failure that
                // lost the writer's work, because it did not.
                Label(partialWarning, systemImage: "exclamationmark.circle")
                    .font(.caption)
                    .foregroundStyle(.orange)
            }

            if let actionError {
                Label(actionError, systemImage: "exclamationmark.triangle")
                    .font(.caption)
                    .foregroundStyle(.orange)
            }

            if citations.isEmpty {
                // ⚠️ An uncited object is a NORMAL state, worded as such — not an
                // error and not a nag. The same rule the aggregate card follows.
                Text(isReadOnly
                     ? "No sources cited."
                     : "No sources cited yet.")
                    .font(.callout)
                    .foregroundStyle(.secondary)
            } else {
                ForEach(citations) { edge in
                    row(edge)
                }
            }
        }
        .task(id: objectID) { load() }
        // ⚠️ T-0456 — the SECOND entry point, opening the SAME popup as the
        // aggregate card (§3.1.1).
        .popover(item: $detailEntry) { entry in
            CitationPopover(detail: detail(for: entry),
                            attribution: attributionSentence(forSourceID: entry.sourceID))
        }
    }

    // MARK: — Chrome

    private var header: some View {
        HStack(spacing: 8) {
            Text("Sources")
                .font(.caption.weight(.semibold))
                .foregroundStyle(.secondary)

            Spacer()

            // ⚠️ T-0455 — attach an EXISTING source. Listed FIRST because reusing
            // a source is the commoner act in a research-heavy project, and
            // because putting "New" first is what trains a writer to create
            // duplicates.
            Button { showAttach = true } label: {
                Label("Cite…", systemImage: "link")
                    .font(.caption)
            }
            .buttonStyle(.borderless)
            .disabled(isReadOnly || isWorking)
            // ⚠️ Disabled AND explained (§7.2) — never merely greyed out.
            .help(isReadOnly
                  ? "This object's world is unavailable, so its citations are frozen."
                  : "Cite a source that already exists in this project")
            .popover(isPresented: $showAttach, arrowEdge: .bottom) {
                ExistingSourcePicker(
                    engine: engine,
                    projectRootPath: projectRootPath,
                    alreadyCitedIDs: Set(citations.map(\.otherID)),
                    onPick: { sourceID in
                        showAttach = false
                        attach(sourceID: sourceID)
                    }
                )
            }

            Button { draftTitle = ""; showCreate = true } label: {
                Label("New Source…", systemImage: "plus.circle")
                    .font(.caption)
            }
            .buttonStyle(.borderless)
            .disabled(isReadOnly || isWorking)
            .help(isReadOnly
                  ? "This object's world is unavailable, so its citations are frozen."
                  : "Create a source documenting “\(objectDisplayName)”")
            .popover(isPresented: $showCreate, arrowEdge: .bottom) {
                createForm
            }
        }
    }

    /// ⚠️ Title only. The remaining citation fields are edited on the source's own
    /// Detail Sheet, reached by double-clicking the row this creates.
    ///
    /// A six-field form here would be a **second editor for an object that already
    /// has one** — two places to change the same data, which is how they drift.
    /// The sheet already edits every field; this only has to bring the source into
    /// existence and attach it.
    private var createForm: some View {
        VStack(alignment: .leading, spacing: 8) {
            Text("New source")
                .font(.headline)
            Text("Documenting “\(objectDisplayName)”.")
                .font(.caption)
                .foregroundStyle(.secondary)

            TextField("Title of the work", text: $draftTitle)
                .textFieldStyle(.roundedBorder)
                .frame(minWidth: 260)
                .onSubmit { if canCreate { create() } }

            Text("Author, year and the rest are edited on the source itself — "
                 + "double-click it once it appears.")
                .font(.caption2)
                .foregroundStyle(.secondary)
                .fixedSize(horizontal: false, vertical: true)

            HStack {
                Spacer()
                Button("Cancel") { showCreate = false }
                Button("Create") { create() }
                    .keyboardShortcut(.defaultAction)
                    .disabled(!canCreate)
            }
        }
        .padding(12)
        .frame(maxWidth: 320)
    }

    private var canCreate: Bool {
        !draftTitle.trimmingCharacters(in: .whitespacesAndNewlines).isEmpty && !isWorking
    }

    @ViewBuilder
    private func row(_ edge: EdgeView) -> some View {
        HStack(spacing: 6) {
            Image(systemName: "text.book.closed")
                .font(.caption)
                .foregroundStyle(.secondary)
                .accessibilityHidden(true)

            Text(edge.otherDisplayName)
                .font(.callout)
                .frame(maxWidth: .infinity, alignment: .leading)

            Spacer(minLength: 4)
        }
        .contentShape(Rectangle())
        // ⚠️ T-0456: single click opens the citation record — the SAME popup the
        // aggregate card opens. Double-click navigates to the source's own sheet,
        // matching R7's verb everywhere else in this Epic.
        //
        // ⚠️ **ORDER IS LOAD-BEARING: `count: 2` must be registered FIRST.**
        // Declared the other way round, the single-tap gesture consumes the event
        // and the double-click never fires — so "Show Source" would be dead on
        // the row while working from the context menu, which is precisely the
        // shape of the four navigation defects SP-118 paid for. This is the order
        // `ObjectRelationsSection` already uses; copied, not re-derived
        // (`feedback_look_for_existing_pattern_first`).
        .onTapGesture(count: 2) { navigate(to: edge) }
        .onTapGesture { detailEntry = entry(for: edge) }
        .contextMenu {
            Button("Show Citation") { detailEntry = entry(for: edge) }
            Button("Show Source") { navigate(to: edge) }
            Divider()
            Button("Remove Citation", role: .destructive) { uncite(edge) }
                .disabled(isReadOnly)
        }
        .accessibilityElement(children: .combine)
        .accessibilityLabel("Cites \(edge.otherDisplayName)")
    }

    // MARK: — Reads

    private func entry(for edge: EdgeView) -> SourceEntry {
        // `viaObjects` is the citing object — this one. It is what
        // `SourceEntry.attribution` would render, and it is correct data even
        // though this surface passes its own wording to the popup below.
        SourceEntry(sourceID: edge.otherID,
                    displayName: edge.otherDisplayName,
                    viaObjects: [objectDisplayName])
    }

    /// ⚠️ **The wording is deliberately NOT the card's.**
    ///
    /// `SourceEntry.attribution` renders *"via Alanna Vex"*, which answers *"why
    /// is this source showing up on this scene?"* — a real question when the path
    /// is scene → object → source. Opened **on the citing object itself** that
    /// sentence is circular: the writer is looking at Alanna Vex.
    ///
    /// ⚠️ So the wording is chosen by the CALLER and passed in, which is why
    /// `CitationPopover` takes `attribution` as a parameter rather than reading it
    /// off the entry. **One popup, two entry points, two sentences** — the popup
    /// itself stays unchanged, which is what §3.1.1 requires.
    ///
    /// ## ⚠️ I-0170 — it must name the OTHER citing objects too
    ///
    /// Found on real data: one source cited **Myton at 23** *and* **Tintagael**,
    /// and this sentence named only whichever sheet was open — ⚠️ **silently
    /// under-reporting the writer's own graph.**
    ///
    /// The aggregate card had this right all along (*"via Myton at 23,
    /// Tintagael"*, T-0365). ⚠️ **The SP-120 decision to reword was correct; the
    /// implementation of it dropped information instead of re-centring it.** So:
    /// lead with this object, because that is what the writer is reading, and
    /// then say who else cites it.
    private func attributionSentence(forSourceID sourceID: String) -> String {
        let others = otherCitingNames(forSourceID: sourceID)
        guard !others.isEmpty else { return "Cited by \(objectDisplayName)" }
        return "Cited by \(objectDisplayName), and by "
             + others.joined(separator: ", ")
    }

    /// Every OTHER object citing this source, by name.
    ///
    /// ⚠️ Walks from the SOURCE outward — the reverse of `load()`, which walks
    /// from this object. `listEdgesFor` resolves the far end regardless of stored
    /// direction, so this never reasons about which way the edge was written.
    ///
    /// ⚠️ **A pending far endpoint is NAMED, not skipped** — `otherDisplayName` is
    /// cached on the edge precisely so an absent object still has a name, and
    /// omitting it would under-report the graph a second way (Doc 3: absence is
    /// never deletion).
    private func otherCitingNames(forSourceID sourceID: String) -> [String] {
        guard let result = try? engine.listEdgesFor(
            projectRootPath: projectRootPath, endpointID: sourceID) else { return [] }
        var seen = Set<String>()
        return result.edges.compactMap { e in
            guard e.relationType == citesType,
                  e.otherID != sourceID,
                  e.otherID != objectID,
                  !e.otherDisplayName.isEmpty,
                  seen.insert(e.otherID).inserted else { return nil }
            return e.otherDisplayName
        }
    }

    /// Read one source's full record for the popup.
    ///
    /// ⚠️ Reuses `SourcesCardModel` rather than duplicating the read — it already
    /// handles the `attributes` wire form and the could-not-be-read case.
    private func detail(for entry: SourceEntry) -> SourceDetail {
        SourcesCardModel(engine: engine,
                         projectRootPath: projectRootPath,
                         sceneID: "").detail(for: entry)
    }

    private func load() {
        guard !objectID.isEmpty, !projectRootPath.isEmpty else {
            citations = []
            return
        }
        do {
            let edges = try engine.listEdgesFor(
                projectRootPath: projectRootPath, endpointID: objectID).edges
            // ⚠️ `cites` is unconstrained on BOTH ends, so an edge of this type
            // touching this object could point at a non-source. Take only the
            // source end, and never this object itself.
            citations = edges.filter {
                $0.relationType == citesType
                && $0.otherID != objectID
                && $0.otherKind == sourceKind
            }
            loadError = nil
        } catch {
            citations = []
            loadError = (error as? ScriviError)?.message ?? "\(error)"
        }
    }

    // MARK: — Writes

    /// ⚠️ **Two writes, one intent** (S11 §5.3).
    ///
    /// `create_object` then `create_edge`, with no transaction across them. If the
    /// object lands and the edge does not, **the source is KEPT** and the writer
    /// is told plainly.
    ///
    /// ⚠️ Rolling back would delete her typing to tidy up a failure she did not
    /// cause, and an unlinked source is a bibliography entry, not debris
    /// (`project_unlinked_world_objects_are_normal`). She can cite it with the
    /// "Cite…" button the moment the cause is fixed.
    private func create() {
        let title = draftTitle.trimmingCharacters(in: .whitespacesAndNewlines)
        guard !title.isEmpty else { return }
        showCreate = false
        isWorking = true
        defer { isWorking = false }

        partialWarning = nil
        actionError = nil

        let created: CreateObjectResult
        do {
            // ⚠️ The scope is DERIVED from the core's own kind table (D5 /
            // `ObjectKindScope`), never from a restated list — the standing rule,
            // and a Swift restatement of exactly this shape blocked object
            // creation outright in SP-104.
            //
            // `source` is project-scoped today, so this resolves to "". ⚠️ If it
            // ever became world-scoped, a hardcoded "" would silently create it
            // in the wrong place; this asks instead. There is no world to name
            // here, so the world-scoped branch reports rather than guessing.
            guard !ObjectKindScope.isWorldScoped(sourceKind) else {
                actionError = "Sources are no longer project-scoped in this build, "
                    + "so one cannot be created from here."
                return
            }
            created = try engine.createObject(
                projectRootPath: projectRootPath,
                objectKind: sourceKind,
                displayName: title,
                authorshipRef: authorshipRef,
                worldID: ""
            )
        } catch {
            actionError = "The source could not be created: "
                + ((error as? ScriviError)?.message ?? "\(error)")
            return
        }

        do {
            _ = try engine.createEdge(
                projectRootPath: projectRootPath,
                fromID: created.objectID,
                toID: objectID,
                relationTypeCode: citesType
            )
        } catch {
            // ⚠️ The source EXISTS. Say so, name it, and say what did not happen.
            partialWarning = "“\(title)” was created, but could not be attached to "
                + "“\(objectDisplayName)”: "
                + ((error as? ScriviError)?.message ?? "\(error)")
                + " The source is saved — use “Cite…” to attach it."
            load()
            onChanged()
            return
        }

        load()
        onChanged()
    }

    /// T-0455 — cite a source that already exists.
    private func attach(sourceID: String) {
        isWorking = true
        defer { isWorking = false }
        do {
            _ = try engine.createEdge(
                projectRootPath: projectRootPath,
                fromID: sourceID,
                toID: objectID,
                relationTypeCode: citesType
            )
            actionError = nil
            partialWarning = nil
            load()
            onChanged()
        } catch {
            // ⚠️ ScriviCore normalizes direction and rejects duplicates, so
            // "already cited" arrives as an error rather than a second row.
            actionError = (error as? ScriviError)?.message ?? "\(error)"
        }
    }

    /// Detach a citation. ⚠️ Removes the EDGE, never the source object — S11 §5.1:
    /// an uncited source is a bibliography entry, and object deletion has no
    /// surface anywhere in Scrivi.
    private func uncite(_ edge: EdgeView) {
        do {
            _ = try engine.deleteEdge(projectRootPath: projectRootPath, edgeID: edge.edgeID)
            actionError = nil
            load()
            onChanged()
        } catch {
            actionError = (error as? ScriviError)?.message ?? "\(error)"
        }
    }

    private func navigate(to edge: EdgeView) {
        onNavigate(ObjectDetailHistory.Entry(
            objectID: edge.otherID,
            kind: sourceKind,
            // ⚠️ Project-scoped: a source has no world, and saying otherwise
            // would send `openObject` looking for one.
            worldID: "",
            displayName: edge.otherDisplayName
        ))
    }
}

// MARK: — T-0455: cite a source that already exists

/// Pick a source already in this project and cite it from this object
/// (EP-034 SP-120, **T-0455**).
///
/// ## ⚠️ Why this exists when AC8 does not ask for it
///
/// It is the **mirror of I-0164** — *"attach an asset already in the world"* —
/// which SP-119 shipped only after a writer hit the gap. I-0164 got through
/// because that sprint's S11 enumerated **fields and not operations**; SP-120's
/// S11 was widened to operations for exactly this reason, and this is the item it
/// caught (S11 §5.1).
///
/// A writer citing one work across three characters must not be made to create it
/// three times. Without this the only path to a citation is *create a new source*,
/// which quietly builds a bibliography full of duplicates.
///
/// ## ⚠️ Deliberately simpler than `ObjectRelationPicker`
///
/// No relation-type picker (the type is always `cites`) and no world grouping
/// (`source` is the one project-scoped kind, so every row is in the same scope).
/// Reusing the object picker would have meant *widening* it with a
/// single-kind-single-type mode — more code, in a shared component, to show less.
struct ExistingSourcePicker: View {

    let engine: ScriviEngine
    let projectRootPath: String
    /// Sources this object already cites — shown, but not selectable.
    ///
    /// ⚠️ The core refuses a duplicate edge anyway; saying so up front is kinder
    /// than an error, which is the rule `ObjectRelationPicker` already follows.
    let alreadyCitedIDs: Set<String>
    let onPick: (String) -> Void

    @State private var sources: [ObjectEntry] = []
    @State private var query = ""
    @State private var loadError: String?

    var body: some View {
        VStack(alignment: .leading, spacing: 8) {
            Text("Cite an existing source")
                .font(.headline)

            if let loadError {
                Label(loadError, systemImage: "exclamationmark.triangle")
                    .font(.caption)
                    .foregroundStyle(.orange)
            }

            TextField("Search sources…", text: $query)
                .textFieldStyle(.roundedBorder)
                .font(.callout)

            if filtered.isEmpty {
                // ⚠️ The empty state distinguishes "none exist" from "none match".
                // They call for different actions, and one message for both leaves
                // the writer guessing which she is looking at.
                Text(sources.isEmpty
                     ? "This project has no sources yet. Use “New Source…” to create one."
                     : "No match.")
                    .font(.caption)
                    .foregroundStyle(.secondary)
                    .frame(minHeight: 180, alignment: .top)
                    .fixedSize(horizontal: false, vertical: true)
            } else {
                ScrollView {
                    LazyVStack(alignment: .leading, spacing: 2) {
                        ForEach(filtered) { source in
                            row(source)
                        }
                    }
                }
                // ⚠️ I-0127: a MINIMUM reserves the space. `maxHeight` alone
                // collapses inside a popover, which sizes to its content.
                .frame(minHeight: 180, maxHeight: 320)
            }
        }
        .padding(12)
        .frame(width: 320)
        .task { load() }
    }

    @ViewBuilder
    private func row(_ source: ObjectEntry) -> some View {
        let cited = alreadyCitedIDs.contains(source.objectID)
        Button {
            onPick(source.objectID)
        } label: {
            HStack(spacing: 6) {
                Image(systemName: "text.book.closed")
                    .font(.caption)
                    .foregroundStyle(.secondary)
                    .accessibilityHidden(true)
                VStack(alignment: .leading, spacing: 1) {
                    Text(source.displayName)
                        .font(.callout)
                    if cited {
                        Text("Already cited here")
                            .font(.caption2)
                            .foregroundStyle(.secondary)
                    }
                }
                Spacer(minLength: 0)
            }
            .contentShape(Rectangle())
            .padding(.vertical, 2)
        }
        .buttonStyle(.plain)
        .disabled(cited)
        .help(cited ? "“\(source.displayName)” is already cited by this object."
                    : "Cite “\(source.displayName)”")
    }

    private var filtered: [ObjectEntry] {
        let q = query.trimmingCharacters(in: .whitespacesAndNewlines)
        guard !q.isEmpty else { return sources }
        return sources.filter { $0.displayName.localizedCaseInsensitiveContains(q) }
    }

    private func load() {
        do {
            sources = try engine.listObjects(
                projectRootPath: projectRootPath,
                kind: SourcesCardModel.sourceKind).objects
            loadError = nil
        } catch {
            sources = []
            loadError = (error as? ScriviError)?.message ?? "\(error)"
        }
    }
}
