#pragma once

#include <cstdint>
#include <string>

namespace scrivi {

enum class ErrorCode : std::uint8_t {
    ok,

    invalidArgument,
    unsupportedVersion,

    ioError,
    permissionDenied,
    parseError,
    validationError,

    repairRequired,

    gitUnavailable,
    gitError,

    secureStoreUnavailable,
    secureStoreError,

    identityError,

    internalError
};

struct Error {
    ErrorCode code = ErrorCode::ok;
    std::string message;
    std::string path;
    std::string detail;
};

} // namespace scrivi
