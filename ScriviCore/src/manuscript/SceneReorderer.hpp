#pragma once

#include "scrivi/Requests.hpp"
#include "scrivi/Results.hpp"
#include "scrivi/Services.hpp"

namespace scrivi::manuscript {

class SceneReorderer {
public:
    explicit SceneReorderer(CoreServices& services);

    [[nodiscard]] Result<ReorderSceneResult> reorder(const ReorderSceneRequest& request);

private:
    CoreServices& services_;
};

} // namespace scrivi::manuscript
