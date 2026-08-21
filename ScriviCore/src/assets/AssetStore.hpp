#pragma once

#include "scrivi/Requests.hpp"
#include "scrivi/Result.hpp"
#include "scrivi/Results.hpp"
#include "scrivi/Services.hpp"

namespace scrivi::assets {

// Handles importAsset / listAssets / removeAsset against assets/<category>/ in the project.
class AssetStore {
public:
    explicit AssetStore(CoreServices& services);

    [[nodiscard]] Result<ImportAssetResult> import(const ImportAssetRequest& request) const;
    [[nodiscard]] Result<ListAssetsResult>  list(const ListAssetsRequest& request) const;
    [[nodiscard]] Result<RemoveAssetResult> remove(const RemoveAssetRequest& request) const;

private:
    CoreServices& services_;

    [[nodiscard]] static AbsolutePath categoryDir(const AbsolutePath& projectRoot, AssetCategory cat);

    // D6 (SP-116 T-0426): resolves the root that assets live under — the project
    // itself when `worldID` is empty, otherwise the world's package path.
    //
    // ⚠️ Mirrors ObjectStore::kindDirFor line for line, INCLUDING the
    // "worldUnavailable:<status>" error detail: an unreachable world is not a
    // missing asset, and both surfaces must fail identically so the app needs
    // one handler rather than two.
    [[nodiscard]] Result<AbsolutePath> assetRoot(const AbsolutePath& projectRoot,
                                                  const std::string& worldID) const;
};

} // namespace scrivi::assets
