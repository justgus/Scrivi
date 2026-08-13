#include "objects/RelationshipStore.hpp"

#include "objects/EndpointResolver.hpp"
#include "util/Json.hpp"
#include "util/PathUtils.hpp"
#include "worlds/WorldTypes.hpp"

#include <algorithm>
#include <set>
#include <sstream>

namespace scrivi::objects {

namespace {

constexpr std::string_view kRecEdge = "edge";
constexpr std::string_view kRecTomb = "tomb";

std::string serializeEdgeRecord(const Edge& e, std::int64_t seq) {
    util::JsonDoc d;
    d.setString("rec",          std::string(kRecEdge));
    d.setInt64 ("seq",          seq);
    d.setString("edgeID",       e.edgeID);
    d.setString("from",         e.fromID);
    d.setString("to",           e.toID);
    d.setString("relationType", e.relationType);
    d.setString("note",         e.note);
    d.setDouble("sortIndex",    e.sortIndex);
    return d.dump(-1);   // one record per line — never pretty-printed
}

std::string serializeTombRecord(const std::string& edgeID, std::int64_t seq) {
    util::JsonDoc d;
    d.setString("rec",    std::string(kRecTomb));
    d.setInt64 ("seq",    seq);
    d.setString("edgeID", edgeID);
    return d.dump(-1);
}

} // namespace

RelationshipStore::RelationshipStore(CoreServices& services)
    : services_(services) {}

AbsolutePath RelationshipStore::logPath(const AbsolutePath& projectRoot) {
    return util::join(util::join(projectRoot, "objects"), "relationships.jsonl");
}

bool RelationshipStore::canonicalize(const RelationType& type,
                                      std::string& fromID, std::string& toID) {
    if (type.canonicalDirection == CanonicalDirection::lexical) {
        // Symmetric (§5.3): neither end is naturally the `from` side, so sort
        // the IDs. Without this, "Vance at-war-with Ordo" and "Ordo at-war-with
        // Vance" would normalize to two different canonical edges — the silent
        // regression §9 AC4 exists to catch.
        if (fromID > toID) { std::swap(fromID, toID); return true; }
        return false;
    }
    // Asymmetric: the declared source→target order IS canonical, so the
    // arguments are already in canonical form as given. Direction-flipped
    // creates are caught by the duplicate check, which compares unordered pairs.
    return false;
}

Result<RelationshipStore::LoadResult>
RelationshipStore::replay(const AbsolutePath& projectRoot) const {
    auto& fs = *services_.fileSystem;
    LoadResult out;

    auto p       = logPath(projectRoot);
    auto existsR = fs.exists(p);
    if (!existsR.ok() || !existsR.value()) {
        return Result<LoadResult>::success(std::move(out));   // no log = empty graph
    }

    auto textR = fs.readTextFile(p);
    if (!textR.ok()) { return Result<LoadResult>::failure(textR.error()); }

    // Replay in file order. Order comes from file position, not from seq — seq
    // is forensic metadata, deliberately not load-bearing (see the note in
    // history/HistoryStore.cpp:194, where a seq-assignment bug shipped and
    // replay survived only because of this property).
    std::vector<Edge>           ordered;
    std::set<std::string>       tombstoned;

    std::istringstream ss(textR.value());
    std::string        line;
    while (std::getline(ss, line)) {
        if (line.empty()) { continue; }

        auto parsedR = util::parseJson(line);
        if (!parsedR.ok()) {
            // Torn final line from a partial write — stop. Everything before it
            // is intact, which is the whole point of an append-only log.
            break;
        }
        const auto& d = parsedR.value();

        const std::string rec = d.getString("rec");
        const std::int64_t seq = d.getInt64("seq");
        if (seq > out.maxSeq) { out.maxSeq = seq; }
        ++out.totalRecords;

        if (rec == kRecEdge) {
            Edge e;
            e.edgeID       = d.getString("edgeID");
            if (e.edgeID.empty()) { continue; }
            e.fromID       = d.getString("from");
            e.toID         = d.getString("to");
            e.relationType = d.getString("relationType");
            e.note         = d.getString("note");
            e.sortIndex    = d.getDouble("sortIndex");
            ordered.push_back(std::move(e));
        } else if (rec == kRecTomb) {
            const std::string id = d.getString("edgeID");
            if (!id.empty()) { tombstoned.insert(id); ++out.tombstones; }
        }
    }

    for (auto& e : ordered) {
        if (!tombstoned.contains(e.edgeID)) { out.edges.push_back(std::move(e)); }
    }

    return Result<LoadResult>::success(std::move(out));
}

Result<std::vector<Edge>> RelationshipStore::load(const AbsolutePath& projectRoot) const {
    auto replayed = replay(projectRoot);
    if (!replayed.ok()) { return Result<std::vector<Edge>>::failure(replayed.error()); }

    auto& r = replayed.value();

    // Compaction on open (§5.4): either threshold fires the same pass.
    const GraphSettings settings;
    const bool ratioExceeded = r.totalRecords > 0 &&
        (static_cast<double>(r.tombstones) / static_cast<double>(r.totalRecords))
            > settings.tombstoneRatio;
    const bool countExceeded = r.tombstones > settings.tombstoneCount;

    if (ratioExceeded || countExceeded) {
        if (auto c = compact(projectRoot, r.edges); !c.ok()) {
            // A failed compaction must not fail the open — the log is still
            // readable and correct, merely larger than we would like.
        }
    }

    return Result<std::vector<Edge>>::success(std::move(r.edges));
}

Result<Edge> RelationshipStore::create(const AbsolutePath& projectRoot,
                                        const std::string& fromID,
                                        const std::string& toID,
                                        const std::string& relationTypeCode,
                                        const std::string& note) const {
    auto& fs   = *services_.fileSystem;
    auto& uuid = *services_.uuidProvider;

    // 1. The relation type must exist — it supplies the canonical direction.
    RelationTypeStore typeStore{services_};
    auto typeR = typeStore.find(projectRoot, relationTypeCode);
    if (!typeR.ok()) { return Result<Edge>::failure(typeR.error()); }
    const auto& type = typeR.value();

    if (fromID.empty() || toID.empty()) {
        return Result<Edge>::failure(
            {.code = ErrorCode::invalidArgument, .message = "edge endpoints must not be empty"});
    }
    if (fromID == toID) {
        return Result<Edge>::failure(
            {.code = ErrorCode::invalidArgument,
             .message = "an object cannot be related to itself"});
    }

    // 2. Both endpoints must resolve. Kind comes from the object index, NEVER
    //    from an ID prefix (T-0402) — every object ID begins "character_".
    //
    //    ⚠️ A pending endpoint is refused SEPARATELY from a missing one, and
    //    explicitly (Doc 3 §4.6 rule 1, AC-A4): the graph is frozen toward an
    //    unavailable world in both directions. Reporting "not found" here would
    //    tell the writer their object is gone when it is merely unreachable —
    //    the same conflation that makes cascade-prune dangerous.
    EndpointResolver resolver{services_};

    auto refusePending = [](const ResolvedEndpoint& ep) {
        return Error{
            .code    = ErrorCode::invalidArgument,
            .message = "cannot add an edge to '" + ep.displayName + "': its world is " +
                       worlds::worldStatusName(ep.worldStatus) +
                       " — the graph is frozen toward that world until it returns",
            .path    = {},
            .detail  = "worldPending:" + worlds::worldStatusName(ep.worldStatus)};
    };

    auto fromEP = resolver.resolve(projectRoot, fromID);
    if (fromEP.pending()) { return Result<Edge>::failure(refusePending(fromEP)); }
    if (!fromEP.found) {
        return Result<Edge>::failure(
            {.code = ErrorCode::invalidArgument, .message = "endpoint not found: " + fromID});
    }
    auto toEP = resolver.resolve(projectRoot, toID);
    if (toEP.pending()) { return Result<Edge>::failure(refusePending(toEP)); }
    if (!toEP.found) {
        return Result<Edge>::failure(
            {.code = ErrorCode::invalidArgument, .message = "endpoint not found: " + toID});
    }

    // 3. Kind constraints, when declared. A null constraint means ANY kind —
    //    `cites` is unconstrained on both ends by design (§3.4).
    auto endpointSatisfies = [](const ResolvedEndpoint& ep,
                                const std::optional<ObjectKind>& kind,
                                bool wantScene) {
        if (wantScene) { return ep.isScene; }
        if (!kind)     { return true; }          // any kind
        return !ep.isScene && ep.kind == *kind;
    };

    const bool forwardOK =
        endpointSatisfies(fromEP, type.sourceKind, type.sourceIsScene) &&
        endpointSatisfies(toEP,   type.targetKind, type.targetIsScene);

    // A writer may create the relationship from EITHER end (§5.3), so the
    // reversed assignment is equally valid — normalize it below.
    const bool reversedOK =
        endpointSatisfies(toEP,   type.sourceKind, type.sourceIsScene) &&
        endpointSatisfies(fromEP, type.targetKind, type.targetIsScene);

    if (!forwardOK && !reversedOK) {
        return Result<Edge>::failure(
            {.code = ErrorCode::invalidArgument,
             .message = "endpoints do not satisfy the kind constraints of relation type '"
                        + relationTypeCode + "'"});
    }

    // 4. Normalize to canonical form BEFORE the duplicate check and the write.
    std::string cFrom = fromID;
    std::string cTo   = toID;
    if (!forwardOK && reversedOK) {
        // Created from the inverse end — flip to the declared direction so the
        // stored record is canonical.
        std::swap(cFrom, cTo);
    }
    canonicalize(type, cFrom, cTo);

    // 5. Duplicate rejection (§5.3). An edge is identified by the UNORDERED pair
    //    plus the relation type, so a direction-flipped create of an existing
    //    relationship is caught here rather than written as a second record.
    auto replayed = replay(projectRoot);
    if (!replayed.ok()) { return Result<Edge>::failure(replayed.error()); }

    for (const auto& e : replayed.value().edges) {
        if (e.relationType != relationTypeCode) { continue; }
        const bool samePair =
            (e.fromID == cFrom && e.toID == cTo) ||
            (e.fromID == cTo   && e.toID == cFrom);
        if (samePair) {
            // Distinct from every other rejection so a caller can tell "already
            // related" from "bad request" — §5.3 requires an error, not a silent
            // no-op. `detail` carries the machine-readable marker; ErrorCode has
            // no `conflict` member and adding one to the public enum for a
            // single case is not worth the ABI churn.
            return Result<Edge>::failure(
                {.code    = ErrorCode::invalidArgument,
                 .message = "this relationship already exists (edge " + e.edgeID + ")",
                 .path    = {},
                 .detail  = "duplicateEdge"});
        }
    }

    // 6. Append exactly one line. seq is assigned HERE and only here, from the
    //    replayed maximum — never captured earlier (HistoryStore.cpp:194).
    Edge edge;
    edge.edgeID       = "edge_" + uuid.newObjectID().value;
    edge.fromID       = cFrom;
    edge.toID         = cTo;
    edge.relationType = relationTypeCode;
    edge.note         = note;
    edge.sortIndex    = 0.0;

    auto objectsDir = util::join(projectRoot, "objects");
    if (auto r = fs.createDirectories(objectsDir); !r.ok()) {
        return Result<Edge>::failure(r.error());
    }

    const std::int64_t seq = replayed.value().maxSeq + 1;
    if (auto r = fs.appendTextFile(logPath(projectRoot),
                                   serializeEdgeRecord(edge, seq) + "\n"); !r.ok()) {
        return Result<Edge>::failure(r.error());
    }

    return Result<Edge>::success(std::move(edge));
}

Result<void> RelationshipStore::remove(const AbsolutePath& projectRoot,
                                        const std::string& edgeID) const {
    auto& fs = *services_.fileSystem;

    auto replayed = replay(projectRoot);
    if (!replayed.ok()) { return Result<void>::failure(replayed.error()); }

    const auto& edges = replayed.value().edges;
    const auto  it    = std::find_if(edges.begin(), edges.end(),
                                     [&](const Edge& e) { return e.edgeID == edgeID; });
    if (it == edges.end()) {
        return Result<void>::failure(
            {.code = ErrorCode::invalidArgument, .message = "unknown edge: " + edgeID});
    }

    // ⚠️ Removal is as frozen as addition (Doc 3 §4.6 rule 1, AC-A4). Deleting
    // an edge whose far end is in an unavailable world is a decision the writer
    // cannot make on evidence they can see — and it is not reversible once the
    // tombstone lands.
    EndpointResolver resolver{services_};
    for (const auto& endpointID : {it->fromID, it->toID}) {
        auto ep = resolver.resolve(projectRoot, endpointID);
        if (!ep.pending()) { continue; }
        return Result<void>::failure(
            {.code    = ErrorCode::invalidArgument,
             .message = "cannot remove an edge to '" + ep.displayName + "': its world is " +
                        worlds::worldStatusName(ep.worldStatus) +
                        " — the graph is frozen toward that world until it returns",
             .path    = {},
             .detail  = "worldPending:" + worlds::worldStatusName(ep.worldStatus)});
    }

    const std::int64_t seq = replayed.value().maxSeq + 1;
    return fs.appendTextFile(logPath(projectRoot),
                             serializeTombRecord(edgeID, seq) + "\n");
}

Result<std::vector<PendingEdge>>
RelationshipStore::listPending(const AbsolutePath& projectRoot) const {
    auto loadedR = load(projectRoot);
    if (!loadedR.ok()) { return Result<std::vector<PendingEdge>>::failure(loadedR.error()); }

    EndpointResolver         resolver{services_};
    std::vector<PendingEdge> out;

    for (const auto& e : loadedR.value()) {
        for (const auto& endpointID : {e.fromID, e.toID}) {
            auto ep = resolver.resolve(projectRoot, endpointID);
            if (!ep.pending()) { continue; }

            PendingEdge p;
            p.edge              = e;
            p.pendingEndpointID = endpointID;
            p.displayName       = ep.displayName;     // cached name, not a bare ID (AC-A7)
            p.worldID           = ep.worldID;
            p.worldStatus       = worlds::worldStatusName(ep.worldStatus);
            out.push_back(std::move(p));
        }
    }

    return Result<std::vector<PendingEdge>>::success(std::move(out));
}

Result<RelationshipStore::CascadeResult>
RelationshipStore::cascadeDelete(const AbsolutePath& projectRoot,
                                  const std::string& endpointID) const {
    auto& fs = *services_.fileSystem;

    if (endpointID.empty()) {
        return Result<CascadeResult>::failure(
            {.code = ErrorCode::invalidArgument, .message = "endpointID must not be empty"});
    }

    auto replayed = replay(projectRoot);
    if (!replayed.ok()) { return Result<CascadeResult>::failure(replayed.error()); }

    EndpointResolver resolver{services_};
    CascadeResult    out;
    std::string      body;
    std::int64_t     seq = replayed.value().maxSeq;

    for (const auto& e : replayed.value().edges) {
        const bool touches = (e.fromID == endpointID || e.toID == endpointID);
        if (!touches) { continue; }

        // ⚠️ THE GUARD THIS WHOLE SPRINT IS ORDERED AROUND (Doc 3 §4.6 rule 1).
        // The deleted endpoint is gone by definition — but the FAR one may be
        // sitting in a world that is merely unmounted. Pruning that edge would
        // silently destroy a relationship whose other end is perfectly intact,
        // and nothing would error. Hold it instead; if the world never returns,
        // load-time repair will still find it dangling later, when that is
        // actually knowable.
        const std::string& farID = (e.fromID == endpointID) ? e.toID : e.fromID;
        if (resolver.resolve(projectRoot, farID).pending()) {
            out.skippedPending.push_back(e.edgeID);
            continue;
        }

        body += serializeTombRecord(e.edgeID, ++seq);
        body += "\n";
        out.tombstoned.push_back(e.edgeID);
    }

    // One append for the whole cascade — the delete and its tombstones land
    // together or not at all, rather than leaving a half-pruned graph behind.
    if (!body.empty()) {
        if (auto r = fs.appendTextFile(logPath(projectRoot), body); !r.ok()) {
            return Result<CascadeResult>::failure(r.error());
        }
    }

    return Result<CascadeResult>::success(std::move(out));
}

Result<RelationshipStore::RepairResult>
RelationshipStore::repairDangling(const AbsolutePath& projectRoot) const {
    auto& fs = *services_.fileSystem;

    auto replayed = replay(projectRoot);
    if (!replayed.ok()) { return Result<RepairResult>::failure(replayed.error()); }

    EndpointResolver resolver{services_};
    RepairResult     out;
    std::string      body;
    std::int64_t     seq = replayed.value().maxSeq;

    for (const auto& e : replayed.value().edges) {
        const auto fromEP = resolver.resolve(projectRoot, e.fromID);
        const auto toEP   = resolver.resolve(projectRoot, e.toID);

        // ⚠️ PENDING WINS OVER DANGLING, unconditionally. If EITHER endpoint is
        // merely unreachable, the edge is held — even if the other end is
        // genuinely gone, because with the world away we cannot know whether the
        // relationship still means something. Absence is never deletion.
        if (fromEP.pending() || toEP.pending()) {
            out.heldPendingEdgeIDs.push_back(e.edgeID);
            continue;
        }

        if (fromEP.dangling() || toEP.dangling()) {
            body += serializeTombRecord(e.edgeID, ++seq);
            body += "\n";
            out.prunedEdgeIDs.push_back(e.edgeID);
        }
    }

    if (!body.empty()) {
        if (auto r = fs.appendTextFile(logPath(projectRoot), body); !r.ok()) {
            return Result<RepairResult>::failure(r.error());
        }
    }

    return Result<RepairResult>::success(std::move(out));
}

Result<std::vector<EdgeView>>
RelationshipStore::listFor(const AbsolutePath& projectRoot,
                            const std::string& endpointID) const {
    auto loadedR = load(projectRoot);
    if (!loadedR.ok()) { return Result<std::vector<EdgeView>>::failure(loadedR.error()); }

    RelationTypeStore typeStore{services_};
    auto typesR = typeStore.load(projectRoot);
    if (!typesR.ok()) { return Result<std::vector<EdgeView>>::failure(typesR.error()); }
    const auto& types = typesR.value();

    EndpointResolver resolver{services_};
    std::vector<EdgeView> out;

    for (const auto& e : loadedR.value()) {
        const bool isFrom = (e.fromID == endpointID);
        const bool isTo   = (e.toID   == endpointID);
        if (!isFrom && !isTo) { continue; }

        EdgeView v;
        v.edge      = e;
        v.isForward = isFrom;
        v.otherID   = isFrom ? e.toID : e.fromID;

        // §5.2's read-time projection: ONE stored edge, two renderings.
        for (const auto& t : types) {
            if (t.code == e.relationType) {
                v.label = isFrom ? t.forwardLabel : t.inverseLabel;
                break;
            }
        }

        // Best-effort: an unresolvable far endpoint still lists. A PENDING one
        // carries its cached name and world status (AC-A7) so the inspector can
        // render "⟨Midgard: Sword of Dawn — unavailable⟩" rather than a bare
        // UUID the writer cannot act on.
        if (auto ep = resolver.resolve(projectRoot, v.otherID); ep.found || ep.pending()) {
            v.otherDisplayName = ep.displayName;
            if (ep.pending()) {
                v.otherPending     = true;
                v.otherWorldID     = ep.worldID;
                v.otherWorldStatus = worlds::worldStatusName(ep.worldStatus);
            }
        }

        out.push_back(std::move(v));
    }

    return Result<std::vector<EdgeView>>::success(std::move(out));
}

Result<void> RelationshipStore::compact(const AbsolutePath& projectRoot,
                                         const std::vector<Edge>& liveEdges) const {
    auto& fs = *services_.fileSystem;

    std::string body;
    std::int64_t seq = 0;
    for (const auto& e : liveEdges) {
        body += serializeEdgeRecord(e, ++seq);
        body += "\n";
    }

    // atomicWriteTextFile is temp-write → rename, so an interrupted compaction
    // leaves the PREVIOUS log intact and readable rather than a partial file.
    return fs.atomicWriteTextFile(logPath(projectRoot), body);
}

} // namespace scrivi::objects
