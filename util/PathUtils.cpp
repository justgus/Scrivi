#include "PathUtils.hpp"

#include "scrivi/Error.hpp"

#include <filesystem>

namespace scrivi::util {

namespace fs = std::filesystem;

AbsolutePath join(const AbsolutePath& base, std::string_view relative) {
    return (fs::path(base) / relative).string();
}

std::string extension(const AbsolutePath& path) {
    return fs::path(path).extension().string();
}

AbsolutePath replaceExtension(const AbsolutePath& path, std::string_view newExt) {
    return fs::path(path).replace_extension(newExt).string();
}

std::string filename(const AbsolutePath& path) {
    return fs::path(path).filename().string();
}

AbsolutePath parent(const AbsolutePath& path) {
    return fs::path(path).parent_path().string();
}

Result<AbsolutePath> makeAbsolute(const RelativePath& rel, const AbsolutePath& base) {
    auto result = fs::path(base) / rel;
    return Result<AbsolutePath>::success(result.lexically_normal().string());
}

} // namespace scrivi::util
