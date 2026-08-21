#include "worlds/WorldStore.hpp"

#include "objects/ObjectIndex.hpp"
#include "schemas/WorldJson.hpp"
#include "util/Json.hpp"
#include "scrivi/AssetTypes.hpp"
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
    //
    // ⚠️ DERIVED, never restated. This was a hardcoded list of seven literals
    // naming the PRE-SP-104 world scope, so a world created after the ruling had
    // no characters/ directory — the fourth instance of this Epic's restated-kind
    // defect (I-0113, SP-098, SP-103, here). Driving it from kAllStorableKinds +
    // objectKindIsWorldScoped means a kind whose scope changes cannot leave this
    // skeleton stale.
    for (auto kind : kAllStorableKinds) {
        if (!objectKindIsWorldScoped(kind)) { continue; }
        if (auto r = fs_.createDirectories(util::join(packagePath, objectKindSubdir(kind)));
            !r.ok()) {
            return Result<WorldRecord>::failure(r.error());
        }
    }

    // Non-kind directories the package also owns (§6.1): historical time and
    // binary assets are not ObjectKinds, so they stay explicit.
    for (const char* sub : {"historical-events", "historical-timelines", "assets"}) {
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

    // ⚠️ T-0419 (I-0137): record where we are about to look, BEFORE we know
    // whether looking succeeds. The first candidate is the best one by §6.4's
    // ordering (relative path first — it survives moving a project and its
    // worlds together). This is "where we looked", never "what we verified".
    if (!candidates.empty()) {
        out.lastKnownPackagePath = candidates.front();
    }

    bool sawContainerButNoPackage = false;

    for (const auto& cand : candidates) {
        auto textR = fs_.readTextFile(worldJsonPath(cand));
        if (!textR.ok()) {
            // ⚠️ A FAILED READ IS NOT EVIDENCE OF ABSENCE. It is equally the
            // signature of an unreadable-but-present package — which is exactly
            // what a sandboxed host produces when the package sits outside the
            // granted paths. Deciding `missing` from a failed read + a readable
            // PARENT reported a perfectly intact world as gone, because the
            // parent (e.g. ~/Desktop) is reachable while the package is not.
            //
            // So require positive evidence that the package itself is absent:
            // ask whether it exists. Only a definitive "no" establishes missing;
            // an error (permission denied) leaves the honest `unavailable`.
            auto pkgE = fs_.exists(cand);
            if (pkgE.ok() && !pkgE.value()) {
                auto parentDir = util::parent(cand);
                if (auto e = fs_.exists(parentDir); e.ok() && e.value()) {
                    sawContainerButNoPackage = true;
                }
            }
            continue;
        }

        auto parsedR = schemas::parseWorld(textR.value());
        if (!parsedR.ok()) {
            // ⚠️ T-0420 (I-0136): a package too NEW to parse is emphatically not
            // absent — we just read its world.json. Falling through to the
            // sawContainerButNoPackage logic could report `missing`, which
            // §6a.0 forbids: absence is never deletion, and a wrong "missing"
            // invites destructive writer remedies against an intact world.
            //
            // `unavailable` is the honest answer — the package is there and we
            // cannot use it. The reason reaches the writer through the parse
            // error's `unsupportedWorldFormatVersion` detail, not by pretending
            // the world is gone.
            if (parsedR.error().code == ErrorCode::unsupportedVersion) {
                out.status = WorldStatus::unavailable;
                out.lastKnownPackagePath = cand;
                return out;
            }
            continue;
        }

        // ⚠️ IDENTITY CHECK. A package whose worldID differs is NOT this world —
        // resolution stops rather than silently substituting a same-named
        // package. A world's name is a label; its worldID is its identity.
        if (parsedR.value().worldID != worldID) {
            out.status = WorldStatus::missing;
            // T-0419: we DID reach a package here — it is simply not this world.
            // Report where, without claiming it as a verified path for this ID.
            out.lastKnownPackagePath = cand;
            return out;
        }

        out.status               = WorldStatus::available;
        out.packagePath          = cand;
        out.lastKnownPackagePath = cand;   // T-0419: the verified path IS the last known
        out.world                = std::move(parsedR.value());
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
        // ⚠️ T-0419 (I-0137): carried REGARDLESS of status — this is the whole
        // point. `packagePath` below stays available-only; the refinement that
        // distinguishes `unmounted` from `offline` needs a path precisely when
        // the world is NOT available.
        s.lastKnownPackagePath = res.lastKnownPackagePath;
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
        // I-0146: sweep on EVERY successful acquire, not only after breaking a
        // stale lock. ⚠️ The observed failure orphaned the lock file AND the
        // partial together — when the lock is later gone (removed, or on a
        // volume that came back without it), the next writer acquires cleanly
        // and never reaches a break path at all. Sweeping only on a break would
        // therefore miss the exact case this Issue was filed for.
        (void)sweepAbandonedPartials();
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
                        // The holder we just declared dead may have died
                        // mid-copy; its partial is unresumable.
                        (void)sweepAbandonedPartials();
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
                    (void)sweepAbandonedPartials();
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


std::size_t WorldLock::sweepAbandonedPartials() {
    if (!held_) { return 0; }   // only a lock HOLDER may sweep

    auto& fs_ = *services_.fileSystem;
    std::size_t removed = 0;

    // Partials only ever appear beside their destination, i.e. under
    // <package>/assets/<category>/. Derived from assetCategorySubdir rather than
    // written out, so a new category cannot leave a directory unswept.
    const auto assetsDir = util::join(packagePath_, "assets");
    for (auto cat : {AssetCategory::image, AssetCategory::audio,
                     AssetCategory::video, AssetCategory::document,
                     AssetCategory::other}) {
        const auto dir = util::join(assetsDir, assetCategorySubdir(cat));

        auto existsR = fs_.exists(dir);
        if (!existsR.ok() || !existsR.value()) { continue; }

        auto listR = fs_.listDirectory(dir);
        if (!listR.ok()) { continue; }

        for (const auto& entry : listR.value()) {
            constexpr std::string_view kSuffix = ".partial";
            if (entry.size() <= kSuffix.size()) { continue; }
            if (entry.compare(entry.size() - kSuffix.size(), kSuffix.size(),
                              kSuffix) != 0) { continue; }

            // Best-effort: a partial we cannot delete is not worth failing the
            // caller's actual write over. It will be swept by the next holder.
            if (auto rm = fs_.removeFile(entry); rm.ok()) { ++removed; }
        }
    }
    return removed;
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

// --- WorldWriteGuard (I-0144, T-0431) ---------------------------------------

WorldWriteGuard::WorldWriteGuard(CoreServices& services,
                                 const AbsolutePath& projectRoot,
                                 const std::string& worldID,
                                 const std::string& projectID)
{
    // A project write needs no lock: project files have a single owning process
    // by construction. Staying inert here is what lets callers drop their own
    // `if (world)` branch — the branch that got forgotten for three sprints.
    if (worldID.empty()) { return; }

    WorldStore store{services};
    auto res = store.resolve(projectRoot, worldID);
    if (res.status != WorldStatus::available) {
        // Same detail as ObjectStore::kindDirFor and AssetStore::assetRoot, so
        // every world-write surface fails identically.
        status_ = Result<void>::failure(
            {.code = ErrorCode::invalidArgument,
             .message = "world '" + worldID + "' is " + worldStatusName(res.status),
             .detail = "worldUnavailable:" + worldStatusName(res.status)});
        return;
    }

    packagePath_ = res.packagePath;
    lock_.emplace(services, res.packagePath);
    status_ = lock_->acquire(projectID.empty() ? "unknown" : projectID);
    if (!status_.ok()) {
        // Do not hold a half-acquired lock: releasing here keeps ~WorldLock from
        // removing a lock file this guard never won.
        lock_.reset();
    }
}

Result<void> WorldWriteGuard::heartbeat() {
    if (!lock_) { return Result<void>::success(); }
    return lock_->heartbeat();
}

} // namespace scrivi::worlds
