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

    Result<ImportAssetResult> import(const ImportAssetRequest& request);
    Result<ListAssetsResult>  list(const ListAssetsRequest& request);
    Result<RemoveAssetResult> remove(const RemoveAssetRequest& request);

private:
    CoreServices& services_;

    AbsolutePath categoryDir(const AbsolutePath& projectRoot, AssetCategory cat) const;
};

} // namespace scrivi::assets
