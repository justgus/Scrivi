import SwiftUI

/// The Object Detail Sheet — a first-class, non-modal surface for seeing and
/// editing what an object actually *is* (EP-034 SP-117; trades **D1-E**, **D2-B**,
/// **D3-A**).
///
/// > *"This is not a popup anymore."* — the writer's brief, R1.
///
/// ## ⚠️ Host-independence is a requirement, not a style (S8)
///
/// D1-E ruled this a **non-modal editor-level pane** rather than a window, and
/// bound the ruling to a condition: it *"must be built as a self-contained,
/// navigable component that does not depend on its host. That is what keeps B (a
/// window) available later without a rewrite."*
///
/// ⚠️ So this view reads **nothing** from its host: no `focusedSession`, no
/// editor environment, no inspector geometry. Everything arrives as a parameter.
/// A `#Preview` with no host at all is the standing proof — if it stops
/// compiling, host-independence has been lost.
///
/// ⚠️ **There is no auxiliary window type in Scrivi** (grep: `.sheet`/`.popover`
/// only), and EP-018's registry, dead-window cache, `.onOpenURL` unreliability
/// and terminate-on-last-window override are the receipts for what adding one
/// costs. D1-E exists to avoid paying that inside an Epic already carrying nine
/// trades.
struct ObjectDetailSheet: View {

    let engine: ScriviEngine
    let projectRootPath: String
    let authorshipRef: AuthorshipRef
    /// Worlds as the app already knows them — passed in so this view never
    /// resolves world state itself (and so a caller can supply fixtures).
    let worlds: [WorldEntry]
    /// Writer-facing scene names, keyed by sceneID (I-0151). Supplied by the host,
    /// which already resolves them for the Navigator — ⚠️ **the pane must never
    /// show a raw `scene_…` ID**, and must not reach into the loader to avoid it
    /// (S8 host-independence). Missing keys fall back inside the section.
    var sceneNames: [String: String] = [:]
    /// I-0151 — take the writer to a related SCENE in the manuscript. A scene has
    /// no Detail Sheet, so the sheet hands this back to its host.
    var onSelectScene: (String) -> Void = { _ in }
    /// I-0155 — announces a successful save so the host can refresh other surfaces
    /// showing the same object (the Scene Inspector's cards).
    ///
    /// ⚠️ A callback, not a direct refresh: this pane must not know the inspector
    /// exists (S8 host-independence).
    var onDidSave: () -> Void = {}
    /// I-0160 — bumped when the object is edited OUTSIDE this sheet (the Scene
    /// Inspector's inline rename), so the sheet re-reads.
    ///
    /// ⚠️ **The mirror of `onDidSave`.** I-0155 made a sheet save refresh the
    /// inspector and stopped there — the reverse direction was left broken, so an
    /// inspector rename left the sheet showing the old name. ⚠️ **The same two
    /// surfaces, the same stale-view bug, fixed in only one direction**, which is
    /// the `feedback_verify_each_half_separately` trap: I tested the half I built.
    var objectRevision: Int = 0
    let onClose: () -> Void

    /// Navigation state. Owned by the caller so it survives the pane being
    /// rebuilt, and so a window host could own it identically later.
    @Bindable var history: ObjectDetailHistory

    @State private var detail: ObjectDetail?
    @State private var loadError: String?
    @State private var saveError: String?

    // Edit buffers. Separate from `detail` so a failed save never discards what
    // the writer typed — the same rule the inline editor follows (I-0126).
    @State private var draftName = ""
    @State private var draftSubtitle = ""
    @State private var draftNotes = ""
    @State private var isSaving = false

    var body: some View {
        VStack(alignment: .leading, spacing: 0) {
            toolbar
            Divider()

            if let loadError {
                message(loadError, systemImage: "exclamationmark.triangle")
            } else if let detail {
                content(for: detail)
            } else {
                message("Select an object to see its details.",
                        systemImage: "square.dashed")
            }
        }
        .frame(minWidth: 420, minHeight: 320)
        .onChange(of: history.current) { _, _ in load() }
        // ⚠️ I-0160: another surface changed this object — re-read.
        //
        // Safe against clobbering: `load()` only overwrites a draft field the
        // writer has not edited (`hasChanges` distinguishes them), and the save
        // path re-reads and merges per field anyway (I-0155).
        .onChange(of: objectRevision) { _, _ in load() }
        .onAppear { load() }
    }

    // MARK: — Chrome

    private var toolbar: some View {
        HStack(spacing: 8) {
            // D2-B: back AND forward. `NavigationStack` gives only back, and the
            // writer asked for "standard NavigatorView buttons".
            Button { navigate(history.goBack()) } label: {
                Image(systemName: "chevron.backward")
            }
            .disabled(!history.canGoBack)
            .help(history.backTarget.map { "Back to \($0.displayName)" } ?? "Back")

            Button { navigate(history.goForward()) } label: {
                Image(systemName: "chevron.forward")
            }
            .disabled(!history.canGoForward)
            .help(history.forwardTarget.map { "Forward to \($0.displayName)" } ?? "Forward")

            Spacer()

            if let detail, !isReadOnly(detail) {
                Button("Save") { save() }
                    .disabled(isSaving || !hasChanges)
                    .keyboardShortcut("s", modifiers: .command)
            }

            Button { onClose() } label: { Image(systemName: "xmark") }
                .help("Close the detail view")
        }
        .buttonStyle(.borderless)
        .padding(8)
    }

    // MARK: — Content

    @ViewBuilder
    private func content(for detail: ObjectDetail) -> some View {
        ScrollView {
            VStack(alignment: .leading, spacing: 14) {

                // ⚠️ R9 / I-0138: a read-only object must say WHY, in words. A
                // greyed-out field that explains nothing is the defect, not the
                // fix — "disabled AND explained" is the rule.
                if let banner = readOnlyBanner(for: detail) {
                    Label(banner, systemImage: "lock")
                        .font(.callout)
                        .foregroundStyle(.secondary)
                        .padding(8)
                        .frame(maxWidth: .infinity, alignment: .leading)
                        .background(.quaternary, in: RoundedRectangle(cornerRadius: 6))
                }

                field("Name") {
                    TextField("Name", text: $draftName)
                        .textFieldStyle(.roundedBorder)
                        .disabled(isReadOnly(detail))
                }

                field("Subtitle") {
                    TextField("A one-line descriptor", text: $draftSubtitle)
                        .textFieldStyle(.roundedBorder)
                        .disabled(isReadOnly(detail))
                }

                // D3-A: a plain TextEditor. ⚠️ D3-C (reusing the manuscript
                // editor) was RULED OUT, not deferred — it would drag EP-019's
                // sentence-granular history engine into object editing.
                field("Notes") {
                    if isReadOnly(detail) {
                        // ⚠️ `.disabled()` DOES NOT MAKE A TextEditor READ-ONLY —
                        // found in the user's live click-through of SP-117. The
                        // two TextFields above honoured it and the TextEditor did
                        // not, so Notes stayed editable directly beneath a banner
                        // saying "read only". This is the app's FIRST disabled
                        // TextEditor; the one other TextEditor in Scrivi
                        // (`WritingToolCards.swift:209`) is never disabled, so
                        // there was no precedent to inherit the problem from.
                        //
                        // ⚠️ Nothing could ever have reached disk — Save is hidden
                        // when read-only — so this was never a write-safety bug.
                        // The real cost was LOST TYPING: `load()` overwrites the
                        // draft from disk, so a paragraph typed during an outage
                        // vanished silently on navigating away and back.
                        //
                        // Rendered as text rather than an inert editor so the
                        // state is visible, not just enforced.
                        ScrollView {
                            Text(draftNotes.isEmpty ? "No notes." : draftNotes)
                                .font(.body)
                                .foregroundStyle(draftNotes.isEmpty ? .tertiary : .secondary)
                                .frame(maxWidth: .infinity, alignment: .leading)
                                .textSelection(.enabled)   // readable ≠ editable
                                .padding(4)
                        }
                        .frame(minHeight: 160)
                        .overlay(RoundedRectangle(cornerRadius: 6)
                            .stroke(.quaternary))
                    } else {
                        TextEditor(text: $draftNotes)
                            .font(.body)
                            .frame(minHeight: 160)
                            .overlay(RoundedRectangle(cornerRadius: 6)
                                .stroke(.quaternary))
                    }
                }

                // ⚠️ Read-only in SP-117 — tags editing is SP-119 (user ruling),
                // so R2/AC2 do not close here. Shown rather than hidden: the data
                // exists and concealing it would be a second capability gap.
                if !detail.tags.isEmpty {
                    field("Tags") {
                        Text(detail.tags.joined(separator: ", "))
                            .font(.callout)
                            .foregroundStyle(.secondary)
                    }
                }

                if let saveError {
                    Label(saveError, systemImage: "exclamationmark.triangle")
                        .font(.callout)
                        .foregroundStyle(.orange)
                }

                Divider()

                // R3/R4/R5/R7 (SP-118): what this object is connected to, and
                // where a writer connects it to something else. Self-contained so
                // the sheet stays host-independent (S8) — it is handed the engine
                // and the worlds the sheet already has, and hands navigation back.
                ObjectRelationsSection(
                    engine: engine,
                    projectRootPath: projectRootPath,
                    objectID: detail.objectID,
                    worlds: worlds,
                    sceneNames: sceneNames,
                    isReadOnly: isReadOnly(detail),
                    onSelectScene: onSelectScene,
                    onNavigate: { entry in
                        // ⚠️ `visit` truncates forward history and no-ops on a
                        // re-visit of the object already showing; `load()` then
                        // runs from `.onChange(of: history.current)`.
                        history.visit(entry)
                    }
                )

                Divider()
                metadata(for: detail)
            }
            .padding(12)
        }
    }

    @ViewBuilder
    private func field<Content: View>(_ label: String,
                                      @ViewBuilder content: () -> Content) -> some View {
        VStack(alignment: .leading, spacing: 4) {
            Text(label)
                .font(.caption.weight(.semibold))
                .foregroundStyle(.secondary)
            content()
        }
    }

    @ViewBuilder
    private func metadata(for detail: ObjectDetail) -> some View {
        VStack(alignment: .leading, spacing: 2) {
            Text(detail.kind.capitalized)
            if !detail.worldID.isEmpty {
                // ⚠️ I-0142: an object's world is shown as a LABEL, never a
                // picker. A picker implied the world could be changed, and moving
                // objects between worlds is an unruled operation.
                Text("World: \(worldName(for: detail.worldID))")
            }
            if !detail.modifiedAt.isEmpty {
                Text("Modified \(detail.modifiedAt)")
            }
        }
        .font(.caption)
        .foregroundStyle(.secondary)
    }

    private func message(_ text: String, systemImage: String) -> some View {
        VStack(spacing: 8) {
            Image(systemName: systemImage).font(.title2)
            Text(text).multilineTextAlignment(.center)
        }
        .foregroundStyle(.secondary)
        .frame(maxWidth: .infinity, maxHeight: .infinity)
        .padding()
    }

    // MARK: — State

    private var hasChanges: Bool {
        guard let detail else { return false }
        return draftName != detail.displayName
            || draftSubtitle != detail.subtitle
            || draftNotes != detail.notes
    }

    /// ⚠️ R9: an object whose world is away is read-only. Doc 3 — *absence is
    /// never deletion* — so the sheet shows it, names the problem, and refuses to
    /// write. It is never hidden and never silently editable.
    private func isReadOnly(_ detail: ObjectDetail) -> Bool {
        guard !detail.worldID.isEmpty else { return false }
        guard let world = worlds.first(where: { $0.worldID == detail.worldID }) else {
            return true   // bound to a world this project cannot see
        }
        return world.worldStatus != .available
    }

    private func readOnlyBanner(for detail: ObjectDetail) -> String? {
        guard isReadOnly(detail) else { return nil }
        guard let world = worlds.first(where: { $0.worldID == detail.worldID }) else {
            return "This object belongs to a world this project cannot find, "
                 + "so it cannot be edited here."
        }
        // T-0440: when the core told us WHY, say why.
        if let explanation = world.unavailabilityExplanation {
            return explanation
        }
        return "“\(world.displayName)” is \(world.worldStatus.rawValue), "
             + "so this object is read-only until it is available again."
    }

    private func worldName(for worldID: String) -> String {
        worlds.first { $0.worldID == worldID }?.displayName ?? worldID
    }

    // MARK: — Load / save

    private func navigate(_ entry: ObjectDetailHistory.Entry?) {
        guard entry != nil else { return }
        load()
    }

    private func load() {
        guard let entry = history.current else {
            detail = nil
            return
        }
        loadError = nil
        saveError = nil
        do {
            let opened = try engine.openObject(
                projectRootPath: projectRootPath,
                objectKind: entry.kind,
                objectID: entry.objectID,
                worldID: entry.worldID
            )
            let parsed = try ObjectDetail(json: opened.objectJson, kind: entry.kind)
            detail = parsed
            draftName = parsed.displayName
            draftSubtitle = parsed.subtitle
            draftNotes = parsed.notes
        } catch {
            detail = nil
            loadError = error.localizedDescription
        }
    }

    private func save() {
        guard let detail, !isReadOnly(detail) else { return }
        isSaving = true
        defer { isSaving = false }
        do {
            // ⚠️ I-0155: PATCH THE DOCUMENT AS IT IS ON DISK RIGHT NOW, not the
            // snapshot taken when this sheet opened.
            //
            // `detail.sourceJson` was read at open. If anything else edited the
            // object since — the Scene Inspector's rename is the reachable case,
            // because BOTH surfaces can be open on the same object at once —
            // patching the stale copy writes the OLD value back over the newer one.
            // Saving a note silently reverted a rename, and the sheet then showed
            // the reverted name as though nothing had happened.
            //
            // ⚠️ Re-reading is what `ObjectCardModel.rename` already does
            // (`ObjectCard.swift:280`) — it opens the object immediately before
            // patching. This surface simply did not follow the same rule.
            //
            // ⚠️ Still a PATCH, never a reconstruction (ObjectDetail.applyingEdits):
            // a rebuilt document would drop `image`, `attributes` and every field a
            // later core adds.
            let onDisk = try engine.openObject(
                projectRootPath: projectRootPath,
                objectKind: detail.kind,
                objectID: detail.objectID,
                worldID: detail.worldID
            )
            let current = try ObjectDetail(json: onDisk.objectJson, kind: detail.kind)

            // ⚠️ Re-reading fixes the OVERWRITE, but it must not silently swap the
            // conflict's direction. If a field changed on disk since this sheet
            // opened AND the writer did not touch it here, her draft still holds
            // the old value — patching it back is the same loss with the roles
            // reversed. Take what she did not edit from disk instead.
            let nameToWrite = (draftName == detail.displayName)
                ? current.displayName : draftName
            let subtitleToWrite = (draftSubtitle == detail.subtitle)
                ? current.subtitle : draftSubtitle
            let notesToWrite = (draftNotes == detail.notes)
                ? current.notes : draftNotes

            let patched = try current.applyingEdits(
                displayName: nameToWrite,
                subtitle: subtitleToWrite,
                notes: notesToWrite
            )
            _ = try engine.saveObject(
                projectRootPath: projectRootPath,
                objectKind: detail.kind,
                objectJson: patched,
                authorshipRef: authorshipRef
            )
            saveError = nil
            load()        // re-read so the view shows what is actually on disk
            onDidSave()   // I-0155: and tell the host, so the inspector agrees
        } catch {
            // Keep the drafts — her typing is not thrown away on a failed save.
            saveError = error.localizedDescription
        }
    }
}
