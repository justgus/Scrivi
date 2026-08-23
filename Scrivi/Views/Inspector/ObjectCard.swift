import SwiftUI

// T-0386 — worldbuilding object cards (EP-031 SP-099).
// Design: Scrivi_Scene_Inspector_Card_Framework_v0_1.md §3.1, §6, §7.2
//       + Scrivi_Worldbuilding_Object_Model_v0_2.md §5.2–5.5.
//
// ⚠️ ONE card implementation, configured per kind — Doc 2 §3.1: "One card type,
// ten configurations — not ten card types." Adding an eleventh kind must be an
// entry in `ObjectCardKind.all`, never a new type. `ObjectCardBody` is generic
// over nothing; the kind arrives as data.
//
// Swift is UI only (Architecture v0.3). Direction normalization, duplicate
// rejection, and pending-vs-dangling all live in ScriviCore.

// MARK: — Per-kind configuration

/// Everything that distinguishes one object card from another: a title, an icon,
/// the kind filter, and the relation type used when relating to a scene.
///
/// This struct **is** the "ten configurations". If adding a kind requires touching
/// anything but `all`, the parameterization has been broken.
struct ObjectCardKind: Sendable, Hashable {
    /// The `ObjectKind` name as ScriviCore spells it — the value that crosses the
    /// C ABI and appears in `objects/index.json`.
    let kind: String
    /// Stable card typeID persisted in `inspector-layout.json`. Treat as a schema
    /// key: renaming it orphans every layout entry naming it.
    let typeID: String
    let title: String
    let systemImage: String
    /// True for kinds stored inside a `.scrivworld` package, which are therefore
    /// subject to pending presentation when their world is away (§7.2).
    ///
    /// ⚠️ **ASKED OF SCRIVICORE, never written down here** (SP-116 T-0429, D5).
    ///
    /// The history of this one property is the whole argument for the standing
    /// rule. It began as a stored `Bool` that still named the pre-SP-104 scope,
    /// so after T-0409 every worldbuilding card passed an EMPTY `worldID` to
    /// `createObject` and **every attempt to create a character failed** — object
    /// creation in the app was broken outright. The fix replaced it with
    /// `kind != "source"`, which was *correct* but still a hand-written partition
    /// of `ObjectKind` in Swift: I-0140, and the eighth occurrence of the class.
    ///
    /// ⚠️ The cause was **structural, not careless** — no ABI endpoint exposed a
    /// kind's scope, so Swift could not derive what the boundary never told it.
    /// `scrivi_list_object_kinds` now does, and this reads it.
    var isWorldScoped: Bool { ObjectKindScope.isWorldScoped(kind) }

    /// The relation type code used when relating an object of this kind to a scene.
    ///
    /// The seeded vocabulary (Doc 1 §5.1) constrains only `character` and
    /// `location`; every other kind relates through an unconstrained type. The card
    /// resolves this against the project's live vocabulary at add-time rather than
    /// assuming the seed — a project may define its own types.
    let preferredRelationType: String

    /// The ten worldbuilding kinds (Doc 2 §3.1). `source` is deliberately absent:
    /// it surfaces through the aggregate `sources` card in the Writing stack
    /// (§3.1.1), not as a per-kind worldbuilding card.
    static let all: [ObjectCardKind] = [
        .init(kind: "character", typeID: "objects.characters", title: "Characters",
              systemImage: "person.2",
              preferredRelationType: "appears-in"),
        .init(kind: "location", typeID: "objects.locations", title: "Locations",
              systemImage: "mappin.and.ellipse",
              preferredRelationType: "located-at"),
        .init(kind: "building", typeID: "objects.buildings", title: "Buildings",
              systemImage: "building.2",
              preferredRelationType: "appears-in"),
        .init(kind: "vehicle", typeID: "objects.vehicles", title: "Vehicles",
              systemImage: "car",
              preferredRelationType: "appears-in"),
        .init(kind: "item", typeID: "objects.items", title: "Items",
              systemImage: "shippingbox",
              preferredRelationType: "appears-in"),
        .init(kind: "map", typeID: "objects.maps", title: "Maps",
              systemImage: "map",
              preferredRelationType: "appears-in"),
        .init(kind: "artifact", typeID: "objects.artifacts", title: "Artifacts",
              systemImage: "crown",
              preferredRelationType: "appears-in"),
        .init(kind: "chronicle", typeID: "objects.chronicles", title: "Chronicles",
              systemImage: "book.closed",
              preferredRelationType: "appears-in"),
        .init(kind: "faction", typeID: "objects.factions", title: "Factions",
              systemImage: "flag.2.crossed",
              preferredRelationType: "appears-in"),
        .init(kind: "rule", typeID: "objects.rules", title: "Rules",
              systemImage: "scroll",
              preferredRelationType: "appears-in"),
    ]

    static func forTypeID(_ typeID: String) -> ObjectCardKind? {
        all.first { $0.typeID == typeID }
    }
}

// MARK: — The card model

/// Loads one scene's edges and keeps the entries of a single kind.
///
/// The read path is Doc 2 §6 exactly: ask the SCENE for its edges, resolve each
/// far endpoint, filter by kind. Nothing is inferred from scene text — AC16 makes
/// that a requirement, not a preference.
@Observable @MainActor final class ObjectCardModel {

    private(set) var entries: [Entry] = []
    private(set) var loadError: String?

    /// One related object as the card renders it.
    struct Entry: Identifiable, Sendable {
        let edgeID:      String
        let objectID:    String
        let displayName: String
        let label:       String
        let sortIndex:   Double
        /// ⚠️ The far endpoint's world is unavailable. Show it, name it, refuse to
        /// modify it — never hide it (§7.2).
        let pending:     Bool
        let pendingStatus: WorldStatus?
        /// Which world is away. Needed by the card footer, because §7.2 requires the
        /// world to be **named** — "World «Eskandar» is on a disconnected volume",
        /// not an anonymous warning. Present only when `pending`.
        let pendingWorldID: String?
        /// ⚠️ I-0142: the world this object BELONGS to, whether or not that world
        /// is currently reachable. Distinct from `pendingWorldID`, which is about
        /// a world being *away*. Empty for a project-scoped object.
        let worldID: String

        var id: String { edgeID }
    }

    private let engine: ScriviEngine
    private let projectRootPath: String
    private let sceneID: String
    private let cardKind: ObjectCardKind

    init(engine: ScriviEngine, projectRootPath: String, sceneID: String,
         cardKind: ObjectCardKind) {
        self.engine = engine
        self.projectRootPath = projectRootPath
        self.sceneID = sceneID
        self.cardKind = cardKind
        load()
    }

    func load() {
        do {
            let edges = try engine.listEdgesFor(
                projectRootPath: projectRootPath, endpointID: sceneID
            ).edges

            // Resolving the far endpoint's kind needs the object index; one listing
            // per load rather than a lookup per edge.
            let objects = try engine.listObjects(
                projectRootPath: projectRootPath, kind: cardKind.kind
            ).objects
            let ofKind = Set(objects.map(\.objectID))

            entries = edges.compactMap { edge -> Entry? in
                // A pending endpoint cannot be confirmed against the index — its
                // world is away, so it is absent from the listing. Trusting the
                // index alone here would silently HIDE pending objects, which is
                // precisely what §7.2 forbids.
                //
                // ⚠️ **I-0124: the pending branch must still check the KIND.** It
                // previously read `edge.otherPending && cardKind.isWorldScoped` —
                // but `isWorldScoped` is a property of THIS CARD, not of the object
                // on the far end, so with a world away every pending object was
                // admitted to all ten cards at once: locations and chronicles listed
                // under Characters. The data was never wrong; only this filter was —
                // which to a writer looks exactly like her world has been scrambled.
                //
                // `otherKind` now travels on the edge (it is the binding's cached
                // kind for a pending endpoint), so the far end can be attributed
                // without the index.
                let known = ofKind.contains(edge.otherID)
                let pendingOfThisKind = edge.otherPending
                    && edge.otherKind == cardKind.kind
                guard known || pendingOfThisKind else {
                    return nil
                }
                return Entry(
                    edgeID: edge.edgeID,
                    objectID: edge.otherID,
                    displayName: edge.otherDisplayName,
                    label: edge.label,
                    sortIndex: edge.sortIndex,
                    pending: edge.otherPending,
                    pendingStatus: edge.pendingStatus,
                    pendingWorldID: edge.otherPending ? edge.otherWorldID : nil,
                    worldID: edge.otherWorldID ?? ""
                )
            }
            loadError = nil
        } catch {
            // Report in place; the stack keeps rendering (§7.1).
            entries = []
            loadError = error.localizedDescription
        }
    }

    /// Orders entries by the STACK's sort (C6). `manual` uses the graph's
    /// `sortIndex` (Doc 1 §5.2), which is what makes manual order persistent.
    func sorted(by sort: InspectorSort) -> [Entry] {
        switch sort {
        case .manual:
            return entries.sorted { $0.sortIndex < $1.sortIndex }
        case .name:
            return entries.sorted {
                $0.displayName.localizedCaseInsensitiveCompare($1.displayName) == .orderedAscending
            }
        case .recentlyAdded:
            // Edges are appended, so later in the log = more recently added.
            return entries.reversed()
        }
    }

    /// Relates an existing object to this scene. ScriviCore normalizes direction
    /// and rejects duplicates — the card supplies only the type.
    ///
    /// ⚠️ **One edge, never two.** The inverse is a read-time label projection
    /// (Doc 1 §5.2); writing a reverse edge is the Cumberland trap the canonical
    /// design exists to prevent.
    func relate(objectID: String, relationTypeCode: String) throws {
        _ = try engine.createEdge(
            projectRootPath: projectRootPath,
            fromID: objectID,
            toID: sceneID,
            relationTypeCode: relationTypeCode
        )
        load()
    }

    /// Creates a NEW object and relates it to this scene in one step (T-0388).
    ///
    /// ⚠️ **Nothing is written until this is called.** A card under construction is
    /// app-side only (§4.6): no `objectID` is allocated, no `objects/` file exists,
    /// and no edge exists until the writer commits. Discard is therefore a pure
    /// app-side drop — nothing to clean up, and no orphan created.
    ///
    /// `worldID` is empty for project-scoped kinds; a world-scoped kind requires a
    /// world, which is why the card refuses to create one without it.
    ///
    /// ⚠️ **`toSceneID` is EXPLICIT and must be the scene the draft was started in
    /// (I-0119).** It previously used this model's own `sceneID`, which is captured
    /// at `init` — but the model is rebuilt by `.task(id:)` the instant the scene
    /// changes, so completing a draft at a scene boundary related the object to the
    /// scene the writer had just moved TO. The object was created correctly; only
    /// the edge endpoint was wrong, which reads to a writer as the object moving.
    func createAndRelate(displayName: String, relationTypeCode: String,
                         worldID: String, authorshipRef: AuthorshipRef,
                         toSceneID: String) throws {
        let name = displayName.trimmingCharacters(in: .whitespacesAndNewlines)
        guard !name.isEmpty else { return }
        guard !toSceneID.isEmpty else { return }

        let created = try engine.createObject(
            projectRootPath: projectRootPath,
            objectKind: cardKind.kind,
            displayName: name,
            authorshipRef: authorshipRef,
            worldID: worldID
        )
        _ = try engine.createEdge(
            projectRootPath: projectRootPath,
            fromID: created.objectID,
            toID: toSceneID,
            relationTypeCode: relationTypeCode
        )
        load()
    }

    /// Renames an existing object in place (the edit half of §4.6).
    func rename(objectID: String, to displayName: String,
                worldID: String, authorshipRef: AuthorshipRef) throws {
        let name = displayName.trimmingCharacters(in: .whitespacesAndNewlines)
        guard !name.isEmpty else { return }

        let opened = try engine.openObject(
            projectRootPath: projectRootPath,
            objectKind: cardKind.kind,
            objectID: objectID,
            worldID: worldID
        )
        // Patch displayName in the object's own JSON rather than reconstructing it,
        // so every field this build does not know about survives the round trip.
        guard var json = try JSONSerialization.jsonObject(
            with: Data(opened.objectJson.utf8)) as? [String: Any] else {
            throw ScriviError(code: -1, message: "Could not read the object.")
        }
        json["displayName"] = name
        let patched = try JSONSerialization.data(withJSONObject: json)

        _ = try engine.saveObject(
            projectRootPath: projectRootPath,
            objectKind: cardKind.kind,
            objectJson: String(decoding: patched, as: UTF8.self),
            authorshipRef: authorshipRef
        )
        load()
    }

    /// Removes the object **from this scene** — deletes the edge only (AC22).
    ///
    /// ⚠️ The object survives as an orphan and stays findable. This must never be
    /// spelled "Delete" in the UI: a writer who reads it as destroying the
    /// character will avoid it and be stuck.
    func removeFromScene(edgeID: String) {
        do {
            _ = try engine.deleteEdge(projectRootPath: projectRootPath, edgeID: edgeID)
            load()
        } catch let error as ScriviError where error.isWorldPending {
            // The graph is frozen toward an unavailable world (Doc 3 §4.6). This is
            // protection, not breakage, and must read that way.
            let status = error.pendingWorldStatus ?? .unavailable
            loadError = "This link is held pending — its world is \(status.writerDescription). "
                      + "Reconnect the world to change it."
        } catch {
            loadError = error.localizedDescription
        }
    }
}

// MARK: — The card

/// The single object-card implementation. Ten registered configurations share it;
/// none of them subclasses or re-implements it.
///
/// `InspectorCard` requires static identity, so each kind gets a thin generated
/// conformer via `ObjectCardType` below — the *body* is this one view.
struct ObjectCardBody: View {
    let context: CardContext
    let cardKind: ObjectCardKind

    @State private var model: ObjectCardModel?
    @State private var showPicker = false

    /// The in-place edit state (§4.6). Non-nil means this card has an unfinished
    /// entry — a NEW object under construction, or an existing one being renamed.
    @State private var draft: ObjectDraft?
    @State private var worlds: [WorldEntry] = []
    @State private var showUnfinishedPrompt = false
    @State private var commitError: String?

    /// A card under construction or under edit. App-side only until committed.
    struct ObjectDraft: Equatable {
        /// Nil for a new object; set when editing an existing one.
        var objectID: String?
        var name: String = ""
        var worldID: String = ""
        /// ⚠️ The scene this draft was STARTED in (I-0119). A draft deliberately
        /// outlives a scene change — §4.6 lets a card sit under construction —
        /// so it must carry its own origin. Reading the scene from the view or
        /// the model at commit time yields whichever scene the writer has since
        /// moved to, which is exactly the bug: the object was filed against the
        /// wrong scene and the writer had to repair it by hand.
        var originSceneID: String = ""

        /// ⚠️ T-0421 (I-0139): the name this draft STARTED with, for an existing
        /// object. It exists to answer one question — has the writer actually
        /// changed anything? — because the exit button's meaning depends on it.
        /// Empty for a new object, where `name` starting empty says the same.
        var originalName: String = ""

        var isNew: Bool { objectID == nil }

        /// True when leaving would discard real work. ⚠️ For an existing object
        /// opened and not edited, this is FALSE — and that is the whole point of
        /// I-0139: a single click on a title opened a mode whose only exit was
        /// labelled as data loss, when there was no data to lose.
        var hasUnsavedChanges: Bool {
            name.trimmingCharacters(in: .whitespacesAndNewlines)
                != originalName.trimmingCharacters(in: .whitespacesAndNewlines)
        }
    }

    var body: some View {
        VStack(alignment: .leading, spacing: 8) {
            if let model {
                if let error = model.loadError {
                    CardErrorView(message: error)
                }
                if let commitError {
                    CardErrorView(message: commitError)
                }

                let entries = model.sorted(by: context.sort)
                if entries.isEmpty {
                    // §2: empty is a normal state, not an error. Nothing is ever
                    // inferred into this card from the scene's text.
                    Text("No \(cardKind.title.lowercased()) in this scene yet.")
                        .font(.caption)
                        .foregroundStyle(.secondary)
                } else {
                    ForEach(entries) { entry in
                        ObjectCardRow(entry: entry) {
                            model.removeFromScene(edgeID: entry.edgeID)
                        } onEdit: {
                            // ⚠️ I-0142: seed the ACTUAL world. This was hardcoded
                            // to "" , so the editor's world control always opened
                            // on nothing — and because it was a Picker, choosing
                            // the object's own world looked like a pending edit.
                            draft = ObjectDraft(objectID: entry.objectID,
                                                name: entry.displayName,
                                                worldID: entry.worldID,
                                                originSceneID: context.sceneID,
                                                originalName: entry.displayName)
                        } onViewDetail: {
                            // ⚠️ I-0155: close this card's inline editor when the
                            // Detail Sheet takes over the same object.
                            //
                            // Both surfaces edit the SAME fields and were happily
                            // visible at once, each holding its own draft. Renaming
                            // in one did not reach the other, and whichever saved
                            // last silently won — the writer saw two truths and was
                            // given no hint which one disk held.
                            //
                            // ⚠️ Only when it is the SAME object: a draft on a
                            // DIFFERENT row is unrelated work and discarding it
                            // would destroy typing she never put in question.
                            //
                            // ⚠️ An UNSAVED draft is never thrown away silently —
                            // §4.6's complete-or-discard prompt owns that decision,
                            // exactly as it does at a scene boundary. An untouched
                            // draft (nothing typed) just closes.
                            if let d = draft, d.objectID == entry.objectID {
                                if d.name != d.originalName {
                                    showUnfinishedPrompt = true
                                } else {
                                    draft = nil
                                    commitError = nil
                                }
                            }

                            // R7. ⚠️ A pending object still opens — R9 requires it
                            // be shown, explained and read-only, never hidden.
                            context.openObjectDetail?(entry.objectID,
                                                      cardKind.kind,
                                                      entry.worldID,
                                                      entry.displayName)
                        }
                    }
                    // §7.2: name the world and say what is wrong with it. The rows
                    // carry a ⚠ badge; only this says *which world* and *why*.
                    PendingWorldFooter(entries: entries, worlds: worlds)
                }

                // §4.6: the edit state lives IN the card, in the stack. No modal,
                // ever — not for creation and not for editing.
                if let current = draft {
                    ObjectDraftEditor(
                        draft: Binding(get: { current }, set: { draft = $0 }),
                        cardKind: cardKind,
                        worlds: worlds,
                        // ⚠️ I-0119: disabled while the unfinished prompt is up, so
                        // the editor's own Save cannot fire *around* the decision
                        // the alert is asking for. That is how the wrong-scene
                        // commit was reached in the first place.
                        canCommit: context.authorshipRef != nil && !showUnfinishedPrompt,
                        // ⚠️ I-0119: names the draft's origin scene, but ONLY when
                        // it differs from the scene on screen. "Complete It"
                        // reopens the draft while the writer stands in a
                        // different scene, and Create still files it against the
                        // origin — correct, but invisible until now, which is
                        // exactly how the wrong-scene surprise happened.
                        originLabel: originSceneLabel,
                        onCommit: { commitDraft() },
                        // ⚠️ I-0126: clear the failure notice too. A stale error
                        // sitting on the card after Discard contradicts what the
                        // writer can plainly see working, and it is what made
                        // I-0125 so confusing to diagnose — she could not tell
                        // which message described the current state.
                        onDiscard: { draft = nil; commitError = nil }
                    )
                }

                HStack(spacing: 10) {
                    Button {
                        showPicker = true
                    } label: {
                        Label("Add \(cardKind.title)…", systemImage: "plus")
                            .font(.caption)
                    }
                    .buttonStyle(.borderless)
                    .popover(isPresented: $showPicker, arrowEdge: .bottom) {
                        ObjectPickerView(
                            engine: context.engine,
                            projectRootPath: context.projectRootPath,
                            cardKind: cardKind,
                            existingObjectIDs: Set(entries.map(\.objectID)),
                            onCreateNew: { typed in
                                // "Create new…" opens the in-place edit state,
                                // pre-filled with whatever she typed — never a dialog.
                                showPicker = false
                                draft = ObjectDraft(objectID: nil, name: typed,
                                                    worldID: defaultWorldID,
                                                    originSceneID: context.sceneID)
                            }
                        ) { objectID, relationType in
                            do {
                                try model.relate(objectID: objectID,
                                                 relationTypeCode: relationType)
                            } catch {
                                model.load()
                            }
                            showPicker = false
                        }
                    }

                    if draft == nil {
                        Button {
                            draft = ObjectDraft(objectID: nil, name: "",
                                                worldID: defaultWorldID,
                                                originSceneID: context.sceneID)
                        } label: {
                            Label("New \(singularTitle)", systemImage: "square.and.pencil")
                                .font(.caption)
                        }
                        .buttonStyle(.borderless)
                        .disabled(context.authorshipRef == nil)
                        .help(context.authorshipRef == nil
                              ? "Identity is still starting up."
                              : "Create a new \(singularTitle.lowercased()) in place")
                    }
                }
            } else {
                ProgressView().controlSize(.small)
            }
        }
        // Reload when the scene changes (C4=A: eager on scene select) OR when world
        // availability changes (I-0128).
        //
        // ⚠️ `sceneID` alone left the card stale after a reconnect: the warning strip
        // cleared and the graph relinked, but each card kept rendering the pending
        // entries it loaded while the world was away until the writer switched scenes.
        // Same fix as I-0105's `historyRevision` — fold the revision into the key so
        // a change the card cannot otherwise observe still invalidates it.
        .task(id: "\(context.sceneID)#\(context.worldRevision)#\(context.objectRevision)") {
            model = ObjectCardModel(
                engine: context.engine,
                projectRootPath: context.projectRootPath,
                sceneID: context.sceneID,
                cardKind: cardKind
            )
            worlds = (try? context.engine.listWorlds(
                projectRootPath: context.projectRootPath).worlds) ?? []
        }
        // §4.6: leaving a scene with an unfinished card surfaces complete-or-discard.
        // ⚠️ It fires at the scene BOUNDARY — never while she is writing within a
        // scene, which §2 forbids outright.
        .onChange(of: context.sceneID) { _, _ in
            if draft != nil { showUnfinishedPrompt = true }
        }
        .alert("Unfinished \(singularTitle.lowercased())",
               isPresented: $showUnfinishedPrompt, presenting: draft) { pending in
            // ⚠️ I-0119: the alert now OWNS the commit. Previously its only
            // options were "Complete It" (reopen the draft) and discard — there
            // was no Save here at all, so the writer reached for the draft
            // editor's own Save button underneath, committing *around* the
            // prompt. Offering it here makes the decision the alert asks for the
            // one that actually runs, and it commits to the ORIGIN scene.
            Button(pending.isNew ? "Create in Previous Scene" : "Save Changes") {
                commitDraft()
                showUnfinishedPrompt = false
            }
            // ⚠️ I-0119: "Keep Editing" rather than "Complete It". The old wording
            // implied the action itself completed the object; it does not — it
            // reopens the draft, and the writer is still standing in the NEW
            // scene while the draft stays bound to the previous one. The card's
            // header and Create button now name that scene explicitly.
            Button("Keep Editing") {
                showUnfinishedPrompt = false
            }
            // ⚠️ §4.6.1: the destructive meaning DIFFERS between create and edit, and
            // the button must name the consequence concretely. A generic "Discard"
            // makes "this character is never created" and "your rename is undone"
            // indistinguishable, and a writer will assume the worse one.
            Button(pending.isNew ? "Discard New \(singularTitle)" : "Revert Changes",
                   role: .destructive) {
                draft = nil
                commitError = nil      // I-0126: the notice goes with the draft.
                showUnfinishedPrompt = false
            }
        } message: { pending in
            Text(unfinishedMessage(for: pending))
        }
    }

    /// The prompt's body copy (§4.6.1 — name the consequence concretely).
    ///
    /// ⚠️ It must say **which scene** the object will be filed against (I-0119).
    /// The writer has already moved on by the time she reads this, so "it will be
    /// saved" is ambiguous between the scene she left and the one she is looking
    /// at — and getting that wrong silently is the bug this prompt now prevents.
    private func unfinishedMessage(for pending: ObjectDraft) -> String {
        let name = pending.name.trimmingCharacters(in: .whitespacesAndNewlines)
        let label = name.isEmpty ? "This \(singularTitle.lowercased())" : "“\(name)”"
        let kindWord = singularTitle.lowercased()

        if pending.isNew {
            return "\(label) hasn't been created yet. Creating it now files it "
                 + "against the scene you were just in, not the one you're viewing. "
                 + "Discarding makes it disappear — no \(kindWord) and no link are saved."
        }
        return "\(label) has unsaved changes. Reverting undoes your edits; "
             + "the \(kindWord) itself stays exactly as it was."
    }

    /// Singular form for writer-facing copy: "New Character", not "New Characters".
    private var singularTitle: String {
        cardKind.title.hasSuffix("s") ? String(cardKind.title.dropLast()) : cardKind.title
    }

    /// "Scene 3" for the draft's origin, or nil when the draft belongs to the
    /// scene already on screen (the ordinary case, where saying so would be noise).
    ///
    /// Position in `allSceneIDs` is the scene's manuscript number — the stack is
    /// handed the scenes in order. When the origin cannot be located (an empty
    /// list, or a scene deleted meanwhile) this says "another scene" rather than
    /// inventing a number.
    private var originSceneLabel: String? {
        guard let pending = draft,
              !pending.originSceneID.isEmpty,
              pending.originSceneID != context.sceneID else { return nil }
        if let i = context.allSceneIDs.firstIndex(of: pending.originSceneID) {
            return "Scene \(i + 1)"
        }
        return "another scene"
    }

    /// A world-scoped kind needs somewhere to live; a project-scoped one never does.
    private var defaultWorldID: String {
        guard cardKind.isWorldScoped else { return "" }
        return worlds.first { $0.worldStatus == .available }?.worldID ?? ""
    }

    private func commitDraft() {
        guard let model, let pending = draft,
              let ref = context.authorshipRef else { return }
        do {
            if let objectID = pending.objectID {
                try model.rename(objectID: objectID, to: pending.name,
                                 worldID: pending.worldID, authorshipRef: ref)
            } else {
                try model.createAndRelate(
                    displayName: pending.name,
                    relationTypeCode: cardKind.preferredRelationType,
                    worldID: pending.worldID,
                    authorshipRef: ref,
                    // ⚠️ I-0119: the scene the draft was STARTED in, not the one
                    // showing now. These differ precisely when the writer completes
                    // a draft at a scene change — the case AC20 exists to protect.
                    toSceneID: pending.originSceneID.isEmpty
                        ? context.sceneID
                        : pending.originSceneID
                )
            }
            draft = nil
            commitError = nil          // I-0126: a success clears the last failure.
            // I-0160: tell the host, so a Detail Sheet open on this same object
            // re-reads instead of showing the name we just replaced.
            context.onObjectChanged?()
        } catch {
            // Keep the draft open — her typing is not thrown away on a failed save.
            commitError = error.localizedDescription
        }
    }
}

/// The in-place edit state (§4.6). Lives inside the card, in the stack — never a
/// sheet, never a dialog.
///
/// ⚠️ §4.6.2: a card being created or edited must be **immediately distinguishable**
/// from a settled one, **without relying on color alone**. Three redundant signals
/// carry it here: a labelled header ("New" / "Editing"), an icon, and a dashed
/// border. A writer who cannot see color still sees two of the three.
private struct ObjectDraftEditor: View {
    @Binding var draft: ObjectCardBody.ObjectDraft
    let cardKind: ObjectCardKind
    let worlds: [WorldEntry]
    let canCommit: Bool
    /// Non-nil when this draft belongs to a DIFFERENT scene than the one on
    /// screen (I-0119) — shown so "Create" can never file somewhere unannounced.
    var originLabel: String? = nil
    let onCommit: () -> Void
    let onDiscard: () -> Void

    @FocusState private var focused: Bool

    var body: some View {
        VStack(alignment: .leading, spacing: 6) {
            HStack(spacing: 4) {
                Image(systemName: draft.isNew ? "plus.square.dashed" : "pencil")
                    .font(.caption2)
                Text(draft.isNew ? "New" : "Editing")
                    .font(.caption2.weight(.semibold))
                Spacer()
                // ⚠️ I-0119: the destination, stated where the decision is made.
                // Not colour-only (§4.6.2) — it is words, and it carries an icon.
                if let originLabel {
                    Label("for: \(originLabel)", systemImage: "arrow.uturn.backward")
                        .font(.caption2.weight(.semibold))
                        .foregroundStyle(.orange)
                }
            }
            .foregroundStyle(.secondary)

            TextField("Name", text: $draft.name)
                .textFieldStyle(.roundedBorder)
                .font(.callout)
                .focused($focused)
                .onSubmit(onCommit)

            // A world-scoped kind must be told where it lives; a project-scoped one
            // never asks, because the question would be meaningless.
            if cardKind.isWorldScoped {
                // ⚠️ I-0142 / user ruling 2026-08-20. An object ALREADY IN A WORLD
                // shows that world as a LABEL, never a picker.
                //
                // A picker here implied the world could be changed, and "Save"
                // then offered to move the object between worlds. That opens a
                // set of questions nobody has answered: do related objects
                // migrate? are cross-world edges allowed? are they deleted,
                // remapped to an equivalent target, or left dangling? Until those
                // are ruled, the UI must not offer the operation at all.
                //
                // The picker survives ONLY for a genuinely unassigned object —
                // creating a new one, where a world must be chosen.
                if !draft.worldID.isEmpty {
                    HStack(spacing: 4) {
                        Text("World")
                            .foregroundStyle(.secondary)
                        Text(worldName(for: draft.worldID))
                    }
                    .font(.caption)
                    .help("An object cannot be moved between worlds.")
                } else if worlds.isEmpty {
                    Text("No world yet — add one from the Worlds menu first.")
                        .font(.caption2)
                        .foregroundStyle(.secondary)
                } else {
                    Picker("World", selection: $draft.worldID) {
                        ForEach(worlds) { world in
                            Text(world.displayName).tag(world.worldID)
                        }
                    }
                    .pickerStyle(.menu)
                    .font(.caption)
                }
            }

            HStack(spacing: 8) {
                // ⚠️ I-0119: when the draft belongs to another scene, the button
                // says so. "Create" alone is what let a writer file a location
                // into the wrong scene believing she was filing it into this one.
                Button(commitTitle, action: onCommit)
                    .font(.caption)
                    .disabled(!canCommit || trimmedName.isEmpty || needsWorld)
                // ⚠️ T-0421 (I-0139): the exit is named for what it ACTUALLY does,
                // and is only destructive-styled when it actually destroys work.
                //
                // The defect: a single click on a title opens this editor, and its
                // only way out read "Revert" in red — a label that means "throw
                // away my changes" to a writer who has made none. She was told the
                // exit was data loss, so she did not take it. The exit existed;
                // it was mislabelled.
                //
                // ⚠️ Deliberately NOT fixed by making this panel modal (§4.6
                // forbids it), by removing the inline editor (Q-b keeps it — the
                // Detail Sheet is ADDITIVE), or by adding a Cancel that bypasses
                // the unfinished-work prompt — that route is how I-0119 filed an
                // object into the wrong scene.
                //
                // Named concretely per §4.6.1 — the consequences genuinely differ.
                Button(exitTitle,
                       role: draft.hasUnsavedChanges ? .destructive : nil,
                       action: onDiscard)
                    .font(.caption)
                    .help(exitHelp)
                Spacer()
            }
        }
        .padding(8)
        .background(.quaternary.opacity(0.4), in: RoundedRectangle(cornerRadius: 6))
        .overlay(
            RoundedRectangle(cornerRadius: 6)
                .strokeBorder(.tertiary, style: StrokeStyle(lineWidth: 1, dash: [3, 2]))
        )
        .task { focused = true }
    }

    /// ⚠️ T-0421 (I-0139). Three cases, three honest labels:
    ///   • a new object with nothing typed → "Cancel" (nothing is lost)
    ///   • an existing object, unedited     → "Done" (viewing, not editing)
    ///   • anything with real changes       → "Discard"/"Revert" (work IS lost)
    /// The world's name for display. ⚠️ Falls back to the raw ID rather than a
    /// blank or a guess: an unreachable world is still the object's world, and
    /// saying nothing would reintroduce exactly the "which world?" gap I-0142 was.
    private func worldName(for id: String) -> String {
        worlds.first { $0.worldID == id }?.displayName ?? id
    }

    private var exitTitle: String {
        if draft.hasUnsavedChanges { return draft.isNew ? "Discard" : "Revert" }
        return draft.isNew ? "Cancel" : "Done"
    }

    private var exitHelp: String {
        if draft.hasUnsavedChanges {
            return draft.isNew
                ? "Discard this new \(cardKind.title.lowercased()) without creating it."
                : "Revert your changes and stop editing. The object is not deleted."
        }
        return draft.isNew
            ? "Close without creating anything."
            : "Stop editing. Nothing is changed and nothing is deleted."
    }

    private var commitTitle: String {
        let base = draft.isNew ? "Create" : "Save"
        guard let originLabel else { return base }
        return "\(base) in \(originLabel)"
    }

    private var trimmedName: String {
        draft.name.trimmingCharacters(in: .whitespacesAndNewlines)
    }

    /// A world-scoped kind with no world selected cannot be stored anywhere, so
    /// creation is refused rather than failing at the boundary.
    private var needsWorld: Bool {
        cardKind.isWorldScoped && draft.worldID.isEmpty
    }
}

/// The §7.2 pending block: a divider, then one line per unavailable world **naming
/// it** and reporting its status.
///
/// ⚠️ **The rows alone are not enough, which is why this exists.** Before T-0389 a
/// pending entry showed a ⚠ badge and a greyed name, and the only place the world
/// and its status appeared was a `.help()` tooltip — hover-only, undiscoverable, and
/// absent entirely on iPad. A writer could see that *something* was wrong and had no
/// way to learn *which world* or *why*, which is precisely the state that invites the
/// destructive remedy I-0115 warns about.
///
/// **Never offer a cleanup affordance here.** Anything that would drop pending links
/// belongs solely to the Worlds menu (§7.2, §7.3); inline it reads as routine tidying.
private struct PendingWorldFooter: View {
    let entries: [ObjectCardModel.Entry]
    /// Bound worlds, for resolving an ID to a display name.
    let worlds: [WorldEntry]

    /// One line per distinct world, so a card holding entries from two absent worlds
    /// reports both rather than blaming whichever sorted first.
    private var groups: [(worldID: String, name: String, status: WorldStatus, count: Int)] {
        let pending = entries.filter(\.pending)
        guard !pending.isEmpty else { return [] }

        // An empty key covers the case where the core reported pending without a
        // world ID: still reported, just not named. Silence would be worse.
        let byWorld = Dictionary(grouping: pending) { $0.pendingWorldID ?? "" }
        return byWorld.map { worldID, rows in
            let name = worlds.first { $0.worldID == worldID }?.displayName
            let status = rows.compactMap(\.pendingStatus).first ?? .unavailable
            return (worldID: worldID,
                    // ⚠️ Falling back to the ID is deliberate: an unnamed world is
                    // still better than an unattributed warning.
                    name: name ?? (worldID.isEmpty ? "" : worldID),
                    status: status,
                    count: rows.count)
        }
        .sorted { $0.name.localizedCaseInsensitiveCompare($1.name) == .orderedAscending }
    }

    var body: some View {
        if !groups.isEmpty {
            Divider().padding(.vertical, 2)
            VStack(alignment: .leading, spacing: 3) {
                ForEach(groups, id: \.worldID) { group in
                    HStack(alignment: .firstTextBaseline, spacing: 5) {
                        Image(systemName: "exclamationmark.triangle.fill")
                            .font(.caption2)
                            .foregroundStyle(.orange)
                            // The icon is decorative; the sentence carries the meaning,
                            // so it must not be announced twice.
                            .accessibilityHidden(true)
                        Text(sentence(for: group))
                            .font(.caption2)
                            .foregroundStyle(.secondary)
                            .fixedSize(horizontal: false, vertical: true)
                    }
                }
            }
            .accessibilityElement(children: .combine)
        }
    }

    /// ⚠️ Never says "missing" unless the core positively established it (I-0115), and
    /// never implies the links are gone — they are **held**, which is the guarantee
    /// AC7 makes and the one thing the writer most needs to know.
    private func sentence(
        for group: (worldID: String, name: String, status: WorldStatus, count: Int)
    ) -> String {
        let links = group.count == 1 ? "This link is" : "These \(group.count) links are"
        let world = group.name.isEmpty
            ? "That world"
            : "World “\(group.name)”"
        return "\(world) is \(group.status.writerDescription). \(links) held pending."
    }
}

private struct ObjectCardRow: View {
    let entry: ObjectCardModel.Entry
    let onRemove: () -> Void
    let onEdit: () -> Void
    /// Opens the Detail Sheet (EP-034 SP-117, T-0438 / R7).
    ///
    /// ⚠️ **ADDITIVE — this replaces nothing** (design Q-b). A single click still
    /// opens the inline editor exactly as it did; the sheet is a third path
    /// alongside the list item (identify) and the inline editor (capture).
    var onViewDetail: (() -> Void)? = nil

    var body: some View {
        HStack(spacing: 6) {
            if entry.pending {
                // ⚠ badge — the card is showing something it cannot currently
                // reach (§7.2). Named, never a bare ID.
                Image(systemName: "exclamationmark.triangle.fill")
                    .font(.caption2)
                    .foregroundStyle(.orange)
                    .help(pendingHelp)
            }

            VStack(alignment: .leading, spacing: 1) {
                Text(entry.displayName)
                    .font(.callout)
                    .foregroundStyle(entry.pending ? .secondary : .primary)
                if !entry.label.isEmpty {
                    Text(entry.label)
                        .font(.caption2)
                        .foregroundStyle(.secondary)
                }
            }
            // Editing happens in place too (§4.6) — a pending object cannot be
            // edited, because the graph is frozen toward its world.
            .contentShape(Rectangle())
            // ⚠️ ORDER MATTERS. SwiftUI dispatches the LONGEST matching gesture,
            // so the double-click modifier must come FIRST or a double click is
            // delivered as two single clicks and opens the inline editor twice.
            //
            // ⚠️ This is the I-0132 class of bug — that Issue took FOUR attempts
            // and three misdiagnoses, and the lesson recorded was to fix it in
            // SwiftUI's own terms rather than reach beneath it with an NSEvent
            // monitor. Do not "fix" a click problem here with an event monitor.
            .onTapGesture(count: 2) { onViewDetail?() }
            .onTapGesture { if !entry.pending { onEdit() } }
            .contextMenu {
                // R7's discoverable half. Double-click is invisible until someone
                // tries it; the menu is how a writer FINDS the sheet.
                if let onViewDetail {
                    Button("View Detail", systemImage: "square.text.square") {
                        onViewDetail()
                    }
                }
                Button("Remove from Scene", systemImage: "minus.circle",
                       role: .destructive, action: onRemove)
                    .disabled(entry.pending)
            }

            Spacer(minLength: 4)

            Button(action: onRemove) {
                Image(systemName: "minus.circle")
                    .font(.caption)
            }
            .buttonStyle(.borderless)
            .foregroundStyle(.secondary)
            // ⚠️ AC22: the edge goes, the object stays. The wording is load-bearing
            // — "Delete" here would misdescribe what happens and scare a writer off
            // an action that is not destructive to her object.
            //
            // ⚠️ T-0423 (I-0138): when the button is DISABLED, the tooltip must say
            // WHY. It previously read "Remove from scene" in both states, so
            // hovering a greyed-out button told the writer what it would do and
            // never why she could not use it — disabled and UNexplained, while the
            // comment below claimed the opposite. §7.2 requires disabled-AND-
            // explained. `pendingHelp` already composed the right sentence and was
            // applied only to the ⚠ badge.
            .help(entry.pending ? pendingHelp : "Remove from scene")
            .accessibilityLabel(
                entry.pending
                    ? "Remove \(entry.displayName) from scene — unavailable. \(pendingHelp)"
                    : "Remove \(entry.displayName) from scene")
            // The graph is frozen toward an unavailable world: the affordance is
            // disabled and explained, never simply absent (§7.2).
            .disabled(entry.pending)
        }
        .padding(.vertical, 1)
    }

    private var pendingHelp: String {
        let status = entry.pendingStatus ?? .unavailable
        return "Held pending — this object's world is \(status.writerDescription)."
    }
}

// MARK: — Registration

/// Generates the ten `InspectorCard` conformers over the one body above.
///
/// Swift's protocol needs *static* identity per card, which a single struct cannot
/// vary at runtime — so each kind gets a phantom-typed shell whose entire
/// implementation is `ObjectCardBody`. The shells carry no behavior; adding a kind
/// is still one line in `ObjectCardKind.all` plus one line here.
protocol ObjectCardKindProviding {
    static var cardKind: ObjectCardKind { get }
}

struct ObjectCardType<Provider: ObjectCardKindProviding>: InspectorCard {
    static var typeID: String { Provider.cardKind.typeID }
    static var title: String { Provider.cardKind.title }
    static var systemImage: String { Provider.cardKind.systemImage }
    static var stack: InspectorStack { .worldbuilding }

    init() {}

    func body(context: CardContext) -> AnyView {
        AnyView(ObjectCardBody(context: context, cardKind: Provider.cardKind))
    }
}

// One provider per kind. These exist only to give the protocol a static identity.
enum CharactersProvider: ObjectCardKindProviding {
    static let cardKind = ObjectCardKind.all[0]
}
enum LocationsProvider: ObjectCardKindProviding {
    static let cardKind = ObjectCardKind.all[1]
}
enum BuildingsProvider: ObjectCardKindProviding {
    static let cardKind = ObjectCardKind.all[2]
}
enum VehiclesProvider: ObjectCardKindProviding {
    static let cardKind = ObjectCardKind.all[3]
}
enum ItemsProvider: ObjectCardKindProviding {
    static let cardKind = ObjectCardKind.all[4]
}
enum MapsProvider: ObjectCardKindProviding {
    static let cardKind = ObjectCardKind.all[5]
}
enum ArtifactsProvider: ObjectCardKindProviding {
    static let cardKind = ObjectCardKind.all[6]
}
enum ChroniclesProvider: ObjectCardKindProviding {
    static let cardKind = ObjectCardKind.all[7]
}
enum FactionsProvider: ObjectCardKindProviding {
    static let cardKind = ObjectCardKind.all[8]
}
enum RulesProvider: ObjectCardKindProviding {
    static let cardKind = ObjectCardKind.all[9]
}

extension InspectorCardRegistry {
    /// Registers all ten object cards. Called from `registerBuiltIns()`.
    ///
    /// ⚠️ Registering a card does **not** put it in any stack. The Worldbuilding
    /// stack ships **empty** (Doc 2 AC7) — nothing appears without an explicit
    /// writer action. These are offered in the "+" menu and nowhere else.
    static func registerObjectCards() {
        register(ObjectCardType<CharactersProvider>.self)
        register(ObjectCardType<LocationsProvider>.self)
        register(ObjectCardType<BuildingsProvider>.self)
        register(ObjectCardType<VehiclesProvider>.self)
        register(ObjectCardType<ItemsProvider>.self)
        register(ObjectCardType<MapsProvider>.self)
        register(ObjectCardType<ArtifactsProvider>.self)
        register(ObjectCardType<ChroniclesProvider>.self)
        register(ObjectCardType<FactionsProvider>.self)
        register(ObjectCardType<RulesProvider>.self)
    }
}
