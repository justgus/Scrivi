#include "LocalFileSystem.hpp"

#include "util/AtomicWrite.hpp"
#include "util/PathUtils.hpp"

#include <cerrno>
#include <filesystem>
#include <fstream>
#include <sstream>
#include <system_error>
#include <vector>

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

namespace {

// T-0478 -- map errno to a STABLE, machine-readable discriminator for Error::detail.
//
// ⚠️ Only the values a platform layer can ACT on are named. Everything else
// returns "" and the caller keeps the honest generic failure: inventing a
// discriminator for an errno nobody handles would grow a vocabulary that drifts
// out of sync with its readers.
//
// ⚠️ EHOSTDOWN/EHOSTUNREACH/ENETDOWN/ENETUNREACH are the SERVER-IS-GONE family.
// SP-124's S2 measured `EHOSTDOWN` (112) from a `cifs` mount whose serving host
// stopped sharing -- ✅ specific evidence of a REMOTE volume being unreachable,
// which is exactly what `WorldStatus::offline` means and what nothing has ever
// been able to prove before.
//
// ⚠️ ESTALE and EIO are DELIBERATELY NOT in that family. A stale handle or an I/O
// error can equally mean a LOCAL device vanished, and `offline` asserts something
// stronger -- that the volume is remote. Reporting `offline` for a pulled USB
// stick would be a new wrong answer, not a sharper one.
std::string errnoDetail(int err) {
    if (err == 0) { return {}; }

    // ⚠️ Compared as VALUES, not `case` labels: MSVC does not define EHOSTDOWN at
    // all, and several of these alias one another on some platforms -- duplicate
    // `case` labels would not compile. A chain of `if`s is portable and lets each
    // constant be guarded independently.
#ifdef EHOSTDOWN
    if (err == EHOSTDOWN)    { return "hostUnreachable"; }
#endif
#ifdef EHOSTUNREACH
    if (err == EHOSTUNREACH) { return "hostUnreachable"; }
#endif
#ifdef ENETDOWN
    if (err == ENETDOWN)     { return "hostUnreachable"; }
#endif
#ifdef ENETUNREACH
    if (err == ENETUNREACH)  { return "hostUnreachable"; }
#endif
    return {};
}

} // namespace

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
    // I-0191 / AC2: ScriviCore must NEVER create a directory whose name holds
    // unprintable characters. Enforced HERE rather than at the call sites --
    // this is the single virtual chokepoint every core directory write passes
    // through, so one check covers all of them and cannot go stale the way a
    // restated per-caller rule would.
    //
    // Such a name is never a legitimate request. It is the signature of a
    // dangling or uninitialised `const char*` crossing the C ABI, and creating
    // it silently turns a memory-safety bug into filesystem litter (three such
    // directories appeared in the repo root on 2026-08-17).
    if (util::containsControlCharacter(path)) {
        return Result<void>::failure({
            .code    = ErrorCode::invalidArgument,
            .message = "refusing to create a directory whose path contains "
                       "unprintable characters",
            .path    = path});
    }

    std::error_code ec;
    fs::create_directories(path, ec);
    if (ec) { return Result<void>::failure({.code=ErrorCode::ioError, .message=ec.message(), .path=path});
}
    return Result<void>::success();
}

Result<Utf8Text> LocalFileSystem::readTextFile(const AbsolutePath& path) {
    // T-0478: errno is captured BEFORE anything else can clobber it.
    //
    // ⚠️ `std::ifstream` discards the reason a file could not be opened -- every
    // failure arrives as a bare "could not open file". That flattening is why a
    // DEAD NETWORK SERVER was indistinguishable from a deleted package, and why
    // `WorldStatus::offline` has never been produced by any platform (SP-124 S2
    // measured the network case returning `unavailable`, twice).
    //
    // ⚠️ errno is only meaningful immediately after the failing call, so it is
    // read on the very next line -- not after constructing an Error, not after a
    // log call. See `errnoDetail` for what survives into the envelope.
    errno = 0;
    std::ifstream in(path, std::ios::binary);
    if (!in) {
        const int err = errno;
        return Result<Utf8Text>::failure({.code    = ErrorCode::ioError,
                                          .message = "could not open file",
                                          .path    = path,
                                          .detail  = errnoDetail(err)});
    }
    std::ostringstream ss;
    ss << in.rdbuf();
    if (!in && !in.eof()) {
        const int err = errno;
        return Result<Utf8Text>::failure({.code    = ErrorCode::ioError,
                                          .message = "read failed",
                                          .path    = path,
                                          .detail  = errnoDetail(err)});
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


Result<void> LocalFileSystem::copyFileInBlocks(
    const AbsolutePath& from,
    const AbsolutePath& to,
    std::size_t blockSize,
    const std::function<Result<void>()>& onBlock)
{
    if (blockSize == 0) { blockSize = 1u << 20; }   // 1 MiB default

    std::ifstream in(from, std::ios::binary);
    if (!in) {
        return Result<void>::failure(
            {.code = ErrorCode::ioError, .message = "cannot open source", .path = from});
    }

    // ⚠️ Copy to a TEMPORARY and rename on success. The destination must never
    // exist in a partial state: list() would not see it (no sidecar yet), but a
    // later import of the same filename would find bytes it did not write. The
    // rename is atomic within a filesystem, so a reader sees all or nothing.
    const AbsolutePath tmp = to + ".partial";
    {
        std::ofstream out(tmp, std::ios::binary | std::ios::trunc);
        if (!out) {
            return Result<void>::failure(
                {.code = ErrorCode::ioError, .message = "cannot open destination", .path = tmp});
        }

        std::vector<char> buf(blockSize);
        while (in) {
            in.read(buf.data(), static_cast<std::streamsize>(blockSize));
            const auto got = in.gcount();
            if (got <= 0) { break; }

            out.write(buf.data(), got);
            if (!out) {
                out.close();
                std::error_code ec;
                std::filesystem::remove(tmp, ec);
                return Result<void>::failure(
                    {.code = ErrorCode::ioError, .message = "write failed", .path = tmp});
            }

            // Kick the watchdog. A failure here means the caller has lost its
            // lock (or otherwise wants out) — stop rather than write on, and
            // leave nothing behind.
            if (onBlock) {
                if (auto r = onBlock(); !r.ok()) {
                    out.close();
                    std::error_code ec;
                    std::filesystem::remove(tmp, ec);
                    return r;
                }
            }
        }

        out.flush();
        if (!out) {
            out.close();
            std::error_code ec;
            std::filesystem::remove(tmp, ec);
            return Result<void>::failure(
                {.code = ErrorCode::ioError, .message = "flush failed", .path = tmp});
        }
    }

    std::error_code ec;
    std::filesystem::rename(tmp, to, ec);
    if (ec) {
        std::error_code rm;
        std::filesystem::remove(tmp, rm);
        return Result<void>::failure(
            {.code = ErrorCode::ioError, .message = "rename failed: " + ec.message(), .path = to});
    }
    return Result<void>::success();
}

} // namespace scrivi::platform
