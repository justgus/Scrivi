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
    public let packagePath:   String
    public let epochOffsetMs: Int64

    public var id: String { worldID }

    public var worldStatus: WorldStatus {
        WorldStatus(rawValue: status) ?? .unavailable
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
    public let packagePath: String

    public var worldStatus: WorldStatus {
        WorldStatus(rawValue: status) ?? .unavailable
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
}

public struct ListRelationTypesResult: Decodable, Sendable {
    public let types: [RelationTypeEntry]

    private enum CodingKeys: String, CodingKey { case types }

    public init(from decoder: Decoder) throws {
        let c = try decoder.container(keyedBy: CodingKeys.self)
        types = try c.decodeIfPresent([RelationTypeEntry].self, forKey: .types) ?? []
    }
}
