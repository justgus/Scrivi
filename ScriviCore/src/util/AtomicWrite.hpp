#pragma once

#include "scrivi/Result.hpp"
#include "scrivi/Types.hpp"

#include <cstdint>
#include <string_view>

namespace scrivi::util {

// Writes utf8Text to path atomically: writes to a sibling temp file first,
// then renames into place. On POSIX, rename(2) is atomic within a filesystem.
Result<void> atomicWriteTextFile(const AbsolutePath& path, std::string_view utf8Text);

// [SP-188] AC11 / EP-052 inventory: file operations made on THIS thread so far. Every `LocalFileSystem` call and every
// atomic write counts one (an atomic write is one call, as in `CountingFileSystem`); a project call reports the difference
// across itself (`ProjectLock.cpp`). ⚠️ Reads fanned out to reader threads (`kParallelReaders`) are not counted.
void countFileOperation();
std::int64_t fileOperationsOnThisThread();

} // namespace scrivi::util
