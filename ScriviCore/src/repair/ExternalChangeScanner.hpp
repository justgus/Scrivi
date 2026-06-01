#pragma once

#include "scrivi/Requests.hpp"
#include "scrivi/Result.hpp"
#include "scrivi/Results.hpp"
#include "scrivi/Services.hpp"

namespace scrivi::repair {

class ExternalChangeScanner {
public:
    explicit ExternalChangeScanner(CoreServices& services);

    [[nodiscard]] Result<ExternalChangeScanResult> scan(const ExternalChangeScanRequest& request) const;

private:
    CoreServices& services_;
};

} // namespace scrivi::repair
