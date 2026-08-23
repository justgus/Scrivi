import SwiftUI

/// Picks a **relation type** and a **target object** for an object→object
/// relationship, created from the Detail Sheet (EP-034 SP-118, T-0443; trade
/// **D4-A**). Closes the creation half of **AC6**.
///
/// ## ⚠️ Why this is not `ObjectPickerView`
///
/// D4-A says *"reuse the picker, do not write a second one"*, and the reusable
/// part — AC17's unfiltered all-worlds listing, the `minHeight` that I-0127 paid
/// for, world grouping — is reused here deliberately and verbatim in shape.
///
/// ⚠️ **What could not be reused is the frame.** `ObjectPickerView` is bound to
/// one `ObjectCardKind` and one **scene** target: it titles itself "Add
/// Characters", filters to a single kind, hands typed text to a *card's* draft
/// state, and picks its relation type by asking which types accept
/// `kind → scene`. Object→object inverts every one of those: the kind is not
/// known in advance (the writer chooses *what* to relate to), the target is not a
/// scene, and there is no card draft to hand anything to. Parameterising one view
/// over both would have left a picker whose every property was conditional on a
/// mode flag — more coupling than the duplication it saved.
///
/// ⚠️ The one change T-0443 named **was** made in `ObjectPickerView`: its
/// hardcoded `$0.targetKind == "scene"` is now the named `targetAcceptsScene`,
/// with `targetAcceptsObject` as this file's counterpart. ✅ **No ABI change** —
/// the C ABI already round-trips the scene endpoint as the sentinel `"scene"`.
///
/// ## ⚠️ AC17 holds here too
///
/// The list is **deliberately unfiltered by world**. A writer relating an object
/// must be able to reach one that lives in another world; filtering by the source
/// object's world would silently hide exactly what she is reaching for.
struct ObjectRelationPicker: View {

    let engine: ScriviEngine
    let projectRootPath: String
    /// The object the relationship is created **from**.
    let sourceObjectID: String
    /// Already related — shown with a check, not hidden. ScriviCore would reject
    /// the duplicate anyway (AC21); saying so up front is kinder than an error
    /// after the click.
    let existingObjectIDs: Set<String>
    /// (targetObjectID, relationTypeCode)
    let onCreate: (String, String) -> Void

    @State private var query = ""
    @State private var objects: [ObjectEntry] = []
    @State private var worldNames: [String: String] = [:]
    @State private var types: [RelationTypeEntry] = []
    @State private var selectedType: String = ""
    @State private var sourceKind: String = ""
    @State private var loadError: String?

    var body: some View {
        VStack(alignment: .leading, spacing: 8) {
            Text("Relate to…")
                .font(.headline)

            if let loadError {
                Label(loadError, systemImage: "exclamationmark.triangle")
                    .font(.caption)
                    .foregroundStyle(.orange)
            }

            typePicker

            TextField("Search all worlds…", text: $query)
                .textFieldStyle(.roundedBorder)
                .font(.callout)

            // AC17, said out loud. Without this a writer cannot tell whether the
            // list is scoped to one world or spans all of them.
            Text("Objects from every world in this project are listed.")
                .font(.caption2)
                .foregroundStyle(.secondary)

            if usableTypes.isEmpty {
                // ⚠️ Explained, not empty. Before T-0441 this was the *silent*
                // shape of the I-0125 defect in an old project.
                Text("This project has no relation type that can link two objects.")
                    .font(.caption)
                    .foregroundStyle(.secondary)
                    .frame(minHeight: 180, alignment: .top)
            } else if filtered.isEmpty {
                Text(objects.isEmpty
                     ? "There are no other objects in this project yet."
                     : "No match.")
                    .font(.caption)
                    .foregroundStyle(.secondary)
                    // Matches the list's minimum so the popover keeps one size as
                    // the writer types.
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
                // ⚠️ I-0127: a MINIMUM is what reserves the space. `maxHeight`
                // alone collapsed this to a sliver inside a popover, which sizes
                // to its content — the writer saw a fraction of one row and had to
                // scroll blind through a list she could not survey.
                .frame(minHeight: 180, maxHeight: 320)
            }
        }
        .padding(12)
        .frame(width: 340)
        .task { load() }
    }

    // MARK: — Type selection

    @ViewBuilder
    private var typePicker: some View {
        if usableTypes.count > 1 {
            Picker("Relationship", selection: $selectedType) {
                ForEach(usableTypes) { type in
                    // The FORWARD label: this relationship reads from the source
                    // object outward. ⚠️ The core supplies both labels; which one
                    // reads correctly from an endpoint is its answer, never ours.
                    Text(type.forwardLabel).tag(type.code)
                }
            }
            .font(.callout)
            .labelsHidden()
            .pickerStyle(.menu)
        } else if let only = usableTypes.first {
            Text(only.forwardLabel)
                .font(.callout.weight(.medium))
                .foregroundStyle(.secondary)
        }
    }

    @ViewBuilder
    private func pickerRow(_ object: ObjectEntry) -> some View {
        let already = existingObjectIDs.contains(object.objectID)
        // ⚠️ The chosen type may constrain its target kind (`located-at` →
        // `location`). An object the type cannot accept is shown but not
        // selectable, so the writer sees why rather than wondering where it went.
        let accepted = selected?.acceptsTarget(kind: object.kind) ?? true

        Button {
            onCreate(object.objectID, resolvedType)
        } label: {
            HStack(spacing: 4) {
                Text(object.displayName)
                    .font(.callout)
                Text(object.kind.capitalized)
                    .font(.caption2)
                    .foregroundStyle(.tertiary)
                Spacer(minLength: 4)
                if already {
                    Image(systemName: "checkmark")
                        .font(.caption2)
                        .foregroundStyle(.secondary)
                }
            }
            .contentShape(Rectangle())
        }
        .buttonStyle(.borderless)
        .disabled(already || !accepted)
        // Disabled AND explained (§7.2) — a greyed row that says nothing is the
        // I-0138 defect, not the fix.
        .help(already
              ? "Already related."
              : (accepted
                 ? "Relate to \(object.displayName)"
                 : "“\(selected?.forwardLabel ?? "This relationship")” cannot point at a \(object.kind)."))
    }

    // MARK: — Data

    private static let projectScopeKey = ""

    /// Types that can relate THIS object to another object.
    ///
    /// ⚠️ Three conditions, each load-bearing:
    /// - the source end accepts this object's kind;
    /// - the source end is not constrained to a **scene** (`located-at` is seeded
    ///   scene→location and cannot be created from an object);
    /// - the target end is not the scene sentinel — this is T-0443's actual
    ///   widening, since `ObjectPickerView` asks the opposite question.
    private var usableTypes: [RelationTypeEntry] {
        types.filter {
            $0.acceptsSource(kind: sourceKind)
                && !$0.sourceIsScene
                && $0.targetAcceptsObject
        }
    }

    private var selected: RelationTypeEntry? {
        usableTypes.first { $0.code == selectedType } ?? usableTypes.first
    }

    private var resolvedType: String {
        selected?.code ?? selectedType
    }

    private var filtered: [ObjectEntry] {
        let q = query.trimmingCharacters(in: .whitespacesAndNewlines)
        // ⚠️ Never offer the object itself — ScriviCore rejects a self-edge, but
        // listing it invites a click that can only fail.
        let others = objects.filter { $0.objectID != sourceObjectID }
        guard !q.isEmpty else { return others }
        return others.filter { $0.displayName.localizedCaseInsensitiveContains(q) }
    }

    /// Grouped by world, project-scoped first. Grouping is a display choice; it
    /// never removes anything from the list (AC17).
    private var grouped: [(world: String, objects: [ObjectEntry])] {
        Dictionary(grouping: filtered, by: \.worldID)
            .map { (world: $0.key, objects: $0.value.sorted {
                $0.displayName.localizedCaseInsensitiveCompare($1.displayName) == .orderedAscending
            }) }
            .sorted { lhs, rhs in
                if lhs.world == Self.projectScopeKey { return true }
                if rhs.world == Self.projectScopeKey { return false }
                return lhs.world < rhs.world
            }
    }

    private func load() {
        do {
            // AC17: every object, every kind, every world. No filtering.
            objects = try engine.listObjects(projectRootPath: projectRootPath).objects

            // This object's own kind, read from the index rather than guessed —
            // it decides which types can start here.
            sourceKind = objects.first { $0.objectID == sourceObjectID }?.kind ?? ""

            let worlds = try engine.listWorlds(projectRootPath: projectRootPath).worlds
            worldNames = Dictionary(uniqueKeysWithValues:
                worlds.map { ($0.worldID, $0.displayName) })

            // ⚠️ The project's LIVE vocabulary, never the assumed seed — a project
            // may define its own types, and since T-0441 an old project's seeded
            // types are repaired by this very call.
            types = try engine.listRelationTypes(projectRootPath: projectRootPath).types
            if selectedType.isEmpty || !usableTypes.contains(where: { $0.code == selectedType }) {
                selectedType = usableTypes.first?.code ?? ""
            }
            loadError = nil
        } catch {
            objects = []
            types = []
            loadError = (error as? ScriviError)?.message ?? "\(error)"
        }
    }
}
