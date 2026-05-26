#pragma once

#include "scrivi/Requests.hpp"
#include "scrivi/Results.hpp"
#include "scrivi/Result.hpp"
#include "scrivi/Services.hpp"

namespace scrivi::repair {

// Stateless repair dispatcher. Re-reads project state on each call to validate
// the issue still applies, then routes to the correct handler based on actionKind.
class RepairDispatcher {
public:
    explicit RepairDispatcher(CoreServices services);

    Result<ApplyRepairResult> apply(const ApplyRepairRequest& request);

private:
    CoreServices services_;
};

} // namespace scrivi::repair
