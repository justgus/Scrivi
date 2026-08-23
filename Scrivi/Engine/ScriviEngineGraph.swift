import Foundation
#if os(macOS) || os(iOS)
import ScriviCore
#endif

// ScriviEngineGraph — EP-031 SP-099 / T-0407.
//
// Swift wrappers for the relationship graph, object discovery, and world
// endpoints. Every one of these has existed and been exported in scrivi.h since
// SP-096–SP-098; none had a Swift wrapper, so the graph has never been reachable
// from the Apple layer at all. That gap is what this file closes.
//
// Swift is UI only (Architecture v0.3): these wrap and decode, and implement no
// behavior. Direction normalization, duplicate rejection, cascade-prune, and the
// pending-vs-dangling distinction all live in ScriviCore and are never re-decided
// here.

#if os(macOS) || os(iOS)

public extension ScriviEngine {

    // MARK: — Relationship graph (SP-096)

    /// Creates ONE canonical edge. ScriviCore normalizes direction and rejects
    /// duplicates from either creation order.
    ///
    /// ⚠️ **Never create a reverse edge to "complete" a relationship.** The inverse
    /// is a read-time label projection off the same stored record (Doc 1 §5.2) —
    /// writing both directions is exactly the Cumberland trap the canonical-edge
    /// design exists to prevent.
    ///
    /// - Throws: `ScriviError` with `detail == "duplicateEdge"` if the relationship
    ///   already exists, or `isWorldPending == true` if either endpoint's world is
    ///   unavailable (the graph is frozen toward it, Doc 3 §4.6).
    func createEdge(
        projectRootPath: String,
        fromID: String,
        toID: String,
        relationTypeCode: String,
        note: String = ""
    ) throws -> EdgeResult {
        let raw = projectRootPath.withCString { prp in
            fromID.withCString { f in
                toID.withCString { t in
                    relationTypeCode.withCString { rt in
                        note.withCString { n in
                            scrivi_create_edge(prp, f, t, rt, n)
                        }
                    }
                }
            }
        }
        return try decodeC(raw)
    }

    /// Deletes an edge. **The objects at both ends survive** — Doc 1 §5.5: the far
    /// endpoint becomes an orphan and stays findable. This is what backs
    /// "Remove from scene" (AC22); it is never a way to delete an object.
    func deleteEdge(projectRootPath: String, edgeID: String) throws -> DeleteEdgeResult {
        let raw = projectRootPath.withCString { prp in
            edgeID.withCString { eid in
                scrivi_delete_edge(prp, eid)
            }
        }
        return try decodeC(raw)
    }

    /// Every edge touching `endpointID` in either direction, each already carrying
    /// the label that reads correctly *from that endpoint*. This is the object
    /// card's entire read path (Doc 2 §6): pass a sceneID, filter by kind.
    ///
    /// Rows whose far endpoint sits in an unavailable world come back with
    /// `otherPending == true` plus a cached `otherDisplayName` — named, never a
    /// bare ID, which is what makes pending presentation possible (Doc 2 §7.2).
    func listEdgesFor(projectRootPath: String, endpointID: String) throws -> ListEdgesResult {
        let raw = projectRootPath.withCString { prp in
            endpointID.withCString { eid in
                scrivi_list_edges_for(prp, eid)
            }
        }
        return try decodeC(raw)
    }

    /// Every edge held pending because its world is unavailable, project-wide.
    ///
    /// ⚠️ **Reports; repairs nothing.** Pending is not dangling: these edges are
    /// held, never pruned, never modified. Feeds the warning surface beneath the
    /// Timeline (Doc 2 §7.3) — the count of what the writer would lose.
    func listPendingEdges(projectRootPath: String) throws -> ListPendingEdgesResult {
        let raw = projectRootPath.withCString { prp in
            scrivi_list_pending_edges(prp)
        }
        return try decodeC(raw)
    }

    // MARK: — Object discovery (SP-098 T-0378)

    /// Objects in the project, optionally filtered by kind. Backs the picker,
    /// which lists **all objects across all worlds, deliberately unfiltered** by
    /// the scene's world association (Doc 3 §4.1, AC17).
    ///
    /// Objects in an unavailable world are simply absent here rather than reported
    /// as deleted. An unrecognised `kind` throws rather than returning an empty
    /// list — "you have no characters" must never be the way a typo presents.
    func listObjects(projectRootPath: String, kind: String = "") throws -> ListObjectsResult {
        let raw = projectRootPath.withCString { prp in
            kind.withCString { k in
                scrivi_list_objects(prp, k)
            }
        }
        return try decodeC(raw)
    }

    /// Objects present in the index but touched by no edge. Doc 1 §5.5: deleting a
    /// relationship must leave the object **findable**, not silently stranded.
    func listOrphanedObjects(projectRootPath: String) throws -> ListObjectsResult {
        let raw = projectRootPath.withCString { prp in
            scrivi_list_orphaned_objects(prp)
        }
        return try decodeC(raw)
    }

    /// Promotes an object to another kind (`item` → `artifact`), **preserving its
    /// `objectID` and rewriting zero edges** — SP-098 T-0379 proves the edge log
    /// stays byte-identical across a promotion.
    func promoteObject(
        projectRootPath: String,
        objectID: String,
        targetKind: String,
        worldID: String = ""
    ) throws -> PromoteObjectResult {
        let raw = projectRootPath.withCString { prp in
            objectID.withCString { oid in
                targetKind.withCString { tk in
                    worldID.withCString { wid in
                        scrivi_promote_object(prp, oid, tk, wid)
                    }
                }
            }
        }
        return try decodeC(raw)
    }

    // MARK: — Worlds (SP-097)

    /// Creates a new `.scrivworld` package and binds this project to it.
    ///
    /// The package is **self-contained and identity-bearing** — its `worldID` is
    /// what every later resolution verifies against, so a same-named package with
    /// a different identity is refused rather than substituted (AC8).
    func createWorld(
        projectRootPath: String,
        packagePath: String,
        displayName: String,
        epochLabel: String = ""
    ) throws -> WorldRecordResult {
        let raw = projectRootPath.withCString { prp in
            packagePath.withCString { pp in
                displayName.withCString { dn in
                    epochLabel.withCString { el in
                        scrivi_create_world(prp, pp, dn, el)
                    }
                }
            }
        }
        return try decodeC(raw)
    }

    /// Binds an **existing** package to this project. Read-only toward the world —
    /// takes no lock, and never modifies the package.
    func addWorld(projectRootPath: String, packagePath: String) throws -> WorldRecordResult {
        let raw = projectRootPath.withCString { prp in
            packagePath.withCString { pp in
                scrivi_add_world(prp, pp)
            }
        }
        return try decodeC(raw)
    }

    /// Worlds this project references, each with its current availability status.
    /// Backs the Worlds menu and the picker's world grouping.
    func listWorlds(projectRootPath: String) throws -> ListWorldsResult {
        let raw = projectRootPath.withCString { prp in
            scrivi_list_worlds(prp)
        }
        return try decodeC(raw)
    }

    /// Current availability of one world.
    ///
    /// ⚠️ ScriviCore reports only `available`, `missing`, or `unavailable` today —
    /// `offline` and `unmounted` are declared but never produced (`WorldTypes.hpp:68`).
    /// Refining them is Apple-layer work (SP-102 / T-0389, AC24). Until then a
    /// generic `unavailable` is the honest answer and must not be dressed up as a
    /// specific one: a wrong "missing" invites a destructive remedy.
    func getWorldStatus(projectRootPath: String, worldID: String) throws -> WorldStatusResult {
        let raw = projectRootPath.withCString { prp in
            worldID.withCString { wid in
                scrivi_get_world_status(prp, wid)
            }
        }
        return try decodeC(raw)
    }

    /// This project's binding to a world, including the **cached index** of object
    /// names. That cache is what lets a pending card show names instead of opaque
    /// IDs while the world is away (Doc 3 §5).
    func getWorldBinding(projectRootPath: String, worldID: String) throws -> WorldBindingResult {
        let raw = projectRootPath.withCString { prp in
            worldID.withCString { wid in
                scrivi_get_world_binding(prp, wid)
            }
        }
        return try decodeC(raw)
    }

    /// Re-points a moved world. ScriviCore **verifies the `worldID`** before
    /// accepting the path — a same-named package with a different identity is
    /// refused, never silently substituted (AC8).
    func relinkWorld(
        projectRootPath: String,
        worldID: String,
        newPackagePath: String
    ) throws -> RelinkWorldResult {
        let raw = projectRootPath.withCString { prp in
            worldID.withCString { wid in
                newPackagePath.withCString { np in
                    scrivi_relink_world(prp, wid, np)
                }
            }
        }
        return try decodeC(raw)
    }

    /// Removes only **this project's reference**; the world package itself is never
    /// touched.
    ///
    /// ⚠️ **Destructive to pending links** and must stay hard to reach: Doc 2 §7.3
    /// puts it in the Worlds menu alone — never inline on a card, where it would
    /// read as routine tidying.
    func removeWorldReference(
        projectRootPath: String,
        worldID: String
    ) throws -> RemoveWorldReferenceResult {
        let raw = projectRootPath.withCString { prp in
            worldID.withCString { wid in
                scrivi_remove_world_reference(prp, wid)
            }
        }
        return try decodeC(raw)
    }

    // MARK: — Relation type vocabulary (SP-096)

    /// The relation-type vocabulary. Cards pick their type from this rather than
    /// hardcoding codes, so a project's custom types are honoured.
    func listRelationTypes(projectRootPath: String) throws -> ListRelationTypesResult {
        let raw = projectRootPath.withCString { prp in
            scrivi_list_relation_types(prp)
        }
        return try decodeC(raw)
    }
}

#endif

// MARK: — Result types
//
// Outside the #if: the visionOS stub shares them, exactly as ScriviEngine.swift's
// result types do.

public struct EdgeResult: Decodable, Sendable {
    public let edgeID:       String
    public let from:         String
    public let to:           String
    public let relationType: String
    public let note:         String
    public let sortIndex:    Double
}

public struct DeleteEdgeResult: Decodable, Sendable {
    public let edgeID:  String
    public let deleted: Bool
}

/// One edge as seen **from** a particular endpoint — the same stored record reads
/// forward from one end and inverse from the other (Doc 1 §5.2).
public struct EdgeView: Decodable, Sendable, Identifiable {
    public let edgeID:       String
    public let from:         String
    public let to:           String
    public let relationType: String
    public let note:         String
    public let sortIndex:    Double

    /// True when the queried endpoint is this edge's `from`.
    public let isForward: Bool
    /// The label that reads correctly from the queried endpoint.
    public let label:     String

    public let otherID:          String
    /// Cached even while pending, so the far end always has a name.
    public let otherDisplayName: String
    /// The far endpoint's kind ("character", "location", …). Empty for a scene.
    ///
    /// ⚠️ **Load-bearing for pending edges (I-0124).** A resolved object can be
    /// attributed to a card via the object index, but a PENDING object is absent from
    /// that index by definition — its world is away. Without the kind travelling on
    /// the edge, every pending object appeared on every world-scoped card.
    /// Optional so an older core's payload still decodes.
    public let otherKind:        String?

    /// ⚠️ The far endpoint's world is unavailable: this edge is **pending, not
    /// dangling**. Show it, name it, and refuse to modify it — never hide it, which
    /// would read as the writer's work having vanished (Doc 2 §7.2).
    public let otherPending: Bool
    /// Present only when `otherPending`.
    public let otherWorldID: String?
    public let otherWorldStatus: String?

    public var id: String { edgeID }

    /// Typed form of `otherWorldStatus`, falling back to `.unavailable` rather than
    /// guessing at a more specific status.
    public var pendingStatus: WorldStatus? {
        guard otherPending else { return nil }
        guard let otherWorldStatus else { return .unavailable }
        return WorldStatus(rawValue: otherWorldStatus) ?? .unavailable
    }
}

public struct ListEdgesResult: Decodable, Sendable {
    public let edges: [EdgeView]

    private enum CodingKeys: String, CodingKey { case edges }

    public init(from decoder: Decoder) throws {
        let c = try decoder.container(keyedBy: CodingKeys.self)
        // The C ABI omits the key entirely when there are no edges — a scene with
        // no relationships is the common case, not an error.
        edges = try c.decodeIfPresent([EdgeView].self, forKey: .edges) ?? []
    }
}

/// A pending edge as reported project-wide. Carries a `displayName` from the
/// binding's cache (AC-A7) so a writer is never asked to decide about a bare ID.
public struct PendingEdgeView: Decodable, Sendable, Identifiable {
    public let edgeID:            String
    public let from:              String
    public let to:                String
    public let relationType:      String
    public let note:              String
    public let pendingEndpointID: String
    public let displayName:       String
    public let worldID:           String
    public let worldStatus:       String

    public var id: String { edgeID }

    public var status: WorldStatus {
        WorldStatus(rawValue: worldStatus) ?? .unavailable
    }
}

public struct ListPendingEdgesResult: Decodable, Sendable {
    public let pending: [PendingEdgeView]

    private enum CodingKeys: String, CodingKey { case pending }

    public init(from decoder: Decoder) throws {
        let c = try decoder.container(keyedBy: CodingKeys.self)
        pending = try c.decodeIfPresent([PendingEdgeView].self, forKey: .pending) ?? []
    }
}

/// An object index entry. `worldID` is `""` for project-scoped kinds.
public struct ObjectEntry: Decodable, Sendable, Identifiable {
    public let objectID:    String
    public let kind:        String
    public let slug:        String
    public let displayName: String
    public let worldID:     String

    public var id: String { objectID }

    /// True for the world-scoped kinds — `artifact`, `chronicle`, `faction`, `rule`.
    public var isWorldScoped: Bool { !worldID.isEmpty }
}

public struct ListObjectsResult: Decodable, Sendable {
    public let objects: [ObjectEntry]

    private enum CodingKeys: String, CodingKey { case objects }

    public init(from decoder: Decoder) throws {
        let c = try decoder.container(keyedBy: CodingKeys.self)
        objects = try c.decodeIfPresent([ObjectEntry].self, forKey: .objects) ?? []
    }
}

public struct PromoteObjectResult: Decodable, Sendable {
    /// The **same** objectID that went in — promotion preserves identity, which is
    /// what lets every existing edge keep resolving untouched.
    public let objectID: String
    public let kind:     String
    public let worldID:  String
    public let fromPath: String
    public let path:     String
}

/// A world package's own record, as returned by create/add.
public struct WorldRecordResult: Decodable, Sendable {
    public let worldID:     String
    public let displayName: String
    public let epochLabel:  String
    public let createdAt:   String
    public let modifiedAt:  String
}

public struct WorldEntry: Decodable, Sendable, Identifiable {
    public let worldID:       String
    public let displayName:   String
    public let status:        String
    /// A **verified** path. Non-empty only when `status == "available"`.
    public let packagePath:   String
    /// ⚠️ Where resolution **looked**, whatever the outcome (T-0419, I-0137).
    /// May not exist, may be unreadable, may hold a different world.
    /// **Never treat it as proof of anything** — it exists so volume refinement
    /// can run in exactly the case `packagePath` is empty by design.
    /// Defaulted for tolerance of an older core that does not emit it.
    public let lastKnownPackagePath: String
    public let epochOffsetMs: Int64
    /// ⚠️ WHY the world is unavailable, when ScriviCore knows (T-0440, SP-117).
    /// Empty means no further detail — the ordinary case. Read it through
    /// `unavailabilityExplanation`, never by comparing strings at a call site.
    public let statusReason: String

    private enum CodingKeys: String, CodingKey {
        case worldID, displayName, status, packagePath, lastKnownPackagePath,
             epochOffsetMs, statusReason
    }

    public init(from decoder: Decoder) throws {
        let c = try decoder.container(keyedBy: CodingKeys.self)
        worldID       = try c.decodeIfPresent(String.self, forKey: .worldID) ?? ""
        displayName   = try c.decodeIfPresent(String.self, forKey: .displayName) ?? ""
        status        = try c.decodeIfPresent(String.self, forKey: .status) ?? ""
        packagePath   = try c.decodeIfPresent(String.self, forKey: .packagePath) ?? ""
        lastKnownPackagePath =
            try c.decodeIfPresent(String.self, forKey: .lastKnownPackagePath) ?? ""
        epochOffsetMs = try c.decodeIfPresent(Int64.self, forKey: .epochOffsetMs) ?? 0
        statusReason  = try c.decodeIfPresent(String.self, forKey: .statusReason) ?? ""
    }

    public var id: String { worldID }

    /// The world's status, **refined by the platform layer** (AC24).
    ///
    /// ScriviCore reports only `missing` or `unavailable` — `offline` and `unmounted`
    /// need volume inspection that Doc 3 §4.4.1 keeps out of the shared model. This
    /// accessor is the single place that refinement is applied, so every consumer
    /// (`WorldsView`, the object cards, the warning view, `AppEnvironment`) reports
    /// the same status without any of them restating the rule.
    ///
    /// ⚠️ **Refine here, never at the call sites.** Five sites read this; a per-site
    /// copy is the restated-list defect in another costume, and this Epic has paid
    /// for that seven times.
    ///
    /// The unknown-status fallback stays `.unavailable` rather than a guess: a status
    /// string from a newer core must degrade honestly.
    public var worldStatus: WorldStatus {
        let reported = WorldStatus(rawValue: status) ?? .unavailable
        // ⚠️ T-0419 (I-0137): refine from the LAST-KNOWN path, not the verified one.
        // `packagePath` is empty by design for every world this refinement exists
        // to diagnose, so passing it guaranteed the refinement could never fire.
        // Fall back to `packagePath` so an older core still behaves as before.
        let probe = lastKnownPackagePath.isEmpty ? packagePath : lastKnownPackagePath
        return WorldVolumeStatus.refine(coreStatus: reported, packagePath: probe)
    }

    /// A sentence explaining WHY this world cannot be used, or nil when there is
    /// nothing more to say than its status (T-0440, SP-117).
    ///
    /// ⚠️ **This closes a debt owed since SP-115.** T-0420 fixed
    /// `unsupportedWorldFormatVersion` in the core and was Verified there — while
    /// a writer opening a too-new world still saw a bare "unavailable" with **no
    /// explanation**, across two sprints. The core knew and could not say so; now
    /// it says so and this turns it into words.
    ///
    /// ⚠️ **Explained here, once.** A per-call-site string comparison would be the
    /// restated-list defect in another costume — the class this Epic has paid for
    /// eight times.
    ///
    /// An unrecognised reason from a newer core degrades to nil rather than
    /// showing the writer a raw code.
    public var unavailabilityExplanation: String? {
        switch statusReason {
        case "unsupportedWorldFormatVersion":
            return "This world was saved by a newer version of Scrivi. "
                 + "Update Scrivi to open it. Your world has not been changed."
        default:
            return nil
        }
    }
}

public struct ListWorldsResult: Decodable, Sendable {
    public let worlds: [WorldEntry]

    private enum CodingKeys: String, CodingKey { case worlds }

    public init(from decoder: Decoder) throws {
        let c = try decoder.container(keyedBy: CodingKeys.self)
        worlds = try c.decodeIfPresent([WorldEntry].self, forKey: .worlds) ?? []
    }
}

public struct WorldStatusResult: Decodable, Sendable {
    public let worldID:     String
    public let status:      String
    /// A **verified** path — non-empty only when the world is available.
    public let packagePath: String
    /// ⚠️ Where resolution looked, whatever the outcome (T-0419, I-0137).
    public let lastKnownPackagePath: String

    private enum CodingKeys: String, CodingKey {
        case worldID, status, packagePath, lastKnownPackagePath
    }

    public init(from decoder: Decoder) throws {
        let c = try decoder.container(keyedBy: CodingKeys.self)
        worldID     = try c.decodeIfPresent(String.self, forKey: .worldID) ?? ""
        status      = try c.decodeIfPresent(String.self, forKey: .status) ?? ""
        packagePath = try c.decodeIfPresent(String.self, forKey: .packagePath) ?? ""
        lastKnownPackagePath =
            try c.decodeIfPresent(String.self, forKey: .lastKnownPackagePath) ?? ""
    }

    /// ⚠️ Refined, exactly as `WorldEntry.worldStatus` is. This accessor
    /// previously returned the RAW core status while its sibling refined —
    /// two paths to "what status is this world in", disagreeing. Found while
    /// fixing I-0137 (T-0419).
    public var worldStatus: WorldStatus {
        let reported = WorldStatus(rawValue: status) ?? .unavailable
        let probe = lastKnownPackagePath.isEmpty ? packagePath : lastKnownPackagePath
        return WorldVolumeStatus.refine(coreStatus: reported, packagePath: probe)
    }
}

/// One cached object name from a world binding — what a pending card displays
/// while the world itself is away.
public struct CachedObjectEntry: Decodable, Sendable, Identifiable {
    public let objectID:    String
    public let kind:        String
    public let displayName: String

    public var id: String { objectID }
}

public struct WorldBindingResult: Decodable, Sendable {
    public let worldID:               String
    public let displayName:           String
    public let epochOffsetMs:         Int64
    public let lastKnownPath:         String
    public let lastKnownAbsolutePath: String
    public let cachedIndex:           [CachedObjectEntry]

    private enum CodingKeys: String, CodingKey {
        case worldID, displayName, epochOffsetMs
        case lastKnownPath, lastKnownAbsolutePath, cachedIndex
    }

    public init(from decoder: Decoder) throws {
        let c = try decoder.container(keyedBy: CodingKeys.self)
        worldID               = try c.decode(String.self, forKey: .worldID)
        displayName           = try c.decode(String.self, forKey: .displayName)
        epochOffsetMs         = try c.decode(Int64.self,  forKey: .epochOffsetMs)
        lastKnownPath         = try c.decode(String.self, forKey: .lastKnownPath)
        lastKnownAbsolutePath = try c.decode(String.self, forKey: .lastKnownAbsolutePath)
        cachedIndex = try c.decodeIfPresent([CachedObjectEntry].self,
                                            forKey: .cachedIndex) ?? []
    }
}

public struct RelinkWorldResult: Decodable, Sendable {
    public let worldID:  String
    public let relinked: Bool
}

public struct RemoveWorldReferenceResult: Decodable, Sendable {
    public let worldID: String
    public let removed: Bool
}

/// A relation type. `sourceKind`/`targetKind` are **absent when unconstrained** —
/// the C ABI omits the key rather than emitting null (`RelationTypes.cpp`), so nil
/// means "any kind", never "unknown".
public struct RelationTypeEntry: Decodable, Sendable, Identifiable {
    public let code:               String
    public let forwardLabel:       String
    public let inverseLabel:       String
    public let sourceKind:         String?
    public let targetKind:         String?
    public let canonicalDirection: String
    public let symmetric:          Bool

    public var id: String { code }

    /// True when this type may relate the given kind as its source. An
    /// unconstrained type accepts anything.
    public func acceptsSource(kind: String) -> Bool {
        sourceKind == nil || sourceKind == kind
    }

    public func acceptsTarget(kind: String) -> Bool {
        targetKind == nil || targetKind == kind
    }

    /// ⚠️ **The scene sentinel.** ScriviCore cannot express "scene" as an
    /// `ObjectKind` — scenes are not objects (Doc 1 §8) — so a constrained scene
    /// endpoint crosses the ABI as the literal string `"scene"`
    /// (`RelationTypes.cpp`, `kSceneToken`). It round-trips unchanged, so no ABI
    /// change was needed to tell the two endpoint shapes apart (T-0443).
    ///
    /// ⚠️ Spelled out **once, here**, rather than compared inline at each call
    /// site — the second such comparison is where a partition starts drifting.
    public static let sceneToken = "scene"

    /// True when this type may point at a **scene**. Includes the unconstrained
    /// case: a type with no target constraint accepts anything, scenes included.
    public var targetAcceptsScene: Bool {
        targetKind == nil || targetKind == Self.sceneToken
    }

    /// True when this type may point at an **object** rather than a scene — what
    /// object→object relating needs (T-0443, D4-A).
    ///
    /// ⚠️ Not the negation of `targetAcceptsScene`: an unconstrained type accepts
    /// **both**, so the two are deliberately overlapping, not complementary.
    public var targetAcceptsObject: Bool {
        targetKind != Self.sceneToken
    }

    /// True when this type's SOURCE end is constrained to a scene — such a type
    /// cannot be created *from* an object (`located-at` is the seeded example).
    public var sourceIsScene: Bool {
        sourceKind == Self.sceneToken
    }
}

public struct ListRelationTypesResult: Decodable, Sendable {
    public let types: [RelationTypeEntry]

    private enum CodingKeys: String, CodingKey { case types }

    public init(from decoder: Decoder) throws {
        let c = try decoder.container(keyedBy: CodingKeys.self)
        types = try c.decodeIfPresent([RelationTypeEntry].self, forKey: .types) ?? []
    }
}
