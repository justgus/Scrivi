#pragma once

#include <string>
#include <string_view>

namespace scrivi::util {

// Returns a lowercase hex-encoded SHA-256 digest of the input bytes.
std::string sha256Hex(std::string_view data);

} // namespace scrivi::util
