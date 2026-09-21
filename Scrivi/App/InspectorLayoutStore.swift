import Foundation

// T-0360 — inspector-layout.json (`scrivi.inspector-layout.v1`).
// Design: Scrivi_Scene_Inspector_Card_Framework_v0_1.md §C2, §4.7.
//
// Project-level and Git-visible: a card stack is a CREATIVE decision (this scene is
// about these characters), so it travels with the project. Deliberately NOT:
//   - the scene sidecar — EP-027 keeps those filesystem-authoritative and identity-
//     bearing; mixing view preference into them enlarges what EP-027 works to keep clean;
//   - workspace state / UserDefaults — that is per-device, and the layout must not
//     evaporate on another machine.

/// One card's entry in a stack.
struct InspectorCardEntry: Codable, Equatable, Sendable {
    var type: String
    var collapsed: Bool
    var kindFilter: String?

    init(type: String, collapsed: Bool = false, kindFilter: String? = nil) {
        self.type = type
        self.collapsed = collapsed
        self.kindFilter = kindFilter
    }

    enum CodingKeys: String, CodingKey { case type, collapsed, kindFilter }

    init(from decoder: Decoder) throws {
        let c = try decoder.container(keyedBy: CodingKeys.self)
        type = try c.decode(String.self, forKey: .type)
        // Tolerant of older/partial files: absent keys take defaults rather than throwing.
        collapsed = (try? c.decode(Bool.self, forKey: .collapsed)) ?? false
        kindFilter = try? c.decodeIfPresent(String.self, forKey: .kindFilter)
    }

    var config: CardConfig { CardConfig(collapsed: collapsed, kindFilter: kindFilter) }
}

/// Per-scene stacks. A scene absent from the file falls back to `defaultStacks`.
struct InspectorSceneLayout: Codable, Equatable, Sendable {
    var writing: [InspectorCardEntry]
    var worldbuilding: [InspectorCardEntry]

    init(writing: [InspectorCardEntry] = [], worldbuilding: [InspectorCardEntry] = []) {
        self.writing = writing
        self.worldbuilding = worldbuilding
    }

    init(from decoder: Decoder) throws {
        let c = try decoder.container(keyedBy: CodingKeys.self)
        writing = (try? c.decode([InspectorCardEntry].self, forKey: .writing)) ?? []
        worldbuilding = (try? c.decode([InspectorCardEntry].self, forKey: .worldbuilding)) ?? []
    }

    subscript(stack: InspectorStack) -> [InspectorCardEntry] {
        get { stack == .writing ? writing : worldbuilding }
        set { if stack == .writing { writing = newValue } else { worldbuilding = newValue } }
    }

    var isEmpty: Bool { writing.isEmpty && worldbuilding.isEmpty }
}

/// The on-disk document.
struct InspectorLayoutDocument: Codable, Equatable, Sendable {
    static let schemaID = "scrivi.inspector-layout.v1"

    var schema: String
    var selectedTab: InspectorTab
    var inspectorHidden: Bool
    var defaultStacks: [String: [InspectorCardEntry]]
    var stackSort: [String: InspectorSort]
    var scenes: [String: InspectorSceneLayout]

    /// A fresh project's layout, per the ruled defaults (§4.7, Doc 2 AC7/AC8):
    /// Worldbuilding ships EMPTY; Writing ships with empty tags/outline/todo cards.
    static func makeDefault() -> InspectorLayoutDocument {
        InspectorLayoutDocument(
            schema: schemaID,
            selectedTab: .writing,
            inspectorHidden: false,
            defaultStacks: [
                InspectorStack.worldbuilding.rawValue: [],
                InspectorStack.writing.rawValue: [
                    InspectorCardEntry(type: "tags"),
                    InspectorCardEntry(type: "outline"),
                    InspectorCardEntry(type: "todo"),
                ],
            ],
            stackSort: [
                InspectorStack.writing.rawValue: .manual,
                InspectorStack.worldbuilding.rawValue: .manual,
            ],
            scenes: [:]
        )
    }

    init(schema: String,
         selectedTab: InspectorTab,
         inspectorHidden: Bool,
         defaultStacks: [String: [InspectorCardEntry]],
         stackSort: [String: InspectorSort],
         scenes: [String: InspectorSceneLayout]) {
        self.schema = schema
        self.selectedTab = selectedTab
        self.inspectorHidden = inspectorHidden
        self.defaultStacks = defaultStacks
        self.stackSort = stackSort
        self.scenes = scenes
    }

    init(from decoder: Decoder) throws {
        let c = try decoder.container(keyedBy: CodingKeys.self)
        schema = (try? c.decode(String.self, forKey: .schema)) ?? Self.schemaID
        // An unknown tab string (a newer Scrivi wrote it) degrades to the ruled
        // default rather than failing the whole load.
        selectedTab = (try? c.decode(InspectorTab.self, forKey: .selectedTab)) ?? .writing
        inspectorHidden = (try? c.decode(Bool.self, forKey: .inspectorHidden)) ?? false
        defaultStacks = (try? c.decode([String: [InspectorCardEntry]].self, forKey: .defaultStacks))
            ?? Self.makeDefault().defaultStacks
        stackSort = (try? c.decode([String: InspectorSort].self, forKey: .stackSort))
            ?? Self.makeDefault().stackSort
        scenes = (try? c.decode([String: InspectorSceneLayout].self, forKey: .scenes)) ?? [:]
    }

    func defaultStack(_ stack: InspectorStack) -> [InspectorCardEntry] {
        defaultStacks[stack.rawValue] ?? []
    }

    func sort(for stack: InspectorStack) -> InspectorSort {
        stackSort[stack.rawValue] ?? .manual
    }
}

/// Result of resolving a scene's stack: the cards that resolved, plus the typeIDs that
/// did not. Callers surface `unknownTypeIDs` as a notice (AC11) instead of dropping them
/// silently — a writer whose layout references a card this build lacks should be told.
@MainActor
struct ResolvedStack {
    var entries: [InspectorCardEntry] = []
    var unknownTypeIDs: [String] = []
}

/// Loads and saves `inspector-layout.json` — THROUGH ScriviCore (EP-041 / T-0507).
///
/// ⚠️ This class used to read and write the file itself. That was [I-0197] Class B:
/// the same schema had two app-side owners, Swift and Qt, and they had already
/// drifted ([I-0215] — Linux preserved unknown keys, Apple dropped them).
/// ⛔ DO NOT REINTRODUCE FILE I/O HERE. The core owns atomicity, durability and
/// repair; this class owns MEANING — the typed document, the defaults, and the
/// decision of what an absent or damaged layout should look like on screen.
@Observable @MainActor final class InspectorLayoutStore {

    private(set) var document: InspectorLayoutDocument
    /// The project this layout belongs to; `nil` when there is no project (the
    /// store then serves defaults and never persists).
    ///
    /// ⚠️ EP-041 / T-0507: this replaced a `fileURL`. The app no longer knows where
    /// `inspector-layout.json` lives — that is ScriviCore's business now, and the
    /// path exists in exactly one place (`inspectorLayoutPath`, scrivi_c_api.cpp).
    private let projectRootPath: String?

    /// ⚠️ EP-041 / T-0507: the store now talks to ScriviCore instead of the
    /// filesystem, so it needs an engine. INJECTED, matching `BufferService` —
    /// `feedback_look_for_existing_pattern_first`; there is no `ScriviEngine.shared`
    /// in this app and inventing one would be a second way to reach the core.
    private let engine: ScriviEngine

    /// The file exactly as it was read, including every key this build does not
    /// understand (T-0536, [I-0215]).
    ///
    /// ⚠️ **WITHOUT THIS, A ROUND TRIP THROUGH APPLE DESTROYED DATA.**
    /// `InspectorLayoutDocument` is a fixed `Codable` struct, so decoding discarded
    /// any key it did not declare and the next `save()` wrote the file back without
    /// them. The same projects are opened on Linux, whose store deliberately keeps
    /// the whole document (`InspectorLayoutStore.cpp:97-100`: *"THE WHOLE DOCUMENT is
    /// kept … This is what makes the round trip lossless"*), so Apple was silently
    /// deleting what the other platform preserved.
    ///
    /// ⚠️ **IT IS A SOURCE FOR UNKNOWN KEYS ONLY.** For the keys this build owns, the
    /// typed `document` is always authoritative — see `mergedForSave()`. Reading a
    /// known value back out of here would resurrect a setting the writer just changed.
    ///
    /// `nil` when no file was read (absent, or unreadable): there is nothing to
    /// preserve, and `save()` then writes the typed document alone.
    private var rawDocument: [String: Any]?

    /// Last load's unreadable-file error, if any. Non-nil means the store fell back to
    /// defaults; the UI can note it rather than silently discarding the writer's layout.
    private(set) var loadError: String?

    init(engine: ScriviEngine, projectRootPath: String?) {
        self.engine = engine
        guard let root = projectRootPath else {
            self.projectRootPath = nil
            self.document = .makeDefault()
            return
        }
        self.projectRootPath = root

        let fetched: InspectorLayoutFetch
        do {
            fetched = try engine.getInspectorLayout(projectRootPath: root)
        } catch {
            // The CALL itself failed (not the document). Defaults + report, same as
            // an unreadable document: we still have no layout to show.
            self.document = .makeDefault()
            self.loadError = "Could not read inspector-layout.json (\(error.localizedDescription)). "
                + "Using default inspector layout; the existing file was left untouched."
            return
        }

        switch fetched.status {
        case .absent:
            // ⚠️ NORMAL, NOT AN ERROR. Every project created before this file existed
            // has no layout; so does every project a writer has never opened the
            // inspector in. No `loadError` — there is nothing wrong to report.
            self.document = .makeDefault()

        case .unreadable:
            // ⛔ The core reported damage and deliberately did NOT overwrite it. We
            // default and SAY SO — this is the case the writer may be able to
            // recover by hand, and it is why `unreadable` is distinct from `absent`.
            self.document = .makeDefault()
            self.loadError = "Could not read inspector-layout.json (\(fetched.message ?? "unreadable")). "
                + "Using default inspector layout; the existing file was left untouched."

        case .ok:
            guard let json = fetched.documentJSON else {
                self.document = .makeDefault()
                return
            }
            let data = Data(json.utf8)
            do {
                self.document = try JSONDecoder().decode(InspectorLayoutDocument.self, from: data)
            } catch {
                // The document is valid JSON (the core parsed it) but does not fit
                // our typed shape. Defaults, and keep the raw copy below so a later
                // save does not destroy what we could not read.
                self.document = .makeDefault()
                self.loadError = "Could not read inspector-layout.json (\(error.localizedDescription)). "
                    + "Using default inspector layout; the existing file was left untouched."
            }
            // T-0536: keep the document as the core returned it so `save()` can put
            // back the keys the typed decode above just dropped. Parsed SECOND and
            // non-fatally — a document that decoded cleanly must still load even if
            // this does not produce an object.
            self.rawDocument = (try? JSONSerialization.jsonObject(with: data))
                as? [String: Any]
        }
    }

    // MARK: — Reads

    /// The card entries for a scene's stack, falling back to the project default when the
    /// scene has no saved layout. Unknown typeIDs are separated out, not dropped (AC11).
    func resolvedStack(sceneID: String, stack: InspectorStack) -> ResolvedStack {
        let entries = document.scenes[sceneID].map { $0[stack] } ?? document.defaultStack(stack)
        var result = ResolvedStack()
        for entry in entries {
            if InspectorCardRegistry.isRegistered(entry.type) {
                result.entries.append(entry)
            } else {
                result.unknownTypeIDs.append(entry.type)
            }
        }
        return result
    }

    /// Raw entries including unknown types — used when mutating, so an unknown card is
    /// preserved through an edit rather than quietly removed by a round-trip.
    func rawEntries(sceneID: String, stack: InspectorStack) -> [InspectorCardEntry] {
        document.scenes[sceneID].map { $0[stack] } ?? document.defaultStack(stack)
    }

    func sort(for stack: InspectorStack) -> InspectorSort { document.sort(for: stack) }

    // MARK: — Writes

    func setSelectedTab(_ tab: InspectorTab) {
        guard document.selectedTab != tab else { return }
        document.selectedTab = tab
        save()
    }

    func setInspectorHidden(_ hidden: Bool) {
        guard document.inspectorHidden != hidden else { return }
        document.inspectorHidden = hidden
        save()
    }

    func setSort(_ sort: InspectorSort, for stack: InspectorStack) {
        guard document.stackSort[stack.rawValue] != sort else { return }
        document.stackSort[stack.rawValue] = sort
        save()
    }

    func setEntries(_ entries: [InspectorCardEntry], sceneID: String, stack: InspectorStack) {
        var layout = document.scenes[sceneID]
            ?? InspectorSceneLayout(writing: document.defaultStack(.writing),
                                    worldbuilding: document.defaultStack(.worldbuilding))
        layout[stack] = entries
        document.scenes[sceneID] = layout
        save()
    }

    func addCard(typeID: String, sceneID: String, stack: InspectorStack) {
        var entries = rawEntries(sceneID: sceneID, stack: stack)
        guard !entries.contains(where: { $0.type == typeID }) else { return }
        entries.append(InspectorCardEntry(type: typeID))
        setEntries(entries, sceneID: sceneID, stack: stack)
    }

    func removeCard(typeID: String, sceneID: String, stack: InspectorStack) {
        var entries = rawEntries(sceneID: sceneID, stack: stack)
        entries.removeAll { $0.type == typeID }
        setEntries(entries, sceneID: sceneID, stack: stack)
    }

    func moveCards(fromOffsets: IndexSet, toOffset: Int, sceneID: String, stack: InspectorStack) {
        var entries = rawEntries(sceneID: sceneID, stack: stack)
        entries.move(fromOffsets: fromOffsets, toOffset: toOffset)
        setEntries(entries, sceneID: sceneID, stack: stack)
    }

    func setCollapsed(_ collapsed: Bool, typeID: String, sceneID: String, stack: InspectorStack) {
        var entries = rawEntries(sceneID: sceneID, stack: stack)
        guard let idx = entries.firstIndex(where: { $0.type == typeID }) else { return }
        guard entries[idx].collapsed != collapsed else { return }
        entries[idx].collapsed = collapsed
        setEntries(entries, sceneID: sceneID, stack: stack)
    }

    /// "Apply this card layout to all scenes" (C1). Explicit and writer-invoked — the
    /// alternative (a project default that silently mutates every non-overridden scene)
    /// was rejected because inheritance is invisible.
    ///
    /// Applies to every scene the writer names, and updates the project default so
    /// scenes created later inherit it too.
    func applyStackToAllScenes(sceneID: String, stack: InspectorStack, allSceneIDs: [String]) {
        let entries = rawEntries(sceneID: sceneID, stack: stack)
        for id in allSceneIDs {
            var layout = document.scenes[id]
                ?? InspectorSceneLayout(writing: document.defaultStack(.writing),
                                        worldbuilding: document.defaultStack(.worldbuilding))
            layout[stack] = entries
            document.scenes[id] = layout
        }
        document.defaultStacks[stack.rawValue] = entries
        save()
    }

    /// Drops a deleted scene's entry (cascade, mirroring Doc 1 §5.5).
    func removeScene(_ sceneID: String) {
        guard document.scenes[sceneID] != nil else { return }
        document.scenes.removeValue(forKey: sceneID)
        save()
    }

    // MARK: — Persistence

    /// The bytes to write: the typed document, with any keys the build does not
    /// understand carried over from the file as it was read (T-0536, [I-0215]).
    ///
    /// ⚠️ **DIRECTION MATTERS.** The typed document is encoded first and then written
    /// OVER the retained raw object — never the other way round. Every key this build
    /// owns therefore takes the value the writer just set, and only keys the typed
    /// encode did not produce survive from the old file.
    ///
    /// ⚠️ **Preservation is TOP-LEVEL, and that is a deliberate limit, not an
    /// oversight.** A merge deep enough to preserve an unknown key *inside* a card
    /// entry would have to reconcile arrays the writer may have reordered, inserted
    /// into, or deleted from — with no identity to match on, that reconciliation can
    /// silently reattach a stale value to the wrong card. Losing an unknown key
    /// inside a card is bad; putting one on the wrong card is worse. ✅ The nested
    /// case belongs with the schema work in [SP-141], which can give entries identity.
    private func mergedForSave() throws -> Data {
        let encoder = JSONEncoder()
        encoder.outputFormatting = [.prettyPrinted, .sortedKeys]

        guard let rawDocument, !rawDocument.isEmpty else {
            return try encoder.encode(document)
        }

        // Round-trip the typed document through JSON so the merge compares like with
        // like — `Codable` key names, not Swift property names.
        let typedData = try encoder.encode(document)
        guard let typed = try JSONSerialization.jsonObject(with: typedData)
                as? [String: Any] else {
            return typedData
        }

        var merged = rawDocument
        for (key, value) in typed { merged[key] = value }

        // `.sortedKeys` is not available for JSONSerialization, so sort explicitly:
        // this file is Git-visible project state and unstable key order would churn
        // the diff on every save.
        return try JSONSerialization.data(
            withJSONObject: merged,
            options: [.prettyPrinted, .sortedKeys])
    }

    func save() {
        guard let root = projectRootPath else { return }
        do {
            let data = try mergedForSave()
            guard let json = String(data: data, encoding: .utf8) else {
                loadError = "Could not save inspector-layout.json (not UTF-8)."
                return
            }
            // ⚠️ ATOMICITY IS THE CORE'S JOB NOW (EP-041 ruling): it writes a temp
            // and renames, the same discipline this method used to implement itself.
            // ⛔ Do not reintroduce a write here — that is the [I-0197] bypass.
            try engine.putInspectorLayout(projectRootPath: root, documentJson: json)
            // T-0536: the file on disk is now what we just merged, so the retained
            // copy must track it. Without this a SECOND save in the same session
            // would merge against the state at load and could resurrect a top-level
            // key the writer's first save had legitimately removed.
            rawDocument = (try? JSONSerialization.jsonObject(with: data)) as? [String: Any]
        } catch {
            loadError = "Could not save inspector-layout.json (\(error.localizedDescription))."
        }
    }
}
