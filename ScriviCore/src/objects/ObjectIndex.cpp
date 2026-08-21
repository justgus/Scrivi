#include "objects/ObjectIndex.hpp"

#include "schemas/ObjectJson.hpp"
#include "util/Json.hpp"
#include "util/PathUtils.hpp"
#include "worlds/WorldStore.hpp"

#include <algorithm>

namespace scrivi::objects {

namespace {

constexpr std::string_view kIndexSchema = "scrivi.object-index.v1";

// Kinds the PROJECT index scans — project-scoped only. World-scoped kinds
// (artifact/chronicle/faction/rule) live in .scrivworld packages and are carried
// in the binding's cachedIndex instead (Doc 3 §6.3). `rule` left this list in
// SP-097 when it became world-scoped (T-0404).
// ⚠️ SP-103 / T-0409: this was a THIRD copy of the kind list (after the enum and
// the C ABI's) and it still named the pre-ruling project scope, so a rebuild
// scanned for characters under objects/ where none can now live. It is now
// derived from objectKindIsWorldScoped() rather than restated — the same defect
// shape as I-0113 and the `source` kind table, found a third time.
// ⚠️ SP-104: the list itself now lives in ObjectTypes.hpp beside the enum, as
// `scrivi::kAllStorableKinds`. It was private to this file, which is precisely
// why WorldStore::createWorld restated it — and shipped a world package with no
// characters/ directory. One canonical list, reachable by everything that needs
// it, is the only version of this rule that holds.

// NB: kind parsing lives in ObjectTypes.hpp as objectKindFromName() — it
// accepts world-scoped names too, so an index written by a later sprint is
// readable here rather than triggering a spurious rebuild.

} // namespace

ObjectIndex::ObjectIndex(CoreServices& services)
    : services_(services) {}

AbsolutePath ObjectIndex::indexPath(const AbsolutePath& projectRoot) {
    return util::join(util::join(projectRoot, "objects"), "index.json");
}

std::optional<std::vector<ObjectIndexEntry>>
ObjectIndex::parse(std::string_view json) const {
    auto docR = util::parseJson(json);
    if (!docR.ok()) { return std::nullopt; }

    auto& doc = docR.value();
    if (doc.getString("schema") != kIndexSchema) { return std::nullopt; }

    std::vector<ObjectIndexEntry> entries;
    const auto count = doc.arraySize("entries");
    entries.reserve(count);

    for (std::size_t i = 0; i < count; ++i) {
        auto item = doc.arrayItem("entries", i);

        auto kind = objectKindFromName(item.getString("kind"));
        if (!kind) { return std::nullopt; }          // unknown kind ⇒ rebuild

        ObjectIndexEntry e;
        e.objectID.value = item.getString("objectID");
        if (e.objectID.value.empty()) { return std::nullopt; }  // no identity ⇒ rebuild
        e.kind        = *kind;
        e.slug        = item.getString("slug");
        e.displayName = item.getString("displayName");
        e.worldID     = item.getString("worldID");
        entries.push_back(std::move(e));
    }

    return entries;
}

Result<std::vector<ObjectIndexEntry>>
ObjectIndex::load(const AbsolutePath& projectRoot) const {
    auto& fs = *services_.fileSystem;

    auto path    = indexPath(projectRoot);
    auto existsR = fs.exists(path);

    if (existsR.ok() && existsR.value()) {
        auto textR = fs.readTextFile(path);
        if (textR.ok()) {
            if (auto parsed = parse(textR.value())) {
                return Result<std::vector<ObjectIndexEntry>>::success(std::move(*parsed));
            }
        }
        // Present but unreadable or malformed — fall through to rebuild. A
        // corrupt index must never keep the project from opening.
    }

    return rebuild(projectRoot);
}

// Scans one directory tree for object files. Shared by the project rebuild and
// the world rebuild — the two differ only in WHICH kinds can live there and
// where the subdirectories hang from.
std::vector<ObjectIndexEntry>
ObjectIndex::scanDir(const AbsolutePath& baseDir, bool worldScoped) const {
    auto& fs = *services_.fileSystem;
    std::vector<ObjectIndexEntry> entries;

    for (auto kind : kAllStorableKinds) {
        if (objectKindIsWorldScoped(kind) != worldScoped) { continue; }
        auto dir     = util::join(baseDir, objectKindSubdir(kind));
        auto existsR = fs.exists(dir);
        if (!existsR.ok() || !existsR.value()) { continue; }

        auto listR = fs.listDirectory(dir);
        if (!listR.ok()) { continue; }

        for (const auto& entry : listR.value()) {
            if (util::extension(entry) != ".json") { continue; }

            auto textR = fs.readTextFile(entry);
            if (!textR.ok()) { continue; }

            // Best-effort: one unparseable object file must not cost the index.
            auto parseR = schemas::parseWorldObject(textR.value(), kind);
            if (!parseR.ok()) { continue; }

            const auto& f = worldObjectFields(parseR.value());
            if (f.objectID.value.empty()) { continue; }

            ObjectIndexEntry e;
            e.objectID    = f.objectID;
            e.kind        = kind;
            e.slug        = f.slug;
            e.displayName = f.displayName;
            e.worldID     = f.worldID;
            entries.push_back(std::move(e));
        }
    }

    // Deterministic order so a rebuild is idempotent regardless of the order the
    // filesystem hands back directory entries.
    std::sort(entries.begin(), entries.end(),
              [](const ObjectIndexEntry& a, const ObjectIndexEntry& b) {
                  return a.objectID.value < b.objectID.value;
              });
    return entries;
}

Result<std::vector<ObjectIndexEntry>>
ObjectIndex::rebuild(const AbsolutePath& projectRoot) const {
    auto& fs         = *services_.fileSystem;
    auto  objectsDir = util::join(projectRoot, "objects");

    std::vector<ObjectIndexEntry> entries;

    for (auto kind : kAllStorableKinds) {
        if (objectKindIsWorldScoped(kind)) { continue; }
        auto dir     = util::join(objectsDir, objectKindSubdir(kind));
        auto existsR = fs.exists(dir);
        if (!existsR.ok() || !existsR.value()) { continue; }

        auto listR = fs.listDirectory(dir);
        if (!listR.ok()) { continue; }

        for (const auto& entry : listR.value()) {
            if (util::extension(entry) != ".json") { continue; }

            auto textR = fs.readTextFile(entry);
            if (!textR.ok()) { continue; }

            // Best-effort, matching the posture elsewhere in the codebase: one
            // unparseable object file must not cost the whole index.
            auto parseR = schemas::parseWorldObject(textR.value(), kind);
            if (!parseR.ok()) { continue; }

            const auto& f = worldObjectFields(parseR.value());
            if (f.objectID.value.empty()) { continue; }

            ObjectIndexEntry e;
            e.objectID    = f.objectID;
            e.kind        = kind;
            e.slug        = f.slug;
            e.displayName = f.displayName;
            e.worldID     = f.worldID;
            entries.push_back(std::move(e));
        }
    }

    // Deterministic order so a rebuild is idempotent regardless of the order
    // the filesystem hands back directory entries.
    std::sort(entries.begin(), entries.end(),
              [](const ObjectIndexEntry& a, const ObjectIndexEntry& b) {
                  return a.objectID.value < b.objectID.value;
              });

    if (auto r = write(projectRoot, entries); !r.ok()) {
        return Result<std::vector<ObjectIndexEntry>>::failure(r.error());
    }

    return Result<std::vector<ObjectIndexEntry>>::success(std::move(entries));
}

Result<void> ObjectIndex::write(const AbsolutePath& projectRoot,
                                 const std::vector<ObjectIndexEntry>& entries) const {
    auto& fs = *services_.fileSystem;

    auto objectsDir = util::join(projectRoot, "objects");
    if (auto r = fs.createDirectories(objectsDir); !r.ok()) { return r; }

    util::JsonDoc root;
    root.setString("schema", std::string(kIndexSchema));

    for (const auto& e : entries) {
        util::JsonDoc item;
        item.setString("objectID",    e.objectID.value);
        item.setString("kind",        objectKindName(e.kind));
        item.setString("slug",        e.slug);
        item.setString("displayName", e.displayName);
        item.setString("worldID",     e.worldID);
        root.appendToArray("entries", std::move(item));
    }

    return fs.atomicWriteTextFile(indexPath(projectRoot), root.dump());
}

// --- world-package index (Doc 3 §6.1) ---------------------------------------
//
// Same schema, different file: <package>/index.json rather than
// <project>/objects/index.json. Kept here so there is ONE index reader/writer
// rather than a second implementation that could drift.

Result<std::vector<ObjectIndexEntry>>
ObjectIndex::loadWorldIndex(const AbsolutePath& packagePath) const {
    auto& fs = *services_.fileSystem;
    auto  p  = util::join(packagePath, "index.json");

    if (auto textR = fs.readTextFile(p); textR.ok()) {
        if (auto parsed = parse(textR.value())) {
            return Result<std::vector<ObjectIndexEntry>>::success(std::move(*parsed));
        }
    }

    // ⚠️ SP-103 / T-0409 — this used to return an EMPTY index here, under a
    // comment claiming it would be "rebuilt by the next write." It is not: the
    // next upsertWorld writes an index containing only the object it just added,
    // silently dropping every other object in the world from the index.
    //
    // Verified by probe before fixing: three characters created, world index
    // deleted, one more character created → scrivi_list_objects returned ONLY
    // the new one. The other three files were still on disk and perfectly
    // readable; they had simply become invisible. To a writer that is three
    // characters vanishing.
    //
    // That was survivable while worlds held only artifacts/chronicles/factions.
    // It is not now: with EVERY worldbuilding kind in the world (Doc 1 §3.0),
    // this is where a whole cast disappears. The world index gets the same
    // scan-rebuild guarantee the project index has always had (AC2).
    auto entries = scanDir(packagePath, /*worldScoped=*/true);

    // ⚠️ I-0144 (SP-116 T-0431) — A READ PATH THAT WRITES, DELIBERATELY LEFT
    // UNLOCKED. Recorded here because the omission is a decision, not an
    // oversight.
    //
    // This rebuild fires during what callers believe is a read
    // (EndpointResolver.cpp:41, ObjectStore::findByID) and writes into a shared
    // world package. It is the one world-package write T-0431 does NOT lock, for
    // a reason that matters:
    //
    // ⚠️ WorldLock IS NOT REENTRANT. `save` and `remove` call findByID — which
    // reaches here — while already holding the package lock. Acquiring again
    // would fail with "worldLocked" against ITSELF, and the rebuild would be
    // silently skipped in exactly the write paths that most need the index
    // correct. A guard here would be worse than none.
    //
    // Why leaving it unlocked is tolerable: the rebuild is a full scan of the
    // package and is IDEMPOTENT — two processes racing it write the same bytes,
    // and atomicWriteTextFile is temp-write→rename, so no reader ever sees a
    // torn file. The worst case is a redundant write, not a corrupt index.
    //
    // ⚠️ The proper fix is a reentrant/recursive world lock, which is a change to
    // the locking MODEL and belongs with the network-worlds design that must
    // already revisit "exactly one winner". Deliberately not invented here.
    if (auto r = writeWorldIndex(packagePath, entries); !r.ok()) {
        return Result<std::vector<ObjectIndexEntry>>::failure(r.error());
    }
    return Result<std::vector<ObjectIndexEntry>>::success(std::move(entries));
}

// One writer for the world index, so the rebuild path and the upsert path can
// never drift in schema or ordering.
Result<void> ObjectIndex::writeWorldIndex(
    const AbsolutePath& packagePath,
    const std::vector<ObjectIndexEntry>& entries) const {
    util::JsonDoc root;
    root.setString("schema", std::string(kIndexSchema));
    for (const auto& e : entries) {
        util::JsonDoc item;
        item.setString("objectID",    e.objectID.value);
        item.setString("kind",        objectKindName(e.kind));
        item.setString("slug",        e.slug);
        item.setString("displayName", e.displayName);
        item.setString("worldID",     e.worldID);
        root.appendToArray("entries", std::move(item));
    }
    return services_.fileSystem->atomicWriteTextFile(
        util::join(packagePath, "index.json"), root.dump());
}

Result<void> ObjectIndex::upsertWorld(const AbsolutePath& packagePath,
                                       const ObjectIndexEntry& entry) const {
    auto loadedR = loadWorldIndex(packagePath);
    if (!loadedR.ok()) { return Result<void>::failure(loadedR.error()); }

    auto entries = std::move(loadedR.value());
    auto it = std::find_if(entries.begin(), entries.end(),
                           [&](const ObjectIndexEntry& e) {
                               return e.objectID.value == entry.objectID.value;
                           });
    if (it != entries.end()) { *it = entry; } else { entries.push_back(entry); }

    std::sort(entries.begin(), entries.end(),
              [](const ObjectIndexEntry& a, const ObjectIndexEntry& b) {
                  return a.objectID.value < b.objectID.value;
              });

    return writeWorldIndex(packagePath, entries);
}

Result<void> ObjectIndex::eraseWorld(const AbsolutePath& packagePath,
                                      const ObjectID& id) const {
    auto loadedR = loadWorldIndex(packagePath);
    if (!loadedR.ok()) { return Result<void>::failure(loadedR.error()); }

    auto entries = std::move(loadedR.value());
    entries.erase(std::remove_if(entries.begin(), entries.end(),
                                 [&](const ObjectIndexEntry& e) {
                                     return e.objectID.value == id.value;
                                 }),
                  entries.end());

    util::JsonDoc root;
    root.setString("schema", std::string(kIndexSchema));
    for (const auto& e : entries) {
        util::JsonDoc item;
        item.setString("objectID",    e.objectID.value);
        item.setString("kind",        objectKindName(e.kind));
        item.setString("slug",        e.slug);
        item.setString("displayName", e.displayName);
        item.setString("worldID",     e.worldID);
        root.appendToArray("entries", std::move(item));
    }
    return services_.fileSystem->atomicWriteTextFile(
        util::join(packagePath, "index.json"), root.dump());
}

Result<ObjectIndexEntry> ObjectIndex::find(const AbsolutePath& projectRoot,
                                            const ObjectID& id) const {
    auto entriesR = load(projectRoot);
    if (!entriesR.ok()) { return Result<ObjectIndexEntry>::failure(entriesR.error()); }

    auto match = [&](const std::vector<ObjectIndexEntry>& v)
        -> std::optional<ObjectIndexEntry> {
        for (const auto& e : v) {
            if (e.objectID.value == id.value) { return e; }
        }
        return std::nullopt;
    };

    if (auto hit = match(entriesR.value())) {
        return Result<ObjectIndexEntry>::success(std::move(*hit));
    }

    // A miss may just mean the index is behind disk (an object created by an
    // older build, or a file dropped in by hand). Rebuild once before
    // concluding the object does not exist.
    auto rebuiltR = rebuild(projectRoot);
    if (!rebuiltR.ok()) { return Result<ObjectIndexEntry>::failure(rebuiltR.error()); }

    if (auto hit = match(rebuiltR.value())) {
        return Result<ObjectIndexEntry>::success(std::move(*hit));
    }

    return Result<ObjectIndexEntry>::failure(
        {.code = ErrorCode::ioError, .message = "object not found: " + id.value});
}

Result<std::vector<ObjectIndexEntry>>
ObjectIndex::loadAllVisible(const AbsolutePath& projectRoot) const {
    auto entriesR = load(projectRoot);
    if (!entriesR.ok()) { return entriesR; }

    auto entries = std::move(entriesR.value());

    worlds::WorldStore ws{services_};
    auto idsR = ws.listBoundWorldIDs(projectRoot);
    // A project that never used worlds does no world work at all (Doc 3 §4.5) —
    // listBoundWorldIDs returns empty for a missing worlds/ dir, so this loop
    // simply does not run.
    if (idsR.ok()) {
        for (const auto& worldID : idsR.value()) {
            auto res = ws.resolve(projectRoot, worldID);
            if (res.status != worlds::WorldStatus::available) { continue; }

            if (auto worldEntries = loadWorldIndex(res.packagePath); worldEntries.ok()) {
                for (auto& e : worldEntries.value()) {
                    // The world's own index records the kind and slug; stamp the
                    // owning worldID so a caller can tell partitions apart
                    // without a second lookup.
                    if (e.worldID.empty()) { e.worldID = worldID; }
                    entries.push_back(std::move(e));
                }
            }
        }
    }

    return Result<std::vector<ObjectIndexEntry>>::success(std::move(entries));
}

Result<void> ObjectIndex::upsert(const AbsolutePath& projectRoot,
                                  const ObjectIndexEntry& entry) const {
    auto entriesR = load(projectRoot);
    if (!entriesR.ok()) { return Result<void>::failure(entriesR.error()); }

    auto entries = std::move(entriesR.value());
    auto it = std::find_if(entries.begin(), entries.end(),
                           [&](const ObjectIndexEntry& e) {
                               return e.objectID.value == entry.objectID.value;
                           });

    if (it != entries.end()) {
        *it = entry;
    } else {
        entries.push_back(entry);
        std::sort(entries.begin(), entries.end(),
                  [](const ObjectIndexEntry& a, const ObjectIndexEntry& b) {
                      return a.objectID.value < b.objectID.value;
                  });
    }

    return write(projectRoot, entries);
}

Result<void> ObjectIndex::erase(const AbsolutePath& projectRoot,
                                 const ObjectID& id) const {
    auto entriesR = load(projectRoot);
    if (!entriesR.ok()) { return Result<void>::failure(entriesR.error()); }

    auto entries = std::move(entriesR.value());
    entries.erase(std::remove_if(entries.begin(), entries.end(),
                                 [&](const ObjectIndexEntry& e) {
                                     return e.objectID.value == id.value;
                                 }),
                  entries.end());

    return write(projectRoot, entries);
}

} // namespace scrivi::objects
