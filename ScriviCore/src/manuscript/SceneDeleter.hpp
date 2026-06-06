#pragma once

#include "scrivi/Requests.hpp"
#include "scrivi/Results.hpp"
#include "scrivi/Services.hpp"

namespace scrivi::manuscript {

class SceneDeleter {
public:
    explicit SceneDeleter(CoreServices& services);

    [[nodiscard]] Result<DeleteSceneResult> remove(const DeleteSceneRequest& request);

private:
    CoreServices& services_;
};

} // namespace scrivi::manuscript
