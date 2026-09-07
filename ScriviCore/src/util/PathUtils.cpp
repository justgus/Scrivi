#include "PathUtils.hpp"

#include "scrivi/Error.hpp"

#include <filesystem>
#include <string>

namespace scrivi::util {

namespace fs = std::filesystem;

AbsolutePath join(const AbsolutePath& base, std::string_view relative) {
    auto p = (fs::path(base) / relative).generic_string();
    return p;
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
    return Result<AbsolutePath>::success(result.lexically_normal().generic_string());
}

// --- Path validation (I-0191) ---------------------------------------------

bool containsControlCharacter(std::string_view s) {
    for (unsigned char c : s) {
        // C0 controls and DEL. Compared as `unsigned char` deliberately: on a
        // platform where `char` is signed, a UTF-8 continuation byte (>= 0x80)
        // sign-extends to a negative value and would spuriously trip a `< 0x20`
        // test, rejecting perfectly good non-ASCII paths.
        if (c < 0x20 || c == 0x7F) { return true; }
    }
    return false;
}

Result<void> validateRootPath(const AbsolutePath& path, std::string_view label) {
    if (path.empty()) {
        return Result<void>::failure({
            .code    = ErrorCode::invalidArgument,
            .message = std::string(label) + " must not be empty",
            .path    = path});
    }

    // Checked BEFORE is_absolute: a control character is the more specific
    // diagnosis, and reporting "not absolute" for a string of junk bytes would
    // send the reader looking for the wrong bug.
    if (containsControlCharacter(path)) {
        return Result<void>::failure({
            .code    = ErrorCode::invalidArgument,
            .message = std::string(label) +
                       " contains unprintable characters; this usually means an "
                       "uninitialised or dangling string crossed the C ABI",
            .path    = path});
    }

    if (!fs::path(path).is_absolute()) {
        return Result<void>::failure({
            .code    = ErrorCode::invalidArgument,
            .message = std::string(label) +
                       " must be an absolute path (a relative one resolves "
                       "against the process working directory)",
            .path    = path});
    }

    return Result<void>::success();
}

} // namespace scrivi::util
