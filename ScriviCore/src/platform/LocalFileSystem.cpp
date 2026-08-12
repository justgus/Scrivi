#include "LocalFileSystem.hpp"

#include "util/AtomicWrite.hpp"

#include <filesystem>
#include <fstream>
#include <sstream>
#include <system_error>

// Exclusive create needs a real O_EXCL — the C++ standard library offers no
// portable "create only if absent" file mode (T-0403).
#ifdef _WIN32
  #include <io.h>
  #include <share.h>
  #include <fcntl.h>
  #include <sys/stat.h>
#else
  #include <fcntl.h>
  #include <unistd.h>
#endif

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

Result<void> LocalFileSystem::createFileExclusive(const AbsolutePath& path,
                                                   std::string_view utf8Text) {
    // O_CREAT|O_EXCL is atomic at the OS level: exactly one caller can create
    // the file, and everyone else gets EEXIST. An exists()-then-write sequence
    // would race — both callers could see "absent" before either wrote.
#ifdef _WIN32
    int fd = -1;
    const errno_t err = _sopen_s(&fd, path.c_str(),
                                 _O_CREAT | _O_EXCL | _O_WRONLY | _O_BINARY,
                                 _SH_DENYNO, _S_IREAD | _S_IWRITE);
    if (err != 0 || fd < 0) {
        return Result<void>::failure(
            {.code = (err == EEXIST) ? ErrorCode::invalidArgument : ErrorCode::ioError,
             .message = (err == EEXIST) ? "file already exists" : "exclusive create failed",
             .path = path,
             .detail = (err == EEXIST) ? "alreadyExists" : ""});
    }
    const int written = _write(fd, utf8Text.data(), static_cast<unsigned>(utf8Text.size()));
    _close(fd);
#else
    const int fd = ::open(path.c_str(), O_CREAT | O_EXCL | O_WRONLY, 0644);
    if (fd < 0) {
        const bool exists = (errno == EEXIST);
        return Result<void>::failure(
            {.code = exists ? ErrorCode::invalidArgument : ErrorCode::ioError,
             .message = exists ? "file already exists" : "exclusive create failed",
             .path = path,
             .detail = exists ? "alreadyExists" : ""});
    }
    const auto written = ::write(fd, utf8Text.data(), utf8Text.size());
    ::close(fd);
#endif

    if (written < 0 || static_cast<std::size_t>(written) != utf8Text.size()) {
        // Partial write — remove the file so the caller does not hold a lock
        // whose contents are unreadable.
        std::error_code ec;
        fs::remove(path, ec);
        return Result<void>::failure(
            {.code = ErrorCode::ioError, .message = "exclusive create wrote short", .path = path});
    }

    return Result<void>::success();
}

Result<void> LocalFileSystem::appendTextFile(const AbsolutePath& path, std::string_view utf8Text) {
    std::ofstream out(path, std::ios::binary | std::ios::app);
    if (!out) {
        return Result<void>::failure({.code=ErrorCode::ioError, .message="could not open file for append", .path=path});
    }
    out.write(utf8Text.data(), static_cast<std::streamsize>(utf8Text.size()));
    out.flush();
    if (!out) {
        return Result<void>::failure({.code=ErrorCode::ioError, .message="append failed", .path=path});
    }
    return Result<void>::success();
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

Result<void> LocalFileSystem::renamePath(const AbsolutePath& from, const AbsolutePath& to) {
    std::error_code ec;

    // `from` must exist — renaming a missing path is a caller error, not I/O noise.
    if (!fs::exists(from, ec) || ec) {
        return Result<void>::failure({.code=ErrorCode::invalidArgument,
                                      .message="rename source does not exist", .path=from});
    }

    // Never clobber: refuse if the destination already exists. std::filesystem::rename
    // has platform-dependent overwrite behavior (it may replace an empty dir or a file,
    // but errors on a non-empty dir), so we guard explicitly to guarantee no destination
    // is ever destroyed — the no-clobber invariant EP-027 depends on (cf. I-0072, where a
    // slug collision overwrote a live chapter sidecar). std::error_code overload (no throw).
    ec.clear();
    if (fs::exists(to, ec) || ec) {
        return Result<void>::failure({.code=ErrorCode::invalidArgument,
                                      .message="rename destination already exists", .path=to});
    }

    // Atomic within a filesystem: the OS rename either fully succeeds or fully fails, so a
    // crash mid-rename never leaves a half-moved directory.
    ec.clear();
    fs::rename(from, to, ec);
    if (ec) {
        // A cross-filesystem move surfaces as cross_device_link — report it rather than
        // silently doing a non-atomic copy+delete (in-package moves are same-filesystem,
        // so this should not occur for chapter/scene folders).
        return Result<void>::failure({.code=ErrorCode::ioError, .message=ec.message(),
                                      .path=from, .detail=to});
    }
    return Result<void>::success();
}

} // namespace scrivi::platform
