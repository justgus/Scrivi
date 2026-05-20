#pragma once

#include "scrivi/Types.hpp"

#include <string_view>

namespace scrivi::util {

// Converts arbitrary UTF-8 text to a lowercase hyphen-separated slug.
// Non-ASCII characters are dropped; runs of non-alphanumeric characters
// become a single hyphen; leading/trailing hyphens are stripped.
Slug makeSlug(std::string_view text);

} // namespace scrivi::util
