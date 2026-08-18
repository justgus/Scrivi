import SwiftUI
import UniformTypeIdentifiers

// T-0408 — the Worlds manager (EP-031 SP-099).
// Design: Scrivi_World_Data_Separation_v0_1.md §4.1, §4.4 + Card Framework §7.3.
//
// ⚠️ Added mid-sprint (R4) after live verification: `scrivi_list_worlds` had no UI
// call site anywhere in the app, so a writer could not see which worlds a project
// used, could not create one, and had no way to reach the four world-scoped kinds.
// Doc 2 §7.3 scoped the Worlds menu as the *warning surface* for unavailable
// worlds; nobody scoped how a writer acquires a world in the first place.
//
// What is NOT here, deliberately (SP-102): the pending/warning surface, and
// "Remove All World References" — destructive to pending links, and it must be
// sought out rather than sitting next to routine actions.

struct WorldsView: View {
    let engine: ScriviEngine
    let projectRootPath: String
    /// Re-acquires world access across open projects and refreshes every surface that
    /// reports world state — the warning strip and the inspector cards (I-0123, I-0130).
    /// Injected rather than reached for, because this view is handed its engine
    /// explicitly.
    ///
    /// ⚠️ **Every mutating action here must call this, not just `load()`.** `load()`
    /// refreshes *this sheet's own list* and nothing else, so a relink repaired the
    /// world while the project-wide warning kept insisting it was missing until the
    /// writer changed scenes (I-0130).
    var onWorldsChanged: () -> Void = {}

    @Environment(\.dismiss) private var dismiss

    @State private var worlds: [WorldEntry] = []
    @State private var loadError: String?
    @State private var actionError: String?
    @State private var newWorldName: String = ""
    @State private var isCreating = false
    /// Non-nil while the remove confirmation is up. Holds the world itself rather
    /// than a Bool so the prompt can name it.
    @State private var pendingRemoval: WorldEntry?

    var body: some View {
        VStack(alignment: .leading, spacing: 12) {
            Text("Worlds")
                .font(.title3.weight(.semibold))

            Text("A world holds your characters, locations, items, and everything else "
                 + "you invent, and can be shared between projects. Only sources — your "
                 + "research and references — belong to the project itself.")
                .font(.caption)
                .foregroundStyle(.secondary)
                .fixedSize(horizontal: false, vertical: true)

            if let loadError {
                CardErrorView(message: loadError)
            }
            if let actionError {
                CardErrorView(message: actionError)
            }

            Divider()

            if worlds.isEmpty {
                Text("This project uses no worlds yet.")
                    .font(.callout)
                    .foregroundStyle(.secondary)
                    .frame(maxWidth: .infinity, alignment: .leading)
            } else {
                ScrollView {
                    LazyVStack(alignment: .leading, spacing: 6) {
                        ForEach(worlds) { world in
                            worldRow(world)
                        }
                    }
                }
                .frame(maxHeight: 220)
            }

            Divider()

            // Create — the path that did not exist before T-0408.
            HStack(spacing: 8) {
                TextField("New world name", text: $newWorldName)
                    .textFieldStyle(.roundedBorder)
                    .onSubmit { createWorld() }
                Button("Create…", action: createWorld)
                    .disabled(newWorldName.trimmingCharacters(in: .whitespacesAndNewlines).isEmpty
                              || isCreating)
            }

            // Bind an existing package — a world is shareable, so a project must be
            // able to attach one it did not create.
            HStack {
                Button("Add Existing World…", action: bindExistingWorld)

                // ⚠️ I-0123, and the writer's own proposed remedy: *"An option on the
                // world menu to 'refresh' this links would be a minimal intervention
                // here and allow the app to relink easily."*
                //
                // Re-takes the sandbox grant for every bound world and re-reads their
                // status, so a drive plugged in after launch is picked up WITHOUT a
                // relaunch. Foregrounding does this automatically now; this is the
                // explicit control for when it is needed on demand.
                Button("Reconnect Worlds") {
                    onWorldsChanged()
                    load()
                }
                .help("Re-check worlds that were unavailable — use after reconnecting a drive.")

                Spacer()
                Button("Done") { dismiss() }
                    .keyboardShortcut(.defaultAction)
            }
        }
        .padding(16)
        .frame(width: 460)
        .task { load() }
        // §4.4: naming the consequence concretely is the whole job here. A generic
        // "Are you sure?" leaves the writer to guess whether the package dies too.
        .confirmationDialog("Remove this world from the project?",
                            isPresented: Binding(get: { pendingRemoval != nil },
                                                 set: { if !$0 { pendingRemoval = nil } }),
                            presenting: pendingRemoval) { world in
            Button("Remove Reference", role: .destructive) { removeWorld(world) }
            Button("Cancel", role: .cancel) { pendingRemoval = nil }
        } message: { world in
            Text(removalWarning(for: world))
        }
    }

    /// Built as a plain String rather than inline in the `Text`: concatenated
    /// interpolation inside a `confirmationDialog` closure is what pushed this
    /// view past the type-checker's time budget.
    private func removalWarning(for world: WorldEntry) -> String {
        let name = world.displayName
        var text = "“\(name)” will no longer be part of this project, and anything "
        text += "in this project that refers to it will stop resolving. "
        text += "The world package itself is not deleted — it stays on disk and can "
        text += "be added back."
        return text
    }

    @ViewBuilder
    private func worldRow(_ world: WorldEntry) -> some View {
        let isAvailable: Bool = world.worldStatus == .available
        let symbol: String = isAvailable ? "globe" : "exclamationmark.triangle.fill"
        let tint: Color = isAvailable ? Color.secondary : Color.orange

        HStack(spacing: 8) {
            Image(systemName: symbol)
                .foregroundStyle(tint)

            VStack(alignment: .leading, spacing: 1) {
                Text(world.displayName)
                    .font(.callout)
                Text(world.packagePath)
                    .font(.caption2)
                    .foregroundStyle(.secondary)
                    .lineLimit(1)
                    .truncationMode(.middle)
            }

            Spacer(minLength: 4)

            if world.worldStatus != .available {
                // Says what is actually wrong, and never guesses: a wrong "missing"
                // would invite restoring from backup when the volume was merely
                // disconnected (Doc 2 §7.2.1).
                Text(world.worldStatus.writerDescription)
                    .font(.caption2)
                    .foregroundStyle(.orange)

                // Only offered when there is something to fix. Relinking a world
                // that resolves fine is not a repair, it is a way to bind the
                // wrong package.
                Button("Locate…") { relinkWorld(world) }
                    .font(.caption2)
                    .buttonStyle(.borderless)
            }

            // ⚠️ Removes THIS PROJECT'S REFERENCE ONLY — the package on disk is
            // never touched (scrivi.h:264). The confirmation says so explicitly,
            // because "Remove" next to a world a writer just built reads as
            // "delete my world" unless the copy rules it out.
            Button(role: .destructive) {
                pendingRemoval = world
            } label: {
                Image(systemName: "minus.circle")
            }
            .buttonStyle(.borderless)
            .help("Remove this world from the project")
        }
        .padding(.vertical, 2)
    }

    // MARK: — Actions

    private func load() {
        do {
            worlds = try engine.listWorlds(projectRootPath: projectRootPath).worlds
            loadError = nil
        } catch {
            worlds = []
            loadError = error.localizedDescription
        }
    }

    private func createWorld() {
        let name = newWorldName.trimmingCharacters(in: .whitespacesAndNewlines)
        guard !name.isEmpty else { return }

        // The writer chooses where the package lives — a world is a real, movable,
        // shareable package on disk, not hidden project-internal state (Doc 3 §4.1).
        let panel = NSSavePanel()
        panel.title = "Create World"
        panel.nameFieldStringValue = "\(name).scrivworld"
        panel.canCreateDirectories = true
        guard panel.runModal() == .OK, let url = panel.url else { return }

        isCreating = true
        defer { isCreating = false }
        do {
            let created = try engine.createWorld(
                projectRootPath: projectRootPath,
                packagePath: url.path(percentEncoded: false),
                displayName: name,
                epochLabel: ""
            )
            // ⚠️ Persist the sandbox grant the panel just gave us. Without this the
            // world is readable only until the app quits, after which ScriviCore
            // cannot open the package at all (SP-104).
            WorldBookmarkStore.record(worldID: created.worldID, url: url)
            newWorldName = ""
            actionError = nil
            // ⚠️ I-0130: refresh the WHOLE app, not just this sheet. `load()` only
            // re-reads the list in this panel; the warning strip and the inspector
            // cards live elsewhere and would keep reporting the old state.
            onWorldsChanged()
            load()
        } catch {
            actionError = error.localizedDescription
        }
    }

    /// Drops this project's reference. The package on disk is untouched, which is
    /// what makes this recoverable via "Add Existing World…".
    private func removeWorld(_ world: WorldEntry) {
        pendingRemoval = nil
        do {
            _ = try engine.removeWorldReference(
                projectRootPath: projectRootPath,
                worldID: world.worldID
            )
            // Drop the sandbox grant with the reference — keeping it would leave
            // this app holding access to a package nothing here binds any more.
            WorldBookmarkStore.forget(worldID: world.worldID)
            actionError = nil
            // ⚠️ I-0130: refresh the WHOLE app, not just this sheet. `load()` only
            // re-reads the list in this panel; the warning strip and the inspector
            // cards live elsewhere and would keep reporting the old state.
            onWorldsChanged()
            load()
        } catch {
            actionError = error.localizedDescription
        }
    }

    /// Re-points a world that moved. ScriviCore verifies the `worldID` before
    /// accepting the package, so picking the wrong one is refused rather than
    /// silently substituted (AC8) — the error surfaces in place.
    private func relinkWorld(_ world: WorldEntry) {
        let panel = NSOpenPanel()
        panel.title = "Locate “\(world.displayName)”"
        panel.canChooseDirectories = true
        panel.canChooseFiles = true
        panel.allowsMultipleSelection = false
        guard panel.runModal() == .OK, let url = panel.url else { return }

        do {
            _ = try engine.relinkWorld(
                projectRootPath: projectRootPath,
                worldID: world.worldID,
                newPackagePath: url.path(percentEncoded: false)
            )
            // The relinked package is a freshly-picked path, so it carries a NEW
            // grant — record it or the repair would last only this session.
            WorldBookmarkStore.record(worldID: world.worldID, url: url)
            actionError = nil
            // ⚠️ I-0130: refresh the WHOLE app, not just this sheet. `load()` only
            // re-reads the list in this panel; the warning strip and the inspector
            // cards live elsewhere and would keep reporting the old state.
            onWorldsChanged()
            load()
        } catch {
            actionError = error.localizedDescription
        }
    }

    private func bindExistingWorld() {
        let panel = NSOpenPanel()
        panel.title = "Add Existing World"
        panel.canChooseDirectories = true
        panel.canChooseFiles = true
        panel.allowsMultipleSelection = false
        guard panel.runModal() == .OK, let url = panel.url else { return }

        do {
            // ScriviCore verifies the package's worldID; a same-named package with a
            // different identity is refused rather than substituted (AC8).
            let bound = try engine.addWorld(
                projectRootPath: projectRootPath,
                packagePath: url.path(percentEncoded: false)
            )
            // Recorded only AFTER ScriviCore accepts the package, so a refused bind
            // never leaves a grant behind for a world this project does not use.
            WorldBookmarkStore.record(worldID: bound.worldID, url: url)
            actionError = nil
            // ⚠️ I-0130: refresh the WHOLE app, not just this sheet. `load()` only
            // re-reads the list in this panel; the warning strip and the inspector
            // cards live elsewhere and would keep reporting the old state.
            onWorldsChanged()
            load()
        } catch {
            actionError = error.localizedDescription
        }
    }
}
