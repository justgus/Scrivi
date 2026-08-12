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
    // that does not resolve; a self-edge; a kind-constraint violation; and an
    // existing equivalent edge (duplicate).
    [[nodiscard]] Result<Edge> create(const AbsolutePath& projectRoot,
                                      const std::string& fromID,
                                      const std::string& toID,
                                      const std::string& relationTypeCode,
                                      const std::string& note) const;

    // Appends a tombstone. Deleting an unknown edgeID is an error, not a no-op.
    [[nodiscard]] Result<void> remove(const AbsolutePath& projectRoot,
                                      const std::string& edgeID) const;

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
