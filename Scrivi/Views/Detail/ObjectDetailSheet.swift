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
    /// Needed for world-package writes: the core records which project holds the
    /// lock, so a shared world can name the holder rather than "unknown".
    var projectID: String = ""
    /// ⚠️ I-0162: bumped when a world's availability changes (mount/eject).
    ///
    /// The sheet re-reads on it, because `imagePath` and the read-only banner are
    /// both **snapshots taken at load time** and a drive can vanish afterwards.
    /// ⚠️ Without this the sheet kept a path to bytes that were no longer
    /// reachable and reported *"This image could not be read from disk"* — a
    /// damaged-file message for what is actually an outage.
    ///
    /// ⚠️ `session.worldRevision` ALREADY EXISTED for this (I-0128/I-0129); the
    /// inspector's cards have watched it all along. This surface simply did not.
    var worldRevision: Int = 0
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
    @State private var draftTags: [String] = []
    @State private var newTag = ""
    /// ⚠️ T-0452: a pending exit that would discard unsaved edits. Carries WHAT
    /// the writer was trying to do, so answering the prompt completes it rather
    /// than merely dismissing.
    @State private var pendingExit: PendingExit?

    /// ⚠️ I-0168: a navigation requested from OUTSIDE the sheet — the Scene
    /// Inspector double-click, which calls the host directly.
    ///
    /// ⚠️ **The guard cannot live only in the sheet**, because the sheet is not
    /// on that path: the host owns `ObjectDetailHistory` and the inspector asks
    /// the host. So the host now hands the request HERE instead of applying it,
    /// and the sheet either performs it or prompts first. Same veto, one owner.
    var externalNavigation: ObjectDetailHistory.Entry?
    /// Cleared by the sheet once the request has been consumed.
    var onExternalNavigationHandled: () -> Void = {}

    /// Where the writer was heading when unsaved edits stopped her.
    private enum PendingExit: Equatable {
        case close
        case navigate(ObjectDetailHistory.Entry)
        /// `target` is only for the prompt's wording; the step itself replays
        /// through history so the cursor stays consistent.
        case step(back: Bool, target: ObjectDetailHistory.Entry)
    }
    @State private var isSaving = false
    /// ⚠️ T-0447: the object's image path, resolved by the CORE at list time
    /// (T-0446) rather than by this view. `ObjectDetail` carries only the
    /// assetID — a path is not in the object file, and deliberately is not.
    @State private var imagePath: String?
    /// ⚠️ Set when the object could not be opened because its WORLD is away
    /// (I-0166). Distinct from `loadError`, which means a real failure.
    @State private var unavailableStatus: WorldStatus?

    var body: some View {
        VStack(alignment: .leading, spacing: 0) {
            toolbar
            Divider()

            if let unavailableStatus {
                // ⚠️ Named and explained, never a code. The writer asked for a
                // specific object; tell her about THAT object.
                unavailableView(unavailableStatus)
            } else if let loadError {
                message(loadError, systemImage: "exclamationmark.triangle")
            } else if let detail {
                content(for: detail)
            } else {
                message("Select an object to see its details.",
                        systemImage: "square.dashed")
            }
        }
        .frame(minWidth: 420, minHeight: 320)
        // ⚠️ T-0452: closing or navigating away used to DISCARD unsaved edits
        // silently — the third route into this Epic's recurring data loss
        // (I-0155, I-0165b), and the only one the writer triggers herself.
        .alert("Save your changes?", isPresented: showingExitPrompt) {
            Button("Save") { resolveExit(saving: true) }
            Button("Discard", role: .destructive) { resolveExit(saving: false) }
            Button("Cancel", role: .cancel) { pendingExit = nil }
        } message: {
            Text("You have unsaved changes to “\(detail?.displayName ?? "this object")”. "
                 + "Closing without saving will discard them.")
        }
        .onChange(of: externalNavigation) { _, entry in
            guard let entry else { return }
            onExternalNavigationHandled()
            requestNavigate(entry)
        }
        .onChange(of: history.current) { _, _ in load() }
        // ⚠️ I-0160: another surface changed this object — re-read.
        //
        // ⚠️ This comment used to ASSERT that `load()` only overwrote unedited
        // draft fields. It did not — it overwrote all three unconditionally, so
        // every reload discarded the writer's typing (I-0165b). ✅ The property is
        // now actually IMPLEMENTED in `load()`, not merely claimed here.
        // The save path also re-reads and merges per field (I-0155).
        .onChange(of: objectRevision) { _, _ in load() }
        .onChange(of: worldRevision) { _, _ in load() }
        .onAppear { load() }
    }

    // MARK: — Chrome

    private var toolbar: some View {
        HStack(spacing: 8) {
            // D2-B: back AND forward. `NavigationStack` gives only back, and the
            // writer asked for "standard NavigatorView buttons".
            // ⚠️ T-0452: these MUTATE history before load() runs, so an unguarded
            // press discards edits with no prompt — the same exposure as the ✕.
            Button { requestStep(back: true) } label: {
                Image(systemName: "chevron.backward")
            }
            .disabled(!history.canGoBack)
            .help(history.backTarget.map { "Back to \($0.displayName)" } ?? "Back")

            Button { requestStep(back: false) } label: {
                Image(systemName: "chevron.forward")
            }
            .disabled(!history.canGoForward)
            .help(history.forwardTarget.map { "Forward to \($0.displayName)" } ?? "Forward")

            Spacer()

            if let detail, !isReadOnly(detail) {
                // ⚠️ T-0452: revert to what is on disk.
                //
                // Deliberately NOT undo — no history, no per-keystroke state. The
                // writer asked for exactly this: *"a Cancel option next to Save
                // would allow me to just revert back to the saved version."*
                // ⚠️ EP-019's sentence-granular history is for the manuscript and
                // stays out of object editing (the D3-C ruling).
                Button("Cancel") { revert() }
                    .disabled(isSaving || !hasChanges)
                    .help("Discard your changes and return to the saved version")

                Button("Save") { save() }
                    .disabled(isSaving || !hasChanges)
                    .keyboardShortcut("s", modifiers: .command)
            }

            Button { requestClose() } label: { Image(systemName: "xmark") }
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

                // ⚠️ T-0450 — AC9's second half: EDITS AT RISK.
                //
                // ✅ The recover-and-save path already worked and was verified by
                // use: the world returns, the fields unlock, Save writes her
                // typing. ⚠️ **What was missing is that she is never TOLD** her
                // unsaved work is held only in memory — and if Scrivi quits before
                // the world returns, it is gone.
                //
                // ⚠️ **Deliberately a SECOND banner, not a longer first one.** The
                // read-only banner states a fact about the object; this states a
                // risk to HER WORK, and folding them together buries the urgent one
                // in the routine one.
                //
                // ⚠️ Shown only when edits actually exist (`hasChanges`) — a
                // warning on an untouched sheet trains her to ignore it.
                if isReadOnly(detail) && hasChanges {
                    Label(
                        "Your unsaved changes are held here only. "
                        + "They will be saved when this object's world is available "
                        + "again — but they will be lost if Scrivi quits first.",
                        systemImage: "exclamationmark.triangle.fill"
                    )
                    .font(.callout)
                    .foregroundStyle(.orange)
                    .padding(8)
                    .frame(maxWidth: .infinity, alignment: .leading)
                    .background(.orange.opacity(0.12),
                                in: RoundedRectangle(cornerRadius: 6))
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

                // ⚠️ T-0449 — EDITABLE at last, closing R2 and AC2.
                //
                // SP-117 deferred this "for want of a chip-editor precedent in the
                // app". ⚠️ **The precedent was `WritingToolCards.swift` all along**
                // — the scene Tags card has had `TagChip` and `FlowLayout` since
                // T-0363. They are reused here rather than rewritten, which is the
                // rule SP-118 paid for four times.
                field("Tags") {
                    VStack(alignment: .leading, spacing: 6) {
                        if draftTags.isEmpty {
                            Text(isReadOnly(detail) ? "No tags." : "No tags yet.")
                                .font(.callout)
                                .foregroundStyle(.secondary)
                        } else {
                            FlowLayout(spacing: 6) {
                                ForEach(draftTags, id: \.self) { tag in
                                    if isReadOnly(detail) {
                                        // ⚠️ I-0148's lesson: read-only must be
                                        // ENFORCED, not merely styled. A chip whose
                                        // ✕ still worked would discard a tag that
                                        // could never be saved.
                                        Text(tag)
                                            .font(.caption)
                                            .padding(.horizontal, 7)
                                            .padding(.vertical, 3)
                                            .background(.quaternary, in: Capsule())
                                    } else {
                                        TagChip(tag: tag) {
                                            draftTags.removeAll { $0 == tag }
                                        }
                                    }
                                }
                            }
                        }

                        if !isReadOnly(detail) {
                            TextField("Add a tag…", text: $newTag)
                                .textFieldStyle(.roundedBorder)
                                .font(.callout)
                                .onSubmit { addTag() }
                        }
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
                // AC3 (T-0447): the image. Placed above relationships — what the
                // object IS comes before what it is connected to.
                ObjectImageSection(
                    engine: engine,
                    projectRootPath: projectRootPath,
                    objectID: detail.objectID,
                    objectKind: detail.kind,
                    worldID: detail.worldID,
                    projectID: projectID,
                    authorshipRef: authorshipRef,
                    isReadOnly: isReadOnly(detail),
                    imagePath: imagePath,
                    imageAssetID: detail.imageAssetID,
                    onChanged: {
                        load()        // re-read the object and its image path
                        onDidSave()   // and let the host refresh the inspector
                    }
                )

                Divider()

                // ⚠️ R6 / AC8 (SP-120): the citation surface, placed between what
                // the object IS (image, fields) and what it is connected to.
                //
                // ⚠️ **Deliberately its own section, not a row in the relations
                // list.** A `cites` edge would otherwise appear there as an
                // ordinary relationship — technically true, and useless: a
                // citation is attribution, and burying it among "sibling-of" and
                // "located-at" is what made source creation unreachable for four
                // sprints. ⚠️ Uncited objects still show the relations list
                // unchanged; nothing here replaces it (Q-b's additive rule).
                ObjectSourcesSection(
                    engine: engine,
                    projectRootPath: projectRootPath,
                    objectID: detail.objectID,
                    objectDisplayName: detail.displayName,
                    authorshipRef: authorshipRef,
                    isReadOnly: isReadOnly(detail),
                    onNavigate: { entry in requestNavigate(entry) },
                    onChanged: {
                        // The graph changed — let the host refresh the Scene
                        // Inspector, whose `sources` card reads this same edge.
                        onDidSave()
                    }
                )

                Divider()

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
                        requestNavigate(entry)
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

    /// The object exists; its world is temporarily out of reach (R9, I-0166).
    ///
    /// ⚠️ **Never says "not found" or shows an error code.** Doc 3: absence is
    /// never deletion — and to a writer, "error -1" on a character she wrote is
    /// indistinguishable from having lost her.
    @ViewBuilder
    private func unavailableView(_ status: WorldStatus) -> some View {
        VStack(spacing: 10) {
            Image(systemName: "externaldrive.badge.xmark")
                .font(.title2)
                .foregroundStyle(.secondary)
                .accessibilityHidden(true)

            // ⚠️ The NAME comes from history, which holds it precisely so a
            // writer is never asked to recognise an ID (the AC-A7 rule again).
            Text(history.current?.displayName ?? "This object")
                .font(.headline)

            Text(worldSentence(status))
                .font(.callout)
                .foregroundStyle(.secondary)
                .multilineTextAlignment(.center)

            Text("Its details will appear when the world is available again.")
                .font(.caption)
                .foregroundStyle(.tertiary)
                .multilineTextAlignment(.center)
        }
        .frame(maxWidth: .infinity, maxHeight: .infinity)
        .padding()
    }

    /// ⚠️ The status is DIAGNOSTIC, not behavioural (Doc 2 §7.2.1): the behaviour
    /// is identical for every value, but the remedy differs — so name the world
    /// when we can, and never guess at a cause.
    private func worldSentence(_ status: WorldStatus) -> String {
        let name = history.current.map { worldName(for: $0.worldID) }
        if let name, !name.isEmpty {
            return "“\(name)” is \(status.writerDescription)."
        }
        return "This object's world is \(status.writerDescription)."
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
            || draftTags != detail.tags
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

    /// ⚠️ Duplicates are refused rather than appended — two identical chips are
    /// unremovable individually, since the ✕ matches by value.
    /// Bound rather than a plain Bool so dismissing the alert any other way
    /// clears the pending exit instead of stranding it.
    private var showingExitPrompt: Binding<Bool> {
        Binding(get: { pendingExit != nil },
                set: { if !$0 { pendingExit = nil } })
    }

    /// Discards the writer's edits and re-adopts what is on disk.
    ///
    /// ⚠️ Re-reads rather than restoring from the in-memory `detail`: another
    /// surface may have written since (I-0155), and "the saved version" means
    /// what is actually saved, not what this sheet last saw.
    private func revert() {
        guard let entry = history.current else { return }
        detail = nil          // force load() to adopt disk wholesale
        history.visit(entry)  // no-op for the same object; load() runs below
        load()
        saveError = nil
    }

    /// ⚠️ Every exit routes through here. A close or a navigation with unsaved
    /// edits ASKS; without them it proceeds unchanged.
    private func requestClose() {
        if hasChanges && !isReadOnlyNow {
            pendingExit = .close
        } else {
            onClose()
        }
    }

    private func requestNavigate(_ entry: ObjectDetailHistory.Entry) {
        if hasChanges && !isReadOnlyNow {
            pendingExit = .navigate(entry)
        } else {
            history.visit(entry)
        }
    }

    /// ⚠️ Read-only objects cannot be saved, so their drafts are already covered
    /// by T-0450's at-risk banner — prompting here would offer a Save that is
    /// disabled two points away.
    private var isReadOnlyNow: Bool {
        guard let detail else { return false }
        return isReadOnly(detail)
    }

    /// Back/forward, guarded. ⚠️ The step is deferred until the prompt is
    /// answered — asking *after* moving would strand the drafts on an object the
    /// writer has already left.
    private func requestStep(back: Bool) {
        guard hasChanges && !isReadOnlyNow else {
            navigate(back ? history.goBack() : history.goForward())
            return
        }
        guard let target = back ? history.backTarget : history.forwardTarget else { return }
        pendingExit = .step(back: back, target: target)
    }

    private func resolveExit(saving: Bool) {
        let exit = pendingExit
        pendingExit = nil
        if saving { save() }
        switch exit {
        case .close:               onClose()
        case .navigate(let entry): history.visit(entry)
        case .step(let back, _):   navigate(back ? history.goBack() : history.goForward())
        case .none:                break
        }
    }

    private func addTag() {
        let tag = newTag.trimmingCharacters(in: .whitespacesAndNewlines)
        guard !tag.isEmpty, !draftTags.contains(tag) else {
            newTag = ""
            return
        }
        draftTags.append(tag)
        newTag = ""
    }

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
        unavailableStatus = nil
        do {
            let opened = try engine.openObject(
                projectRootPath: projectRootPath,
                objectKind: entry.kind,
                objectID: entry.objectID,
                worldID: entry.worldID
            )
            let parsed = try ObjectDetail(json: opened.objectJson, kind: entry.kind)
            // ⚠️ Resolved by the CORE, once, from the listing — not by walking the
            // assets directory here. Empty is normal: no image, or its world is
            // away (T-0446).
            imagePath = (try? engine.listObjects(projectRootPath: projectRootPath)
                .objects.first { $0.objectID == parsed.objectID }?.imagePath) ?? nil
            // ⚠️ I-0165b: adopt disk ONLY for fields the writer has not edited.
            //
            // This used to overwrite all three unconditionally — and a comment on
            // `onChange(of: objectRevision)` claimed the opposite ("load() only
            // overwrites a draft field the writer has not edited"). ⚠️ **The claim
            // was false and nothing enforced it**, so any reload — an Inspector
            // rename (I-0160), a remount (I-0162) — silently discarded her typing.
            // ⚠️ **A comment asserting a safety property is not that property**;
            // the same failure as I-0151.
            //
            // `previous` is what disk said last time we read. A draft still equal
            // to it is untouched, so taking the new value is right. A draft that
            // differs is HER work and is kept — she decides at Save, which already
            // merges per field (I-0155).
            // ⚠️ Only when this is the SAME object. Navigating elsewhere must adopt
            // disk wholesale — the drafts belong to the object we just left, and
            // carrying them across would write one object's text onto another.
            let previous = (detail?.objectID == parsed.objectID) ? detail : nil
            let isSameObject = previous != nil

            if !isSameObject || draftName == (previous?.displayName ?? draftName) {
                draftName = parsed.displayName
            }
            if !isSameObject || draftSubtitle == (previous?.subtitle ?? draftSubtitle) {
                draftSubtitle = parsed.subtitle
            }
            if !isSameObject || draftNotes == (previous?.notes ?? draftNotes) {
                draftNotes = parsed.notes
            }
            if !isSameObject || draftTags == (previous?.tags ?? draftTags) {
                draftTags = parsed.tags
            }
            detail = parsed
        } catch {
            // ⚠️ I-0165: a failed re-read must NOT throw the sheet away.
            //
            // R9 requires that an object whose world is away stays **shown,
            // read-only and explained** — never hidden. But `load()` also runs on
            // `worldRevision` (I-0162), and ejecting the drive makes `openObject`
            // fail — so this branch replaced the whole sheet with a raw
            // "ScriviError -1", ⚠️ **and `detail = nil` DISCARDED THE WRITER'S
            // UNSAVED EDITS**, which is the more serious half.
            //
            // ⚠️ The distinction that matters: **losing the ability to re-read is
            // not the same as having nothing to show.** If we already have the
            // object, keep it — `isReadOnly` will report the outage in words, the
            // banner explains it, and her drafts survive until the world returns.
            if detail != nil {
                imagePath = nil   // the bytes are unreachable; stop pointing at them
                // Deliberately NOT setting loadError: the read-only banner already
                // says why, and two explanations of one outage read as two faults.
            } else {
                // ⚠️ I-0166: NOT "nothing to show" — the earlier comment here said
                // that and it was wrong.
                //
                // `history.current` carries the object's NAME, kind and world, so
                // we know exactly what she asked for and why it will not open.
                // R9 requires it be shown, named and explained; a raw
                // "ScriviApp:Scrivi Error -1" is none of those.
                //
                // ⚠️ This is the COLD-OPEN case: the drive was already away when
                // she clicked Show. The earlier fix only covered a sheet that was
                // already displaying an object, which is why this survived it.
                detail = nil
                if (error as? ScriviError)?.isWorldUnavailable == true {
                    unavailableStatus =
                        (error as? ScriviError)?.unavailableWorldStatus ?? .unavailable
                    loadError = nil
                } else {
                    unavailableStatus = nil
                    loadError = error.localizedDescription
                }
            }
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
            let tagsToWrite = (draftTags == detail.tags)
                ? current.tags : draftTags

            let patched = try current.applyingEdits(
                displayName: nameToWrite,
                subtitle: subtitleToWrite,
                notes: notesToWrite,
                tags: tagsToWrite
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
