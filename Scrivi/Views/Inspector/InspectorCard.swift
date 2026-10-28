import SwiftUI

// T-0359 — the Scene Inspector card protocol + registry.
// Design: Scrivi_Scene_Inspector_Card_Framework_v0_1.md §3, §5.
//
// A card is a stable typeID + a title + a body view + optional per-card config.
// Swift is UI only (Architecture v0.3): cards read through ScriviEngine and never
// implement backend logic.

/// Which stack a card belongs to. The Properties tab is NOT a stack (§4.7), so it
/// has no case here.
enum InspectorStack: String, Codable, CaseIterable, Sendable {
    case writing
    case worldbuilding
}

/// The three bottom tabs, in their ruled order: Writing | Worldbuilding | Properties.
/// Writing is the default when nothing is persisted (§4.7).
enum InspectorTab: String, Codable, CaseIterable, Identifiable, Sendable {
    case writing
    case worldbuilding
    case properties

    var id: String { rawValue }

    var title: String {
        switch self {
        case .writing:       return "Writing"
        case .worldbuilding: return "Worldbuilding"
        case .properties:    return "Properties"
        }
    }

    var systemImage: String {
        switch self {
        case .writing:       return "pencil.line"
        case .worldbuilding: return "globe"
        case .properties:    return "info.circle"
        }
    }

    /// The card stack this tab renders, or nil for Properties (a fixed view, §4.8).
    var stack: InspectorStack? {
        switch self {
        case .writing:       return .writing
        case .worldbuilding: return .worldbuilding
        case .properties:    return nil
        }
    }
}

/// How a stack is ordered. Sort is per-STACK, not per-card (C6 ruling, §4.5) — three
/// cards sorted by name while two sort by date is incoherent to read and fiddly to set.
enum InspectorSort: String, Codable, CaseIterable, Sendable {
    case manual          // writer-ordered; backed by Doc 1 §5.2 sortIndex for object cards
    case name
    case recentlyAdded

    var title: String {
        switch self {
        case .manual:        return "Manual"
        case .name:          return "Name"
        case .recentlyAdded: return "Recently Added"
        }
    }
}

/// Per-card configuration (C6): collapsed state plus an optional kind sub-filter for
/// object cards. Sort deliberately lives on the stack, not here.
struct CardConfig: Codable, Equatable, Sendable {
    var collapsed: Bool = false
    var kindFilter: String? = nil

    init(collapsed: Bool = false, kindFilter: String? = nil) {
        self.collapsed = collapsed
        self.kindFilter = kindFilter
    }
}

/// What a card body is handed when it renders.
@MainActor
struct CardContext {
    let sceneID: String
    let projectRootPath: String
    let engine: ScriviEngine
    let config: CardConfig

    /// Every scene in manuscript order (I-0119). A card that holds a draft across
    /// a scene change needs to NAME the scene the draft belongs to — the writer
    /// is looking at a different one by then, and "it will be saved" is otherwise
    /// ambiguous between the two. Position in this array is the scene's number.
    var allSceneIDs: [String] = []

    /// The project's undo/redo history, when open. Only the `history` card uses it;
    /// nil when history failed to open, in which case that card reports it in place
    /// rather than the stack failing (§7.1).
    var history: HistoryCapture? = nil

    /// Scene-local UTF-8 byte offset of the caret, or nil when unknown. Used by the
    /// history card to bold the entry the caret is sitting inside.
    var caretByteOffset: Int? = nil

    /// Monotonic counter bumped whenever the history tree changes (I-0105). The
    /// history card folds it into its `.task(id:)` so a commit re-fetches the tree;
    /// reloading on `sceneID` alone left the card stale until the project reopened.
    var historyRevision: Int = 0

    /// Monotonic counter bumped whenever world availability changes — a world is
    /// reconnected, relinked, added, or removed (I-0128).
    ///
    /// ⚠️ **Object cards keyed on `sceneID` alone, so reconnecting a drive did not
    /// refresh them.** The project-level warning cleared and the graph relinked, but
    /// each card kept rendering the pending entries it had loaded while the world was
    /// away, until the writer changed scenes and forced a rebuild. **This is exactly
    /// I-0105 one layer over** — there a history commit left the card stale; here a
    /// world reappearing does. Same cause: state changed somewhere the card's
    /// `.task(id:)` could not observe.
    var worldRevision: Int = 0

    /// Bumped whenever an object is edited OUTSIDE the inspector — today, a save
    /// in the Object Detail Sheet (I-0155).
    ///
    /// ⚠️ The two surfaces edit the same fields and can both be open at once. A
    /// rename in the sheet left the card showing the old name until the writer
    /// changed scenes, so the app displayed two names for one object and neither
    /// was marked stale. Same mechanism as `worldRevision`: a token in the card's
    /// `.task(id:)`, so a reload is a data dependency rather than a broadcast.
    var objectRevision: Int = 0

    /// Announces that a card CHANGED an object, so the host can refresh other
    /// surfaces showing it — today the Object Detail Sheet (I-0160).
    ///
    /// ⚠️ The counterpart of `objectRevision`, which carries changes the other
    /// way. Both directions are needed: the two surfaces edit the same fields and
    /// can be open at once, so whichever one is not editing must be told.
    var onObjectChanged: (() -> Void)? = nil

    /// Asks the HOST to open the Object Detail Sheet for this object
    /// (EP-034 SP-117, T-0438 / R7).
    ///
    /// ⚠️ **A request, not a presentation.** D1-E ruled the sheet an
    /// *editor-level* pane, and a card lives inside the 280pt inspector — the
    /// width D1-C was rejected over. A card that presented the sheet itself would
    /// either be confined to that strip or reach outside its own hierarchy.
    ///
    /// ⚠️ Optional, so a host that does not offer the sheet simply does not pass
    /// one: the card then shows no "View Detail" affordance rather than offering
    /// an action that goes nowhere.
    var openObjectDetail: ((_ objectID: String, _ kind: String,
                            _ worldID: String, _ displayName: String) -> Void)? = nil

    /// The **stack's** sort, applied by every card in it (C6 / §4.5 — sort is per
    /// stack, never per card). Object cards order their contents by this; cards
    /// with no orderable content ignore it.
    ///
    /// ⚠️ A card must not grow its own sort control. Three cards sorted by name
    /// while two sort by date is exactly what the C6 ruling exists to prevent.
    var sort: InspectorSort = .manual

    /// Who to attribute created objects to. Nil when identity has not bootstrapped,
    /// in which case a card must refuse to create rather than inventing authorship
    /// (T-0388).
    var authorshipRef: AuthorshipRef? = nil

    /// SP-197 (EP-052 S3) — the Inspector's ONE read for this scene, shared by every card. Nil only in a host that has none
    /// (a test building one card by hand).
    var inspector: SceneInspectorReader? = nil

    /// The read's parts for THIS scene once they have landed; nil while the read is out (the card keeps its last contents,
    /// dimmed: `sceneReadGate`).
    var sceneRead: SceneInspectorParts? {
        guard let read = inspector?.read, read.sceneID == sceneID else { return nil }
        return read.parts
    }

    /// The whole read failed for this scene (not one part): the card reports it in place.
    var sceneReadFailure: Error? {
        guard let f = inspector?.failure, f.sceneID == sceneID else { return nil }
        return f.error
    }

    /// One part of this scene's read, the whole read's failure standing in for every part; nil while the read is out.
    func scenePart<T>(_ part: (SceneInspectorParts) -> Result<T, ScriviError>) -> Result<T, Error>? {
        if let parts = sceneRead { return part(parts).mapError { $0 } }
        if let error = sceneReadFailure { return .failure(error) }
        return nil
    }

    /// Changes whenever a new read lands: a card's `.task(id:)` rebuilds its model from it.
    var sceneReadID: String { "\(sceneID)#\(inspector?.read?.sceneID ?? "")#\(inspector?.readCount ?? 0)" }
}

/// SP-197 (EP-052 S3, Background Writes v0.2 R3a/R3b) — the Inspector reads a scene ONCE, off the main actor.
///
/// ⛔ Each card used to read for itself in its model's `init` on the main actor: 32 calls and 78 file operations every
/// time the caret or the scroll entered a scene (v0.2 §1a), each a round trip over a share. Now the shell asks for one
/// `scrivi_load_scene_inspector`, the cards keep their last contents dimmed while it is out ("Loading…" after 400 ms,
/// SP-197 Q2), and they rebuild from it when it lands.
@Observable @MainActor final class SceneInspectorReader {
    /// The last read that landed — for an earlier scene while a new read is out.
    private(set) var read: SceneInspectorRead?
    /// Bumped on every landing, so a re-read of the same scene still rebuilds the cards.
    private(set) var readCount = 0
    /// The whole call failed (not one part): the cards report it in place.
    private(set) var failure: (sceneID: String, error: Error)?
    /// The read has been out 400 ms (Q2).
    private(set) var slow = false

    private var wanted: (engine: ScriviEngine, root: String, sceneID: String, citations: Bool, key: String)?
    private var generation = 0
    private var landedGeneration = 0
    /// ✅ [SP-197] rig: ONE read out at a time. Cancelling a task never stops the C call it is in, so every scrolled-past
    /// scene's read used to run, one after another, each holding the project's disk lock. Now a read that lands while a
    /// newer scene is wanted is dropped, and the scene wanted NOW is read.
    private var reading = false
    /// Reads sent to the core (a test counts them).
    private(set) var readsStarted = 0

    /// Asks for `sceneID`'s read; `key` folds in whatever else must re-read it (world and object revisions). A repeat of
    /// the same key does nothing.
    func want(engine: ScriviEngine, projectRootPath: String, sceneID: String, includeCitations: Bool, key: String) {
        guard wanted?.key != key else { return }
        wanted = (engine, projectRootPath, sceneID, includeCitations, key)
        start()
    }

    /// Reads the current scene again: after a card's own write, so every card shows what landed.
    func reload() { if wanted != nil { start() } }

    /// Every read off the main actor has the same revision race to settle (D4): a result whose revision moved is read
    /// again. shortcut: after 3 lost races the last read is applied anyway, since autosave can bump the revision
    /// faster than a slow share answers; a card's own write still re-reads (`reload`).
    private func start() {
        guard wanted != nil else { return }
        generation += 1
        let gen = generation
        slow = false
        Task { @MainActor [weak self] in
            try? await Task.sleep(for: .milliseconds(400))
            if let self, self.generation == gen, self.landedGeneration != gen { self.slow = true }
        }
        guard !reading else { return }   // the read out now is followed by one for what is wanted when it lands
        reading = true
        Task { [weak self] in
            var lostRaces = 0
            while true {
                guard let self, let w = self.wanted else { return }
                let gen = self.generation
                self.readsStarted += 1
                let outcome = await Task.detached(priority: .userInitiated) { () -> Result<SceneInspectorRead, Error> in
                    Result { try w.engine.loadSceneInspector(projectRootPath: w.root, sceneID: w.sceneID,
                                                             includeCitations: w.citations) }
                }.value
                guard self.generation == gen else { lostRaces = 0; continue }   // asked again while it was out
                if case .success(let read) = outcome, lostRaces < 2,
                   let now = try? w.engine.projectRevision(projectRootPath: w.root), now != read.revision {
                    lostRaces += 1
                    continue
                }
                switch outcome {
                case .failure(let error):
                    self.failure = (w.sceneID, error)
                case .success(let read):
                    self.failure = nil
                    self.read = read
                    self.readCount += 1
                }
                self.landedGeneration = gen
                self.slow = false
                self.reading = false
                return
            }
        }
    }
}

extension View {
    /// SP-197 Q2 — while the scene's read is out, the card shows its last contents dimmed and takes no edits;
    /// "Loading…" after 400 ms. A host with no reader (a hand-built card) is never gated.
    @MainActor func sceneReadGate(_ context: CardContext) -> some View {
        let waiting = context.inspector != nil && context.sceneRead == nil
            && context.inspector?.failure?.sceneID != context.sceneID
        return VStack(alignment: .leading, spacing: 4) {
            if waiting && context.inspector?.slow == true {
                Text("Loading…").font(.caption).foregroundStyle(.secondary)
            }
            self.opacity(waiting ? 0.45 : 1).disabled(waiting)
        }
    }
}

/// A card the inspector can render. Conformers are registered in `InspectorCardRegistry`
/// under their `typeID`, which is what `inspector-layout.json` persists.
@MainActor
protocol InspectorCard {
    /// Stable across releases — this string is written to disk. Renaming it orphans
    /// every layout entry that references it, so treat it as a schema key.
    static var typeID: String { get }
    static var title: String { get }
    static var systemImage: String { get }
    /// Which stack this card is offered in.
    static var stack: InspectorStack { get }

    init()

    /// The card's content, below the header chrome the stack draws.
    @ViewBuilder func body(context: CardContext) -> AnyView

    /// Throwing variant, for a card that can fail while *building* its content and wants
    /// the framework to present the failure (AC12, soft-failure scope — Doc 2 §7.1).
    ///
    /// Default implementation forwards to `body(context:)` and never throws, so adopting
    /// this is opt-in and existing cards are unaffected. A card that already surfaces its
    /// own load error inline (`CardErrorView`) should keep doing so — that message is more
    /// specific than the framework's generic fallback. This exists as the **backstop** for
    /// cards that don't, so stack isolation doesn't depend on every card author.
    ///
    /// ⚠️ Throwing is the only failure this can catch. A card whose view body **traps**
    /// takes the process down — SwiftUI has no mechanism to contain that (Doc 2 §7.1).
    func makeContent(context: CardContext) throws -> AnyView
}

extension InspectorCard {
    func makeContent(context: CardContext) throws -> AnyView { body(context: context) }
}

/// Type-erased handle so the registry can hold heterogeneous card types.
@MainActor
struct AnyInspectorCard {
    let typeID: String
    let title: String
    let systemImage: String
    let stack: InspectorStack
    private let makeBody: (CardContext) throws -> AnyView

    init<C: InspectorCard>(_ type: C.Type) {
        self.typeID = C.typeID
        self.title = C.title
        self.systemImage = C.systemImage
        self.stack = C.stack
        self.makeBody = { context in try C().makeContent(context: context) }
    }

    /// Builds the card's content, propagating a soft failure to the framework boundary
    /// (`CardBodyBoundary`) so one card's failure can't blank the stack (AC12).
    func body(context: CardContext) throws -> AnyView { try makeBody(context) }
}

/// Maps `typeID` → card implementation.
///
/// The registry is what keeps `inspector-layout.json` decoupled from Swift types: an
/// unknown typeID (a layout written by a newer Scrivi, or a card not yet built) is
/// **skipped with a notice, never a crash** (§5, AC11). That path is live from day one
/// because the default Writing stack names `tags`/`outline`/`todo`, which do not exist
/// until SP-091.
@MainActor
enum InspectorCardRegistry {

    private static var cards: [String: AnyInspectorCard] = [:]

    static func register<C: InspectorCard>(_ type: C.Type) {
        cards[C.typeID] = AnyInspectorCard(type)
    }

    static func card(for typeID: String) -> AnyInspectorCard? {
        cards[typeID]
    }

    static func isRegistered(_ typeID: String) -> Bool {
        cards[typeID] != nil
    }

    /// Cards offered in a stack, for the "add card" menu. Sorted by title so the menu
    /// order does not depend on registration order.
    static func available(in stack: InspectorStack) -> [AnyInspectorCard] {
        cards.values
            .filter { $0.stack == stack }
            .sorted { $0.title.localizedCaseInsensitiveCompare($1.title) == .orderedAscending }
    }

    /// Registers every card this build knows about. Called once at app start.
    /// SP-092 adds the `history` card here; EP-031 SP-098 adds the object cards.
    static func registerBuiltIns() {
        guard cards.isEmpty else { return }
        // Writing-tool cards (SP-091). These three are the default Writing stack.
        register(TagsCard.self)
        register(OutlineCard.self)
        register(TodoCard.self)
        // History card (SP-092 T-0366) — supersedes EP-019's T-0215 panel.
        register(HistoryCard.self)
        // Sources card (SP-102 T-0365) — ONE aggregate card, not one per source
        // (§3.1.1). Offered in the "+" menu; not in any default stack.
        register(SourcesCard.self)
        // Worldbuilding object cards (EP-031 SP-099 T-0386) — ten configurations
        // of ONE implementation. Registering them only OFFERS them in the "+"
        // menu; the Worldbuilding stack still ships empty (Doc 2 AC7).
        InspectorCardRegistry.registerObjectCards()
    }

    #if DEBUG
    /// Test seam — lets a test start from a known-empty registry.
    static func resetForTesting() { cards.removeAll() }
    #endif
}

// `PlaceholderCard` (SP-090 scaffolding) was retired in SP-091, replaced by the real
// `tags` / `outline` / `todo` cards in WritingToolCards.swift.
//
// A layout file written during SP-090 may still name "placeholder" in a scene's stack.
// That resolves through the unknown-typeID path (§5, AC11): reported in the card stack,
// never a crash, and removable via the per-card ✕ in `unknownCardsNotice`.
//
// I-0101: that removal affordance did NOT exist when the card was retired — the notice
// was display-only, stranding the entry. Any future card retirement must confirm the
// unknown-type path is *actionable*, not merely visible.
