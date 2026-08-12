#include "objects/RelationshipStore.hpp"

#include "objects/EndpointResolver.hpp"
#include "util/Json.hpp"
#include "util/PathUtils.hpp"

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
    EndpointResolver resolver{services_};
    auto fromEP = resolver.resolve(projectRoot, fromID);
    if (!fromEP.found) {
        return Result<Edge>::failure(
            {.code = ErrorCode::invalidArgument, .message = "endpoint not found: " + fromID});
    }
    auto toEP = resolver.resolve(projectRoot, toID);
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
    const bool  known = std::any_of(edges.begin(), edges.end(),
                                    [&](const Edge& e) { return e.edgeID == edgeID; });
    if (!known) {
        return Result<void>::failure(
            {.code = ErrorCode::invalidArgument, .message = "unknown edge: " + edgeID});
    }

    const std::int64_t seq = replayed.value().maxSeq + 1;
    return fs.appendTextFile(logPath(projectRoot),
                             serializeTombRecord(edgeID, seq) + "\n");
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

        // Best-effort: an unresolvable far endpoint still lists (SP-097 needs
        // this to surface edges into an unavailable world).
        if (auto ep = resolver.resolve(projectRoot, v.otherID); ep.found) {
            v.otherDisplayName = ep.displayName;
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
