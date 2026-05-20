#pragma once

#include "scrivi/RepairIssue.hpp"
#include "scrivi/Result.hpp"
#include "scrivi/Services.hpp"
#include "scrivi/Types.hpp"

#include <vector>

namespace scrivi::project_package {

class ProjectValidator {
public:
    explicit ProjectValidator(CoreServices& services);

    // Check for structural issues in the project package. Returns all detected
    // issues; an empty vector means the project is healthy.
    Result<std::vector<RepairIssue>> validate(const AbsolutePath& projectRoot);

private:
    CoreServices& services_;
};

} // namespace scrivi::project_package
