import SwiftUI

/// [SP-237] (EP-059 C5) — a writer's relationship type, new or edited: the two labels, Same both ways, and the kinds at
/// each end. Used by Project Settings ▸ Relationship Types and by Relate…'s New Type….
///
/// ⚠️ The rules are the core's (`RelationTypeStore::upsert`): this form only greys what the core would refuse, and says
/// why. A type that edges use keeps its kinds and Same both ways (Q4); its labels may always change. The code is never
/// shown — an empty code asks the core for a new, unique one (Q3).
struct RelationTypeForm: View {

    let engine: ScriviEngine
    let projectRootPath: String
    /// nil = a new type.
    let editing: RelationTypeEntry?
    /// A new type's From kind (Relate… passes the object's own kind). "" = Any.
    var initialSourceKind: String = ""
    /// The saved type's code, or nil when cancelled.
    let onDone: (String?) -> Void

    @State private var forwardLabel = ""
    @State private var inverseLabel = ""
    @State private var symmetric = false
    @State private var sourceKind = ""
    @State private var targetKind = ""
    @State private var kinds: [String] = []
    @State private var error: String?

    private var shapeLocked: Bool { editing?.inUse ?? false }
    private func trimmed(_ s: String) -> String { s.trimmingCharacters(in: .whitespacesAndNewlines) }
    private var canSave: Bool { !trimmed(forwardLabel).isEmpty && (symmetric || !trimmed(inverseLabel).isEmpty) }

    var body: some View {
        Form {
            Section {
                TextField("Reads from this end", text: $forwardLabel, prompt: Text("mentor of"))
                Toggle("Same both ways", isOn: $symmetric)
                    .disabled(shapeLocked)
                if !symmetric {
                    TextField("Reads from the other end", text: $inverseLabel, prompt: Text("mentored by"))
                }
            } footer: {
                Text(symmetric ? "Both ends read “\(trimmed(forwardLabel))”, as “sibling of” does."
                               : "Each end of a relationship reads its own label.")
            }
            Section {
                kindPicker(symmetric ? "Between" : "From", selection: $sourceKind)
                if !symmetric { kindPicker("To", selection: $targetKind) }
            } footer: {
                if shapeLocked {
                    Text("Relationships use this type, so its kinds and Same both ways can't change. Its labels can.")
                }
            }
            if let error {
                Text(error).foregroundStyle(.red).font(.callout)
            }
        }
        .formStyle(.grouped)
        .navigationTitle(editing == nil ? "New Relationship Type" : "Edit Relationship Type")
        .toolbar {
            ToolbarItem(placement: .cancellationAction) { Button("Cancel") { onDone(nil) } }
            ToolbarItem(placement: .confirmationAction) { Button("Save", action: save).disabled(!canSave) }
        }
        .onAppear(perform: load)
    }

    private func kindPicker(_ title: String, selection: Binding<String>) -> some View {
        Picker(title, selection: selection) {
            Text("Any kind").tag("")
            Text("Scene").tag(RelationTypeEntry.sceneToken)
            ForEach(kinds, id: \.self) { Text($0.capitalized).tag($0) }
        }
        .disabled(shapeLocked)
    }

    private func load() {
        // Derived from the core, never restated (the standing rule on kind lists).
        kinds = (try? engine.listObjectKinds().kinds.map(\.kind)) ?? []
        if let t = editing {
            forwardLabel = t.forwardLabel
            inverseLabel = t.inverseLabel
            symmetric = t.symmetric
            sourceKind = t.sourceKind ?? ""
            targetKind = t.targetKind ?? ""
        } else {
            sourceKind = initialSourceKind
        }
    }

    private func save() {
        let forward = trimmed(forwardLabel)
        do {
            let saved = try engine.upsertRelationType(
                projectRootPath: projectRootPath, code: editing?.code ?? "", forwardLabel: forward,
                inverseLabel: symmetric ? forward : trimmed(inverseLabel),
                // A symmetric type relates one kind to the same kind: its ends are sorted, not directed (§5.3).
                sourceKind: sourceKind, targetKind: symmetric ? sourceKind : targetKind, symmetric: symmetric)
            onDone(saved.code)
        } catch {
            self.error = (error as? ScriviError)?.message ?? "\(error)"
        }
    }
}

/// [SP-237] Project Settings ▸ Relationship Types: the built-in types listed and locked, the writer's own editable.
struct RelationTypesSection: View {
    let engine: ScriviEngine
    let rootPath: String

    @State private var types: [RelationTypeEntry] = []
    @State private var error: String?
    /// The form's subject: a type, or a new one.
    @State private var form: FormSubject?

    private struct FormSubject: Identifiable {
        let type: RelationTypeEntry?
        var id: String { type?.code ?? "" }
    }

    var body: some View {
        Section {
            ForEach(types) { type in
                HStack {
                    VStack(alignment: .leading, spacing: 2) {
                        Text(type.symmetric ? type.forwardLabel : "\(type.forwardLabel) · \(type.inverseLabel)")
                        Text(ends(type)).font(.caption).foregroundStyle(.secondary)
                    }
                    Spacer()
                    if type.builtIn {
                        Image(systemName: "lock")
                            .foregroundStyle(.secondary)
                            .help("Built in: every project has it, and it can't be changed.")
                    } else {
                        Button("Edit…") { form = FormSubject(type: type) }
                            .buttonStyle(.borderless)
                    }
                }
            }
            Button("New Relationship Type…") { form = FormSubject(type: nil) }
            if let error { Text(error).foregroundStyle(.red).font(.callout) }
        } header: {
            Text("Relationship Types")
        } footer: {
            Text("A relationship reads one label from each end, such as “mentor of” and “mentored by”.")
        }
        .onAppear(perform: load)
        .sheet(item: $form) { subject in
            NavigationStack {
                RelationTypeForm(engine: engine, projectRootPath: rootPath, editing: subject.type) { _ in
                    form = nil
                    load()
                }
            }
            #if os(macOS)
            .frame(minWidth: 400, minHeight: 320)
            #endif
        }
    }

    private func ends(_ type: RelationTypeEntry) -> String {
        func name(_ kind: String?) -> String { kind.map(\.capitalized) ?? "Any kind" }
        return type.symmetric ? "Between \(name(type.sourceKind))"
                              : "\(name(type.sourceKind)) → \(name(type.targetKind))"
    }

    private func load() {
        do {
            types = try engine.listRelationTypes(projectRootPath: rootPath).types
            error = nil
        } catch {
            self.error = (error as? ScriviError)?.message ?? "\(error)"
        }
    }
}
