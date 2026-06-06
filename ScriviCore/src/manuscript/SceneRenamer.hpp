#pragma once

#include "scrivi/Requests.hpp"
#include "scrivi/Results.hpp"
#include "scrivi/Services.hpp"

namespace scrivi::manuscript {

class SceneRenamer {
public:
    explicit SceneRenamer(CoreServices& services);

    [[nodiscard]] Result<RenameSceneResult> rename(const RenameSceneRequest& request);

    // Walks the manuscript index to resolve a sceneID to its metadataPath.
    // Available for future operations that need path resolution from an ID.
    [[nodiscard]] Result<RelativePath> findSceneMetadataPath(
        const AbsolutePath& projectRootPath, const SceneID& sceneID);

private:
    CoreServices& services_;
};

} // namespace scrivi::manuscript
