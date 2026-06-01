#include "assets/AssetStore.hpp"

#include "domain/Slug.hpp"
#include "schemas/AssetMetaJson.hpp"
#include "util/PathUtils.hpp"

namespace scrivi::assets {

AssetStore::AssetStore(CoreServices& services)
    : services_(services) {}

AbsolutePath AssetStore::categoryDir(const AbsolutePath& projectRoot,
                                      AssetCategory cat) 
{
    return util::join(util::join(projectRoot, "assets"),
                      assetCategorySubdir(cat));
}

// ---------------------------------------------------------------------------
// import
// ---------------------------------------------------------------------------

Result<ImportAssetResult> AssetStore::import(const ImportAssetRequest& request) const
{
    auto& fs    = *services_.fileSystem;
    auto& uuid  = *services_.uuidProvider;
    auto& clock = *services_.clock;

    auto dir = categoryDir(request.projectRootPath, request.category);
    if (auto r = fs.createDirectories(dir); !r.ok()) {
        return Result<ImportAssetResult>::failure(r.error());
}

    // Read source file bytes (binary mode via readTextFile).
    auto srcR = fs.readTextFile(request.sourcePath);
    if (!srcR.ok()) { return Result<ImportAssetResult>::failure(srcR.error());
}

    // Destination: assets/<category>/<filename>
    auto filename = util::filename(request.sourcePath);
    auto destPath = util::join(dir, filename);

    if (auto r = fs.atomicWriteTextFile(destPath, srcR.value()); !r.ok()) {
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
    if (auto r = fs.atomicWriteTextFile(sidecarPath, sidecarJson); !r.ok()) {
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

    ListAssetsResult result;

    // Helper to scan one category directory for .meta.json files.
    auto scanCategory = [&](AssetCategory cat) -> Result<void> {
        auto dir = categoryDir(request.projectRootPath, cat);

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
            result.assets.push_back(std::move(metaR.value()));
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

    // Scan all categories to find the sidecar with this assetID.
    for (auto cat : {AssetCategory::image, AssetCategory::audio,
                     AssetCategory::video, AssetCategory::document,
                     AssetCategory::other}) {
        auto dir = categoryDir(request.projectRootPath, cat);

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

            // Found — delete sidecar, then binary file.
            auto sidecarPath = entry;
            // Binary path is the sidecar path without the trailing ".meta.json"
            auto assetPath = sidecarPath.substr(0, sidecarPath.size() - 10);

            (void)fs.removeFile(sidecarPath);
            (void)fs.removeFile(assetPath);

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
