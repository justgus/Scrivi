#include "assets/AssetStore.hpp"

#include "domain/Slug.hpp"
#include "schemas/AssetMetaJson.hpp"
#include "util/PathUtils.hpp"
#include "worlds/WorldStore.hpp"

#include <optional>

namespace scrivi::assets {

// Block size for asset copies (T-0432). 1 MiB: large enough that the per-block
// callback is negligible against real I/O, small enough that even a very slow
// volume kicks the watchdog many times inside one kStaleSeconds window.
static constexpr std::size_t kCopyBlockBytes = 1u << 20;

AssetStore::AssetStore(CoreServices& services)
    : services_(services) {}

AbsolutePath AssetStore::categoryDir(const AbsolutePath& projectRoot,
                                      AssetCategory cat)
{
    return util::join(util::join(projectRoot, "assets"),
                      assetCategorySubdir(cat));
}

Result<AbsolutePath> AssetStore::assetRoot(const AbsolutePath& projectRoot,
                                            const std::string& worldID) const
{
    if (worldID.empty()) {
        return Result<AbsolutePath>::success(projectRoot);
    }

    worlds::WorldStore store{services_};
    auto res = store.resolve(projectRoot, worldID);
    if (res.status != worlds::WorldStatus::available) {
        // ⚠️ An unreachable world is NOT a missing asset. Refusing here is what
        // keeps a writer with an ejected drive from being told nothing and left
        // with a half-written asset, or a sidecar orphaned from its bytes.
        //
        // The detail string is byte-identical to ObjectStore::kindDirFor's so
        // both surfaces fail the same way.
        return Result<AbsolutePath>::failure(
            {.code = ErrorCode::invalidArgument,
             .message = "world '" + worldID + "' is " +
                        worlds::worldStatusName(res.status),
             .detail = "worldUnavailable:" + worlds::worldStatusName(res.status)});
    }

    return Result<AbsolutePath>::success(res.packagePath);
}

// ---------------------------------------------------------------------------
// import
// ---------------------------------------------------------------------------

Result<ImportAssetResult> AssetStore::import(const ImportAssetRequest& request) const
{
    auto& fs    = *services_.fileSystem;
    auto& uuid  = *services_.uuidProvider;
    auto& clock = *services_.clock;

    // D6: where do these bytes belong — this project, or the world package?
    auto rootR = assetRoot(request.projectRootPath, request.worldID);
    if (!rootR.ok()) { return Result<ImportAssetResult>::failure(rootR.error());
}
    const auto& root = rootR.value();

    // A world package is write-locked; the project is not. The lock is held for
    // the duration of THIS write only, never a session (World Data Separation
    // §6.5), and released by RAII on every exit path below — including failure.
    std::optional<worlds::WorldLock> lock;
    if (!request.worldID.empty()) {
        lock.emplace(services_, root);
        if (auto r = lock->acquire(request.projectID.empty() ? "unknown"
                                                            : request.projectID);
            !r.ok()) {
            return Result<ImportAssetResult>::failure(r.error());
        }
    }

    auto dir = categoryDir(root, request.category);
    if (auto r = fs.createDirectories(dir); !r.ok()) {
        return Result<ImportAssetResult>::failure(r.error());
}

    // Destination: assets/<category>/<filename>
    auto filename = util::filename(request.sourcePath);
    auto destPath = util::join(dir, filename);

    // ⚠️ Did these bytes already exist? atomicWriteTextFile is temp-write →
    // rename, so importing the same filename twice OVERWRITES the first copy.
    // The rollback below must then delete nothing: removing destPath would
    // destroy bytes the EARLIER asset's sidecar still points at, turning a
    // failed import into data loss for an asset that was perfectly healthy.
    // Roll back only what this call actually created.
    const bool destExisted = [&] {
        auto e = fs.exists(destPath);
        return e.ok() && e.value();
    }();

    // ⚠️ D6's heartbeat requirement, now satisfied properly (T-0432).
    //
    // An asset import is the ONLY world write whose duration depends on FILE
    // SIZE, so a large map on a slow volume could approach kStaleSeconds (60)
    // and have its lock broken mid-write. The copy therefore runs in BLOCKS and
    // kicks the watchdog after each one: lock liveness now tracks PROGRESS
    // rather than a guess about how long a write "should" take, so a transfer of
    // any size is safe. ⚠️ kStaleSeconds is NOT raised (D6) — that would have
    // weakened crash recovery for every world write to paper over this one.
    //
    // ⚠️ Losing the lock ABORTS the transfer. heartbeat() fails if another
    // process broke and re-took the lock, and returning that failure from the
    // callback stops the copy rather than writing on into a package this process
    // no longer owns. copyFileInBlocks writes to a temporary and renames only on
    // success, so an abort leaves nothing behind.
    if (auto r = fs.copyFileInBlocks(
            request.sourcePath, destPath, kCopyBlockBytes,
            [&]() -> Result<void> {
                if (!lock) { return Result<void>::success(); }
                return lock->heartbeat();
            });
        !r.ok()) {
        return Result<ImportAssetResult>::failure(r.error());
}

    // Build sidecar metadata.
    AssetMeta meta;
    meta.assetID                = uuid.newObjectID().value;  // reuse ObjectID UUIDs
    meta.slug                   = util::makeSlug(
        request.title.empty() ? filename : request.title);
    meta.filename               = filename;
    meta.category               = request.category;
    meta.mimeType               = "";   // MIME detection is out of scope for v1
    meta.importedAt             = clock.nowUTC();
    meta.importedByIdentityID   = request.author.identityID.value;
    meta.importedByPersonaID    = request.author.personaID.value;
    meta.importedByDisplayName  = request.author.displayName;
    meta.title                  = request.title;

    auto sidecarPath = destPath + ".meta.json";
    auto sidecarJson = schemas::serializeAssetMeta(meta);

    // Second heartbeat: the byte copy above may itself have taken most of a
    // stale window, so the sidecar write starts from a fresh one.
    if (lock) {
        if (auto r = lock->heartbeat(); !r.ok()) {
            if (!destExisted) { (void)fs.removeFile(destPath); }
            return Result<ImportAssetResult>::failure(r.error());
        }
    }

    if (auto r = fs.atomicWriteTextFile(sidecarPath, sidecarJson); !r.ok()) {
        // ⚠️ D6: "never leave a sidecar orphaned from its bytes" — and the
        // converse is just as bad. Bytes with no sidecar are INVISIBLE to
        // list() (which scans for *.meta.json) yet still occupy the package,
        // so a failed import would silently bloat a shared world with
        // unreachable files. Roll the copy back — but only if WE created it
        // (see destExisted above); otherwise this would delete an existing
        // asset's bytes on a failure that had nothing to do with it.
        if (!destExisted) { (void)fs.removeFile(destPath); }
        return Result<ImportAssetResult>::failure(r.error());
}

    ImportAssetResult result;
    result.assetID     = meta.assetID;
    result.assetPath   = destPath;
    result.sidecarPath = sidecarPath;
    return Result<ImportAssetResult>::success(std::move(result));
}

// ---------------------------------------------------------------------------
// list
// ---------------------------------------------------------------------------

Result<ListAssetsResult> AssetStore::list(const ListAssetsRequest& request) const
{
    auto& fs = *services_.fileSystem;

    // D6: list the world's assets when a worldID is given, the project's
    // otherwise. Reads never take the write lock — any number of projects may
    // read a world at once (World Data Separation §6.5).
    auto rootR = assetRoot(request.projectRootPath, request.worldID);
    if (!rootR.ok()) { return Result<ListAssetsResult>::failure(rootR.error());
}
    const auto& root = rootR.value();

    ListAssetsResult result;

    // Helper to scan one category directory for .meta.json files.
    auto scanCategory = [&](AssetCategory cat) -> Result<void> {
        auto dir = categoryDir(root, cat);

        auto existsR = fs.exists(dir);
        if (!existsR.ok() || !existsR.value()) {
            return Result<void>::success();  // directory absent — skip silently
}

        auto listR = fs.listDirectory(dir);
        if (!listR.ok()) { return Result<void>::failure(listR.error());
}

        for (const auto& entry : listR.value()) {
            if (util::extension(entry) != ".json") { continue;
}
            // Only consider files ending in .meta.json
            auto base = util::filename(entry);
            if (base.size() < 10 ||
                base.substr(base.size() - 10) != ".meta.json") { continue;
}

            auto textR = fs.readTextFile(entry);
            if (!textR.ok()) { continue;
}
            auto metaR = schemas::parseAssetMeta(textR.value());
            if (!metaR.ok()) { continue;
}
            // T-0427 (D7): resolve the bytes' path from the sidecar we just
            // read — import writes it as "<binary>.meta.json", so stripping the
            // suffix yields the binary. Derived from the root being scanned, so
            // it stays correct for a world package opened from any project.
            ListedAsset listed;
            listed.assetPath = entry.substr(0, entry.size() - 10);
            listed.meta      = std::move(metaR.value());
            result.assets.push_back(std::move(listed));
        }
        return Result<void>::success();
    };

    if (request.category.has_value()) {
        auto r = scanCategory(request.category.value());
        if (!r.ok()) { return Result<ListAssetsResult>::failure(r.error());
}
    } else {
        for (auto cat : {AssetCategory::image, AssetCategory::audio,
                         AssetCategory::video, AssetCategory::document,
                         AssetCategory::other}) {
            auto r = scanCategory(cat);
            if (!r.ok()) { return Result<ListAssetsResult>::failure(r.error());
}
        }
    }

    return Result<ListAssetsResult>::success(std::move(result));
}

// ---------------------------------------------------------------------------
// remove
// ---------------------------------------------------------------------------

Result<RemoveAssetResult> AssetStore::remove(const RemoveAssetRequest& request) const
{
    auto& fs = *services_.fileSystem;

    // D6: remove from the world package when a worldID is given.
    auto rootR = assetRoot(request.projectRootPath, request.worldID);
    if (!rootR.ok()) { return Result<RemoveAssetResult>::failure(rootR.error());
}
    const auto& root = rootR.value();

    // A deletion is a world write, so it takes the lock like any other. No
    // heartbeat loop here: removeFile is bounded by metadata, not file size,
    // which is precisely what makes import the special case.
    std::optional<worlds::WorldLock> lock;
    if (!request.worldID.empty()) {
        lock.emplace(services_, root);
        if (auto r = lock->acquire(request.projectID.empty() ? "unknown"
                                                            : request.projectID);
            !r.ok()) {
            return Result<RemoveAssetResult>::failure(r.error());
        }
    }

    // Scan all categories to find the sidecar with this assetID.
    for (auto cat : {AssetCategory::image, AssetCategory::audio,
                     AssetCategory::video, AssetCategory::document,
                     AssetCategory::other}) {
        auto dir = categoryDir(root, cat);

        auto existsR = fs.exists(dir);
        if (!existsR.ok() || !existsR.value()) { continue;
}

        auto listR = fs.listDirectory(dir);
        if (!listR.ok()) { continue;
}

        for (const auto& entry : listR.value()) {
            auto base = util::filename(entry);
            if (base.size() < 10 ||
                base.substr(base.size() - 10) != ".meta.json") { continue;
}

            auto textR = fs.readTextFile(entry);
            if (!textR.ok()) { continue;
}
            auto metaR = schemas::parseAssetMeta(textR.value());
            if (!metaR.ok()) { continue;
}
            if (metaR.value().assetID != request.assetID) { continue;
}

            // Found — delete the BINARY first, then the sidecar.
            auto sidecarPath = entry;
            // Binary path is the sidecar path without the trailing ".meta.json"
            auto assetPath = sidecarPath.substr(0, sidecarPath.size() - 10);

            // ⚠️ Order matters, and it used to be the other way round. Both
            // results were discarded, so a sidecar that deleted while the binary
            // did not left a record pointing at nothing — and the reverse left
            // BYTES WITH NO SIDECAR, which `list` cannot see and no later remove
            // can ever find again: an unreclaimable file inside a shared world.
            //
            // Deleting the binary first means a failure between the two leaves
            // the sidecar, which is still listable and still removable — a state
            // a writer can act on. And the failure is now REPORTED rather than
            // swallowed (D6: never separate bytes from their sidecar).
            if (auto r = fs.removeFile(assetPath); !r.ok()) {
                return Result<RemoveAssetResult>::failure(r.error());
            }
            if (auto r = fs.removeFile(sidecarPath); !r.ok()) {
                return Result<RemoveAssetResult>::failure(r.error());
            }

            RemoveAssetResult result;
            result.assetID = request.assetID;
            result.deleted = true;
            return Result<RemoveAssetResult>::success(std::move(result));
        }
    }

    return Result<RemoveAssetResult>::failure(
        {.code=ErrorCode::ioError, .message="asset not found: " + request.assetID});
}

} // namespace scrivi::assets
