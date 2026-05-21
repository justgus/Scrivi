#pragma once

#include "scrivi/Requests.hpp"
#include "scrivi/Result.hpp"
#include "scrivi/Results.hpp"
#include "scrivi/Services.hpp"

namespace scrivi::identity {

class IdentityService {
public:
    explicit IdentityService(CoreServices& services);

    Result<EnsureIdentityResult> ensureLocalIdentity(
        const EnsureIdentityRequest& request);

private:
    CoreServices& services_;

    // Storage key under which the identity bundle is kept in SecureStore.
    static constexpr std::string_view kIdentityKey = "scrivi.identity.v1";
};

} // namespace scrivi::identity
