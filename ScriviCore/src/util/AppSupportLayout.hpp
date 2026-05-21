#pragma once

#include "scrivi/Result.hpp"
#include "scrivi/Services.hpp"
#include "scrivi/Types.hpp"

namespace scrivi::util {

// Idempotently creates the required subdirectory tree under appSupportRoot.
// Called at facade entry points before any operation that reads or writes
// app-local state. Safe to call on a directory that already exists.
Result<void> bootstrapAppSupport(const AbsolutePath& appSupportRoot,
                                 FileSystem& fs);

} // namespace scrivi::util
