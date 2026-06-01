#include "AtomicWrite.hpp"

#include "scrivi/Error.hpp"

#include <filesystem>
#include <fstream>
#include <system_error>

namespace scrivi::util {

namespace fs = std::filesystem;

Result<void> atomicWriteTextFile(const AbsolutePath& path, std::string_view utf8Text) {
    auto target  = fs::path(path);
    auto tmpPath = target;
    tmpPath += ".tmp";

    {
        std::ofstream out(tmpPath, std::ios::binary | std::ios::trunc);
        if (!out) {
            return Result<void>::failure({.code = ErrorCode::ioError,
                .message = "could not open temp file for writing", .path = tmpPath.string()});
        }
        out.write(utf8Text.data(), static_cast<std::streamsize>(utf8Text.size()));
        if (!out) {
            return Result<void>::failure({.code = ErrorCode::ioError,
                .message = "write failed", .path = tmpPath.string()});
        }
    }

    std::error_code ec;
    fs::rename(tmpPath, target, ec);
    if (ec) {
        fs::remove(tmpPath, ec);
        return Result<void>::failure({.code = ErrorCode::ioError,
            .message = "rename failed: " + ec.message(), .path = path});
    }

    return Result<void>::success();
}

} // namespace scrivi::util
