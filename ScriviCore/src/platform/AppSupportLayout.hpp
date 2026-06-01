#pragma once

#include "scrivi/Result.hpp"
#include "scrivi/Services.hpp"
#include "scrivi/Types.hpp"

namespace scrivi::util {

// Returns the platform-appropriate app support root for Scrivi.
//   macOS/iOS/visionOS : ~/Library/Application Support/Scrivi
//   Linux              : $XDG_DATA_HOME/Scrivi  (falls back to ~/.local/share/Scrivi)
//   Windows            : %APPDATA%\Scrivi  (via SHGetKnownFolderPath)
Result<AbsolutePath> platformDefault();

// Idempotently creates the required subdirectory tree under appSupportRoot.
// Called at facade entry points before any operation that reads or writes
// app-local state. Safe to call on a directory that already exists.
Result<void> bootstrapAppSupport(const AbsolutePath& appSupportRoot,
                                 FileSystem& fs);

} // namespace scrivi::util
