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
};

} // namespace scrivi::assets
