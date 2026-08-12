#include "worlds/WorldStore.hpp"

#include "objects/ObjectIndex.hpp"
#include "schemas/WorldJson.hpp"
#include "util/Json.hpp"
#include "util/PathUtils.hpp"

#include <algorithm>
#include <filesystem>

namespace scrivi::worlds {

namespace fs = std::filesystem;

namespace {

constexpr std::string_view kLockSchema = "scrivi.world-lock.v1";

// Seconds between two ISO-8601 timestamps. Returns a large number when either
// is unparseable, so an undated lock is treated as stale rather than eternal.
long long secondsBetween(const std::string& earlier, const std::string& later) {
    auto parse = [](const std::string& t) -> long long {
        // "YYYY-MM-DDTHH:MM:SSZ" — compared as a monotonic field sum. Exact
        // calendar arithmetic is unnecessary: this only needs to answer
        // "older than 60 seconds?", and the fields are zero-padded and ordered.
        if (t.size() < 19) { return -1; }
        auto num = [&](std::size_t p, std::size_t n) -> long long {
            try { return std::stoll(t.substr(p, n)); } catch (...) { return -1; }
        };
        const long long Y = num(0,4), M = num(5,2), D = num(8,2);
        const long long h = num(11,2), m = num(14,2), s = num(17,2);
        if (Y < 0 || M < 0 || D < 0 || h < 0 || m < 0 || s < 0) { return -1; }
        // Days-since-epoch approximation is fine for a 60 s window; the only
        // requirement is monotonicity within a run.
        const long long days = Y * 365 + M * 31 + D;
        return ((days * 24 + h) * 60 + m) * 60 + s;
    };

    const long long a = parse(earlier);
    const long long b = parse(later);
    if (a < 0 || b < 0) { return 1'000'000; }   // unparseable ⇒ treat as stale
    return b - a;
}

} // namespace

// ---------------------------------------------------------------------------
// Layout
// ---------------------------------------------------------------------------

WorldStore::WorldStore(CoreServices& services) : services_(services) {}

AbsolutePath WorldStore::worldsDir(const AbsolutePath& projectRoot) {
    return util::join(projectRoot, "worlds");
}

AbsolutePath WorldStore::bindingPath(const AbsolutePath& projectRoot,
                                      const std::string& worldID) {
    return util::join(util::join(worldsDir(projectRoot), worldID), "binding.json");
}

AbsolutePath WorldStore::worldJsonPath(const AbsolutePath& packagePath) {
    return util::join(packagePath, "world.json");
}

AbsolutePath WorldStore::worldIndexPath(const AbsolutePath& packagePath) {
    return util::join(packagePath, "index.json");
}

AbsolutePath WorldStore::lockPath(const AbsolutePath& packagePath) {
    return util::join(packagePath, ".lock");
}

// ---------------------------------------------------------------------------
// T-0381 — create / add
// ---------------------------------------------------------------------------

Result<WorldRecord> WorldStore::createWorld(const AbsolutePath& projectRoot,
                                              const AbsolutePath& packagePath,
                                              const std::string& displayName,
                                              const std::string& epochLabel) const {
    auto& fs_  = *services_.fileSystem;
    auto& uuid = *services_.uuidProvider;
    auto& clk  = *services_.clock;

    if (displayName.empty()) {
        return Result<WorldRecord>::failure(
            {.code = ErrorCode::invalidArgument, .message = "world requires a displayName"});
    }

    // Refuse to create over an existing package — that would silently adopt or
    // destroy someone else's world.
    if (auto e = fs_.exists(worldJsonPath(packagePath)); e.ok() && e.value()) {
        return Result<WorldRecord>::failure(
            {.code = ErrorCode::invalidArgument,
             .message = "a world package already exists at that path",
             .path = packagePath});
    }

    WorldRecord w;
    w.worldID     = "world_" + uuid.newObjectID().value;
    w.displayName = displayName;
    w.epochLabel  = epochLabel;
    w.createdAt   = clk.nowUTC();
    w.modifiedAt  = w.createdAt;

    // Package skeleton (§6.1) — self-contained: its own kind directories,
    // its own object index, its own assets.
    for (const char* sub : {"artifacts", "rules", "chronicles", "factions",
                            "historical-events", "historical-timelines", "assets"}) {
        if (auto r = fs_.createDirectories(util::join(packagePath, sub)); !r.ok()) {
            return Result<WorldRecord>::failure(r.error());
        }
    }

    if (auto r = fs_.atomicWriteTextFile(worldJsonPath(packagePath),
                                         schemas::serializeWorld(w)); !r.ok()) {
        return Result<WorldRecord>::failure(r.error());
    }

    // The world's own index — same schema as the project's (SP-095), so it
    // rebuilds by the same scan-and-repair path.
    {
        util::JsonDoc idx;
        idx.setString("schema", "scrivi.object-index.v1");
        if (auto r = fs_.atomicWriteTextFile(worldIndexPath(packagePath), idx.dump());
            !r.ok()) {
            return Result<WorldRecord>::failure(r.error());
        }
    }

    // ...and this project's binding, in the same operation (Doc 3 §7).
    WorldBindingRecord b;
    b.worldID       = w.worldID;
    b.displayName   = w.displayName;
    b.epochOffsetMs = 0;                    // a new binding sits on the project epoch (§7.1)
    std::error_code ec;
    auto rel = fs::relative(fs::path(packagePath), fs::path(projectRoot), ec);
    b.reference.lastKnownPath         = ec ? std::string{} : rel.string();
    b.reference.lastKnownAbsolutePath = packagePath;

    if (auto r = saveBinding(projectRoot, b); !r.ok()) {
        return Result<WorldRecord>::failure(r.error());
    }

    return Result<WorldRecord>::success(std::move(w));
}

Result<WorldRecord> WorldStore::addWorld(const AbsolutePath& projectRoot,
                                          const AbsolutePath& packagePath) const {
    auto& fs_ = *services_.fileSystem;

    // Adding a world is READ-ONLY toward the world, so it takes no lock (§7).
    auto textR = fs_.readTextFile(worldJsonPath(packagePath));
    if (!textR.ok()) {
        return Result<WorldRecord>::failure(
            {.code = ErrorCode::invalidArgument,
             .message = "no world package at that path", .path = packagePath});
    }

    auto parsedR = schemas::parseWorld(textR.value());
    if (!parsedR.ok()) { return Result<WorldRecord>::failure(parsedR.error()); }
    const auto& w = parsedR.value();

    WorldBindingRecord b;
    b.worldID       = w.worldID;
    b.displayName   = w.displayName;
    b.epochOffsetMs = 0;
    std::error_code ec;
    auto rel = fs::relative(fs::path(packagePath), fs::path(projectRoot), ec);
    b.reference.lastKnownPath         = ec ? std::string{} : rel.string();
    b.reference.lastKnownAbsolutePath = packagePath;

    if (auto r = saveBinding(projectRoot, b); !r.ok()) {
        return Result<WorldRecord>::failure(r.error());
    }
    (void)refreshCachedIndex(projectRoot, w.worldID, packagePath);

    return Result<WorldRecord>::success(w);
}

// ---------------------------------------------------------------------------
// T-0382 — bindings, resolution, relink
// ---------------------------------------------------------------------------

Result<WorldBindingRecord> WorldStore::loadBinding(const AbsolutePath& projectRoot,
                                                     const std::string& worldID) const {
    auto textR = services_.fileSystem->readTextFile(bindingPath(projectRoot, worldID));
    if (!textR.ok()) {
        return Result<WorldBindingRecord>::failure(
            {.code = ErrorCode::invalidArgument, .message = "world not bound: " + worldID});
    }
    return schemas::parseWorldBinding(textR.value());
}

Result<void> WorldStore::saveBinding(const AbsolutePath& projectRoot,
                                      const WorldBindingRecord& binding) const {
    auto& fs_ = *services_.fileSystem;
    auto  dir = util::join(worldsDir(projectRoot), binding.worldID);
    if (auto r = fs_.createDirectories(dir); !r.ok()) { return r; }
    return fs_.atomicWriteTextFile(util::join(dir, "binding.json"),
                                   schemas::serializeWorldBinding(binding));
}

Result<std::vector<std::string>>
WorldStore::listBoundWorldIDs(const AbsolutePath& projectRoot) const {
    auto& fs_ = *services_.fileSystem;
    std::vector<std::string> out;

    auto dir = worldsDir(projectRoot);
    if (auto e = fs_.exists(dir); !e.ok() || !e.value()) {
        // A project that never used worlds does nothing here (§4.5).
        return Result<std::vector<std::string>>::success(std::move(out));
    }

    auto listR = fs_.listDirectory(dir);
    if (!listR.ok()) { return Result<std::vector<std::string>>::failure(listR.error()); }

    for (const auto& entry : listR.value()) {
        auto isDir = fs_.isDirectory(entry);
        if (!isDir.ok() || !isDir.value()) { continue; }
        out.push_back(util::filename(entry));
    }
    std::sort(out.begin(), out.end());
    return Result<std::vector<std::string>>::success(std::move(out));
}

WorldResolution WorldStore::resolve(const AbsolutePath& projectRoot,
                                     const std::string& worldID) const {
    auto& fs_ = *services_.fileSystem;
    WorldResolution out;

    auto bindingR = loadBinding(projectRoot, worldID);
    if (!bindingR.ok()) { return out; }          // not bound ⇒ unavailable
    const auto& b = bindingR.value();

    // Candidates in order (§6.4): the RELATIVE path first — it is what survives
    // moving a project and its worlds together — then the absolute one.
    std::vector<AbsolutePath> candidates;
    if (!b.reference.lastKnownPath.empty()) {
        std::error_code ec;
        auto abs = fs::weakly_canonical(fs::path(projectRoot) / b.reference.lastKnownPath, ec);
        candidates.push_back(ec ? (fs::path(projectRoot) / b.reference.lastKnownPath).string()
                                : abs.string());
    }
    if (!b.reference.lastKnownAbsolutePath.empty()) {
        candidates.push_back(b.reference.lastKnownAbsolutePath);
    }

    bool sawContainerButNoPackage = false;

    for (const auto& cand : candidates) {
        auto textR = fs_.readTextFile(worldJsonPath(cand));
        if (!textR.ok()) {
            // Is the CONTAINING FOLDER present and readable? Only then can we
            // positively say the package is missing rather than unreachable.
            auto parentDir = util::parent(cand);
            if (auto e = fs_.exists(parentDir); e.ok() && e.value()) {
                sawContainerButNoPackage = true;
            }
            continue;
        }

        auto parsedR = schemas::parseWorld(textR.value());
        if (!parsedR.ok()) { continue; }

        // ⚠️ IDENTITY CHECK. A package whose worldID differs is NOT this world —
        // resolution stops rather than silently substituting a same-named
        // package. A world's name is a label; its worldID is its identity.
        if (parsedR.value().worldID != worldID) {
            out.status = WorldStatus::missing;
            return out;
        }

        out.status      = WorldStatus::available;
        out.packagePath = cand;
        out.world       = std::move(parsedR.value());
        return out;
    }

    // ⚠️ `missing` ONLY when positively established (§4.6). Otherwise the honest
    // default: a wrong "missing" invites destructive writer remedies when the
    // world may be perfectly intact on an unreachable volume. NEVER GUESS.
    out.status = sawContainerButNoPackage ? WorldStatus::missing
                                          : WorldStatus::unavailable;
    return out;
}

Result<void> WorldStore::relink(const AbsolutePath& projectRoot,
                                 const std::string& worldID,
                                 const AbsolutePath& newPackagePath) const {
    auto& fs_ = *services_.fileSystem;

    auto textR = fs_.readTextFile(worldJsonPath(newPackagePath));
    if (!textR.ok()) {
        return Result<void>::failure(
            {.code = ErrorCode::invalidArgument,
             .message = "no world package at that path", .path = newPackagePath});
    }
    auto parsedR = schemas::parseWorld(textR.value());
    if (!parsedR.ok()) { return Result<void>::failure(parsedR.error()); }

    // Verify identity BEFORE accepting — relink is exactly where a same-named
    // package would otherwise be substituted.
    if (parsedR.value().worldID != worldID) {
        return Result<void>::failure(
            {.code = ErrorCode::invalidArgument,
             .message = "that package is a different world (" + parsedR.value().worldID +
                        "), not " + worldID,
             .path = newPackagePath,
             .detail = "worldIDMismatch"});
    }

    auto bindingR = loadBinding(projectRoot, worldID);
    if (!bindingR.ok()) { return Result<void>::failure(bindingR.error()); }

    auto b = std::move(bindingR.value());
    std::error_code ec;
    auto rel = fs::relative(fs::path(newPackagePath), fs::path(projectRoot), ec);
    b.reference.lastKnownPath         = ec ? std::string{} : rel.string();
    b.reference.lastKnownAbsolutePath = newPackagePath;
    b.displayName                     = parsedR.value().displayName;

    if (auto r = saveBinding(projectRoot, b); !r.ok()) { return r; }
    (void)refreshCachedIndex(projectRoot, worldID, newPackagePath);
    return Result<void>::success();
}

Result<void> WorldStore::removeReference(const AbsolutePath& projectRoot,
                                          const std::string& worldID) const {
    auto& fs_ = *services_.fileSystem;

    auto p = bindingPath(projectRoot, worldID);
    if (auto e = fs_.exists(p); !e.ok() || !e.value()) {
        return Result<void>::failure(
            {.code = ErrorCode::invalidArgument, .message = "world not bound: " + worldID});
    }
    // Removes only the PROJECT's reference. The world package itself is never
    // touched — worlds are shared, not owned by any one project.
    return fs_.removeFile(p);
}

Result<std::vector<WorldSummary>>
WorldStore::listWorlds(const AbsolutePath& projectRoot) const {
    auto idsR = listBoundWorldIDs(projectRoot);
    if (!idsR.ok()) { return Result<std::vector<WorldSummary>>::failure(idsR.error()); }

    std::vector<WorldSummary> out;
    for (const auto& id : idsR.value()) {
        auto bindingR = loadBinding(projectRoot, id);
        if (!bindingR.ok()) { continue; }

        WorldSummary s;
        s.worldID       = id;
        s.epochOffsetMs = bindingR.value().epochOffsetMs;
        s.displayName   = bindingR.value().displayName;   // cached name as fallback

        auto res = resolve(projectRoot, id);
        s.status = res.status;
        if (res.status == WorldStatus::available) {
            s.packagePath = res.packagePath;
            s.displayName = res.world.displayName;        // live name wins
            (void)refreshCachedIndex(projectRoot, id, res.packagePath);
        }
        out.push_back(std::move(s));
    }

    return Result<std::vector<WorldSummary>>::success(std::move(out));
}

// ---------------------------------------------------------------------------
// T-0384 — the epoch chain
// ---------------------------------------------------------------------------

Result<void> WorldStore::setWorldEpochOffset(const AbsolutePath& projectRoot,
                                               const std::string& worldID,
                                               std::int64_t epochOffsetMs) const {
    auto bindingR = loadBinding(projectRoot, worldID);
    if (!bindingR.ok()) { return Result<void>::failure(bindingR.error()); }

    auto b = std::move(bindingR.value());
    b.epochOffsetMs = epochOffsetMs;

    // ⚠️ ONLY the binding is written. world.json is deliberately untouched: two
    // projects may bind the SAME world at different offsets (a prequel and its
    // sequel), so the offset belongs to the project's view of the world, never
    // to the world itself.
    return saveBinding(projectRoot, b);
}

namespace {
// Per-world timeline offsets live beside the binding, keyed by timelineID.
// They are PROJECT-local only in storage; their VALUE is world-relative.
AbsolutePath timelineOffsetsPath(const AbsolutePath& projectRoot,
                                  const std::string& worldID) {
    return util::join(util::join(WorldStore::worldsDir(projectRoot), worldID),
                      "timeline-offsets.json");
}
} // namespace

Result<void> WorldStore::setTimelineEpochOffset(const AbsolutePath& projectRoot,
                                                  const std::string& worldID,
                                                  const std::string& timelineID,
                                                  std::int64_t epochOffsetMs) const {
    auto& fs_ = *services_.fileSystem;
    if (timelineID.empty()) {
        return Result<void>::failure(
            {.code = ErrorCode::invalidArgument, .message = "timelineID required"});
    }

    auto path = timelineOffsetsPath(projectRoot, worldID);

    // Read-modify-write the whole map.
    std::vector<std::pair<std::string, std::int64_t>> entries;
    if (auto textR = fs_.readTextFile(path); textR.ok()) {
        if (auto docR = util::parseJson(textR.value()); docR.ok()) {
            const auto& doc = docR.value();
            const auto count = doc.arraySize("offsets");
            for (std::size_t i = 0; i < count; ++i) {
                auto item = doc.arrayItem("offsets", i);
                auto id = item.getString("timelineID");
                if (!id.empty() && id != timelineID) {
                    entries.emplace_back(id, item.getInt64("epochOffsetMs"));
                }
            }
        }
    }
    entries.emplace_back(timelineID, epochOffsetMs);
    std::sort(entries.begin(), entries.end());

    util::JsonDoc root;
    root.setString("schema", "scrivi.world-timeline-offsets.v1");
    for (const auto& [id, off] : entries) {
        util::JsonDoc item;
        item.setString("timelineID",    id);
        item.setInt64 ("epochOffsetMs", off);
        root.appendToArray("offsets", std::move(item));
    }

    auto dir = util::join(worldsDir(projectRoot), worldID);
    if (auto r = fs_.createDirectories(dir); !r.ok()) { return r; }
    return fs_.atomicWriteTextFile(path, root.dump());
}

Result<std::int64_t> WorldStore::timelineEpochOffset(const AbsolutePath& projectRoot,
                                                       const std::string& worldID,
                                                       const std::string& timelineID) const {
    auto textR = services_.fileSystem->readTextFile(
        timelineOffsetsPath(projectRoot, worldID));
    if (!textR.ok()) { return Result<std::int64_t>::success(0); }   // unset ⇒ 0

    auto docR = util::parseJson(textR.value());
    if (!docR.ok()) { return Result<std::int64_t>::success(0); }

    const auto& doc = docR.value();
    const auto count = doc.arraySize("offsets");
    for (std::size_t i = 0; i < count; ++i) {
        auto item = doc.arrayItem("offsets", i);
        if (item.getString("timelineID") == timelineID) {
            return Result<std::int64_t>::success(item.getInt64("epochOffsetMs"));
        }
    }
    return Result<std::int64_t>::success(0);
}

Result<std::int64_t> WorldStore::resolveTimelineProjectOffset(
    const AbsolutePath& projectRoot,
    const std::string& worldID,
    const std::string& timelineID) const {

    auto bindingR = loadBinding(projectRoot, worldID);
    if (!bindingR.ok()) { return Result<std::int64_t>::failure(bindingR.error()); }

    auto tlR = timelineEpochOffset(projectRoot, worldID, timelineID);
    if (!tlR.ok()) { return Result<std::int64_t>::failure(tlR.error()); }

    // timeline → world → project. The event's own offsetMs is added by the
    // caller, which owns the event.
    return Result<std::int64_t>::success(tlR.value() + bindingR.value().epochOffsetMs);
}

// ---------------------------------------------------------------------------
// T-0385 — cached index
// ---------------------------------------------------------------------------

Result<void> WorldStore::refreshCachedIndex(const AbsolutePath& projectRoot,
                                             const std::string& worldID,
                                             const AbsolutePath& packagePath) const {
    auto& fs_ = *services_.fileSystem;

    auto textR = fs_.readTextFile(worldIndexPath(packagePath));
    if (!textR.ok()) { return Result<void>::success(); }   // nothing to cache yet

    auto docR = util::parseJson(textR.value());
    if (!docR.ok()) { return Result<void>::success(); }

    auto bindingR = loadBinding(projectRoot, worldID);
    if (!bindingR.ok()) { return Result<void>::failure(bindingR.error()); }
    auto b = std::move(bindingR.value());

    // The world's own index WINS whenever the world is reachable — the cache is
    // never authoritative (§6.3), only a display aid for pending entries.
    b.cachedIndex.clear();
    const auto& doc = docR.value();
    const auto count = doc.arraySize("entries");
    for (std::size_t i = 0; i < count; ++i) {
        auto item = doc.arrayItem("entries", i);
        CachedWorldObject e;
        e.objectID    = item.getString("objectID");
        e.kind        = item.getString("kind");
        e.displayName = item.getString("displayName");
        if (!e.objectID.empty()) { b.cachedIndex.push_back(std::move(e)); }
    }

    return saveBinding(projectRoot, b);
}

// ---------------------------------------------------------------------------
// T-0383 — write locking
// ---------------------------------------------------------------------------

WorldLock::WorldLock(CoreServices& services, AbsolutePath packagePath)
    : services_(services), packagePath_(std::move(packagePath)) {}

WorldLock::~WorldLock() {
    if (held_) { (void)release(); }
}

Result<void> WorldLock::acquire(const std::string& projectID) {
    auto& fs_  = *services_.fileSystem;
    auto& uuid = *services_.uuidProvider;
    auto& clk  = *services_.clock;

    const auto path = WorldStore::lockPath(packagePath_);
    const auto now  = clk.nowUTC();

    lockID_ = "lock_" + uuid.newObjectID().value;

    util::JsonDoc holder;
    holder.setString("host",      "local");
    holder.setInt   ("pid",       0);
    holder.setString("projectID", projectID);

    util::JsonDoc doc;
    doc.setString("schema",      std::string(kLockSchema));
    doc.setString("lockID",      lockID_);
    doc.setSubDoc("holder",      std::move(holder));
    doc.setString("acquiredAt",  now);
    doc.setString("heartbeatAt", now);
    const auto body = doc.dump();

    // Exclusive create is the ONLY thing that makes "exactly one winner" true
    // across processes (T-0403).
    if (auto r = fs_.createFileExclusive(path, body); r.ok()) {
        held_ = true;
        return Result<void>::success();
    }

    // Lost the race — but the holder may be dead. A lock whose heartbeat is
    // older than 60 s is presumed dead and may be broken, so a crashed writer
    // blocks others for at most a minute rather than permanently (§6.5).
    if (auto textR = fs_.readTextFile(path); textR.ok()) {
        if (auto docR = util::parseJson(textR.value()); docR.ok()) {
            const auto beat = docR.value().getString("heartbeatAt");
            if (secondsBetween(beat, now) > kStaleSeconds) {
                if (auto rm = fs_.removeFile(path); rm.ok()) {
                    if (auto r2 = fs_.createFileExclusive(path, body); r2.ok()) {
                        held_ = true;
                        return Result<void>::success();
                    }
                }
            }
        } else {
            // Unparseable lock file — an artifact of a crash mid-write. Treat as
            // stale rather than letting garbage lock a world forever.
            if (auto rm = fs_.removeFile(path); rm.ok()) {
                if (auto r2 = fs_.createFileExclusive(path, body); r2.ok()) {
                    held_ = true;
                    return Result<void>::success();
                }
            }
        }
    }

    // Contention: report and move on. NEVER hang — a contended world write must
    // not stall the writer (§4.5); it surfaces as a retryable state.
    return Result<void>::failure(
        {.code = ErrorCode::ioError,
         .message = "world is locked by another writer",
         .path = packagePath_,
         .detail = "worldLocked"});
}

Result<void> WorldLock::heartbeat() {
    if (!held_) {
        return Result<void>::failure(
            {.code = ErrorCode::invalidArgument, .message = "lock not held"});
    }
    auto& fs_ = *services_.fileSystem;
    auto  path = WorldStore::lockPath(packagePath_);

    auto textR = fs_.readTextFile(path);
    if (!textR.ok()) { return Result<void>::failure(textR.error()); }
    auto docR = util::parseJson(textR.value());
    if (!docR.ok()) { return Result<void>::failure(docR.error()); }

    // Only the holder may refresh — otherwise a stale-breaker could keep
    // someone else's lock alive.
    if (docR.value().getString("lockID") != lockID_) {
        held_ = false;
        return Result<void>::failure(
            {.code = ErrorCode::ioError, .message = "lock was broken by another writer"});
    }

    util::JsonDoc doc;
    doc.setString("schema",      std::string(kLockSchema));
    doc.setString("lockID",      lockID_);
    doc.setSubDoc("holder",      docR.value().getSubDoc("holder"));
    doc.setString("acquiredAt",  docR.value().getString("acquiredAt"));
    doc.setString("heartbeatAt", services_.clock->nowUTC());
    return fs_.atomicWriteTextFile(path, doc.dump());
}

Result<void> WorldLock::release() {
    if (!held_) { return Result<void>::success(); }
    held_ = false;

    auto& fs_  = *services_.fileSystem;
    auto  path = WorldStore::lockPath(packagePath_);

    // Only remove OUR lock — if it was broken and re-acquired, the file now
    // belongs to someone else.
    if (auto textR = fs_.readTextFile(path); textR.ok()) {
        if (auto docR = util::parseJson(textR.value()); docR.ok()) {
            if (docR.value().getString("lockID") != lockID_) {
                return Result<void>::success();
            }
        }
    }
    return fs_.removeFile(path);
}

} // namespace scrivi::worlds
