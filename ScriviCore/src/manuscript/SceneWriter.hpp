#pragma once

#include "scrivi/Requests.hpp"
#include "scrivi/Result.hpp"
#include "scrivi/Results.hpp"
#include "scrivi/Services.hpp"

namespace scrivi::manuscript {

class SceneWriter {
public:
    explicit SceneWriter(CoreServices& services);

    Result<SaveSceneResult> save(const SaveSceneRequest& request);

private:
    CoreServices& services_;
};

} // namespace scrivi::manuscript
