import SwiftUI

// T-0387 — the object picker (EP-031 SP-099).
// Design: Scrivi_Scene_Inspector_Card_Framework_v0_1.md §C5, §4.6, §9 AC17.
//
// ⚠️ AC17: the picker lists objects from ALL WORLDS in the project, deliberately
// UNFILTERED by the scene's world association (World Data Separation §4.1). A
// writer relating a character to a scene must be able to reach a character that
// lives in another world — filtering by the scene's world would silently hide
// exactly the objects she is reaching for.
//
// Two entrances, one operation (C5): inline type-ahead for the fast path, and
// this list for browsing. Both call createEdge; ScriviCore normalizes direction
// and rejects duplicates.

struct ObjectPickerView: View {
    let engine: ScriviEngine
    let projectRootPath: String
    let cardKind: ObjectCardKind
    /// Objects already related to this scene — shown, but not addable twice.
    let existingObjectIDs: Set<String>
    /// Opens the card's in-place edit state, carrying whatever the writer typed
    /// (§4.6). The picker never creates anything itself and never opens a dialog.
    let onCreateNew: (String) -> Void
    /// (objectID, relationTypeCode)
    let onSelect: (String, String) -> Void

    @State private var query: String = ""
    @State private var objects: [ObjectEntry] = []
    @State private var worldNames: [String: String] = [:]
    @State private var relationType: String = ""
    @State private var loadError: String?

    var body: some View {
        VStack(alignment: .leading, spacing: 8) {
            Text("Add \(cardKind.title)")
                .font(.headline)

            TextField("Search all worlds…", text: $query)
                .textFieldStyle(.roundedBorder)
                .font(.callout)

            if let loadError {
                CardErrorView(message: loadError)
            }

            // Where these objects live. Without this a writer cannot tell whether a
            // kind belongs to a world at all — the confusion this line exists to fix.
            Text(scopeDescription)
                .font(.caption2)
                .foregroundStyle(.secondary)

            if filtered.isEmpty {
                VStack(alignment: .leading, spacing: 4) {
                    Text(objects.isEmpty
                         ? "No \(cardKind.title.lowercased()) in this project yet."
                         : "No match.")
                        .font(.caption)
                        .foregroundStyle(.secondary)
                }
                // Matches the list's minimum so the popover keeps one size as the
                // writer types — a box that resizes on every keystroke is its own
                // kind of unusable.
                .frame(minHeight: 180, alignment: .top)
            } else {
                ScrollView {
                    LazyVStack(alignment: .leading, spacing: 2) {
                        ForEach(grouped, id: \.world) { group in
                            if group.world != Self.projectScopeKey {
                                Text(worldNames[group.world] ?? group.world)
                                    .font(.caption2.weight(.semibold))
                                    .foregroundStyle(.secondary)
                                    .padding(.top, 4)
                            }
                            ForEach(group.objects) { object in
                                pickerRow(object)
                            }
                        }
                    }
                }
                // ⚠️ **`maxHeight` alone collapsed this to a sliver** (I-0127). A
                // `ScrollView` has no intrinsic height, and inside a popover — which
                // sizes to its content rather than being given a height — a bare
                // maximum let it shrink to barely one row. The writer could see a
                // fraction of a single object and had to scroll blind through a list
                // she could not survey, which defeats the point of an unfiltered
                // picker (AC17).
                //
                // A MINIMUM is what actually reserves the space; the maximum then
                // caps it so a large world does not produce an unusable popover.
                .frame(minHeight: 180, maxHeight: 320)
            }

            Divider()

            // §4.6: this does NOT create anything and does NOT open a dialog — it
            // hands the typed text to the card's in-place edit state (T-0388).
            Button {
                onCreateNew(query.trimmingCharacters(in: .whitespacesAndNewlines))
            } label: {
                Label(createLabel, systemImage: "plus.circle")
                    .font(.caption)
            }
            .buttonStyle(.borderless)
        }
        .padding(12)
        .frame(width: 320)
        .task { load() }
    }

    /// Names what will be created when the writer has typed a search that matched
    /// nothing — §C5's "type a name that matches nothing" path.
    private var createLabel: String {
        let typed = query.trimmingCharacters(in: .whitespacesAndNewlines)
        return typed.isEmpty ? "Create new…" : "Create “\(typed)”…"
    }

    /// Says plainly where this kind's objects live. A writer looking at Characters
    /// should not have to infer that they belong to the project rather than a world.
    private var scopeDescription: String {
        guard cardKind.isWorldScoped else {
            return "\(cardKind.title) belong to this project, not to a world."
        }
        if worldNames.isEmpty {
            return "\(cardKind.title) live in a world — add one from the Worlds menu."
        }
        let names = worldNames.values.sorted().joined(separator: ", ")
        return "\(cardKind.title) live in a world: \(names)."
    }

    @ViewBuilder
    private func pickerRow(_ object: ObjectEntry) -> some View {
        let already = existingObjectIDs.contains(object.objectID)
        Button {
            onSelect(object.objectID, resolvedRelationType)
        } label: {
            HStack(spacing: 4) {
                Text(object.displayName)
                    .font(.callout)
                Spacer(minLength: 4)
                if already {
                    // Adding it again would be rejected as a duplicate by
                    // ScriviCore anyway (AC21); saying so up front is kinder than
                    // an error after the click.
                    Image(systemName: "checkmark")
                        .font(.caption2)
                        .foregroundStyle(.secondary)
                }
            }
            .contentShape(Rectangle())
        }
        .buttonStyle(.borderless)
        .disabled(already)
    }

    // MARK: — Data

    private static let projectScopeKey = ""

    private var filtered: [ObjectEntry] {
        let q = query.trimmingCharacters(in: .whitespacesAndNewlines)
        guard !q.isEmpty else { return objects }
        return objects.filter {
            $0.displayName.localizedCaseInsensitiveContains(q)
        }
    }

    /// Grouped by world (C5), with project-scoped objects first. Grouping is a
    /// display choice; it never removes anything from the list.
    private var grouped: [(world: String, objects: [ObjectEntry])] {
        let buckets = Dictionary(grouping: filtered, by: \.worldID)
        return buckets
            .map { (world: $0.key, objects: $0.value.sorted {
                $0.displayName.localizedCaseInsensitiveCompare($1.displayName) == .orderedAscending
            }) }
            .sorted { lhs, rhs in
                if lhs.world == Self.projectScopeKey { return true }
                if rhs.world == Self.projectScopeKey { return false }
                return lhs.world < rhs.world
            }
    }

    /// The relation type to create with. Resolved against the project's LIVE
    /// vocabulary rather than assuming the seed, so a project that defines its own
    /// types is honoured; falls back to the kind's preferred code.
    private var resolvedRelationType: String {
        relationType.isEmpty ? cardKind.preferredRelationType : relationType
    }

    private func load() {
        do {
            // AC17: every object of this kind, across every world. No filtering by
            // the scene's world association.
            objects = try engine.listObjects(
                projectRootPath: projectRootPath, kind: cardKind.kind
            ).objects

            // World display names, so a group header reads "Midgard" not an ID.
            let worlds = try engine.listWorlds(projectRootPath: projectRootPath).worlds
            worldNames = Dictionary(uniqueKeysWithValues:
                worlds.map { ($0.worldID, $0.displayName) })

            // Pick a relation type that actually accepts this kind → scene.
            let types = try engine.listRelationTypes(projectRootPath: projectRootPath).types
            if let exact = types.first(where: { $0.code == cardKind.preferredRelationType }),
               exact.acceptsSource(kind: cardKind.kind) {
                relationType = exact.code
            } else if let usable = types.first(where: {
                // ⚠️ This picker's target IS a scene — that is what the card does,
                // and the filter is correct here. The object→object case does NOT
                // reuse this line: it needs types whose target is NOT a scene, and
                // narrowing it there would have offered the writer nothing.
                // `ObjectRelationPicker` (SP-118 T-0443) carries that half.
                $0.acceptsSource(kind: cardKind.kind) && $0.targetAcceptsScene
            }) {
                relationType = usable.code
            } else {
                relationType = cardKind.preferredRelationType
            }
            loadError = nil
        } catch {
            objects = []
            loadError = error.localizedDescription
        }
    }
}
