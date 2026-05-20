#pragma once

#include "scrivi/Services.hpp"

namespace scrivi::mocks {

class FixedClock final : public Clock {
public:
    explicit FixedClock(ISO8601Timestamp fixed) : fixed_(std::move(fixed)) {}
    ISO8601Timestamp nowUTC() override { return fixed_; }

private:
    ISO8601Timestamp fixed_;
};

} // namespace scrivi::mocks
