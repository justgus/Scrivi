import Foundation

/// The object-kind scope partition, **derived from ScriviCore** rather than
/// restated in Swift (EP-034 SP-116 T-0429, design doc D5; closes I-0140).
///
/// ## Why this type exists
///
/// CLAUDE.md's standing rule: *any list that partitions or enumerates
/// `ObjectKind` must derive from `kAllStorableKinds` and
/// `objectKindIsWorldScoped()` — a restatement is a defect on sight, even when it
/// is currently correct.* That rule had been broken **eight times** by the end of
/// EP-031, once badly enough to block object creation in the app entirely.
///
/// ⚠️ Swift kept breaking it for a structural reason, not a careless one: **no C
/// ABI endpoint exposed a kind's scope**, so Swift could not derive what the
/// boundary never told it, and wrote `kind != "source"` instead. SP-116 added
/// `scrivi_list_object_kinds`; this type is the Swift side of that cure.
///
/// ## Two properties worth keeping
///
/// - **Loaded once, from the core.** A kind's scope is a property of the model,
///   not of a project, so the table needs no project root and never varies by
///   document. It is fetched on first use and cached for the process.
/// - ⚠️ **The fallback is not a second copy of the rule.** If the core cannot be
///   reached, `isWorldScoped` returns `true` for any kind it does not recognise.
///   That is deliberate: treating a kind as world-scoped makes the app *ask* for
///   a world, which fails loudly and recoverably. Guessing project-scope would
///   silently pass an empty `worldID` and reproduce the SP-104 outage. **It is a
///   safety default, not a restatement — it names no kinds.**
enum ObjectKindScope {

    /// Kind name → world-scoped, exactly as ScriviCore reports it.
    private static let table: [String: Bool] = load()

    private static func load() -> [String: Bool] {
        do {
            let result = try ScriviEngine().listObjectKinds()
            // ⚠️ `Dictionary(uniqueKeysWithValues:)` TRAPS on a duplicate key —
            // it would crash the app outright if the core ever emitted a kind
            // twice. That is an unacceptable failure mode for a type whose whole
            // job is to degrade safely, so duplicates resolve to world-scoped
            // (`||`) instead: the same conservative direction as the fallback
            // below, for the same reason.
            return result.kinds.reduce(into: [String: Bool]()) { table, info in
                table[info.kind] = (table[info.kind] ?? false) || info.isWorldScoped
            }
        } catch {
            // ⚠️ Deliberately empty rather than a hardcoded partition — see the
            // fallback note above. An empty table sends every kind through the
            // world-scoped path, which fails visibly instead of corrupting scope.
            return [:]
        }
    }

    /// Whether objects of `kind` live in a world package rather than the project.
    ///
    /// Unknown kinds report `true` — see the fallback rationale on the type.
    static func isWorldScoped(_ kind: String) -> Bool {
        table[kind] ?? true
    }

    /// Every storable kind ScriviCore knows, for callers that need to enumerate
    /// rather than test. Empty only if the core could not be reached.
    static var allKinds: [String] {
        Array(table.keys).sorted()
    }
}
