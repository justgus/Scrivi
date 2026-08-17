#pragma once

// RelationshipStore — the relationship graph (EP-031 SP-096, T-0374/T-0375/T-0376).
// Design: Worldbuilding Object Model v0.2 §5.2–§5.4.
//
//   objects/relationships.jsonl   append-only records: rec:"edge" | "tomb"
//
// ONE CANONICAL EDGE per relationship (§5.2). The inverse direction is a
// read-time label projection, never a second record — that is precisely the
// Cumberland failure this model exists to prevent. Creating the same
// relationship from either endpoint therefore normalizes to the same canonical
// edge and the second attempt is REJECTED as a duplicate (§5.3).
//
// Storage reuses the EP-019 append-log pattern (history/HistoryStore.cpp):
// rec/seq records via FileSystem::appendTextFile, torn-final-line detection at
// load. Adding an edge appends ONE line regardless of graph size; the full
// in-memory map is retained because memory is not the constraint (§5.4) and it
// makes duplicate detection and SP-097's orphan queries O(1).

#include "objects/RelationTypes.hpp"
#include "scrivi/Result.hpp"
#include "scrivi/Services.hpp"

#include <cstdint>
#include <string>
#include <vector>

namespace scrivi::objects {

struct Edge {
    std::string edgeID;
    std::string fromID;
    std::string toID;
    std::string relationType;
    std::string note;
    double      sortIndex = 0.0;
};

// One edge as seen FROM a particular endpoint — carries the label that applies
// in that reading direction (§5.2's read-time projection).
struct EdgeView {
    Edge        edge;
    bool        isForward = true;   // true: `endpoint` is the edge's `from`
    std::string label;              // forwardLabel or inverseLabel accordingly
    std::string otherID;
    std::string otherDisplayName;
    // The far endpoint's KIND ("character", "location", …), empty for a scene or an
    // unresolvable endpoint.
    //
    // ⚠️ I-0124: the core has always known this — `ResolvedEndpoint::kind` backs the
    // relation-type constraint check — but it was never carried across the boundary,
    // so the Apple layer could not filter PENDING edges by kind and every pending
    // object appeared on every world-scoped card. Resolved endpoints are filterable
    // via the object index; pending ones are not in that index by definition, which
    // is exactly why the kind has to travel with the edge.
    std::string otherKind;

    // The far endpoint's world is unavailable (T-0380). `otherDisplayName` is
    // then the binding's cached name (AC-A7), never a bare ID — a writer being
    // asked whether to clear references cannot decide blind.
    bool        otherPending = false;
    std::string otherWorldID;
    std::string otherWorldStatus;   // "offline" | "unmounted" | "missing" | "unavailable"
};

// One edge held pending because an endpoint's world is unavailable (Doc 3 §4.6,
// §8's scrivi_list_pending_edges). Reported, never repaired.
struct PendingEdge {
    Edge        edge;
    std::string pendingEndpointID;  // the endpoint whose world is away
    std::string displayName;        // from binding.cachedIndex (AC-A7)
    std::string worldID;
    std::string worldStatus;
};

// Compaction thresholds (§5.4, ruled 2026-08-05). BOTH bounds are needed: the
// ratio alone makes a small graph compact constantly; the absolute alone lets a
// 100k-edge graph accumulate tens of thousands of dead records first.
struct GraphSettings {
    double tombstoneRatio = 0.30;   // compact when tombstones exceed 30% of records
    int    tombstoneCount = 1000;   // ...or 1,000 tombstones, whichever comes first
};

class RelationshipStore {
public:
    explicit RelationshipStore(CoreServices& services);

    [[nodiscard]] static AbsolutePath logPath(const AbsolutePath& projectRoot);

    // Replays the log into `edges`, truncating at a torn final line. Compacts
    // first when either threshold is exceeded. A missing log is not an error —
    // it is simply an empty graph.
    [[nodiscard]] Result<std::vector<Edge>> load(const AbsolutePath& projectRoot) const;

    // Creates ONE canonical edge. Normalizes direction per the relation type
    // (§5.3) before both the duplicate check and the write, so creating from
    // either endpoint yields the same record.
    //
    // Rejects, each with a distinct message: unknown relation type; an endpoint
    // that does not resolve; a self-edge; a kind-constraint violation; an
    // existing equivalent edge (duplicate); and — since T-0380 — an endpoint
    // whose world is unavailable (detail "worldPending"), because the graph is
    // FROZEN toward such a world in both directions (Doc 3 §4.6 rule 1).
    [[nodiscard]] Result<Edge> create(const AbsolutePath& projectRoot,
                                      const std::string& fromID,
                                      const std::string& toID,
                                      const std::string& relationTypeCode,
                                      const std::string& note) const;

    // Appends a tombstone. Deleting an unknown edgeID is an error, not a no-op.
    //
    // Refuses (detail "worldPending") when EITHER endpoint's world is
    // unavailable: removal is exactly as frozen as addition. A writer who could
    // delete a pending edge while its world is away would be making an
    // irreversible decision on evidence they cannot see.
    [[nodiscard]] Result<void> remove(const AbsolutePath& projectRoot,
                                      const std::string& edgeID) const;

    // Every edge with an endpoint whose world is unavailable (Doc 3 §4.6, §8).
    // A pure REPORT — it repairs nothing and modifies nothing, which is the
    // point: these edges are held, and only the writer decides what becomes of
    // them.
    [[nodiscard]] Result<std::vector<PendingEdge>> listPending(
        const AbsolutePath& projectRoot) const;

    // Appends tombstones for every live edge touching `endpointID` (T-0377
    // cascade-prune). Returns the tombstoned edgeIDs.
    //
    // ⚠️ Callers pass an endpoint they have JUST DELETED — this never consults
    // whether it resolves, because by then it does not. The pending guard still
    // applies to the FAR side: an edge whose other end lives in an unavailable
    // world is left alone and reported in `skippedPending`, because pruning it
    // would destroy a relationship whose far endpoint may be perfectly intact.
    struct CascadeResult {
        std::vector<std::string> tombstoned;
        std::vector<std::string> skippedPending;
    };
    [[nodiscard]] Result<CascadeResult> cascadeDelete(const AbsolutePath& projectRoot,
                                                       const std::string& endpointID) const;

    // Load-time repair (T-0377, §5.5): drops edges whose endpoints no longer
    // resolve. Belt-and-suspenders behind cascade-prune — an edge can still be
    // orphaned by a crash between the file delete and the tombstone, or by a
    // hand-edited project.
    //
    // ⚠️ IT CONSULTS pending() FIRST AND ALWAYS. An endpoint whose world is
    // unavailable is NOT dangling; pruning it is the silent, unrecoverable
    // failure this whole sprint is ordered around (Doc 1 §5.5's critical
    // exception, Doc 3 §4.6). Repairing on open is safe only because that
    // distinction exists.
    struct RepairResult {
        std::vector<std::string> prunedEdgeIDs;
        std::vector<std::string> heldPendingEdgeIDs;
    };
    [[nodiscard]] Result<RepairResult> repairDangling(const AbsolutePath& projectRoot) const;

    // Every edge touching `endpointID` in EITHER direction, each with the label
    // that reads correctly from that endpoint. Does NOT refuse an unresolved
    // endpoint — it reports what it has, because SP-097 must be able to surface
    // edges whose far side is temporarily unreachable.
    [[nodiscard]] Result<std::vector<EdgeView>> listFor(const AbsolutePath& projectRoot,
                                                        const std::string& endpointID) const;

    // Rewrites the log with live edges only, renumbering seq from 1. Temp-write
    // → atomic replace, so an interrupted compaction leaves the previous log
    // readable.
    [[nodiscard]] Result<void> compact(const AbsolutePath& projectRoot,
                                       const std::vector<Edge>& liveEdges) const;

private:
    CoreServices& services_;

    struct LoadResult {
        std::vector<Edge> edges;
        int               tombstones  = 0;
        int               totalRecords = 0;
        std::int64_t      maxSeq      = 0;
    };

    [[nodiscard]] Result<LoadResult> replay(const AbsolutePath& projectRoot) const;

    // Normalizes (from,to) to canonical order for `type` (§5.3). Returns true if
    // the arguments were swapped.
    static bool canonicalize(const RelationType& type,
                             std::string& fromID, std::string& toID);
};

} // namespace scrivi::objects
