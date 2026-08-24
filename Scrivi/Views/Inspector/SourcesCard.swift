import SwiftUI

// MARK: — The aggregate `sources` card (T-0365, EP-031 SP-102)
//
// Design: `Scrivi_Scene_Inspector_Card_Framework_v0_1.md` §3.1.1 (ruled 2026-08-12).
//
// ⚠️ **ONE aggregate card, not a card per source.** A per-source card would flood the
// Writing stack in any project with real research, and — decisively — one card can be
// shown/hidden as a unit in the picker, which a proliferating set cannot.
//
// **Sources attach to OBJECTS, never to scenes** (Doc 1 §3.4, amended 2026-08-12). So this
// card renders an *indirect* path and never a direct scene→source edge:
//
//     this scene ──edges──▶ worldbuilding objects ──cites──▶ sources
//
// A scene with no objects, or objects with no citations, shows an **empty card** — that is
// a normal state, not an error.
//
// ⏸ **Sources do not appear IN the manuscript in this version.** Footnotes and pull quotes
// need object rendering inside scene text — Doc 1 §3.4.1, deferred to EP-032. This card is
// a **read-only view onto the graph**: it writes no scene text and implies no source→scene
// edge.

/// One source reached from this scene, with the object(s) that cite it.
///
/// ⚠️ A source reached through two objects appears **once**, listing both — the writer needs
/// to know *why* a citation is surfacing here, and two rows for one source reads as two
/// sources.
struct SourceEntry: Identifiable, Sendable {
    let sourceID:    String
    let displayName: String
    /// Display names of the objects whose `cites` edges reached this source, in the order
    /// encountered. Never empty — a source with no citing object cannot be reached at all.
    let viaObjects:  [String]

    var id: String { sourceID }

    /// "via Alanna Vex" / "via Alanna Vex, The Sunless Court"
    var attribution: String {
        "via " + viaObjects.joined(separator: ", ")
    }
}

/// Full citation detail, loaded lazily when the writer opens the popup.
///
/// The card row is a summary; **the popup is the record** (§3.1.1). Loading on demand keeps
/// the card's own fetch to the graph walk — a project with many citations should not read
/// every source file to draw a list of names.
struct SourceDetail: Sendable {
    let displayName: String
    let subtitle:    String
    let notes:       String
    /// ⚠️ T-0457 (SP-120): the structured citation fields, read from the source's
    /// `attributes` map.
    ///
    /// ⚠️ **Without this the popup could not show them.** It rendered
    /// `displayName`/`subtitle`/`notes` and nothing else, so every citation field
    /// a writer typed would have been written, stored, round-tripped — and
    /// invisible in the very popup §3.1.1 calls *"the record"*. That is
    /// `capability_without_surface` reappearing INSIDE the sprint curing it,
    /// which is why S11 named it in advance.
    var citation: CitationFields = CitationFields(attributes: [:])
}

@MainActor
@Observable
final class SourcesCardModel {
    private(set) var entries: [SourceEntry] = []
    private(set) var loadError: String?

    private let engine: ScriviEngine
    private let projectRootPath: String
    private let sceneID: String

    init(engine: ScriviEngine, projectRootPath: String, sceneID: String) {
        self.engine = engine
        self.projectRootPath = projectRootPath
        self.sceneID = sceneID
    }

    /// Walk scene → objects → sources.
    ///
    /// Two hops, because the graph has no scene→source edge by design. Both hops go through
    /// `listEdgesFor`, which returns edges in *either* direction with `otherID` already
    /// resolved to the far end — so this never has to reason about canonical edge direction.
    func load() {
        do {
            // Hop 1 — the objects related to this scene.
            let sceneEdges = try engine.listEdgesFor(
                projectRootPath: projectRootPath, endpointID: sceneID).edges

            // Hop 2 — each object's citations. Accumulate into an ordered map so one source
            // reached twice lists both objects instead of appearing twice.
            var order: [String] = []
            var names: [String: String] = [:]
            var via: [String: [String]] = [:]

            for objectEdge in sceneEdges {
                let objectID = objectEdge.otherID
                // ⚠️ A pending object's world is away, so its `cites` edges are unreadable —
                // skip it rather than reporting it as having no sources, which would be a
                // silent lie about the writer's research. The pending object itself is
                // already surfaced (badged) on its own object card; this card stays quiet
                // rather than duplicating that warning in a place it cannot act on.
                if objectEdge.otherPending { continue }

                let citations = try engine.listEdgesFor(
                    projectRootPath: projectRootPath, endpointID: objectID).edges

                for citation in citations where citation.relationType == Self.citesType {
                    // `cites` is unconstrained on both ends, so an edge from this object
                    // could point at a non-source. Take only the source end, and never the
                    // object we arrived from.
                    let farID = citation.otherID
                    guard farID != objectID,
                          citation.otherKind == Self.sourceKind else { continue }

                    if names[farID] == nil {
                        order.append(farID)
                        names[farID] = citation.otherDisplayName
                        via[farID] = []
                    }
                    let objectName = objectEdge.otherDisplayName
                    if via[farID]?.contains(objectName) == false {
                        via[farID]?.append(objectName)
                    }
                }
            }

            entries = order.map { id in
                SourceEntry(sourceID: id,
                            displayName: names[id] ?? id,
                            viaObjects: via[id] ?? [])
            }
            loadError = nil
        } catch {
            // §7.1 — a card that cannot load reports it in place; it never takes the stack
            // down with it.
            entries = []
            loadError = (error as? ScriviError)?.message ?? "\(error)"
        }
    }

    /// Read one source's full record for the citation popup.
    func detail(for entry: SourceEntry) -> SourceDetail {
        guard let opened = try? engine.openObject(
                projectRootPath: projectRootPath,
                objectKind: Self.sourceKind,
                objectID: entry.sourceID),
              let json = try? JSONSerialization.jsonObject(
                with: Data(opened.objectJson.utf8)) as? [String: Any]
        else {
            // The graph named it but the file would not open. Say so plainly rather than
            // showing an empty popup that reads as "this source has no detail".
            return SourceDetail(displayName: entry.displayName,
                                subtitle: "",
                                notes: "This source could not be read from disk.")
        }
        // ⚠️ `attributes` is an ARRAY of {"k","v"} pairs, never an object
        // (`ObjectJson.cpp:46-50`) — the same wire form `tags` uses, and the same
        // trap. `ObjectDetail` decodes it; this card parses raw JSON, so it must
        // honour the shape here too.
        var attributes: [String: String] = [:]
        if let raw = json["attributes"] as? [[String: Any]] {
            for pair in raw {
                if let k = pair["k"] as? String, let v = pair["v"] as? String, !k.isEmpty {
                    attributes[k] = v
                }
            }
        }

        return SourceDetail(
            displayName: (json["displayName"] as? String) ?? entry.displayName,
            subtitle:    (json["subtitle"] as? String) ?? "",
            notes:       (json["notes"] as? String) ?? "",
            citation:    CitationFields(attributes: attributes))
    }

    /// The relation type seeded by `RelationTypeStore` (SP-096/T-0373).
    static let citesType  = "cites"
    /// `source` is the one project-scoped object kind (SP-098/T-0406).
    static let sourceKind = "source"
}

struct SourcesCard: InspectorCard {
    static let typeID = "sources"
    static let title  = "Sources"
    static let systemImage = "quote.opening"
    /// §3.1 lists `sources` among the **writing-tool** cards: it serves the writer's
    /// research while drafting. It is offered in the "+" menu, not in any default stack.
    static let stack: InspectorStack = .writing

    func body(context: CardContext) -> AnyView {
        AnyView(SourcesCardBody(context: context))
    }
}

private struct SourcesCardBody: View {
    let context: CardContext

    @State private var model: SourcesCardModel?
    @State private var detailEntry: SourceEntry?

    /// ⚠️ I-0169 — open the source's own Detail Sheet.
    ///
    /// ⚠️ **The hook ALREADY EXISTED and this card simply never used it**:
    /// `CardContext.openObjectDetail` (`InspectorCard.swift:149-150`) carries
    /// exactly `(objectID, kind, worldID, displayName)`, and `ObjectCard` has
    /// called it since SP-117. Nothing new was needed — which is the point, and
    /// the rule (`feedback_look_for_existing_pattern_first`).
    ///
    /// ⚠️ `worldID` is **""** because `source` is the one project-scoped kind
    /// (SP-098/T-0406). Passing a world would send `openObject` looking for a
    /// package that does not hold it.
    private func openDetail(_ entry: SourceEntry) {
        context.openObjectDetail?(entry.sourceID,
                                  SourcesCardModel.sourceKind,
                                  "",
                                  entry.displayName)
    }

    var body: some View {
        VStack(alignment: .leading, spacing: 6) {
            if let error = model?.loadError {
                Label(error, systemImage: "exclamationmark.triangle")
                    .font(.callout)
                    .foregroundStyle(.secondary)
            } else if let entries = model?.entries, !entries.isEmpty {
                ForEach(entries) { entry in
                    VStack(alignment: .leading, spacing: 1) {
                        Text(entry.displayName)
                            .font(.callout)
                            .foregroundStyle(.primary)
                        // Naming the citing object is required, not decorative: without
                        // it the writer cannot tell why this source is on this scene.
                        Text(entry.attribution)
                            .font(.caption)
                            .foregroundStyle(.secondary)
                    }
                    .frame(maxWidth: .infinity, alignment: .leading)
                    .contentShape(Rectangle())
                    // ⚠️ I-0169: this card taught a DIFFERENT VERB from every other
                    // object row in the app, and it was the writer's first instinct
                    // that found it: *"I started by trying to show the data sheet
                    // from the sources card in the Scene Inspector."*
                    //
                    // A source IS an object, so it must open its sheet the way every
                    // other object does — double-click, and right-click → "View
                    // Detail" (AC1 / R7, `ObjectCard.swift:1049-1058`). Single click
                    // keeps opening the citation popup, which is this card's own
                    // established behaviour and what §3.1.1 asks of it.
                    //
                    // ⚠️ ORDER IS LOAD-BEARING: `count: 2` must be registered FIRST or
                    // the single tap consumes the event and the double-click never
                    // fires — the same trap the relations section documents.
                    .onTapGesture(count: 2) { openDetail(entry) }
                    .onTapGesture { detailEntry = entry }
                    .contextMenu {
                        // R7's discoverable half. Double-click is invisible until
                        // someone tries it; the menu is how a writer FINDS the sheet.
                        Button("View Detail", systemImage: "square.text.square") {
                            openDetail(entry)
                        }
                        Button("Show Citation", systemImage: "quote.opening") {
                            detailEntry = entry
                        }
                    }
                }
            } else {
                // A scene whose objects carry no citations is a normal state (§3.1.1) —
                // word it as such, not as a failure or an invitation to fix something.
                Text("No sources cited by this scene's objects.")
                    .font(.callout)
                    .foregroundStyle(.secondary)
            }
        }
        .frame(maxWidth: .infinity, alignment: .leading)
        // Reload when the scene changes, and when world availability changes (I-0128) —
        // a reconnected world can make previously-skipped pending objects readable.
        .task(id: "\(context.sceneID)#\(context.worldRevision)") {
            let m = SourcesCardModel(engine: context.engine,
                                     projectRootPath: context.projectRootPath,
                                     sceneID: context.sceneID)
            m.load()
            model = m
        }
        .popover(item: $detailEntry) { entry in
            CitationPopover(detail: model?.detail(for: entry)
                            ?? SourceDetail(displayName: entry.displayName,
                                            subtitle: "", notes: ""),
                            attribution: entry.attribution)
        }
    }
}

/// The citation record. **One implementation, two entry points** (§3.1.1): this card and the
/// worldbuilding-object cards both open it.
struct CitationPopover: View {
    let detail: SourceDetail
    let attribution: String

    var body: some View {
        VStack(alignment: .leading, spacing: 8) {
            Text(detail.displayName)
                .font(.headline)
            if !detail.subtitle.isEmpty {
                Text(detail.subtitle)
                    .font(.subheadline)
                    .foregroundStyle(.secondary)
            }
            Text(attribution)
                .font(.caption)
                .foregroundStyle(.secondary)

            // ⚠️ T-0457: the structured citation record. Shown above the notes
            // because it is the citation; the notes are commentary ON it.
            //
            // ⚠️ **`url` is rendered as TEXT, never as a live link** (S11 §5.2) —
            // opening an arbitrary URL out of project data is a trust decision
            // nobody has ruled, and a citation is a record, not a launcher.
            if !detail.citation.isEmpty {
                Divider()
                VStack(alignment: .leading, spacing: 3) {
                    ForEach(detail.citation.entries, id: \.label) { entry in
                        HStack(alignment: .firstTextBaseline, spacing: 6) {
                            Text(entry.label)
                                .font(.caption.weight(.semibold))
                                .foregroundStyle(.secondary)
                                .frame(width: 70, alignment: .leading)
                            Text(entry.value)
                                .font(.caption)
                                .textSelection(.enabled)
                                .frame(maxWidth: .infinity, alignment: .leading)
                        }
                    }
                }
            }

            if !detail.notes.isEmpty {
                Divider()
                ScrollView {
                    Text(detail.notes)
                        .font(.callout)
                        .frame(maxWidth: .infinity, alignment: .leading)
                        .textSelection(.enabled)
                }
                .frame(maxHeight: 220)
            }
        }
        .padding(14)
        .frame(minWidth: 260, maxWidth: 360)
    }
}
