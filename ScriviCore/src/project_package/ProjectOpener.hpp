#pragma once

#include "scrivi/Requests.hpp"
#include "scrivi/Result.hpp"
#include "scrivi/Results.hpp"
#include "scrivi/Services.hpp"

namespace scrivi::project_package {

class ProjectOpener {
public:
    explicit ProjectOpener(CoreServices& services);

    Result<OpenProjectResult> open(const OpenProjectRequest& request);

private:
    CoreServices& services_;
};

} // namespace scrivi::project_package
