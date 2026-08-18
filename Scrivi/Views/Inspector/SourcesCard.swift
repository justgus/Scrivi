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
        return SourceDetail(
            displayName: (json["displayName"] as? String) ?? entry.displayName,
            subtitle:    (json["subtitle"] as? String) ?? "",
            notes:       (json["notes"] as? String) ?? "")
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

    var body: some View {
        VStack(alignment: .leading, spacing: 6) {
            if let error = model?.loadError {
                Label(error, systemImage: "exclamationmark.triangle")
                    .font(.callout)
                    .foregroundStyle(.secondary)
            } else if let entries = model?.entries, !entries.isEmpty {
                ForEach(entries) { entry in
                    Button {
                        detailEntry = entry
                    } label: {
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
                    }
                    .buttonStyle(.plain)
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
