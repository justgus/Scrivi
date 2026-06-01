#include "LocalFileSystem.hpp"

#include "util/AtomicWrite.hpp"

#include <filesystem>
#include <fstream>
#include <sstream>
#include <system_error>

namespace scrivi::platform {

namespace fs = std::filesystem;

Result<bool> LocalFileSystem::exists(const AbsolutePath& path) {
    std::error_code ec;
    bool result = fs::exists(path, ec);
    if (ec) { return Result<bool>::failure({.code=ErrorCode::ioError, .message=ec.message(), .path=path});
}
    return Result<bool>::success(result);
}

Result<bool> LocalFileSystem::isDirectory(const AbsolutePath& path) {
    std::error_code ec;
    bool result = fs::is_directory(path, ec);
    if (ec) { return Result<bool>::failure({.code=ErrorCode::ioError, .message=ec.message(), .path=path});
}
    return Result<bool>::success(result);
}

Result<void> LocalFileSystem::createDirectories(const AbsolutePath& path) {
    std::error_code ec;
    fs::create_directories(path, ec);
    if (ec) { return Result<void>::failure({.code=ErrorCode::ioError, .message=ec.message(), .path=path});
}
    return Result<void>::success();
}

Result<Utf8Text> LocalFileSystem::readTextFile(const AbsolutePath& path) {
    std::ifstream in(path, std::ios::binary);
    if (!in) { return Result<Utf8Text>::failure({.code=ErrorCode::ioError, .message="could not open file", .path=path});
}
    std::ostringstream ss;
    ss << in.rdbuf();
    if (!in && !in.eof()) {
        return Result<Utf8Text>::failure({.code=ErrorCode::ioError, .message="read failed", .path=path});
}
    return Result<Utf8Text>::success(ss.str());
}

Result<void> LocalFileSystem::atomicWriteTextFile(const AbsolutePath& path, std::string_view utf8Text) {
    return util::atomicWriteTextFile(path, utf8Text);
}

Result<std::vector<AbsolutePath>> LocalFileSystem::listDirectory(const AbsolutePath& path) {
    std::error_code ec;
    std::vector<AbsolutePath> entries;
    for (const auto& entry : fs::directory_iterator(path, ec)) {
        if (ec) { return Result<std::vector<AbsolutePath>>::failure({.code=ErrorCode::ioError, .message=ec.message(), .path=path});
}
        entries.push_back(entry.path().generic_string());
    }
    if (ec) { return Result<std::vector<AbsolutePath>>::failure({.code=ErrorCode::ioError, .message=ec.message(), .path=path});
}
    return Result<std::vector<AbsolutePath>>::success(std::move(entries));
}

Result<void> LocalFileSystem::removeFile(const AbsolutePath& path) {
    std::error_code ec;
    fs::remove(path, ec);
    if (ec) { return Result<void>::failure({.code=ErrorCode::ioError, .message=ec.message(), .path=path});
}
    return Result<void>::success();
}

} // namespace scrivi::platform
