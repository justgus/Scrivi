#pragma once

#include "scrivi/Requests.hpp"
#include "scrivi/Results.hpp"
#include "scrivi/Services.hpp"

namespace scrivi::manuscript {

class SceneCreator {
public:
    explicit SceneCreator(CoreServices& services);

    [[nodiscard]] Result<CreateSceneResult> create(const CreateSceneRequest& request);

private:
    CoreServices& services_;
};

} // namespace scrivi::manuscript
