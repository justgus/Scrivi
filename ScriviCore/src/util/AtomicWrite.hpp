#pragma once

#include "scrivi/Result.hpp"
#include "scrivi/Types.hpp"

#include <string_view>

namespace scrivi::util {

// Writes utf8Text to path atomically: writes to a sibling temp file first,
// then renames into place. On POSIX, rename(2) is atomic within a filesystem.
Result<void> atomicWriteTextFile(const AbsolutePath& path, std::string_view utf8Text);

} // namespace scrivi::util
