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
}

/// Type-erased handle so the registry can hold heterogeneous card types.
@MainActor
struct AnyInspectorCard {
    let typeID: String
    let title: String
    let systemImage: String
    let stack: InspectorStack
    private let makeBody: (CardContext) -> AnyView

    init<C: InspectorCard>(_ type: C.Type) {
        self.typeID = C.typeID
        self.title = C.title
        self.systemImage = C.systemImage
        self.stack = C.stack
        self.makeBody = { context in C().body(context: context) }
    }

    func body(context: CardContext) -> AnyView { makeBody(context) }
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
