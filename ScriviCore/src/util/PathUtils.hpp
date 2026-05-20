#pragma once

#include "scrivi/Types.hpp"
#include "scrivi/Result.hpp"

#include <string_view>

namespace scrivi::util {

AbsolutePath join(const AbsolutePath& base, std::string_view relative);

std::string extension(const AbsolutePath& path);

AbsolutePath replaceExtension(const AbsolutePath& path, std::string_view newExt);

std::string filename(const AbsolutePath& path);

AbsolutePath parent(const AbsolutePath& path);

Result<AbsolutePath> makeAbsolute(const RelativePath& rel, const AbsolutePath& base);

} // namespace scrivi::util
