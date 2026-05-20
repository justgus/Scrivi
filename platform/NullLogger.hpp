#pragma once

#include "scrivi/Services.hpp"

namespace scrivi::platform {

class NullLogger final : public Logger {
public:
    void log(LogLevel, std::string_view) override {}
};

} // namespace scrivi::platform
